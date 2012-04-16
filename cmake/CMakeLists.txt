

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/local.cmake")
	message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/local.cmake found, do inclusion")
	include("${CMAKE_CURRENT_BINARY_DIR}/local.cmake")
endif()


cmake_minimum_required(VERSION 2.6)

project(adel)

#message(${CMAKE_BUILD_TYPE})

set(TOPDIR ${CMAKE_SOURCE_DIR}/..)
get_filename_component(TOPDIR ${TOPDIR} ABSOLUTE)
set(SRCDIR ${TOPDIR}/src)
set(INCDIR ${TOPDIR}/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TOPDIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TOPDIR}/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TOPDIR}/RelWithDebInfo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${TOPDIR}/MinSizeRel)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TOPDIR}/Debug)

INCLUDE_DIRECTORIES(${INCDIR})
INCLUDE_DIRECTORIES(${SRCDIR})

###################################################################
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    ON)

find_package( Boost 1.48.0 REQUIRED COMPONENTS system date_time filesystem thread random program_options)
if (Boost_FOUND)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTI_THREADED ON)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

###################################################################
set(CMAKE_THREAD_PREFER_PTHREAD ON)
include(FindThreads)

###################################################################
find_library(l4cpl log4cplus)
find_library(l4cpld log4cplusd)

if("l4cpl-NOTFOUND" STREQUAL "${l4cpl}")
	message(FATAL_ERROR "log4cplus library not found")
endif()

if("l4cpld-NOTFOUND" STREQUAL "${l4cpld}")
	message("log4cplus library not found in debug mode, will use release")
	set(l4cpld ${l4cpl})
endif()

set(l4cpl optimized ${l4cpl} debug ${l4cpld})

message(STATUS "Found log4cplus ${l4cpl}")


include(pch.cmake)
if(MSVC)
	add_definitions(-D_WIN32_WINNT=0x0501)
endif()

add_subdirectory(app)
add_subdirectory(lib)

