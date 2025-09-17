/*
* @Description: MYSQL connection manager
* @Author: abellli
* @Date: 2025-09-16
* @LastEditTime: 2025-09-16
*/
#ifndef CONNECTION_POOL_CONNECTION_H
#define CONNECTION_POOL_CONNECTION_H
#include "iostream"
#include <mysql/mysql.h>
#include "Logger.hpp"
using namespace std;

class connection
{
public:
    connection();
    ~connection();
    bool connect(string ip,
                 unsigned short port,
                 string user, string password,
                 string dbname);
    bool reconnect(string ip, unsigned short port, 
                  string user, string password, string dbname);
    void refreshsAliveTime(){ _alivetime = clock();}
    clock_t  getAliveTime() const {return clock() - _alivetime;}
    bool isValid(int timeout=30);
    bool update(string sql);
    MYSQL_RES* query(string sql);
    
private:
    MYSQL* _conn; // MYSQL connection
    clock_t _alivetime; // Alive time
};

#endif //CONNECTION_POOL_CONNECTION_H
