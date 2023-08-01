enable_language(CXX)

enable_testing()
include(FetchContent)


FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)


add_executable(dfu_file_test test_dfu_file.cpp ../dfu_file.c ../dfu_crc.c)

target_link_libraries(dfu_file_test
        PRIVATE
        GTest::GTest
        )
include_directories(dfu_file_test PRIVATE ../)
add_test(dfu_file_test_gtests dfu_file_test)

configure_file(Config.h.in ${CMAKE_BINARY_DIR}/Config.h)
include_directories(${CMAKE_BINARY_DIR})

set(TEST_APP_NAME dfu_file_tests)

#add_custom_command(TARGET ${TEST_APP_NAME} COMMAND ./${TEST_APP_NAME} POST_BUILD)