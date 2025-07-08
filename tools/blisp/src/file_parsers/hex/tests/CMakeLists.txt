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


add_executable(hex_file_test test_hex_file.cpp ../hex_file.c)

target_link_libraries(hex_file_test
        PRIVATE
        GTest::GTest
        )
include_directories(hex_file_test PRIVATE ../ ../../)
add_test(hex_file_test_gtests hex_file_test)

configure_file(Config.h.in ${CMAKE_BINARY_DIR}/Config.h)
include_directories(${CMAKE_BINARY_DIR})

set(TEST_APP_NAME hex_file_tests)
