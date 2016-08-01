// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2014-11-13
//

#include "shmcache.h"

/// ====================================CShmCache========================

CShmCache::CShmCache(bool lock)
{
    m_lock = lock;
    m_block_num = 0;
    m_block_size = 0;
    m_timeout = 60;
}

CShmCache::~CShmCache()
{
    *m_shm_size = 0;
    if (m_data_buffer)
        shmdt(m_data_buffer);
}

bool CShmCache::InitIPC(uint32_t key)
{
    m_total_size = m_block_num * m_block_size + m_block_num * sizeof(SBlockHead)
        + HASH_MASK * sizeof(SHashHead) + 3 * sizeof(uint32_t);

    m_shmid = shmget((key_t)key, m_total_size, 0666|IPC_CREAT);
    if (m_shmid == -1) {
        fprintf(stderr, "shmget error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        return false;
    }

    m_data_buffer = (char*) shmat(m_shmid, (void*)0, 0);
    if (m_data_buffer == NULL) {
        fprintf(stderr, "shmat error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        return false;
    }

    m_semid = semget((key_t)key, 1, 0666|IPC_CREAT);
    if (m_semid < 0 ) {
        fprintf(stderr, "semget error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        return false;
    }
    return true;
}

int CShmCache::Init(uint32_t ipc_key, uint32_t block_num, uint32_t block_size, uint32_t timeout)
{
    m_block_num = block_num;
    m_block_size = block_size;
    m_timeout = timeout;

    if (!InitIPC(ipc_key)) {
        return ERR_IPC;
    }

    m_block = (SBlockHead *) (m_data_buffer + block_num * block_size);
    m_hash = (SHashHead *) (m_block + block_num);
    m_lru_head = (int32_t *) (m_hash + HASH_MASK);
    m_lru_tail = (int32_t *) (m_lru_head + 1);
    m_shm_size = (uint32_t*) (m_lru_head + 2); /// 4字节
    m_op_time = (time_t*) (m_lru_head + 3); /// 8字节

    if (*m_shm_size == m_total_size) { /// 已经初始化过
        struct tm tm;
        localtime_r(m_op_time, &tm);
        printf("share memory cache already init![%04d-%02d-%02d %02d:%02d:%02d]\n",
                 1900+tm.tm_year, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        return ERR_OK;
    }

    /// 初始化可用
    if (semctl(m_semid, 0, SETVAL, 1) == -1) {
        fprintf(stderr, "semctr error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        return ERR_IPC;
    }

    Lock();
    printf("init share memory cache!total_size[%d]M\n", (int)(m_total_size / (1024*1024)));
    *m_shm_size = m_total_size;
    *m_op_time = time(NULL);

    for (uint32_t idx = 0; idx < block_num; idx++) {
        SBlockHead& block = m_block[idx];
        block.m_idx = idx;
        block.m_len = 0;
        block.m_next = -1;
        block.m_key = 0;

        if (idx == 0) {
            block.m_lru_pre = block_num - 1;
        } else {
            block.m_lru_pre = idx - 1;
        }

        if (idx == block_num - 1) {
            block.m_lru_next = 0;
        } else {
            block.m_lru_next = idx + 1;
        }
    }

    for (uint32_t idx = 0; idx < HASH_MASK; idx++) {
        m_hash[idx].m_len = 0;
        m_hash[idx].m_head = -1;
    }

    *m_lru_head = 0;
    *m_lru_tail = block_num - 1;
    Unlock();
    return ERR_OK;
}

int CShmCache::Set(uint64_t key, const char *data_buf, uint32_t data_len)
{
    if (key == 0) {
        printf("use key is 0\n");
        return ERR_KEY_ZERO;
    }
    Lock();
    time_t now = time(NULL);

    /// 删除原key
    DeleteHash(key);

    /// 获取最近最少使用的节点
    uint32_t copyed = 0;
    while(data_len > 0) {
        SBlockHead b = m_block[*m_lru_head];
        while (b.m_key != 0) {
            DeleteHash(b.m_key);
            b = m_block[*m_lru_head];
        }

        SBlockHead& block = m_block[*m_lru_head];
        if (block.m_key != 0) {
            printf("no zero!\n");
            assert(0);
        }
        block.m_key = key;
        block.m_uptime = now;

        if (data_len <= m_block_size) {
            memcpy(m_data_buffer + block.m_idx * m_block_size, data_buf + copyed, data_len);
            block.m_len = data_len;
            copyed += data_len;
            data_len = 0;
        } else {
            memcpy(m_data_buffer + block.m_idx * m_block_size, data_buf + copyed, m_block_size);
            data_len -= m_block_size;
            copyed += m_block_size;
            block.m_len = m_block_size;
        }

        /// 插入hash
        InsertHash(block.m_idx);
    }

    Unlock();
    return ERR_OK;
}

int CShmCache::Get(uint64_t key, char *data_buf, uint32_t *data_len)
{
    Lock();
    uint32_t hash_key = key % HASH_MASK;
    int32_t cur = m_hash[hash_key].m_head;
    time_t now = time(NULL);

    int iRet = ERR_KEY_NO_EXIST;
    *data_len = 0;
    m_total_req++;
    while(cur != -1) {
        SBlockHead& block = m_block[cur];
        if (block.m_key == key) {
            /// 超时 删除
            if (now - block.m_uptime > m_timeout) {
                DeleteHash(key);
                iRet = ERR_KEY_TIMEOUT;
                break;
            }

            memcpy(data_buf + *data_len, m_data_buffer + block.m_idx * m_block_size, block.m_len);
            *data_len += block.m_len;
            iRet = ERR_OK;
            MoveLRUTail(block.m_idx);
        } else if (block.m_key > key) {
            break;
        }
        cur = m_block[cur].m_next;
    }
    Unlock();
    if (iRet == ERR_OK) {
        m_total_hit++;
    }
    return iRet;
}


int CShmCache::Del(uint64_t key)
{
    Lock();
    DeleteHash(key);
    Unlock();
    return ERR_OK;
}

void CShmCache::Check()
{
    Lock();
    time_t now = time(NULL);

    int32_t cur;
    for (uint32_t idx = 0; idx < HASH_MASK; ++idx) {
        cur = m_hash[idx].m_head;

        while(cur != -1) {
            SBlockHead& block = m_block[cur];
            if (now - block.m_uptime > m_timeout) {
                cur = m_block[cur].m_next;
                DeleteHash(block.m_key);
                continue;
            }
            cur = m_block[cur].m_next;
        }
    }

    Unlock();
}

void CShmCache::InsertHash(int32_t idx)
{
    uint64_t key = m_block[idx].m_key;
    uint32_t hash_key = key % HASH_MASK;
    static int c = 0;
    c++;

    ///插入
    int32_t cur = m_hash[hash_key].m_head;
    m_hash[hash_key].m_len++;
    int32_t pre = -1;
    while(cur != -1) {
        if (key >= m_block[cur].m_key) {
            pre = cur;
            cur = m_block[cur].m_next;
        } else {
            m_block[idx].m_next = m_block[cur].m_idx;
            assert(pre != idx);
            pre != -1 ? m_block[pre].m_next = idx : m_hash[hash_key].m_head = idx;
            MoveLRUTail(idx);
            return;
        }
        //printf("insert\n");
        //this->PrintHash();
    }

    m_block[idx].m_next = -1;
    assert(pre != idx);
    pre != -1 ? m_block[pre].m_next = idx : m_hash[hash_key].m_head = idx;
    MoveLRUTail(idx);
}

void CShmCache::DeleteHash(uint64_t key)
{
    uint32_t hash_key = key % HASH_MASK;
    SHashHead& hash_head = m_hash[hash_key];

    /// 删除原key
    int32_t cur = hash_head.m_head;
    int32_t pre = -1;
    int32_t tmp = -1;
    while(cur != -1) {
        if (m_block[cur].m_key == key) {
            pre != -1 ? m_block[pre].m_next = m_block[cur].m_next : hash_head.m_head = m_block[cur].m_next;
            tmp = m_block[cur].m_next;

            m_block[cur].m_key = 0;
            m_block[cur].m_next = -1;
            m_block[cur].m_len = 0;

            hash_head.m_len--;
            MoveLRUHead(cur);
            cur = tmp;
        } else if (key > m_block[cur].m_key) {
            pre = cur;
            cur = m_block[cur].m_next;
        } else {
            break;
        }
        //printf("del\n");
    }
}

void CShmCache::MoveLRUTail(int32_t idx)
{
    if (*m_lru_tail == idx) {
        return ;
    }

    if (*m_lru_head == idx) {
        *m_lru_tail = *m_lru_head;
        *m_lru_head = m_block[idx].m_lru_next;
        return;
    }

    SBlockHead& block = m_block[idx];
    m_block[block.m_lru_pre].m_lru_next = block.m_lru_next;
    m_block[block.m_lru_next].m_lru_pre= block.m_lru_pre;

    block.m_lru_pre = *m_lru_tail;
    block.m_lru_next = *m_lru_head;
    m_block[*m_lru_tail].m_lru_next = idx;
    m_block[*m_lru_head].m_lru_pre = idx;

    *m_lru_tail = idx;
}

void CShmCache::MoveLRUHead(int32_t idx)
{
    if (*m_lru_head == idx) {
        return ;
    }

    if (*m_lru_tail == idx) {
        *m_lru_head = *m_lru_tail;
        *m_lru_tail = m_block[idx].m_lru_pre;
        return;
    }

    SBlockHead& block = m_block[idx];
    m_block[block.m_lru_pre].m_lru_next = block.m_lru_next;
    m_block[block.m_lru_next].m_lru_pre = block.m_lru_pre;

    block.m_lru_pre = *m_lru_tail;
    block.m_lru_next = *m_lru_head;
    m_block[*m_lru_tail].m_lru_next = idx;
    m_block[*m_lru_head].m_lru_pre = idx;

    *m_lru_head = idx;
}

void CShmCache::Lock()
{
    if (m_lock) {
        int ret = 0;
        struct sembuf sb;
        sb.sem_num = 0;
        sb.sem_op = -1;
        sb.sem_flg = SEM_UNDO;
        while(1) {
            ret = semop(m_semid, &sb, 1);
            if (!ret) {
                break;
            } else if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                usleep(100);
                continue;
            } else {
                fprintf(stderr, "semop post error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
            }
        }
    }
}

void CShmCache::Unlock()
{
    if (m_lock) {
        struct sembuf sb;
        sb.sem_num = 0;
        sb.sem_op = 1;
        sb.sem_flg = SEM_UNDO;
        if (semop(m_semid, &sb, 1) == -1) {
            fprintf(stderr, "semop post error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        }
    }
}

void CShmCache::GetStat(SCacheStat *stat)
{
    Lock();
    stat->m_total_req = m_total_req;
    stat->m_total_hit = m_total_hit;
    stat->m_total_size = m_block_num * m_block_size;
    stat->m_block_num = m_block_num;
    stat->m_block_size = m_block_size;
    stat->m_use_block = 0;
    stat->m_use_size = 0;
    stat->m_has_key = 0;

    for (uint32_t i = 0;i < HASH_MASK; ++i) {
        SHashHead& head = m_hash[i];
        if (head.m_len == 0) continue;
        stat->m_use_block += head.m_len;

        int32_t cur = head.m_head;
        uint64_t pre_key = 0;
        while(cur != -1) {
            SBlockHead& block = m_block[cur];
            stat->m_use_size += block.m_len;
            if (pre_key != block.m_key) {
                stat->m_has_key++;
            }
            pre_key = block.m_key;
            cur = block.m_next;
        }
    }

    m_total_req = 0;
    m_total_hit = 0;
    Unlock();
}

void CShmCache::PrintStat()
{
    SCacheStat stat;
    GetStat(&stat);
    printf("has_key[%d],hit_rate[%d/%d=%.2f],use_block[%d/%d=%0.2f],use_size[%d/%d=%2.f]\n",
           stat.m_has_key, stat.m_total_hit, stat.m_total_req, float(stat.m_total_hit)/stat.m_total_req,
        stat.m_use_block, stat.m_block_num, float(stat.m_use_block) / stat.m_block_num,
           stat.m_use_size, stat.m_total_size, float(stat.m_use_size) / stat.m_total_size);
    //printf("%d\n", m_total_size);
}

void CShmCache::PrintHash()
{
    for (uint32_t i = 0;i < HASH_MASK; ++i) {
        SHashHead& head = m_hash[i];
        if (head.m_len == 0) continue;
        printf("hash[%d][%d]:", i, head.m_len);
        int32_t cur = head.m_head;
        while(cur != -1) {
            SBlockHead& block = m_block[cur];
            printf("->[idx=%d][key=%llu]%s", block.m_idx, (long long unsigned int)block.m_key,
                   std::string(m_data_buffer + block.m_idx * m_block_size, block.m_len).c_str());
            cur = block.m_next;
        }
        printf("\n");
    }
}

void CShmCache::PrintLRU()
{
    int32_t cur = *m_lru_head;
    printf("LRU:");
    while(cur != *m_lru_tail) {
        SBlockHead& block = m_block[cur];

        printf("[idx=%d][key=%llu]%s->", block.m_idx, (long long unsigned int)block.m_key,
               std::string(m_data_buffer + block.m_idx * m_block_size, block.m_len).c_str());
        cur = block.m_lru_next;
    }

    SBlockHead& block = m_block[*m_lru_tail];
    printf("[idx=%d][key=%llu]%s->", block.m_idx,( long long unsigned int) block.m_key,
           std::string(m_data_buffer + block.m_idx * m_block_size, block.m_len).c_str());
    printf("\n");
}
