cmake_minimum_required(VERSION 3.22)
project(blisp C)

set(CMAKE_C_STANDARD 23)

option(BLISP_BUILD_CLI "Build CLI Tool" OFF)

add_library(libblisp_obj OBJECT
        lib/blisp.c
        lib/chip/blisp_chip_bl60x.c
        lib/chip/blisp_chip_bl70x.c)

target_include_directories(libblisp_obj PRIVATE ${CMAKE_SOURCE_DIR}/include/)

set_property(TARGET libblisp_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(libblisp SHARED $<TARGET_OBJECTS:libblisp_obj>)
add_library(libblisp_static STATIC $<TARGET_OBJECTS:libblisp_obj>)


set_target_properties(libblisp PROPERTIES
        PUBLIC_HEADER "include/blisp.h"
        VERSION 1.0
        SOVERSION 1
        LIBRARY_OUTPUT_DIRECTORY "shared"
        OUTPUT_NAME "blisp")

set_target_properties(libblisp_static PROPERTIES
        PUBLIC_HEADER "include/blisp.h"
        VERSION 1.0
        SOVERSION 1
        ARCHIVE_OUTPUT_DIRECTORY "static"
        OUTPUT_NAME "blisp")

target_sources(libblisp_obj PRIVATE
        ${CMAKE_SOURCE_DIR}/vendor/libserialport/serialport.c
        ${CMAKE_SOURCE_DIR}/vendor/libserialport/timing.c)

target_include_directories(libblisp_obj PRIVATE ${CMAKE_SOURCE_DIR}/vendor/libserialport)

if(WIN32)
    target_link_libraries(libblisp_obj PRIVATE Setupapi.lib)
    target_compile_definitions(libblisp_obj PRIVATE LIBSERIALPORT_MSBUILD)
    target_sources(libblisp_obj PRIVATE
            ${CMAKE_SOURCE_DIR}/vendor/libserialport/windows.c)
elseif(UNIX AND NOT APPLE)
    target_sources(libblisp_obj PRIVATE
            ${CMAKE_SOURCE_DIR}/vendor/libserialport/linux.c
            ${CMAKE_SOURCE_DIR}/vendor/libserialport/linux_termios.c)
    target_compile_definitions(libblisp_obj PRIVATE
            LIBSERIALPORT_ATBUILD
            HAVE_TERMIOS2_SPEED
            HAVE_STRUCT_TERMIOS2
            HAVE_DECL_BOTHER
            "SP_API=__attribute__((visibility(\"default\")))"
            "SP_PRIV=__attribute__((visibility(\"hidden\")))")
    write_file(${CMAKE_SOURCE_DIR}/vendor/libserialport/config.h "// bypass errors.")
elseif(APPLE)
    target_sources(libblisp_obj PRIVATE
            ${CMAKE_SOURCE_DIR}/vendor/libserialport/macosx.c)
    target_link_libraries(libblisp_obj PRIVATE "-framework IOKit" "-framework CoreFoundation")
    target_compile_definitions(libblisp_obj PRIVATE LIBSERIALPORT_ATBUILD "SP_API=__attribute__((visibility(\"default\")))")
    target_include_directories(libblisp_obj PRIVATE ${CMAKE_SOURCE_DIR}/vendor/libserialport)
    write_file(${CMAKE_SOURCE_DIR}/vendor/libserialport/config.h "// bypass errors.")
endif()

if(BLISP_BUILD_CLI)
    add_subdirectory(tools/blisp)
endif()
