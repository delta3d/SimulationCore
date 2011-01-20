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

#include <prefix/SimCorePrefix.h>
#include <SimCore/CollisionGroupEnum.h>

namespace SimCore
{
   namespace CollisionGroup
   {
      void SetupDefaultGroupCollisions(dtPhysics::PhysicsComponent& physicsComponent)
      {
         // Which groups collide with each other. Typically, on the world and particles collide.
         // DO NOT FORGET - If you want bullets to collide, make sure to update this line:
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_GROUND, GROUP_VEHICLE_GROUND, true);   // the world interacts with itself
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_GROUND, GROUP_PARTICLE, true);  // particles interact with the world
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_GROUND, GROUP_TERRAIN, true);   // the world interacts with itself
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_GROUND, GROUP_BULLET, false);   // the world interacts with itself

         physicsComponent.SetGroupCollision(GROUP_TERRAIN, GROUP_TERRAIN, true);   // the world interacts with itself
         physicsComponent.SetGroupCollision(GROUP_TERRAIN, GROUP_PARTICLE, true);  // particles interact with the world
         physicsComponent.SetGroupCollision(GROUP_BULLET, GROUP_PARTICLE, false);// bullets and particles do not interact
         physicsComponent.SetGroupCollision(GROUP_BULLET, GROUP_TERRAIN, false); // bullets and the world do NOT interact (this seems odd, but we handle with raycast)
         physicsComponent.SetGroupCollision(GROUP_PARTICLE, GROUP_PARTICLE, false);// particles do not interact with itself
         physicsComponent.SetGroupCollision(GROUP_BULLET, GROUP_BULLET, false);// bullets do not interact with itself

         physicsComponent.SetGroupCollision(GROUP_HUMAN_LOCAL, GROUP_HUMAN_LOCAL, true); // characters interact with themselves
         physicsComponent.SetGroupCollision(GROUP_HUMAN_LOCAL, GROUP_BULLET, false); // characters interact with bullets
         physicsComponent.SetGroupCollision(GROUP_HUMAN_LOCAL, GROUP_PARTICLE, true); // characters interact with physics particles
         physicsComponent.SetGroupCollision(GROUP_HUMAN_LOCAL, GROUP_TERRAIN, true);  // characters interact with world
         physicsComponent.SetGroupCollision(GROUP_HUMAN_LOCAL, GROUP_VEHICLE_GROUND, true);  // local characters can hit vehicles.

         // For remote characters, we want to collide with some things, but not the vehicle
         physicsComponent.SetGroupCollision(GROUP_HUMAN_REMOTE, GROUP_HUMAN_LOCAL, true); // local characters interact with remote characters
         physicsComponent.SetGroupCollision(GROUP_HUMAN_REMOTE, GROUP_BULLET, false); // remote characters interact with bullets
         physicsComponent.SetGroupCollision(GROUP_HUMAN_REMOTE, GROUP_PARTICLE, true); // remote characters interact with physics particles
         physicsComponent.SetGroupCollision(GROUP_HUMAN_REMOTE, GROUP_TERRAIN, false);  // remote characters DO NOT interact with world - don't push the HMMWV
         physicsComponent.SetGroupCollision(GROUP_HUMAN_REMOTE, GROUP_VEHICLE_GROUND, false);  // remote characters DO NOT interact with world - don't push the HMMWV

         // water interactions
         physicsComponent.SetGroupCollision(GROUP_WATER, GROUP_BULLET, false);  //  bullets can hit the water, (turn off so raycast handles it)
         physicsComponent.SetGroupCollision(GROUP_WATER, GROUP_PARTICLE, true);  // particles interact with the water
         physicsComponent.SetGroupCollision(GROUP_WATER, GROUP_TERRAIN, false);   // everything in group 0 can not hit the water
         physicsComponent.SetGroupCollision(GROUP_WATER, GROUP_HUMAN_LOCAL, false); // characters and water do not collide

         // 26 is our boat actor.
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_WATER, GROUP_WATER, true);  // boats can drive on the water
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_WATER, GROUP_BULLET, false);  // bullets don't hit anything directly.
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_WATER, GROUP_VEHICLE_WATER, true);  // boats can drive on the water
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_WATER, GROUP_PARTICLE, true);  // boats and particles
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_WATER, GROUP_HUMAN_REMOTE, true);  // bullets remote humans
         physicsComponent.SetGroupCollision(GROUP_VEHICLE_WATER, GROUP_TERRAIN, true);  // land & vehicles and boats
         physicsComponent.SetGroupCollision(GROUP_HUMAN_REMOTE, GROUP_VEHICLE_WATER, false);  // remote characters DO NOT interact with world - don't push the boat

      }

   }
}
