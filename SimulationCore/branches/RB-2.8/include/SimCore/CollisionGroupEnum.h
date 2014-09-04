/* -*-c++-*-
* Simulation Core
* Copyright 2007-2009, Alion Science and Technology
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
* @author Chris Rodgers
* @author David Guthrie
*
* NOTE: This enumeration is for avoiding hard-coded values previously found
* throughout the Sim Core code.
*/

#ifndef SIMCORE_NX_COLLISION_GROUP_ENUM_H_
#define SIMCORE_NX_COLLISION_GROUP_ENUM_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <SimCore/PhysicsTypes.h>

#ifdef AGEIA_PHYSICS

#include <NxPhysics.h>
namespace SimCore
{
   typedef NxCollisionGroup CollisionGroupType;
}
#else

namespace SimCore
{
   typedef  dtPhysics::CollisionGroup CollisionGroupType;
}

#endif

namespace SimCore
{
   ////////////////////////////////////////////////////////////////////////////////
   // Physics COLLISION GROUPS ENUMERATION CODE
   ////////////////////////////////////////////////////////////////////////////////
   namespace CollisionGroup
   {
      enum CollisionGroupE
      {
         GROUP_TERRAIN        = 0
         , GROUP_VEHICLE_GROUND = 1
         , GROUP_BULLET         = 2
         , GROUP_PARTICLE       = 3
         , GROUP_WATER          = 4
         , GROUP_VEHICLE_WATER  = 5
         , GROUP_HUMAN_LOCAL    = 6
         , GROUP_HUMAN_REMOTE   = 7
         , GROUP_USER_DEFINED   = 10
      };
      void SIMCORE_EXPORT SetupDefaultGroupCollisions(dtPhysics::PhysicsComponent& comp);
   }
}


#endif
