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
#include <SimCore/Components/ViewerNetworkPublishingComponent.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <dtGame/messagetype.h>
#include <dtGame/messagefactory.h>

#include <osg/Vec3>

#include <dtDAL/actortype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/messageparameter.h>
#include <dtGame/messagetype.h>
#include <dtGame/deadreckoninghelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {

      ///////////////////////////////////////////////////////////////////////////
      ViewerNetworkPublishingComponent::ViewerNetworkPublishingComponent(const std::string& name):
         dtGame::DefaultNetworkPublishingComponent(name)
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      ViewerNetworkPublishingComponent::~ViewerNetworkPublishingComponent()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerNetworkPublishingComponent::ProcessPublishActor(const dtGame::Message& msg)
      {
         dtGame::GameActorProxy* gap = GetGameManager()->FindGameActorById(msg.GetSendingActorId());

         const dtDAL::ActorType& stealthActorType = *SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE;
         const dtDAL::ActorType* actualType = &gap->GetActorType();
         if(actualType != NULL || stealthActorType != *actualType)
         {
            // Param added by Eddie. It takes a proxy also now. Is this correct?
            dtGame::DefaultNetworkPublishingComponent::ProcessPublishActor(msg, *gap);
         }
         else
         {
            RefPtr<dtGame::ActorUpdateMessage> createMsg;
            GetGameManager()->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_CREATED, createMsg);

            std::vector<dtUtil::RefString> propNames;
            propNames.push_back(dtGame::DeadReckoningHelper::PROPERTY_LAST_KNOWN_ROTATION);
            propNames.push_back(dtGame::DeadReckoningHelper::PROPERTY_LAST_KNOWN_TRANSLATION);
            gap->PopulateActorUpdate(*createMsg, propNames);

            SendStealthActorMessages(*createMsg);
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerNetworkPublishingComponent::SendStealthActorMessages(const dtGame::ActorUpdateMessage& msg)
      {
         const dtGame::MessageParameter* mp = msg.GetUpdateParameter("Last Known Rotation");

         //If the message has a rotation parameter, send a rotation changed message.
         if (mp != NULL)
         {
            RefPtr<StealthActorUpdatedMessage> rotMsg;
            GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::STEALTH_ACTOR_ROTATION, rotMsg);

            rotMsg->SetName(msg.GetName());
            rotMsg->SetAboutActorId(msg.GetAboutActorId());
            rotMsg->SetRotation(static_cast<const dtGame::Vec3MessageParameter*>(mp)->GetValue());
            GetGameManager()->SendNetworkMessage(*rotMsg);
         }

         mp = msg.GetUpdateParameter("Last Known Translation");

         //If the message has a location parameter, send a location changed message.
         if (mp != NULL)
         {
            RefPtr<StealthActorUpdatedMessage> transMsg;
            GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::STEALTH_ACTOR_TRANSLATION, transMsg);
            transMsg->SetName(msg.GetName());
            transMsg->SetAboutActorId(msg.GetAboutActorId());
            transMsg->SetTranslation(static_cast<const dtGame::Vec3MessageParameter*>(mp)->GetValue());
            GetGameManager()->SendNetworkMessage(*transMsg);
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerNetworkPublishingComponent::ProcessUpdateActor(const dtGame::ActorUpdateMessage& msg)
      {
         const dtDAL::ActorType &stealthActorType = *SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE;
         const dtDAL::ActorType *actualType = GetGameManager()->FindActorType(msg.GetActorTypeCategory(), msg.GetActorTypeName());

         //Here, the stealth actor does not publish regular actor updates because the HLA component needs one off messages for them.
         //This is due to the fact that RPR FOM uses specific interactions for the stealth actors rather than using an object.
         if(actualType != NULL && stealthActorType == *actualType)
         {
            SendStealthActorMessages(msg);
         }
         else
         {
            dtGame::DefaultNetworkPublishingComponent::ProcessUpdateActor(msg);
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerNetworkPublishingComponent::ProcessUnhandledLocalMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == MessageType::TIME_QUERY)
         {
            GetGameManager()->SendNetworkMessage(msg);
         }
      }

   }
}
