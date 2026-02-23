cmake_minimum_required(VERSION 3.20)
project(myftp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(CRITERION_LIB criterion)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
list(REMOVE_ITEM SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(myftp ${SRC_FILES} src/main.cpp
        src/Server/Server.cpp
        src/Server/Server.h++
        src/Utils/Utils.cpp
        src/Utils/Utils.h++
        src/Errors/MyFtpErrors.cpp
        src/Errors/MyFtpErrors.h++
        src/FtpSession/FtpSession.cpp
        src/FtpSession/FtpSession.h++)
target_include_directories(myftp PRIVATE include src)
target_compile_options(myftp PRIVATE -Wall -Wextra -Werror)

add_executable(unit_tests ${SRC_FILES} tests/main-tests.cpp)
target_include_directories(unit_tests PRIVATE include src)

target_link_libraries(unit_tests PRIVATE ${CRITERION_LIB})