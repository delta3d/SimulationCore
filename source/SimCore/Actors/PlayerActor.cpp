/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/invokable.h>

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

         AddProperty(new dtDAL::EnumActorProperty<SimCore::MessageType>("Enabled Tool", "Enabled Tool", 
            dtDAL::MakeFunctor(static_cast<PlayerActor&>(GetGameActor()), &PlayerActor::SetEnabledTool), 
            dtDAL::MakeFunctorRet(static_cast<PlayerActor&>(GetGameActor()), &PlayerActor::GetEnabledTool), 
            "Sets the currently enabled tool on the player"));
      }

      //////////////////////////////////////////////////////////////////////////
      void PlayerActorProxy::BuildInvokables()
      {
         PlayerActor &pa = static_cast<PlayerActor&>(GetGameActor());
       
         StealthActorProxy::BuildInvokables();

         AddInvokable(*new dtGame::Invokable("Enable Tool", 
            dtDAL::MakeFunctor(pa, &PlayerActor::EnableTool)));

         // TODO Register same invokable for the other tools when implemented
         RegisterForMessagesAboutSelf(SimCore::MessageType::BINOCULARS, "Enable Tool");
         RegisterForMessagesAboutSelf(SimCore::MessageType::LASER_RANGE_FINDER, "Enable Tool");
         RegisterForMessagesAboutSelf(SimCore::MessageType::COMPASS, "Enable Tool");
         RegisterForMessagesAboutSelf(SimCore::MessageType::GPS, "Enable Tool");
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

         dtGame::MessageType &type = const_cast<dtGame::MessageType&>(msg.GetMessageType());
            
         const SimCore::ToolMessage& tm = dynamic_cast<const SimCore::ToolMessage&>(msg);
                  
         SetEnabledTool(tm.IsEnabled() ? static_cast<SimCore::MessageType&>(type) : SimCore::MessageType::NO_TOOL);
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::MessageType& PlayerActor::GetEnabledTool() const
      {
         return *mActiveTool;
      }

      //////////////////////////////////////////////////////////////////////////
      void PlayerActor::SetEnabledTool(SimCore::MessageType &tool)
      {
         mActiveTool = &tool;
      }
   }
}
