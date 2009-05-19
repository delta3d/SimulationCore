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
#include <dtUtil/log.h>


namespace SimCore
{
   namespace Actors
   {
      class NxAgeiaTerraPageLandActor;
      class TerrainActor;
   }
}


namespace NetDemo
{
   class PlayerStatusActor;


   //////////////////////////////////////////////////////////////////////////////
   /* 
    * This component is responsible for some of the core game logic such as changing game
    * states and responding to major messages from the server. It also listens for server
    * players and manages/creates the PlayerStatusActor.
    */
   class GameAppComponent : public SimCore::Components::BaseGameAppComponent
   {
      public:
         typedef SimCore::Components::BaseGameAppComponent BaseClass;

         /// Constructor
         GameAppComponent(const std::string& name = BaseGameAppComponent::DEFAULT_NAME);

         virtual void ProcessMessage(const dtGame::Message& msg);

         void InitializePlayer();

         // Returns the PHYSICS land actor as its real type. Prevents having to hold onto the real type, which allows forward declaration in the header.
         SimCore::Actors::NxAgeiaTerraPageLandActor *GetGlobalTerrainPhysicsActor();

         // Returns the DRAW land actor as its real type. Prevents having to hold onto the real type, which allows forward declaration in the header.
         SimCore::Actors::TerrainActor *GetCurrentTerrainDrawActor();

         // IsServer can only be set at startup, probably from the GameEntryPoint()
         void SetIsServer(bool newValue) { if (!mPlayerStatus.valid()) mIsServer = newValue; }
         bool GetIsServer() { return mIsServer; }

      protected: 
         void HandleActorUpdateMessage(const dtGame::Message& msg);

         void FindThePhysicsLandActor();
         void LoadNewTerrain();
         void UnloadPreviousTerrain();
         void HandleLoadingState();

         dtUtil::Log* mLogger;

      private:
         bool mIsServer;

         //dtCore::RefPtr<SimCore::Actors::StealthActor> mStealth;
         // Each client & server has one player status that they are publishing.
         dtCore::RefPtr<PlayerStatusActor> mPlayerStatus;

         // This holds the terrain prototype we want to load once we enter LOADING state. 
         std::string mTerrainToLoad;

         // This is our current terrain prototype name.
         std::string mCurrentTerrainPrototypeName;

         // The current DRAWING terrain actor. This actor comes and goes - it's the visible geometry, not physics. 
         dtCore::RefPtr<dtGame::GameActor> mCurrentTerrainDrawActor;

         // The one and only physics actor. This one persists regardless of how many times we change terrains.
         dtCore::RefPtr<dtGame::GameActor> mGlobalTerrainPhysicsActor;
   };

}
#endif /* RES_GAMEAPPCOMPONENT_H_ */
