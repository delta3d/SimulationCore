SET(LIB_NAME   ${SIMCORE_LIBRARY})

SET(HEADER_PATH ${SimulationCore_SOURCE_DIR}/include/${LIB_NAME})
SET(SOURCE_PATH ${SimulationCore_SOURCE_DIR}/source/${LIB_NAME})

file(GLOB LIB_PUBLIC_HEADERS 
   "${HEADER_PATH}/*.h"
   "${HEADER_PATH}/Actors/*.h"
   "${HEADER_PATH}/Tools/*.h"
   "${HEADER_PATH}/Components/*.h"
   )
file(GLOB LIB_SOURCES 
   "${SOURCE_PATH}/*.cpp"
   "${SOURCE_PATH}/Actors/*.cpp"
   "${SOURCE_PATH}/Tools/*.cpp"
   "${SOURCE_PATH}/Components/*.cpp")

list(REMOVE_ITEM LIB_SOURCES ${SOURCE_PATH}/precomp.cpp)

ADD_LIBRARY(${LIB_NAME} SHARED                        
    ${LIB_PUBLIC_HEADERS}                         
    ${LIB_SOURCES}
)

LINK_WITH_VARIABLES(${LIB_NAME}
											  DTUTIL_LIBRARY
											  DTDAL_LIBRARY
											  DTCORE_LIBRARY
											  DTANIM_LIBRARY
											  DTAI_LIBRARY
											  DTABC_LIBRARY
											  DTGAME_LIBRARY
											  DTGUI_LIBRARY
											  DTAUDIO_LIBRARY
											  DTACTORS_LIBRARY
											  OSG_LIBRARY
											  OSGGA_LIBRARY
											  OSGVIEWER_LIBRARY
											  OSGUTIL_LIBRARY
											  OSGFX_LIBRARY
											  OSGSIM_LIBRARY
											  OSGPARTICLE_LIBRARY
											  OSGEPHEMERIS_LIBRARY
											  OSGTEXT_LIBRARY
											  GDAL_LIBRARY
											  CAL3D_LIBRARY
											  XERCES_LIBRARY
											  OSGDB_LIBRARY
											  OPENTHREADS_LIBRARY
											  CEGUI_LIBRARY
											  CEGUIOPENGL_LIBRARY
											  DTPHYSX_LIBRARY
											  )

TARGET_LINK_LIBRARIES(${LIB_NAME} 
   ${OPENGL_LIBRARY}
   ${AGL_LIBRARY}
   ${PHYSX_LIBRARIES}
)


SET_TARGET_PROPERTIES(${LIB_NAME}
                     PROPERTIES DEFINE_SYMBOL SIMCORE_LIBRARY)

#little hack to get the .dll into /bin and .lib into /lib
IF (MSVC)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "/../../bin/")
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../")
ENDIF (MSVC)
