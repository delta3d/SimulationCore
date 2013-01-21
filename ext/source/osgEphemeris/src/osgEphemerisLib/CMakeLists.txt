
SET(LIB_NAME     ${OSGEPHEMERIS_LIBRARY})

SET(HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../include/osgEphemeris)

SET(LIB_PUBLIC_HEADERS 
  ${HEADER_PATH}/CelestialBodies
  ${HEADER_PATH}/DateTime
  ${HEADER_PATH}/EphemerisData
  ${HEADER_PATH}/EphemerisEngine
  ${HEADER_PATH}/EphemerisModel
  ${HEADER_PATH}/EphemerisUpdateCallback
  ${HEADER_PATH}/GroundPlane
  ${HEADER_PATH}/MoonModel
  ${HEADER_PATH}/Planets
  ${HEADER_PATH}/Shmem
  ${HEADER_PATH}/SkyDome
  ${HEADER_PATH}/Sphere
  ${HEADER_PATH}/StarField)

file(GLOB LIB_SOURCES "*.cpp")

IF (MSVC)
  SET(STATIC_OR_SHARED STATIC)
  ADD_DEFINITIONS(-DOSGEPHEMERIS_LIBRARY_STATIC)
ELSE (MSVC)  
  SET(STATIC_OR_SHARED SHARED)
ENDIF (MSVC)

if (APPLE)
   # It uses this weird macro for OS X support.  I'm not sure why.
   ADD_DEFINITIONS(-D_DARWIN)
endif (APPLE)

  
ADD_LIBRARY(${LIB_NAME} ${STATIC_OR_SHARED}
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

TARGET_LINK_LIBRARIES(${LIB_NAME} ${MATH_LIBRARY})


SET_TARGET_PROPERTIES(${LIB_NAME}
                     PROPERTIES DEFINE_SYMBOL OSGEPHEMERIS_LIBRARY)

INCLUDE(ModuleInstall OPTIONAL)

