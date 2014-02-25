
SET(LIB_NAME     ${OSGEPHEMERIS_LIBRARY})

SET(HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../include/osgEphemeris)

SET(LIB_PUBLIC_HEADERS 
  ${HEADER_PATH}/CelestialBodies.h
  ${HEADER_PATH}/DateTime.h
  ${HEADER_PATH}/EphemerisData.h
  ${HEADER_PATH}/EphemerisEngine.h
  ${HEADER_PATH}/EphemerisModel.h
  ${HEADER_PATH}/EphemerisUpdateCallback.h
  ${HEADER_PATH}/GroundPlane.h
  ${HEADER_PATH}/MoonModel.h
  ${HEADER_PATH}/Planets.h
  ${HEADER_PATH}/Shmem.h
  ${HEADER_PATH}/SkyDome.h
  ${HEADER_PATH}/Sphere.h
  ${HEADER_PATH}/StarField.h)

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

