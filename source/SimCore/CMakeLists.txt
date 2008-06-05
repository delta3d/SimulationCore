SET(LIB_NAME   ${SIMCORE_LIBRARY})

SET(HEADER_PATH ${SimulationCore_SOURCE_DIR}/include/${LIB_NAME})
SET(SOURCE_PATH ${SimulationCore_SOURCE_DIR}/source/${LIB_NAME})

file(GLOB LIB_PUBLIC_HEADERS_1 
   "${HEADER_PATH}/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_2 
   "${HEADER_PATH}/Actors/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_3 
   "${HEADER_PATH}/Tools/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_4 
   "${HEADER_PATH}/Components/*.h"
   )
SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS_1} ${LIB_PUBLIC_HEADERS_2} ${LIB_PUBLIC_HEADERS_3} ${LIB_PUBLIC_HEADERS_4})

file(GLOB LIB_SOURCES_1 
   "${SOURCE_PATH}/*.cpp"
   )
file(GLOB LIB_SOURCES_2 
   "${SOURCE_PATH}/Actors/*.cpp"
   )
file(GLOB LIB_SOURCES_3 
   "${SOURCE_PATH}/Tools/*.cpp"
   )
file(GLOB LIB_SOURCES_4 
   "${SOURCE_PATH}/Components/*.cpp"
   )
SET(LIB_SOURCES ${LIB_SOURCES_1} ${LIB_SOURCES_2} ${LIB_SOURCES_3} ${LIB_SOURCES_4})


list(REMOVE_ITEM LIB_SOURCES ${SOURCE_PATH}/precomp.cpp)

ADD_LIBRARY(${LIB_NAME} SHARED                        
    ${LIB_PUBLIC_HEADERS}                         
    ${LIB_SOURCES}
)

source_group("Header Files\\ActorsH" FILES ${LIB_PUBLIC_HEADERS_2})
source_group("Header Files\\ToolsH" FILES ${LIB_PUBLIC_HEADERS_3})
source_group("Header Files\\ComponentsH" FILES ${LIB_PUBLIC_HEADERS_4})

source_group("Source Files\\Actors" FILES ${LIB_SOURCES_2})
source_group("Source Files\\Tools" FILES ${LIB_SOURCES_3})
source_group("Source Files\\Components" FILES ${LIB_SOURCES_4})

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
                                                                                          DTPHYSICS_LIBRARY
                                                                                          PAL_LIBRARY
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
