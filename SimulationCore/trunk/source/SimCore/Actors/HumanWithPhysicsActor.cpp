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
#include <prefix/SimCorePrefix-src.h>
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

#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>

#ifdef AGEIA_PHYSICS
#include <NxAgeiaWorldComponent.h>
#include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
#endif

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      HumanWithPhysicsActor::HumanWithPhysicsActor(dtGame::GameActorProxy &proxy) :
         Human(proxy)
      , mAcceptInput(false)
      , mNotifyChangePosition(false)
      , mNotifyChangeOrient(false)
      , mNotifyChangeVelocity(false)
      , mMoveRateConstant(30.0f, 30.0f, 0.0f)
      // With 30, this is about 12.66 MPH, which is a decently fast sustainable run. The fastest human sprint
      // speed is like 27 MPH. Typically slow walk speed is like 3 MPH. A marathoner can sustain 12.55 MPH
      // for 2 hours. Note, this multiplies times the frame speed using the motion model, but it should be
      // irrelevant of FPS.
      {
         mTimeForSendingDeadReckoningInfoOut = 0.0f;
         mTimesASecondYouCanSendOutAnUpdate  = 3.0f;
#ifdef AGEIA_PHYSICS
         mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaCharacterHelper(proxy);
         mPhysicsHelper->SetBaseInterfaceClass(this);
         mPhysicsHelper->SetCharacterInterfaceClass(this);
         mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_CHARACTER | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
#endif
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

#ifdef AGEIA_PHYSICS
         if(IsRemote())
            return;

         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();

         if(mTimesASecondYouCanSendOutAnUpdate == 0)
         {
            LOG_ERROR("Not sending out dead reckoning, the mTimesASecondYouCanSendOutAnUpdate is set to 0");
            return;
         }

         mTimeForSendingDeadReckoningInfoOut += ElapsedTime;

         if(mTimeForSendingDeadReckoningInfoOut > 1.0f / mTimesASecondYouCanSendOutAnUpdate)
         {
            mTimeForSendingDeadReckoningInfoOut = 0.0f;
         }
         else
            return;

         if( mPhysicsHelper != NULL )
         {
            if( mPhysicsHelper->GetActor() != NULL && mPhysicsHelper->GetActor()->readBodyFlag(NX_BF_DISABLE_GRAVITY))
            {
               std::vector<dtDAL::ActorProxy*> toFill;
               GetGameActorProxy().GetGameManager()->FindActorsByClassName("NxAgeiaTerraPageLand", toFill);
               if(toFill.size())
               {
                  NxAgeiaTerraPageLandActor* landActor = dynamic_cast<NxAgeiaTerraPageLandActor*>((*toFill.begin())->GetActor());
                  if(landActor != NULL)
                  {
                     if(landActor->HasSomethingBeenLoaded())
                     {
                        mPhysicsHelper->GetActor()->clearBodyFlag(NX_BF_DISABLE_GRAVITY);
                     }
                  }
               }
            }

            if(mPhysicsHelper->GetActor() != NULL)
            {
               float amountChange = GetMaxTranslationError();//0.5f;
               float glmat[16];
               dtPhysics::PhysicsObject* physxObj = mPhysicsHelper->GetActor();
               NxMat33 rotation = physxObj->getGlobalOrientation();
               rotation.getColumnMajorStride4(glmat);
               glmat[12] = physxObj->getGlobalPosition()[0];
               glmat[13] = physxObj->getGlobalPosition()[1];
               glmat[14] = physxObj->getGlobalPosition()[2];
               glmat[15] = 1.0f;

               osg::Matrix currentMatrix(glmat);
               osg::Vec3 globalOrientation;
               dtUtil::MatrixUtil::MatrixToHpr(globalOrientation, currentMatrix);
               osg::Vec3 nxVecTemp;
               nxVecTemp.set( mPhysicsHelper->GetActor()->getGlobalPosition().x,
                              mPhysicsHelper->GetActor()->getGlobalPosition().y,
                              mPhysicsHelper->GetActor()->getGlobalPosition().z);

               const osg::Vec3 &translationVec = GetDeadReckoningHelper().GetCurrentDeadReckonedTranslation();
               const osg::Vec3 &orientationVec = GetDeadReckoningHelper().GetCurrentDeadReckonedRotation();

               mNotifyChangePosition = dtUtil::Equivalent(*((osg::Vec3*)(&nxVecTemp)), translationVec, amountChange);
               mNotifyChangeOrient = !dtUtil::Equivalent(globalOrientation, orientationVec, osg::Vec3::value_type(3.0f));

               GetDeadReckoningHelper().SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);

               const osg::Vec3 &turnVec = GetDeadReckoningHelper().GetAngularVelocityVector();
               const osg::Vec3 &velocityVec = GetVelocityVector();

               osg::Vec3 angularVelocity(physxObj->getAngularVelocity().x, physxObj->getAngularVelocity().y, physxObj->getAngularVelocity().z);
               osg::Vec3 linearVelocity(physxObj->getLinearVelocity().x, physxObj->getLinearVelocity().y, physxObj->getLinearVelocity().z);

               float velocityDiff = (velocityVec - linearVelocity).length();
               bool velocityNearZero = linearVelocity.length() < 0.1;
               mNotifyChangeVelocity = velocityDiff > 0.2 || (velocityNearZero && velocityVec.length() > 0.1 );

               if( mNotifyChangeVelocity )
               {
                  if( velocityNearZero )
                  {
                     // DEBUG:
                     /*std::cout << "\tStopping"
                        << "\n\tDifference:\t" << velocityDiff
                        << "\n\tOld velocity:\t" << velocityVec
                        << "\n\tNew velocity:\t" << linearVelocity << std::endl;//*/

                     linearVelocity.set(0.0,0.0,0.0);
                  }
                  SetVelocityVector(linearVelocity);
               }
            }
         }
#endif
         Human::OnTickLocal(tickMessage);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::SetLastKnownRotation(const osg::Vec3 &vec)
      {
         Human::SetLastKnownRotation( osg::Vec3( vec.x(), 0.0f, 0.0f) );
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::SetLastKnownTranslation(const osg::Vec3 &vec)
      {
#ifdef AGEIA_PHYSICS
         if(mPhysicsHelper != NULL)
         {
            float zValue = mPhysicsHelper->GetCharacterExtents()[2];

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

      ////////////////////////////////////////////////////////////////////////////////////
      bool HumanWithPhysicsActor::ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate)
      {
         osg::Vec3 position = pos;
#ifdef AGEIA_PHYSICS
         if(mPhysicsHelper != NULL)
            position[2] -= (mPhysicsHelper->GetCharacterExtents()[2]);
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

         // DEBUG:
         /*if( mNotifyChangeVelocity )
         {
            std::cout << "\n\tUpdate Velocity:\t" << GetVelocityVector() << std::endl;
         }//*/

         // Do full updates for now until partial updates are required.
         return mNotifyChangeVelocity ||
            mNotifyChangeOrient || mNotifyChangePosition;
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::FillPartialUpdatePropertyVector(std::vector<std::string>& propNamesToFill)
      {
         SimCore::Actors::BaseEntity::FillPartialUpdatePropertyVector(propNamesToFill);
         propNamesToFill.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_VELOCITY_VECTOR);
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::SetMovementTransform(const osg::Vec3& movement)
      {
#ifdef AGEIA_PHYSICS
         if( mPhysicsHelper->GetActor() != NULL && mPhysicsHelper->GetActor()->readBodyFlag(NX_BF_DISABLE_GRAVITY))
         {
            return;
         }

         mSentOverTransform = movement;

         NxVec3 displacementVector;
         osg::Vec3 currentTransform = mSentOverTransform;
         displacementVector.x = currentTransform[0] - mPreviousTransform[0];
         displacementVector.y = currentTransform[1] - mPreviousTransform[1];
         displacementVector.z = currentTransform[2] - mPreviousTransform[2];
         mPreviousTransform = currentTransform;

         dtGame::GMComponent *comp =
            GetGameActorProxy().GetGameManager()->GetComponentByName(dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_NAME);

         dtAgeiaPhysX::NxAgeiaWorldComponent &ageiaComp =
            static_cast<dtAgeiaPhysX::NxAgeiaWorldComponent&>(*comp);
         NxVec3 gravity = ageiaComp.GetGravity(mPhysicsHelper->GetSceneName());

         displacementVector.x *= mMoveRateConstant[0];
         displacementVector.y *= mMoveRateConstant[1];

         // Changed by Eddie. This Z value should be set from the gravity of the world,
         // not hardcoded to -9.8, as this would overwrite the value supplied by the user
         // This still isn't entirely correct, as gravity can be set in any direction,
         // and should be applied to all values of the displacement vector,
         // not just the Z
         displacementVector.z = gravity[2];//-9.8f;
         //displacementVector.z = 0.0;

         //printf("%f %f %f\n", displacementVector.x, displacementVector.y, displacementVector.z);

         // Note time needs to be send in here correctly.
         mPhysicsHelper->UpdatePhysicsCharacterController(displacementVector, 1.0f/60.0f, 0);
#endif
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         Human::OnTickRemote(tickMessage);
#ifdef AGEIA_PHYSICS
         dtCore::Transform transform;
         GetTransform(transform);

         transform.GetTranslation(mPreviousTransform);

         osg::Vec3 offset = mPhysicsHelper->GetDimensions();
         osg::Vec3 xyz;
         transform.GetTranslation(xyz);

         mPhysicsHelper->ForcefullyMoveCharacterPos(NxVec3(xyz[0], xyz[1], xyz[2] + ( offset[2] / 2 ) ) );
#endif
      }

      ////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::SetPosition( const osg::Vec3& position )
      {
         dtCore::Transform xform;
         xform.SetTranslation( position );
         SetTransform(xform);

#ifdef AGEIA_PHYSICS
         if( ! mPhysicsHelper.valid() )
         {
            return;
         }

         mPhysicsHelper->ForcefullyMoveCharacterPos(NxVec3(position.x(), position.y(), position.z()));

         if(!IsRemote())
         {
            if (mPhysicsHelper->GetActor() != NULL)
               mPhysicsHelper->GetActor()->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
         }
#endif
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

#ifdef AGEIA_PHYSICS
         // We don't want remote players to kick the HMMWV around, so we put them in a different group.
         if (IsRemote())
            mPhysicsHelper->SetCollisionGroup(31);

         mPhysicsHelper->SetDimensions(osg::Vec3(0.5, 0.5, 1.0));
         mPhysicsHelper->InitializeCharacter();
         dtCore::Transform transform;
         GetTransform(transform);
         osg::Vec3 xyz;
         transform.GetTranslation(xyz);

         SetPosition(xyz);
         dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"))->RegisterAgeiaHelper(*mPhysicsHelper.get());

#endif

         if(!IsRemote())
         {
            GetDeadReckoningHelper().SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION);
         }
      }

#ifdef AGEIA_PHYSICS
      //////////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::AgeiaPrePhysicsUpdate()
      {
      }

      //////////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::AgeiaPostPhysicsUpdate()
      {
         if(!IsRemote())
         {
            dtCore::Transform xform;
            GetTransform(xform);
            const NxExtendedVec3* pos = mPhysicsHelper->GetCharacterPos();
            xform.SetTranslation(pos->x, pos->y, pos->z);
            SetTransform(xform);
            xform.GetTranslation(mPreviousTransform);
         }
      }

      //////////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit)
      {

      }

      //////////////////////////////////////////////////////////////////////////////
      NxControllerAction HumanWithPhysicsActor::AgeiaCharacterShapeReport(const NxControllerShapeHit& shapeHit)
      {
         if(shapeHit.shape->getActor().userData != NULL)
         {
            dtPhysics::PhysicsHelper* physicsHelper =
               (dtPhysics::PhysicsHelper*)(shapeHit.shape->getActor().userData);
            NxAgeiaFourWheelVehicleActor *hitTarget = NULL;
            if (physicsHelper != NULL)
            {
               hitTarget = dynamic_cast<NxAgeiaFourWheelVehicleActor*>(physicsHelper->GetPhysicsGameActorProxy().GetActor());
               if(hitTarget != NULL)
               {
                  // we hit a vehicle lets do something with the character
               }
            }
         }
         return NX_ACTION_NONE;
      }

      //////////////////////////////////////////////////////////////////////////////
      NxControllerAction HumanWithPhysicsActor::AgeiaCharacterControllerReport(const NxControllersHit& controllerHit)
      {
         return NX_ACTION_NONE;
      }
#endif

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
      void HumanWithPhysicsActorProxy::CreateActor()
      {
         HumanWithPhysicsActor* p = new HumanWithPhysicsActor(*this);
         SetActor(*p);

         if(!IsRemote())
         {
            p->InitDeadReckoningHelper();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActorProxy::BuildPropertyMap()
      {
         HumanActorProxy::BuildPropertyMap();
#ifdef AGEIA_PHYSICS
         HumanWithPhysicsActor* actor = dynamic_cast<HumanWithPhysicsActor*>(GetActor());
         std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
         actor->GetPhysicsHelper()->BuildPropertyMap(toFillIn);
         for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
            AddProperty(toFillIn[i].get());
#endif
      }

      //////////////////////////////////////////////////////////////////////////
      void HumanWithPhysicsActorProxy::BuildInvokables()
      {
         HumanActorProxy::BuildInvokables();
      }
   }
}
