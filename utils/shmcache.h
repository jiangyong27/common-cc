// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2014-11-13
//

#ifndef _DATA_JASONYJIANG_COMMON_CPP_SHM_CACHE_H
#define _DATA_JASONYJIANG_COMMON_CPP_SHM_CACHE_H
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <map>
#include <list>
#include <vector>
#include <iostream>

///////////////////////////////////////////////////////
const uint32_t HASH_MASK = 65536;

struct SBlockHead
{
    uint64_t m_key;
    uint32_t m_idx;
    uint32_t m_len;

    int32_t m_next;
    int32_t m_lru_pre;
    int32_t m_lru_next;

    time_t m_uptime;
};

struct SHashHead
{
    uint32_t m_len;
    uint32_t m_head;
};

struct SCacheStat
{
    uint32_t m_block_num;
    uint32_t m_block_size;
    uint32_t m_total_size;
    uint32_t m_use_size;
    uint32_t m_use_block;
    uint32_t m_has_key;
    uint32_t m_total_req;
    uint32_t m_total_hit;
};

class CShmCache
{
public:
    enum {
        ERR_OK = 0,
        ERR_KEY_NO_EXIST = -1,
        ERR_KEY_TIMEOUT = -2,
        ERR_KEY_ZERO = -3,
        ERR_IPC = -4,
    };
public:
    CShmCache(bool lock = true);
    ~CShmCache();
    int Get(uint64_t key, char *data_buf, uint32_t *data_len);
    int Set(uint64_t key, const char *data_buf, uint32_t data_len);
    int Del(uint64_t key);
    int Init(uint32_t shm_key, uint32_t block_num, uint32_t block_size, uint32_t timeout = 60);

    void Check();
    void PrintHash();
    void PrintLRU();
    void GetStat(SCacheStat *stat);
    void PrintStat();

private:
    void MoveLRUTail(int32_t idx);
    void MoveLRUHead(int32_t idx);
    void DeleteHash(uint64_t key);
    void InsertHash(int32_t idx);
    void Lock();
    void Unlock();
    bool InitIPC(uint32_t key);

private:
    bool m_lock;
    int m_semid;
    int m_shmid;

    uint32_t m_block_num;
    uint32_t m_block_size;
    uint32_t m_timeout;
    uint32_t m_total_size;

    uint32_t m_total_hit;
    uint32_t m_total_req;

    char *m_data_buffer;
    SBlockHead *m_block;
    SHashHead *m_hash;
    int32_t *m_lru_head;
    int32_t *m_lru_tail;
    time_t *m_op_time;
    uint32_t *m_shm_size;
};

#endif // _DATA_JASONYJIANG_COMMON_CPP_TEST_SIMPLE_CACHE_H
