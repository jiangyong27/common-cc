#include "database.h"

int CRecord::GetInt(int field_id)
{
    if (row[field_id] != NULL)
        return atoi(row[field_id]);
    else
        return 0;
}

string CRecord::GetStr(int field_id)
{
    if (row[field_id] != NULL) {
        return row[field_id];
    }
    else {
        return string("");
    }
}

CDatabase::CDatabase()
{
    m_server = NULL;
    m_handle = NULL;
    m_result = NULL;
    m_num_rows = 0;
    Init();
}

CDatabase::~CDatabase()
{
    if (m_result != NULL)
    {
        mysql_free_result(m_result);
    }

    if (m_handle != NULL)
    {
        mysql_close(m_handle);
    }
    if (m_server) {
        delete m_server;
        m_server = NULL;
    }
}

void CDatabase::Init()
{
    m_server = new Server();
    m_handle = mysql_init(NULL);
    int timeout = DB_READ_TIMEOUT;
    char value = 1;
    mysql_options(m_handle, MYSQL_OPT_READ_TIMEOUT, (const char *)&timeout);
    mysql_options(m_handle, MYSQL_OPT_RECONNECT, &value);
}

void CDatabase::Ping()
{
    mysql_ping(m_handle);
}

bool CDatabase::ReConnect()
{
    if (m_handle != NULL) {
        mysql_close(m_handle);
    }

    m_handle = mysql_init(NULL);
    return Connect(m_server);
}

bool CDatabase::Connect(Server *server)
{
    *m_server = *server;
    if (mysql_real_connect(m_handle, m_server->host.c_str(), m_server->user.c_str(),
            m_server->passwd.c_str(), m_server->db.c_str(), m_server->port, NULL,
            0) != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CDatabase::Connect(string host, int port, string user, string passwd, string db)
{
    Server server;
    server.host = host;
    server.port = port;
    server.user = user;
    server.passwd = passwd;
    server.db = db;

    return Connect(&server);
}

bool CDatabase::Update(const std::string& table, const std::map<std::string, std::string>& dat, const std::string& where)
{
    std::string sql = "UPDATE " + table + " SET ";
    std::string key;
    std::string value;
    for (std::map<std::string, std::string>::const_iterator iter = dat.begin(); iter != dat.end(); ++iter) {
        value = iter->second;
        if (iter != dat.begin()) {
            sql += ",";
        }

        if (value == "NULL" || value == "null") {
            sql += iter->first + "=NULL";
        } else {
            sql += iter->first + "='" + value +"'";
        }
    }

    if (where.size() != 0) {
        sql += " WHERE " + where;
    }
    return DoQuery(sql);
}

bool CDatabase::Insert(const std::string& table, const std::map<std::string, std::string>& dat)
{
    std::string sql = "INSERT INTO " + table + " (";
    for (std::map<std::string, std::string>::const_iterator iter = dat.begin(); iter != dat.end(); ++iter) {
        if (iter == dat.begin()) {
            sql += iter->first;
        } else {
            sql += "," + iter->first;
        }
    }

    std::string value;
    sql += ") VALUES (";
    for (std::map<std::string, std::string>::const_iterator iter = dat.begin(); iter != dat.end(); ++iter) {
        value = iter->second;
        if (iter == dat.begin()) {
            if (value == "NULL" || value == "null")
                sql += "NULL";
            else
                sql += "'" + value + "'";
        } else {
            if (value == "NULL" || value == "null")
                sql += ",NULL";
            else
                sql += ",'" + value + "'";
        }
    }
    sql += ")";
    //printf("sql=%s\n", sql.c_str());
    return DoQuery(sql);
}


bool CDatabase::DoQuery(string sql)
{
    m_last_sql.assign(sql);
    // 执行查询语句
    if (mysql_query(m_handle, sql.c_str()) != 0)
    {
        m_error = mysql_error(m_handle);
        return false;
    }

    // 保存查询结果
    if (m_result != NULL)
    {
        mysql_free_result(m_result);
    }

    m_result = mysql_store_result(m_handle);

    if (m_result)
    {
        return true;
    }
    else
    {
        // 如果查询结果为空, 先判断本次查询是否应该有结果
        if (mysql_field_count(m_handle) == 0)
        {
            // 如果是update等本就没有查询结果的语句, 记录有做更改的行数
            m_num_rows = mysql_affected_rows(m_handle);
            return true;
        }
        else
        {
            m_error = mysql_error(m_handle);
            return false;
        }
    }
}

int CDatabase::GetLastId()
{
    return (int) mysql_insert_id(m_handle);
}
int CDatabase::GetResultNum()
{
   return (int) mysql_num_rows(m_result);
}

bool CDatabase::GetRecord(CRecord *record)
{
    record->row = mysql_fetch_row(m_result);

    if (record->row == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

std::string CDatabase::GetLastSql() const
{
    return m_last_sql;
}
std::string CDatabase::GetError() const
{
    return m_error;
}

