/*
* Copyright, 2009, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* David Guthrie
* Brad Anderegg
* Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>
#include <ActorRegistry.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <dtActors/engineactorregistry.h>

#include <Actors/HoverVehicleActor.h>
#include <Actors/PlayerStatusActor.h>
#include <Actors/ServerGameStatusActor.h>
#include <Actors/FortActor.h>
#include <Actors/EnemyMine.h>
#include <Actors/EnemyHelix.h>
#include <Actors/EnemyMothership.h>
#include <Actors/SpawnVolumeActor.h>
#include <Actors/EnemyDescriptionActor.h>
#include <Actors/PropelledVehicleActor.h>
#include <Actors/TowerActor.h>
#include <Actors/FireBallTowerActor.h>
#include <Actors/FireBallActor.h>
#include <Actors/LightTower.h>
#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

#include <AIComponent.h>
#include <Components/GUIComponent.h>
#include <Components/InputComponent.h>
#include <Components/SpawnComponent.h>
#include <Components/GameLogicComponent.h>

using dtCore::RefPtr;

namespace NetDemo
{

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::HOVER_VEHICLE_ACTOR_TYPE(
      new dtCore::ActorType("HoverActor", "NetDemo", "A floaty drivable vehicle for Driver Demo",
      SimCore::Actors::EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE(
      new dtCore::ActorType("PlayerStatusActor", "NetDemo", "Status of each real player such as game state, team, and vehicle for NetDemo",
      SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE(
      new dtCore::ActorType("ServerGameStatusActor", "NetDemo", "Status of the overall game - controlled by the server. "));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::PROPELLED_VEHICLE_ACTOR_TYPE(
      new dtCore::ActorType("Propelled Vehicle Actor", "NetDemo", "The propelled vehicle is basically a wheeled vehicle with a jet engine attached.",
            SimCore::Actors::EntityActorRegistry::FOUR_WHEEL_VEHICLE_MIL_ACTOR_TYPE.get()));


   RefPtr<dtCore::ActorType> NetDemoActorRegistry::TOWER_ACTOR_TYPE(
      new dtCore::ActorType("Tower Actor", "NetDemo", "A generic tower with a turret, used for defense.",
      SimCore::Actors::EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::FORT_ACTOR_TYPE(
      new dtCore::ActorType("FortActor", "NetDemo", "The team base - when destroyed the team looses.",
      SimCore::Actors::EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE(
      new dtCore::ActorType("EnemyMineActor", "NetDemo", "Flies around and destroys self near base",
      SimCore::Actors::EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE(
      new dtCore::ActorType("EnemyHelixActor", "NetDemo", "A more sophisticated enemy then the enemy mine.",
      SimCore::Actors::EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::ENEMY_MOTHERSHIP_ACTOR_TYPE(
      new dtCore::ActorType("EnemyMothershipActor", "NetDemo", "The motherhship spawns the other enemies.",
      SimCore::Actors::EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::ENEMY_DESCRIPTION_TYPE(
      new dtCore::ActorType("EnemyDecriptionActor", "NetDemo", "Describes the attributes of an enemy prototype."));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::LIGHT_TOWER_ACTOR_TYPE(
      new dtCore::ActorType("LightTower", "NetDemo", "A light tower with a tracking spot light, for a front line of defense.",
      SimCore::Actors::EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::FIREBALL_TOWER_ACTOR_TYPE(
      new dtCore::ActorType("FireBallTower", "NetDemo", "A tower which shoots a powerful fireball for a last line of defense.",
      SimCore::Actors::EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtCore::ActorType> NetDemoActorRegistry::FIREBALL_ACTOR_TYPE(
      new dtCore::ActorType("FireBall", "NetDemo", "A blazing fireball shot through the fireball tower.",
      SimCore::Actors::EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE.get()));



   const dtCore::RefPtr<dtCore::SystemComponentType> AIComponent::TYPE(new dtCore::SystemComponentType("AIComponent","GMComponents.SimCore.NetDemo", "", dtGame::BaseInputComponent::DEFAULT_TYPE));
   const dtCore::RefPtr<dtCore::SystemComponentType> InputComponent::TYPE(new dtCore::SystemComponentType("InputComponent","GMComponents.SimCore.NetDemo", "", dtGame::BaseInputComponent::DEFAULT_TYPE));
   const dtCore::RefPtr<dtCore::SystemComponentType> SpawnComponent::TYPE(new dtCore::SystemComponentType("SpawnComponent","GMComponents.SimCore.NetDemo", "", dtGame::GMComponent::BaseGMComponentType));
   const dtCore::RefPtr<dtCore::SystemComponentType> GameLogicComponent::TYPE(new dtCore::SystemComponentType("GameLogicComponent","GMComponents.SimCore.NetDemo", "", BaseClass::TYPE));
   const dtCore::RefPtr<dtCore::SystemComponentType> GUIComponent::TYPE(new dtCore::SystemComponentType("GUIComponent","GMComponents.SimCore.NetDemo", "", dtGame::GMComponent::BaseGMComponentType));
   const std::string SpawnComponent::DEFAULT_NAME = "SpawnComponent";


   ///////////////////////////////////////////////////////////////////////////
   extern "C" NETDEMO_EXPORT dtCore::ActorPluginRegistry* CreatePluginRegistry()
   {
       return new NetDemoActorRegistry;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" NETDEMO_EXPORT void DestroyPluginRegistry(dtCore::ActorPluginRegistry *registry)
   {
       delete registry;
   }

   ///////////////////////////////////////////////////////////////////////////
   NetDemoActorRegistry::NetDemoActorRegistry() :
      dtCore::ActorPluginRegistry("This library holds actors from the NetDemo")
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   void NetDemoActorRegistry::RegisterActorTypes()
   {
      mActorFactory->RegisterType<HoverVehicleActorProxy>(HOVER_VEHICLE_ACTOR_TYPE.get());
      mActorFactory->RegisterType<PlayerStatusActorProxy>(PLAYER_STATUS_ACTOR_TYPE.get());
      mActorFactory->RegisterType<ServerGameStatusActorProxy>(SERVER_GAME_STATUS_ACTOR_TYPE.get());
      mActorFactory->RegisterType<PropelledVehicleActorProxy>(PROPELLED_VEHICLE_ACTOR_TYPE.get());
      mActorFactory->RegisterType<FortActorProxy>(FORT_ACTOR_TYPE.get());
      mActorFactory->RegisterType<TowerActorProxy>(TOWER_ACTOR_TYPE.get());
      mActorFactory->RegisterType<EnemyMineActorProxy>(ENEMY_MINE_ACTOR_TYPE.get());
      mActorFactory->RegisterType<EnemyHelixActorProxy>(ENEMY_HELIX_ACTOR_TYPE.get());
      mActorFactory->RegisterType<EnemyDescriptionActorProxy>(ENEMY_DESCRIPTION_TYPE.get());
      mActorFactory->RegisterType<EnemyMothershipActorProxy>(ENEMY_MOTHERSHIP_ACTOR_TYPE.get());
      mActorFactory->RegisterType<LightTowerProxy>(LIGHT_TOWER_ACTOR_TYPE.get());
      mActorFactory->RegisterType<FireBallTowerActorProxy>(FIREBALL_TOWER_ACTOR_TYPE.get());
      mActorFactory->RegisterType<FireBallActorProxy>(FIREBALL_ACTOR_TYPE.get());

   }
}
