#ifndef _HOME_UBUNTU_WXP_SRC_NET_HTTP_PARSER_H
#define _HOME_UBUNTU_WXP_SRC_NET_HTTP_PARSER_H
#pragma once
#include <iostream>
#include <cstring>
#include <sstream>
#include <map>
#include <stdint.h>
#include <stdlib.h>

class CHttpRequest
{
public:
    CHttpRequest();
    void Parse(const std::string& content);
    std::string ToString();

    std::string GetHeaer(const std::string& key) {return m_header[key];}
    std::string GetUri() {return m_uri;}
    std::string GetMethod() {return m_method;}
    std::string GetBody() {return m_body;}
    void GetParam(const std::string& key, const std::string& def, std::string *value);
    void GetParam(const std::string& key, int def, int *value);

    void SetUri(const std::string& uri) {m_uri = uri;}
    void SetMethod(const std::string& method) {m_method = method;}
    void SetHeader(const std::string& key, const std::string& value) {m_header[key] = value;}
    void SetBody(const std::string& body) {m_body = body;}
    void ParseParams(const std::string& param);

private:
    void ParseHeader(const std::string& headers);
    void ParseStartLine(const std::string& start_line);

private:
    std::map<std::string, std::string> m_header;
    std::map<std::string, std::string> m_params;
    std::string m_method;
    std::string m_uri;
    std::string m_body;
};

class CHttpResponse
{
public:
    CHttpResponse();
    void Parse(const std::string& content);
    std::string GetHeaer(const std::string& key) {return m_header[key];}
    std::string GetBody() {return m_body;}
    void SetHeader(const std::string& key, const std::string& value) {m_header[key] = value;}
    void SetBody(const std::string& body);
    void SetHttpVersion(const std::string& http_version) { m_http_version = http_version;}
    void SetRetCode(const std::string& ret_code, const std::string& ret_msg) {
        m_ret_code = ret_code;
        m_ret_msg = ret_msg;
    }

    std::string ToString();
    void ParseHeader(const std::string& headers);
    void ParseStartLine(const std::string& start_line);
private:
    std::string m_ret_code;
    std::string m_ret_msg;
    std::string m_http_version;
    std::map<std::string, std::string> m_header;
    std::string m_body;
};

#endif // _HOME_UBUNTU_WXP_SRC_NET_HTTP_PARSER_H
