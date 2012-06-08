set(PROJECT spider)
set(PSRC ${SRCDIR}/spider)
set(PINC ${INCDIR}/spider)

project(${PROJECT})

set(INC
	${PINC}/log.hpp
	${PINC}/service.hpp
)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/service.cpp
	${PSRC}/phrase.cpp
	${PSRC}/phraseStreamer.cpp
	${PSRC}/textParser.cpp
	${PSRC}/word.cpp
	${PSRC}/wordBucket.cpp
)
source_group(src FILES ${SRC})

set(SRC_HDR
	${PSRC}/phrase.hpp
	${PSRC}/phraseStreamer.hpp
	${PSRC}/textParser.hpp
	${PSRC}/word.hpp
	${PSRC}/wordBucket.hpp
)
source_group(src FILES ${SRC_HDR})


set(SRC_PCH
	${PSRC}/pch.hpp
	${PSRC}/pch.cpp
)
source_group(src FILES ${SRC_PCH})

include(FindHunSpell.cmake)

if(NOT HUNSPELL_FOUND)
	message(FATAL_ERROR "hunspell required")
endif()

include_directories(${HUNSPELL_INCLUDE_DIR})
add_definitions(-DHUNSPELL_STATIC)


include(../htmlcxx/FindIconv.cmake)
if(NOT ICONV_FOUND)
	message(FATAL_ERROR "iconv required")
endif()

include_directories(${ICONV_INCLUDE_DIRS})




add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_PCH} ${SRC_HDR})
create_pch(${PROJECT} ${PSRC}/pch.hpp ${PSRC}/pch.cpp)
use_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC})

target_link_libraries(${PROJECT} async http pgc utils htmlcxx utf8proc)
target_link_libraries(${PROJECT} ${l4cpl})
target_link_libraries(${PROJECT} ${HUNSPELL_LIBRARIES})
target_link_libraries(${PROJECT} ${ICONV_LIBRARIES})


