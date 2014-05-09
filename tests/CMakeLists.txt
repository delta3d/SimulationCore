
SET(PROG_NAME     allTests)

SET(DIRS SimCore)

if (BUILD_HLA)
   list(APPEND DIRS SimCoreHLA)
   if (BUILD_STEALTHVIEWER)
      list(APPEND DIRS StealthViewer)
      INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR}) 
   endif()
endif()


INCLUDE_DIRECTORIES(include) 

ADD_DEFINITIONS(-DTEST_ROOT="${CMAKE_SOURCE_DIR}")

file(GLOB TEMP_SOURCES "*.cpp" "*.h" "include/*.h")
SET(ALL_SOURCES ${TEMP_SOURCES})

FOREACH(varname ${DIRS}) 
  file(GLOB TEMP_SOURCES "${varname}/*.cpp" "${varname}/*.h")
  SET(ALL_SOURCES ${ALL_SOURCES} ${TEMP_SOURCES})
ENDFOREACH(varname)

list(REMOVE_ITEM ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/precomp.cpp)

if (MSVC)
   #building it dynamic now
   #ADD_DEFINITIONS(-DOSGEPHEMERIS_LIBRARY_STATIC)
endif (MSVC)

ADD_PRECOMPILED_HEADER(${PROG_NAME} prefix/SimCorePrefix.h prefix/SimCorePrefix.cpp ALL_SOURCES)

ADD_EXECUTABLE(${PROG_NAME}
    ${ALL_SOURCES}
)

TARGET_LINK_LIBRARIES(${PROG_NAME} 
   ${SIMCORE_LIBRARY}
   ${OSGEPHEMERIS_LIBRARY}
)

if (BUILD_HLA)
   TARGET_LINK_LIBRARIES(${PROG_NAME}
      ${SIMCOREHLA_LIBRARY}
   )
endif()

LINK_WITH_VARIABLES(${PROG_NAME}
                    CPPUNIT_LIBRARY)

if (BUILD_STEALTHVIEWER)
   TARGET_LINK_LIBRARIES(${PROG_NAME} 
      ${STEALTHGM_LIBRARY}
      ${STEALTHQT_LIBRARY}
   )


   LINK_WITH_VARIABLES(${PROG_NAME}
                    QT_QTCORE_LIBRARY
                    QT_QTGUI_LIBRARY
                    QT_QTOPENGL_LIBRARY
                    DTQT_LIBRARY)
endif ()


INCLUDE(ModuleInstall OPTIONAL)

IF (MSVC)
  SET_TARGET_PROPERTIES(${PROG_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ENDIF (MSVC)

#This will automatically run the unit test executable upon successfull compile/link
IF (AUTO_RUN_TESTS)
  IF (MSVC)
    ADD_CUSTOM_COMMAND(TARGET ${PROG_NAME}
                          POST_BUILD
                          COMMAND $(TargetPath)
                          #WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                          COMMENT "Run the unit tests")  
  ELSE (MSVC)
     ADD_CUSTOM_COMMAND(TARGET ${PROG_NAME}
                          POST_BUILD
                          COMMAND ${CMAKE_BINARY_DIR}/bin/allTests
                          #WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                          COMMENT Run the unit tests)
  ENDIF (MSVC)
ENDIF (AUTO_RUN_TESTS)
