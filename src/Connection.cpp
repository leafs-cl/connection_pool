#include "Connection.h"
#include "Logger.hpp"
#include <mysql/mysql.h>
#include <string>
using namespace std;
connection::connection()
{
    _conn = mysql_init(nullptr);
}
connection::~connection()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
bool connection::connect(string ip, unsigned short port, string user, string password,
             string dbname)
{
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), port, nullptr, 0);
    return p != nullptr;
}
bool connection::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        WARN_LOG("Update failed:" + sql);
        return false;
    }
    return true;
}
MYSQL_RES* connection::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        WARN_LOG("Query failed:" + sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}

bool connection::reconnect(string ip, unsigned short port, 
                          string user, string password, string dbname) {
    
    if (_conn != nullptr) {
        mysql_close(_conn);
        _conn = nullptr;
    }
    
    _conn = mysql_init(nullptr);
    if (_conn == nullptr) {
        ERROR_LOG("MySQL initialization failed during reconnect");
        return false;
    }
    
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), user.c_str(),
                                 password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p == nullptr) {
        ERROR_LOG("MySQL reconnect failed: " + string(mysql_error(_conn)));
        return false;
    }
    
    INFO_LOG("MySQL connection reestablished successfully");
    return true;
}

bool connection::isValid(int timeout) {
    if (_conn == nullptr) {
        INFO_LOG("MySQL connection not initialized or has been destroyed!");
        return false;
    }
    
    // MYSQL driver provides ping method to test if connection is valid
    // USE COM_PING with low cost
    if (mysql_ping(_conn) == 0) {
        return true;
    }
    return false;
}

