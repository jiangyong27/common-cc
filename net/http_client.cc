#include "http_client.h"

/** curl静态写回数据*/
static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
    if( NULL == str || NULL == buffer )
    {
        return -1;
    }
    char* pData = (char*)buffer;
    //str->clear();
    str->append(pData, size * nmemb);
    return nmemb;
}

/**初始化curl*/
CHttpClient::CHttpClient()
{
    m_curl = curl_easy_init();
    assert(m_curl!=NULL);
    m_timeout = 3;
}

/** */
CHttpClient::~CHttpClient()
{
    curl_easy_cleanup(m_curl);
}

/** 设置代理*/
void CHttpClient::SetProxy(const string& proxy)
{
    curl_easy_setopt(m_curl, CURLOPT_PROXY, proxy.c_str());
    curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
}

/** Get数据 url*/
bool CHttpClient::Get(const string& url,string* res)
{
    res->clear();
    curl_easy_setopt(m_curl,CURLOPT_URL,url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);

    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, (void *)res);

    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, m_timeout);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, m_timeout);
    m_retcode = curl_easy_perform(m_curl);
    if(m_retcode == 0)
        return true;
    return false;
}

/** Post请求*/
bool CHttpClient::Post(const string& url,const string& data,string* res)
{
    res->clear();
    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_POST, 1);
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, (void *)res);
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, m_timeout);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, m_timeout);
    m_retcode = curl_easy_perform(m_curl);
    if(m_retcode == 0)
        return true;
    return false;
}

