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
* @author Allen Danklefsen
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/PortalActor.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/exceptionenum.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtUtil/log.h>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////
      //    The Proxy
      ///////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////
      PortalProxy::PortalProxy()
      {
         SetClassName("Portal");
      }

      ///////////////////////////////////////////////////////
      void PortalProxy::BuildPropertyMap()
      {
         static const std::string GROUP = "Portal";
         dtGame::GameActorProxy::BuildPropertyMap();

         Portal* actor = NULL;
         GetActor(actor);

         AddProperty(new dtDAL::StringActorProperty("Portal Name", "Portal Name",
            dtDAL::StringActorProperty::SetFuncType(actor, &Portal::SetPortalName),
            dtDAL::StringActorProperty::GetFuncType(actor, &Portal::GetPortalName),
            "Portal Send over name",GROUP ));

         AddProperty(new dtDAL::ActorActorProperty(*this, "ActorLink", "ActorLink",
            dtDAL::ActorActorProperty::SetFuncType(actor, &Portal::SetActorLink),
            dtDAL::ActorActorProperty::GetFuncType(),
            "SimCore::Actors::Platform",
            "Portal Attached", GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("DoorOpen", "DoorOpen",
            dtDAL::BooleanActorProperty::SetFuncType(actor, &Portal::SetIsOpen),
            dtDAL::BooleanActorProperty::GetFuncType(actor, &Portal::GetIsOpen),
            "Is the door open?",GROUP ));
      }

      ///////////////////////////////////////////////////////
      PortalProxy::~PortalProxy()
      {

      }

      ///////////////////////////////////////////////////////
      void PortalProxy::CreateDrawable()
      {
         SetDrawable(*new Portal(*this));
      }

      ///////////////////////////////////////////////////////
      void PortalProxy::OnEnteredWorld()
      {
         if(!IsRemote())
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
      }

      ///////////////////////////////////////////////////////
      //    The Actor
      ///////////////////////////////////////////////////////
      // HACK: This is a temporary work around to avoid RILOMData errors on other machines.
      //       The error states that the expected string should be 64 bytes long.
      // NOTE: The length includes a NULL terminator.
      //       The HLA parameter translator adds this to the string automatically,
      //       thus expect to use 63 bytes for actual name characters.
      void Portal::SetPortalName(const std::string& name)
      {
         mPortalName = name;
         if( mPortalName.size() > 63 )
         {
            mPortalName = mPortalName.substr(0,63);
         }
         else
         {
            std::string padding;
            padding.insert(0, 63, ' ');
            mPortalName += padding.substr(mPortalName.size(),63);
         }
      }

      ///////////////////////////////////////////
      Portal::Portal(dtGame::GameActorProxy &proxy) :
         dtGame::GameActor(proxy),
         mIsOpen(false)
      {
         mTimeToSendOut = 10.0f;
      }

      ///////////////////////////////////////////////////////
      void Portal::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         mTimeToSendOut -= tickMessage.GetDeltaSimTime();
         if(mTimeToSendOut < 0.0f)
         {
            GetGameActorProxy().NotifyFullActorUpdate();
            mTimeToSendOut = 10.0f;
         }
      }
   }
}
