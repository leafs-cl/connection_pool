# ConnectionPool

## Overview

ConnectionPool is a high-performance MySQL database connection pool developed based on C++17. It significantly reduces the performance overhead associated with frequent creation and destruction of database connections by reusing them, thereby enhancing the database access efficiency of applications and the overall system throughput.

### Features

*   **Connection Reuse:** Efficiently manages the connection lifecycle, reducing resource consumption.
*   **Thread Safety:** Utilizes mutexes (`std::mutex`) and condition variables (`std::condition_variable`) to ensure safe access in multi-threaded environments.
*   **Flexible Configuration:** Easily configure database connection parameters and connection pool behavior through an external INI file.
*   **Automatic Recycling:** Supports automatic recycling of idle connections that have timed out, preventing resource leaks.
*   **Graceful Initialization and Cleanup:** Provides smooth initialization and cleanup of connections.
*   **Cross-Platform:** Built with CMake, supporting Linux, Windows, and macOS.

### Technology Stack

*   **Programming Language:** C++17
*   **Database:** MySQL (Official `libmysqlclient` or `mariadb-connector-c`)
*   **Build Tool:** CMake (minimum version 3.10)
*   **Configuration File:** INI format (using `inih` or other parsing libraries)

### Project Structure

```
ConnectionPool/
├── CMakeLists.txt              # Root CMake configuration file
├── build/                      # Build directory (typically empty, for out-of-source builds)
├── doc/
│   └── README.md               # Project documentation
├── include/                    # Public header directory
│   └── connection_pool/        # Library headers
│       ├── Config.h           # Configuration parsing class
│       ├── Connection.h       # Database connection wrapper class
│       ├── ConnectionPool.h   # Core connection pool class
│       └── Logger.hpp         # Logging utility class (optional)
├── resources/                  # Resource files
│   ├── config.ini.example     # Example configuration file
│   └── schema.sql             # Example database schema
├── scripts/                    # Utility scripts
│   ├── build.sh               # Quick build script
│   └── setup_db.sh            # Script to initialize test database
├── src/                       # Source code directory
│   ├── CMakeLists.txt         # CMake configuration for source files
│   ├── Connection.cpp         # Implementation of the Connection class
│   ├── ConnectionPool.cpp     # Implementation of the ConnectionPool class
│   └── Config.cpp             # Implementation of the Config class
├── tests/                     # Test directory
│   ├── CMakeLists.txt         # CMake configuration for tests
│   ├── BasicTest.cpp          # Basic functionality tests
│   ├── ConcurrentTest.cpp     # Concurrent stress tests
│   └── AsyncLoggerTest.cpp    # Logging tests (optional)
└── third_party/               # Third-party dependencies (optional, e.g., inih)
    └── CMakeLists.txt
```

### Installation and Building

#### Prerequisites

1.  **Compiler:** A compiler supporting C++17 (GCC 7+, Clang 5+, MSVC 2019+).
2.  **MySQL Development Libraries:** Install `libmysqlclient-dev` on Ubuntu/Debian, `mysql-devel` on CentOS/RHEL, or use `brew install mysql-client` on macOS.
3.  **CMake:** Version 3.10 or higher.

#### Build Steps

1.  **Clone the project:**
    ```bash
    git clone <your-repo-url>
    cd ConnectionPool
    ```

2.  **Configure the build environment:**
    ```bash
    # Create a build directory and navigate into it
    mkdir build && cd build
    # Run CMake to generate Makefiles (or files for other build systems)
    cmake ..
    ```
    To specify an installation path or enable tests, use:
    ```bash
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_TESTING=ON
    ```

3.  **Compile the project (choose one of the following methods):**
    *   `make -j$(nproc)` # Use multiple cores for compilation on Linux/macOS
    *   `cmake --build .`

4.  **(Optional) Install to the system:**
    ```bash
    sudo make install
    ```

5.  **(Optional) Run tests (ensure the test database is configured, then choose one of the following methods):**
    *   `make test`
    *   `./tests/ConcurrentTest`

### Usage

1.  **Configuration File**
    Create a `config.ini` in the project root or runtime directory (refer to `resources/config.ini.example`):
    ```ini
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
    ```

2.  **Using in Code**
    Here is a simple usage example:
    ```cpp
    #include "connection_pool/ConnectionPool.h"
    #include <iostream>

    int main() {
        // 1. Get the connection pool instance (Singleton pattern)
        ConnectionPool* pool = ConnectionPool::getInstance();
        // 2. Load the configuration file and initialize the connection pool
        pool->init("/path/to/your/config.ini");

        // 3. Get a connection from the pool
        auto conn = pool->getConnection();
        if (conn) {
            // 4. Use the connection to execute a query
            bool success = conn->execute("UPDATE users SET last_login = NOW() WHERE id = 1;");
            if (success) {
                std::cout << "Query executed successfully." << std::endl;
            } else {
                std::cerr << "Query failed: " << conn->getError() << std::endl;
            }

            // 5. (Optional) When `conn` goes out of scope, its destructor automatically returns the connection to the pool.
            // You can also manually release it: pool->releaseConnection(conn);
        } else {
            std::cerr << "Failed to get connection from pool." << std::endl;
        }

        // 6. When the program ends, the connection pool automatically cleans up all connections
        return 0;
    }
    ```

3.  **Linking to Your Project**
    In your project's `CMakeLists.txt`:
    ```cmake
    find_package(ConnectionPool REQUIRED) # If installed
    # Or use add_subdirectory
    add_subdirectory(path/to/ConnectionPool)
    target_link_libraries(your_target PRIVATE ConnectionPool::ConnectionPool)
    ```

4.  **Testing and Performance Optimization**
    *   **Stress Testing:** Use `tests/ConcurrentTest.cpp` for multi-threaded concurrent testing to simulate high-load scenarios.
    *   **Performance Monitoring:** Focus on metrics like average connection acquisition time, queries per second (QPS), and connection pool utilization rate.
    *   **Optimization Suggestions:**
        *   Adjust `init_size` and `max_size` based on actual load to avoid being too large or too small.
        *   Set a reasonable `max_idle_time` to periodically clean up idle connections.
        *   Ensure `connection_timeout` is set appropriately to prevent threads from blocking for too long.

## How to Contribute

Contributions of all forms are welcome!
1.  Fork the repository.
2.  Create a feature branch (`git checkout -b feature/AmazingFeature`).
3.  Commit your changes (`git commit -m 'Add some AmazingFeature'`).
4.  Push to the branch (`git push origin feature/AmazingFeature`).
5.  Open a Pull Request.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Getting Help

If you have questions or suggestions, you can contact us via:
*   Submitting an issue at: `<your-github-repo-url>/issues`.
*   Sending an email to: [lic@mail.nwpu.edu.cn].