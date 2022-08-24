cmake_minimum_required(VERSION 3.22)
project(blisp C)

set(CMAKE_C_STANDARD 23)

add_subdirectory(vendor/argtable3)

add_executable(blisp src/main.c src/cmd/write.c)

target_include_directories(blisp PRIVATE vendor/argtable3/src)
target_link_libraries(blisp PRIVATE argtable3)