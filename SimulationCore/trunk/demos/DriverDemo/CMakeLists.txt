SET(LIB_NAME   DriverDemo)

SET(HEADER_PATH include)
SET(SOURCE_PATH source)

INCLUDE_DIRECTORIES(${HEADER_PATH} ${RTIS_INCLUDE_DIR})

file(GLOB LIB_PUBLIC_HEADERS 
   "${HEADER_PATH}/*.h"
   )

file(GLOB LIB_SOURCES 
   "${SOURCE_PATH}/*.cpp"
   )

ADD_LIBRARY(DriverDemo SHARED                        
    ${LIB_PUBLIC_HEADERS}                         
    ${LIB_SOURCES}
)

LINK_WITH_VARIABLES(DriverDemo
   SIMCORE_LIBRARY
   SIMCOREHLA_LIBRARY
)


SET_TARGET_PROPERTIES(DriverDemo
                     PROPERTIES DEFINE_SYMBOL DRIVER_DEMO_LIBRARY)

#little hack to get the .dll into /bin and .lib into /lib
IF (MSVC)
  SET_TARGET_PROPERTIES(DriverDemo PROPERTIES PREFIX "/../../bin/")
  SET_TARGET_PROPERTIES(DriverDemo PROPERTIES IMPORT_PREFIX "../")
ENDIF (MSVC)
