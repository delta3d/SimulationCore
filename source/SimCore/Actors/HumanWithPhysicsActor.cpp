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

#include <dtDAL/enginepropertytypes.h>

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
      HumanWithPhysicsActor::HumanWithPhysicsActor(dtGame::GameActorProxy& proxy) :
         Human(proxy)
      // With 30, this is about 12.66 MPH, which is a decently fast sustainable run. The fastest human sprint
      , mMoveRateConstant(30.0f, 30.0f, 0.0f)
      , mSecsSinceLastUpdateSent(0.0f)
      , mMaxUpdateSendRate(3.0f)
      , mAcceptInput(false)
      , mNotifyChangePosition(false)
      , mNotifyChangeOrient(false)
      , mNotifyChangeVelocity(false)
      // speed is like 27 MPH. Typically slow walk speed is like 3 MPH. A marathoner can sustain 12.55 MPH
      // for 2 hours. Note, this multiplies times the frame speed using the motion model, but it should be
      // irrelevant of FPS.
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

      ////////////////////////////////////////////////////////////////////////////////////
      //void HumanWithPhysicsActor::SetLastKnownRotation(const osg::Vec3 &vec)
      //{
      //   Human::SetLastKnownRotation( osg::Vec3( vec.x(), 0.0f, 0.0f) );
      //}


/*      ////////////////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::SetLastKnownTranslation(const osg::Vec3 &vec)
      {
#ifdef AGEIA_PHYSICS
         if(mPhysicsActComp != nullptr)
         {
            float zValue = mPhysicsActComp->GetCharacterExtents()[2];

            // Note this should really be zValue /= 2. However, jsaf doesnt use
            // the float value for w/e reason. so it has to round the number,
            // this being that the number is 1.5 which turns out to move the character down
            // correctly (well reporting it down correctly). Without subtracting,
            // 0.75 the altitude is 1 meter, you subtract 0.75 (the correct amount)
            // and the alt is still 1 meter. You subtract 20 and its 19. Subtract
            // 1.5 and its 1.
            Human::SetLastKnownTranslation(osg::Vec3(vec[0], vec[1], vec[2] - zValue));
         }
         else
#endif
            Human::SetLastKnownTranslation(osg::Vec3(vec[0], vec[1], vec[2]));
      }

      */
      /*
      ////////////////////////////////////////////////////////////////////////////////////
      bool HumanWithPhysicsActor::ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate)
      {
         osg::Vec3 position = pos;
#ifdef AGEIA_PHYSICS
         if(mPhysicsActComp != nullptr)
            position[2] -= (mPhysicsActComp->GetCharacterExtents()[2]);
#endif
         osg::Vec3 distanceMoved = pos - GetLastKnownTranslation();

         float distanceTurned = rot.x() - GetLastKnownRotation().x();

         if (distanceMoved.length2() > GetMaxTranslationError())
         {
            // DEBUG: std::cout << "\n\tUpdate Translation:\t" << GetMaxTranslationError() << std::endl;
            mNotifyChangePosition = true;
         }

         if (distanceTurned * distanceTurned > GetMaxRotationError())
         {
            // DEBUG: std::cout << "\n\tUpdate Orientation\n" << std::endl;
            mNotifyChangeOrient = true;
         }

         // Do full updates for now until partial updates are required.
         return mNotifyChangeVelocity ||
            mNotifyChangeOrient || mNotifyChangePosition;
      }
*/
      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         Human::OnTickRemote(tickMessage);

         dtCore::Transform transform;
         GetTransform(transform);

         transform.GetTranslation(mPreviousTransform);
         
         osg::Vec3 xyz;
         transform.GetTranslation(xyz);
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
         dtPhysics::PhysicsActComp* physAC = nullptr;
         GetComponent(physAC);
         return physAC;
      }


      ////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::PrePhysicsUpdate()
      {
          dtCore::Transform xform;
          GetTransform(xform);
          dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();
          if (physObj != nullptr)
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
            std::shared_ptr<dtPhysics::PhysicsActComp> physAC = new dtPhysics::PhysicsActComp();

			   std::shared_ptr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("Body");
			   physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CYLINDER);
			   physicsObject->SetMechanicsType(dtPhysics::MechanicsType::KINEMATIC);
			   physicsObject->SetCollisionGroup(SimCore::CollisionGroup::GROUP_HUMAN_LOCAL);
			   physicsObject->SetMass(100.0f);
			   physicsObject->SetExtents(osg::Vec3(1.8f, 0.5f, 0.0f));
			   physAC->AddPhysicsObject(*physicsObject);

            AddComponent(*physAC);
         }

         BaseClass::BuildActorComponents();

         std::shared_ptr<dtGame::DeadReckoningHelper> drAC;
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
