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
#ifndef _PLAYER_ACTOR_H_
#define _PLAYER_ACTOR_H_

#include <SimCore/Actors/StealthActor.h>

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   class MessageType;

   namespace Actors
   {
      class SIMCORE_EXPORT PlayerActor : public StealthActor
      {
      public:
         typedef StealthActor BaseClass;

         /// Constructor
         PlayerActor(dtGame::GameActorProxy &proxy);

         /// Destructor
         virtual ~PlayerActor();

         /**
          * Returns the enumeration of currently enabled tool
          * @return The currently enabled tool
          */
         SimCore::MessageType& GetEnabledTool() const;

         /**
          * Overload to take enable a tool by its enumeration
          * @param tool The tool to enable
          */
         void SetEnabledTool(SimCore::MessageType &tool);

         ///Method to handle the enable/disable tool message.
         void EnableTool(const dtGame::Message &msg);

      private:

         SimCore::MessageType* mActiveTool;
      };

      class SIMCORE_EXPORT PlayerActorProxy : public StealthActorProxy
      {
      public:

         /// Constructor
         PlayerActorProxy();

         /// Destructor
         virtual ~PlayerActorProxy();

         /// Builds the properties associated with this player
         virtual void BuildPropertyMap();

         /// Builds the invokables associated with this player
         virtual void BuildInvokables();

         /// Overridden - sets init values
         virtual void BuildActorComponents();

         /// Instantiates the actor this proxy encapsulated
         virtual void CreateActor();
      };
   }
}
#endif
