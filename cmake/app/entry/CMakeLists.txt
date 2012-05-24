set(PROJECT entry)
set(PSRC ${SRCDIR}/entry)
set(PINC ${INCDIR}/entry)

project(entry)

set(INC
	${PSRC}/pch.hpp
)
SOURCE_GROUP(include FILES ${INC})

set(SRC
	${PSRC}/main.cpp
)
source_group(src FILES ${SRC})

set(SRC_PCH
	${PSRC}/pch.cpp
)
source_group(src FILES ${SRC_PCH})

add_executable(entry ${INC} ${SRC} ${SRC_PCH})
create_pch(entry ${PSRC}/pch.hpp ${SRC_PCH})
use_pch(entry ${PSRC}/pch.hpp ${SRC})

target_link_libraries(entry 
    async
    net
    http
    pgc
    spider)