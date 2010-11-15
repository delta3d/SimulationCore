SET(LIB_NAME   NetDemo)

SET(HEADER_PATH ${CMAKE_SOURCE_DIR}/demos/${LIB_NAME}/include)
SET(SOURCE_PATH ${CMAKE_SOURCE_DIR}/demos/${LIB_NAME}/source)

INCLUDE_DIRECTORIES(${HEADER_PATH})

file(GLOB LIB_PUBLIC_HEADERS_1 
   "${HEADER_PATH}/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_2 
   "${HEADER_PATH}/Actors/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_3 
   "${HEADER_PATH}/Components/*.h"
   )
file(GLOB LIB_PUBLIC_HEADERS_4 
   "${HEADER_PATH}/GUI/*.h"
   )
SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS_1} ${LIB_PUBLIC_HEADERS_2} ${LIB_PUBLIC_HEADERS_3} ${LIB_PUBLIC_HEADERS_4})

set(LIB_SOURCES_1
   "${SOURCE_PATH}/ActorRegistry.cpp"
   "${SOURCE_PATH}/AIComponent.cpp"
   "${SOURCE_PATH}/AIEvent.cpp"
   "${SOURCE_PATH}/AIPhysicsModel.cpp"
   "${SOURCE_PATH}/AIState.cpp"
   "${SOURCE_PATH}/AISteeringModel.cpp"
   "${SOURCE_PATH}/AIUtility.cpp"
   "${SOURCE_PATH}/BaseAIHelper.cpp"
   "${SOURCE_PATH}/Booster.cpp"
   "${SOURCE_PATH}/ConfigParameters.cpp"
   "${SOURCE_PATH}/EnemyAIHelper.cpp"
   "${SOURCE_PATH}/EnemyHelixAIHelper.cpp"
   "${SOURCE_PATH}/EnemyMineAIHelper.cpp"
   "${SOURCE_PATH}/EnemyMothershipAIHelper.cpp"
   "${SOURCE_PATH}/GameEntryPoint.cpp"
   "${SOURCE_PATH}/NetDemoMessages.cpp"
   "${SOURCE_PATH}/NetDemoMessageTypes.cpp"
   "${SOURCE_PATH}/NetDemoUtils.cpp"
   "${SOURCE_PATH}/States.cpp"
   "${SOURCE_PATH}/TowerAIHelper.cpp"
   "${SOURCE_PATH}/AIWeaponUtility.cpp"
   "${SOURCE_PATH}/NetDemoWaypoint.cpp"
   )
set(LIB_SOURCES_2 
   "${SOURCE_PATH}/Actors/BaseEnemyActor.cpp"
   "${SOURCE_PATH}/Actors/EnemyDescriptionActor.cpp"
   "${SOURCE_PATH}/Actors/EnemyHelix.cpp"
   "${SOURCE_PATH}/Actors/EnemyMine.cpp"
   "${SOURCE_PATH}/Actors/EnemyMothership.cpp"
   "${SOURCE_PATH}/Actors/FortActor.cpp"
   "${SOURCE_PATH}/Actors/HoverVehicleActor.cpp"
   "${SOURCE_PATH}/Actors/HoverVehiclePhysicsHelper.cpp"
   "${SOURCE_PATH}/Actors/PlayerStatusActor.cpp"
   "${SOURCE_PATH}/Actors/PropelledVehicleActor.cpp"
   "${SOURCE_PATH}/Actors/ServerGameStatusActor.cpp"
   "${SOURCE_PATH}/Actors/SpawnVolumeActor.cpp"
   "${SOURCE_PATH}/Actors/TowerActor.cpp"
   "${SOURCE_PATH}/Actors/LightTower.cpp"
   "${SOURCE_PATH}/Actors/FireBallActor.cpp"
   "${SOURCE_PATH}/Actors/FireBallTowerActor.cpp"
   )
   
set(LIB_SOURCES_3 
   "${SOURCE_PATH}/Components/GameLogicComponent.cpp"
   "${SOURCE_PATH}/Components/GUIComponent.cpp"
   "${SOURCE_PATH}/Components/InputComponent.cpp"
   "${SOURCE_PATH}/Components/SpawnComponent.cpp"
   "${SOURCE_PATH}/Components/WeaponComponent.cpp"
   )
   
set(LIB_SOURCES_4 
   "${SOURCE_PATH}/GUI/ButtonHighlight.cpp"
   "${SOURCE_PATH}/GUI/CustomCeguiWidgets.cpp"
   "${SOURCE_PATH}/GUI/HUDScreen.cpp"
   "${SOURCE_PATH}/GUI/MainMenuScreen.cpp"
   "${SOURCE_PATH}/GUI/ReadyRoomScreen.cpp"
   "${SOURCE_PATH}/GUI/ScoreLabelManager.cpp"
   )
SET(LIB_SOURCES ${LIB_SOURCES_1} ${LIB_SOURCES_2} ${LIB_SOURCES_3} ${LIB_SOURCES_4})

ADD_LIBRARY(${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${LIB_SOURCES}
)

source_group("Header Files\\ActorsH" FILES ${LIB_PUBLIC_HEADERS_2})
source_group("Header Files\\ComponentsH" FILES ${LIB_PUBLIC_HEADERS_3})
source_group("Header Files\\GUIH" FILES ${LIB_PUBLIC_HEADERS_4})

source_group("Source Files\\Actors" FILES ${LIB_SOURCES_2})
source_group("Source Files\\Components" FILES ${LIB_SOURCES_3})
source_group("Source Files\\GUI" FILES ${LIB_SOURCES_4})

LINK_WITH_VARIABLES(${LIB_NAME}
   DTNETGM_LIBRARY
)

TARGET_LINK_LIBRARIES(${LIB_NAME} 
   ${SIMCORE_LIBRARY}
   ${APPSERVICES_LIBRARY}
)


SET_TARGET_PROPERTIES(${LIB_NAME}
                     PROPERTIES DEFINE_SYMBOL RES_GAME_LIBRARY)

#little hack to get the .dll into /bin and .lib into /lib
IF (MSVC_IDE)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "/../../bin/")
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../")
ENDIF (MSVC_IDE)

