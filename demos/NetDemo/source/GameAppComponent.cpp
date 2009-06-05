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
#include <dtNetGM/clientnetworkcomponent.h>
#include <dtNetGM/servernetworkcomponent.h>

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
      , mIsConnectedToNetwork(false)
   { 
      mLogger = &dtUtil::Log::GetInstance("GameAppComponent.cpp");

   }


   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::OnAddedToGM()
   {

      ///////////////////////////////////////////////////////////
      /// CMM - TOTAL AND COMPLETE HACK!!!! DELETE THIS!  aLL OF IT!

      dtUtil::ConfigProperties& configParams = GetGameManager()->GetConfiguration();
      //bool isGMOn = dtUtil::ToType<bool>(configParams.GetConfigPropertyValue("dtNetGM.On", "false"));
      const std::string role = configParams.GetConfigPropertyValue("dtNetGM.Role", "server");
      int serverPort = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.ServerPort", "7329"));
      const std::string gameName = configParams.GetConfigPropertyValue("dtNetGM.GameName", "NetDemo");
      int gameVersion = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.GameVersion", "1"));
      const std::string host = configParams.GetConfigPropertyValue("dtNetGM.ServerHost", "127.0.0.1");

      if (role == "Server" || role == "server" || role == "SERVER")
      {
         dtCore::RefPtr<dtNetGM::ServerNetworkComponent> serverComp =
            new dtNetGM::ServerNetworkComponent(gameName, gameVersion);
         GetGameManager()->AddComponent(*serverComp, dtGame::GameManager::ComponentPriority::NORMAL);
         mNetworkComp = serverComp;
      }
      else if (role == "Client" || role == "client" || role == "CLIENT")
      {
         dtCore::RefPtr<dtNetGM::ClientNetworkComponent> clientComp =
            new dtNetGM::ClientNetworkComponent(gameName, gameVersion);
         GetGameManager()->AddComponent(*clientComp, dtGame::GameManager::ComponentPriority::NORMAL);
         mNetworkComp = clientComp;
      }

   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::ProcessMessage(const dtGame::Message& msg)
   {

      BaseClass::ProcessMessage(msg);

      const dtGame::MessageType& messageType = msg.GetMessageType();

      // Process game state changes.
      if (messageType == SimCore::MessageType::GAME_STATE_CHANGED)
      {
         HandleStateChangeMessage(static_cast<const SimCore::Components::GameStateChangedMessage&>(msg));
      }
      else if (dtGame::MessageType::INFO_MAP_LOADED == msg.GetMessageType())
      {
         HandleMapLoaded();
      }
      else if (dtGame::MessageType::INFO_MAP_UNLOADED == msg.GetMessageType())
      {
         HandleTransition(Transition::TRANSITION_FORWARD);
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
                  "Changing terrain to [%s].", statusActor->GetTerrainPreference().c_str());


               // Set the terrain to start loading once we switch to loading state.
               mTerrainToLoad = statusActor->GetTerrainPreference();
               UnloadCurrentTerrain();
               mCurrentTerrainPrototypeName = mTerrainToLoad;
               mTerrainToLoad = "";
               LoadNewTerrain();
            }
         }
      }
   }

   ///////////////////////////////////////////////////////////
   void GameAppComponent::HandleMapLoaded()
   {
      InitializePlayer();

      // if we are a client, we let the server know we are now ready to receive messages
      // We can't receive/send messages to the server until we do this. Do it AFTER the map is loaded.
      dtNetGM::ClientNetworkComponent *networkComponent = 
         dynamic_cast<dtNetGM::ClientNetworkComponent *>(mNetworkComp.get());

      if (mIsConnectedToNetwork && !mIsServer && networkComponent != NULL)
      {
         networkComponent->SendRequestConnectionMessage();
      }

      // Now we can go to the ready room.
      HandleTransition(Transition::TRANSITION_FORWARD);
   }

   ///////////////////////////////////////////////////////////
   bool GameAppComponent::JoinNetworkAsServer(int serverPort, const std::string &gameName, int gameVersion)
   {
      bool result = false;

      /// HACK - can't add a component while in a tick... so, we hacked it! 
      LOG_ALWAYS("HACK MESSAGE - WE CANNOT ADD A COMPONENT WHILE IN A tick MESSAGE. FIX ME!");
      //dtCore::RefPtr<dtNetGM::ServerNetworkComponent> networkComp =
      //   new dtNetGM::ServerNetworkComponent(gameName, gameVersion);
      //GetGameManager()->AddComponent(*networkComp, dtGame::GameManager::ComponentPriority::NORMAL);

      dtCore::RefPtr<dtNetGM::ServerNetworkComponent> serverComp =
         dynamic_cast<dtNetGM::ServerNetworkComponent*>(mNetworkComp.get());

      result = serverComp->SetupServer(serverPort);

      if (result)
      {         
         mIsConnectedToNetwork = true;
         mIsServer = true;
         mNetworkComp = serverComp;
      }
      return result;
   }

   ///////////////////////////////////////////////////////////
   bool GameAppComponent::JoinNetworkAsClient(int serverPort, const std::string &serverIPAddress, 
      const std::string &gameName, int gameVersion)
   {
      bool result = false;

      mIsServer = false;
      /// HACK - can't add a component while in a tick... so, we hacked it! 
      LOG_ALWAYS("HACK MESSAGE - WE CANNOT ADD A COMPONENT WHILE IN A tick MESSAGE. FIX ME!");
      //dtCore::RefPtr<dtNetGM::ClientNetworkComponent> clientComp =
      //   new dtNetGM::ClientNetworkComponent(gameName, gameVersion);
      //GetGameManager()->AddComponent(*clientComp, dtGame::GameManager::ComponentPriority::NORMAL);
      dtCore::RefPtr<dtNetGM::ClientNetworkComponent> clientComp =
         dynamic_cast<dtNetGM::ClientNetworkComponent*>(mNetworkComp.get());
      result = clientComp->SetupClient(serverIPAddress, serverPort);

      if (result)
      {
         mIsConnectedToNetwork = true;
         mNetworkComp = clientComp;
      }
      mIsServer = false;

      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::DisconnectFromNetwork()
   {
      if (mIsConnectedToNetwork)
      {
         // SERVER
         if (mIsServer)
         {
            dtNetGM::ServerNetworkComponent *serverComponent = 
               dynamic_cast<dtNetGM::ServerNetworkComponent *>(mNetworkComp.get());
            if (serverComponent != NULL)
            {
               GetGameManager()->RemoveComponent(*serverComponent);
               // How do we disconnect the server?!?!?!!! 
               mNetworkComp = NULL;
            }
         }

         // CLIENT
         else 
         {
            dtNetGM::ClientNetworkComponent *clientComponent = 
               dynamic_cast<dtNetGM::ClientNetworkComponent *>(mNetworkComp.get());
            if (clientComponent != NULL)
            {
               GetGameManager()->RemoveComponent(*clientComponent);
               // How do we disconnect the client?!?!?!!! 
               mNetworkComp = NULL;
            }
         }

         mIsConnectedToNetwork = false;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::HandleStateChangeMessage( const SimCore::Components::GameStateChangedMessage& stateChange )
   {
      const SimCore::Components::StateType& state = stateChange.GetNewState();
      LOG_WARNING("Changing to stage[" + state.GetName() + "].");

      // Note - when we change the player status, it gets published when the actor is ticked.

      if (state == SimCore::Components::StateType::STATE_MENU)
      {
      }
      else if (state == SimCore::Components::StateType::STATE_LOADING)
      {
         if (mPlayerStatus != NULL)
            mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::LOADING);
         LoadMaps(mMapName);
         // When loaded, we trap the MAP_LOADED message and finish our setup
      }
      else if (state == NetDemoState::STATE_UNLOAD)
      {
         if (mPlayerStatus != NULL)
            mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::UNKNOWN);
         HandleUnloadingState();
      }
      else if (state == NetDemoState::STATE_GAME_RUNNING)
      {
         if (mPlayerStatus != NULL)
            mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_GAME_ALIVE);
      }
      else if (state == NetDemoState::STATE_GAME_READYROOM)
      {
         if (mPlayerStatus != NULL)
            mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_GAME_READYROOM);

         // CMM - Hack - bypass this screen for now, since we don't have one
         HandleTransition(SimCore::Components::EventType::TRANSITION_FORWARD); // to loading
      }
      else if (state == SimCore::Components::StateType::STATE_SHUTDOWN )
      {
         GetGameManager()->GetApplication().Quit();
      }
      else if (state == NetDemoState::STATE_LOBBY)
      {
         //if (mPlayerStatus != NULL)
         //   mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_LOBBY);

         // CMM - TEMP HACK - Eventually, this is a full game state with a UI and this 
         // Gets done from the UI somewhere, somehow. 

         bool success = false;
         dtUtil::ConfigProperties& configParams = GetGameManager()->GetConfiguration();
         //bool isGMOn = dtUtil::ToType<bool>(configParams.GetConfigPropertyValue("dtNetGM.On", "false"));
         const std::string role = configParams.GetConfigPropertyValue("dtNetGM.Role", "server");
         int serverPort = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.ServerPort", "7329"));
         const std::string gameName = configParams.GetConfigPropertyValue("dtNetGM.GameName", "NetDemo");
         int gameVersion = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.GameVersion", "1"));
         const std::string host = configParams.GetConfigPropertyValue("dtNetGM.ServerHost", "127.0.0.1");

         if (role == "Server" || role == "server" || role == "SERVER")
         {
            success = JoinNetworkAsServer(serverPort, gameName, gameVersion);
         }
         else if (role == "Client" || role == "client" || role == "CLIENT")
         {
            success = JoinNetworkAsClient(serverPort, host, gameName, gameVersion);
         }

         if (success) // start map load
         {
            HandleTransition(SimCore::Components::EventType::TRANSITION_FORWARD); // to loading
         }
         else  // go back
         {
            HandleTransition(SimCore::Components::EventType::TRANSITION_BACK); // back to menu
         }

      }
   }


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

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::HandleUnloadingState()
   {
      // Have to disconnect first, else we will get network messages about stuff we don't understand.
      DisconnectFromNetwork();

      GetGameManager()->CloseCurrentMap(); 
      // When the map is unloaded, we get the UNLOADED msg and change our states back to lobby.
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::UnloadCurrentTerrain()
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
      if (mCurrentTerrainPrototypeName.empty())
         return;

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
