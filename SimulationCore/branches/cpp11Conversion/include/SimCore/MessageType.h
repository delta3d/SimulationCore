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
 * @author David Guthrie
 */
#ifndef _MESSAGE_TYPES_H_
#define _MESSAGE_TYPES_H_

#include <dtGame/messagetype.h>
#include <SimCore/Export.h>

namespace dtGame
{
   class MessageFactory;
}

namespace SimCore
{

   /**
    * @class MessageType
    * @brief The messages types specific to the image generator and stealth viewer.
    */
   DT_DECLARE_MESSAGE_TYPE_CLASS_BEGIN(MessageType, SIMCORE_EXPORT)
      static bool IsValidToolType(const dtGame::MessageType &type);

      static const MessageType ATTACH_TO_ACTOR;
      static const MessageType DETONATION;
      static const MessageType SHOT_FIRED;

      ///Marks an update to the Stealth actor field of view.
      static const MessageType STEALTH_ACTOR_FOV;
      ///Marks an update to the Stealth actor rotation.
      static const MessageType STEALTH_ACTOR_ROTATION;
      ///Marks an update to the Stealth actor translation.
      static const MessageType STEALTH_ACTOR_TRANSLATION;

      static const MessageType MAGNIFICATION;

      static const MessageType TIME_QUERY;
      static const MessageType TIME_VALUE;

      static const MessageType INFO_TERRAIN_LOADED;

      // These are NON-CONST because they are used in an actor property
      static MessageType BINOCULARS;
      static MessageType COMPASS;
      static MessageType COMPASS_360;
      static MessageType NIGHT_VISION;
      static MessageType LASER_RANGE_FINDER;
      static MessageType GPS;
      static MessageType MAP;
      static MessageType NO_TOOL;

      static MessageType CONTROL_STATE_CONFLICT;

      static const MessageType REQUEST_WARP_TO_POSITION;

      static const MessageType INFO_EMBEDDED_DATA;

      // Conversation-related messages
      static const MessageType INTERACTION_CHANGED;
      static const MessageType CONVERSATION_RESPONSE;

      // Game state-related messages
      static const MessageType GAME_STATE_CHANGED;

   DT_DECLARE_MESSAGE_TYPE_CLASS_END()
}
#endif
