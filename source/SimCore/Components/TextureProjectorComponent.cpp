/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson 
 */
#include <prefix/dvteprefix-src.h>
#include <SimCore/Components/TextureProjectorComponent.h>
#include <SimCore/Actors/TextureProjectorActor.h>

#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/actorupdatemessage.h>

#include <dtABC/application.h>

#include <dtCore/scene.h>
using namespace dtGame;

namespace SimCore
{
   namespace Components
   {
      const std::string TextureProjectorComponent::DEFAULT_NAME = "TextureProjectorComponent";

      ////////////////////////////////////////////////////////////////////
      TextureProjectorComponent::TextureProjectorComponent(const std::string &name) :
   dtGame::GMComponent(name) , mMaxNumberOfProjectedTextures(100)
      {
      }

      ////////////////////////////////////////////////////////////////////
      TextureProjectorComponent::~TextureProjectorComponent()
      {
         //mActorList.clear();
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorComponent::ProcessMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == MessageType::TICK_LOCAL)
         {
            ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }
         else if(msg.GetMessageType() == MessageType::TICK_REMOTE){}
         else if(msg.GetMessageType() == MessageType::INFO_ACTOR_PUBLISHED){}
         else if(msg.GetMessageType() == MessageType::INFO_ACTOR_DELETED)
         {
            //GameActorProxy *ga = GetGameManager()->FindGameActorById(msg.GetAboutActorId());
            //if(ga && ga->IsPublished())
            //   ProcessDeleteActor(static_cast<const ActorDeletedMessage&>(msg));
         }
         else if(msg.GetMessageType() == MessageType::INFO_ACTOR_UPDATED){}
         else if(msg.GetMessageType() == MessageType::INFO_MAP_LOADED){}
         else if(msg.GetMessageType() == MessageType::INFO_MAP_UNLOADED){}
      }
 
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorComponent::ProcessTick(const dtGame::TickMessage &msg)
      {
         std::list<dtCore::RefPtr<TextureProjectorActor> >::iterator listIter = mActorList.begin();
         for(; listIter != mActorList.end(); ++listIter)
         {
            if( (*listIter)->GetCurrentTime() > (*listIter)->GetMaxTime() )
            {
               GetGameManager()->DeleteActor((*listIter)->GetGameActorProxy());
               mActorList.erase(listIter);
               listIter = mActorList.begin();
            }
         }

         while(mActorList.size() > mMaxNumberOfProjectedTextures)
         {
            listIter = mActorList.begin();
            GetGameManager()->DeleteActor((*listIter)->GetGameActorProxy());
            mActorList.erase(listIter);
         }
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorComponent::AddTextureProjectorActorToComponent(TextureProjectorActor &toAdd)
      {
         mActorList.push_back(&toAdd);
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorComponent::RemoveTextureProjectorActorFromComponent(TextureProjectorActor &toRemove)
      {
         std::list<dtCore::RefPtr<TextureProjectorActor> >::iterator listIter = mActorList.begin();
         for(; listIter != mActorList.end(); ++listIter)
         {
            if((*listIter)->GetUniqueId() == toRemove.GetUniqueId())
            {
               mActorList.erase(listIter);
               return;
            }
         }
      }
   }
}
