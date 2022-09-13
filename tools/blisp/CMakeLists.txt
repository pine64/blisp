add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/argtable3 ${CMAKE_CURRENT_BINARY_DIR}/argtable3)

add_executable(blisp src/main.c src/cmd/write.c)

target_include_directories(blisp PRIVATE "${CMAKE_SOURCE_DIR}/vendor/argtable3/src")
target_link_libraries(blisp PRIVATE argtable3)