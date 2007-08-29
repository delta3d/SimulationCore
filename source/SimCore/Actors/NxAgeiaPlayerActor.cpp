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

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::TickLocal(const dtGame::Message &tickMessage)
      {
         Human::TickLocal(tickMessage);

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
               float amountChange = 0.5f;
               float glmat[16];
               NxMat33 rotation = mPhysicsHelper->GetActor()->getGlobalOrientation();
               rotation.getColumnMajorStride4(glmat);
               glmat[12] = mPhysicsHelper->GetActor()->getGlobalPosition()[0];
               glmat[13] = mPhysicsHelper->GetActor()->getGlobalPosition()[1];
               glmat[14] = mPhysicsHelper->GetActor()->getGlobalPosition()[2];
               glmat[15] = 1.0f;
               //float zoffset = 0.0;
               float zoffset = 0;
               osg::Matrix currentMatrix(glmat);
               osg::Vec3 globalOrientation;
               dtUtil::MatrixUtil::MatrixToHpr(globalOrientation, currentMatrix);
               osg::Vec3 nxVecTemp;
               nxVecTemp.set(mPhysicsHelper->GetActor()->getGlobalPosition().x, mPhysicsHelper->GetActor()->getGlobalPosition().y, mPhysicsHelper->GetActor()->getGlobalPosition().z + zoffset);

               const osg::Vec3 &translationVec = GetDeadReckoningHelper().GetCurrentDeadReckonedTranslation();
               const osg::Vec3 &orientationVec = GetDeadReckoningHelper().GetCurrentDeadReckonedRotation();
               if(!dtUtil::Equivalent<osg::Vec3, float>(nxVecTemp, translationVec, 3, amountChange) || 
                  !dtUtil::Equivalent<osg::Vec3, float>(globalOrientation, orientationVec, 3, 3.0f))
               {
                  osg::Vec3 globalPosition; 
                  globalPosition.set(  mPhysicsHelper->GetActor()->getGlobalPosition().x, 
                                       mPhysicsHelper->GetActor()->getGlobalPosition().y,
                                       mPhysicsHelper->GetActor()->getGlobalPosition().z + zoffset);
                  SetLastKnownTranslation(globalPosition);
                  SetLastKnownRotation(globalOrientation);
                  
                  osg::Vec3 AngularVelocity; 
                  AngularVelocity.set( mPhysicsHelper->GetActor()->getAngularVelocity().x, 
                                       mPhysicsHelper->GetActor()->getAngularVelocity().y, 
                                       mPhysicsHelper->GetActor()->getAngularVelocity().z);
                  
                  //printf("Angular velocity %f %f %f\n", AngularVelocity[0], AngularVelocity[1], AngularVelocity[2]);

                  SetAngularVelocityVector(AngularVelocity);

                  osg::Vec3 linearVelocity; 
                  linearVelocity.set(  mPhysicsHelper->GetActor()->getLinearVelocity().x, 
                                       mPhysicsHelper->GetActor()->getLinearVelocity().y, 
                                       mPhysicsHelper->GetActor()->getLinearVelocity().z);              
                  
                  //printf("linearVelocity %f %f %f\n", linearVelocity[0], linearVelocity[1], linearVelocity[2]);

                  SetVelocityVector(linearVelocity);
                  
                  GetGameActorProxy().NotifyFullActorUpdate();
               }
               else
               {
                  printf("Not sending out\n");
               }
            }
         }
#endif
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

         if(terrainNode != NULL)
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
         }
         
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
         mPhysicsHelper->ForcefullyMoveCharacterPos(NxVec3(transform.GetTranslation()[0], transform.GetTranslation()[1], transform.GetTranslation()[2]) );
#endif
      }

      ////////////////////////////////////////////////////////////
      void NxAgeiaPlayerActor::OnEnteredWorld()
      {
         Human::OnEnteredWorld();

#ifdef AGEIA_PHYSICS
         mPhysicsHelper->InitializeCharacter();
         dtCore::Transform transform;
         GetTransform(transform);
         mPhysicsHelper->ForcefullyMoveCharacterPos(NxVec3(transform.GetTranslation()[0], transform.GetTranslation()[1], transform.GetTranslation()[2]));

         if(!IsRemote())
         {
            mPhysicsHelper->GetActor()->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
         }

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
         dtCore::Transform xform;
         GetTransform(xform);
         const NxExtendedVec3* pos = mPhysicsHelper->GetCharacterPos();
         xform.SetTranslation(pos->x, pos->y, pos->z);
         SetTransform(xform);
         mPreviousTransform = xform.GetTranslation();
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

      NxAgeiaPlayerActorProxy::~NxAgeiaPlayerActorProxy()
      {

      }

      void NxAgeiaPlayerActorProxy::BuildPropertyMap()
      {
         HumanActorProxy::BuildPropertyMap();
#ifdef AGEIA_PHYSICS
         NxAgeiaPlayerActor* actor = dynamic_cast<NxAgeiaPlayerActor*>(GetActor());
         actor->mPhysicsHelper->BuildPropertyMap();
#endif
      }

      void NxAgeiaPlayerActorProxy::BuildInvokables()
      {
         HumanActorProxy::BuildInvokables();
      }
   }
}
