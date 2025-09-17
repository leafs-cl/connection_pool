# ConnectionPool

## 概述

ConnectionPool 是一个基于 C++17 开发的高性能 MySQL 数据库连接池。它通过复用数据库连接，显著减少频繁创建和销毁连接带来的性能开销，提升应用程序的数据库访问效率和系统整体吞吐量。

### 特性

•   连接复用: 有效管理连接生命周期，降低资源消耗。

•   线程安全: 使用互斥锁 (std::mutex) 和条件变量 (std::condition_variable) 实现多线程安全访问。

•   灵活配置: 通过外部 INI 文件轻松配置数据库连接参数和连接池行为。

•   自动回收: 支持空闲连接超时自动回收，防止资源泄漏。

•   优雅上下线: 提供连接的平滑初始化和清理。

•   跨平台: 基于 CMake 构建，支持 Linux/Windows/macOS。

### 技术栈

•   编程语言: C++17

•   数据库: MySQL (官方 libmysqlclient 或 mariadb-connector-c)

•   构建工具: CMake (最低版本 3.10)

•   配置文件: INI 格式 (使用 inih 或其他解析库)

### 项目结构


ConnectionPool/
├── CMakeLists.txt              # 项目根 CMake 配置文件
├── build/                      # 编译构建目录（通常为空，用于 out-of-source 构建）
├── doc/
│   └── README.md               # 项目说明文档
├── include/                    # 公共头文件目录
│   └── connection_pool/       # 库的头文件
│       ├── Config.h           # 配置解析类
│       ├── Connection.h       # 数据库连接封装类
│       ├── ConnectionPool.h   # 连接池核心类
│       └── Logger.hpp         # 日志工具类（可选）
├── resources/                  # 资源文件
│   ├── config.ini.example     # 配置文件示例
│   └── schema.sql             # 示例数据库表结构
├── scripts/                    # 实用脚本
│   ├── build.sh               # 快速构建脚本
│   └── setup_db.sh            # 初始化测试数据库脚本
├── src/                       # 源代码目录
│   ├── CMakeLists.txt         # 源文件的 CMake 配置
│   ├── Connection.cpp         # Connection 类实现
│   ├── ConnectionPool.cpp     # ConnectionPool 类实现
│   └── Config.cpp             # Config 类实现
├── tests/                      # 测试目录
│   ├── CMakeLists.txt         # 测试的 CMake 配置
│   ├── BasicTest.cpp          # 基础功能测试
│   ├── ConcurrentTest.cpp     # 并发压力测试
│   └── AsyncLoggerTest.cpp    # 日志测试（可选）
└── third_party/               # 第三方依赖（可选，如 inih）
    └── CMakeLists.txt


### 安装与构建

#### 前提条件

1.  编译器: 支持 C++17 的编译器 (GCC 7+, Clang 5+, MSVC 2019+)
2.  MySQL 开发库: 在 Ubuntu/Debian 上安装 libmysqlclient-dev，在 CentOS/RHEL 上安装 mysql-devel，或在 macOS 上使用 brew install mysql-client。
3.  CMake: 版本 3.10 或更高。

#### 编译步骤

1.  克隆项目:
    git clone git@github.com:leafs-cl/connection_pool.git
    cd ConnectionPool
    

2.  配置编译环境:
    # 创建一个构建目录并进入
    mkdir build && cd build
    # 运行 CMake 生成 Makefile（或其他构建系统文件）
    cmake ..
    
    如需指定安装路径或开启测试，可使用：
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_TESTING=ON
    

3.  编译项目(任选以下其中一种方式即可):
    - 1. make -j$(nproc) # Linux/macOS 使用多核编译
    - 2. cmake --build .
    

4.  （可选）安装到系统:
    sudo make install
    

5.  （可选）运行测试 (确保已配置好测试数据库，然后任选以下任意一种方式即可):
   - make test
   - ./tests/ConcurrentTest
    

### 使用方法

1. 配置文件
在项目根目录或运行目录创建 config.ini（可参考 resources/config.ini.example）：
[mysql]
host=127.0.0.1
port=3306
user=your_username
password=your_password
database=your_database_name
charset=utf8mb4

[connection_pool]
init_size=10
max_size=20
max_idle_time=60
connection_timeout=3


2. 在代码中使用

以下是一个简单的使用示例：
#include "connection_pool/ConnectionPool.h"
#include <iostream>

int main() {
    // 1. 获取连接池实例（单例模式）
    ConnectionPool* pool = ConnectionPool::getInstance();
    // 2. 加载配置文件，初始化连接池
    pool->init("/path/to/your/config.ini");

    // 3. 从连接池获取一个连接
    auto conn = pool->getConnection();
    if (conn) {
        // 4. 使用连接执行查询
        bool success = conn->execute("UPDATE users SET last_login = NOW() WHERE id = 1;");
        if (success) {
            std::cout << "Query executed successfully." << std::endl;
        } else {
            std::cerr << "Query failed: " << conn->getError() << std::endl;
        }

        // 5. (可选) 当 `conn` 离开作用域时，其析构函数会自动将连接归还给连接池。
        // 你也可以手动释放：pool->releaseConnection(conn);
    } else {
        std::cerr << "Failed to get connection from pool." << std::endl;
    }

    // 6. 程序结束时，连接池会自动清理所有连接
    return 0;
}


3. 链接到你的项目

在你的项目的 CMakeLists.txt 中：
find_package(ConnectionPool REQUIRED) # 如果已安装
或者使用 add_subdirectory
add_subdirectory(path/to/ConnectionPool)
target_link_libraries(your_target PRIVATE ConnectionPool::ConnectionPool)


4. 测试与性能优化

•   压力测试: 使用 tests/ConcurrentTest.cpp 进行多线程并发测试，模拟高负载场景。

•   性能监控: 关注指标如连接获取平均时间、每秒查询率 (QPS)、连接池使用率。

•   优化建议:

    ◦   根据实际负载调整 init_size 和 max_size，避免过大或过小。

    ◦   设置合理的 max_idle_time 以定期清理空闲连接。

    ◦   确保 connection_timeout 设置合理，避免线程长时间阻塞。

## 如何贡献

我们欢迎任何形式的贡献！
1.  Fork 本仓库。
2.  创建特性分支 (git checkout -b feature/AmazingFeature)。
3.  提交更改 (git commit -m 'Add some AmazingFeature')。
4.  推送到分支 (git push origin feature/AmazingFeature)。
5.  发起 Pull Request。

## License
本项目采用 MIT License。详情请参阅 LICENSE 文件。

## 获取帮助
如有问题或建议，可以通过以下方式联系：
•   提交 [https://github.com/leafs-cl/connection_pool.git/issues]。

•   发送邮件至: [lic@mail.nwpu.edu.cn]。
