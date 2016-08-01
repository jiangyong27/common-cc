#include "http_server.h"


/** ************************HttpRequest start*******************************************/
/** HttpRequest构造函数*/
HttpRequest::~HttpRequest()
{
    evhttp_clear_headers(&m_params);
}

/** HttpRequest构造函数*/
HttpRequest::HttpRequest(http_handler_t *handler)
{
    m_handler = handler;
    char *decode = evhttp_decode_uri(handler->uri);
    m_uri = decode;
    delete decode;

    //获取 请求方法
    switch(evhttp_request_get_command(handler)){
        case EVHTTP_REQ_GET:m_method = "GET";break;
        case EVHTTP_REQ_POST:m_method = "POST";break;
        case EVHTTP_REQ_HEAD:m_method = "HEAD";break;
        case EVHTTP_REQ_PUT: m_method = "PUT"; break;
        case EVHTTP_REQ_DELETE: m_method = "DELETE"; break;
        case EVHTTP_REQ_OPTIONS: m_method = "OPTIONS"; break;
        case EVHTTP_REQ_TRACE: m_method = "TRACE"; break;
        case EVHTTP_REQ_CONNECT: m_method = "CONNECT"; break;
        case EVHTTP_REQ_PATCH: m_method = "PATCH"; break;
        default: m_method = "unknown"; break;
    }

    //获取头部
    struct evkeyval *header;
    struct evkeyvalq *headers;
    headers = evhttp_request_get_input_headers(handler);
    for (header = headers->tqh_first; header; header = header->next.tqe_next) {
        m_headers[header->key] = header->value;
    }

    //获取请求参数
    int ret = evhttp_parse_query(handler->uri,&m_params);
    if(ret!=0){
        cout <<"evhttp_parse_query error![uri="<<handler->uri<<"]";
    }

    //获取post参数
    m_post_data = "";
    if(m_method == "POST") {
        try{
            m_post_data = string((char *) EVBUFFER_DATA(handler->input_buffer), EVBUFFER_LENGTH(handler->input_buffer));
        }catch(...) {
            m_post_data = "";
            PRINT_ERROR("post data error");
        }

    }
}

/**获得HttpRequest请求参数*/
bool HttpRequest::GetHttpParam(const string& key,string *value)
{
    const char *tmp = evhttp_find_header(&m_params,key.c_str());
    if(tmp==NULL)
        return false;
    *value = string(tmp);
    return true;
}

/**获得头部文件*/
bool HttpRequest::GetHttpHeader(const string& key,string *value)
{
    if(m_headers.find(key)==m_headers.end())
        return false;
    *value = m_headers[key];
    return true;
}

/** ************************HttpResponse start*******************************************/
/**httpresponse构造函数*/
HttpResponse::HttpResponse(http_handler_t *handler)
{
    m_handler = handler;
}

/**增加头部信息*/
void HttpResponse::AddHeader(const string& key,const string& value)
{
    m_headers[key] = value;
}

/**发送回复信息*/
void HttpResponse::SendReplay(const string& content)
{
    struct evbuffer *buf;
    buf = evbuffer_new();
    if (buf == NULL)
        perror("failed to create response buffer");

    //添加返回头部
    for(map<string,string>::iterator it=m_headers.begin();it!=m_headers.end();it++) {
        evhttp_add_header(m_handler->output_headers,it->first.c_str(),it->second.c_str());
    }

    evbuffer_add_printf(buf, "%s",content.c_str());
    evhttp_send_reply(m_handler, HTTP_OK, "OK", buf);
    evbuffer_free(buf);

}

/** ************************HttpServer start*******************************************/
/**HttpServer构造函数*/
HttpServer::HttpServer(const string& addr,const uint16_t& port)
{
    m_httpd = NULL;
    m_isset_default_callback = false;
    SetAddr(addr,port);

    //只初始化一次
    event_init();
}

/**设置服务器端口地址*/
void HttpServer::SetAddr(const string& addr,const uint16_t& port)
{
    m_http_addr = addr;
    m_http_port = port;
}

/** 设置默认回调函数*/
void HttpServer::SetDefaultHttpCallback(const http_callback_t& fun)
{
    m_isset_default_callback = true;
    m_default_callback = fun;
}

/**设置用户回调函数*/
void HttpServer::AddHttpCallback(const string& uri,const http_callback_t& fun)
{
    m_callback_set[uri] = fun;
}


/**工作驱动函数*/
void* HttpServer::Work(void* param)
{
    HttpServer *httpserver = (HttpServer *) param;
    event_dispatch();
    evhttp_free(httpserver->m_httpd);

    return NULL;
}

/**启动httpserver服务*/
bool HttpServer::Start()
{
    //只能启动一次
    if(m_httpd){
        std::cout<<"HttpServer Init repeat!"<<endl;
        return false;
    }

    //检查IP端口是否设置
    if(m_http_addr.size()==0||m_http_port==0){
        std::cout <<"HttpServer SetAddr() before Start()"<<endl;
        return false;
    }

    //初始化libevent
    m_httpd = evhttp_start(m_http_addr.c_str(),m_http_port);
    if(!m_httpd){
        std::cout<<"evhttp_start failed![addr="<<m_http_addr<<":"<<m_http_port<<"]"<<endl;;
        return false;
    }

    //设置超时时间
    evhttp_set_timeout(m_httpd,30);

    //设置用户回调函数
    for(map<string,http_callback_t>::iterator it=m_callback_set.begin();it!=m_callback_set.end();it++){
        string uri = it->first;
        evhttp_set_cb(m_httpd,uri.c_str(),it->second,NULL);
    }

    //设置默认回调函数
    if(m_isset_default_callback)
        evhttp_set_gencb(m_httpd,m_default_callback,NULL);
    else
        evhttp_set_gencb(m_httpd,GenericHandler,NULL);

    //启动线程运行httpserver 启动驱动线程
    pthread_t pid;
    if(pthread_create(&pid,NULL,Work,this)!=0) {
        std::cout <<"pthread_create failed!"<<std::endl;
        return false;
    }

    //设置运行状态
    m_uptime = time(NULL);
    return true;
}


/**默认回调函数*/
void HttpServer::GenericHandler(http_handler_t *handler, void *arg)
{
    HttpRequest req(handler);
    HttpResponse res(handler);

    res.SendReplay("callback["+req.GetUri()+"] no set!\n");
}
