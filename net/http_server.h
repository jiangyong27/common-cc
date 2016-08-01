// Copyright (c) 2014, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2014-09-24
//
#ifndef _JASONYJINAG_LIB_NET_HTTP_H
#define _JASONYJINAG_LIB_NET_HTTP_H
#pragma once

#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <err.h>

#include <map>
#include <iostream>

#include <event.h>
#include <evhttp.h>

/**打印错误信息 */
#ifndef PRINT_ERROR
#define PRINT_ERROR(str)\
    cerr <<"[ERROR]["<<__FILE__<<"]["<<__LINE__<<"]["<<__FUNCTION__<<"()] ";\
    cerr << str <<endl;
#endif

/** 打印信息 */
#ifndef PRINT_INFO
#define PRINT_INFO(str)\
    cout <<"[INFO]["<<__FILE__<<"]["<<__LINE__<<"]["<<__FUNCTION__<<"()] ";\
    cout << str <<endl;
#endif

/** 删除指针*/
#ifndef _DELETE
#define _DELETE(x) if(x){delete x;x=NULL;}
#endif
#ifndef _DELETE_ARRAY
#define _DELETE_ARRAY(x) if(x){delete []x;x=NULL;}
#endif

using namespace std;

///回调函数定义 http_callback_t
typedef void (*http_callback_t)(struct evhttp_request *, void *);

///将evhttp_request 定义为handler
typedef struct evhttp_request http_handler_t;

/** ============================================HttpServer Start ======================================*/

/** HttpRequest */
class HttpRequest{
    public:
        HttpRequest(http_handler_t *handler);
        ~HttpRequest();
        bool GetHttpHeader(const string& key,string *value);
        bool GetHttpParam(const string& key,string *value);
        string GetUri(){return m_uri;}
        string GetMethod(){return m_method;}
        string GetPostData(){return m_post_data;}
    private:
        string m_uri;
        string m_method;
        string m_post_data;
        map<string,string> m_headers;
        struct evkeyvalq m_params;
        http_handler_t *m_handler;
};

/**HttpResponse */
class HttpResponse {
    public:
        HttpResponse(http_handler_t *handler);
        void AddHeader(const string& key,const string& value);
        void SendReplay(const string& content="");
    private:
        map<string,string> m_headers;
        http_handler_t *m_handler;
};

/** HttpServer类*/
class HttpServer
{
    public:
        /** 初始化httpserver */
        HttpServer(const string& addr,const uint16_t& port);
        ~HttpServer(){}

        /** 设置服务地址和端口*/
        void SetAddr(const string& addr,const uint16_t& port);

        /** 设置uri对应的回调函数*/
        void AddHttpCallback(const string& uri,const http_callback_t& fun);

        /** 设置默认回调函数*/
        void SetDefaultHttpCallback(const http_callback_t& fun);

        /** 启动httpserver服务*/
        bool Start();

        /**获取启动时间*/
        time_t GetUptime(){return m_uptime;}

        /**驱动线程*/
        static void* Work(void* param);

        /**默认回调函数*/
        static void GenericHandler(http_handler_t *handler, void *arg);

    private:
        ///启动时间
        time_t m_uptime;

        ///默认回调函数
        http_callback_t m_default_callback;

        ///是否设置默认回掉函数
        bool m_isset_default_callback;

        /// http地址
        string m_http_addr;

        ///http端口
        uint16_t m_http_port;

        /// libvent http hander
        struct evhttp *m_httpd;

        ///用户回调函数集合
        map<string,http_callback_t> m_callback_set;
};

/** ============================================HttpClient Start ======================================*/



#endif // _DATA_QC_TSHP_PLT_PLT_LIB_TX_NET_HTTP_H
