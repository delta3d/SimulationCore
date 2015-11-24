/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>

#include <StealthViewer/GMApp/ViewerConfigComponent.h>
#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>

#include <dtGame/messagetype.h>
#include <dtGame/message.h>
#include <dtGame/basemessages.h>

namespace StealthGM
{
   const dtCore::RefPtr<dtCore::SystemComponentType> ViewerConfigComponent::TYPE(new dtCore::SystemComponentType("StealthViewerConfigComponent", "GMComponents.SimCore.StealthGM",
         "", BaseClass::BaseGMComponentType));
   const std::string ViewerConfigComponent::DEFAULT_NAME(ViewerConfigComponent::TYPE->GetName());

   /////////////////////////////////////////////////////////////////////////
   ViewerConfigComponent::ViewerConfigComponent(dtCore::SystemComponentType& type)
      : dtGame::GMComponent(type)
   {

   }

   /////////////////////////////////////////////////////////////////////////
   ViewerConfigComponent::~ViewerConfigComponent()
   {

   }

   /////////////////////////////////////////////////////////////////////////
   void ViewerConfigComponent::ProcessMessage(const dtGame::Message& msg)
   {
      if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
         for(size_t i = 0; i < mConfigurationObjects.size(); i++)
         {
            mConfigurationObjects[i]->Reset(*GetGameManager());
         }
      }

      if(msg.GetMessageType() != dtGame::MessageType::TICK_LOCAL)
         return;

      for(size_t i = 0; i < mConfigurationObjects.size(); i++)
      {
         mConfigurationObjects[i]->ApplyChanges(*GetGameManager());
      }
   }

   /////////////////////////////////////////////////////////////////////////
   void ViewerConfigComponent::AddConfigObject(ConfigurationObjectInterface &object)
   {
      mConfigurationObjects.push_back(&object);
   }

   /////////////////////////////////////////////////////////////////////////
   void ViewerConfigComponent::RemoveConfigObject(ConfigurationObjectInterface &object)
   {
      for(size_t i = 0; i < mConfigurationObjects.size(); i++)
      {
         if(mConfigurationObjects[i] == &object)
         {
            mConfigurationObjects.erase(mConfigurationObjects.begin() + i);
         }
      }
   }
}
