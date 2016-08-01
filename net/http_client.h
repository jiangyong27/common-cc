// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2014-10-26
//

#ifndef _ROOT_MTX_NET_HTTPCLIENT_H
#define _ROOT_MTX_NET_HTTPCLIENT_H
#include <assert.h>
#include <stdint.h>
#include <iostream>
#include <curl/curl.h>
using namespace std;

enum{
    HTTP_METHOD_GET=1,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
};

/** HttpClient use curl*/
class CHttpClient
{
public:
    CHttpClient();
    ~CHttpClient();
    void SetProxy(const string& proxy);
    bool Get(const string& url,string* res);
    bool Post(const string& url,const string& data,string* res);
    void SetTimeout(uint32_t timeout) {m_timeout = timeout;}
    int GetRetcode() {return m_retcode;}
private:
    CURL *m_curl;
    int m_method;
    int m_timeout;
    int m_retcode;
};

#endif // _ROOT_MTX_NET_HTTPCLIENT_H
