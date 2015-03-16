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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/HumanWithPhysicsActor.h>

#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <dtCore/enginepropertytypes.h>

#include <dtGame/invokable.h>
#include <dtGame/basemessages.h>
#include <dtGame/deadreckoninghelper.h>

#include <dtCore/keyboard.h>
#include <dtCore/batchisector.h>

#include <dtABC/application.h>

#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>

#include <SimCore/Actors/PagedTerrainPhysicsActor.h>

#include <SimCore/CollisionGroupEnum.h>


namespace SimCore
{
   namespace Actors
   {
      HumanWithPhysicsActorProxy::HumanWithPhysicsActorProxy()
      {
         SetClassName("HumanWithPhysicsActor");
      }

      //////////////////////////////////////////////////////////////////////////
      HumanWithPhysicsActorProxy::~HumanWithPhysicsActorProxy()
      {

      }
      //////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActorProxy::BuildActorComponents()
      {
         if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
         {
            dtCore::RefPtr<dtPhysics::PhysicsActComp> physAC = new dtPhysics::PhysicsActComp();

            dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = dtPhysics::PhysicsObject::CreateNew("Body");
            physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CYLINDER);
            physicsObject->SetMechanicsType(dtPhysics::MechanicsType::KINEMATIC);
            physicsObject->SetCollisionGroup(SimCore::CollisionGroup::GROUP_HUMAN_REMOTE);
            physicsObject->SetMass(100.0f);
            osg::Vec3 extents(1.8f, 0.5f, 0.0f);
            physicsObject->SetExtents(extents);
            // Move the cylinder up half the height to sync up the origins.
            physicsObject->SetOriginOffset(osg::Vec3(0.0f, 0.0f, (extents.x() / 2.0f) + extents.y()));
            physAC->AddPhysicsObject(*physicsObject);
            physAC->SetAutoCreateOnEnteringWorld(true);

            AddComponent(*physAC);
         }

         BaseClass::BuildActorComponents();

         dtCore::RefPtr<dtGame::DeadReckoningActorComponent> drAC;
         GetComponent(drAC);
         if (drAC.valid())
         {
            // We don't want the human to lean sideways, regardless of what is sent. It looks stupid
            drAC->SetForceUprightRotation(true);
            // default to velocity only.  Humans walk.
            drAC->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);
         }

      }

   }
}
