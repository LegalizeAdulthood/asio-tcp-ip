cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(asio-nntp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(BoostAsio)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(nntp)
add_subdirectory(news-reader)
