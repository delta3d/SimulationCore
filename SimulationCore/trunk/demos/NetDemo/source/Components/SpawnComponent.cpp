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
#include <ActorRegistry.h>

#include <vector>

namespace NetDemo
{

   const std::string SpawnComponent::DEFAULT_NAME = "SpawnComponent";

   /////////////////////////////////////////////////////////////
   SpawnComponent::SpawnComponent(const std::string& name)
   : BaseClass(name)
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
         //float dt = float(static_cast<const dtGame::TickMessage&>(message).GetDeltaSimTime());
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_GAME_EVENT)
      {
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
         std::vector<dtDAL::ActorProxy*> proxies;
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::SPAWN_VOLUME_ACTOR_TYPE, proxies);

         std::vector<dtDAL::ActorProxy*>::iterator iter = proxies.begin();
         std::vector<dtDAL::ActorProxy*>::iterator iterEnd = proxies.end();

         for(; iter != iterEnd; ++iter)
         {
            SpawnVolumeActorProxy* spawnProxy = dynamic_cast<SpawnVolumeActorProxy*>(*iter);
            if(spawnProxy != NULL)
            {
               mSpawnVolumes.push_back(spawnProxy);
            }
         }
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
      {
         CleanUp();
      }
   }

}//namespace NetDemo
