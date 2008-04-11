
SET(PROG_NAME     allTests)

IF (BUILD_HLA)
  SET(DIRS SimCore SimCoreHLA StealthViewer)
  INCLUDE_DIRECTORIES(${RTIS_INCLUDE_DIR}) 
ELSE (BUILD_HLA)
  SET(DIRS SimCore)
ENDIF (BUILD_HLA)

INCLUDE_DIRECTORIES(include) 

file(GLOB TEMP_SOURCES "*.cpp" "*.h" "include/*.h")
SET(ALL_SOURCES ${TEMP_SOURCES})

FOREACH(varname ${DIRS}) 
  file(GLOB TEMP_SOURCES "${varname}/*.cpp" "${varname}/*.h")
  SET(ALL_SOURCES ${ALL_SOURCES} ${TEMP_SOURCES})
ENDFOREACH(varname)

list(REMOVE_ITEM ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/precomp.cpp)

ADD_EXECUTABLE(${PROG_NAME}
    ${ALL_SOURCES}
)

LINK_WITH_VARIABLES(${PROG_NAME}
                    SIMCORE_LIBRARY
    					  CPPUNIT_LIBRARY)                                       

IF (BUILD_HLA)
    LINK_WITH_VARIABLES(${PROG_NAME}
                    SIMCOREHLA_LIBRARY
                    STEALTHGM_LIBRARY
                    STEALTHQT_LIBRARY
                    QT_QTCORE_LIBRARY
                    QT_QTGUI_LIBRARY
                    QT_QTOPENGL_LIBRARY)                                       
ENDIF (BUILD_HLA)                                              

INCLUDE(ModuleInstall OPTIONAL)

#We do want tests in bin.
SET_TARGET_PROPERTIES(${PROG_NAME} PROPERTIES PREFIX "../bin/")
