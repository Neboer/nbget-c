cmake_minimum_required(VERSION 3.15)
project(nbget C)

set(CMAKE_C_STANDARD 99)

include_directories(libcurl)
find_library(curl libcurl)
link_libraries(curl)
add_executable(nbget main.c requester.c)
