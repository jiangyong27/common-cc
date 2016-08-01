/*
 * author: tonghuang
 * desc: mysql database helper
 */
#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>

#include <map>
#include <string>
#include <vector>

//数据库读取超时时间
#define DB_READ_TIMEOUT  (60 * 3)


using std::string;

struct Server
{
    string host;
    int port;
    string user;
    string passwd;
    string db;
};

class CRecord
{
public:
    int GetInt(int field_id);
    string GetStr(int field_id);

    MYSQL_ROW row;
};

class CDatabase
{
public:
    CDatabase();
    ~CDatabase();

    bool Connect(Server *server);
    bool ReConnect();
    bool Connect(string host, int port, string user, string passwd, string db);
    bool GetRecord(CRecord *record);

    bool DoQuery(string sql);
    bool Update(const std::string& table, const std::map<std::string, std::string>& dat, const std::string& where="");
    bool Insert(const std::string& table, const std::map<std::string, std::string>& dat);

    void Ping();
    int GetResultNum();
    int GetLastId();
    std::string GetError() const;
    std::string GetLastSql() const;

private:
    void Init();

    Server *m_server;
    MYSQL *m_handle;
    MYSQL_RES *m_result;
    int m_num_rows;
    string m_error;
    std::string m_last_sql;
};

#endif

