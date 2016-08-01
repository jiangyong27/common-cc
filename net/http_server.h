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

/**��ӡ������Ϣ */
#ifndef PRINT_ERROR
#define PRINT_ERROR(str)\
    cerr <<"[ERROR]["<<__FILE__<<"]["<<__LINE__<<"]["<<__FUNCTION__<<"()] ";\
    cerr << str <<endl;
#endif

/** ��ӡ��Ϣ */
#ifndef PRINT_INFO
#define PRINT_INFO(str)\
    cout <<"[INFO]["<<__FILE__<<"]["<<__LINE__<<"]["<<__FUNCTION__<<"()] ";\
    cout << str <<endl;
#endif

/** ɾ��ָ��*/
#ifndef _DELETE
#define _DELETE(x) if(x){delete x;x=NULL;}
#endif
#ifndef _DELETE_ARRAY
#define _DELETE_ARRAY(x) if(x){delete []x;x=NULL;}
#endif

using namespace std;

///�ص��������� http_callback_t
typedef void (*http_callback_t)(struct evhttp_request *, void *);

///��evhttp_request ����Ϊhandler
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

/** HttpServer��*/
class HttpServer
{
    public:
        /** ��ʼ��httpserver */
        HttpServer(const string& addr,const uint16_t& port);
        ~HttpServer(){}

        /** ���÷����ַ�Ͷ˿�*/
        void SetAddr(const string& addr,const uint16_t& port);

        /** ����uri��Ӧ�Ļص�����*/
        void AddHttpCallback(const string& uri,const http_callback_t& fun);

        /** ����Ĭ�ϻص�����*/
        void SetDefaultHttpCallback(const http_callback_t& fun);

        /** ����httpserver����*/
        bool Start();

        /**��ȡ����ʱ��*/
        time_t GetUptime(){return m_uptime;}

        /**�����߳�*/
        static void* Work(void* param);

        /**Ĭ�ϻص�����*/
        static void GenericHandler(http_handler_t *handler, void *arg);

    private:
        ///����ʱ��
        time_t m_uptime;

        ///Ĭ�ϻص�����
        http_callback_t m_default_callback;

        ///�Ƿ�����Ĭ�ϻص�����
        bool m_isset_default_callback;

        /// http��ַ
        string m_http_addr;

        ///http�˿�
        uint16_t m_http_port;

        /// libvent http hander
        struct evhttp *m_httpd;

        ///�û��ص���������
        map<string,http_callback_t> m_callback_set;
};

/** ============================================HttpClient Start ======================================*/



#endif // _DATA_QC_TSHP_PLT_PLT_LIB_TX_NET_HTTP_H
