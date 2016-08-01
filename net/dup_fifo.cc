#include "dup_fifo.h"

CDupFifo::CDupFifo()
{
    if (pipe(m_pipe1)) {
        perror("pipe failed!");
        assert(0);
    }

    if (pipe(m_pipe2)) {
        perror("pipe failed!");
        assert(0);
    }
    SetNoBlock(m_pipe1[0]);
    SetNoBlock(m_pipe1[1]);
    SetNoBlock(m_pipe2[0]);
    SetNoBlock(m_pipe2[1]);
}

void CDupFifo::SetNoBlock(int fd)
{
    int opts;
    opts = fcntl(fd, F_GETFL);
    if(opts < 0) {
        perror("fcntl(F_GETFL)\n");
        assert(0);
    }
    opts = (opts | O_NONBLOCK);
    if(fcntl(fd, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)\n");
        assert(0);
    }
}

CDupFifo::~CDupFifo()
{
    CloseAll();
}

void CDupFifo::CloseAll()
{
    close(m_pipe1[0]);
    close(m_pipe1[1]);
    close(m_pipe2[0]);
    close(m_pipe2[1]);
}

void CDupFifo::CloseByParent()
{
    close(m_pipe1[0]);
    close(m_pipe2[1]);
    m_wfd = m_pipe1[1];
    m_rfd = m_pipe2[0];
}

void CDupFifo::CloseByChild()
{
    close(m_pipe1[1]);
    close(m_pipe2[0]);
    m_rfd = m_pipe1[0];
    m_wfd = m_pipe2[1];
}

int CDupFifo::GetSendFd()
{
    return m_wfd;
}

int CDupFifo::GetRecvFd()
{
    return m_rfd;
}

int CDupFifo::Send(char* buf, int len)
{
    return write(m_wfd, buf, len);
}

int CDupFifo::Recv(char* buf, int len)
{
    return read(m_rfd, buf, len);
}
