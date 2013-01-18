
#-----------------------------------------------------------------------------
# Convenient macro allowing to download a file
#-----------------------------------------------------------------------------

MACRO(downloadFile url dest)
  FILE(DOWNLOAD ${url} ${dest} STATUS status)
  LIST(GET status 0 error_code)
  LIST(GET status 1 error_msg)
  IF(error_code)
    MESSAGE(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  ENDIF()
ENDMACRO()


#-----------------------------------------------------------------------------
# External project settings
#-----------------------------------------------------------------------------

INCLUDE(ExternalProject)

SET(ep_common_args 
  -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
#
#  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
#  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
#  -DCMAKE_C_FLAGS:STRING=${ep_common_C_FLAGS}
#  -DCMAKE_CXX_FLAGS:STRING=${ep_common_CXX_FLAGS}
#  -DCMAKE_PREFIX_PATH:PATH=${CMAKE_CURRENT_SOURCE_DIR}/ext
#  #debug flags
#  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
#  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
#  #release flags
#  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
#  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
#  #relwithdebinfo
#  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
#  -DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_C_FLAGS_RELWITHDEBINFO}
)

#-----------------------------------------------------------------------------
# ExternalProjects 
#-----------------------------------------------------------------------------

SET(external_projects
  Delta3D
  dtPhysics
  )
  
#-----------------------------------------------------------------------------
# Set superbuild boolean args
#-----------------------------------------------------------------------------

OPTION(SUPER_BUILD_IN_SOURCE "Build external projects and SimCore in the source trees" OFF)
OPTION(SUPER_BUILD_SOURCE_AT_TOP_LEVEL "Put the source trees for the dependencies in the same directory as the Simulation Core source" ON)
OPTION(BUILD_BINDINGS "Build delta3d python bindings" OFF)
OPTION(BUILD_3DSMAX_PLUGIN "Build delta3d 3D Studio max plugin" OFF)
OPTION(BUILD_EXAMPLES "Build examples in all the projects (not to be confused with demos)" ON)

OPTION(USE_RTIS "Use RTI-s for the RTI implementation" ON)
OPTION(RTIS_SINGLE_LIBRARY "If using rtis, assume it was compiled into single library, not into multiple" OFF)
OPTION(USE_CERTI "Use CERTI for the RTI implementation" OFF)
OPTION(USE_MAK "Use MAK for the RTI implementation" OFF)
OPTION(USE_PORTICO "Use Portico for the RTI implementation" OFF)
OPTION(USE_HLA_1516e "Use an implementation of RTI 1516e (headers and library names are standardized.)" OFF)

SET(global_cmake_boolean_args
  BUILD_DEMOS
  BUILD_DIS
  BUILD_BINDINGS
  BUILD_EXAMPLES
  BUILD_WITH_PCH
  AUTO_RUN_TESTS
  )

if (RTIS_SINGLE_LIBRARY)
   LIST(APPEND global_cmake_boolean_args RTIS_SINGLE_LIBRARY)
endif(RTIS_SINGLE_LIBRARY)

if (MSVC)
   LIST(APPEND global_cmake_boolean_args BUILD_WITH_MP)
endif()

LIST(APPEND global_cmake_boolean_args RTIS_SINGLE_LIBRARY)
LIST(APPEND global_cmake_boolean_args USE_RTIS)
LIST(APPEND global_cmake_boolean_args USE_CERTI)
LIST(APPEND global_cmake_boolean_args USE_PRTI)
LIST(APPEND global_cmake_boolean_args USE_MAK)
LIST(APPEND global_cmake_boolean_args USE_PORTICO)
LIST(APPEND global_cmake_boolean_args USE_DMSO)

FOREACH(global_cmake_arg ${global_cmake_boolean_args})
  LIST(APPEND ep_common_args -D${global_cmake_arg}:BOOL=${${global_cmake_arg}})
ENDFOREACH()
  
# Include external projects
FOREACH(p ${external_projects})
  INCLUDE(CMakeExternals/${p}.cmake)
ENDFOREACH()

  
#-----------------------------------------------------------------------------
# Create the final variable containing superbuild boolean args
#-----------------------------------------------------------------------------

# this is almost silly to do with only one SimCore only option to pass, but more might get added later.
SET(super_cmake_boolean_args
  BUILD_STEALTH_VIEWER
)

SET(super_boolean_args)
FOREACH(global_cmake_arg ${global_cmake_boolean_args})
  LIST(APPEND global_boolean_args -D${global_cmake_arg}:BOOL=${${global_cmake_arg}})
ENDFOREACH()

SET(superbuild_boolean_args)
FOREACH(super_cmake_arg ${super_cmake_boolean_args})
  LIST(APPEND superbuild_boolean_args -D${super_cmake_arg}:BOOL=${${super_cmake_arg}})
ENDFOREACH()


  SET(proj SimulationCore)
  SET(proj_DEPENDENCIES delta3d dtPhysics)
  SET(SimulationCore_DEPENDS ${proj})

  set(${proj}_BINARY_MODE BINARY_DIR ${CMAKE_BINARY_DIR}/../build)
  if (SUPER_BUILD_IN_SOURCE)
     set(${proj}_BINARY_MODE BUILD_IN_SOURCE 1)
  endif()

  ExternalProject_Add(${proj}
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${ep_common_args}
      ${superbuild_boolean_args}
      -DDELTA_DIR:PATH=${DELTA_DIR}
      -DDELTA3D_INCLUDE_DIR:PATH=${DELTA3D_INCLUDE_DIR}
      -DDELTA3D_LIB_DIR:PATH=${DELTA3D_LIB_DIR}
      -DDELTA3D_EXT_DIR:PATH=${DELTA3D_EXT_DIR}
      -DDTPHYSICS_DIR:PATH=${DTPHYSICS_DIR}
      -DDTPHYSICS_INCLUDE_DIR:PATH=${DTPHYSICS_INCLUDE_DIR}
      -DDTPHYSICS_LIB_DIR:PATH=${DTPHYSICS_LIB_DIR}
      -DDTPHYSICS_EXT_DIR:PATH=${DTPHYSICS_EXT_DIR}
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
     SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
     ${${proj}_BINARY_MODE}
     #BUILD_COMMAND ""
     INSTALL_COMMAND ""
     DEPENDS ${proj_DEPENDENCIES}
   )


#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of the SimCore project itself
#-----------------------------------------------------------------------------

#ADD_CUSTOM_TARGET(SimulationCore-Build
#  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/SimCore-build
#  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SimCore-build
#)

