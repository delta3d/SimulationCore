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
#include <dtGame/gmsettings.h>
#include <dtGame/basemessages.h>
#include <dtNetGM/clientnetworkcomponent.h>
#include <dtNetGM/servernetworkcomponent.h>

#include <dtActors/engineactorregistry.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/Actors/FourWheelVehicleActor.h>
#include <SimCore/ActComps/TrailerHitchActComp.h>
#include <SimCore/Components/GameState/GameStateChangeMessage.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <SimCore/Utilities.h>

#include "ActorRegistry.h"
#include "Actors/FortActor.h"
#include "Actors/PlayerStatusActor.h"
#include "Actors/ServerGameStatusActor.h"
#include "Components/GameLogicComponent.h"
#include "Components/SpawnComponent.h"
#include "NetDemoMessages.h"
#include "NetDemoMessageTypes.h"
#include "States.h"

namespace NetDemo
{
   const std::string GameLogicComponent::TIMER_UPDATE_TERRAIN("FORCE_UPDATE_OF_TERRAIN");
   const std::string GameLogicComponent::HOVER_VEHICLE_PROTOTYPE("NetDemo.DefaultHoverVehiclePrototype");
   const std::string GameLogicComponent::WHEELED_VEHICLE_PROTOTYPE("NetDemo.DefaultWheeledVehiclePrototype");
   const std::string GameLogicComponent::WHEELED_VEHICLE_TRAILER_PROTOTYPE("NetDemo.DefaultWheeledVehicleTrailerPrototype");

   //////////////////////////////////////////////////////////////////////////
   GameLogicComponent::GameLogicComponent(const std::string& name)
      : BaseClass(name)
      , mLogger(dtUtil::Log::GetInstance("GameLogicComponent.cpp"))
      , mIsServer(false)
      , mIsConnectedToNetwork(false)
      , mStartTheGameOnNextGameRunning(false)
      , mVehicleType(&PlayerStatusActor::VehicleTypeEnum::FOUR_WHEEL)
      , mGameDifficulty(1)
   {
      // Register application-specific states.
      AddState(&NetDemoState::STATE_CONNECTING);
      AddState(&NetDemoState::STATE_LOBBY);
      AddState(&NetDemoState::STATE_UNLOAD);
      AddState(&NetDemoState::STATE_GAME_RUNNING);
      AddState(&NetDemoState::STATE_GAME_READYROOM);
      AddState(&NetDemoState::STATE_GAME_DEAD);
      AddState(&NetDemoState::STATE_GAME_OPTIONS);
      AddState(&NetDemoState::STATE_GAME_QUIT);
      AddState(&NetDemoState::STATE_GAME_UNKNOWN);
      AddState(&NetDemoState::STATE_SCORE_SCREEN);
   }

   //////////////////////////////////////////////////////////////////////////
   bool GameLogicComponent::IsRunningState(const GameLogicComponent::GameStateType& state) const
   {
      return &state == &NetDemoState::STATE_GAME_RUNNING
         || &state == &NetDemoState::STATE_GAME_READYROOM
         || &state == &NetDemoState::STATE_GAME_DEAD
         || &state == &NetDemoState::STATE_GAME_OPTIONS
         || &state == &NetDemoState::STATE_GAME_QUIT
         || &state == &NetDemoState::STATE_GAME_UNKNOWN;
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::SetVehicleType(PlayerStatusActor::VehicleTypeEnum& vehicleType)
   {
      mVehicleType = &vehicleType;
   }

   //////////////////////////////////////////////////////////////////////////
   const PlayerStatusActor::VehicleTypeEnum& GameLogicComponent::GetVehicleType() const
   {
      return *mVehicleType;
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::OnAddedToGM()
   {
      //dtUtil::ConfigProperties& configParams = GetGameManager()->GetConfiguration();
      //const std::string role = configParams.GetConfigPropertyValue("dtNetGM.Role", "server");
      //int serverPort = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.ServerPort", "7329"));
      static const std::string gameName = "NetDemo";// configParams.GetConfigPropertyValue("dtNetGM.GameName", "NetDemo");
      int gameVersion = 1; //dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.GameVersion", "1"));
      //const std::string host = configParams.GetConfigPropertyValue("dtNetGM.ServerHost", "127.0.0.1");

      // We can't add components while in a tick message, so we add both components up front.
      // SERVER COMPONENT
      dtCore::RefPtr<dtNetGM::ServerNetworkComponent> serverComp =
         new dtNetGM::ServerNetworkComponent(gameName, gameVersion);
      GetGameManager()->AddComponent(*serverComp, dtGame::GameManager::ComponentPriority::NORMAL);
      // CLIENT COMPONENT
      dtCore::RefPtr<dtNetGM::ClientNetworkComponent> clientComp =
         new dtNetGM::ClientNetworkComponent(gameName, gameVersion);
      GetGameManager()->AddComponent(*clientComp, dtGame::GameManager::ComponentPriority::NORMAL);

      // Get the vehicle type set in the config file.
      std::string vehicleTypeValue = GetGameManager()->GetConfiguration().GetConfigPropertyValue("NetDemo.DefaultPlayMode","FOUR_WHEEL");
      PlayerStatusActor::VehicleTypeEnum* vehicleType = PlayerStatusActor::VehicleTypeEnum::GetValueForName(vehicleTypeValue);
      if(vehicleType != NULL)
      {
         mVehicleType = vehicleType;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::ProcessMessage(const dtGame::Message& msg)
   {
      BaseClass::ProcessMessage(msg);

      const dtGame::MessageType& messageType = msg.GetMessageType();

      // Process game state changes.
      if (messageType == dtGame::MessageType::NETSERVER_ACCEPT_CONNECTION)
      {
         LOG_ALWAYS("The server accepted the connection request message.");
      }
      else if (messageType == SimCore::MessageType::GAME_STATE_CHANGED)
      {
         HandleStateChangeMessage(static_cast<const SimCore::Components::GameStateChangedMessage&>(msg));
      }
      else if (dtGame::MessageType::INFO_MAP_LOADED == messageType)
      {
         HandleMapLoaded();
      }
      else if (dtGame::MessageType::INFO_MAP_UNLOADED == messageType)
      {
         mCurrentTerrainDrawActor = NULL;
         mServerGameStatusProxy = NULL;

         // We do this after the map is unloaded so that we delete our objects on the network.
         DisconnectFromNetwork();

         DoStateTransition(&Transition::TRANSITION_FORWARD);
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == messageType)
      {
         HandleActorUpdateMessage(msg);
      }
      else if (dtGame::MessageType::INFO_TIMER_ELAPSED == messageType)
      {
         HandleTimerElapsedMessage(msg);
      }
      else if (NetDemo::MessageType::ENTITY_ACTION == messageType)
      {
         HandleEntityActionMessage(msg);
      }
      else if (dtGame::MessageType::INFO_CLIENT_CONNECTED == messageType)
      {
         HandleClientConnected(msg);
      }

      // Something about Game State changing here
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::InitializePlayer()
   {
      dtCore::RefPtr<dtGame::GameActorProxy> ap;

      // Every player always has a player actor. On some apps, it is an overkill, but
      // we use it anyway, for consistency. It allows tools, position, ability to have an avatar, walk, run, jump, etc.
      //GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, ap);
      GetGameManager()->CreateActor(*NetDemo::NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE, ap);
      //mPlayerStatus = static_cast<SimCore::Actors::PlayerActor*>(ap->GetDrawable());
      mPlayerStatus = static_cast<NetDemo::PlayerStatusActor*>(ap->GetDrawable());
      // make the camera a child
      dtCore::Camera* cam = GetGameManager()->GetApplication().GetCamera();
      cam->Emancipate();
      mPlayerStatus->AddChild(cam);
      mPlayerStatus->SetName("Player (Unknown)");

      // Hack stuff - move this to UI - user selected and all that.
      mPlayerStatus->SetIsServer(mIsServer);
      mPlayerStatus->SetTerrainPreference("Terrain1");
      //mPlayerStatus->SetTerrainPreference("Terrains:Level_DriverDemo.ive");
      mPlayerStatus->SetTeamNumber(1);
      mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_LOBBY);
      mPlayerStatus->SetVehiclePreference(*mVehicleType);

      GetGameManager()->AddActor(mPlayerStatus->GetGameActorProxy(), false, true);


      //////////////TEMP HACK
      // Set the starting position from a player start actor in the map.
      //dtActors::PlayerStartActorProxy* startPosProxy = NULL;
      //GetGameManager()->FindActorByType(*dtActors::EngineActorRegistry::PLAYER_START_ACTOR_TYPE, startPosProxy);
      //if (startPosProxy != NULL)
      //{
      //   dtCore::Transformable* actor = NULL;
      //   startPosProxy->GetDrawable(actor);
      //   dtCore::Transform xform;
      //   actor->GetTransform(xform);
      //   mPlayerStatus->SetTransform(xform);
      //}
      ////////////////////////
   }

   ////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleActorUpdateMessage(const dtGame::Message& msg)
   {
      const dtGame::ActorUpdateMessage &updateMessage =
         static_cast<const dtGame::ActorUpdateMessage&> (msg);

      // PLAYER STATUS
      if (mIsServer && updateMessage.GetActorType() == NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE)
      {
         // Find the actor in the GM
         PlayerStatusActorProxy* statusActorProxy = NULL;
         GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId(), statusActorProxy);
         if (statusActorProxy == NULL)
         {
            return;
         }

         PlayerStatusActor* statusActor = NULL;
         statusActorProxy->GetDrawable(statusActor);

         HandlePlayerStatusUpdated(statusActor);

      }

      // SERVER STATUS
      else if (!mIsServer && updateMessage.GetActorType() == NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE)
      {
         // Find the actor in the GM
         dtGame::GameActorProxy* gap = GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId());
         if (gap == NULL)
         {
            return;
         }
         ServerGameStatusActor* serverStatus = NULL;
         gap->GetDrawable(serverStatus);

         // If not the server, do a print out...
         std::ostringstream oss;
         oss << "Server Status Updated: \"" << serverStatus->GetGameStatus().GetName() << "], Wave[" <<
            serverStatus->GetWaveNumber() << "] Players[" << serverStatus->GetNumPlayers() << "] TimeLeft[" <<
            serverStatus->GetTimeLeftInCurState() << "], EnemiesKilt[" << serverStatus->GetNumEnemiesKilled() << "].";
         LOG_INFO(oss.str());
      }
   }

   ////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandlePlayerStatusUpdated(PlayerStatusActor *statusActor)
   {
      // If it's a local actor and we are the server, then we need to create our terrain.
      // The terrain will be published to the clients.
      if (!statusActor->IsRemote())
      {
         if (statusActor->GetIsServer() && statusActor->GetTerrainPreference() != mCurrentTerrainPrototypeName)
         {
            mLogger.LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
               "Creating new terrain [%s] on the Server.", statusActor->GetTerrainPreference().c_str());
            mTerrainToLoad = statusActor->GetTerrainPreference();
            UnloadCurrentTerrain();
            mCurrentTerrainPrototypeName = mTerrainToLoad;
            mTerrainToLoad = "";
            LoadNewTerrain();
         }
      }

      // It's remote.
      else if (mIsServer && mServerGameStatusProxy.valid())
      {
         // get the count of player status actors in gm.
         std::vector<dtCore::ActorProxy*> playerActors;
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE, playerActors);
         mServerGameStatusProxy->GetActorAsGameStatus().SetNumPlayers(playerActors.size());

         // (todo - future) Calculate the number of teams
      }

   }

   ////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleTimerElapsedMessage(const dtGame::Message& msg)
   {
      const dtGame::TimerElapsedMessage& timerMsg =
         static_cast<const dtGame::TimerElapsedMessage&>(msg);

      // Force an update of the terrain actor. This will pass the actor out to late-joining clients.
      if(TIMER_UPDATE_TERRAIN == timerMsg.GetTimerName() && mCurrentTerrainDrawActor.valid())
      {
         //mLogger.LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,"Sending out a terrain actor update.");
         mCurrentTerrainDrawActor->GetGameActorProxy().NotifyFullActorUpdate();
      }

   }

   ////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleEntityActionMessage(const dtGame::Message& msg)
   {
      const NetDemo::EntityActionMessage& actionMessage = static_cast<const NetDemo::EntityActionMessage&>(msg);

      if(actionMessage.GetAction() == EntityAction::SCORE)
      {
         if(actionMessage.GetSource() == GetGameManager()->GetMachineInfo())
         {
            mPlayerStatus->UpdateScore(actionMessage.GetPoints());
         }
      }
   }

   ////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleClientConnected(const dtGame::Message& msg)
   {
      // Whenever a new client is connected, spin through all our locally owned
      // and published actors and do a NotifyFullActorUpdate() to let other clients 
      // know what exists.
      std::vector<dtGame::GameActorProxy*> gameActorProxies;
      GetGameManager()->GetAllGameActors(gameActorProxies);
      unsigned numProxies = gameActorProxies.size();
      for (unsigned int i = 0; i < numProxies; ++i)
      {
         dtGame::GameActorProxy* gap = gameActorProxies[i];
         if (gap->IsPublished() && !gap->IsRemote())
         {
            gap->NotifyFullActorUpdate();
         }
      }
   }
   
   ////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleMapLoaded()
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

      CreateServerSideActors();

      // Now we can go to the ready room.
      DoStateTransition(&Transition::TRANSITION_FORWARD);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GameLogicComponent::JoinNetwork(const std::string& role, int serverPort, const std::string& hostIP)
   {
      bool success = false;

      //if (role == "Server" || role == "server" || role == "SERVER")
      if (GetGameManager()->GetGMSettings().IsServerRole())
      {
         success = JoinNetworkAsServer(serverPort);
         if (success)
         {
            GetGameManager()->AddComponent(
               *new SpawnComponent(), dtGame::GameManager::ComponentPriority::NORMAL);
         }
      }
      else // if (role == "Client" || role == "client" || role == "CLIENT")
      {
         success = JoinNetworkAsClient(serverPort, hostIP);
      }

      return success;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GameLogicComponent::JoinNetworkAsServer(int serverPort)
   {
      bool result = false;

      dtNetGM::ServerNetworkComponent* serverComp;
      GetGameManager()->GetComponentByName(dtNetGM::ServerNetworkComponent::DEFAULT_NAME, serverComp);
      if( serverComp != NULL )
      {
         result = serverComp->SetupServer(serverPort);
         if (result)
         {
            mIsConnectedToNetwork = true;
            mIsServer = true;
            mNetworkComp = serverComp;

            // Start a repeating timer - to update terrain
            GetGameManager()->SetTimer(TIMER_UPDATE_TERRAIN, NULL, 3.0f, true, true);
         }
      }
      else
      {
         LOG_ERROR("Critical Error - There is no Server Network Component by the default name.");
      }

      return result;
   }

   ///////////////////////////////////////////////////////////
   bool GameLogicComponent::JoinNetworkAsClient(int serverPort, const std::string& serverIPAddress)
   {
      bool result = false;
      mIsServer = false;

      dtNetGM::ClientNetworkComponent* clientComp;
      GetGameManager()->GetComponentByName(dtNetGM::ClientNetworkComponent::DEFAULT_NAME, clientComp);
      if( clientComp != NULL )
      {
         result = clientComp->SetupClient(serverIPAddress, serverPort);
         if (result)
         {
            mIsConnectedToNetwork = true;
            mNetworkComp = clientComp;
         }
         mIsServer = false;
      }

      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::DisconnectFromNetwork()
   {
      if (mIsConnectedToNetwork)
      {
         mNetworkComp->Disconnect();
         // SERVER
         if (mIsServer)
         {
            GetGameManager()->ClearTimer(TIMER_UPDATE_TERRAIN, NULL);
         }

         mNetworkComp = NULL;
         mIsConnectedToNetwork = false;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleStateChangeMessage( const SimCore::Components::GameStateChangedMessage& stateChange )
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
         SimCore::Utils::LoadMaps(*GetGameManager(), mMapName);
         // When loaded, we trap the MAP_LOADED message and finish our setup
      }
      else if (state == NetDemoState::STATE_UNLOAD)
      {
         HandleUnloadingState();
      }
      else if (state == NetDemoState::STATE_GAME_RUNNING)
      {
         HandleGameRunningState();
      }
      else if (state == NetDemoState::STATE_GAME_READYROOM)
      {
         if (mPlayerStatus != NULL)
            mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_GAME_READYROOM);
         if (mServerGameStatusProxy != NULL)
            mServerGameStatusProxy->GetActorAsGameStatus().SetGameStatus
               (ServerGameStatusActor::ServerGameStatusEnum::READY_ROOM);
         mStartTheGameOnNextGameRunning = true;

         // Clear out any previous vehicle, in case we were previously in a game.
         ClearPreviousGameStuff();

         // curt - hack - replace this with the GUI COMPONENT
         //DoStateTransition(&Transition::TRANSITION_FORWARD);
      }
      else if (state == SimCore::Components::StateType::STATE_SHUTDOWN )
      {
         GetGameManager()->GetApplication().Quit();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleUnloadingState()
   {
      // Have to disconnect first, else we will get network messages about stuff we don't understand.
      //DisconnectFromNetwork();

      // Probably not necessary (since close map deletes all), but clear out stuff we already created
      //ClearPreviousGameStuff();
      mPlayerStatus = NULL;
      mCurrentTerrainDrawActor = NULL;
      mServerGameStatusProxy = NULL;
      mPlayerOwnedVehicle = NULL;
      mServerCreatedFortActor = NULL;

      GetGameManager()->DeleteAllActors();

      GetGameManager()->CloseCurrentMap();
      // When the map is unloaded, we get the UNLOADED msg and change our states back to lobby.
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::UnloadCurrentTerrain()
   {
      if (mCurrentTerrainDrawActor.valid())
      {
         // Delete the visible terrain
         GetGameManager()->DeleteActor(mCurrentTerrainDrawActor->GetGameActorProxy());
         mCurrentTerrainDrawActor = NULL;
         mCurrentTerrainPrototypeName = "";
         //mLogger.LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__, "Now unloading the previous terrain.");
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::CreatePrototypes(const dtCore::ActorType& type)
   {
      std::vector<dtCore::ActorProxy*> actors;
      GetGameManager()->FindPrototypesByActorType(type, actors);
      for (unsigned i = 0; i < actors.size(); ++i)
      {
         dtCore::ActorProxy* curProto  = actors[i];
         dtCore::RefPtr<dtGame::GameActorProxy> newActor;
         GetGameManager()->CreateActorFromPrototype(curProto->GetId(), newActor);
         if (newActor == NULL)
         {
            LOG_ERROR("Creating prototype \"" + curProto->GetName() + "\" with type \"" + type.GetFullName() + "\" failed for an unknown reason.");
         }
         else
         {
            GetGameManager()->AddActor(*newActor, false, true);
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::LoadNewTerrain()
   {
      if (mCurrentTerrainPrototypeName.empty())
         return;

      dtCore::RefPtr<SimCore::Actors::TerrainActorProxy> newDrawLandActorProxy = NULL;
      SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
         mCurrentTerrainPrototypeName, newDrawLandActorProxy, "Check your additional maps in config.xml (compare to config_example.xml).");
      newDrawLandActorProxy->SetName("Terrain"); // has to be named 'Terrain' or it won't do ground clamping and other stuff
      mCurrentTerrainDrawActor = dynamic_cast<SimCore::Actors::TerrainActor*>
         (newDrawLandActorProxy->GetDrawable());
      dtCore::RefPtr<dtGame::DRPublishingActComp> drpac = NULL;
      mCurrentTerrainDrawActor->GetComponent(drpac);
      if (!drpac.valid())
      {
         // Add a DR publishing component to the terrain so it sends heartbeats.
         drpac = new dtGame::DRPublishingActComp(false);
         mCurrentTerrainDrawActor->AddComponent(*drpac);
         drpac->SetMaxUpdateSendRate(0.01f);
      }

      GetGameManager()->AddActor(*newDrawLandActorProxy, false, true);
   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::Actors::TerrainActor* GameLogicComponent::GetCurrentTerrainDrawActor()
   {
      return dynamic_cast<SimCore::Actors::TerrainActor *>(mCurrentTerrainDrawActor.get());
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::SetIsServer(bool newValue)
   {
      if (!mPlayerStatus.valid()) mIsServer = newValue;
   }

   //////////////////////////////////////////////////////////////////////////
   bool GameLogicComponent::GetIsServer()
   {
      return mIsServer;
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::SetMapName(const std::string& mapName)
   {
      mMapName = mapName;
   }

   //////////////////////////////////////////////////////////////////////////
   const std::string& GameLogicComponent::GetMapName() const
   {
      return mMapName;
   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::CreateServerSideActors()
   {
      if (mIsServer)
      {
         //////////////////////////
         // Create the server game status actor
         dtCore::RefPtr<dtGame::GameActorProxy> ap;
         GetGameManager()->CreateActor(*NetDemo::NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE, ap);
         mServerGameStatusProxy = dynamic_cast<ServerGameStatusActorProxy*> (ap.get());
         ServerGameStatusActor &gameStatus = mServerGameStatusProxy->GetActorAsGameStatus();
         gameStatus.SetNumTeams(1);
         gameStatus.SetNumPlayers(1); // we account for ourself already
         gameStatus.SetGameDifficulty(mGameDifficulty);
         gameStatus.SetGameStatus(ServerGameStatusActor::ServerGameStatusEnum::WAVE_ABOUT_TO_START);
         gameStatus.SetNumEnemiesKilled(0);
         gameStatus.SetWaveNumber(1);
         gameStatus.SetTimeLeftInCurState(10.0f);
         GetGameManager()->AddActor(*(mServerGameStatusProxy.get()), false, true);
      }

   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::HandleGameRunningState()
   {
      mPlayerStatus->SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum::IN_GAME_ALIVE);

      if (mStartTheGameOnNextGameRunning)
      {
         mStartTheGameOnNextGameRunning = false;

         ////// Server stuff
         if (mIsServer)
         {
            // Start our waves up
            mServerGameStatusProxy->GetActorAsGameStatus().StartTheGame();
         }

         /////// CLIENT STUFF

         // Create a vehicle and stick our player in it.
         if (mPlayerStatus->GetVehiclePreference() != PlayerStatusActor::VehicleTypeEnum::OBSERVER)
         {
            if (mPlayerStatus->GetVehiclePreference() == PlayerStatusActor::VehicleTypeEnum::HOVER)
            {
               std::string vehiclePrototype = GetGameManager()->GetConfiguration().GetConfigPropertyValue(HOVER_VEHICLE_PROTOTYPE,"Hover Vehicle");
               SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
                        vehiclePrototype, mPlayerOwnedVehicle, "Check your additional maps in config.xml (compare to config_example.xml).");
            }
            else if (mPlayerStatus->GetVehiclePreference() == PlayerStatusActor::VehicleTypeEnum::FOUR_WHEEL)
            {
               std::string vehiclePrototype = GetGameManager()->GetConfiguration().GetConfigPropertyValue(WHEELED_VEHICLE_PROTOTYPE, "Truck");

               SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
                        vehiclePrototype, mPlayerOwnedVehicle, "Check your additional maps in config.xml (compare to config_example.xml).");

               std::string trailerPrototype = GetGameManager()->GetConfiguration().GetConfigPropertyValue(WHEELED_VEHICLE_TRAILER_PROTOTYPE, "");
               if (!trailerPrototype.empty())
               {
                  dtCore::RefPtr<SimCore::Actors::FourWheelVehicleActorProxy> trailer;
                  SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
                           trailerPrototype, trailer, "Check your additional maps in config.xml (compare to config_example.xml).");

                  dtCore::RefPtr<SimCore::ActComps::TrailerHitchActComp> trailerAC = new SimCore::ActComps::TrailerHitchActComp();

                  mPlayerOwnedVehicle->AddComponent(*trailerAC);

                  trailerAC->SetHitchType(SimCore::ActComps::HitchTypeEnum::HITCH_TYPE_5TH_WHEEL);
                  trailerAC->SetTrailerActorId(trailer->GetId());

                  GetGameManager()->AddActor(*trailer, false, true);
               }
            }

            SimCore::Actors::BasePhysicsVehicleActor* vehicleActor = NULL;
            mPlayerOwnedVehicle->GetDrawable(vehicleActor);
            vehicleActor->SetHasDriver(true);
            GetGameManager()->AddActor(*mPlayerOwnedVehicle, false, true);

            mPlayerStatus->SetAttachedVehicleID(mPlayerOwnedVehicle->GetId());
         }
      }

   }

   //////////////////////////////////////////////////////////////////////////
   void GameLogicComponent::ClearPreviousGameStuff()
   {
      if (mPlayerOwnedVehicle.valid())
      {
         mPlayerStatus->SetAttachedVehicleID(dtCore::UniqueId(""));
         GetGameManager()->DeleteActor(*mPlayerOwnedVehicle);
         mPlayerOwnedVehicle = NULL;
      }

      if (mServerCreatedFortActor.valid())
      {
         GetGameManager()->DeleteActor(*mServerCreatedFortActor);
         mServerCreatedFortActor = NULL;
      }

   }



}
