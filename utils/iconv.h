#ifndef _DATA_JASONYJIANG_COMMON_CPP_TEST_CHARSET_CONVERTER_H
#define _DATA_JASONYJIANG_COMMON_CPP_TEST_CHARSET_CONVERTER_H
#pragma once

#include <iconv.h>
#include <iostream>

class CIConv
{
public:
    CIConv(const std::string& from, const std::string& to);
    virtual ~CIConv();
    bool Convert(const std::string& str, std::string *res);
private:
    iconv_t m_cd;
};

class CUtf8ToGbk : public CIConv
{
public:
    CUtf8ToGbk() : CIConv("UTF-8", "GBK"){
    }
};

class CGbkToUtf8 : public CIConv
{
public:
    CGbkToUtf8() : CIConv("GBK", "UTF-8"){
    }
};

#endif // _DATA_JASONYJIANG_COMMON_CPP_TEST_CHARSET_CONVERTER_H
