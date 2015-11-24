SET(LIB_NAME ${STEALTHQT_LIBRARY})

SET(HEADER_PATH ${SimulationCore_SOURCE_DIR}/include/StealthViewer/Qt)
SET(SOURCE_PATH ${SimulationCore_SOURCE_DIR}/source/StealthViewer/Qt)
SET(UI_PATH ${SimulationCore_SOURCE_DIR}/source/StealthViewer/ui)

file(GLOB UI_FILES "${UI_PATH}/*.ui")
file(GLOB LIB_PUBLIC_HEADERS "${HEADER_PATH}/*.h")
file(GLOB LIB_SOURCES "*.cpp")

list(REMOVE_ITEM LIB_SOURCES ${SOURCE_PATH}/precomp.cpp)

qt5_wrap_cpp(MOC_SOURCES ${LIB_PUBLIC_HEADERS})
qt5_wrap_ui(UIC_HEADERS ${UI_FILES})

#The ui files are output here, so it must be included.
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDE_DIRS})

## had to disable because of an internal compiler error on VS 2010 :-/
#ADD_PRECOMPILED_HEADER(${LIB_NAME} prefix/StealthQtPrefix.h prefix/StealthQtPrefix.cpp LIB_SOURCES)

ADD_LIBRARY(${LIB_NAME} STATIC
    ${LIB_PUBLIC_HEADERS}
    ${UIC_HEADERS}
    ${LIB_SOURCES}
    ${MOC_SOURCES}
)
    
INCLUDE(ModuleInstall OPTIONAL)

IF (MSVC)
  SET_TARGET_PROPERTIES(${PROG_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ENDIF (MSVC)

IF (BUILD_DIS)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_DEFINITIONS DIS_CONNECTIONS_AVAILABLE=1)
ENDIF (BUILD_DIS)
