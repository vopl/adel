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
SOURCE_GROUP(include FILES ${INC})

set(SRC
	${PSRC}/pch.hpp
	${PSRC}/options.cpp
	${PSRC}/logInitializer.cpp
	${PSRC}/variantDumper.hpp
	${PSRC}/variantLoadGrammar.cpp
	${PSRC}/variantLoadScope.hpp
	${PSRC}/variantConvertMatrix.hpp
	${PSRC}/variantImpl.hpp
	${PSRC}/variantLoadGrammar.hpp
	${PSRC}/variantSerialization.cpp
	${PSRC}/variant.cpp
	${PSRC}/variantLoad.cpp
	${PSRC}/variantLoadScope.cpp
)
SOURCE_GROUP(src FILES ${SRC})


add_library(${PROJECT} STATIC ${INC} ${SRC})

set(PCH ${PSRC}/pch.hpp)
create_pch(${PROJECT} ${PCH})
use_pch(${PROJECT} ${PCH} ${SRC})
