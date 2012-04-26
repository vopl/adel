set(PROJECT net)
set(PINC ${INCDIR}/net)
set(PSRC ${SRCDIR}/net)

project(${PROJECT})





set(INCLUDE
	${PINC}/channel.hpp
	${PINC}/packet.hpp
	${PINC}/acceptor.hpp
	${PINC}/connector.hpp
	${PINC}/message.hpp
	${PINC}/log.hpp
)
source_group(include FILES ${INCLUDE})

set(SRC
	${PSRC}/channel.cpp
	${PSRC}/connector.cpp
	${PSRC}/acceptor.cpp
	${PSRC}/message.cpp
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
	${PSRC}/impl/message.cpp
)
source_group(src/impl FILES ${SRC_IMPL})

set(SRC_IMPL_HDR
	${PSRC}/impl/connector.hpp
	${PSRC}/impl/acceptor.hpp
	${PSRC}/impl/channel.hpp
	${PSRC}/impl/message.hpp
)
source_group(src/impl FILES ${SRC_IMPL_HDR})

















set(INCLUDE_HTTP
	${PINC}/http/server.hpp
	${PINC}/http/client.hpp
	${PINC}/http/statusCode.hpp
	${PINC}/http/method.hpp
	${PINC}/http/version.hpp
	${PINC}/http/contentEncoding.hpp
	${PINC}/http/transferEncoding.hpp
	${PINC}/http/headerValue.hpp
	${PINC}/http/headerName.hpp
)
source_group(include/http FILES ${INCLUDE_HTTP})

set(INCLUDE_HTTP_IMPL
	${PINC}/http/impl/headerName.hpp
)
source_group(include/http/impl FILES ${INCLUDE_HTTP_IMPL})

set(INCLUDE_HTTP_SERVER
	${PINC}/http/server/log.hpp
	${PINC}/http/server/request.hpp
	${PINC}/http/server/response.hpp
	${PINC}/http/server/handlerFs.hpp
)
source_group(include/http/server FILES ${INCLUDE_HTTP_SERVER})

set(SRC_HTTP
	${PSRC}/http/server.cpp
	${PSRC}/http/client.cpp
	${PSRC}/http/statusCode.cpp
	${PSRC}/http/headerValue_parsers.cpp
	${PSRC}/http/headerValue_generators.cpp
)
source_group(src/http FILES ${SRC_HTTP})

set(SRC_HTTP_SERVER
	${PSRC}/http/server/request.cpp
	${PSRC}/http/server/response.cpp
	${PSRC}/http/server/handlerFs.cpp
)
source_group(src/http/server FILES ${SRC_HTTP_SERVER})

set(SRC_HTTP_HDR
)
source_group(src/http FILES ${SRC_HTTP_HDR})

set(SRC_HTTP_IMPL
	${PSRC}/http/impl/server.cpp
	${PSRC}/http/impl/client.cpp
	${PSRC}/http/impl/contentFilter.cpp
	${PSRC}/http/impl/contentFilterEncodeZlib.cpp
	${PSRC}/http/impl/contentFilterEncodeChunked.cpp
	${PSRC}/http/impl/contentFilterDecodeZlib.cpp
	${PSRC}/http/impl/contentFilterDecodeChunked.cpp
)
source_group(src/http/impl FILES ${SRC_HTTP_IMPL})

set(SRC_HTTP_IMPL_HDR
	${PSRC}/http/impl/server.hpp
	${PSRC}/http/impl/client.hpp
	${PSRC}/http/impl/contentFilter.hpp
	${PSRC}/http/impl/contentFilterEncodeZlib.hpp
	${PSRC}/http/impl/contentFilterEncodeChunked.hpp
	${PSRC}/http/impl/contentFilterDecodeZlib.hpp
	${PSRC}/http/impl/contentFilterDecodeChunked.hpp
)
source_group(src/http/impl FILES ${SRC_HTTP_IMPL_HDR})



set(SRC_HTTP_SERVER_IMPL
	${PSRC}/http/server/impl/request.cpp
	${PSRC}/http/server/impl/response.cpp
	${PSRC}/http/server/impl/handlerFs.cpp
)
source_group(src/http/server/impl FILES ${SRC_HTTP_SERVER_IMPL})

set(SRC_HTTP_SERVER_IMPL_HDR
	${PSRC}/http/server/impl/request.hpp
	${PSRC}/http/server/impl/response.hpp
	${PSRC}/http/server/impl/handlerFs.hpp
)
source_group(src/http/server/impl FILES ${SRC_HTTP_SERVER_IMPL_HDR})





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
    ${INCLUDE_HTTP} 
    ${INCLUDE_HTTP_IMPL} 
    ${SRC_HTTP}
    ${INCLUDE_HTTP_SERVER} 
    ${SRC_HTTP_SERVER}
    ${SRC_HTTP_HDR} 
    ${SRC_HTTP_IMPL} 
    ${SRC_HTTP_IMPL_HDR}
    ${SRC_HTTP_SERVER_IMPL} 
    ${SRC_HTTP_SERVER_IMPL_HDR}
    
    ${SRC_PCH}
    ${SRC_NOPCH} 
    )

create_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC_PCH})
use_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC} ${SRC_IMPL} ${SRC_HTTP} ${SRC_HTTP_IMPL})

target_link_libraries(${PROJECT} utils async)
target_link_libraries(${PROJECT} ${OPENSSL_LIBRARIES})
target_link_libraries(${PROJECT} ${ZLIB_LIBRARIES})


if((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG) AND WINDOWS)
	target_link_libraries(${PROJECT} Mswsock.lib)
endif()
