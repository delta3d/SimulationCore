/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen
*/
#include <prefix/dvteprefix-src.h>
#include <SimCore/Components/PortalComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>
#include <dtGame/actorupdatemessage.h>
#include <dtABC/application.h>
#include <dtCore/scene.h>
#include <dtDAL/enginepropertytypes.h>

namespace SimCore
{
   namespace Components
   {
      const std::string PortalComponent::DEFAULT_NAME("PortalComponent");

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      PortalComponent::PortalComponent(const std::string &name) : dtGame::GMComponent(name)
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      PortalComponent::~PortalComponent(void)
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void PortalComponent::ProcessMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }
         else if(msg.GetMessageType() == dtGame::MessageType::TICK_REMOTE)
         {
            //ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED)
		   {
            SimCore::Actors::PortalProxy* portal  = NULL;
			   GetGameManager()->FindGameActorById(msg.GetAboutActorId(), portal);
            if(portal != NULL)
            {
               RegisterPortal(dynamic_cast<SimCore::Actors::Portal*>(portal->GetActor()));
            }
		   }
		   else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_PUBLISHED)
         {
            /*dtGame::GameActorProxy* ga = GetGameManager()->FindGameActorById(msg.GetAboutActorId());
            if(ga && ga->IsPublished())
            {
               PortalProxy* portal = dynamic_cast<PortalProxy*>(ga);
               if(portal != NULL)
               {
                  RegisterPortal(dynamic_cast<Portal*>(portal->GetActor()));
               }
            }*/
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED)
         {
            dtGame::GameActorProxy* ga = GetGameManager()->FindGameActorById(msg.GetAboutActorId());
            if(ga)
            {
               SimCore::Actors::PortalProxy* portal = dynamic_cast<SimCore::Actors::PortalProxy*>(ga);
               if(portal != NULL)
               {
                  UnRegisterPortal(dynamic_cast<SimCore::Actors::Portal*>(portal->GetActor()));
               }
            }
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED)
         {
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
         {
            mOurPortals.clear();
         }
      }

      //////////////////////////////////////////
      void PortalComponent::RegisterPortal(SimCore::Actors::Portal* portal)
      {
         mOurPortals.push_back(portal);
      }

      //////////////////////////////////////////
      void PortalComponent::UnRegisterPortal(SimCore::Actors::Portal* portal)
      {
         std::vector<dtCore::RefPtr<SimCore::Actors::Portal> >::iterator iter = mOurPortals.begin();
         for(; iter != mOurPortals.end(); ++iter)
         {
            if((*iter) == portal)
            {
               mOurPortals.erase(iter);
               return;
            }
         }
         LOG_WARNING("Could not unregister portal, could not find it in the list");
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void PortalComponent::ProcessTick(const dtGame::TickMessage &msg)
      {
         
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void PortalComponent::FindPortalsInRange(const osg::Vec3& position, float radius, std::vector<dtGame::GameActorProxy*> &toFillIn)
      {
         dtCore::Transform transformForTempDrawable;
         for(unsigned int i = 0; i < mOurPortals.size(); ++i)
         {
            dtGame::GameActor* actor = dynamic_cast<dtGame::GameActor*>(mOurPortals[i]->GetActorLink());
            if(actor != NULL)
            {
               /*static_cast<dtGame::GameActor*>(proxy->GetActor())->*/actor->GetTransform(transformForTempDrawable);
               
               osg::Vec3 relPos = position - transformForTempDrawable.GetTranslation();
               float distSquared = relPos[0] * relPos[0] + relPos[1] * relPos[1] + relPos[2] * relPos[2];
               float minDist = radius + radius;
               if(distSquared <= minDist * minDist)
               {
                  toFillIn.push_back(&actor->GetGameActorProxy());
               }
            }
         }
      }
   } // end entity namespace.
} // end dvte namespace.
