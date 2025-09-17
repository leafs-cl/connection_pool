# 创建静态库
add_library(connect_pool_lib STATIC 
    Connection.cpp
    ConnectionPool.cpp    
)
# 引用依赖的头文件，递归解析
target_include_directories(connection_pool_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
# 链接MYSQL库
target_link_libraries(connect_pool_lib PRIVATE
    ${MYSQL_LIBRARIES}
    fmt::fmt
)

# 创建可执行文件
add_executable(
    connection_pool
    main.cpp
)

# 将静态库链接到可执行文件
target_link_libraries(connection_pool PRIVATE connect_pool_lib)