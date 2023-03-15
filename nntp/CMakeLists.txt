add_library(nntp
    client.h client.cpp
)
target_include_directories(nntp PUBLIC include/nntp)
target_link_libraries(nntp PUBLIC boost::asio)
