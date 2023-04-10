find_package(pdcurses)

add_executable(news-reader
    news-reader.cpp
)
target_link_libraries(news-reader PRIVATE nntp pdcurses)
