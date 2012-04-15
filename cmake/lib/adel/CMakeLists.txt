set(PROJECT adel)
set(PSRC ${SRCDIR}/adel)
set(PINC ${INCDIR}/adel)

project(${PROJECT})

set(INC
	${PINC}/log.hpp
	${PINC}/manager.hpp
)
SOURCE_GROUP(include FILES ${INC})

set(SRC
	${PSRC}/pch.hpp
	${PSRC}/manager.cpp
)
SOURCE_GROUP(src FILES ${SRC})


add_library(${PROJECT} STATIC ${INC} ${SRC})
create_pch(${PROJECT} ${PSRC}/pch.hpp)
use_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC})

target_link_libraries(${PROJECT} async)
target_link_libraries(${PROJECT} ${l4cpl})


