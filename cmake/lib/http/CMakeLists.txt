set(PROJECT http)
set(PINC ${INCDIR}/http)
set(PSRC ${SRCDIR}/http)

project(${PROJECT})





set(INCLUDE
	${PINC}/log.hpp
	${PINC}/server.hpp
	${PINC}/client.hpp
	${PINC}/inputMessage.hpp
	${PINC}/outputMessage.hpp
	${PINC}/statusCode.hpp
	${PINC}/method.hpp
	${PINC}/version.hpp
	${PINC}/contentEncoding.hpp
	${PINC}/transferEncoding.hpp
	${PINC}/connection.hpp
	${PINC}/headerValue.hpp
	${PINC}/headerName.hpp
)
source_group(include FILES ${INCLUDE})

set(INCLUDE_IMPL
	${PINC}/impl/headerName.hpp
)
source_group(include/impl FILES ${INCLUDE_IMPL})

set(SRC
	${PSRC}/server.cpp
	${PSRC}/client.cpp
	${PSRC}/statusCode.cpp
	${PSRC}/headerValue_parsers.cpp
	${PSRC}/headerValue_generators.cpp
	${PSRC}/inputMessage.cpp
	${PSRC}/outputMessage.cpp
)
source_group(src FILES ${SRC})

set(SRC_HDR
	${PSRC}/pch.hpp
)
source_group(src FILES ${SRC_HDR})

set(SRC_IMPL
	${PSRC}/impl/server.cpp
	${PSRC}/impl/client.cpp
	${PSRC}/impl/contentFilter.cpp
	${PSRC}/impl/contentFilterChannelWriter.cpp
	${PSRC}/impl/contentFilterEncodeZlib.cpp
	${PSRC}/impl/contentFilterEncodeChunked.cpp
	${PSRC}/impl/contentFilterDecodeZlib.cpp
	${PSRC}/impl/contentFilterDecodeChunked.cpp
	${PSRC}/impl/inputMessage.cpp
	${PSRC}/impl/inputMessageBuffer.cpp
	${PSRC}/impl/outputMessage.cpp
)
source_group(src/impl FILES ${SRC_IMPL})

set(SRC_IMPL_HDR
	${PSRC}/impl/server.hpp
	${PSRC}/impl/client.hpp
	${PSRC}/impl/contentFilter.hpp
	${PSRC}/impl/contentFilterChannelWriter.hpp
	${PSRC}/impl/contentFilterEncodeZlib.hpp
	${PSRC}/impl/contentFilterEncodeChunked.hpp
	${PSRC}/impl/contentFilterDecodeZlib.hpp
	${PSRC}/impl/contentFilterDecodeChunked.hpp
	${PSRC}/impl/inputMessage.hpp
	${PSRC}/impl/inputMessageBuffer.hpp
	${PSRC}/impl/outputMessage.hpp
)
source_group(src/impl FILES ${SRC_IMPL_HDR})





set(INCLUDE_SERVER
	${PINC}/server/log.hpp
	${PINC}/server/request.hpp
	${PINC}/server/response.hpp
	${PINC}/server/handlerFs.hpp
)
source_group(include/server FILES ${INCLUDE_SERVER})

set(SRC_SERVER
	${PSRC}/server/request.cpp
	${PSRC}/server/response.cpp
	${PSRC}/server/handlerFs.cpp
)
source_group(src/server FILES ${SRC_SERVER})



set(SRC_SERVER_IMPL
	${PSRC}/server/impl/request.cpp
	${PSRC}/server/impl/response.cpp
	${PSRC}/server/impl/handlerFs.cpp
)
source_group(src/server/impl FILES ${SRC_SERVER_IMPL})

set(SRC_SERVER_IMPL_HDR
	${PSRC}/server/impl/request.hpp
	${PSRC}/server/impl/response.hpp
	${PSRC}/server/impl/handlerFs.hpp
)
source_group(src/server/impl FILES ${SRC_SERVER_IMPL_HDR})





set(SRC_PCH ${PSRC}/pch.cpp)
source_group(src FILES ${SRC_PCH})

set(SRC_NOPCH
)
source_group(src FILES ${SRC_NOPCH})


include(FindZLIB)
if(NOT ZLIB_FOUND)
	message(FATAL_ERROR "zlib required")
endif()
include_directories(${ZLIB_INCLUDE_DIR})


include_directories(${PSRC})

add_library(${PROJECT} STATIC
    ${INCLUDE} 
    ${SRC} 
    ${SRC_HDR} 
    ${SRC_IMPL} 
    ${SRC_IMPL_HDR}
    ${INCLUDE_IMPL} 
    ${INCLUDE_SERVER} 
    ${SRC_SERVER}
    ${SRC_IMPL_HDR}
    ${SRC_SERVER_IMPL} 
    ${SRC_SERVER_IMPL_HDR}
    
    ${SRC_PCH}
    ${SRC_NOPCH} 
    )

create_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC_PCH})
use_pch(${PROJECT} ${PSRC}/pch.hpp 
	${SRC}
	${SRC_IMPL}
	${SRC_SERVER}
	${SRC_IMPL} 
	${SRC_SERVER_IMPL} 
)

target_link_libraries(${PROJECT} utils async net)
target_link_libraries(${PROJECT} ${ZLIB_LIBRARIES})
