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
 * @author David Guthrie
 */

#ifndef _VIEWER_RULES_COMPONENT_H_
#define _VIEWER_RULES_COMPONENT_H_

#include <string>
#include <SimCore/Export.h>
#include <dtGame/defaultnetworkpublishingcomponent.h>

namespace SimCore
{
   namespace Components
   {

      /**
       * @class ViewerRulesComponent
       * @brief subclassed rules component used to handle publishing the stealth actor.
       */
      class SIMCORE_EXPORT ViewerNetworkPublishingComponent : public dtGame::DefaultNetworkPublishingComponent
      {
      public:
         typedef dtGame::DefaultNetworkPublishingComponent BaseClass;
         static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;

         ViewerNetworkPublishingComponent(dtCore::SystemComponentType& type = *TYPE);
      protected:

         /**
          * Overridden to handle Stealth Actor Messages.
          * @param The message
          */
         virtual void ProcessUpdateActor(const dtGame::ActorUpdateMessage& msg);

         /**
          * Overridden to handle Stealth Actor Messages.
          * @param The message
          */
         virtual void ProcessPublishActor(const dtGame::Message &msg);

         /**
          * Sends out relevant stealth actor message for the given actor update message.
          * The message is assumed to be about a stealth actor.
          */
         void SendStealthActorMessages(const dtGame::ActorUpdateMessage& msg);

         /**
          * Processes an otherwise unhandled message.
          * @param The message
          */
         virtual void ProcessUnhandledLocalMessage(const dtGame::Message &msg);

         ///protected so this class may not be created on the stack.
         virtual ~ViewerNetworkPublishingComponent();
      };

   }
}

#endif
