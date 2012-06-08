set(PROJECT utf8proc)
set(PINC ${INCDIR}/utf8proc)
set(PSRC ${SRCDIR}/utf8proc)

project(${PROJECT})

set(INC
	${PINC}/utf8proc.h
)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/utf8proc.cpp
)
source_group(src FILES ${SRC})

add_library(${PROJECT} STATIC ${INC} ${SRC})
