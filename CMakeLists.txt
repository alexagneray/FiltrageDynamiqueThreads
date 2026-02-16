cmake_minimum_required(VERSION 3.16)
project(transdata)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FiltrageDynamiqueThreads FiltrageDynamiqueThreads.cpp FiltrageDynamique.cpp)