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
* @author Chris Rodgers
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtGame/messagefactory.h>
#include "NetDemoMessageTypes.h"
#include "NetDemoMessages.h"



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // CODE
   /////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(MessageType);

   const MessageType MessageType::OPTION_NEXT("Option Next", "NetDemo",
      "Generic way to select a next option whether it be in the context of UI or in-game weapon switching or etc.",
      NETDEMO_MESSAGE_TYPE_ID);

   const MessageType MessageType::OPTION_PREV("Option Previous", "NetDemo",
      "Generic way to select a previous option whether it be in the context of UI or in-game weapon switching or etc.",
      NETDEMO_MESSAGE_TYPE_ID + 1);

   const MessageType MessageType::OPTION_SELECT("Option Select", "NetDemo",
      "Generic way to commit to the currently selected option whether it be in the context of UI or in-game weapon switching or etc.",
      NETDEMO_MESSAGE_TYPE_ID + 2);

   const MessageType MessageType::ENTITY_ACTION("Entity Action", "NetDemo",
      "Generic message type for communicating most types of interactions of the network.",
      NETDEMO_MESSAGE_TYPE_ID + 3);
  


   /////////////////////////////////////////////////////////////////////////////
   MessageType::MessageType(const std::string& name, const std::string& category,
      const std::string& description, const unsigned short messageId)
      : BaseClass(name, category, description, messageId)
   {
      AddInstance(this);
   }

   /////////////////////////////////////////////////////////////////////////////
   MessageType::~MessageType()
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void MessageType::RegisterMessageTypes(dtGame::MessageFactory& factory)
   {
      factory.RegisterMessageType<dtGame::Message>(OPTION_NEXT);
      factory.RegisterMessageType<dtGame::Message>(OPTION_PREV);
      factory.RegisterMessageType<dtGame::Message>(OPTION_SELECT);

      factory.RegisterMessageType<NetDemo::EntityActionMessage>(ENTITY_ACTION);
   }

}
