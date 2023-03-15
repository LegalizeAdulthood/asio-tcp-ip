find_package(GTest CONFIG REQUIRED)

add_executable(nntp-test test.cpp)
target_link_libraries(nntp-test PUBLIC nntp GTest::gtest_main)
