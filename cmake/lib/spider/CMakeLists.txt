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
)
source_group(src FILES ${SRC})


set(SRC_PCH
	${PSRC}/pch.hpp
	${PSRC}/pch.cpp
)
source_group(src FILES ${SRC_PCH})

add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_PCH})
create_pch(${PROJECT} ${PSRC}/pch.hpp ${PSRC}/pch.cpp)
use_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC})

target_link_libraries(${PROJECT} async http pgc utils)
target_link_libraries(${PROJECT} ${l4cpl})


