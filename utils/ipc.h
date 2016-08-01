// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2015-01-05
//

#ifndef _ROOT_MTX_TEST_IPC_H
#define _ROOT_MTX_TEST_IPC_H
#pragma once
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <iostream>

class CShm
{
public:
    CShm(uint32_t key, uint32_t size, bool del = false);
    ~CShm();
    int Write(const char *msg_data, uint32_t msg_len, uint32_t pos = 0);
    int Read(char *msg_data, uint32_t msg_len, uint32_t pos = 0);

    void Clear();
    uint32_t Size() {return m_size;}
    char* Memory() {return m_ptr;}

private:
    bool m_delete_on_exit;
    int m_shmid;
    uint32_t m_key;
    char *m_ptr;
    uint32_t m_size;
};

class CSem
{
public:
    CSem(uint32_t key, bool del = false);
    ~CSem();
    void Wait();
    void Post();
    bool TryWait(struct timespec *ts = NULL);
    int GetValue();
    void SetValue(int val);
private:
    int m_semid;
    uint32_t m_key;
    bool m_delete_on_exit;
};

class CSemSet
{
};

#endif // _ROOT_MTX_TEST_IPC_H
