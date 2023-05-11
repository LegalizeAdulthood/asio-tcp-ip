find_package(pdcurses REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)

add_executable(news-reader
    news-reader.cpp
)
target_link_libraries(news-reader PRIVATE Boost::asio OpenSSL::Crypto OpenSSL::SSL pdcurses)
