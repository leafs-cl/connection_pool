#include "ConnectionPool.h"
#include<Logger.hpp>

//Construct connection pool
connection_pool::connection_pool() : _connectionCnt(0){
    // Load config
    if(!loadConfigFile())
    {
        ERROR_LOG("Failed to load configuration file!");
        return;
    }
    // Create core connection
    // Similar as java thread pool, connection pool keeps core connection,
    // which will not be destoryed after use
    for(int i=0; i<_initSize; i++){
        // Create connection object using default constructor
        auto p = std::make_unique<connection>();
        p->connect(_ip, _port, _username, _password, _dbname);
        p->refreshsAliveTime();
        // Remind: After the original pointer managed by std::unique_ptr<connection> (obtained via .get()) is stored in _connectionQue, 
        // the std::unique_ptr<connection> object p is destructed at the end of the loop, and the connection object it manages is destroyed. 
        // This causes the pointers stored in _connectionQue to become dangling pointers, and subsequent use of these pointers will cause undefined behavior.
        // Thus, consider seperate ownership of queue and client, use unique_prt
        _connectionQue.push(std::move(p));
        _connectionCnt++;
    }

    // Start background connection producer thread(named as produce)
    // Use std::bind to make produce thread call produceConnectionTask, because produceConnectionTask
    // needs "this" pointer
    // So produce must pass "this"
    thread produce(std::bind(&connection_pool::produceConnectionTask, this));
    produce.detach();
    // Start background connection to collect thread
    thread scanner(std::bind(&connection_pool::scanRunningConnectionTask, this));
    scanner.detach();
};

//Lazy singleton connection pool
std::shared_ptr<connection_pool> connection_pool::getconnect_pool()
{
    static std::shared_ptr<connection_pool> pool(new connection_pool());
    return pool; // return copied shared ptr
};


bool connection_pool::loadConfigFile(const std::string& filename) {
    std::string configFile = filename.empty() ? "db_config.ini" : filename;
    
    try {
        auto configManager = createConfigManager(configFile);
        if (!configManager->loadConfig(configFile)) {
            ERROR_LOG("Failed to load config file: " + configFile);
            return false;
        }
        
        _ip = configManager->getString("ip", "localhost");
        _port = configManager->getInt("port", 3306);
        _username = configManager->getString("username", "root");
        _password = configManager->getString("password", "");
        _dbname = configManager->getString("dbname", "test");
        _initSize = configManager->getInt("initSize", 5);
        _maxSize = configManager->getInt("maxSize", 10);
        _maxIdleTime = configManager->getInt("maxIdleTime", 60);
        _connectionTimeout = configManager->getInt("connectionTimeOut", 100);
        
        INFO_LOG("Configuration loaded successfully from " + configFile);
        return true;
    } catch (const std::exception& e) {
        ERROR_LOG(std::string("Config loading error: ") + e.what());
        //Try loading mysql config file using default way
        // todo :
        //    Remind default way isn't responsible for loading result
        //    Parser will only work for `key=value` way
        FILE *pf = fopen(configFile.c_str(),"r");
        if(pf == nullptr)
        {
            ERROR_LOG("Ini file is not exit!");
            return false;
        }
        while(!feof(pf)) 
        {
            char line[1024] = {0};
            fgets(line,1023,pf);
            string str = line;
            int idx= str.find('=',0); 
            if(idx == -1)
            {
                continue;
            }
            int endidx = str.find('\n', idx); 
            string key = str.substr(0,idx); 
            string value = str.substr(idx+1, endidx -idx -1); 
            if (key == "ip")
            {
                _ip = value;
            }
            else if (key == "port")
            {
                _port = atoi(value.c_str()); 
            }
            else if (key == "username")
            {
                _username = value;
            }
            else if (key == "password")
            {
                _password = value;
            }
            else if (key == "dbname")
            {
                _dbname = value;
            }
            else if (key == "initSize")
            {
                _initSize = atoi(value.c_str());
            }
            else if (key == "maxSize")
            {
                _maxSize = atoi(value.c_str());
            }
            else if (key == "maxIdleTime")
            {
                _maxIdleTime = atoi(value.c_str());
            }
            else if (key == "connectionTimeOut")
            {
                _connectionTimeout = atoi(value.c_str());
            }
        }
        return true;
    }
}


// Running in independent thread
void connection_pool::produceConnectionTask(){
    for(;;)
    {   
        // _queueMutex is used to protect shared resouce
        // unique_lock automically lock "_queueMutex" and unlock to release resouce
        // unique lock is more heavy than lock guard, but allows manual lock and support condition
        unique_lock<mutex> lock(_queueMutex);
        // Check if existing idle connection, if having, don't need to produce, block producer
        // Otherwise, double check if empty and start producing
        while(!_connectionQue.empty()){ // to avoid Spurious Wakeup
            cv.wait(lock); // atomic unlock and block thread(depends on atomically operate blocking queue)
        }

        if(_connectionCnt < _maxSize){
            auto p = std::make_unique<connection>();
            p->connect( _ip,  _port, _username, _password, _dbname);
            p->refreshsAliveTime();
            _connectionQue.push(std::move(p));
            _connectionCnt++;
        }
        // Notify all consumers
        cv.notify_all(); // non-blocking, keep running
    }
}

// Expose to business, to obtain a free connection
connection_pool::PooledConnection connection_pool::getconnection()
{
    unique_lock<mutex> lock(_queueMutex); // Depends on cas and Mutex primitives
    while(_connectionQue.empty()) // All connections have been borrowed
    {
        if( cv_status::timeout == cv.wait_for(lock, chrono::microseconds (_connectionTimeout))){ // wait for notify
            if(_connectionQue.empty())
            {
                WARN_LOG("Obtain free connection failed!");
                throw std::runtime_error("No available connections!");
            }
        }
    }
    std::unique_ptr<connection> conn = std::move(_connectionQue.front());
    _connectionQue.pop();
    if (!conn->isValid()){
        WARN_LOG("Obtained invalid connection!");
        try {
            conn->reconnect(_ip,_port,_username,_password,_dbname);
            conn->refreshsAliveTime();
        } catch(const std::exception& e){
            _connectionCnt--;
            cv.notify_all();
        }
    }
    connection* rawConn = conn.release(); // release ownership and return original pointer
    // Must use shared from this(inner weak ptr)
    // Class needs to be managed by shared ptr
    // when shared ptr initialized, base class "weak and mutable member" will be set as "this" 
    std::weak_ptr<connection_pool> poolWeakPtr = this->shared_from_this();
    auto deleter = [poolWeakPtr](connection* p){ // use weak ptr instead of this
        // return pooled connection to pool
        // Need to check if connection_pool is alive
        if (auto poolPtr = poolWeakPtr.lock()){
            std::lock_guard<std::mutex> lock(poolPtr->_queueMutex);
            if (p == nullptr || !p->isValid()){
                delete p;
                poolPtr->_connectionCnt--;
            } else{
                p->refreshsAliveTime();
                poolPtr->_connectionQue.push(std::unique_ptr<connection>(p));
            }
            poolPtr->cv.notify_all();
        } else{
            delete p;
        }
    };
    cv.notify_all();
    return PooledConnection(rawConn, deleter);
}

// Collect connections whose idle time > threshold
void connection_pool::scanRunningConnectionTask() {
    for(;;) {
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));
        
        unique_lock<mutex> lock(_queueMutex);
        int invalidCount = 0;
        
        // Temporary Queue to store valid connections
        std::queue<std::unique_ptr<connection>> validConns;
        
        while (!_connectionQue.empty()) {
            auto conn = std::move(_connectionQue.front());
            _connectionQue.pop();
            
            // Check if valid is valid
            if (!conn->isValid()) {
                WARN_LOG("Discovered invalid connection, prepare to reconnect");
                if (!conn->reconnect(_ip, _port, _username, _password, _dbname)) {
                    // Reconnect failed, destory it in pool
                    invalidCount++;
                    _connectionCnt--;
                    continue;
                }
            }
            
            // Check if connection live time > maxIdleTime
            if (conn->getAliveTime() >= (_maxIdleTime * 1000) && _connectionCnt > _initSize) {
                INFO_LOG("Collect idle connection");
                _connectionCnt--;
                continue;
            }
            
            // Return to queue
            validConns.push(std::move(conn));
        }
        
        // Put valid connection to pool
        while (!validConns.empty()) {
            _connectionQue.push(std::move(validConns.front()));
            validConns.pop();
        }
        // If connection number < _initSize, call producer to supply
        if (_connectionCnt < _initSize) {
            cv.notify_all();
        }
        
        lock.unlock();
    }
}

void connection_pool::shutdown(){
    std::lock_guard<std::mutex> lock(_queueMutex);
    _shutdown.store(true);

    // Clear queue
    while(!_connectionQue.empty()){
        _connectionQue.pop();
    }
}

connection_pool::~connection_pool(){
    shutdown();
}