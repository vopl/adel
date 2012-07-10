cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)

PROJECT(test)
include_directories(.)
include_directories(/home/local/include/boost-1_49)

add_executable(test 
	main.cpp
	scom/impl/searcher.cpp)
