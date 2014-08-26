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
      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      HumanWithPhysicsActor::HumanWithPhysicsActor(dtGame::GameActorProxy& owner)
      : Human(owner)
      // With 30, this is about 12.66 MPH, which is a decently fast sustainable run. The fastest human sprint
      // speed is like 27 MPH. Typically slow walk speed is like 3 MPH. A marathoner can sustain 12.55 MPH
      // for 2 hours. Note, this multiplies times the frame speed using the motion model, but it should be
      // irrelevant of FPS.
      , mMoveRateConstant(30.0f, 30.0f, 0.0f)
      , mNotifyChangePosition(false)
      , mNotifyChangeOrient(false)
      , mNotifyChangeVelocity(false)
      {
      }

      ////////////////////////////////////////////////////////////
      HumanWithPhysicsActor::~HumanWithPhysicsActor()
      {
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         mNotifyChangePosition = false;
         mNotifyChangeOrient = false;
         mNotifyChangeVelocity = false;

         //TODO: CURRENTLY NO LOCAL BEHAVIOR WITH dtPhysics in local mode


         Human::OnTickLocal(tickMessage);
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         Human::OnTickRemote(tickMessage);

         dtCore::Transform transform;
         GetTransform(transform);

         transform.GetTranslation(mPreviousTransform);
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::SetPosition( const osg::Vec3& position )
      {
         dtCore::Transform xform;
         xform.SetTranslation( position );
         SetTransform(xform);
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::OffsetPosition( const osg::Vec3& offset )
      {
         dtCore::Transform xform;
         GetTransform( xform );
         osg::Vec3 trans;
         xform.GetTranslation(trans);
         trans += offset;
         xform.SetTranslation( trans );
         SetPosition( trans );
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::OnEnteredWorld()
      {
         Human::OnEnteredWorld();

         GetPhysicsActComp()->SetPrePhysicsCallback(dtPhysics::PhysicsActComp::UpdateCallback(this, &HumanWithPhysicsActor::PrePhysicsUpdate));

         dtPhysics::PhysicsObject* po = GetPhysicsActComp()->GetMainPhysicsObject();
         // Human remote is a bad group name because the local behavior is the same.
         po->SetCollisionGroup(CollisionGroup::GROUP_HUMAN_REMOTE);

         // If the developer didn't set the origin offset to something.
         // This is sort of a problem, because if the user WANTS no offset, they would have to set it to
         // something really small, but not quite 0.
         if (po->GetOriginOffset().length2() < FLT_EPSILON)
         {
            osg::Vec3 extents = po->GetExtents();
            // Move the cylinder up half the height to sync up the origins.
            po->SetOriginOffset(osg::Vec3(0.0f, 0.0f, (extents.x() / 2.0f) + extents.y()));
         }


         po->CreateFromProperties();
      }

      ////////////////////////////////////////////////////////////////////
      dtPhysics::PhysicsActComp* HumanWithPhysicsActor::GetPhysicsActComp()
      {
         dtPhysics::PhysicsActComp* physAC = NULL;
         GetComponent(physAC);
         return physAC;
      }


      ////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::PrePhysicsUpdate()
      {
          dtCore::Transform xform;
          GetTransform(xform);
          dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();
          if (physObj != NULL)
          {
             physObj->SetTransformAsVisual(xform);
          }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::PostPhysicsUpdate()
      {
         if (!IsRemote())
         {
         }
      }

      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      HumanWithPhysicsActorProxy::HumanWithPhysicsActorProxy()
      {
         SetClassName("HumanWithPhysicsActor");
      }

      //////////////////////////////////////////////////////////////////////////
      HumanWithPhysicsActorProxy::~HumanWithPhysicsActorProxy()
      {

      }

      /// Instantiates the actor this proxy encapsulated
      void HumanWithPhysicsActorProxy::CreateDrawable()
      {
         HumanWithPhysicsActor* p = new HumanWithPhysicsActor(*this);
         SetDrawable(*p);
      }

      //////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActorProxy::BuildActorComponents()
      {
         if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
         {
            dtCore::RefPtr<dtPhysics::PhysicsActComp> physAC = new dtPhysics::PhysicsActComp();

            dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("Body");
            physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CYLINDER);
            physicsObject->SetMechanicsType(dtPhysics::MechanicsType::KINEMATIC);
            physicsObject->SetCollisionGroup(SimCore::CollisionGroup::GROUP_HUMAN_LOCAL);
            physicsObject->SetMass(100.0f);
            physicsObject->SetExtents(osg::Vec3(1.8f, 0.5f, 0.0f));
            physAC->AddPhysicsObject(*physicsObject);

            AddComponent(*physAC);
         }

         BaseClass::BuildActorComponents();

         dtCore::RefPtr<dtGame::DeadReckoningHelper> drAC;
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
