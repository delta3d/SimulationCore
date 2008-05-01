# Locate gdal
# This module defines
# SIMULATIONCORE_EXT_DIR
# SIMULATIONCORE_FOUND, if false, do not try to link to gdal 
# SIMULATIONCORE_INCLUDE_DIR, where to find the headers
#
# $SIMULATIONCORE_DIR is an environment variable that would
# correspond to the ./configure --prefix=$DELTA3D
#
# Created by David Guthrie. 

FIND_PATH(SIMULATIONCORE_INCLUDE_DIR SimCore/Export.h
    ${CMAKE_SOURCE_DIR}/include
)

FIND_FILE( SIMULATIONCORE_EXT_DIR NAMES win32 darwin linux2
    PATHS
    ${CMAKE_SOURCE_DIR}/ext/include
)

SET(SIMCORE_LIBRARY SimCore)
SET(SIMCOREHLA_LIBRARY SimCoreHLA)
SET(STEALTHGM_LIBRARY StealthGMApp)
SET(STEALTHQT_LIBRARY StealthQtLib)
SET(OSGEPHEMERIS_LIBRARY osgEphemeris)

IF (WIN32)
SET(SIMCORE_LIBRARY_DEBUG SimCored)
SET(SIMCOREHLA_LIBRARY_DEBUG SimCoreHLAd)
SET(STEALTHGM_LIBRARY_DEBUG StealthGMAppd)
SET(STEALTHQT_LIBRARY_DEBUG StealthQtLibd)
SET(OSGEPHEMERIS_LIBRARY_DEBUG osgEphemerisd)
ENDIF (WIN32)

SET(SIMULATIONCORE_FOUND "NO")
IF(SIMCORE_LIBRARY AND SIMULATIONCORE_INCLUDE_DIR)
    SET(SIMULATIONCORE_FOUND "YES")
ENDIF(SIMCORE_LIBRARY AND SIMULATIONCORE_INCLUDE_DIR)
