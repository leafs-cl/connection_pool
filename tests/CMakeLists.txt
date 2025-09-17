# 创建测试可执行文件
add_executable(test_pool test_pool.cpp)
target_link_libraries(test_pool PRIVATE
    connection_pool_lib  # 链接核心库
    GTest::gtest_main
    fmt::fmt
)

# 注册测试用例
add_test(NAME ConnectionPoolTest COMMAND test_pool)