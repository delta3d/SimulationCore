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

FIND_PATH(SIMULATIONCORE_INCLUDE_DIR dtCore/dt.h
    ${SIMULATIONCORE_DIR}/include
    $ENV{SIMCORE_ROOT}/inc
    $ENV{SIMCORE_INC}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlFIND_DT_LIBRARY\\Control\\Session\ Manager\\Environment;DELTA_ROOT]/inc
    /usr/freeware/include
)

FIND_PATH( SIMULATIONCORE_EXT_DIR inc
    ${SIMULATIONCORE_DIR}/ext
    $ENV{SIMCORE_ROOT}/ext
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
