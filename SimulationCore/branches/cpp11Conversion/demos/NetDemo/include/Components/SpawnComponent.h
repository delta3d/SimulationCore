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
#ifndef NETDEMO_SPAWNCOMPONENT
#define NETDEMO_SPAWNCOMPONENT

#include <DemoExport.h>
#include <dtGame/gmcomponent.h>

#include <Actors/EnemyMothership.h>
#include <Actors/ServerGameStatusActor.h>

#include <dtAI/fsm.h>
#include <AIUtility.h>

namespace NetDemo
{
   class EnemyDescriptionActor;

   class NETDEMO_EXPORT SpawnComponent : public dtGame::GMComponent
   {
   public:

      typedef dtGame::GMComponent BaseClass;
      static const std::string DEFAULT_NAME;

      /// Constructor
      SpawnComponent(const std::string& name = DEFAULT_NAME);

      /**
      * Handles incoming messages
      */
      virtual void ProcessMessage(const dtGame::Message& message);

      /**
      * Called when this component is added to the game manager
      */
      virtual void OnAddedToGM();

      virtual void OnRemovedFromGM();

      DT_DECLARE_ACCESSOR(int, NumEnemiesStart);
      DT_DECLARE_ACCESSOR(int, NumEnemiesCurrent);
      DT_DECLARE_ACCESSOR(int, MaxEnemies);
      DT_DECLARE_ACCESSOR(int, Difficulty);
      DT_DECLARE_ACCESSOR(int, WaveNumber);
      DT_DECLARE_ACCESSOR(int, NumPlayers);
      DT_DECLARE_ACCESSOR(float, TimeLeftInWave);
      DT_DECLARE_ACCESSOR(dtUtil::EnumerationPointer<ServerGameStatusActor::ServerGameStatusEnum>, GameStatus);

   protected:

      /// Destructor
      virtual ~SpawnComponent();

      virtual void CleanUp();

      virtual void SpawnEnemy(const EnemyDescriptionActor* desc);

   private:

      typedef std::vector<std::shared_ptr<EnemyMothershipActor> > EnemyMothershipArray;
      EnemyMothershipArray mSpawnVolumes;

      typedef std::vector<std::shared_ptr<EnemyDescriptionActor> > EnemyDescArray;
      EnemyDescArray mEnemies;

      void Tick(float dt);
      void InitGameState(ServerGameStatusActor::ServerGameStatusEnum& gameStatus, int difficulty, int numPlayers, int numWaves);
      void UpdateGameState(ServerGameStatusActor::ServerGameStatusEnum& gameStatus, int waveNumber, float timeLeftInWave);


   };
}//namespace NetDemo

#endif //NETDEMO_SPAWNCOMPONENT
