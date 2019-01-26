cmake_minimum_required(VERSION 3.13)
project(function)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g -O0)

add_executable(function main.cpp function.h)