find_package(pdcurses REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(news-reader
    news-reader.cpp
)
target_link_libraries(news-reader PRIVATE nntp OpenSSL pdcurses)
