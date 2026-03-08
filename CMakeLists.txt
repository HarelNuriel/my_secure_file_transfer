cmake_minimum_required(VERSION 4.1)
project(secure_file_transfer C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c
        src/server.c
        include/server.h
        src/client.c
        include/client.h
        src/common_utils.c
        include/common_utils.h)

add_executable(secure_file_transfer ${SOURCE_FILES})

target_include_directories(secure_file_transfer PUBLIC include)
target_link_libraries(secure_file_transfer ws2_32)