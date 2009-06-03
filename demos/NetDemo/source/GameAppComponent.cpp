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

#include <dtCore/camera.h>
#include <dtCore/transform.h>
#include <dtABC/application.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>
//#include <dtActors/playerstartactorproxy.h>
#include <dtActors/engineactorregistry.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Components/GameState/GameStateChangeMessage.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>

#include <GameAppComponent.h>
#include <ActorRegistry.h>
#include <PlayerStatusActor.h>
#include <States.h>

// Temp - delete this unless you are using COuts.
//#include <iostream>
#include <HoverVehicleActor.h>

namespace NetDemo
{
   //////////////////////////////////////////////////////////////////////////
   GameAppComponent::GameAppComponent(const std::string& name)
      : BaseClass(name)
      , mIsServer(false)
   {
      mLogger = &dtUtil::Log::GetInstance("GameAppComponent.cpp");
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::ProcessMessage(const dtGame::Message& msg)
   {

      BaseClass::ProcessMessage(msg);

      const dtGame::MessageType& messageType = msg.GetMessageType();

      // Process game state changes.
      if (messageType == SimCore::MessageType::GAME_STATE_CHANGED)
      {
         HandleStateChangeMessage( static_cast<const SimCore::Components::GameStateChangedMessage&>(msg) );
      }

      if (dtGame::MessageType::INFO_MAP_LOADED == msg.GetMessageType())
      {
         InitializePlayer();
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == msg.GetMessageType())
      {
         HandleActorUpdateMessage(msg);
      }

      // Something about Game State changing here
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

      // Hack stuff - move this to UI - user selected and all that.
      mPlayerStatus->SetIsServer(mIsServer);
      mPlayerStatus->SetTerrainPreference("Terrain1");
      //mPlayerStatus->SetTerrainPreference("Terrains:Level_DriverDemo.ive");
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
            if (statusActor->GetTerrainPreference() != mCurrentTerrainPrototypeName)
            {
               mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
                  "Changing terrain to [%s]. Switching to LOADING state.", statusActor->GetTerrainPreference().c_str());

               // Switch to LOADING STATE - Update will be published as soon as actor is ticked,
               // which will happen later this tick (actors are after components).
               mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::LOADING);

               // Set the terrain to start loading once we switch to loading state.
               mTerrainToLoad = statusActor->GetTerrainPreference();

               // Change state to LOADING here
               HandleTransition( Transition::TRANSITION_FORWARD );
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::HandleStateChangeMessage( const SimCore::Components::GameStateChangedMessage& stateChange )
   {
      const SimCore::Components::StateType& state = stateChange.GetNewState();

      if( state == NetDemoState::STATE_CONNECTING )
      {
         // Change state to LOADING here
         LoadMaps( mMapName );
      }
      else if( state == SimCore::Components::StateType::STATE_LOADING )
      {
         HandleLoadingState();
      }
      else if( state == SimCore::Components::StateType::STATE_SHUTDOWN )
      {
         GetGameManager()->GetApplication().Quit();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::HandleLoadingState()
   {
      if (mCurrentTerrainPrototypeName != mTerrainToLoad && mTerrainToLoad != "")
      {
         UnloadPreviousTerrain();
         mCurrentTerrainPrototypeName = mTerrainToLoad;
         mTerrainToLoad = "";
         LoadNewTerrain();

         mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_GAME_ALIVE);

         // Loading state is complete. Automatically advance to the next state.
         HandleTransition( SimCore::Components::EventType::TRANSITION_FORWARD );




         /////////////////////////////////////////////////////////
         // Hack stuff - add a vehicle here. For testing purposes.
/*         HoverVehicleActorProxy* prototypeProxy = NULL;
         GetGameManager()->FindPrototypeByName("Hover Vehicle", prototypeProxy);
         if (prototypeProxy == NULL)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
               "Critical Error - can't find vehicle prototype [Hover Vehicle]. Likely error - incorrect additional maps in your config.xml. Compare to the config_example.xml.");
            return;
         }
         dtCore::RefPtr<HoverVehicleActorProxy> testVehicleProxy = NULL;
         GetGameManager()->CreateActorFromPrototype(prototypeProxy->GetId(), testVehicleProxy);
         if(testVehicleProxy != NULL)
         {
            //HoverVehicleActor *vehicleActor = dynamic_cast<HoverVehicleActor*>(testVehicleProxy->GetGameActor());
            //if (vehicleActor != NULL)
            //{
               GetGameManager()->AddActor(*testVehicleProxy.get(), false, true);

            //}
         }
         */
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::UnloadPreviousTerrain()
   {
      if (mCurrentTerrainDrawActor.valid())
      {
         // Delete the visible terrain
         GetGameManager()->DeleteActor(mCurrentTerrainDrawActor->GetGameActorProxy());
         mCurrentTerrainDrawActor = NULL;
         mCurrentTerrainPrototypeName = "";

         mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__, "Now unloading the previous terrain.");
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::LoadNewTerrain()
   {

      // NOTE - there are TWO steps. 1) load the DRAWN Terrain. 2) sync the PHYSICS terrain


      // Find the prototype for the DRAWN terrain.
      SimCore::Actors::TerrainActorProxy* drawLandPrototypeProxy = NULL;
      GetGameManager()->FindPrototypeByName(mCurrentTerrainPrototypeName, drawLandPrototypeProxy);
      if (drawLandPrototypeProxy == NULL)
      {
         mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
            "Cannot load the drawLandPrototype [%s] from the GM. Likely error - incorrect additional maps in your config.xml. Compare to the config_example.xml.",
            mCurrentTerrainPrototypeName.c_str());
         mCurrentTerrainPrototypeName = "";
         return;
      }


      // Create a new instance of the DRAWN Terrain.
      dtCore::RefPtr<SimCore::Actors::TerrainActorProxy> newDrawLandActorProxy = NULL;
      GetGameManager()->CreateActorFromPrototype(drawLandPrototypeProxy->GetId(), newDrawLandActorProxy);
      if (!newDrawLandActorProxy.valid())
      {
         mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
            "Cannot create a newDrawLandActor for prototype [%s] from the GM. CRITICAL ERROR!",
            mCurrentTerrainPrototypeName.c_str());
         mCurrentTerrainPrototypeName = "";
         return;
      }


      // Add to the GM
      GetGameManager()->AddActor(*newDrawLandActorProxy, false, false);
      mCurrentTerrainDrawActor = dynamic_cast<SimCore::Actors::TerrainActor*>
         (newDrawLandActorProxy->GetActor());

   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::Actors::TerrainActor* GameAppComponent::GetCurrentTerrainDrawActor()
   {
      return dynamic_cast<SimCore::Actors::TerrainActor *>(mCurrentTerrainDrawActor.get());
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::SetIsServer(bool newValue)
   {
      if (!mPlayerStatus.valid()) mIsServer = newValue;
   }

   //////////////////////////////////////////////////////////////////////////
   bool GameAppComponent::GetIsServer()
   {
      return mIsServer;
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::SetMapName(const std::string& mapName)
   {
      mMapName = mapName;
   }

   //////////////////////////////////////////////////////////////////////////
   const std::string& GameAppComponent::GetMapName() const
   {
      return mMapName;
   }

   //////////////////////////////////////////////////////////////////////////
   StateComponent* GameAppComponent::GetStateComponent()
   {
      if( ! mStateComp.valid() )
      {
         dtGame::GameManager* gm = GetGameManager();
         if( gm == NULL )
         {
            LOG_ERROR("Could not access Game Manager to access the State Component.");
         }
         else
         {
            StateComponent* stateComp = NULL;
            gm->GetComponentByName( StateComponent::DEFAULT_NAME.Get(), stateComp );
            mStateComp = stateComp;
         }
      }

      return mStateComp.get();
   }

   //////////////////////////////////////////////////////////////////////////
   bool GameAppComponent::HandleTransition( const std::string& transitionName )
   {
      bool success = false;

      SimCore::Components::EventType* transition
         = SimCore::Components::EventType::GetValueForName( transitionName );

      if( transition != NULL )
      {
         success = HandleTransition( *transition );
      }
      else
      {
         LOG_WARNING("Could not handle transition \"" + transitionName + "\"");
      }

      return success;
   }

   //////////////////////////////////////////////////////////////////////////
   bool GameAppComponent::HandleTransition( SimCore::Components::EventType& transition )
   {
      bool success = false;

      StateComponent* stateComp = GetStateComponent();
      if( stateComp != NULL )
      {
         stateComp->HandleEvent( &transition );
         success = true;
      }

      return success;
   }

}
