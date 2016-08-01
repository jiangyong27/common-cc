// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2015-01-05
//

#include "pthread.h"

/** Mutex */
Mutex::Mutex()
{
	pthread_mutex_init(&m_mutex, NULL);
}


Mutex::~Mutex()
{
	pthread_mutex_destroy(&m_mutex);
}


void Mutex::Lock() const
{
	pthread_mutex_lock(&m_mutex);
}


void Mutex::Unlock() const
{
	pthread_mutex_unlock(&m_mutex);
}


/** Semaphore */
Semaphore::Semaphore(value_t start_val)
{
	sem_init(&m_sem, 0, start_val);
}


Semaphore::~Semaphore()
{
	sem_destroy(&m_sem);
}


int Semaphore::Post()
{
	return sem_post(&m_sem);
}


int Semaphore::Wait()
{
	return sem_wait(&m_sem);
}


int Semaphore::TryWait()
{
	return sem_trywait(&m_sem);
}


int Semaphore::GetValue()
{
    int value;
	if(sem_getvalue(&m_sem, &value)==0)
        return value;
    return -1;
}


Thread::Thread()
{
    m_core = -1;
}
void Thread::Start()
{
    if (0 != pthread_create(&m_pid, NULL, Work, this)) {
        throw std::string("pthread create failed!");
    }
}

void Thread::Join()
{
    pthread_join(m_pid, NULL);
}

void Thread::Bind(int cpu_core)
{
    m_core = cpu_core % sysconf(_SC_NPROCESSORS_CONF);

}

void *Thread::Work(void *param)
{
    Thread *self = (Thread*)(param);

    // 绑定cpu核运行
    if (self->m_core >= 0) {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(self->m_core, &mask);
        if (pthread_setaffinity_np(self->m_pid, sizeof(mask), &mask) < 0) {
            throw std::string("pthread_setaffinity_np failed!");
        }
        printf("pthread[%u] bind cpu[%d]\n", (uint32_t)self->m_pid, self->m_core);
    }

    self->Run();
    return NULL;
}

