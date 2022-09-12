cmake_minimum_required(VERSION 3.22)
project(blisp C)

set(CMAKE_C_STANDARD 23)

add_subdirectory(vendor/argtable3)

add_executable(blisp src/main.c src/cmd/write.c)

target_include_directories(blisp PRIVATE vendor/argtable3/src)
target_link_libraries(blisp PRIVATE argtable3)

target_sources(blisp PRIVATE
        ${CMAKE_SOURCE_DIR}/vendor/libserialport/serialport.c
        ${CMAKE_SOURCE_DIR}/vendor/libserialport/timing.c)

if(WIN32)
    target_link_libraries(blisp PRIVATE Setupapi.lib)
    target_compile_definitions(blisp PRIVATE LIBSERIALPORT_MSBUILD)
    target_sources(blisp PRIVATE
            ${CMAKE_SOURCE_DIR}/vendor/libserialport/windows.c)
elseif(UNIX AND NOT APPLE)
    target_sources(blisp PRIVATE
            ${CMAKE_SOURCE_DIR}/vendor/libserialport/linux.c
            ${CMAKE_SOURCE_DIR}/vendor/libserialport/linux_termios.c)
    target_compile_definitions(blisp PRIVATE
            LIBSERIALPORT_ATBUILD
            "SP_API=__attribute__((visibility(\"default\")))"
            "SP_PRIV=__attribute__((visibility(\"hidden\")))")
    target_include_directories(blisp PRIVATE ${CMAKE_SOURCE_DIR}/vendor/libserialport)
    write_file(${CMAKE_SOURCE_DIR}/vendor/libserialport/config.h "// bypass errors.")
endif()