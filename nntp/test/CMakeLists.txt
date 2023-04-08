find_package(GTest CONFIG REQUIRED)

add_executable(nntp-test
    testClient.cpp
    testProcessor.cpp
    testRope.cpp
)
target_link_libraries(nntp-test PUBLIC nntp GTest::gmock_main)
target_compile_features(nntp-test PUBLIC cxx_std_17)
