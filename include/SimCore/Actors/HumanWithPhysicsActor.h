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
#ifndef _NX_AGEIA_PLAYER_ACTOR_H_
#define _NX_AGEIA_PLAYER_ACTOR_H_

#include <SimCore/Actors/Human.h>

#include <dtPhysics/charactercontroller.h>

#include <SimCore/PhysicsTypes.h>

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   class MessageType;

   namespace Actors
   {
      typedef Human HumanWithPhysicsActor;

      // proxy
      class SIMCORE_EXPORT HumanWithPhysicsActorProxy : public HumanActorProxy
      {
         public:
            typedef HumanActorProxy BaseClass;

            /// Constructor
            HumanWithPhysicsActorProxy();

            /// Destructor
            virtual ~HumanWithPhysicsActorProxy();

            /// Overridden to force upright rotations on the Dead Reckoning Helper
            void BuildActorComponents();
      };
   }
}
#endif
