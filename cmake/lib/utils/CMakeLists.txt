set(PROJECT utils)
set(PINC ${INCDIR}/utils)
set(PSRC ${SRCDIR}/utils)

project(${PROJECT})

set(INC
	${PINC}/options.hpp
	${PINC}/logInitializer.hpp
	${PINC}/logTemplate.hpp
	${PINC}/dateTimeDuration.hpp
	${PINC}/fixEndian.hpp
	${PINC}/implAccess.hpp
	${PINC}/julian.hpp
	${PINC}/ntoa.hpp
	${PINC}/aton.hpp
	${PINC}/xton.hpp
	${PINC}/variant.hpp
)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/options.cpp
	${PSRC}/logInitializer.cpp
	${PSRC}/variantLoadGrammar.cpp
	${PSRC}/variantSerialization.cpp
	${PSRC}/variant.cpp
	${PSRC}/variantLoad.cpp
	${PSRC}/variantLoadScope.cpp
)
source_group(src FILES ${SRC})

set(SRC_HDR
	${PSRC}/pch.hpp
	${PSRC}/variantDumper.hpp
	${PSRC}/variantLoadScope.hpp
	${PSRC}/variantConvertMatrix.hpp
	${PSRC}/variantImpl.hpp
	${PSRC}/variantLoadGrammar.hpp
)
source_group(src FILES ${SRC_HDR})


set(SRC_PCH
	${PSRC}/pch.cpp
)
source_group(src FILES ${SRC})

add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_HDR} ${SRC_PCH})

create_pch(${PROJECT} ${PSRC}/pch.hpp ${PSRC}/pch.cpp)
use_pch(${PROJECT} ${PCH} ${SRC})
