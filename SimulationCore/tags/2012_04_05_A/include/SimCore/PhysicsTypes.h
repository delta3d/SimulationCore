/* -*-c++-*-
 * SimulationCore
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
 * david
 */

#ifndef PHYSICSTYPES_H_
#define PHYSICSTYPES_H_

#ifdef AGEIA_PHYSICS

#include <NxAgeiaPhysicsHelper.h>
#include <NxAgeiaWorldComponent.h>


namespace dtPhysics
{

typedef NxActor PhysicsObject;
typedef dtAgeiaPhysX::PhysicsComponentInitCallback PhysicsComponentInitCallback;

typedef dtAgeiaPhysX::NxAgeiaPhysicsHelper PhysicsHelper;
typedef dtAgeiaPhysX::NxAgeiaWorldComponent PhysicsComponent;

typedef NxVec3 VectorType;

typedef NxReal Real;

typedef int CollisionGroup;
typedef int CollisionMask;

}
#else
#include <dtPhysics/physicstypes.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsactcomp.h>
#endif
#endif /* PHYSICSTYPES_H_ */
