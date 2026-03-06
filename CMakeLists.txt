cmake_minimum_required(VERSION 3.20)
project(myftp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        src/FtpSession/FtpSession.h++
        src/Server/Commands.cpp
        src/Server/Commands.h++
        src/Client/Client.cpp
        src/Client/Client.h++
        src/Socket/Socket.cpp
        src/Socket/Socket.h++
        src/Poller/Poller.cpp
        src/Poller/Poller.h++
        src/Client/DataTransferManager.cpp
        src/Client/DataTransferManager.h++)
target_include_directories(myftp PRIVATE include src)
target_compile_options(myftp PRIVATE -Wall -Wextra -Werror)

# TEST CRITERION

option(BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)
    enable_testing()
    find_library(CRITERION_LIB criterion)

    if (CRITERION_LIB)
        file(GLOB_RECURSE LIB_SRC "src/*.cpp")
        list(FILTER LIB_SRC EXCLUDE REGEX "src/main\\.cpp$")
        file(GLOB_RECURSE TEST_SRC "tests/*.cpp")

        add_executable(unittests EXCLUDE_FROM_ALL ${LIB_SRC} ${TEST_SRC})

        target_compile_options(unittests PRIVATE -Wall -Wextra --coverage)
        target_link_options(unittests PRIVATE --coverage)
        target_include_directories(unittests PRIVATE include src)
        target_link_libraries(unittests ${CRITERION_LIB})

        add_test(NAME unittests COMMAND unit_tests)
    else ()
        message(WARNING "Criterion not found, unit tests will not be built.")
    endif ()
endif ()