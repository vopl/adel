set(PROJECT htmlcxx)
set(PINC ${INCDIR}/htmlcxx)
set(PSRC ${SRCDIR}/htmlcxx)

project(${PROJECT})






set(INC

	${PINC}/html/ParserSax.h
	${PINC}/html/ParserSax.tcc
	${PINC}/html/ParserDom.h
	${PINC}/html/utils.h
	${PINC}/html/tree.h
	${PINC}/html/Node.h
	${PINC}/html/Uri.h
	${PINC}/html/CharsetConverter.h
	${PINC}/html/Extensions.h
	${PINC}/html/ci_string.h
	${PINC}/html/tld.h

)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/html/ParserSax.cc
	${PSRC}/html/ParserDom.cc
	${PSRC}/html/Extensions.cc
	${PSRC}/html/CharsetConverter.cc
	${PSRC}/html/Uri.cc
	${PSRC}/html/Node.cc
	${PSRC}/html/utils.cc
)
source_group(src FILES ${SRC})

set(SRC_HDR
	${PSRC}/html/wincstring.h
)
source_group(src FILES ${SRC_HDR})

include_directories(${PINC}/html)

include(FindIconv.cmake)
if(NOT ICONV_FOUND)
	message(FATAL_ERROR "iconv required")
endif()

include_directories(${ICONV_INCLUDE_DIRS})


add_library(${PROJECT} STATIC
    ${INC} 
    ${SRC} 
    ${SRC_HDR}
    )
target_link_libraries(${PROJECT} ${ICONV_LIBRARIES})
