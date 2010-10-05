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
#include "NetDemoMessageTypes.h"
#include "NetDemoMessages.h"



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // CODE
   /////////////////////////////////////////////////////////////////////////////
   DT_IMPLEMENT_MESSAGE_TYPE_CLASS(MessageType);

   const MessageType MessageType::UI_OPTION_NEXT("UI Option Next", "NetDemo",
      "Generic way to select a next option whether it be in the context of UI or in-game weapon switching or etc.",
      NETDEMO_MESSAGE_TYPE_ID, DT_MSG_CLASS(dtGame::Message));

   const MessageType MessageType::UI_OPTION_PREV("UI Option Previous", "NetDemo",
      "Generic way to select a previous option whether it be in the context of UI or in-game weapon switching or etc.",
      NETDEMO_MESSAGE_TYPE_ID + 1, DT_MSG_CLASS(dtGame::Message));

   const MessageType MessageType::UI_OPTION_SELECT("UI Option Select", "NetDemo",
      "Generic way to commit to the currently selected option whether it be in the context of UI or in-game weapon switching or etc.",
      NETDEMO_MESSAGE_TYPE_ID + 2, DT_MSG_CLASS(dtGame::Message));

   const MessageType MessageType::UI_HELP("UI Help", "NetDemo",
      "Used to toggle the UI help window",
      NETDEMO_MESSAGE_TYPE_ID + 3, DT_MSG_CLASS(dtGame::Message));

   const MessageType MessageType::UI_DEBUGINFO_UPDATED("UI Debug Info Updated", "NetDemo",
	   "Used to indicate the DebugInfo was updated for the GUI Component",
	   NETDEMO_MESSAGE_TYPE_ID + 4, DT_MSG_CLASS(dtGame::Message));

   const MessageType MessageType::ENTITY_ACTION("Entity Action", "NetDemo",
      "Generic message type for communicating most types of interactions of the network.",
      NETDEMO_MESSAGE_TYPE_ID + 5, DT_MSG_CLASS(NetDemo::EntityActionMessage));
  

}
