cmake_minimum_required(VERSION 4.1)
project(my_secure_file_transfer C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c
        src/server.c
        include/server.h
        src/client.c
        include/client.h
        src/common_utils.c
        include/common_utils.h
        src/auth.c
        include/auth.h
        src/sha256.c
        include/sha256.h)

add_executable(my_secure_file_transfer ${SOURCE_FILES})

target_include_directories(my_secure_file_transfer PUBLIC include)
