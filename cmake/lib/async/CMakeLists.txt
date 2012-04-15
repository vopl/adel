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
SOURCE_GROUP(include FILES ${INC})

set(SRC
	${PSRC}/pch.hpp

	${PSRC}/service.cpp
	${PSRC}/event.cpp
	${PSRC}/mutex.cpp
	
	${PSRC}/freeFunctions.cpp
)
SOURCE_GROUP(src FILES ${SRC})

set(SRC_IMPL
	${PSRC}/impl/service.hpp
	${PSRC}/impl/service.cpp
	${PSRC}/impl/event.hpp
	${PSRC}/impl/event.cpp
	${PSRC}/impl/mutex.hpp
	${PSRC}/impl/mutex.cpp
	
	${PSRC}/impl/fiber.hpp
	${PSRC}/impl/fiber.cpp
	${PSRC}/impl/fiberRoot.hpp
	${PSRC}/impl/fiberRoot.cpp
	${PSRC}/impl/worker.hpp
	${PSRC}/impl/worker.cpp

	${PSRC}/impl/fiberPool.hpp
	${PSRC}/impl/threadLocalStorage.hpp
)
SOURCE_GROUP(src/impl FILES ${SRC_IMPL})




include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckPrototypeDefinition)
include(CheckTypeSize)

check_include_files (ucontext.h HAVE_UCONTEXT_H)

if(WINDOWS)
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








add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_IMPL})

set(PCH ${PSRC}/pch.hpp)
create_pch(${PROJECT} ${PCH})
use_pch(${PROJECT} ${PCH} ${SRC} ${SRC_IMPL})

target_link_libraries(${PROJECT} utils)
target_link_libraries(${PROJECT} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT} ${l4cpl})


