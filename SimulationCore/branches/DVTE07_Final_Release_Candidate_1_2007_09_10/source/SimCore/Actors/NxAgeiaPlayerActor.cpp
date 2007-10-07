/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/NxAgeiaPlayerActor.h>

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
      NxAgeiaPlayerActor::NxAgeiaPlayerActor(dtGame::GameActorProxy &proxy) : 
         Human(proxy)
      , mAcceptInput(false)
      , mNotifyChangePosition(false)
      , mNotifyChangeOrient(false)
      , mNotifyChangeVelocity(false)
      , mMoveRateConstant(25.0f, 25.0f, 0.0f)
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
      NxAgeiaPlayerActor::~NxAgeiaPlayerActor()
      {
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool CompareVectorsPlayer( const osg::Vec3& op1, const osg::Vec3& op2, float epsilon )
      {
         return fabs(op1.x() - op2.x()) < epsilon
            && fabs(op1.y() - op2.y()) < epsilon
            && fabs(op1.z() - op2.z()) < epsilon;
      }

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::TickLocal(const dtGame::Message &tickMessage)
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

            if(mPhysicsHelper->GetActor())
            {
               float amountChange = GetMaxTranslationError();//0.5f;
               float glmat[16];
               NxActor* physxObj = mPhysicsHelper->GetActor();
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

               mNotifyChangePosition = CompareVectorsPlayer(nxVecTemp, translationVec, amountChange);
               mNotifyChangeOrient = !dtUtil::Equivalent<osg::Vec3, float>(globalOrientation, orientationVec, 1, 3.0f);

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
         Human::TickLocal(tickMessage);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::SetLastKnownRotation(const osg::Vec3 &vec)
      {
         Human::SetLastKnownRotation( osg::Vec3( vec.x(), 0.0f, 0.0f) );
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::SetLastKnownTranslation(const osg::Vec3 &vec)
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
      bool NxAgeiaPlayerActor::ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate)
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
      void NxAgeiaPlayerActor::FillPartialUpdatePropertyVector(std::vector<std::string>& propNamesToFill)
      {
         SimCore::Actors::BaseEntity::FillPartialUpdatePropertyVector(propNamesToFill);
         propNamesToFill.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_VELOCITY_VECTOR);
      }

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::SetMovementTransform(const osg::Vec3& movement)
      {
#ifdef AGEIA_PHYSICS
         if( mPhysicsHelper->GetActor() != NULL && mPhysicsHelper->GetActor()->readBodyFlag(NX_BF_DISABLE_GRAVITY))
         {
            return;
         }

         mSentOverTransform = movement;

         bool underearth = false;
         std::vector<dtDAL::ActorProxy*> toFill;
         GetGameActorProxy().GetGameManager()->FindActorsByName("Terrain", toFill);
         dtDAL::ActorProxy* terrainNode = NULL;
         if(!toFill.empty())
            terrainNode = (dynamic_cast<dtDAL::ActorProxy*>(&*toFill[0]));

         /*if(terrainNode != NULL)
         {
            osg::Vec3 hp;
            dtCore::RefPtr<dtCore::BatchIsector> iSector = new dtCore::BatchIsector();
            iSector->SetScene( &GetGameActorProxy().GetGameManager()->GetScene() );
            iSector->SetQueryRoot(terrainNode->GetActor());
            dtCore::BatchIsector::SingleISector& SingleISector = iSector->EnableAndGetISector(0);
            osg::Vec3 pos( mSentOverTransform );
            osg::Vec3 endPos = pos;
            pos[2] += 100; 
            endPos[2] -= 100;
            SingleISector.SetSectorAsLineSegment(pos, endPos);
            if( iSector->Update(osg::Vec3(0,0,0), true) )
            {
               if( SingleISector.GetNumberOfHits() > 0 ) 
               {
                  SingleISector.GetHitPoint(hp);

                  if(mSentOverTransform[2] < hp[2])
                  {
                     underearth = true;
                  }
               }
            }

            if(underearth)
            {
               mPhysicsHelper->ForcefullyMoveCharacterPos(NxVec3(mSentOverTransform[0], mSentOverTransform[1], hp[2] + 2) );
               return;
            }
         }*/
         
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
         //printf("%f %f %f\n", displacementVector.x, displacementVector.y, displacementVector.z);
         NxVec3 gravity = ageiaComp.GetGravity(mPhysicsHelper->GetSceneName());
         
         displacementVector.x *= mMoveRateConstant[0];
         displacementVector.y *= mMoveRateConstant[1];
         
         // Changed by Eddie. This Z value should be set from the gravity of the world, 
         // not hardcoded to -9.8, as this would overwrite the value supplied by the user
         // This still isn't entirely correct, as gravity can be set in any direction, 
         // and should be applied to all values of the displacement vector, 
         // not just the Z
         displacementVector.z = gravity[2];//-9.8f;
  
         //printf("%f %f %f\n", displacementVector.x, displacementVector.y, displacementVector.z);

         mPhysicsHelper->UpdatePhysicsCharacterController(displacementVector, 1.0f/60.0f, 0);
#endif
      }

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::TickRemote(const dtGame::Message &tickMessage)
      {
         Human::TickRemote(tickMessage);
#ifdef AGEIA_PHYSICS
         dtCore::Transform transform;
         GetTransform(transform);
         mPreviousTransform = transform.GetTranslation(); 

         osg::Vec3 offset = mPhysicsHelper->GetDimensions();
         mPhysicsHelper->ForcefullyMoveCharacterPos(NxVec3(transform.GetTranslation()[0], transform.GetTranslation()[1], transform.GetTranslation()[2] + ( offset[2] / 2 ) ) );
#endif
      }

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::SetPosition( const osg::Vec3& position )
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
            mPhysicsHelper->GetActor()->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
         }
#endif
      }

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::OffsetPosition( const osg::Vec3& offset )
      {
         dtCore::Transform xform;
         GetTransform( xform );
         xform.SetTranslation( xform.GetTranslation() + offset );
         SetPosition( xform.GetTranslation() );
      }

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::OnEnteredWorld()
      {
         Human::OnEnteredWorld();

#ifdef AGEIA_PHYSICS
         // We don't want remote players to kick the HMMWV around, so we put them in a different group.
         if (IsRemote())
            mPhysicsHelper->SetCollisionGroup(31);

         mPhysicsHelper->InitializeCharacter();
         dtCore::Transform transform;
         GetTransform(transform);
         SetPosition( transform.GetTranslation() );
         mPhysicsHelper->SetAgeiaUserData(mPhysicsHelper.get());
         dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"))->RegisterAgeiaHelper(*mPhysicsHelper.get());

#endif

         if(!IsRemote())
         {
            GetDeadReckoningHelper().SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION);
         }
      }

#ifdef AGEIA_PHYSICS
      //////////////////////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::AgeiaPrePhysicsUpdate()
      {
      }

      //////////////////////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::AgeiaPostPhysicsUpdate()
      {
         if(!IsRemote())
         {
            dtCore::Transform xform;
            GetTransform(xform);
            const NxExtendedVec3* pos = mPhysicsHelper->GetCharacterPos();
            xform.SetTranslation(pos->x, pos->y, pos->z);
            SetTransform(xform);
            mPreviousTransform = xform.GetTranslation();
         }
      }

      //////////////////////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, NxActor& ourSelf, NxActor& whatWeHit)
      {
         
      }

      //////////////////////////////////////////////////////////////////////////////
      NxControllerAction NxAgeiaPlayerActor::AgeiaCharacterShapeReport(const NxControllerShapeHit& shapeHit)
      {
         if(shapeHit.shape->getActor().userData != NULL)
         {
            dtAgeiaPhysX::NxAgeiaPhysicsHelper* physicsHelper = 
               (dtAgeiaPhysX::NxAgeiaPhysicsHelper*)(shapeHit.shape->getActor().userData);
            NxAgeiaFourWheelVehicleActor *hitTarget = NULL;
            if (physicsHelper != NULL)
            {
               hitTarget = dynamic_cast<NxAgeiaFourWheelVehicleActor*>(physicsHelper->GetPhysicsGameActorProxy()->GetActor());
               if(hitTarget != NULL)
               {
                  // we hit a vehicle lets do something with the character
               }
            }
         }
         return NX_ACTION_NONE;
      }

      //////////////////////////////////////////////////////////////////////////////
      NxControllerAction NxAgeiaPlayerActor::AgeiaCharacterControllerReport(const NxControllersHit& controllerHit)
      {
         return NX_ACTION_NONE;
      }
#endif

      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      NxAgeiaPlayerActorProxy::NxAgeiaPlayerActorProxy()
      {
         SetClassName("NxAgeiaPlayerActor");
      }

      //////////////////////////////////////////////////////////////////////////
      NxAgeiaPlayerActorProxy::~NxAgeiaPlayerActorProxy()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActorProxy::BuildPropertyMap()
      {
         HumanActorProxy::BuildPropertyMap();
#ifdef AGEIA_PHYSICS
         NxAgeiaPlayerActor* actor = dynamic_cast<NxAgeiaPlayerActor*>(GetActor());
         actor->GetPhysicsHelper()->BuildPropertyMap();
#endif
      }

      //////////////////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActorProxy::BuildInvokables()
      {
         HumanActorProxy::BuildInvokables();
      }
   }
}
