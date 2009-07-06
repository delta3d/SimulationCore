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
#include <dtGame/actorupdatemessage.h>
#include <vector>

namespace NetDemo
{

   const std::string SpawnComponent::DEFAULT_NAME = "SpawnComponent";

   /////////////////////////////////////////////////////////////
   SpawnComponent::SpawnComponent(const std::string& name)
   : BaseClass(name)
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
            if(gap == NULL)
            {
               ServerGameStatusActor* serverStatus = static_cast<ServerGameStatusActor*>(gap->GetActor());
               UpdateGameState(&serverStatus->GetGameStatus(), serverStatus->GetWaveNumber(), serverStatus->GetTimeLeftInCurState());
            }
         }
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
         //first look for a server status actor
         ServerGameStatusActorProxy* proxy;
         GetGameManager()->FindActorByType(*NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE, proxy);
         if(proxy != NULL)
         {
            ServerGameStatusActor& actor = static_cast<ServerGameStatusActor&>(proxy->GetGameActor());
            InitGameState(&actor.GetGameStatus(), actor.GetGameDifficulty(), actor.GetNumPlayers(), actor.GetMaxNumWaves());
         }
         else
         {
            LOG_ERROR("Unable to find a ServerGameStatusActorProxy on message MapLoaded!");
         }

         //collect all spawn volumes
         std::vector<dtDAL::ActorProxy*> proxies;
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::SPAWN_VOLUME_ACTOR_TYPE, proxies);

         std::vector<dtDAL::ActorProxy*>::iterator iter = proxies.begin();
         std::vector<dtDAL::ActorProxy*>::iterator iterEnd = proxies.end();

         for(; iter != iterEnd; ++iter)
         {
            SpawnVolumeActorProxy* spawnProxy = dynamic_cast<SpawnVolumeActorProxy*>(*iter);
            if(spawnProxy != NULL)
            {
               mSpawnVolumes.push_back(static_cast<SpawnVolumeActor*>(spawnProxy->GetActor()));
            }
         }


         //collect all Enemy Descriptions
         proxies.clear();
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_DESCRIPTION_TYPE, proxies);

         for(iter = proxies.begin(), iterEnd = proxies.end(); iter != iterEnd; ++iter)
         {
            EnemyDescriptionActorProxy* enemyProxy = dynamic_cast<EnemyDescriptionActorProxy*>(*iter);
            if(enemyProxy != NULL)
            {
               mEnemies.push_back(static_cast<EnemyDescriptionActor*>(enemyProxy->GetActor()));
            }
         }
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
      {
         CleanUp();
      }
   }

   void SpawnComponent::SpawnEnemy( const EnemyDescriptionActor& desc )
   {
      //static int count = 0;
      //if(count++ < 5)
      {
         std::string errorMessage("Error attempting to spawn enemy, cannot find prototype by the name '" + desc.GetPrototypeName() + ".'");

         dtCore::RefPtr<BaseEnemyActorProxy> enemyProxy = NULL;
         SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
            desc.GetSpawnInfo().GetEnemyPrototypeName(), enemyProxy, errorMessage);

         if(enemyProxy.valid())
         {
            osg::Vec3 point;
            BaseEnemyActor& enemyActor = static_cast<BaseEnemyActor&>(*(enemyProxy->GetActor()));

            if(!mSpawnVolumes.empty())
            {
               int index = dtUtil::RandRange(0, mSpawnVolumes.size() - 1);
               if(mSpawnVolumes[index].valid())
               {
                  point = mSpawnVolumes[index]->GetRandomPointInVolume();
               }
            }

            enemyActor.InitAI(desc);
            GetGameManager()->AddActor(*enemyProxy, false, true);

            //this doesn't appear to work
            dtCore::Transform trans;
            enemyActor.GetTransform(trans);
            trans.SetTranslation(point);
            enemyActor.SetTransform(trans);
         }
      }
   }


   void SpawnComponent::InitGameState(const ServerGameStatusActor::ServerGameStatusEnum* gameStatus, int difficulty, int numPlayers, int numWaves )
   {
      SetGameStatus(gameStatus);
      SetWaveNumber(0);
      SetNumPlayers(numPlayers);
      SetTimeLeftInWave(0);
      SetDifficulty(difficulty);
   }

   void SpawnComponent::UpdateGameState(const ServerGameStatusActor::ServerGameStatusEnum* gameStatus, int waveNumber, float timeLeftInWave)
   {
      SetGameStatus(gameStatus);
      SetWaveNumber(waveNumber);
      SetTimeLeftInWave(timeLeftInWave);
   }

   void SpawnComponent::Tick(float dt)
   {
      if(1)//GetGameStatus() == &ServerGameStatusActor::ServerGameStatusEnum::WAVE_IN_PROGRESS)
      {
         EnemyDescriptionActor* desc = NULL;

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
                  SpawnEnemy(*desc);
               }

               info.SetLastSpawnTime(lastSpawn);
            }

         }

      }

   }

}//namespace NetDemo

