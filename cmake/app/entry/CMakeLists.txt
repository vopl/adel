project(entry)

set(INC
	${SRCDIR}/entry/pch.hpp
)

set(SRC
	${SRCDIR}/entry/main.cpp
)

add_executable(entry ${INC} ${SRC})
CREATE_PCH(entry ${SRCDIR}/entry/pch.hpp)
USE_PCH(entry ${SRCDIR}/entry/pch.hpp ${SRC})

target_link_libraries(entry 
    adel
    net)