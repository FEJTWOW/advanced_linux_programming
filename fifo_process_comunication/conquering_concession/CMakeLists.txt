cmake_minimum_required(VERSION 3.17)
project(zdobywanie_koncesji C)

set(CMAKE_C_STANDARD 99)

add_executable(zdobywanie_koncesji urzad-regulacji.c poszukiwacz.c)