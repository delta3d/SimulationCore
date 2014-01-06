/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2009, Alion Science and Technology, BMH Operation
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Bradley Anderegg
*/


#include <Components/SpawnComponent.h>
#include <Actors/EnemyDescriptionActor.h>
#include <Actors/EnemyMine.h>
#include <Actors/EnemyHelix.h>
#include <ActorRegistry.h>
#include <SimCore/Utilities.h>
#include <dtUtil/mathdefines.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <iostream>
#include <vector>

namespace NetDemo
{

   const std::string SpawnComponent::DEFAULT_NAME = "SpawnComponent";

   /////////////////////////////////////////////////////////////
   SpawnComponent::SpawnComponent(const std::string& name)
   : BaseClass(name)
   , mNumEnemiesStart(2)
   , mNumEnemiesCurrent(0)
   , mMaxEnemies(150)
   , mDifficulty(1)
   , mWaveNumber(0)
   , mNumPlayers(1)
   , mTimeLeftInWave(0.0f)
   , mGameStatus(&ServerGameStatusActor::ServerGameStatusEnum::UNKNOWN)
   {

   }

   /////////////////////////////////////////////////////////////
   SpawnComponent::~SpawnComponent()
   {

   }
   DT_IMPLEMENT_ACCESSOR(SpawnComponent, int, NumEnemiesStart);
   DT_IMPLEMENT_ACCESSOR(SpawnComponent, int, NumEnemiesCurrent);
   DT_IMPLEMENT_ACCESSOR(SpawnComponent, int, MaxEnemies);
   DT_IMPLEMENT_ACCESSOR(SpawnComponent, int, Difficulty);
   DT_IMPLEMENT_ACCESSOR(SpawnComponent,int, WaveNumber);
   DT_IMPLEMENT_ACCESSOR(SpawnComponent, int, NumPlayers);
   DT_IMPLEMENT_ACCESSOR(SpawnComponent, float, TimeLeftInWave);
   DT_IMPLEMENT_ACCESSOR(SpawnComponent, dtUtil::EnumerationPointer<ServerGameStatusActor::ServerGameStatusEnum>, GameStatus);

   /////////////////////////////////////////////////////////////
   void SpawnComponent::CleanUp()
   {
      mSpawnVolumes.clear();
   }

   /////////////////////////////////////////////////////////////
   void SpawnComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();
   }

   /////////////////////////////////////////////////////////////
   void SpawnComponent::OnRemovedFromGM()
   {
      CleanUp();
   }

   /////////////////////////////////////////////////////////////
   void SpawnComponent::ProcessMessage(const dtGame::Message& message)
   {
      if(message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
      {
         float dt = float(static_cast<const dtGame::TickMessage&>(message).GetDeltaSimTime());
         Tick(dt);
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_GAME_EVENT)
      {
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == message.GetMessageType())
      {
         const dtGame::ActorUpdateMessage& updateMessage = static_cast<const dtGame::ActorUpdateMessage&> (message);

         if (updateMessage.GetActorType() == NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE)
         {
            // Find the actor in the GM
            dtGame::GameActorProxy* gap = GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId());
            if(gap != nullptr)
            {
               ServerGameStatusActor* serverStatus = static_cast<ServerGameStatusActor*>(gap->GetDrawable());
               UpdateGameState(serverStatus->GetGameStatus(), serverStatus->GetWaveNumber(), serverStatus->GetTimeLeftInCurState());
            }
         }
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
         //first look for a server status actor
         ServerGameStatusActorProxy* proxy = nullptr;
         GetGameManager()->FindActorByType(*NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE, proxy);
         if(proxy != nullptr)
         {
            ServerGameStatusActor& actor = static_cast<ServerGameStatusActor&>(proxy->GetGameActor());
            InitGameState(actor.GetGameStatus(), actor.GetGameDifficulty(), actor.GetNumPlayers(), actor.GetMaxNumWaves());
         }
         else
         {
            LOG_ERROR("Unable to find a ServerGameStatusActorProxy on message MapLoaded!");
         }

         //collect all enemy motherships
         std::vector<dtDAL::ActorProxy*> proxies;
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_MOTHERSHIP_ACTOR_TYPE, proxies);

         std::vector<dtDAL::ActorProxy*>::iterator iter = proxies.begin();
         std::vector<dtDAL::ActorProxy*>::iterator iterEnd = proxies.end();

         for(; iter != iterEnd; ++iter)
         {
            EnemyMothershipActorProxy* spawnProxy = dynamic_cast<EnemyMothershipActorProxy*>(*iter);
            if(spawnProxy != nullptr)
            {
               mSpawnVolumes.push_back(static_cast<EnemyMothershipActor*>(spawnProxy->GetDrawable()));
            }
         }


         //collect all Enemy Descriptions
         proxies.clear();
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_DESCRIPTION_TYPE, proxies);

         for(iter = proxies.begin(), iterEnd = proxies.end(); iter != iterEnd; ++iter)
         {
            EnemyDescriptionActorProxy* enemyProxy = dynamic_cast<EnemyDescriptionActorProxy*>(*iter);
            if(enemyProxy != nullptr)
            {
               mEnemies.push_back(static_cast<EnemyDescriptionActor*>(enemyProxy->GetDrawable()));
            }
         }
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
      {
         CleanUp();
      }
   }

   void SpawnComponent::SpawnEnemy( const EnemyDescriptionActor* desc )
   {
      std::string errorMessage("Error attempting to spawn enemy, cannot find prototype by the name '" + desc->GetPrototypeName() + ".'");

      std::shared_ptr<BaseEnemyActorProxy> enemyProxy = nullptr;
      SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
         desc->GetSpawnInfo().GetEnemyPrototypeName(), enemyProxy, errorMessage);

      if(enemyProxy.valid())
      {
         osg::Vec3 point;
         BaseEnemyActor& enemyActor = static_cast<BaseEnemyActor&>(*(enemyProxy->GetDrawable()));

         if(!mSpawnVolumes.empty())
         {
            int index = dtUtil::RandRange(0U, unsigned(mSpawnVolumes.size()) - 1U);
            if(mSpawnVolumes[index].valid())
            {
               point = mSpawnVolumes[index]->GetSpawnPoint();
            }
         }

         enemyActor.InitAI(desc);

         //this doesn't appear to work
         dtCore::Transform trans;
         enemyActor.GetTransform(trans);
         trans.SetTranslation(point);
         enemyActor.SetTransform(trans);

         GetGameManager()->AddActor(*enemyProxy, false, true);
      }
   }


   void SpawnComponent::InitGameState(ServerGameStatusActor::ServerGameStatusEnum& gameStatus, int difficulty, int numPlayers, int numWaves )
   {
      SetGameStatus(gameStatus);
      SetWaveNumber(0);
      SetNumPlayers(numPlayers);
      SetTimeLeftInWave(0);
      SetDifficulty(difficulty);
   }

   void SpawnComponent::UpdateGameState(ServerGameStatusActor::ServerGameStatusEnum& gameStatus, int waveNumber, float timeLeftInWave)
   {
      SetGameStatus(gameStatus);
      SetWaveNumber(waveNumber);
      SetTimeLeftInWave(timeLeftInWave);

      //int tempDifficulty = (mDifficulty > 0) ? mDifficulty : 1;
      mNumEnemiesCurrent = mNumEnemiesStart + (mNumPlayers * mDifficulty * waveNumber);
      if (mDifficulty <= 0)
      {
         mNumEnemiesCurrent = (int) (mNumEnemiesCurrent / 2.0f);
      }
      //printf("Num Enemies spawning is: %d, capped at %d.\r\n", mNumEnemiesCurrent, mMaxEnemies);
      dtUtil::ClampMax(mNumEnemiesCurrent, mMaxEnemies);
   }

   void SpawnComponent::Tick(float dt)
   {
      if(GetGameStatus() == ServerGameStatusActor::ServerGameStatusEnum::WAVE_IN_PROGRESS)
      {
         std::vector<dtDAL::ActorProxy*> proxies;
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE, proxies);

         int numEnemies = proxies.size();
         proxies.clear();

         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE, proxies);
         numEnemies += proxies.size();

         if(numEnemies <= mNumEnemiesCurrent)
         {
            //LOG_ALWAYS("Wave in progress");
            EnemyDescriptionActor* desc = nullptr;

            EnemyDescArray::iterator iter = mEnemies.begin();
            EnemyDescArray::iterator iterEnd = mEnemies.end();
            for(;iter != iterEnd; ++iter)
            {
               if((*iter).valid())
               {
                  desc = (*iter).get();

                  EnemyDescriptionActor::EnemySpawnInfo& info = desc->GetSpawnInfo();

                  float lastSpawn = info.GetLastSpawnTime();
                  lastSpawn += dt;

                  //check if it is time to spawn a new one
                  if((info.GetNumSpawnPerMinute() / 60) * lastSpawn > 1.0)
                  {
                     lastSpawn = 0;
                     SpawnEnemy(desc);
                  }

                  info.SetLastSpawnTime(lastSpawn);
               }

            }
         }

       }

   }

}//namespace NetDemo

