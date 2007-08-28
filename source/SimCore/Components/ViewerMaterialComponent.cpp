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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Components/ViewerMaterialComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/actorupdatemessage.h>
#include <dtABC/application.h>
#include <dtCore/scene.h>

namespace SimCore
{
   namespace Components
   {
      const std::string ViewerMaterialComponent::DEFAULT_NAME = "ViewerMaterialComponent";

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ViewerMaterialComponent::ViewerMaterialComponent(const std::string &name) : dtGame::GMComponent(name)
      {
         mClearMaterialsOnMapChange = false;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ViewerMaterialComponent::~ViewerMaterialComponent(void)
      {
         RemoveAllMaterials();
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void ViewerMaterialComponent::ProcessMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }
         else if(msg.GetMessageType() == dtGame::MessageType::TICK_REMOTE)
         {
            //ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_PUBLISHED)
         {
            //GameActorProxy* ga = GetGameManager()->FindGameActorById(msg.GetAboutActorId());
            //if(ga && ga->IsPublished())
            //   ProcessPublishActor(static_cast<const ActorPublishedMessage&>(msg));
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED)
         {
            //dtGame::GameActorProxy *ga = GetGameManager()->FindGameActorById(msg.GetAboutActorId());
            //if(ga && ga->IsPublished())
            //   ProcessDeleteActor(static_cast<const dtGame::ActorDeletedMessage&>(msg));
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED)
         {
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            // Load initial materials.
            CreateOrChangeMaterialByName("DefaultMaterial");
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
         {
            // Remove all materials.
            if(mClearMaterialsOnMapChange)
            {
               RemoveAllMaterials();
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void ViewerMaterialComponent::ProcessTick(const dtGame::TickMessage &msg)
      {
        
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void ViewerMaterialComponent::RemoveAllMaterials()
      {
         mOurMaterials.clear();
      }

      //////////////////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::ViewerMaterialActor& ViewerMaterialComponent::GetConstMaterialByName(const std::string& materialName)
      {
         std::vector<dtCore::RefPtr<SimCore::Actors::ViewerMaterialActor> >::iterator iter =  mOurMaterials.begin();
         for(;iter != mOurMaterials.end(); ++iter)
         {
            if((*iter)->GetName() == materialName)
            {
               return *(*iter).get();
            }
         }
         LOG_WARNING("GetConstMaterialByName(const std::string& materialName) Could not find your material. Returning Default Material"); 
         if(mOurMaterials.size() == 0)
         {
            LOG_WARNING("Map must not have been intialized, default material wasnt made, making now.");
            CreateOrChangeMaterialByName("DefaultMaterial");
         }

         return GetConstMaterialByName("DefaultMaterial");
      }

      //////////////////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::ViewerMaterialActor& ViewerMaterialComponent::GetConstMaterialByFID(const unsigned int fidIDToCheckWith)
      {
         return GetConstMaterialByName(FID_ID_ToString(fidIDToCheckWith));
      }

      //////////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::ViewerMaterialActor& ViewerMaterialComponent::CreateOrChangeMaterialByName(const std::string& materialName)
      {
         std::vector<dtCore::RefPtr<SimCore::Actors::ViewerMaterialActor> >::iterator iter = mOurMaterials.begin();
         for(;iter != mOurMaterials.end(); ++iter)
         {
            if((*iter)->GetName() == materialName)
            {
               return *(*iter).get();
            }
         }

         // Couldnt find material... make a new one.
         dtCore::RefPtr<SimCore::Actors::ViewerMaterialActorProxy> materialToMakeProxy;
         GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::MATERIAL_ACTOR_TYPE, materialToMakeProxy);
         SimCore::Actors::ViewerMaterialActor* materialToMake = dynamic_cast<SimCore::Actors::ViewerMaterialActor*>(materialToMakeProxy->GetActor());
         materialToMake->SetName(materialName);
         mOurMaterials.push_back(materialToMake);
         return *materialToMake;
      }

      //////////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::ViewerMaterialActor& ViewerMaterialComponent::CreateOrChangeMaterialByFID(const unsigned int fidIDToMakeWith)
      {
         return CreateOrChangeMaterialByName(FID_ID_ToString(fidIDToMakeWith));
      }

      //////////////////////////////////////////////////////////////////////////////////////
      const std::string ViewerMaterialComponent::FID_ID_ToString(const unsigned int nID)
      {
         std::ostringstream returnString;
         returnString << "MATERIAL: " << nID << " TYPE: FID";
         return returnString.str();
      }
   } // end entity namespace.
} // end dvte namespace.
