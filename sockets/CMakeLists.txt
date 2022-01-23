cmake_minimum_required(VERSION 3.17)
project(szokety C)

set(CMAKE_C_STANDARD 99)

add_executable(szokety main.c CircularBuffer.h CircularBuffer.c)