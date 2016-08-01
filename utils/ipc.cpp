// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2015-01-05
//

#include "ipc.h"


CShm::CShm(uint32_t key, uint32_t size, bool del)
{
    m_size = size;
    m_key = key;
    m_delete_on_exit = del;

    m_shmid = shmget((key_t)key, size, 0666|IPC_CREAT);
    if (m_shmid == -1) {
        fprintf(stderr, "shmget error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        throw std::string("shmget error");
    }

    m_ptr = (char*) shmat(m_shmid, (void*)0, 0);
    if (m_ptr == NULL) {
        fprintf(stderr, "shmat error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        throw std::string("shmat error");
    }
}

CShm::~CShm()
{
    shmdt(m_ptr);
    if (m_delete_on_exit) {
        shmctl(m_shmid, IPC_RMID, NULL);
    }
}

void CShm::Clear()
{
    memset(m_ptr, 0, m_size);
}

int CShm::Write(const char *msg_data, uint32_t msg_len, uint32_t pos)
{
    if (msg_data == NULL || msg_len > m_size || (pos+msg_len) > m_size) {
        return -1;
    }
    memcpy(m_ptr+pos, msg_data, msg_len);
    return msg_len;
}

int CShm::Read(char *msg_data, uint32_t msg_len, uint32_t pos)
{
    if (msg_data == NULL || msg_len > m_size || (pos+msg_len) > m_size) {
        return -1;
    }
    memcpy(msg_data, m_ptr + pos, msg_len);
    return msg_len;
}

/**  ====================================CSem====================================*/
CSem::CSem(uint32_t key, bool del)
{
    m_key = key;
    m_delete_on_exit = del;

    m_semid = semget((key_t)key, 1, 0666|IPC_CREAT);
    if (m_semid < 0 ) {
        fprintf(stderr, "semget error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
        throw std::string("semget error");
    }
}

CSem::~CSem()
{
    if (m_delete_on_exit) {
        semctl(m_semid, 0, IPC_RMID);
    }
}

void CSem::Post()
{
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    if (semop(m_semid, &sb, 1) == -1) {
        fprintf(stderr, "semop post error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
    }
}

void CSem::Wait()
{
    struct sembuf sb;
    for (uint32_t i = 0; i < 5; i++) {
        sb.sem_num = 0;
        sb.sem_op = -1;
        sb.sem_flg = SEM_UNDO;
        int ret = semop(m_semid, &sb, 1);
        if (ret) {
            if (errno != EINTR) {
                throw std::string("semop wait error!");
            }
        } else {
            return ;
        }
    }
    throw std::string("semop wait 5 time error!");
}

bool CSem::TryWait(struct timespec *ts)
{
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO | IPC_NOWAIT;

    /// no wait
    if (ts == NULL) {
        if (semop(m_semid, &sb, 1) == -1) {
            if (errno != EAGAIN) {
                fprintf(stderr, "semop wait error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
            }
            return false;
        }
        return true;
    }

    ///wait time
    for (int i = 0;i < 5; i++) {
        if (semtimedop(m_semid, &sb, 1, ts) == -1) {
            if (errno != EAGAIN) {
                fprintf(stderr, "semtimedop wait error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
                return false;
            } else if (errno == EINTR) {
                continue;
            } else {
                return false;
            }
        }
        else {
            return true;
        }
    }

    return false;
}

int CSem::GetValue()
{
    int ret = semctl(m_semid, 0, GETVAL, NULL);
    if (ret == -1) {
        fprintf(stderr, "semctl getvalue error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
    }
    return ret;
}

void CSem::SetValue(int val)
{
    if (semctl(m_semid, 0, SETVAL, val) == -1) {
        fprintf(stderr, "semctl setvalue error![%s:%d][%s]\n", __FILE__, __LINE__,strerror(errno));
    }
}
