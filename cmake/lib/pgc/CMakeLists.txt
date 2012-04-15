set(PROJECT pgc)
set(PINC ${INCDIR}/pgc)
set(PSRC ${SRCDIR}/pgc)

project(${PROJECT})

set(INCLUDE
	${PINC}/db.hpp
	${PINC}/connection.hpp
	${PINC}/statement.hpp
	${PINC}/result.hpp
	${PINC}/log.hpp
)
source_group(include FILES ${INCLUDE})

set(SRC
	${PSRC}/db.cpp
	${PSRC}/statement.cpp
	${PSRC}/result.cpp
	${PSRC}/connection.cpp
)
source_group(src FILES ${SRC})

set(SRC_IMPL
	${PSRC}/impl/db.cpp
	${PSRC}/impl/statement.cpp
	${PSRC}/impl/result.cpp
	${PSRC}/impl/connectionHolder.cpp
	${PSRC}/impl/connection.cpp
	${PSRC}/impl/bindData.cpp
)
source_group(src/impl FILES ${SRC_IMPL})


set(SRC_NOPCH
	${PSRC}/pch.hpp
)
source_group(src FILES ${SRC_NOPCH})

set(SRC_IMPL_HDR
	${PSRC}/impl/db.hpp
	${PSRC}/impl/statement.hpp
	${PSRC}/impl/result.hpp
	${PSRC}/impl/connectionHolder.hpp
	${PSRC}/impl/connection.hpp
	${PSRC}/impl/bindData.hpp
)
source_group(src/impl FILES ${SRC_IMPL_HDR})


if(MSVC)
	set(SRC_PCH ${pSRC}/pch.cpp)
	source_group(src FILES ${SRC_PCH})
endif()

include_directories(${PSRC})

find_package(PostgreSQL)
if(NOT PostgreSQL_FOUND)
	MESSAGE(FATAL_ERROR "PostgreSQL not found")
endif()
#message(STATUS "Found PostgreSQL ${PostgreSQL_LIBRARY}")
include_directories(${PostgreSQL_INCLUDE_DIRS})









add_library(${PROJECT} STATIC
	${SRC_PCH}
	${SRC_NOPCH}
	${INCLUDE}
	${SRC}
	${SRC_HDR}
	${SRC_IMPL}
	${SRC_IMPL_HDR})

create_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC_PCH})
use_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC} ${SRC_IMPL})


target_link_libraries(${PROJECT} utils async)
target_link_libraries(${PROJECT} ${PostgreSQL_LIBRARY})

#if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
#	target_link_libraries(${PROJECT} ${Boost_LIBRARIES})
#if()
