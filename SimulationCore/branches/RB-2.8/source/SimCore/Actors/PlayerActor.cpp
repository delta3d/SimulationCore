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
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <dtCore/enginepropertytypes.h>
#include <dtGame/invokable.h>
#include <dtGame/deadreckoninghelper.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      PlayerActorProxy::PlayerActorProxy()
      {
         SetClassName("SimCore::Actors::PlayerActor");
      }

      //////////////////////////////////////////////////////////////////////////
      PlayerActorProxy::~PlayerActorProxy()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void PlayerActorProxy::BuildPropertyMap()
      {
         StealthActorProxy::BuildPropertyMap();

         PlayerActor* playerActor = NULL;
         GetActor(playerActor);

         AddProperty(new dtCore::EnumActorProperty<SimCore::MessageType>("Enabled Tool", "Enabled Tool",
                  dtCore::EnumActorProperty<SimCore::MessageType>::SetFuncType(playerActor, &PlayerActor::SetEnabledTool),
                  dtCore::EnumActorProperty<SimCore::MessageType>::GetFuncType(playerActor, &PlayerActor::GetEnabledTool),
            "Sets the currently enabled tool on the player"));
      }

      //////////////////////////////////////////////////////////////////////////
      void PlayerActorProxy::BuildInvokables()
      {
         PlayerActor* pa = NULL;
         GetActor(pa);

         StealthActorProxy::BuildInvokables();

         AddInvokable(*new dtGame::Invokable("Enable Tool",
            dtUtil::MakeFunctor(&PlayerActor::EnableTool, *pa)));

         // TODO Register same invokable for the other tools when implemented
         RegisterForMessagesAboutSelf(SimCore::MessageType::BINOCULARS, "Enable Tool");
         RegisterForMessagesAboutSelf(SimCore::MessageType::LASER_RANGE_FINDER, "Enable Tool");
         RegisterForMessagesAboutSelf(SimCore::MessageType::COMPASS, "Enable Tool");
         RegisterForMessagesAboutSelf(SimCore::MessageType::GPS, "Enable Tool");
      }

      //////////////////////////////////////////////////////////////////////////
      void PlayerActorProxy::BuildActorComponents()
      {
         BaseClass::BuildActorComponents();

         dtGame::DeadReckoningHelper* drHelper = NULL;
         GetComponent(drHelper);
         if (drHelper != NULL)
         {
            // Initiaze Dead reckoning offset values after DR Act Component added
            drHelper->SetGroundOffset(3.0f);
         }
      }

      //////////////////////////////////////////////////////////
      void PlayerActorProxy::CreateDrawable()
      {
         PlayerActor* p = new PlayerActor(*this);
         SetDrawable(*p);
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      PlayerActor::PlayerActor(dtGame::GameActorProxy &proxy) :
         StealthActor(proxy),
         mActiveTool(&SimCore::MessageType::NO_TOOL)
      {
         SetAttachAsThirdPerson(false);
      }

      //////////////////////////////////////////////////////////////////////////
      PlayerActor::~PlayerActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void PlayerActor::EnableTool(const dtGame::Message &msg)
      {
         if (!SimCore::MessageType::IsValidToolType(msg.GetMessageType()))
            SetEnabledTool(SimCore::MessageType::NO_TOOL);

         dtGame::MessageType& type = const_cast<dtGame::MessageType&>(msg.GetMessageType());

         const SimCore::ToolMessage& tm = dynamic_cast<const SimCore::ToolMessage&>(msg);

         SetEnabledTool(tm.GetEnabled() ? static_cast<SimCore::MessageType&>(type) : SimCore::MessageType::NO_TOOL);
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::MessageType& PlayerActor::GetEnabledTool() const
      {
         return *mActiveTool;
      }

      //////////////////////////////////////////////////////////////////////////
      void PlayerActor::SetEnabledTool(SimCore::MessageType& tool)
      {
         mActiveTool = &tool;
      }
   }
}
