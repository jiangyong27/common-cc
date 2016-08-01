#ifndef _DATA_MTX_PUBLIC_UTILS_URL_H
#define _DATA_MTX_PUBLIC_UTILS_URL_H
#pragma once
#include <ctype.h>
#include <assert.h>
#include <iostream>

class CUrl
{
public:
    static std::string Decode(const std::string& str);
    static std::string Encode(const std::string& str);

};

#endif // _DATA_MTX_PUBLIC_UTILS_URL_H
