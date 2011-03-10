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

FIND_PATH( DTPHYSICS_LIB_DIR NAMES libdtPhysics.so libdtPhysics.dylib dtPhysics.dll .
    PATHS
       ${DTPHYSICS_DIR}
    PATH_SUFFIXES
       Build/lib
       Debug/lib
       Release/lib
       lib
    NO_DEFAULT_PATH
)

FIND_PATH( DTPHYSICS_EXT_DIR NAMES inc include
    PATHS
    ${DTPHYSICS_DIR}/ext
    NO_DEFAULT_PATH
)

SET(DTPHYSICS_LIBRARY dtPhysics)

IF (WIN32)
    SET(DTPHYSICS_LIBRARY_DEBUG dtPhysicsd)
ENDIF (WIN32)

SET(DTPHYSICS_FOUND "NO")
IF(DTPHYSICS_LIBRARY AND DTPHYSICS_INCLUDE_DIR AND DTPHYSICS_LIB_DIR)
    SET(DTPHYSICS_FOUND "YES")
ENDIF(DTPHYSICS_LIBRARY AND DTPHYSICS_INCLUDE_DIR AND DTPHYSICS_LIB_DIR)

# handle the QUIETLY and REQUIRED arguments and set DELTA3D_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DTPhysics DEFAULT_MSG DTPHYSICS_INCLUDE_DIR DTPHYSICS_LIBRARY DTPHYSICS_LIB_DIR)