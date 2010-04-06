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
#include <prefix/SimCorePrefix.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <SimCore/Components/Conversations/ConversationMessages.h>
#include <SimCore/Components/GameState/GameStateChangeMessage.h>
#include <dtGame/messagefactory.h>

namespace SimCore
{
   IMPLEMENT_ENUM(MessageType);
   const MessageType MessageType::ATTACH_TO_ACTOR("Attach To Actor", "StealthViewer", "Attach Camera To Actor", USER_DEFINED_MESSAGE_TYPE + 1);
   const MessageType MessageType::DETONATION("Detonation", "StealthViewer", "Munition Detonation", USER_DEFINED_MESSAGE_TYPE + 2);
   const MessageType MessageType::SHOT_FIRED("Shot Fired", "StealthViewer", "Entity Weapon Fired", USER_DEFINED_MESSAGE_TYPE + 3);

   const MessageType MessageType::STEALTH_ACTOR_FOV("Stealth Actor Field of View", "StealthViewer",
      "Carries an update or change to the stealth actor field of view.",
      USER_DEFINED_MESSAGE_TYPE + 4);
   const MessageType MessageType::STEALTH_ACTOR_ROTATION("Stealth Actor Rotation", "StealthViewer",
      "Carries and update or change to the stealth actor rotation.",
      USER_DEFINED_MESSAGE_TYPE + 5);
   const MessageType MessageType::STEALTH_ACTOR_TRANSLATION("Stealth Actor Translation", "StealthViewer",
      "Carries and update or change to the stealth actor translation.",
      USER_DEFINED_MESSAGE_TYPE + 6);

   const MessageType MessageType::MAGNIFICATION("Magnification", "Magnification", "Set when the magnification of the actors need to be updated", USER_DEFINED_MESSAGE_TYPE + 16);

   const MessageType MessageType::TIME_QUERY("Time Query", "Time", "Query the network for the current time (HLA).",
      USER_DEFINED_MESSAGE_TYPE + 18);
   const MessageType MessageType::TIME_VALUE("Time Value", "Time", "Respose to a time query (HLA).",
      USER_DEFINED_MESSAGE_TYPE + 19);

   MessageType MessageType::BINOCULARS("Binoculars", "Tools", "Binoculars", USER_DEFINED_MESSAGE_TYPE + 10);
   MessageType MessageType::COMPASS("Compass", "Tools", "Compass", USER_DEFINED_MESSAGE_TYPE + 11);
   MessageType MessageType::NIGHT_VISION("Night Vision", "Tools", "Night Vision", USER_DEFINED_MESSAGE_TYPE + 12);
   MessageType MessageType::LASER_RANGE_FINDER("Laser Range Finder", "Tools", "Laser Range Finder", USER_DEFINED_MESSAGE_TYPE + 13);
   MessageType MessageType::GPS("GPS", "Tools", "GPS", USER_DEFINED_MESSAGE_TYPE + 14);
   MessageType MessageType::NO_TOOL("No Tool", "Tools", "No Tool", USER_DEFINED_MESSAGE_TYPE + 15);
   MessageType MessageType::MAP("Map", "Tools", "Map", USER_DEFINED_MESSAGE_TYPE + 17);

   MessageType MessageType::CONTROL_STATE_CONFLICT("Conflict", "Control State",
      "Conflict in controls states, possibly pointing to the same station",
      USER_DEFINED_MESSAGE_TYPE + 20);

   const MessageType MessageType::REQUEST_WARP_TO_POSITION("Warp To Position", "StealthViewer",
            "Warp the Stealth Actor to a Position", USER_DEFINED_MESSAGE_TYPE + 21);
   const MessageType MessageType::INFO_EMBEDDED_DATA("Embedded Data", "Info",
            "Holds a buffer of embedded binary data.  Traditionally a radio message", USER_DEFINED_MESSAGE_TYPE + 22);

   // Conversation-related messages
   const MessageType MessageType::INTERACTION_CHANGED("INTERACTION_CHANGED", "INFO", "Sent when the conversation component has received a new interaction.", USER_DEFINED_MESSAGE_TYPE + 23);
   const MessageType MessageType::CONVERSATION_RESPONSE("CONVERSATION_RESPONSE", "INFO", "Sent when the player responds to an interaction.", USER_DEFINED_MESSAGE_TYPE + 24);

   // Game state-related messages
   const MessageType MessageType::GAME_STATE_CHANGED("GAME_STATE_CHANGED", "INFO", "Sent when the game state changes.", USER_DEFINED_MESSAGE_TYPE + 25);

   // Tool message continued...
   MessageType MessageType::COMPASS_360("Compass360", "Tools", "Compass360", USER_DEFINED_MESSAGE_TYPE + 26);

   ///////////////////////////////////////////////////////////////////////
   MessageType::MessageType(
      const std::string &name,
      const std::string &category,
      const std::string &description,
      const unsigned short messageId) :
      dtGame::MessageType(name, category, description, messageId)
   {
      AddInstance(this);
   }

   ///////////////////////////////////////////////////////////////////////
   void MessageType::RegisterMessageTypes(dtGame::MessageFactory& factory)
   {
      factory.RegisterMessageType<AttachToActorMessage>(ATTACH_TO_ACTOR);
      factory.RegisterMessageType<DetonationMessage>(DETONATION);
      factory.RegisterMessageType<ShotFiredMessage>(SHOT_FIRED);

      factory.RegisterMessageType<StealthActorUpdatedMessage>(STEALTH_ACTOR_FOV);
      factory.RegisterMessageType<StealthActorUpdatedMessage>(STEALTH_ACTOR_ROTATION);
      factory.RegisterMessageType<StealthActorUpdatedMessage>(STEALTH_ACTOR_TRANSLATION);

      factory.RegisterMessageType<TimeQueryMessage>(TIME_QUERY);
      factory.RegisterMessageType<TimeValueMessage>(TIME_VALUE);

      factory.RegisterMessageType<MagnificationMessage>(MAGNIFICATION);

      factory.RegisterMessageType<ToolMessage>(BINOCULARS);
      factory.RegisterMessageType<ToolMessage>(LASER_RANGE_FINDER);
      factory.RegisterMessageType<ToolMessage>(COMPASS);
      factory.RegisterMessageType<ToolMessage>(COMPASS_360);
      factory.RegisterMessageType<ToolMessage>(GPS);
      factory.RegisterMessageType<ToolMessage>(MAP);
      factory.RegisterMessageType<ToolMessage>(NIGHT_VISION);

      factory.RegisterMessageType<ControlStateMessage>(CONTROL_STATE_CONFLICT);

      factory.RegisterMessageType<StealthActorUpdatedMessage>(REQUEST_WARP_TO_POSITION);

      factory.RegisterMessageType<EmbeddedDataMessage>(INFO_EMBEDDED_DATA);

      factory.RegisterMessageType<InteractionChangedMessage>(MessageType::INTERACTION_CHANGED);
      factory.RegisterMessageType<ConversationResponseMessage>(MessageType::CONVERSATION_RESPONSE);

      factory.RegisterMessageType<GameStateChangedMessage>(MessageType::GAME_STATE_CHANGED);
   }

   ///////////////////////////////////////////////////////////////////////
   bool MessageType::IsValidToolType(const dtGame::MessageType &type)
   {
      return type == MessageType::BINOCULARS
         || type == MessageType::COMPASS
         || type == MessageType::GPS
         || type == MessageType::MAP
         || type == MessageType::NIGHT_VISION
         || type == MessageType::NO_TOOL
         || type == MessageType::LASER_RANGE_FINDER;
   }

   /// Destructor
   MessageType::~MessageType() { }
}
