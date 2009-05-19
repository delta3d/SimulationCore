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
* @author David Guthrie, Curtiss Murphy
*/

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PlayerActor.h>
#include <dtCore/camera.h>
#include <dtCore/transform.h>
#include <dtABC/application.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
//#include <dtActors/playerstartactorproxy.h>
#include <dtActors/engineactorregistry.h>

#include <GameAppComponent.h>
#include <ActorRegistry.h>
#include <PlayerStatusActor.h>

// Temp - delete this unless you are using COuts.
//#include <iostream>


namespace NetDemo
{
   //////////////////////////////////////////////////////////////////////////
   GameAppComponent::GameAppComponent(const std::string& name)
   : BaseClass(name)
   {
      mLogger = &dtUtil::Log::GetInstance("GameAppComponent.cpp");
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::ProcessMessage(const dtGame::Message& msg)
   {
      BaseClass::ProcessMessage(msg);

      if (dtGame::MessageType::INFO_MAP_LOADED == msg.GetMessageType())
      {
         InitializePlayer();
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == msg.GetMessageType())
      {
         HandleActorUpdateMessage(msg);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::InitializePlayer()
   {
      dtCore::RefPtr<dtGame::GameActorProxy> ap;

      // Every player always has a player actor. On some apps, it is an overkill, but 
      // we use it anyway, for consistency. It allows tools, position, ability to have an avatar, walk, run, jump, etc. 
      //GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, ap);
      GetGameManager()->CreateActor(*NetDemo::NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE, ap);
      //mPlayerStatus = static_cast<SimCore::Actors::PlayerActor*>(ap->GetActor());
      mPlayerStatus = static_cast<NetDemo::PlayerStatusActor*>(ap->GetActor());
      // make the camera a child
      mPlayerStatus->AddChild(GetGameManager()->GetApplication().GetCamera());
      mPlayerStatus->SetName("Player (Unknown)");

      // Hack stuff - move this to UI.
      mPlayerStatus->SetIsServer(true);
      mPlayerStatus->SetTerrainPreference("Terrains:Level_DriverDemo.ive");
      mPlayerStatus->SetTeamNumber(1);
      mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_LOBBY);
 
      GetGameManager()->AddActor(mPlayerStatus->GetGameActorProxy(), false, true);


      //////////////TEMP HACK
      // Set the starting position from a player start actor in the map.
      //dtActors::PlayerStartActorProxy* startPosProxy = NULL;
      //GetGameManager()->FindActorByType(*dtActors::EngineActorRegistry::PLAYER_START_ACTOR_TYPE, startPosProxy);
      //if (startPosProxy != NULL)
      //{
      //   dtCore::Transformable* actor = NULL;
      //   startPosProxy->GetActor(actor);
      //   dtCore::Transform xform;
      //   actor->GetTransform(xform);
      //   mPlayerStatus->SetTransform(xform);
      //}
      ////////////////////////
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::HandleActorUpdateMessage(const dtGame::Message& msg)
   {
      const dtGame::ActorUpdateMessage &updateMessage =
         static_cast<const dtGame::ActorUpdateMessage&> (msg);

      if (updateMessage.GetActorType() == NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE)
      {
         //if (updateMessage.GetSource() != GetGameManager()->GetMachineInfo()) ... Is remote
         // Find the actor in the GM
         dtGame::GameActorProxy *gap = GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId());
         if(gap == NULL) return;
         PlayerStatusActor *statusActor = dynamic_cast<PlayerStatusActor*>(gap->GetActor());
         if(statusActor == NULL)  return;

         // If the actor belongs to the server, then we have work to do.
         if (statusActor->GetIsServer())
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
               "Got a player status update message from the server. Time to sync up.");

            if (statusActor->GetTerrainPreference() != mCurrentTerrainLoaded)
            {
               mCurrentTerrainLoaded = statusActor->GetTerrainPreference();
               mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
                  "Need to load terrain [%s].", statusActor->GetTerrainPreference().c_str());

            }

         }
      }



   }

}
