cmake_minimum_required(VERSION 3.22)
project(blisp C)

set(CMAKE_C_STANDARD 23)

option(BLISP_BUILD_CLI "Build CLI Tool" OFF)
option(BLISP_DYNAMIC_LIBRARY "Build dynamic library" OFF)

if(BLISP_DYNAMIC_LIBRARY)
    add_library(libblisp SHARED)
else()
    add_library(libblisp STATIC)
endif()

target_sources(libblisp PRIVATE lib/blisp_easy.c)

target_include_directories(libblisp PRIVATE ${CMAKE_SOURCE_DIR}/include/)

set_target_properties(libblisp PROPERTIES
        PUBLIC_HEADER "include/blisp_easy.h"
        VERSION 1.0
        SOVERSION 1
        OUTPUT_NAME "blisp"
)

if(BLISP_BUILD_CLI)
    add_subdirectory(tools/blisp)
endif()


#target_sources(blisp PRIVATE
#        ${CMAKE_SOURCE_DIR}/vendor/libserialport/serialport.c
#        ${CMAKE_SOURCE_DIR}/vendor/libserialport/timing.c)
#
#if(WIN32)
#    target_link_libraries(blisp PRIVATE Setupapi.lib)
#    target_compile_definitions(blisp PRIVATE LIBSERIALPORT_MSBUILD)
#    target_sources(blisp PRIVATE
#            ${CMAKE_SOURCE_DIR}/vendor/libserialport/windows.c)
#elseif(UNIX AND NOT APPLE)
#    target_sources(blisp PRIVATE
#            ${CMAKE_SOURCE_DIR}/vendor/libserialport/linux.c
#            ${CMAKE_SOURCE_DIR}/vendor/libserialport/linux_termios.c)
#    target_compile_definitions(blisp PRIVATE
#            LIBSERIALPORT_ATBUILD
#            "SP_API=__attribute__((visibility(\"default\")))"
#            "SP_PRIV=__attribute__((visibility(\"hidden\")))")
#    target_include_directories(blisp PRIVATE ${CMAKE_SOURCE_DIR}/vendor/libserialport)
#    write_file(${CMAKE_SOURCE_DIR}/vendor/libserialport/config.h "// bypass errors.")
#endif()