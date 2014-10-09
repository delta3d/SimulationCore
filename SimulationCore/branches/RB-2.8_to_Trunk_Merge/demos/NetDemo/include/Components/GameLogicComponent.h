/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @author David Guthrie, Curtiss Murphy
*/
#ifndef RES_GAMEAPPCOMPONENT_H_
#define RES_GAMEAPPCOMPONENT_H_

#include <SimCore/Components/BaseGameAppComponent.h>
#include <SimCore/Actors/StealthActor.h>
#include <dtGame/gamestatecomponent.h>

#include <dtNetGM/networkcomponent.h>

#include <Actors/PlayerStatusActor.h>

namespace dtUtil
{
   class Log;
}

namespace SimCore
{
   namespace Actors
   {
      class TerrainActor;
      class BasePhysicsVehicleActorProxy;
      class WeaponActor;
   }

   namespace Components
   {
      class GameStateChangedMessage;
      class EventType;
   }
}

namespace NetDemo
{
   class ServerGameStatusActorProxy;
   class FortActorProxy;


   /**
    * This component is responsible for some of the core game logic such as changing game
    * states and responding to major messages from the server. It also listens for server
    * players and manages/creates the PlayerStatusActor.
    */
   class GameLogicComponent : public dtGame::GameStateComponent
   {
      /// Internal data struct/class for debug specific data. Member vars are public to act like a struct. Accessed by InputComponent and HUDScreen
      class DebugInformation 
      {
      public:
         /// Constructor
         DebugInformation()
            : mShowDebugWindow(false)
            , mDRPublishRate(1)
            , mDRUseFixedBlend(false)
            , mDRPublishAngularVel(false)
            , mDRAvgError(0.0f)
            , mDRAvgSpeed(0.0f)
         {  }

         bool mShowDebugWindow;
         std::string mCurDebugVar;
         std::string mDRAlgorithm;
         std::string mDRGroundClampStatus;
         std::string mDRGhostMode;
         int mDRPublishRate;
         bool mDRUseFixedBlend; 
         bool mDRPublishAngularVel;
         float mDRAvgError; // reported by a vehicle such as the PropelledVehicleActor
         float mDRAvgSpeed; // reported by a vehicle such as the PropelledVehicleActor
      };

      //// Rest of class
      public:
         static const std::string TIMER_UPDATE_TERRAIN;
         static const std::string HOVER_VEHICLE_PROTOTYPE;
         static const std::string WHEELED_VEHICLE_PROTOTYPE;
         static const std::string WHEELED_VEHICLE_TRAILER_PROTOTYPE;

         typedef dtGame::GameStateComponent BaseClass;
         static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;

         /// Constructor
         GameLogicComponent(dtCore::SystemComponentType& type = *TYPE);

         virtual void ProcessMessage(const dtGame::Message& msg);

         void InitializePlayer();

         /// Returns the DRAW land actor as its real type. Prevents having to hold onto the real type, which allows forward declaration in the header.
         SimCore::Actors::TerrainActor* GetCurrentTerrainDrawActor();

         /// IsServer can only be set at startup, probably from the GameEntryPoint()
         void SetIsServer(bool newValue);
         bool GetIsServer();
         /// Are we currently connected (as either a server or a client)?
         bool GetIsConnectedToNetwork() {return mIsConnectedToNetwork; }

         void SetMapName(const std::string& mapName);
         const std::string& GetMapName() const;

         /// Called from the UI after it knows how the user wants to connect
         bool JoinNetwork(const std::string& role, int serverPort, const std::string& hostIP);
         bool JoinNetworkAsServer(int serverPort);
         bool JoinNetworkAsClient(int serverPort, const std::string &serverIPAddress);

         virtual void OnAddedToGM();

         void HandlePlayerStatusUpdated(PlayerStatusActor *statusActor);

         void CreateServerSideActors();

         typedef dtGame::StateType GameStateType;
         bool IsRunningState(const GameStateType& state) const;

         /**
          * Convenience method for finding an actor and casting it to a specific actor type.
          * @param actorId Id of the actor to be found.
          * @param outActor Pointer to capture the actor that may be found.
          * @return TRUE if the actor was found.
          */
         template<typename T_Actor>
         bool FindDrawable(const dtCore::UniqueId& actorId, T_Actor*& outActor);

         void SetVehicleType(PlayerStatusActor::VehicleTypeEnum& vehicleType);
         const PlayerStatusActor::VehicleTypeEnum& GetVehicleType() const;

         /// Returns the debug info struct - directly modifiable
         DebugInformation& GetDebugInfo() { return mDebugInformation; } 

         /// Set this BEFORE the server is created.
         void SetGameDifficulty(int newValue) { mGameDifficulty = newValue; }
         int GetGameDifficulty() const { return mGameDifficulty; }

      protected:
         void HandleActorUpdateMessage(const dtGame::Message& msg);
         void HandleTimerElapsedMessage(const dtGame::Message& msg);
         void HandleEntityActionMessage(const dtGame::Message& msg);
         void HandleClientConnected(const dtGame::Message& msg);

         void CreatePrototypes(const dtCore::ActorType& type);

         void HandleUnloadingState();
         void HandleGameRunningState();
         void HandleMapLoaded();
         void DisconnectFromNetwork();
         void HandleStateChangeMessage(
            const dtGame::GameStateChangedMessage& stateChange);
         void LoadNewTerrain();
         void UnloadCurrentTerrain();
         void ClearPreviousGameStuff();

         dtUtil::Log& mLogger;

      private:
         bool mIsServer;
         bool mIsConnectedToNetwork;

         //dtCore::RefPtr<SimCore::Actors::StealthActor> mStealth;
         /// Each client & server has one player status that they are publishing.
         dtCore::RefPtr<PlayerStatusActor> mPlayerStatus;
         dtCore::RefPtr<SimCore::Actors::BasePhysicsVehicleActorProxy> mPlayerOwnedVehicle;
         dtCore::RefPtr<FortActorProxy> mServerCreatedFortActor;

         /// This holds the terrain prototype we want to load once we enter LOADING state.
         std::string mTerrainToLoad;

         /// This is our current terrain prototype name.
         std::string mCurrentTerrainPrototypeName;

         /// The current DRAWING terrain actor. This actor comes and goes - it's the visible geometry, not physics.
         dtCore::RefPtr<dtGame::GameActor> mCurrentTerrainDrawActor;

         // Reference to the State Component to control automatic transitions.
         //dtCore::ObserverPtr<StateComponent> mStateComp;

         std::string mMapName;

         /// May be either a Network or Client component. We create it when we connect
         dtCore::RefPtr<dtNetGM::NetworkComponent> mNetworkComp;

         dtCore::RefPtr<ServerGameStatusActorProxy> mServerGameStatusProxy;

         bool mStartTheGameOnNextGameRunning;

         PlayerStatusActor::VehicleTypeEnum* mVehicleType;

         DebugInformation mDebugInformation;

         int mGameDifficulty;
   };



   /////////////////////////////////////////////////////////////////////////////
   // TEMPLATE METHOD DEFINITIONS
   /////////////////////////////////////////////////////////////////////////////
   template<typename T_Actor>
   bool GameLogicComponent::FindDrawable(const dtCore::UniqueId& actorId, T_Actor*& outDrawable)
   {
      // Get the actor to which the message refers.
      dtCore::BaseActorObject* actor = NULL;
      GetGameManager()->FindActorById(actorId, actor);

      if(actor != NULL)
      {
         actor->GetDrawable(outDrawable);
      }

      return outDrawable != NULL;
   }

}
#endif /* RES_GAMEAPPCOMPONENT_H_ */
