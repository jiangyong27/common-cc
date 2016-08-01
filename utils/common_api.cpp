#include "common_api.h"
uint32_t MyRandom()
{
    struct timeval rand_timeval;
    gettimeofday(&rand_timeval, NULL);
    uint32_t srand_sed = rand_timeval.tv_sec * 1000 + rand_timeval.tv_usec/1000;
    srand(srand_sed);
    return (uint32_t)(rand());
}

std::string GetCurrentTime()
{
    std::string ret_time = "";
    time_t now;
    struct tm *tm_now;

    time(&now);
    tm_now = localtime(&now);

    char c_time[100] = {0};
    snprintf(c_time, sizeof(c_time), "%04d%02d%02d %02d:%02d:%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1,
             tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
    ret_time.assign(c_time, strlen(c_time));
    return ret_time;
}

std::string GetHostIp()
{
    std::string ret_ip = "0.0.0.0";
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct!=NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            std::string str_name = "";
            std::string str_ip = "";
            str_name.assign(ifAddrStruct->ifa_name, strlen(ifAddrStruct->ifa_name));
            str_ip.assign(addressBuffer, strlen(addressBuffer));
            if (str_name.find("eth1") != std::string::npos)
            {
                ret_ip = str_ip;
                break;
            }
        } else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        }
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
    return ret_ip;
}

