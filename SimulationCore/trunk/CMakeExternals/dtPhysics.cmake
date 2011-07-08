#-----------------------------------------------------------------------------
# dtPhysics
#-----------------------------------------------------------------------------

  # Sanity checks
  IF(DEFINED DTPHYSICS_DIR AND NOT EXISTS ${DTPHYSICS_DIR})
    MESSAGE(FATAL_ERROR "DTPHYSICS_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()

  SET(proj dtPhysics)
  SET(proj_DEPENDENCIES delta3d)
  SET(dtPhysics_DEPENDS ${proj})


  IF(NOT DEFINED DTPHYSICS_DIR)
    set(${proj}_BINARY_MODE BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
      if (SUPER_BUILD_IN_SOURCE)
      set(${proj}_BINARY_MODE BUILD_IN_SOURCE 1)
    endif()
      
    set(${proj}_SOURCE_DIR  )
    if (SUPER_BUILD_SOURCE_AT_TOP_LEVEL)
      set(${proj}_SOURCE_DIR  )
      set(${proj}_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../)
    endif()

    SET(DTPHYSICS_DIR ${SOURCE_DIR})
    SET(DTPHYSICS_INCLUDE_DIR ${DTPHYSICS_DIR}/include)
    if (NOT SUPER_BUILD_IN_SOURCE)
      SET(DTPHYSICS_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build/lib)
    else()
      SET(DTPHYSICS_LIB_DIR ${DTPHYSICS_DIR}/lib)
    endif()
    SET(DTPHYSICS_EXT_DIR ${DTPHYSICS_DIR}/ext)

    ExternalProject_Add(${proj}
      SVN_REPOSITORY https://delta3d-extras.svn.sourceforge.net/svnroot/delta3d-extras/dtPhysics/trunk
      ${${proj}_BINARY_MODE}
      SOURCE_DIR ${${proj}_SOURCE_DIR}${proj}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DBUILD_EXAMPLES:BOOLEAN=${DELTA3D_BUILD_EXAMPLES}
        -DDELTA_DIR:PATH=${DELTA_DIR}
        -DDELTA3D_INCLUDE_DIR:PATH=${DELTA3D_INCLUDE_DIR}
        -DDELTA3D_LIB_DIR:PATH=${DELTA3D_LIB_DIR}
        -DDELTA3D_EXT_DIR:PATH=${DELTA3D_EXT_DIR}
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        -DDTPHYSICS_LIB_DIR:PATH=${DTPHYSICS_LIB_DIR}
      DEPENDS ${proj_DEPENDENCIES}
     )
     ExternalProject_Get_Property(${proj} SOURCE_DIR)

  ELSE()

    MacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
  ENDIF()
  
