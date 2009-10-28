# Locate dtPhysics
# This module defines
# DTPHYSICS_LIBRARY
# DTPHYSICS_EXT_DIR
# DTPHYSICS_FOUND, if false, do not try to link to gdal 
# DTPHYSICS_INCLUDE_DIR, where to find the headers
#
# $DTPHYSICS_DIR is an environment variable that would
# correspond to the ./configure --prefix=$DTPHYSICS
#
# Created by David Guthrie. 

FIND_PATH(DTPHYSICS_DIR include/dtPhysics
    PATHS
    ${CMAKE_SOURCE_DIR}/../dtPhysics
    $ENV{DELTA_ROOT}
    $ENV{DELTA_INC}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControl\\Control\\Session\ Manager\\Environment;DELTA_ROOT]
    /usr/freeware/include
)

FIND_PATH(DTPHYSICS_INCLUDE_DIR dtPhysics/physicsexport.h
    ${DTPHYSICS_DIR}/include
)

if (NOT DTPHYSICS_LIB_DIR)
   FIND_PATH( DTPHYSICS_LIB_DIR NAMES lib
      PATHS
      ${DTPHYSICS_DIR}
      NO_DEFAULT_PATH
   )

   if (DTPHYSICS_LIB_DIR)
      SET(DTPHYSICS_LIB_DIR ${DTPHYSICS_LIB_DIR}/lib CACHE PATH "Library path for dtPhysics" FORCE)
   endif (DTPHYSICS_LIB_DIR)
endif (NOT DTPHYSICS_LIB_DIR)


FIND_PATH( DTPHYSICS_EXT_DIR inc
    ${DTPHYSICS_DIR}/ext
)

SET(DTPHYSICS_LIBRARY dtPhysics)

IF (WIN32)
    SET(DTPHYSICS_LIBRARY_DEBUG dtPhysicsd)
ENDIF (WIN32)

SET(DTPHYSICS_FOUND "NO")
IF(DTPHYSICS_LIBRARY AND DTPHYSICS_INCLUDE_DIR AND DTPHYSICS_LIB_DIR)
    SET(DTPHYSICS_FOUND "YES")
ENDIF(DTPHYSICS_LIBRARY AND DTPHYSICS_INCLUDE_DIR AND DTPHYSICS_LIB_DIR)
