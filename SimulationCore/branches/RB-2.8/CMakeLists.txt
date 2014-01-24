CMAKE_MINIMUM_REQUIRED(VERSION 2.6.1 FATAL_ERROR)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW) 
endif(COMMAND cmake_policy)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)


OPTION(SUPER_BUILD "Enables the build to download and build delta3d" OFF)
MARK_AS_ADVANCED(SUPER_BUILD)

if (SUPER_BUILD)
	PROJECT(SimulationCore-SuperBuild)
else ()
	PROJECT(SimulationCore)
endif ()

################################################################################
# User Options
OPTION(BUILD_DIS "Enables the linking of dtDIS for StealthViewer" OFF)
OPTION(BUILD_HLA "Enables the building of HLA related projects (e.g., # SimCoreHLA)" ON)
OPTION(BUILD_DEMOS "Enables building SimCore Demos in the demos directory" ON)
OPTION(AUTO_RUN_TESTS "Automatically runs the unit tests upon successful compile and link" ON)
OPTION(BUILD_WITH_PCH      "Enables use of precomplied headers, experimental" OFF)
MARK_AS_ADVANCED(BUILD_WITH_PCH)

OPTION(BUILD_STEALTH_VIEWER "Build the Stealth Viewer" ON)

# We want to build SONAMES shared librariess
SET(DELTA32_SONAMES TRUE)

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;/usr/local/share/cmake/Modules;${CMAKE_MODULE_PATH}")

# Find OpenGL
FIND_PACKAGE(OpenGL REQUIRED)

# Add macro functions 
include(UtilityMacros)



################################################################################
# 3rd Party Dependency Stuff
# Common to all platforms:

if (SUPER_BUILD)
    SET(QT_USE_QTOPENGL "true")
    FIND_PACKAGE(Qt4 REQUIRED)

   include(${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake)
   return()
endif()

#This is so simcore itself won't get messed up with its own config
set (SIMULATIONCORE_IGNORE_CONFIG_FILE ON)

#Force the lib dir so it will generate the config file correctly without
# getting the possibility of a searching mistake.
set (SIMULATIONCORE_LIB_DIR ${CMAKE_BINARY_DIR}/lib CACHE FORCE "Path to the simulation core libraries" )
FIND_PACKAGE(SimulationCore REQUIRED)

FIND_PACKAGE(Delta3D REQUIRED)
IF (DELTA3D_FOUND)
   #look for module loaders in delta3d CMakeModules
   SET(CMAKE_MODULE_PATH "${DELTA_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

   include(PlatformConfiguration)

   INCLUDE(PCHMacros)

   IF (BUILD_WITH_PCH)
     ADD_DEFINITIONS(-DSIMCORE_USE_PCH)
   ENDIF (BUILD_WITH_PCH)
   
   IF(NOT WIN32)
      FIND_PACKAGE(Curses REQUIRED)
   ENDIF(NOT WIN32)

   FIND_PACKAGE(OpenAL REQUIRED)

   FIND_PACKAGE(OpenThreads REQUIRED)
   FIND_PACKAGE(OSG REQUIRED)

   FIND_PACKAGE(Cal3d REQUIRED)
   FIND_PACKAGE(CEGUI REQUIRED)
   FIND_PACKAGE(CPPUNIT REQUIRED)
   FIND_PACKAGE(ODE REQUIRED)
   FIND_PACKAGE(XercesC REQUIRED)
   FIND_PACKAGE(HAWKNL REQUIRED)
   FIND_PACKAGE(GNE REQUIRED)
   FIND_PACKAGE(Pal REQUIRED)
   INCLUDE_DIRECTORIES( ${PAL_INCLUDE_DIR} )
   
   FIND_PACKAGE(Boost)
   

   IF (NOT WIN32 AND NOT APPLE)
     FIND_PACKAGE(UUID REQUIRED) 
   ENDIF (NOT WIN32 AND NOT APPLE)

   #FIND_PACKAGE(GDAL)
   #FIND_PACKAGE(DIS)

   IF (BUILD_DIS)
     FIND_PACKAGE(dtDIS REQUIRED)
   ENDIF (BUILD_DIS)

   if (BUILD_STEALTH_VIEWER)
      SET(QT_USE_QTOPENGL "true")
      FIND_PACKAGE(Qt4 REQUIRED)
   endif (BUILD_STEALTH_VIEWER)

ELSE ()
   return()
ENDIF ()


# Make the headers visible to everything
INCLUDE_DIRECTORIES(
    ${DELTA3D_INCLUDE_DIR}
    ${SIMULATIONCORE_INCLUDE_DIR}
    ${OSGEPHEMERIS_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${OPENTHREADS_INCLUDE_DIR}
    ${OSG_INCLUDE_DIR}
    ${CAL3D_INCLUDE_DIR}
    ${CEGUI_INCLUDE_DIR}
    ${ODE_INCLUDE_DIR}
    ${GNE_INCLUDE_DIR}
    ${HAWKNL_INCLUDE_DIR}
    ${XERCES_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${PYTHON_INCLUDE_PATH}
    ${PAL_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
)

IF (APPLE)
   INCLUDE_DIRECTORIES(/usr/include/malloc)
ENDIF (APPLE)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

LINK_DIRECTORIES( ${DELTA3D_LIB_DIR} )

IF (DELTA3D_EXT_DIR)
  LINK_DIRECTORIES( ${DELTA3D_EXT_DIR}/lib )
ENDIF (DELTA3D_EXT_DIR)
################################################################################
# Create bin and lib directories if required

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib )
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")


################################################################################
# Installation stuff

IF(UNIX AND NOT WIN32 AND NOT APPLE)
  IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET(LIB_POSTFIX "64" CACHE STRING "suffix for 32/64 dir placement")
      MARK_AS_ADVANCED(LIB_POSTFIX)
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
ENDIF(UNIX AND NOT WIN32 AND NOT APPLE)
IF(NOT DEFINED LIB_POSTFIX)
    SET(LIB_POSTFIX "")
ENDIF(NOT DEFINED LIB_POSTFIX)
    
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})

SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})

SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})

SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})


SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${OUTPUT_LIBDIR} CACHE PATH "build directory")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${OUTPUT_BINDIR} CACHE PATH "build directory")
IF(MSVC_IDE)
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${OUTPUT_BINDIR} CACHE PATH "build directory")
ELSE(MSVC_IDE)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR} CACHE PATH "build directory")
ENDIF(MSVC_IDE)


# For each configuration (Debug, Release, MinSizeRel... and/or anything the
# user chooses)
FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})
  # Go uppercase (DEBUG, RELEASE...)
  STRING(TOUPPER "${CONF}" CONF)
  SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
  SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
  IF(MSVC_IDE)
    SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
  ELSE()
    SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
  ENDIF()
ENDFOREACH()



# Expose CMAKE_INCLUDE_PATH and CMAKE_LIBARY_PATH to the GUI so users
# may set these values without needing to manipulate the environment.
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
# We are proposing that a new variable called CMAKE_PREFIX_PATH be introduced
# to CMake to compliment CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH.
# A formal feature request has been submited to CMake, Bug #4947.
# It is intended for those users who have common prefixes for their INCLUDE
# and LIBRARY locations. So if users have headers in /usr/local/include
# and libraries in /usr/local/lib, the common prefix is /usr/local.
# It should also cover the case where headers and libraries are
# in the same directory.
# Our proposal expects that FIND_* commands will automatically search for
# CMAKE_PREFIX_PATH right after CMAKE_INCLUDE_PATH or CMAKE_LIBRARY_PATH.
# Obviously, since CMake does not currently support this, we must write
# our Find*.cmake modules to explicitly support this. Otherwise, this variable
# will have no impact.
# This is unofficial so this may be removed or changed at anytime.
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE STRING "(EXPERIMENTAL) You may add additional search paths here. Use ; to separate multiple paths.")


ADD_SUBDIRECTORY(ext/source)
ADD_SUBDIRECTORY(source)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(utilities)

if (BUILD_DEMOS)
   ADD_SUBDIRECTORY(demos)
endif (BUILD_DEMOS)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/SimCoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/SimCoreConfig.cmake"
  IMMEDIATE @ONLY)


#-----------------------------------------------------------------------------
### uninstall target
#-----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

