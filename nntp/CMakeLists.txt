add_library(nntp
    include/nntp/client.h client.cpp
    include/nntp/processor.h processor.cpp
)
target_compile_features(nntp PUBLIC cxx_std_17)
target_include_directories(nntp PUBLIC include)
target_link_libraries(nntp PUBLIC Boost::asio)

add_subdirectory(test)
