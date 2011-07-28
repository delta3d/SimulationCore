#-----------------------------------------------------------------------------
# Delta3D
#-----------------------------------------------------------------------------

  # Sanity checks
  IF(DEFINED DELTA_DIR AND NOT EXISTS ${DELTA_DIR})
    MESSAGE(FATAL_ERROR "DELTA_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()

  SET(proj delta3d)
  SET(proj_DEPENDENCIES )
  SET(Delta3D_DEPENDS ${proj})

  IF(NOT DEFINED DELTA_DIR)
    set(${proj}_BINARY_MODE BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
    if (SUPER_BUILD_IN_SOURCE)
      set(${proj}_BINARY_MODE BUILD_IN_SOURCE 1)
    endif()

    set(${proj}_SOURCE_DIR  ${CMAKE_BINARY_DIR})
    if (SUPER_BUILD_SOURCE_AT_TOP_LEVEL)
      set(${proj}_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
    endif()

     
    SET(DELTA_DIR ${${proj}_SOURCE_DIR}/${proj})
    SET(DELTA3D_INCLUDE_DIR ${DELTA_DIR}/inc)
    if (NOT SUPER_BUILD_IN_SOURCE)
      SET(DELTA3D_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build/lib)
    else()
      SET(DELTA3D_LIB_DIR ${DELTA_DIR}/lib)
    endif()
     
    SET(DELTA3D_EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

    ExternalProject_Add(${proj}
      SVN_REPOSITORY https://delta3d.svn.sourceforge.net/svnroot/delta3d/trunk/delta3d
      ${${proj}_BINARY_MODE}
      SOURCE_DIR ${${proj}_SOURCE_DIR}/${proj}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        #Set to on so stage will be built.  Could keep off of BUILD_STEALTH_VIEWER
        -DBUILD_WITH_QT:BOOL=ON
        -DBUILD_NET:BOOL=ON
        -DBUILD_TERRAIN:BOOL=OFF
        -DBUILD_EXAMPLES=${BUILD_EXAMPLES}
        -DBUILD_3DSMAX_PLUGIN=${BUILD_3DSMAX_PLUGIN}
        -DAUTO_RUN_TESTS:BOOL=${AUTO_RUN_TESTS}
        -DDELTA3D_LIB_DIR:PATH=${DELTA3D_LIB_DIR}
        -DDELTA3D_EXT_DIR:PATH=${CMAKE_SOURCE_DIR}/ext
      DEPENDS ${proj_DEPENDENCIES}
     )
     
  ELSE()

    MacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
  ENDIF()
  
