include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(argparser_tests test_argparser.cpp)
target_link_libraries(argparser_tests PRIVATE argparser GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(argparser_tests)
