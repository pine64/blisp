set(ARGTABLE3_ENABLE_TESTS OFF CACHE BOOL "Enable unit tests")
set(ARGTABLE3_ENABLE_EXAMPLES OFF CACHE BOOL "Enable examples")
#set(ARGTABLE3_REPLACE_GETOPT OFF CACHE BOOL "Replace getopt in the system C library")

add_executable(blisp src/main.c src/cmd/write.c src/util.c src/common.c src/cmd/iot.c)

add_subdirectory(src/file_parsers)


if(BLISP_USE_SYSTEM_LIBRARIES)
    find_package(Argtable3 REQUIRED)
else()
    add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/argtable3 ${CMAKE_CURRENT_BINARY_DIR}/argtable3)

    target_include_directories(blisp PRIVATE
        "${CMAKE_SOURCE_DIR}/vendor/argtable3/src")
endif()

target_include_directories(blisp PRIVATE
        "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(blisp PRIVATE
        argtable3
        libblisp_static file_parsers)

if (WIN32)
    target_link_libraries(blisp PRIVATE Setupapi.lib)
elseif (APPLE)
    target_link_libraries(blisp PRIVATE "-framework IOKit" "-framework CoreFoundation")
endif ()

install(TARGETS blisp DESTINATION bin)
