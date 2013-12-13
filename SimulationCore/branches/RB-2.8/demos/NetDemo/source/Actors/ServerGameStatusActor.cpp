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
#include <prefix/SimCorePrefix.h>

#include <dtUtil/mswin.h>
#include <Actors/ServerGameStatusActor.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagefactory.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>
#include <dtGame/actorupdatemessage.h>



namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(ServerGameStatusActor::ServerGameStatusEnum);
   ServerGameStatusActor::ServerGameStatusEnum ServerGameStatusActor::ServerGameStatusEnum::UNKNOWN("UNKNOWN");
   ServerGameStatusActor::ServerGameStatusEnum ServerGameStatusActor::ServerGameStatusEnum::WAVE_ABOUT_TO_START("WAVE_ABOUT_TO_START");
   ServerGameStatusActor::ServerGameStatusEnum ServerGameStatusActor::ServerGameStatusEnum::WAVE_IN_PROGRESS("WAVE_IN_PROGRESS");
   ServerGameStatusActor::ServerGameStatusEnum ServerGameStatusActor::ServerGameStatusEnum::WAVE_ENDING("WAVE_ENDING");
   ServerGameStatusActor::ServerGameStatusEnum ServerGameStatusActor::ServerGameStatusEnum::GAME_PAUSE("GAME_PAUSE");
   ServerGameStatusActor::ServerGameStatusEnum ServerGameStatusActor::ServerGameStatusEnum::GAME_OVER("GAME_OVER");
   ServerGameStatusActor::ServerGameStatusEnum ServerGameStatusActor::ServerGameStatusEnum::READY_ROOM("READY_ROOM");

   ///////////////////////////////////////////////////////////////////////////////////
   ServerGameStatusActor::ServerGameStatusActor(ServerGameStatusActorProxy &proxy)
      : BaseClass(proxy)
      , mGameStatus(&ServerGameStatusEnum::UNKNOWN)
      , mWaveNumber(0)
      , mNumEnemiesKilled(0)
      , mTimeLeftInCurState(0.0f)
      , mLastPublishTime(0.0)
      , mNumPlayers(0)
      , mNumTeams(0)
      , mWaveStartDuration(7.0f)
      , mWaveDuration(27.0f)
      , mGameDifficulty(0)
      , mMaxNumberWaves(30)
      , mTimeTillNextUpdate(0.0f)
      , mTimeBetweenUpdates(1.0f)
      , mDirtySettings(false)
   {
      SetName("ServerGameStatusActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   ServerGameStatusActor::~ServerGameStatusActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      // LOCAL - means server, means we have control
      if(!IsRemote())
      {
         mTimeTillNextUpdate = mTimeBetweenUpdates;
      }
   }

   //////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
   {
      BaseClass::OnTickLocal(tickMessage);
      bool publishGameStatus = false;

      mTimeTillNextUpdate -= tickMessage.GetDeltaRealTime();
      if (mDirtySettings || mTimeTillNextUpdate <= 0.0f)
      {
         publishGameStatus = true;
      }

      mTimeLeftInCurState -= tickMessage.GetDeltaSimTime();
      publishGameStatus = publishGameStatus || ProcessWaveTimer();

      if (publishGameStatus)
      {
         PublishGameStatus();
      }
   }

   //////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
   {
      BaseClass::OnTickRemote(tickMessage);

      // NOT REGISTERED - BE SURE TO REGISTER IF YOU WANT THESE
   }

   //////////////////////////////////////////////////////////////////////
   bool ServerGameStatusActor::ProcessWaveTimer()
   {
      bool needToPublish = false;

      if (mTimeLeftInCurState < 0.0f)
      {
         // WAVE_ABOUT_TO_START --> WAVE_IN_PROGRESS
         if (mGameStatus == &ServerGameStatusEnum::WAVE_ABOUT_TO_START)
         {
            mGameStatus = &ServerGameStatusEnum::WAVE_IN_PROGRESS;
            mTimeLeftInCurState = mWaveDuration;
            needToPublish = true;
            // TODO - Send a message letting folks know that the game status has changed.
            LOG_ALWAYS("Server - STARTING the wave. (TODO - fire a state change message here).");
         }

         // WAVE_IN_PROGRESS --> WAVE_ENDING
         else if (mGameStatus == &ServerGameStatusEnum::WAVE_IN_PROGRESS)
         {
            mGameStatus = &ServerGameStatusEnum::WAVE_ENDING;
            mTimeLeftInCurState = 4.0f; // just enough time to do some UI animations
            needToPublish = true;
            // TODO - Send a message letting folks know that the game status has changed.
            LOG_ALWAYS("Server - ENDING the wave. (TODO - fire a state change message here).");
         }

         // WAVE_ENDING --> WAVE_ABOUT_TO_START
         else if (mGameStatus == &ServerGameStatusEnum::WAVE_ENDING)
         {
            mWaveNumber ++;
            if (mWaveNumber > mMaxNumberWaves)
            {
               mGameStatus = &ServerGameStatusEnum::GAME_OVER;
               mTimeLeftInCurState = 0.0;
               // TODO - Send a message letting folks know that the game status has changed.
               LOG_ALWAYS("Server - GAME OVER ... (TODO - fire a state change message here).");
            }
            else
            {
               mGameStatus = &ServerGameStatusEnum::WAVE_ABOUT_TO_START;
               mTimeLeftInCurState = mWaveStartDuration;
               // TODO - Send a message letting folks know that the game status has changed.
               LOG_ALWAYS("Server - ABOUT TO START THE GAME. (TODO - fire a state change message here).");
            }
            needToPublish = true;
         }

      }

      return needToPublish;
   }

   //////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::PublishGameStatus()
   {
      dtCore::RefPtr<dtGame::Message> msg = GetGameActorProxy().GetGameManager()->
         GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED);

      // Set the last published time to now
      double simTime = GetGameActorProxy().GetGameManager()->GetSimulationTime();
      SetLastPublishTime(simTime);

      std::vector<dtUtil::RefString> propNames;
      propNames.push_back(ServerGameStatusActorProxy::PROP_GAME_STATUS);
      propNames.push_back(ServerGameStatusActorProxy::PROP_WAVE_NUMBER);
      propNames.push_back(ServerGameStatusActorProxy::PROP_NUM_ENEMIES_KILLED);
      propNames.push_back(ServerGameStatusActorProxy::PROP_TIME_LEFT_IN_CUR_STATE);
      propNames.push_back(ServerGameStatusActorProxy::PROP_LAST_PUBLISHED_TIME);
      propNames.push_back(ServerGameStatusActorProxy::PROP_GAME_DIFFICULTY);
      propNames.push_back(ServerGameStatusActorProxy::PROP_NUM_PLAYERS);
      propNames.push_back(ServerGameStatusActorProxy::PROP_NUM_TEAMS);
      GetGameActorProxy().PopulateActorUpdate(static_cast<dtGame::ActorUpdateMessage&>(*msg), propNames);

      GetGameActorProxy().GetGameManager()->SendMessage(*msg);

      // Clears the dirty flag. Allows others to call this and keep flag in sync.
      mDirtySettings = false;
      mTimeTillNextUpdate = mTimeBetweenUpdates;
   }


   ////////////////////////////////////////////////////////////////////////////////////
   ServerGameStatusActor::ServerGameStatusEnum& ServerGameStatusActor::GetGameStatus() const
   {
      return *mGameStatus;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetGameStatus(ServerGameStatusActor::ServerGameStatusEnum &newValue)
   {
      if (mGameStatus != &newValue)
      {
         mDirtySettings = true;
         mGameStatus = &newValue;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetWaveNumber(int newValue)
   {
      if (mWaveNumber != newValue)
      {
         mDirtySettings = true;
         mWaveNumber = newValue;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetNumEnemiesKilled(int newValue)
   {
      //mDirtySettings = true;
      mNumEnemiesKilled = newValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetTimeLeftInCurState(float newValue)
   {
      //mDirtySettings = true;
      mTimeLeftInCurState = newValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetLastPublishTime(double newValue)
   {
      //mDirtySettings = true;
      mLastPublishTime = newValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetGameDifficulty(int newValue)
   {
      if (mGameDifficulty != newValue)
      {
         mDirtySettings = true;
         mGameDifficulty = newValue;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetNumPlayers(int newValue)
   {
      if (mNumPlayers != newValue)
      {
         mDirtySettings = true;
         mNumPlayers = newValue;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::SetNumTeams(int newValue)
   {
      if (mNumTeams != newValue)
      {
         mDirtySettings = true;
         mNumTeams = newValue;
      }
   }

   //////////////////////////////////////////////////////////////////////
   void ServerGameStatusActor::StartTheGame()
   {
      // We assume the server knows what it's doing, so we just do what it says.
      mWaveNumber = 1;
      mNumEnemiesKilled = 0;
      mGameStatus = &ServerGameStatusEnum::WAVE_ABOUT_TO_START;
      mTimeLeftInCurState = mWaveStartDuration + 25.0f; // the 5 is to allow players to load and get into the game;
      PublishGameStatus();

      // TODO - Send a message letting folks know that the game status has changed.
      LOG_ALWAYS("Server - Starting the game. (TODO - fire a state change message here).");
   }


   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   ServerGameStatusActorProxy::ServerGameStatusActorProxy()
   {
      SetClassName("ServerGameStatusActor");
   }

   const dtUtil::RefString ServerGameStatusActorProxy::PROP_GAME_STATUS("Game Status");
   const dtUtil::RefString ServerGameStatusActorProxy::PROP_WAVE_NUMBER("Wave Number");
   const dtUtil::RefString ServerGameStatusActorProxy::PROP_NUM_ENEMIES_KILLED("Number of Enemies Killed");
   const dtUtil::RefString ServerGameStatusActorProxy::PROP_TIME_LEFT_IN_CUR_STATE("Time Left In Current State");
   const dtUtil::RefString ServerGameStatusActorProxy::PROP_LAST_PUBLISHED_TIME("Last Published Time");
   const dtUtil::RefString ServerGameStatusActorProxy::PROP_GAME_DIFFICULTY("Game Difficulty");
   const dtUtil::RefString ServerGameStatusActorProxy::PROP_NUM_PLAYERS("Number of Players");
   const dtUtil::RefString ServerGameStatusActorProxy::PROP_NUM_TEAMS("Number of Teams");


   ///////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActorProxy::BuildPropertyMap()
   {
      const std::string& GROUP = "Game Status";

      BaseClass::BuildPropertyMap();

      ServerGameStatusActor* actor = NULL;
      GetActor(actor);

      static const dtUtil::RefString PROP_GAME_STATUS_DESC("Indicates the game status - server perspective.");
      AddProperty(new dtDAL::EnumActorProperty<ServerGameStatusActor::ServerGameStatusEnum>(PROP_GAME_STATUS, PROP_GAME_STATUS,
         dtDAL::EnumActorProperty<ServerGameStatusActor::ServerGameStatusEnum>::SetFuncType(actor, &ServerGameStatusActor::SetGameStatus),
         dtDAL::EnumActorProperty<ServerGameStatusActor::ServerGameStatusEnum>::GetFuncType(actor, &ServerGameStatusActor::GetGameStatus),
         PROP_GAME_STATUS_DESC, GROUP));

      static const dtUtil::RefString PROP_WAVE_NUMBER_DESC("The current wave we are fighting.");
      AddProperty(new dtDAL::IntActorProperty(PROP_WAVE_NUMBER, PROP_WAVE_NUMBER,
         dtDAL::IntActorProperty::SetFuncType(actor, &ServerGameStatusActor::SetWaveNumber),
         dtDAL::IntActorProperty::GetFuncType(actor, &ServerGameStatusActor::GetWaveNumber),
         PROP_WAVE_NUMBER_DESC, GROUP));

      static const dtUtil::RefString PROP_NUM_ENEMIES_KILLED_DESC("The total number of enemies killed so far.");
      AddProperty(new dtDAL::IntActorProperty(PROP_NUM_ENEMIES_KILLED, PROP_NUM_ENEMIES_KILLED,
         dtDAL::IntActorProperty::SetFuncType(actor, &ServerGameStatusActor::SetNumEnemiesKilled),
         dtDAL::IntActorProperty::GetFuncType(actor, &ServerGameStatusActor::GetNumEnemiesKilled),
         PROP_NUM_ENEMIES_KILLED_DESC, GROUP));

      static const dtUtil::RefString PROP_TIME_LEFT_IN_CUR_STATE_DESC("The time remaining in our current state (such as wave begin or end)");
      AddProperty(new dtDAL::FloatActorProperty(PROP_TIME_LEFT_IN_CUR_STATE, PROP_TIME_LEFT_IN_CUR_STATE,
         dtDAL::FloatActorProperty::SetFuncType(actor, &ServerGameStatusActor::SetTimeLeftInCurState),
         dtDAL::FloatActorProperty::GetFuncType(actor, &ServerGameStatusActor::GetTimeLeftInCurState),
         PROP_TIME_LEFT_IN_CUR_STATE_DESC, GROUP));

      static const dtUtil::RefString PROP_LAST_PUBLISHED_TIME_DESC("The last time we published an update (sim time, in seconds)");
      AddProperty(new dtDAL::DoubleActorProperty(PROP_LAST_PUBLISHED_TIME, PROP_LAST_PUBLISHED_TIME,
         dtDAL::DoubleActorProperty::SetFuncType(actor, &ServerGameStatusActor::SetLastPublishTime),
         dtDAL::DoubleActorProperty::GetFuncType(actor, &ServerGameStatusActor::GetLastPublishTime),
         PROP_LAST_PUBLISHED_TIME_DESC, GROUP));

      static const dtUtil::RefString PROP_GAME_DIFFICULTY_DESC("A difficulty modifier");
      AddProperty(new dtDAL::IntActorProperty(PROP_GAME_DIFFICULTY, PROP_GAME_DIFFICULTY,
         dtDAL::IntActorProperty::SetFuncType(actor, &ServerGameStatusActor::SetGameDifficulty),
         dtDAL::IntActorProperty::GetFuncType(actor, &ServerGameStatusActor::GetGameDifficulty),
         PROP_GAME_DIFFICULTY_DESC, GROUP));

      static const dtUtil::RefString PROP_NUM_PLAYERS_DESC("The number of current players");
      AddProperty(new dtDAL::IntActorProperty(PROP_NUM_PLAYERS, PROP_NUM_PLAYERS,
         dtDAL::IntActorProperty::SetFuncType(actor, &ServerGameStatusActor::SetNumPlayers),
         dtDAL::IntActorProperty::GetFuncType(actor, &ServerGameStatusActor::GetNumPlayers),
         PROP_NUM_PLAYERS_DESC, GROUP));

      static const dtUtil::RefString PROP_NUM_TEAMS_DESC("The number of teams (usually 1)");
      AddProperty(new dtDAL::IntActorProperty(PROP_NUM_TEAMS, PROP_NUM_TEAMS,
         dtDAL::IntActorProperty::SetFuncType(actor, &ServerGameStatusActor::SetNumTeams),
         dtDAL::IntActorProperty::GetFuncType(actor, &ServerGameStatusActor::GetNumTeams),
         PROP_NUM_TEAMS_DESC, GROUP));


   }

   ///////////////////////////////////////////////////////////////////////////////////
   ServerGameStatusActorProxy::~ServerGameStatusActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActorProxy::CreateDrawable()
   {
      ServerGameStatusActor* newActor = new ServerGameStatusActor(*this);
      SetDrawable(*newActor);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void ServerGameStatusActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      if (!IsRemote())
      {
         RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
      }
   }

} // namespace
//#endif
