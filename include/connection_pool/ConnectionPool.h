/*
 * @Description: Connection pool
 * @Author: abellli
 * @Date: 2025-09-16
 * @LastEditTime: 2025-09-16
 */
#ifndef CONNECTION_POOL_CONNECT_POOL_H
#define CONNECTION_POOL_CONNECT_POOL_H

#include <string>
#include <queue>
#include <memory>
#include "mutex"
#include "functional"
#include "atomic"
#include "thread"
#include "condition_variable"

#include "Connection.h"
#include "ConfigManager.h"

class connection_pool : public std::enable_shared_from_this<connection_pool>
{
using PooledConnection = std::unique_ptr<connection, std::function<void(connection*)>>;
public:
    static std::shared_ptr<connection_pool> getconnect_pool();
    PooledConnection getconnection();
    ~connection_pool();

private:
    connection_pool(); // Singleton connection pool
    connection_pool(const connection_pool &) = delete;
    connection_pool &operator=(const connection_pool &) = delete;
    bool loadConfigFile(const std::string &filename = "");

    // Runs in a separate thread, specifically responsible for generating new connections
    void produceConnectionTask();

    // Start a new thread to scan for excess idle connections, idle connections that exceed maxIdleTime, and recycle excess connections
    void scanRunningConnectionTask();

    // Safely shutdown connection pool
    void shutdown();
    string _ip;
    unsigned short _port;
    string _username;
    string _password;
    string _dbname;
    int _initSize;          // connection pool initial size
    int _maxSize;           // connection pool max size
    int _maxIdleTime;       // connection max idle time
    int _connectionTimeout; // time out for obtaining connection

    std::queue<std::unique_ptr<connection>> _connectionQue; // queue to save connection
    std::mutex _queueMutex;
    std::atomic_int _connectionCnt; // number of active connection
    std::atomic<bool> _shutdown; // shutdown flag
    std::condition_variable cv;     // communication between producers and consumers
};

#endif // CONNECTION_POOL_CONNECT_POOL_H
