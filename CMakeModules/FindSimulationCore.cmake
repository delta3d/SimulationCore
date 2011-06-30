# Locate Simulation Core
# This module defines
# SIMULATIONCORE_FOUND, if false, do not try to link to gdal 
# SIMULATIONCORE_INCLUDE_DIR, where to find the headers
# SIMULATIONCORE_LIB_DIR
# OSGEPHEMERIS_INCLUDE_DIR
#
# $SIMULATIONCORE_DIR is an environment variable that would
# correspond to the ./configure --prefix=$DELTA3D
#
# Created by David Guthrie. 

if (NOT SIMULATIONCORE_DIR)
  SET(SIMULATIONCORE_DIR $ENV{SIMCORE_ROOT})
endif(NOT SIMULATIONCORE_DIR)

IF(SIMULATIONCORE_DIR)
  FILE(TO_CMAKE_PATH ${SIMULATIONCORE_DIR} SIMULATIONCORE_DIR)
ENDIF(SIMULATIONCORE_DIR)

FIND_PATH(SIMULATIONCORE_DIR include/SimCore/Export.h
    HINTS
      ${CMAKE_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/../SimulationCore
      $ENV{SIMCORE_ROOT}
    PATHS
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
)

FIND_FILE(SIMULATIONCORE_CONFIG_FILE SimCoreConfig.cmake
    PATH_SUFFIXES
       superbuild/SimulationCore-build
       build
       Debug
       Release
       share/SimCore
       share/SimulationCore
    HINTS
       ${SIMULATIONCORE_DIR}
    PATHS
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
)

if (SIMULATIONCORE_CONFIG_FILE)
   message (Found the file!!!  ${SIMULATIONCORE_CONFIG_FILE})
   include(${SIMULATIONCORE_CONFIG_FILE})
endif(SIMULATIONCORE_CONFIG_FILE)

find_path(SIMULATIONCORE_INCLUDE_DIR NAMES SimCore/Export.h PATH_SUFFIXES include inc
    HINTS
    ${SIMULATIONCORE_DIR}
)

MARK_AS_ADVANCED(SIMULATIONCORE_INCLUDE_DIR)

find_path(SIMULATIONCORE_LIB_DIR NAMES libSimCore.so libSimCore.dylib SimCore.dll .
    HINTS
       ${SIMULATIONCORE_DIR}
       ${CMAKE_BINARY_DIR}
    PATH_SUFFIXES       
       superbuild/SimulationCore-build/lib
       Build/lib
       Debug/lib
       Release/lib
       lib
)

MARK_AS_ADVANCED(SIMULATIONCORE_LIB_DIR)

find_path(OSGEPHEMERIS_INCLUDE_DIR NAMES osgEphemeris
    HINTS
    ${SIMULATIONCORE_DIR}/ext/source/osgEphemeris/include
)

MARK_AS_ADVANCED(OSGEPHEMERIS_INCLUDE_DIR)

SET(SIMCORE_LIBRARY SimCore)
SET(SIMCORE_WIDGETS_LIBRARY SimCoreWidgets)
SET(SIMCOREHLA_LIBRARY SimCoreHLA)
SET(STEALTHGM_LIBRARY StealthGMApp)
SET(STEALTHQT_LIBRARY StealthQtLib)
SET(OSGEPHEMERIS_LIBRARY osgEphemeris)

IF (WIN32)
SET(SIMCORE_LIBRARY_DEBUG SimCored)
SET(SIMCORE_WIDGETS_LIBRARY_DEBUG SimCoreWidgetsd)
SET(SIMCOREHLA_LIBRARY_DEBUG SimCoreHLAd)
SET(STEALTHGM_LIBRARY_DEBUG StealthGMAppd)
SET(STEALTHQT_LIBRARY_DEBUG StealthQtLibd)
SET(OSGEPHEMERIS_LIBRARY_DEBUG osgEphemerisd)
ENDIF (WIN32)

SET(SIMULATIONCORE_FOUND "NO")
IF(SIMCORE_LIBRARY AND SIMULATIONCORE_INCLUDE_DIR AND SIMULATIONCORE_LIB_DIR)
    SET(SIMULATIONCORE_FOUND "YES")
ENDIF(SIMCORE_LIBRARY AND SIMULATIONCORE_INCLUDE_DIR AND SIMULATIONCORE_LIB_DIR)
