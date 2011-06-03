#-----------------------------------------------------------------------------
# dtPhysics
#-----------------------------------------------------------------------------

  # Sanity checks
  IF(DEFINED DTPHYSICS_DIR AND NOT EXISTS ${DTPHYSICS_DIR})
    MESSAGE(FATAL_ERROR "DTPHYSICS_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()

  SET(proj dtPhysics)
  SET(proj_DEPENDENCIES Delta3D)
  SET(dtPhysics_DEPENDS ${proj})

  IF(NOT DEFINED DTPHYSICS_DIR)
    ExternalProject_Add(${proj}
      SVN_REPOSITORY https://delta3d-extras.svn.sourceforge.net/svnroot/delta3d-extras/dtPhysics/branches/REL-2.7.0
      BINARY_DIR ${proj}-build
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DBUILD_EXAMPLES:BOOLEAN=${DELTA3D_BUILD_EXAMPLES}
        -DDELTA_DIR:PATH=${DELTA_DIR}
        -DDELTA3D_INCLUDE_DIR:PATH=${DELTA3D_INCLUDE_DIR}
        -DDELTA3D_LIB_DIR:PATH=${DELTA3D_LIB_DIR}
        -DDELTA3D_EXT_DIR:PATH=${DELTA3D_EXT_DIR}
      DEPENDS ${proj_DEPENDENCIES}
     )
     ExternalProject_Get_Property(${proj} SOURCE_DIR)

     SET(DTPHYSICS_DIR ${SOURCE_DIR})
     SET(DTPHYSICS_INCLUDE_DIR ${DTPHYSICS_DIR}/include)
     SET(DTPHYSICS_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build/lib)
     SET(DTPHYSICS_EXT_DIR ${DTPHYSICS_DIR}/ext)
  ELSE()

    MacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
  ENDIF()
  
