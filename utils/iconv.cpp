
#include "iconv.h"
CIConv::CIConv(const std::string& from, const std::string& to)
{
    m_cd = iconv_open(to.c_str(), from.c_str());
}

CIConv::~CIConv()
{
    iconv_close(m_cd);
}

bool CIConv::Convert(const std::string& str, std::string *res)
{
    res->clear();

    size_t in_len = str.size() + 1;
    size_t out_len = in_len * 2;
    size_t out_buffer_len = out_len;

    char *tmp = new char[out_len];
    char *in = const_cast<char*>(str.c_str());
    char *out = tmp;

    size_t retBytes;
    int saveErrorNo;

    retBytes = iconv(m_cd, &in, &in_len, &out, &out_len);
    saveErrorNo = errno;

    if (out != tmp) {
        res->append(tmp, out - tmp);
    }

    if (retBytes != (size_t)-1) {
        out = tmp;
        out_len = out_buffer_len;
        iconv(m_cd, NULL, NULL, &out, &out_len);
        if (out != tmp) {
            res->append(tmp, out - tmp);
        }
        saveErrorNo = 0;
    }

    bool bRet = false;
    switch (saveErrorNo) {
    case E2BIG:
        bRet = false;
        break;

    case EILSEQ:
        bRet = false;
        break;

    case EINVAL:
        bRet = false;
        break;

    default :
        bRet = true;
    }

    errno = saveErrorNo;
    delete [] tmp;
    return bRet;
}
