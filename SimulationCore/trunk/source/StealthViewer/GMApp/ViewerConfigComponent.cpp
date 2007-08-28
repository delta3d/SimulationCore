/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Eddie Johnson
 */
#include <prefix/dvteprefix-src.h>

#include <StealthGM/ViewerConfigComponent.h>
#include <StealthGM/ConfigurationObjectInterface.h>

#include <dtGame/messagetype.h>
#include <dtGame/message.h>
#include <dtGame/basemessages.h>

namespace StealthGM
{
   const std::string ViewerConfigComponent::DEFAULT_NAME = "ViewerConfigComponent";

   ViewerConfigComponent::ViewerConfigComponent(const std::string &name)
      : dtGame::GMComponent(name)
   {

   }

   ViewerConfigComponent::~ViewerConfigComponent()
   {

   }

   void ViewerConfigComponent::ProcessMessage(const dtGame::Message &msg)
   {
      if(msg.GetMessageType() != dtGame::MessageType::TICK_LOCAL)
         return;

      for(size_t i = 0; i < mConfigurationObjects.size(); i++)
      {
         mConfigurationObjects[i]->ApplyChanges(*GetGameManager());
      }
   }

   void ViewerConfigComponent::AddConfigObject(ConfigurationObjectInterface &object)
   {
      mConfigurationObjects.push_back(&object);
   }

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
