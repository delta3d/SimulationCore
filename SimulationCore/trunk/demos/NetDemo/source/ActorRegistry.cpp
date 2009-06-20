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
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix-src.h>
#include <ActorRegistry.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <Actors/HoverVehicleActor.h>
#include <Actors/PlayerStatusActor.h>
#include <Actors/ServerGameStatusActor.h>
#include <Actors/FortActor.h>
#include <WheeledVehicleActor.h>
#include <Actors/EnemyMine.h>

#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

using dtCore::RefPtr;

namespace NetDemo
{

   RefPtr<dtDAL::ActorType> NetDemoActorRegistry::HOVER_VEHICLE_ACTOR_TYPE(
      new dtDAL::ActorType("HoverActor", "NetDemo", "A floaty drivable vehicle for Driver Demo",
      SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtDAL::ActorType> NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE(
      new dtDAL::ActorType("PlayerStatusActor", "NetDemo", "Status of each real player such as game state, team, and vehicle for NetDemo",
      SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE.get()));

   RefPtr<dtDAL::ActorType> NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE(
      new dtDAL::ActorType("ServerGameStatusActor", "NetDemo", "Status of the overall game - controlled by the server. "));

   RefPtr<dtDAL::ActorType> NetDemoActorRegistry::WHEELED_VEHICLE_ACTOR_TYPE(
      new dtDAL::ActorType("Wheeled Vehicle Actor", "NetDemo", "The wheeled vehicle we all know and love"));

   RefPtr<dtDAL::ActorType> NetDemoActorRegistry::FORT_ACTOR_TYPE(
      new dtDAL::ActorType("FortActor", "NetDemo", "The team base - when destroyed the team looses.",
      SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

   RefPtr<dtDAL::ActorType> NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE(
      new dtDAL::ActorType("EnemyMineActor", "NetDemo", "Flies around and destroys self near base",
      SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

   ///////////////////////////////////////////////////////////////////////////
   extern "C" NETDEMO_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
   {
       return new NetDemoActorRegistry;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" NETDEMO_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry *registry)
   {
       delete registry;
   }

   ///////////////////////////////////////////////////////////////////////////
   NetDemoActorRegistry::NetDemoActorRegistry() :
      dtDAL::ActorPluginRegistry("This library holds actors from the NetDemo")
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   void NetDemoActorRegistry::RegisterActorTypes()
   {
      mActorFactory->RegisterType<HoverVehicleActorProxy>(HOVER_VEHICLE_ACTOR_TYPE.get());
      mActorFactory->RegisterType<PlayerStatusActorProxy>(PLAYER_STATUS_ACTOR_TYPE.get());
      mActorFactory->RegisterType<ServerGameStatusActorProxy>(SERVER_GAME_STATUS_ACTOR_TYPE.get());
      mActorFactory->RegisterType<WheeledVehicleActorProxy>(WHEELED_VEHICLE_ACTOR_TYPE.get());
      mActorFactory->RegisterType<FortActorProxy>(FORT_ACTOR_TYPE.get());
      mActorFactory->RegisterType<EnemyMineActorProxy>(ENEMY_MINE_ACTOR_TYPE.get());

   }
}
