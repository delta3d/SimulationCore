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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Components/BaseGameAppComponent.h>
#include <SimCore/Actors/StealthActor.h>
#include <dtUtil/log.h>
#include <SimCore/Components/GameState/GameStateComponent.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   namespace Actors
   {
      class TerrainActor;
   }

   namespace Components
   {
      class GameStateChangedMessage;
      class EventType;
   }
}



namespace NetDemo
{
   class PlayerStatusActor;
   class ServerGameStatusActorProxy;


   //////////////////////////////////////////////////////////////////////////////
   /*
    * This component is responsible for some of the core game logic such as changing game
    * states and responding to major messages from the server. It also listens for server
    * players and manages/creates the PlayerStatusActor.
    */
   class GameLogicComponent : public SimCore::Components::GameStateComponent
   {
      public:
         static const std::string TIMER_UPDATE_TERRAIN;

         typedef SimCore::Components::GameStateComponent BaseClass;

         /// Constructor
         GameLogicComponent(const std::string& name = SimCore::Components::GameStateComponent::DEFAULT_NAME);

         virtual void ProcessMessage(const dtGame::Message& msg);

         void InitializePlayer();

         // Returns the DRAW land actor as its real type. Prevents having to hold onto the real type, which allows forward declaration in the header.
         SimCore::Actors::TerrainActor* GetCurrentTerrainDrawActor();

         // IsServer can only be set at startup, probably from the GameEntryPoint()
         void SetIsServer(bool newValue);
         bool GetIsServer();
         /// Are we currently connected (as either a server or a client)?
         bool GetIsConnectedToNetwork() {return mIsConnectedToNetwork; }

         void SetMapName(const std::string& mapName);
         const std::string& GetMapName() const;

         // Called from the UI after it knows how the user wants to connect
         bool JoinNetwork(const std::string& role, int serverPort, const std::string& hostIP);
         bool JoinNetworkAsServer(int serverPort);
         bool JoinNetworkAsClient(int serverPort, const std::string &serverIPAddress);

         virtual void OnAddedToGM();

         void HandlePlayerStatusUpdated(PlayerStatusActor *statusActor);

         void CreateServerSideActors();

      protected:
         void HandleActorUpdateMessage(const dtGame::Message& msg);
         void HandleTimerElapsedMessage(const dtGame::Message& msg);

         void HandleUnloadingState();
         void HandleMapLoaded();
         void DisconnectFromNetwork();
         void HandleStateChangeMessage(
            const SimCore::Components::GameStateChangedMessage& stateChange);
         void LoadNewTerrain();
         void UnloadCurrentTerrain();

         dtUtil::Log* mLogger;

      private:
         bool mIsServer;
         bool mIsConnectedToNetwork;

         //dtCore::RefPtr<SimCore::Actors::StealthActor> mStealth;
         // Each client & server has one player status that they are publishing.
         dtCore::RefPtr<PlayerStatusActor> mPlayerStatus;

         // This holds the terrain prototype we want to load once we enter LOADING state.
         std::string mTerrainToLoad;

         // This is our current terrain prototype name.
         std::string mCurrentTerrainPrototypeName;

         // The current DRAWING terrain actor. This actor comes and goes - it's the visible geometry, not physics.
         dtCore::RefPtr<dtGame::GameActor> mCurrentTerrainDrawActor;

         // Reference to the State Component to control automatic transitions.
         //dtCore::ObserverPtr<StateComponent> mStateComp;

         std::string mMapName;

         // May be either a Network or Client component. We create it when we connect
         dtCore::RefPtr<dtGame::GMComponent> mNetworkComp;

         dtCore::RefPtr<ServerGameStatusActorProxy> mServerGameStatusProxy;

         bool mStartTheGameOnNextGameRunning;
   };

}
#endif /* RES_GAMEAPPCOMPONENT_H_ */
