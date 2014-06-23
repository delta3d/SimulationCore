SET(APP_NAME   LightCompiler)

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
                                   DTCORE_LIBRARY
                                   DTGAME_LIBRARY
                                   DTABC_LIBRARY
                                   DTAUDIO_LIBRARY
                                   DTPHYSICS_LIBRARY
                                   OSG_LIBRARY
                                   OSGUTIL_LIBRARY
                                   OSGTEXT_LIBRARY
                                   XERCES_LIBRARY
                                   OSGDB_LIBRARY
                                   OPENTHREADS_LIBRARY)

TARGET_LINK_LIBRARIES(${APP_NAME} 
   ${OPENGL_LIBRARY}
   ${AGL_LIBRARY}
)

INCLUDE(ProgramInstall OPTIONAL)
