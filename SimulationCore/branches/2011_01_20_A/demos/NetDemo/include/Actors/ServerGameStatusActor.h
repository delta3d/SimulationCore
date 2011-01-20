/* -*-c++-*-
* Driver Demo - PlayerStatusActor (.cpp & .h) - Using 'The MIT License'
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
* @author Curtiss Murphy
*/
#ifndef _SERVER_GAME_STATUS_ACTOR_
#define _SERVER_GAME_STATUS_ACTOR_

#include <DemoExport.h>

#include <dtGame/gameactor.h>
//#include <HoverVehiclePhysicsActComp.h>
//#include <SimCore/PhysicsTypes.h>
//#include <SimCore/Actors/BasePhysicsVehicleActor.h>
//#include <SimCore/Actors/PlayerActor.h>


namespace NetDemo
{
   class ServerGameStatusActorProxy;


   ////////////////////////////////////////////////////////////////////////////////
   /* This actor holds the status of the game. The server creates one of these once it enters
    * the Ready Room and keeps publishing every second until it dies. This actor 
    * tracks the number of enemies, the wave status, timer count downs, etc... 
    */
   class NETDEMO_EXPORT ServerGameStatusActor : public dtGame::GameActor
   {
      public:

         ////////////////////////////////////////////////////////////////////////////////
         // This enum describes the current state of the game - from the server's perspective. 
         ////////////////////////////////////////////////////////////////////////////////
         class NETDEMO_EXPORT ServerGameStatusEnum : public dtUtil::Enumeration
         {
            DECLARE_ENUM(ServerGameStatusEnum);
         public:
            static ServerGameStatusEnum UNKNOWN;
            static ServerGameStatusEnum WAVE_ABOUT_TO_START;
            static ServerGameStatusEnum WAVE_IN_PROGRESS;
            static ServerGameStatusEnum WAVE_ENDING;
            static ServerGameStatusEnum GAME_PAUSE;
            static ServerGameStatusEnum GAME_OVER;
            static ServerGameStatusEnum READY_ROOM;
         private:
            ServerGameStatusEnum(const std::string &name) : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
         };


         typedef dtGame::GameActor BaseClass;

         /// Constructor
         ServerGameStatusActor (ServerGameStatusActorProxy &proxy);

      protected:
         /// Destructor
         virtual ~ServerGameStatusActor();

      // INHERITED PUBLIC
      public:

         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);
         virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);

      public:

         // Publishes an actor update message for the important properties.  
         void PublishGameStatus();


         /////////////////////////////////////////////////////////////
         // PROPERTIES 

         // Player Status - Property
         void SetGameStatus(ServerGameStatusActor::ServerGameStatusEnum &newValue);
         ServerGameStatusActor::ServerGameStatusEnum& GetGameStatus() const;

         // WaveNumber - Property
         void SetWaveNumber(int newValue);
         int GetWaveNumber() const { return mWaveNumber; }

         // NumEnemiesKilled - Property
         void SetNumEnemiesKilled(int newValue);
         int GetNumEnemiesKilled() const { return mNumEnemiesKilled; }

         // TimeLeftInCurState - Property
         void SetTimeLeftInCurState(float newValue);
         float GetTimeLeftInCurState() const { return mTimeLeftInCurState; }

         // LastPublishTime - Property
         void SetLastPublishTime(double newValue);
         double GetLastPublishTime() const { return mLastPublishTime; }

         // GameDifficulty - Property
         void SetGameDifficulty(int newValue);
         int GetGameDifficulty() const { return mGameDifficulty; }

         // NumPlayers - Property
         void SetNumPlayers(int newValue);
         int GetNumPlayers() const { return mNumPlayers; }

         // NumTeams - Property
         void SetNumTeams(int newValue);
         int GetNumTeams() const { return mNumTeams; }

         // MaxNumWaves- Property
         void SetMaxNumWaves(int newValue) { mMaxNumberWaves = newValue; }
         int GetMaxNumWaves() const { return mMaxNumberWaves; }


         /////////////////////////////////////////////////////
         /////////////////////////////////////////////////////


         // WaveDuration - how long each wave lasts
         void SetWaveDuration(float newValue) { mWaveDuration = newValue; }
         float GetWaveDuration() const { return mWaveDuration; }

         // WaveStartDuration - how long the game pauses at the start of each wave
         void SetWaveStartDuration(float newValue) { mWaveStartDuration = newValue; }
         float GetWaveStartDuration() const { return mWaveStartDuration; }

         // Time between doing updates. Not published, set in the server constructor.  
         void SetTimeBetweenUpdates(float newValue) { mTimeBetweenUpdates = newValue; }
         float GetTimeBetweenUpdates() { return mTimeBetweenUpdates; }

         /// Starts the timing, etc - Initiated by the GameLogicComponent - on the server. 
         void StartTheGame();

      protected:

         /// Checks the game state and the time remaining. Changes states as needed. Returns true if we need to publish an update
         bool ProcessWaveTimer();


      // Private vars
      private:

         ServerGameStatusEnum* mGameStatus;
         int mWaveNumber;
         int mNumEnemiesKilled;
         float mTimeLeftInCurState;
         double mLastPublishTime;
         int mNumPlayers;
         int mNumTeams;

         // Config props
         float mWaveStartDuration; //unpublished
         float mWaveDuration; // unpublished
         int mGameDifficulty;
         int mMaxNumberWaves;

         float mTimeTillNextUpdate;
         float mTimeBetweenUpdates;
         bool mDirtySettings; // only set if important settings are dirty

   };


   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////
   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT ServerGameStatusActorProxy : public dtGame::GameActorProxy
   {
      public:
         typedef dtGame::GameActorProxy BaseClass;

         // public strings for the properties
         static const dtUtil::RefString PROP_GAME_STATUS;
         static const dtUtil::RefString PROP_WAVE_NUMBER;
         static const dtUtil::RefString PROP_NUM_ENEMIES_KILLED;
         static const dtUtil::RefString PROP_TIME_LEFT_IN_CUR_STATE;
         static const dtUtil::RefString PROP_LAST_PUBLISHED_TIME;
         static const dtUtil::RefString PROP_GAME_DIFFICULTY;
         static const dtUtil::RefString PROP_NUM_PLAYERS;
         static const dtUtil::RefString PROP_NUM_TEAMS;

         ServerGameStatusActorProxy();
         virtual void BuildPropertyMap();

         /// Returns a useful reference to our actor. If no actor is created yet, this will likley crash.
         ServerGameStatusActor &GetActorAsGameStatus() 
         {
            return *(static_cast<ServerGameStatusActor*>(GetActor()));
         }

      protected:
         virtual ~ServerGameStatusActorProxy();
         void CreateActor();
         virtual void OnEnteredWorld();
   };

}

#endif
//#endif
