cmake_minimum_required(VERSION 3.15)
project(connection_pool_mysql CXX)

# set cpp verison = 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

find_package(fmt REQUIRED)
find_package(MYSQL REQUIRED) # 严格区分大小写，注意保持一致，生成{MYSQL_LIBRARIES}和${MYSQL_INCLUDE_DIR}
if (NOT MYSQL_FOUND)
    message(FATAL_ERROR "MYSQL development libraries not found!")
else()
    include_directories(${MYSQL_INCLUDE_DIRS})
endif()


if(EXISTS ${PROJECT_SOURCE_DIR}/third_party/mysql)
    include_directories(${PROJECT_SOURCE_DIR}/third_party/mysql/include)
    link_directories(${PROJECT_SOURCE_DIR}/third_party/mysql/lib)
endif()


if(UNIX AND NOT APPLE)
    target_compile_options(connection_pool PRIVATE -Wall -Wextra -Wpedantic)
endif()