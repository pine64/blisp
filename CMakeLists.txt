cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
project(blisp C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

option(BLISP_BUILD_CLI "Build CLI Tool" OFF)
option(BLISP_USE_SYSTEM_LIBRARIES "Use system-installed libraries" "${CMAKE_USE_SYSTEM_LIBRARIES}")
option(COMPILE_TESTS "Compile the tests" OFF)

add_library(libblisp_obj OBJECT
        lib/blisp.c lib/blisp_easy.c
        lib/chip/blisp_chip_bl60x.c
        lib/chip/blisp_chip_bl70x.c
        lib/chip/blisp_chip_bl808.c)

target_include_directories(libblisp_obj PRIVATE ${CMAKE_SOURCE_DIR}/include/)

set_property(TARGET libblisp_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(libblisp SHARED $<TARGET_OBJECTS:libblisp_obj>)
add_library(libblisp_static STATIC $<TARGET_OBJECTS:libblisp_obj>)

set(BLISP_PUBLIC_HEADERS
    include/blisp.h
    include/blisp_easy.h
    include/blisp_chip.h
    include/blisp_struct.h
    include/blisp_util.h)

set_target_properties(libblisp PROPERTIES
        PUBLIC_HEADER "${BLISP_PUBLIC_HEADERS}"
        VERSION 0.0.4
        SOVERSION 1
        LIBRARY_OUTPUT_DIRECTORY "shared"
        OUTPUT_NAME "blisp")

set_target_properties(libblisp_static PROPERTIES
        PUBLIC_HEADER "${BLISP_PUBLIC_HEADERS}"
        VERSION 0.0.4
        SOVERSION 1
        ARCHIVE_OUTPUT_DIRECTORY "static"
        OUTPUT_NAME "blisp")

if(BLISP_USE_SYSTEM_LIBRARIES)
    find_package(Libserialport REQUIRED)
    target_link_libraries(libblisp PUBLIC Libserialport::Libserialport)
    target_link_libraries(libblisp_static PUBLIC Libserialport::Libserialport)
    target_include_directories(libblisp_obj PUBLIC ${Libserialport_INCLUDE_DIRS})
else()
    if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        target_sources(libblisp_obj PRIVATE
        ${CMAKE_SOURCE_DIR}/vendor/libserialport/serialport.c
        ${CMAKE_SOURCE_DIR}/vendor/libserialport/timing.c)

        target_include_directories(libblisp_obj PRIVATE ${CMAKE_SOURCE_DIR}/vendor/libserialport)
    endif()

    if(WIN32)
        target_link_libraries(libblisp PRIVATE Setupapi.lib)
        target_link_libraries(libblisp_static PRIVATE Setupapi.lib)
        target_compile_definitions(libblisp_obj PRIVATE LIBSERIALPORT_MSBUILD)
        target_sources(libblisp_obj PRIVATE
                ${CMAKE_SOURCE_DIR}/vendor/libserialport/windows.c)
    elseif(UNIX AND NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
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
    elseif(UNIX AND ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        target_include_directories(libblisp_obj PRIVATE /usr/local/include/)
        target_link_libraries(libblisp PRIVATE -L/usr/local/lib usb serialport)
        target_link_libraries(libblisp_static PRIVATE -L/usr/local/lib usb serialport)
    elseif(APPLE)
        target_sources(libblisp_obj PRIVATE
                ${CMAKE_SOURCE_DIR}/vendor/libserialport/macosx.c)
        target_link_libraries(libblisp PRIVATE "-framework IOKit" "-framework CoreFoundation")
        target_compile_definitions(libblisp_obj PRIVATE
                LIBSERIALPORT_ATBUILD
                "SP_PRIV=__attribute__((visibility(\"hidden\")))"
                "SP_API=__attribute__((visibility(\"default\")))")
        target_include_directories(libblisp_obj PRIVATE ${CMAKE_SOURCE_DIR}/vendor/libserialport)
        write_file(${CMAKE_SOURCE_DIR}/vendor/libserialport/config.h "// bypass errors.")
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS libblisp libblisp_static
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(BLISP_BUILD_CLI)
    add_subdirectory(tools/blisp)
endif()


if(COMPILE_TESTS)
    # Bring in googletest & C++
    enable_language(CXX)
    enable_testing()
    include(FetchContent)


    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)
    add_library(GTest::GTest INTERFACE IMPORTED)
    target_link_libraries(GTest::GTest INTERFACE gtest_main)

    add_subdirectory(tools/blisp/src/file_parsers/dfu/tests)
    add_subdirectory(tools/blisp/src/file_parsers/hex/tests)
endif(COMPILE_TESTS)
