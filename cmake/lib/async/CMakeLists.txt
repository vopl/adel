set(PROJECT async)
set(PINC ${INCDIR}/async)
set(PSRC ${SRCDIR}/async)

project(${PROJECT})

set(INC
	${PINC}/log.hpp

	${PINC}/service.hpp
	${PINC}/event.hpp
	${PINC}/future.hpp
	${PINC}/mutex.hpp
	
	${PINC}/cmaHandler.hpp
	${PINC}/exception.hpp
	${PINC}/eventWaiter.hpp
	${PINC}/futureWaiter.hpp
	${PINC}/asioBridge.hpp
	
	${PINC}/freeFunctions.hpp
)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/pch.hpp

	${PSRC}/service.cpp
	${PSRC}/event.cpp
	${PSRC}/mutex.cpp
	
	${PSRC}/freeFunctions.cpp
)
source_group(src FILES ${SRC})

set(SRC_PCH
	${PSRC}/pch.cpp
)
source_group(src FILES ${SRC_PCH})

set(SRC_IMPL
	${PSRC}/impl/service.cpp
	${PSRC}/impl/event.cpp
	${PSRC}/impl/mutex.cpp
	
	${PSRC}/impl/fiber.cpp
	${PSRC}/impl/fiberRoot.cpp
	${PSRC}/impl/worker.cpp
)
source_group(src/impl FILES ${SRC_IMPL})

set(SRC_IMPL_HDR
	${PSRC}/impl/service.hpp
	${PSRC}/impl/event.hpp
	${PSRC}/impl/mutex.hpp
	
	${PSRC}/impl/fiber.hpp
	${PSRC}/impl/fiberRoot.hpp
	${PSRC}/impl/worker.hpp

	${PSRC}/impl/fiberPool.hpp
	${PSRC}/impl/threadLocalStorage.hpp
)
source_group(src/impl FILES ${SRC_IMPL_HDR})



include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckPrototypeDefinition)
include(CheckTypeSize)

check_include_files (ucontext.h HAVE_UCONTEXT_H)

if(WIN32)
	check_prototype_definition(
		CreateFiberEx
		"LPVOID WINAPI CreateFiberEx(SIZE_T dwStackCommitSize, SIZE_T dwStackReserveSize, DWORD dwFlags, LPFIBER_START_ROUTINE lpStartAddress, LPVOID lpParameter);"
		"NULL"
		windows.h
		HAVE_WINFIBER)
endif()

check_type_size("void *" PVOID_SIZE)
add_definitions(-D\"PVOID_SIZE=${PVOID_SIZE}\")

check_type_size("int" INT_SIZE)
add_definitions(-D\"INT_SIZE=${INT_SIZE}\")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})








add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_PCH} ${SRC_IMPL} ${SRC_IMPL_HDR})

create_pch(${PROJECT} ${PSRC}/pch.hpp ${PSRC}/pch.cpp)
use_pch(${PROJECT} ${PSRC}/pch.hpp ${SRC} ${SRC_IMPL})

target_link_libraries(${PROJECT} utils)
target_link_libraries(${PROJECT} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT} ${l4cpl})


