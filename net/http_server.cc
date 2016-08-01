#include "http_server.h"


/** ************************HttpRequest start*******************************************/
/** HttpRequest���캯��*/
HttpRequest::~HttpRequest()
{
    evhttp_clear_headers(&m_params);
}

/** HttpRequest���캯��*/
HttpRequest::HttpRequest(http_handler_t *handler)
{
    m_handler = handler;
    char *decode = evhttp_decode_uri(handler->uri);
    m_uri = decode;
    delete decode;

    //��ȡ ���󷽷�
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

    //��ȡͷ��
    struct evkeyval *header;
    struct evkeyvalq *headers;
    headers = evhttp_request_get_input_headers(handler);
    for (header = headers->tqh_first; header; header = header->next.tqe_next) {
        m_headers[header->key] = header->value;
    }

    //��ȡ�������
    int ret = evhttp_parse_query(handler->uri,&m_params);
    if(ret!=0){
        cout <<"evhttp_parse_query error![uri="<<handler->uri<<"]";
    }

    //��ȡpost����
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

/**���HttpRequest�������*/
bool HttpRequest::GetHttpParam(const string& key,string *value)
{
    const char *tmp = evhttp_find_header(&m_params,key.c_str());
    if(tmp==NULL)
        return false;
    *value = string(tmp);
    return true;
}

/**���ͷ���ļ�*/
bool HttpRequest::GetHttpHeader(const string& key,string *value)
{
    if(m_headers.find(key)==m_headers.end())
        return false;
    *value = m_headers[key];
    return true;
}

/** ************************HttpResponse start*******************************************/
/**httpresponse���캯��*/
HttpResponse::HttpResponse(http_handler_t *handler)
{
    m_handler = handler;
}

/**����ͷ����Ϣ*/
void HttpResponse::AddHeader(const string& key,const string& value)
{
    m_headers[key] = value;
}

/**���ͻظ���Ϣ*/
void HttpResponse::SendReplay(const string& content)
{
    struct evbuffer *buf;
    buf = evbuffer_new();
    if (buf == NULL)
        perror("failed to create response buffer");

    //��ӷ���ͷ��
    for(map<string,string>::iterator it=m_headers.begin();it!=m_headers.end();it++) {
        evhttp_add_header(m_handler->output_headers,it->first.c_str(),it->second.c_str());
    }

    evbuffer_add_printf(buf, "%s",content.c_str());
    evhttp_send_reply(m_handler, HTTP_OK, "OK", buf);
    evbuffer_free(buf);

}

/** ************************HttpServer start*******************************************/
/**HttpServer���캯��*/
HttpServer::HttpServer(const string& addr,const uint16_t& port)
{
    m_httpd = NULL;
    m_isset_default_callback = false;
    SetAddr(addr,port);

    //ֻ��ʼ��һ��
    event_init();
}

/**���÷������˿ڵ�ַ*/
void HttpServer::SetAddr(const string& addr,const uint16_t& port)
{
    m_http_addr = addr;
    m_http_port = port;
}

/** ����Ĭ�ϻص�����*/
void HttpServer::SetDefaultHttpCallback(const http_callback_t& fun)
{
    m_isset_default_callback = true;
    m_default_callback = fun;
}

/**�����û��ص�����*/
void HttpServer::AddHttpCallback(const string& uri,const http_callback_t& fun)
{
    m_callback_set[uri] = fun;
}


/**������������*/
void* HttpServer::Work(void* param)
{
    HttpServer *httpserver = (HttpServer *) param;
    event_dispatch();
    evhttp_free(httpserver->m_httpd);

    return NULL;
}

/**����httpserver����*/
bool HttpServer::Start()
{
    //ֻ������һ��
    if(m_httpd){
        std::cout<<"HttpServer Init repeat!"<<endl;
        return false;
    }

    //���IP�˿��Ƿ�����
    if(m_http_addr.size()==0||m_http_port==0){
        std::cout <<"HttpServer SetAddr() before Start()"<<endl;
        return false;
    }

    //��ʼ��libevent
    m_httpd = evhttp_start(m_http_addr.c_str(),m_http_port);
    if(!m_httpd){
        std::cout<<"evhttp_start failed![addr="<<m_http_addr<<":"<<m_http_port<<"]"<<endl;;
        return false;
    }

    //���ó�ʱʱ��
    evhttp_set_timeout(m_httpd,30);

    //�����û��ص�����
    for(map<string,http_callback_t>::iterator it=m_callback_set.begin();it!=m_callback_set.end();it++){
        string uri = it->first;
        evhttp_set_cb(m_httpd,uri.c_str(),it->second,NULL);
    }

    //����Ĭ�ϻص�����
    if(m_isset_default_callback)
        evhttp_set_gencb(m_httpd,m_default_callback,NULL);
    else
        evhttp_set_gencb(m_httpd,GenericHandler,NULL);

    //�����߳�����httpserver ���������߳�
    pthread_t pid;
    if(pthread_create(&pid,NULL,Work,this)!=0) {
        std::cout <<"pthread_create failed!"<<std::endl;
        return false;
    }

    //��������״̬
    m_uptime = time(NULL);
    return true;
}


/**Ĭ�ϻص�����*/
void HttpServer::GenericHandler(http_handler_t *handler, void *arg)
{
    HttpRequest req(handler);
    HttpResponse res(handler);

    res.SendReplay("callback["+req.GetUri()+"] no set!\n");
}
