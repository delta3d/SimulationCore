
SET(LIB_NAME     ${OSGEPHEMERIS_LIBRARY})

SET(HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
file(GLOB LIB_PUBLIC_HEADERS "${HEADER_PATH}/*.h")
file(GLOB LIB_SOURCES "*.cpp")

ADD_LIBRARY(${LIB_NAME} SHARED                        
    ${LIB_PUBLIC_HEADERS}                         
    ${LIB_SOURCES}
)

LINK_WITH_VARIABLES(${LIB_NAME}
                                   OSGUTIL_LIBRARY
                                   OSG_LIBRARY
                                   OSGDB_LIBRARY
                                   OSGTEXT_LIBRARY
											  OSGSIM_LIBRARY
											  OPENTHREADS_LIBRARY)

TARGET_LINK_LIBRARIES(${MATH_LIBRARY})

IF (WIN32)
#TARGET_LINK_LIBRARIES(${LIB_NAME} wsock32)
ENDIF (WIN32)

SET_TARGET_PROPERTIES(${LIB_NAME}
                     PROPERTIES DEFINE_SYMBOL OSGEPHEMERIS_LIBRARY)

INCLUDE(ModuleInstall OPTIONAL)

#little hack to get the .dll into /bin and .lib into /lib
IF (MSVC)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "../../../../../bin/")
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../../../../")
ENDIF (MSVC)
