add_executable(dfu_file_test test_dfu_file.cpp ../dfu_file.c ../dfu_crc.c ../../get_file_contents.c)

target_link_libraries(dfu_file_test
        PRIVATE
        GTest::GTest
        )
include(GoogleTest)
include_directories(dfu_file_test PRIVATE ../ ../../)
target_compile_definitions(dfu_file_test PUBLIC "SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
gtest_discover_tests(dfu_file_test)
