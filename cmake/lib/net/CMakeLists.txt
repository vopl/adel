set(PROJECT net)
set(PINC ${INCDIR}/net)
set(PSRC ${SRCDIR}/net)

project(${PROJECT})





set(INCLUDE
	${PINC}/channel.hpp
	${PINC}/packet.hpp
	${PINC}/acceptor.hpp
	${PINC}/connector.hpp
	${PINC}/log.hpp
)
source_group(include FILES ${INCLUDE})

set(SRC
	${PSRC}/channel.cpp
	${PSRC}/connector.cpp
	${PSRC}/acceptor.cpp
)
source_group(src FILES ${SRC})

set(SRC_HDR
	${PSRC}/pch.hpp
)
source_group(src FILES ${SRC_HDR})

set(SRC_IMPL
	${PSRC}/impl/connector.cpp
	${PSRC}/impl/acceptor.cpp
	${PSRC}/impl/channel.cpp
)
source_group(src/impl FILES ${SRC_IMPL})

set(SRC_IMPL_HDR
	${PSRC}/impl/connector.hpp
	${PSRC}/impl/acceptor.hpp
	${PSRC}/impl/channel.hpp
)
source_group(src/impl FILES ${SRC_IMPL_HDR})










set(SRC_PCH ${PSRC}/pch.cpp)
source_group(src FILES ${SRC_PCH})

set(SRC_NOPCH
)
source_group(src FILES ${SRC_NOPCH})


include(FindOpenSSL)
if(NOT OPENSSL_FOUND)
	message(FATAL_ERROR "OpenSSL required")
endif()
include_directories(${OPENSSL_INCLUDE_DIR})


include_directories(${PSRC})

add_library(${PROJECT} STATIC
    ${INCLUDE} 
    ${SRC} 
    ${SRC_HDR} 
    ${SRC_IMPL} 
    ${SRC_IMPL_HDR}
    ${SRC_PCH}
    ${SRC_NOPCH} 
    )

create_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC_PCH})
use_pch(${PROJECT} ${PSRC}/pch.hpp 
	${SRC}
	${SRC_IMPL}
)

target_link_libraries(${PROJECT} utils async)
target_link_libraries(${PROJECT} ${OPENSSL_LIBRARIES})


if((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG) AND WINDOWS)
	target_link_libraries(${PROJECT} Mswsock.lib)
endif()
