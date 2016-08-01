#include "http_message.h"
CHttpRequest::CHttpRequest()
{
    m_method = "GET";
    m_uri = "/";
}

void CHttpRequest::ParseHeader(const std::string& headers)
{
    std::string::size_type pos = 0;
    std::string::size_type pos1 = 0;
    std::string split;

    pos1 = headers.find("\r\n");
    if (pos1 == std::string::npos) {
        pos1 = headers.find("\n");
        split = "\n";
    } else {
        split = "\r\n";
    }

    std::string line;
    while(1) {
        if (pos1 == std::string::npos) {
            line = headers.substr(pos);
            std::string::size_type p = line.find(":");
            if (p != std::string::npos) {
                m_header[line.substr(0, p)] = line.substr(p+2);
            }
            break;
        } else {
            line = headers.substr(pos, pos1 - pos);
            std::string::size_type p = line.find(":");
            if (p != std::string::npos) {
                m_header[line.substr(0, p)] = line.substr(p+2);
            }
        }
        ///  toline
        pos = pos1 + split.size();
        pos1 = headers.find(split, pos);
    }
}

void CHttpRequest::ParseStartLine(const std::string& start_line)
{
    std::string::size_type pos;
    std::string::size_type pos1;

    /// method
    pos = start_line.find(" ");
    if (pos == std::string::npos) {
        return;
    }
    m_method.assign(start_line, 0, pos);

    /// uri
    pos1 = start_line.find(" ", pos+1);
    if (pos1 == std::string::npos) {
        return ;
    }
    m_uri.assign(start_line, pos+1, pos1-pos-1);

    pos = m_uri.find("?");
    if (pos != std::string::npos) {
        ParseParams(m_uri.substr(pos+1));
        m_uri.assign(m_uri.substr(0, pos));
    }
}

void CHttpRequest::ParseParams(const std::string& param)
{
    std::string::size_type pos1 = 0;
    std::string::size_type pos2 = 0;;
    std::string kv;
    pos2 = param.find("&");
    while (1) {
        if (pos2 == std::string::npos) {
            kv = param.substr(pos1);
        } else {
            kv = param.substr(pos1, pos2 - pos1);
        }

        std::string::size_type p = kv.find("=");
        if (p != std::string::npos) {
            m_params[kv.substr(0, p)] = kv.substr(p+1);
        }

        if (pos2 == std::string::npos) {
            break;
        }
        pos1 = pos2 + 1;
        pos2 = param.find("&", pos1);
    }
}
void CHttpRequest::GetParam(const std::string& key, int def, int *value)
{
    if (m_params.find(key) != m_params.end()) {
        *value = atoi(m_params[key].c_str());
        return ;
    }
    *value = def;
}

void CHttpRequest::GetParam(const std::string& key, const std::string& def, std::string *value)
{
    if (m_params.find(key) != m_params.end()) {
        value->assign(m_params[key]);
        return ;
    }
    value->assign(def);
}

void CHttpRequest::Parse(const std::string& content)
{
    m_header.clear();
    m_params.clear();
    std::string::size_type pos;
    std::string::size_type pos1;

    /// start_line
    pos = content.find("\r\n");
    if (pos == std::string::npos) {
        pos = content.find("\n");
        if (pos == std::string::npos) {
            return;
        }
    }
    std::string start_line = content.substr(0, pos);
    ParseStartLine(start_line);


    /// body
    pos1 = content.find("\r\n\r\n");
    if (pos1 != std::string::npos) {
        m_body.assign(content.substr(pos1 + 4));
    } else {
        pos1 = content.find("\n\n");
        if (pos == std::string::npos) {
            return;
        }
        m_body.assign(content.substr(pos1 + 2));
    }

    /// header
    pos = content.find("\r\n");
    if (pos == std::string::npos) {
        pos = content.find("\n");
        pos += 1;
    } else {
        pos += 2;
    }
    std::string header = content.substr(pos, pos1 - pos);
    ParseHeader(header);
}

std::string CHttpRequest::ToString()
{
    std::string result;
    result = m_method + " " + m_uri + " HTTP/1.1\r\n";
    for(std::map<std::string, std::string>::iterator it = m_header.begin();it != m_header.end();++it) {
        result += it->first + ": " + it->second + "\r\n";
    }
    result += "\r\n";
    result += m_body;
    return result;
}


/// ====================================================
CHttpResponse::CHttpResponse()
{
    m_ret_code = "200";
    m_ret_msg = "OK";
    m_http_version = "HTTP/1.1";
}

void CHttpResponse::ParseStartLine(const std::string& start_line)
{
    std::string::size_type pos;
    std::string::size_type pos1;

    /// version
    pos = start_line.find(" ");
    if (pos == std::string::npos) {
        return;
    }
    m_http_version.assign(start_line, 0, pos);

    /// retcode
    pos1 = start_line.find(" ", pos+1);
    if (pos1 == std::string::npos) {
        return ;
    }
    m_ret_code.assign(start_line, pos+1, pos1-pos-1);
    m_ret_msg = start_line.substr(pos1 + 1);
}

void CHttpResponse::ParseHeader(const std::string& headers)
{
    std::string::size_type pos = 0;
    std::string::size_type pos1 = 0;
    std::string split;

    pos1 = headers.find("\r\n");
    if (pos1 == std::string::npos) {
        pos1 = headers.find("\n");
        split = "\n";
    } else {
        split = "\r\n";
    }

    std::string line;
    while(1) {
        if (pos1 == std::string::npos) {
            line = headers.substr(pos);
            std::string::size_type p = line.find(":");
            m_header[line.substr(0, p)] = line.substr(p+2);
            break;
        } else {
            line = headers.substr(pos, pos1 - pos);
            std::string::size_type p = line.find(":");
            if (p != std::string::npos) {
                m_header[line.substr(0, p)] = line.substr(p+2);
            }
        }
        ///  toline
        pos = pos1 + split.size();
        pos1 = headers.find(split, pos);
    }
}

void CHttpResponse::Parse(const std::string& content)
{
    std::string::size_type pos;
    std::string::size_type pos1;

    /// start_line
    pos = content.find("\r\n");
    if (pos == std::string::npos) {
        pos = content.find("\n");
        if (pos == std::string::npos) {
            return;
        }
    }
    std::string start_line = content.substr(0, pos);
    ParseStartLine(start_line);


    /// body
    pos1 = content.find("\r\n\r\n");
    if (pos1 != std::string::npos) {
        m_body.assign(content.substr(pos1 + 4));
    } else {
        pos1 = content.find("\n\n");
        if (pos == std::string::npos) {
            return;
        }
        m_body.assign(content.substr(pos1 + 2));
    }

    /// header
    pos = content.find("\r\n");
    if (pos == std::string::npos) {
        pos = content.find("\n");
        pos += 1;
    } else {
        pos += 2;
    }
    std::string header = content.substr(pos, pos1 - pos);
    ParseHeader(header);
}

void CHttpResponse::SetBody(const std::string& body)
{
    m_body = body;

    std::ostringstream os;
    os << body.size();
    m_header["Content-Length"] = os.str();
}

std::string CHttpResponse::ToString()
{
    std::string result;
    result = m_http_version + " " + m_ret_code + " " + m_ret_msg + "\r\n";
    for(std::map<std::string, std::string>::iterator it = m_header.begin();it != m_header.end();++it) {
        result += it->first + ": " + it->second + "\r\n";
    }
    result += "\r\n";
    result += m_body;
    return result;
}
