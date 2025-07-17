add_executable(hex_file_test test_hex_file.cpp ../hex_file.c )

target_link_libraries(hex_file_test
        PRIVATE
        GTest::GTest
        )
include(GoogleTest)
include_directories(hex_file_test PRIVATE ../ ../../)
target_compile_definitions(hex_file_test PUBLIC "SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
gtest_discover_tests(hex_file_test)
