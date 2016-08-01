#ifndef _DATA_JASONYJIANG_WXP_SRC_FIFO_H
#define _DATA_JASONYJIANG_WXP_SRC_FIFO_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>


class CDupFifo
{
public:
    ~CDupFifo();
    CDupFifo();
    void CloseAll();
    void CloseByParent();
    void CloseByChild();
    int Send(char* buf, int len);
    int Recv(char* buf, int len);
    int GetRecvFd();
    int GetSendFd();
private:
    void SetNoBlock(int fd);
private:
    int m_rfd;
    int m_wfd;
    int m_pipe1[2];
    int m_pipe2[2];
};

#endif // _DATA_JASONYJIANG_WXP_SRC_FIFO_H
