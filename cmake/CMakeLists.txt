

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/local.cmake")
	message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/local.cmake found, do inclusion")
	include("${CMAKE_CURRENT_BINARY_DIR}/local.cmake")
endif()


if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "build type")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

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

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/include ${CMAKE_MODULE_PATH})

INCLUDE_DIRECTORIES(${INCDIR})
INCLUDE_DIRECTORIES(${SRCDIR})

#######################################################################################
IF(MSVC)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ot /Og /Oi /Ox /Oy /GL /Zi")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /DEBUG")
	SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /LTCG /DEBUG")
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /DEBUG")
ENDIF(MSVC)

#if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
#    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
#endif()

add_definitions(-D_REENTRANT)
add_definitions(-D_THREAD_SAFE)

###################################################################
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)

find_package( Boost 1.48.0 REQUIRED COMPONENTS system date_time filesystem thread random program_options chrono regex)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

if(MSVC)
    add_definitions(-DBOOST_ALL_NO_LIB)
endif()
###################################################################
set(CMAKE_THREAD_PREFER_PTHREAD ON)
include(FindThreads)

###################################################################
#find_library(l4cpl log4cplus)
#find_library(l4cpld log4cplusd)

#if("l4cpl-NOTFOUND" STREQUAL "${l4cpl}")
#	message(FATAL_ERROR "log4cplus library not found")
#endif()

#if("l4cpld-NOTFOUND" STREQUAL "${l4cpld}")
#	message("log4cplus library not found in debug mode, will use release")
#	set(l4cpld ${l4cpl})
#endif()

#set(l4cpl optimized ${l4cpl} debug ${l4cpld})

#message(STATUS "Found log4cplus ${l4cpl}")

include(FindLog4cplus)
if(NOT LOG4CPLUS_FOUND)
	message(FATAL_ERROR "Log4clpus required")
endif()
include_directories(${LOG4CPLUS_INCLUDE_DIR})
set(l4cpl ${LOG4CPLUS_LIBRARIES})


include(pch.cmake)
if(MSVC)
	add_definitions(-D_WIN32_WINNT=0x0501)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

add_subdirectory(app)
add_subdirectory(lib)

