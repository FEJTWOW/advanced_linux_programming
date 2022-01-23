cmake_minimum_required(VERSION 3.17)
project(sockets C)

set(CMAKE_C_STANDARD 99)

add_executable(sockets main.c CircularBuffer.h CircularBuffer.c)
