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
* @author Chris Rodgers
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

#ifdef AGEIA_PHYSICS

#include <NxPhysics.h>
namespace SimCore
{
   typedef NxCollisionGroup CollisionGroupType;
}
#else

#include <dtPhysics/physicstypes.h>
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
         , GROUP_BULLET         = 15
         , GROUP_PARTICLE       = 20
         , GROUP_WATER          = 23
         //, GROUP_VEHICLE_GROUND = 0 currently also 0
         , GROUP_VEHICLE_WATER  = 26
         , GROUP_HUMAN_LOCAL    = 30
         , GROUP_HUMAN_REMOTE   = 31
      };
   }
}

#endif
