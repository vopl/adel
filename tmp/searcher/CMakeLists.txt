cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)

PROJECT(test)
include_directories(.)

add_executable(test 
	main.cpp
	scom/impl/searcher.cpp)
