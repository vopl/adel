set(PROJECT scom)
set(PSRC ${SRCDIR}/scom)
set(PINC ${INCDIR}/scom)

project(${PROJECT})

set(INC
	${PINC}/log.hpp
	${PINC}/service.hpp
)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/service.cpp
)
source_group(src FILES ${SRC})

set(SRC_HDR
)
source_group(src FILES ${SRC_HDR})


set(SRC_PCH
	${PSRC}/pch.hpp
	${PSRC}/pch.cpp
)
source_group(src FILES ${SRC_PCH})












set(IMPL_INC
	${PSRC}/impl/service.hpp
	${PSRC}/impl/workerRaii.hpp
	${PSRC}/impl/pageRuleApplyer.hpp
	${PSRC}/impl/pageRuleApplyersContainer.hpp
	${PSRC}/impl/phrase.hpp
	${PSRC}/impl/phraseStreamer.hpp
	${PSRC}/impl/textParser.hpp
	${PSRC}/impl/word.hpp
	${PSRC}/impl/wordBucket.hpp
	${PSRC}/impl/reportGenerator.hpp

)
source_group(src/impl FILES ${IMPL_INC})

set(IMPL_SRC
	${PSRC}/impl/service.cpp
	${PSRC}/impl/pageRuleApplyer.cpp
	${PSRC}/impl/pageRuleApplyersContainer.cpp
	${PSRC}/impl/phrase.cpp
	${PSRC}/impl/phraseStreamer.cpp
	${PSRC}/impl/textParser.cpp
	${PSRC}/impl/word.cpp
	${PSRC}/impl/wordBucket.cpp
	${PSRC}/impl/reportGenerator.cpp
)
source_group(src/impl FILES ${IMPL_SRC})



include_directories(${PSRC})


include(FindHunSpell)
if(NOT HUNSPELL_FOUND)
	message(FATAL_ERROR "hunspell required")
endif()

include_directories(${HUNSPELL_INCLUDE_DIR})
add_definitions(-DHUNSPELL_STATIC)


include(FindIconv)
if(NOT ICONV_FOUND)
	message(FATAL_ERROR "iconv required")
endif()

include_directories(${ICONV_INCLUDE_DIRS})


include(FindSqlite3)
if(NOT SQLITE3_FOUND)
	message(FATAL_ERROR "sqlite3 required")
endif()

include_directories(${SQLITE3_INCLUDE_DIR})


#find_package(XMLRPC REQUIRED c++)
#if(NOT XMLRPC_FOUND)
#	message(FATAL_ERROR "xmlrpc required")
#endif()
#message(STATUS "Found xmlrpc: ${XMLRPC_LIBRARIES}")
#include_directories(${XMLRPC_INCLUDE_DIRS})




add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_PCH} ${SRC_HDR} ${IMPL_INC} ${IMPL_SRC})
create_pch(${PROJECT} ${PSRC}/pch.hpp ${PSRC}/pch.cpp)
use_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC} ${IMPL_SRC})

target_link_libraries(${PROJECT} async http pgc utils htmlcxx utf8proc)
target_link_libraries(${PROJECT} ${l4cpl})
target_link_libraries(${PROJECT} ${HUNSPELL_LIBRARIES})
target_link_libraries(${PROJECT} ${ICONV_LIBRARIES})
#target_link_libraries(${PROJECT} ${XMLRPC_LIBRARIES})
target_link_libraries(${PROJECT} ${SQLITE3_LIBRARIES})

