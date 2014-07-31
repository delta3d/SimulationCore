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
file(GLOB LIB_PUBLIC_HEADERS_8 
   "${HEADER_PATH}/ActComps/*.h"
   )
SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS_1} ${LIB_PUBLIC_HEADERS_2} ${LIB_PUBLIC_HEADERS_3} ${LIB_PUBLIC_HEADERS_4} ${LIB_PUBLIC_HEADERS_5} ${LIB_PUBLIC_HEADERS_6} ${LIB_PUBLIC_HEADERS_7} ${LIB_PUBLIC_HEADERS_8})

set(LIB_SOURCES_1 
   "${SOURCE_PATH}/AttachedMotionModel.cpp"
   "${SOURCE_PATH}/BaseGameEntryPoint.cpp"
   "${SOURCE_PATH}/BaseWheeledVehiclePhysicsHelper.cpp"
   "${SOURCE_PATH}/ClampedMotionModel.cpp"
   "${SOURCE_PATH}/CollisionGroupEnum.cpp"
   "${SOURCE_PATH}/CommandLineObject.cpp"
   "${SOURCE_PATH}/CustomCullVisitor.cpp"
   "${SOURCE_PATH}/FourWheelVehiclePhysicsHelper.cpp"
   "${SOURCE_PATH}/IGExceptionEnum.cpp"
   "${SOURCE_PATH}/MatrixManipulations.cpp"
   "${SOURCE_PATH}/Messages.cpp"
   "${SOURCE_PATH}/MessageType.cpp"
   "${SOURCE_PATH}/ModifiedStream.cpp"
   "${SOURCE_PATH}/ParticleUtil.cpp"
   "${SOURCE_PATH}/PlayerMotionModel.cpp"
   "${SOURCE_PATH}/precomp.cpp"
   "${SOURCE_PATH}/Projector.cpp"
   "${SOURCE_PATH}/SimCoreCullVisitor.cpp"
   "${SOURCE_PATH}/StealthMotionModel.cpp"
   "${SOURCE_PATH}/TerrainPhysicsMode.cpp"
   "${SOURCE_PATH}/TrailEffect.cpp"
   "${SOURCE_PATH}/UnitEnums.cpp"
   "${SOURCE_PATH}/Utilities.cpp"
   "${SOURCE_PATH}/VisibilityOptions.cpp"
   "${SOURCE_PATH}/WeaponTypeEnum.cpp"
   "${SOURCE_PATH}/ApplyShaderVisitor.cpp"
   )

set(LIB_SOURCES_2 
   "${SOURCE_PATH}/Actors/AtmosphereData.cpp"
   "${SOURCE_PATH}/Actors/BaseEntity.cpp"
   "${SOURCE_PATH}/Actors/BasePhysicsVehicleActor.cpp"
   "${SOURCE_PATH}/Actors/BaseTerrainActor.cpp"
   "${SOURCE_PATH}/Actors/BaseWaterActor.cpp"
   "${SOURCE_PATH}/Actors/BattlefieldGraphicsActor.cpp"
   "${SOURCE_PATH}/Actors/CamoConfigActor.cpp"
   "${SOURCE_PATH}/Actors/ControlStateActor.cpp"
   "${SOURCE_PATH}/Actors/DayTimeActor.cpp"
   "${SOURCE_PATH}/Actors/DetonationActor.cpp"
   "${SOURCE_PATH}/Actors/DRGhostActor.cpp"
   "${SOURCE_PATH}/Actors/DynamicLightPrototypeActor.cpp"
   "${SOURCE_PATH}/Actors/DynamicParticleSystem.cpp"
   "${SOURCE_PATH}/Actors/EntityActorRegistry.cpp"
   "${SOURCE_PATH}/Actors/EnvironmentProcessActor.cpp"
   "${SOURCE_PATH}/Actors/EphemerisEnvironmentActor.cpp"
   "${SOURCE_PATH}/Actors/FireActor.cpp"
   "${SOURCE_PATH}/Actors/FlareActor.cpp"
   "${SOURCE_PATH}/Actors/FourWheelVehicleActor.cpp"
   "${SOURCE_PATH}/Actors/Human.cpp"
   "${SOURCE_PATH}/Actors/HumanWithPhysicsActor.cpp"
   "${SOURCE_PATH}/Actors/IGActor.cpp"
   "${SOURCE_PATH}/Actors/IGEnvironmentActor.cpp"
   "${SOURCE_PATH}/Actors/InteriorActor.cpp"
   "${SOURCE_PATH}/Actors/LatLongDataActor.cpp"
   "${SOURCE_PATH}/Actors/LensFlareDrawable.cpp"
   "${SOURCE_PATH}/Actors/LocalEffectActor.cpp"
   "${SOURCE_PATH}/Actors/LogicConditionalActor.cpp"
   "${SOURCE_PATH}/Actors/LogicOnEventActor.cpp"
   "${SOURCE_PATH}/Actors/MissileActor.cpp"
   "${SOURCE_PATH}/Actors/MultipleDetonationActor.cpp"
   "${SOURCE_PATH}/Actors/MunitionParticlesActor.cpp"
   "${SOURCE_PATH}/Actors/MunitionTypeActor.cpp"
   "${SOURCE_PATH}/Actors/OceanDataActor.cpp"
   "${SOURCE_PATH}/Actors/OpenFlightToIVETerrain.cpp"
   "${SOURCE_PATH}/Actors/PagedTerrainPhysicsActor.cpp"
   "${SOURCE_PATH}/Actors/PhysicsParticleSystemActor.cpp"
   "${SOURCE_PATH}/Actors/Platform.cpp"
   "${SOURCE_PATH}/Actors/PlayerActor.cpp"
   "${SOURCE_PATH}/Actors/PortalActor.cpp"
   "${SOURCE_PATH}/Actors/PositionMarker.cpp"
   "${SOURCE_PATH}/Actors/SimpleMovingShapeActor.cpp"
   "${SOURCE_PATH}/Actors/StealthActor.cpp"
   "${SOURCE_PATH}/Actors/SurfaceHazeDataActor.cpp"
   "${SOURCE_PATH}/Actors/SurfaceVesselActor.cpp"
   "${SOURCE_PATH}/Actors/TerrainActorProxy.cpp"
   "${SOURCE_PATH}/Actors/TextureProjectorActor.cpp"
   "${SOURCE_PATH}/Actors/UniformAtmosphereActor.cpp"
   "${SOURCE_PATH}/Actors/VehicleAttachingConfigActor.cpp"
   "${SOURCE_PATH}/Actors/ViewerMaterialActor.cpp"
   "${SOURCE_PATH}/Actors/VolumetricLine.cpp"
   "${SOURCE_PATH}/Actors/WaterGridActor.cpp"
   "${SOURCE_PATH}/Actors/WeaponActor.cpp"
   "${SOURCE_PATH}/Actors/WeaponFlashActor.cpp"
   )

set( LIB_SOURCES_3 
   "${SOURCE_PATH}/Tools/Binoculars.cpp"
   "${SOURCE_PATH}/Tools/Compass.cpp"
   "${SOURCE_PATH}/Tools/Compass360.cpp"
   "${SOURCE_PATH}/Tools/GPS.cpp"
   "${SOURCE_PATH}/Tools/HelpWindow.cpp"
   "${SOURCE_PATH}/Tools/LaserRangeFinder.cpp"
   "${SOURCE_PATH}/Tools/Tool.cpp"
   )

set( LIB_SOURCES_4 
   "${SOURCE_PATH}/Components/ArticulationHelper.cpp"
   "${SOURCE_PATH}/Components/BaseGameAppComponent.cpp"
   "${SOURCE_PATH}/Components/BaseHUD.cpp"
   "${SOURCE_PATH}/Components/BaseHUDElements.cpp"
   "${SOURCE_PATH}/Components/BaseInputComponent.cpp"
   "${SOURCE_PATH}/Components/ControlStateComponent.cpp"
   "${SOURCE_PATH}/Components/DamageHelper.cpp"
   "${SOURCE_PATH}/Components/DefaultArticulationHelper.cpp"
   "${SOURCE_PATH}/Components/DefaultFlexibleArticulationHelper.cpp"
   "${SOURCE_PATH}/Components/LabelManager.cpp"
   "${SOURCE_PATH}/Components/MultiSurfaceClamper.cpp"
   "${SOURCE_PATH}/Components/MunitionDamage.cpp"
   "${SOURCE_PATH}/Components/MunitionDamageTable.cpp"
   "${SOURCE_PATH}/Components/MunitionsComponent.cpp"
   "${SOURCE_PATH}/Components/MunitionsConfig.cpp"
   "${SOURCE_PATH}/Components/MunitionTypeTable.cpp"
   "${SOURCE_PATH}/Components/ParticleManagerComponent.cpp"
   "${SOURCE_PATH}/Components/PortalComponent.cpp"
   "${SOURCE_PATH}/Components/RenderingSupportComponent.cpp"
   "${SOURCE_PATH}/Components/StealthHUDElements.cpp"
   "${SOURCE_PATH}/Components/TextureProjectorComponent.cpp"
   "${SOURCE_PATH}/Components/TimedDeleterComponent.cpp"
   "${SOURCE_PATH}/Components/ViewerMaterialComponent.cpp"
   "${SOURCE_PATH}/Components/ViewerMessageProcessor.cpp"
   "${SOURCE_PATH}/Components/ViewerNetworkPublishingComponent.cpp"
   "${SOURCE_PATH}/Components/VolumeRenderingComponent.cpp"
   "${SOURCE_PATH}/Components/WeaponEffectsManager.cpp"
   "${SOURCE_PATH}/Components/WeatherComponent.cpp"
   "${SOURCE_PATH}/Components/VolumeRenderingComponent.cpp"
   )
set( LIB_SOURCES_5 
   "${SOURCE_PATH}/GUI/AnimationController.cpp"
   "${SOURCE_PATH}/GUI/CeguiUtils.cpp"
   "${SOURCE_PATH}/GUI/DefaultAnimationControllers.cpp"
   "${SOURCE_PATH}/GUI/SceneWindow.cpp"
   "${SOURCE_PATH}/GUI/Screen.cpp"
   "${SOURCE_PATH}/GUI/SimpleScreen.cpp"
   )
set( LIB_SOURCES_6 
   "${SOURCE_PATH}/Components/Conversations/Conversation.cpp"
   "${SOURCE_PATH}/Components/Conversations/ConversationComponent.cpp"
   "${SOURCE_PATH}/Components/Conversations/ConversationMessages.cpp"
   "${SOURCE_PATH}/Components/Conversations/Interaction.cpp"
   "${SOURCE_PATH}/Components/Conversations/Response.cpp"
   )
set( LIB_SOURCES_7 
   "${SOURCE_PATH}/Components/GameState/GameState.cpp"
   "${SOURCE_PATH}/Components/GameState/GameStateChangeMessage.cpp"
   "${SOURCE_PATH}/Components/GameState/GameStateComponent.cpp"
   "${SOURCE_PATH}/Components/GameState/TransitionXMLHandler.cpp"
   )
set( LIB_SOURCES_8 
   "${SOURCE_PATH}/ActComps/AbstractWheeledVehicleInputActComp.cpp"
   "${SOURCE_PATH}/ActComps/AnimationClipActComp.cpp"
   "${SOURCE_PATH}/ActComps/BodyPaintActComp.cpp"
   "${SOURCE_PATH}/ActComps/BodyPaintStateActComp.cpp"
   "${SOURCE_PATH}/ActComps/CamoPaintStateActComp.cpp"
   "${SOURCE_PATH}/ActComps/KeyboardWheeledVehicleInputActComp.cpp"
   "${SOURCE_PATH}/ActComps/TrailerHitchActComp.cpp"
   "${SOURCE_PATH}/ActComps/TrailEffectActComp.cpp"
   "${SOURCE_PATH}/ActComps/TrailEffectActComp.cpp"
   "${SOURCE_PATH}/ActComps/WeaponInventoryActComp.cpp"
   "${SOURCE_PATH}/ActComps/WeaponSwapActComp.cpp"
   "${SOURCE_PATH}/ActComps/WheelActComp.cpp"
   )
SET(LIB_SOURCES ${LIB_SOURCES_1} ${LIB_SOURCES_2} ${LIB_SOURCES_3} ${LIB_SOURCES_4} ${LIB_SOURCES_5} ${LIB_SOURCES_6} ${LIB_SOURCES_7} ${LIB_SOURCES_8})


list(REMOVE_ITEM LIB_SOURCES ${SOURCE_PATH}/precomp.cpp)

if (MSVC)
   #ADD_DEFINITIONS(-DOSGEPHEMERIS_LIBRARY_STATIC)
endif (MSVC)

ADD_PRECOMPILED_HEADER(${LIB_NAME} prefix/SimCorePrefix.h prefix/SimCorePrefix.cpp LIB_SOURCES)

source_group("Header Files\\ActorsH" FILES ${LIB_PUBLIC_HEADERS_2})
source_group("Header Files\\ToolsH" FILES ${LIB_PUBLIC_HEADERS_3})
source_group("Header Files\\ComponentsH" FILES ${LIB_PUBLIC_HEADERS_4})
source_group("Header Files\\GUIH" FILES ${LIB_PUBLIC_HEADERS_5})
source_group("Header Files\\ComponentsH\\ConversationsH" FILES ${LIB_PUBLIC_HEADERS_6})
source_group("Header Files\\ComponentsH\\GameStateH" FILES ${LIB_PUBLIC_HEADERS_7})
source_group("Header Files\\ActCompsH" FILES ${LIB_PUBLIC_HEADERS_8})

source_group("Source Files\\Actors" FILES ${LIB_SOURCES_2})
source_group("Source Files\\Tools" FILES ${LIB_SOURCES_3})
source_group("Source Files\\Components" FILES ${LIB_SOURCES_4})
source_group("Source Files\\GUI" FILES ${LIB_SOURCES_5})
source_group("Source Files\\Components\\Conversations" FILES ${LIB_SOURCES_6})
source_group("Source Files\\Components\\GameState" FILES ${LIB_SOURCES_7})
source_group("Source Files\\ActComps" FILES ${LIB_SOURCES_8})

SET(LIB_EXTERNAL_DEPS ${LIB_NAME}
  DTUTIL_LIBRARY
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
  DTPHYSICS_LIBRARY
  PAL_LIBRARY
  )

SET(LIB_DEPS ${LIB_NAME} 
   ${OSGEPHEMERIS_LIBRARY}
   ${OPENGL_LIBRARY}
   ${AGL_LIBRARY}
   ${OPENAL_LIBRARY}
)

DELTA3D_ADD_LIBRARY(${LIB_NAME} SIMCORE_LIBRARY SHARED)
