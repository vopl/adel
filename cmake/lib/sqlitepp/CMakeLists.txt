set(PROJECT sqlitepp)
set(PINC ${INCDIR}/sqlitepp)
set(PSRC ${SRCDIR}/sqlitepp)

project(${PROJECT})

set(INC
	${PINC}/binders.hpp
	${PINC}/converters.hpp
	${PINC}/exception.hpp
	${PINC}/into.hpp
	${PINC}/query.hpp
	${PINC}/session.hpp
	${PINC}/sqlitepp.hpp
	${PINC}/statement.hpp
	${PINC}/string.hpp
	${PINC}/transaction.hpp
	${PINC}/unicode.hpp
	${PINC}/use.hpp
)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/binders.cpp
	${PSRC}/exception.cpp
	${PSRC}/query.cpp
	${PSRC}/session.cpp
	${PSRC}/statement.cpp
	${PSRC}/string.cpp
	${PSRC}/transaction.cpp
)
source_group(src FILES ${SRC})

include(FindSqlite3)
if(NOT SQLITE3_FOUND)
    message(FATAL_ERROR "sqlite3 required")
endif()

include_directories(${SQLITE3_INCLUDE_DIR})


add_library(${PROJECT} STATIC ${INC} ${SRC})
target_link_libraries(${PROJECT} ${SQLITE3_LIBRARIES})
