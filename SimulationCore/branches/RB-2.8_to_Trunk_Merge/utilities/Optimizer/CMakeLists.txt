
SET(APP_NAME     Optimizer)

SET(HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR})
SET(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB PROG_HEADERS 
   "${HEADER_PATH}/*.h"
   )

file(GLOB PROG_SOURCES 
   "${SOURCE_PATH}/*.cpp"
   )

ADD_EXECUTABLE(${APP_NAME}
    ${PROG_HEADERS}
    ${PROG_SOURCES}
)

LINK_WITH_VARIABLES(${APP_NAME}
                                   DTUTIL_LIBRARY
                                   OSG_LIBRARY
                                   OSGGA_LIBRARY
                                   OSGVIEWER_LIBRARY
                                   OSGUTIL_LIBRARY
                                   OSGFX_LIBRARY
                                   OSGSIM_LIBRARY
                                   OSGPARTICLE_LIBRARY
                                   GDAL_LIBRARY 
                                   OSGDB_LIBRARY
                                   OPENTHREADS_LIBRARY
                                 )
                    
INCLUDE(ModuleInstall OPTIONAL)


IF (MSVC)
  SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ENDIF (MSVC)
 