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
file(GLOB LIB_PUBLIC_HEADERS_5 
   "${HEADER_PATH}/GUI/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_6 
   "${HEADER_PATH}/Components/Conversations/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_7 
   "${HEADER_PATH}/Components/GameState/*.h"
   )
SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS_1} ${LIB_PUBLIC_HEADERS_2} ${LIB_PUBLIC_HEADERS_3} ${LIB_PUBLIC_HEADERS_4} ${LIB_PUBLIC_HEADERS_5} ${LIB_PUBLIC_HEADERS_6} ${LIB_PUBLIC_HEADERS_7})

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
file(GLOB LIB_SOURCES_5 
   "${SOURCE_PATH}/GUI/*.cpp"
   )
file(GLOB LIB_SOURCES_6 
   "${SOURCE_PATH}/Components/Conversations/*.cpp"
   )
file(GLOB LIB_SOURCES_7 
   "${SOURCE_PATH}/Components/GameState/*.cpp"
   )
SET(LIB_SOURCES ${LIB_SOURCES_1} ${LIB_SOURCES_2} ${LIB_SOURCES_3} ${LIB_SOURCES_4} ${LIB_SOURCES_5} ${LIB_SOURCES_6} ${LIB_SOURCES_7})


list(REMOVE_ITEM LIB_SOURCES ${SOURCE_PATH}/precomp.cpp)

if (MSVC)
   ADD_DEFINITIONS(-DOSGEPHEMERIS_LIBRARY_STATIC)
endif (MSVC)

ADD_LIBRARY(${LIB_NAME} SHARED                        
    ${LIB_PUBLIC_HEADERS}                         
    ${LIB_SOURCES}
)

source_group("Header Files\\ActorsH" FILES ${LIB_PUBLIC_HEADERS_2})
source_group("Header Files\\ToolsH" FILES ${LIB_PUBLIC_HEADERS_3})
source_group("Header Files\\ComponentsH" FILES ${LIB_PUBLIC_HEADERS_4})
source_group("Header Files\\GUIH" FILES ${LIB_PUBLIC_HEADERS_5})
source_group("Header Files\\ComponentsH\\ConversationsH" FILES ${LIB_PUBLIC_HEADERS_6})
source_group("Header Files\\ComponentsH\\GameStateH" FILES ${LIB_PUBLIC_HEADERS_7})

source_group("Source Files\\Actors" FILES ${LIB_SOURCES_2})
source_group("Source Files\\Tools" FILES ${LIB_SOURCES_3})
source_group("Source Files\\Components" FILES ${LIB_SOURCES_4})
source_group("Source Files\\GUI" FILES ${LIB_SOURCES_5})
source_group("Source Files\\Components\\Conversations" FILES ${LIB_SOURCES_6})
source_group("Source Files\\Components\\GameState" FILES ${LIB_SOURCES_7})

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
   ${OSGEPHEMERIS_LIBRARY}
   ${OPENGL_LIBRARY}
   ${AGL_LIBRARY}
   ${PHYSX_LIBRARIES}
   ${OPENAL_LIBRARY}
)


SET_TARGET_PROPERTIES(${LIB_NAME}
                     PROPERTIES DEFINE_SYMBOL SIMCORE_LIBRARY)

#little hack to get the .dll into /bin and .lib into /lib
IF (MSVC_IDE)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "/../../bin/")
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../")
ENDIF (MSVC_IDE)
