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
#ifndef _PORTAL_ACTOR_H_
#define _PORTAL_ACTOR_H_

#include <dtGame/gameactor.h>
#include <SimCore/Export.h>

namespace SimCore
{
   namespace Actors
   {
      ///////////////////////////////////////////////////////
      //    The Actor
      ///////////////////////////////////////////////////////
      class SIMCORE_EXPORT Portal : public dtGame::GameActor
      {
         public:
            Portal(dtGame::GameActorProxy &proxy);

            ///////////////////////////////////////////
            dtCore::DeltaDrawable* GetActorLink();

            //////////////////////////////////////////////////////////////////
            void  SetActorLink(dtDAL::ActorProxy* proxy)
            {
               GetGameActorProxy().SetLinkedActor("ActorLink", proxy);
            }

            ///////////////////////////////////////////
            bool operator==(const Portal& portal) const
            {
               return portal.GetUniqueId() == this->GetUniqueId();
            }

            ///////////////////////////////////////////
            std::string GetPortalName() {return mPortalName;}
            void SetPortalName(const std::string& name);

            ///////////////////////////////////////////
            bool GetIsOpen() {return mIsOpen;}
            void SetIsOpen(bool value) {mIsOpen = value;}

            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

         protected:
            virtual ~Portal(){}

         private:
            bool                                   mIsOpen;
            std::string                            mPortalName;
            float                                  mTimeToSendOut;
      };

      ///////////////////////////////////////////////////////
      //    The Proxy
      ///////////////////////////////////////////////////////
      class SIMCORE_EXPORT PortalProxy : public dtGame::GameActorProxy
      {
         public:
            PortalProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~PortalProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
      };
   }
}

#endif
