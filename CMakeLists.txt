cmake_minimum_required(VERSION 3.15)
project(nbget C)

set(CMAKE_C_STANDARD 99)

include_directories(libcurl)
find_library(curl libcurl)
find_library(pthread pthread)
link_libraries(curl pthread)
add_executable(nbget main.c requester.c thread_control.c)
