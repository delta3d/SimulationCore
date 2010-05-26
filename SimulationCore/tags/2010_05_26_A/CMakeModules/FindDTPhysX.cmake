# Locate dtPhysX
# This module defines
# DTPHYSX_EXT_DIR
# DTPHYSX_FOUND, if false, do not try to link to gdal 
# DTPHYSX_INCLUDE_DIR, where to find the headers
#
# $DTPHYSX_DIR is an environment variable that would
# correspond to the ./configure --prefix=$DTPHYSX
#
# Created by David Guthrie. 

SET(DTPHYSX_DIR $ENV{DTPHYSX_ROOT})
IF(DTPHYSX_DIR)
  FILE(TO_CMAKE_PATH ${DTPHYSX_DIR} DTPHYSX_DIR)
ENDIF(DTPHYSX_DIR)

FIND_PATH(DTPHYSX_DIR include/NxAgeiaWorldComponent.h
    PATHS
      ${CMAKE_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/../dtPhysX
      ${CMAKE_SOURCE_DIR}/../dtAgeia
      ${CMAKE_SOURCE_DIR}/../dtAgeiaPhysX
    $ENV{DTPHYSX_ROOT}
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
)


FIND_PATH(DTPHYSX_INCLUDE_DIR NxAgeiaWorldComponent.h PATH_SUFFIXES include
    PATHS
    ${DTPHYSX_DIR}
)

if (NOT DTPHYSX_LIB_DIR)
   FIND_PATH( DTPHYSX_LIB_DIR NAMES lib
      PATHS
      ${DTPHYSX_DIR}
      NO_DEFAULT_PATH
   )

   if (DTPHYSX_LIB_DIR)
      SET(DTPHYSX_LIB_DIR ${DTPHYSX_LIB_DIR}/lib CACHE PATH "Library path for dtPhysX" FORCE)
   endif (DTPHYSX_LIB_DIR)
endif (NOT DTPHYSX_LIB_DIR)

MACRO(FIND_DT_LIBRARY MYLIBRARY MYLIBRARYNAMES)

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAMES} PATH_SUFFIXES lib
        PATHS
        ${DTPHYSX_DIR}
    )

ENDMACRO(FIND_DT_LIBRARY LIBRARY LIBRARYNAMES)

SET(DTPHYSX_LIBRARY dtPhysX)

IF (WIN32)
   SET(DTPHYSX_LIBRARY_DEBUG dtPhysXd)
ENDIF (WIN32)

SET(DTPHYSX_FOUND "NO")
IF(DTPHYSX_LIBRARY AND DTPHYSX_INCLUDE_DIR AND DTPHYSX_LIB_DIR)
    SET(DTPHYSX_FOUND "YES")
ENDIF(DTPHYSX_LIBRARY AND DTPHYSX_INCLUDE_DIR AND DTPHYSX_LIB_DIR)
