CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1 FATAL_ERROR)

cmake_policy(VERSION 2.8.1)

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
OPTION(BUILD_UTILITIES "Enables building SimCore utility programs in the in the utilities directory" ON)
OPTION(AUTO_RUN_TESTS "Automatically runs the unit tests upon successful compile and link" ON)
OPTION(BUILD_WITH_PCH      "Enables use of precomplied headers, experimental" OFF)
MARK_AS_ADVANCED(BUILD_WITH_PCH)

OPTION(REGENERATE_VERSION_HEADER      "Uses the SVN version to update the version header.  This can only be used for direct check-outs." OFF)
MARK_AS_ADVANCED(REGENERATE_VERSION_HEADER)

OPTION(BUILD_STEALTH_VIEWER "Build the Stealth Viewer" ON)

# We want to build SONAMES shared librariess
SET(DELTA32_SONAMES TRUE)

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;/usr/local/share/cmake/Modules;${CMAKE_MODULE_PATH}")

# Find OpenGL
FIND_PACKAGE(OpenGL REQUIRED)

if (REGENERATE_VERSION_HEADER)
   FIND_PACKAGE(Subversion REQUIRED)
endif()

# Add macro functions 
include(UtilityMacros)



################################################################################
# 3rd Party Dependency Stuff
# Common to all platforms:

#This is so simcore itself won't get messed up with its own config
set (SIMULATIONCORE_IGNORE_CONFIG_FILE ON)

#Force the lib dir so it will generate the config file correctly without
# getting the possibility of a searching mistake.
set (SIMULATIONCORE_LIB_DIR ${CMAKE_BINARY_DIR}/lib CACHE FORCE "Path to the simulation core libraries" )
FIND_PACKAGE(SimulationCore REQUIRED)

FIND_PACKAGE(Delta3D COMPONENTS dtUtil dtCore dtAI dtAudio dtAnim dtPhysics dtRender dtABC dtGame dtGUI dtActors REQUIRED)
if (BUILD_HLA)
   FIND_PACKAGE(Delta3D COMPONENTS dtHLAGM REQUIRED )
endif()
if (BUILD_DIS)
   FIND_PACKAGE(Delta3D COMPONENTS dtDIS REQUIRED )
endif()
if (BUILD_STEALTH_VIEWER)
   FIND_PACKAGE(Delta3D COMPONENTS dtQt REQUIRED )
   if (NOT BUILD_HLA)
      MESSAGE(FATAL_ERROR "You must enable BUILD_HLA to build the stealth viewer")
   endif()
endif()
if (BUILD_DEMOS)
   FIND_PACKAGE(Delta3D COMPONENTS dtInputPLIB dtNetGM REQUIRED )
   if (NOT BUILD_HLA)
      MESSAGE(FATAL_ERROR "You must enable BUILD_HLA to build the demos")
   endif()
endif()

IF (DELTA3D_FOUND)

   include(PlatformConfiguration)
   INCLUDE(PCHMacros)   
   INCLUDE(SimCoreDataInstall)

   IF (BUILD_WITH_PCH)
     ADD_DEFINITIONS(-DSIMCORE_USE_PCH)
   ENDIF (BUILD_WITH_PCH)
   
   IF(NOT WIN32)
      FIND_PACKAGE(Curses REQUIRED)
   ENDIF(NOT WIN32)

   FIND_PACKAGE(CPPUNIT REQUIRED)

   INCLUDE_DIRECTORIES( ${PAL_INCLUDE_DIR} )

   IF (NOT WIN32 AND NOT APPLE)
     FIND_PACKAGE(UUID REQUIRED) 
   ENDIF (NOT WIN32 AND NOT APPLE)

   if (BUILD_STEALTH_VIEWER)
      FIND_PACKAGE(Qt5 REQUIRED)
   endif (BUILD_STEALTH_VIEWER)

ELSE ()
   return()
ENDIF ()


# Make the headers visible to everything
INCLUDE_DIRECTORIES(
    ${DELTA3D_INCLUDE_DIR}
    ${SIMULATIONCORE_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${OPENTHREADS_INCLUDE_DIR}
    ${OSG_INCLUDE_DIR}
    ${CAL3D_INCLUDE_DIR}
    ${CEGUI_INCLUDE_DIR}
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

if (APPLE)
   SET(OUTPUT_FRAMEWORK_DIR ${PROJECT_BINARY_DIR}/Frameworks)
   MAKE_DIRECTORY(${OUTPUT_FRAMEWORK_DIR})
   SET(CMAKE_FRAMEWORK_OUTPUT_DIRECTORY ${OUTPUT_FRAMEWORK_DIR} CACHE PATH "Framework Build Directory")
   message("FRAMEWORK OUTPUT " ${CMAKE_FRAMEWORK_OUTPUT_DIRECTORY})
endif()

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


ADD_SUBDIRECTORY(source)
ADD_SUBDIRECTORY(tests)

if (BUILD_UTILITIES)	
   ADD_SUBDIRECTORY(utilities)
endif()

if (BUILD_DEMOS)
   ADD_SUBDIRECTORY(demos)
endif ()

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

