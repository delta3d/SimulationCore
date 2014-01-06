/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
* Bradley Anderegg
*/

#include <AIComponent.h>
#include <dtGame/messagetype.h>

namespace NetDemo
{

   const std::string AIComponent::DEFAULT_NAME = "AIComponent";

   /////////////////////////////////////////////////////////////
   AIComponent::AIComponent(const std::string& name)
   : BaseClass(name)
   {

   }

   /////////////////////////////////////////////////////////////
   AIComponent::~AIComponent()
   {

   }

   /////////////////////////////////////////////////////////////
   void AIComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();
   }

   /////////////////////////////////////////////////////////////
   void AIComponent::OnRemovedFromGM()
   {
      BaseClass::OnRemovedFromGM();
   }

   /////////////////////////////////////////////////////////////
   void AIComponent::CleanUp()
   {
      BaseClass::CleanUp();
   }


   /////////////////////////////////////////////////////////////
   void AIComponent::ProcessMessage(const dtGame::Message& message)
   {
      BaseClass::ProcessMessage(message);
//      if (message.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED)
//      {
//         if (message.GetSource() == GetGameManager()->GetMachineInfo())
//         {
//            const dtGame::ActorUpdateMessage& acm =
//                     static_cast<const dtGame::ActorUpdateMessage&>(message);
//            const dtDAL::ActorType* at = acm.GetActorType();
//            if (at != nullptr && at->InstanceOf())
//         }
//      }
   }

}//namespace NetDemo
