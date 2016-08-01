#ifndef _DATA_JASONYJIANG_COMMON_PROJ_C___UTILS_COMMON_API_H
#define _DATA_JASONYJIANG_COMMON_PROJ_C___UTILS_COMMON_API_H
#pragma once
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream>

uint32_t MyRandom();
std::string GetCurrentTime();
std::string GetHostIp();
#endif // _DATA_JASONYJIANG_COMMON_PROJ_C___UTILS_COMMON_API_H
