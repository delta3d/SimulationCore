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
* @author Allen Danklefsen
*/
#include <prefix/SimCorePrefix.h>
#ifdef AGEIA_PHYSICS
#include <SimCore/Actors/NECCBoatActor.h>
#include <NxAgeiaWorldComponent.h>

#include <dtDAL/enginepropertytypes.h>

#include <dtABC/application.h>

#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>

#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/batchisector.h>
#include <dtCore/keyboard.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>

#include <osg/MatrixTransform>

#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PortalActor.h>
#include <NxWheelShapeDesc.h>
#include <NxMat34.h>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      // This class is used to prevent a collision with our selves.
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      class BoatToLandReport : public NxUserRaycastReport
      {
      public:
         /////////////////////////////////////////////////////////////////////////////////////////////
         BoatToLandReport(/*dtPhysics::PhysicsObject *actor, */dtCore::DeltaDrawable* ownerActor) : NxUserRaycastReport()
            //, mOurActor(actor)
            , mGotAHit(false)
            , mOwnerActor(ownerActor)
            , mClosestHitsHelper(nullptr)
         {
         }

         /////////////////////////////////////////////////////////////////////////////////////////////
         virtual ~BoatToLandReport(){}

         /////////////////////////////////////////////////////////////////////////////////////////////
         virtual bool onHit(const NxRaycastHit& hit)
         {
            dtAgeiaPhysX::NxAgeiaPhysicsHelper* physicsHelper =
               (dtAgeiaPhysX::NxAgeiaPhysicsHelper*)(hit.shape->getActor().userData);

            dtCore::DeltaDrawable *hitTarget = nullptr;

            if(physicsHelper != nullptr)
            {
               // null checked up above in the return
               hitTarget = physicsHelper->GetPhysicsGameActorProxy().GetActor();
            }

            // We don't want to hit ourselves.  So, if we don't have a 'self' owner, then we take
            // whatever hit we get.  Otherwise, we check the owner drawables
            if (mOwnerActor == nullptr || hitTarget != mOwnerActor
               // So we dont want to return false if collision is off, this onHit is called for
               // every hit along the line, and returning false tells it to stop the raycast
               // report, its amazing how rereading the sdk can help so much :(
               &&  hit.shape->getActor().readActorFlag(NX_AF_DISABLE_COLLISION) == false)
            {
               if (!mGotAHit || mClosestHit.distance > hit.distance)
               {
                  mClosestHitsHelper = physicsHelper;
                  mGotAHit = true;
                  mClosestHit = hit;
               }
            }

            return true;
         }

      public:
         bool mGotAHit;
         NxRaycastHit mClosestHit;
         dtCore::DeltaDrawable *mOwnerActor;
         dtAgeiaPhysX::NxAgeiaPhysicsHelper* mClosestHitsHelper;
      };

      ///////////////////////////////////////////////////////////////////////////////////
      NECCBoatActor ::NECCBoatActor(PlatformActorProxy &proxy)
         : Platform(proxy)
      , mVehicleMPH(0.0f)
      , mVehicleMaxMPH(60.0f)
      , mVehicleMaxReverseMPH(-20.0f)
      , mCurrentSteeringAngle(0)
      , mBobbingTimer(0)
      , mDegreesDifference(0)
      , mBobbingUp(true)
      , mHasDriver(false)
      , mNotifyFullUpdate(true)
      , mNotifyPartialUpdate(true)
      , mMaxBobbingTimeAmount(0.5f)
      {
         mWheels[0] = nullptr;
         mWheels[1] = nullptr;
         mWheels[2] = nullptr;

         mTimeForSendingDeadReckoningInfoOut = 0.0f;
         mTimesASecondYouCanSendOutAnUpdate  = 3.0f;
         mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy);
         mPhysicsHelper->SetBaseInterfaceClass(this);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      NECCBoatActor::~NECCBoatActor(void)
      {
         if(mSndVehicleIdleLoop.valid())
         {
            mSndVehicleIdleLoop->Stop();
            RemoveChild(mSndVehicleIdleLoop.get());
            mSndVehicleIdleLoop.release();
         }
         if(mSndIgnition.valid())
         {
            mSndIgnition->Stop();
            RemoveChild(mSndIgnition.get());
            mSndIgnition.release();
         }

         if(mSndHorn.valid())
         {
            mSndHorn->Stop();
            RemoveChild(mSndHorn.get());
            mSndHorn.release();
         }

         if(!IsRemote() && mVehiclesPortal.valid() )
         {
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(nullptr);
            GetGameActorProxy().GetGameManager()->DeleteActor(*mVehiclesPortal.get());
            mVehiclesPortal = nullptr;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::OnEnteredWorld()
      {
         Platform::OnEnteredWorld();

         if(SOUND_EFFECT_IGNITION != "")
         {
            mSndIgnition       = dtAudio::AudioManager::GetInstance().NewSound();
            mSndIgnition->LoadFile(SOUND_EFFECT_IGNITION.c_str());
            mSndIgnition->SetListenerRelative(false);
            AddChild(mSndIgnition.get());
            dtCore::Transform xform;
            mSndIgnition->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_VEHICLE_LOOP != "")
         {
            mSndVehicleIdleLoop= dtAudio::AudioManager::GetInstance().NewSound();
            mSndVehicleIdleLoop->LoadFile(SOUND_EFFECT_VEHICLE_LOOP.c_str());
            mSndVehicleIdleLoop->SetListenerRelative(false);
            mSndVehicleIdleLoop->SetLooping(true);
            mSndVehicleIdleLoop->SetMaxDistance(35.0f);
            AddChild(mSndVehicleIdleLoop.get());
            dtCore::Transform xform;
            mSndVehicleIdleLoop->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_HORN_SOUND != "")
         {
            mSndHorn= dtAudio::AudioManager::GetInstance().NewSound();
            mSndHorn->LoadFile(SOUND_EFFECT_HORN_SOUND.c_str());
            mSndHorn->SetListenerRelative(false);
            mSndHorn->SetLooping(false);
            mSndHorn->SetMaxDistance(35.0f);
            AddChild(mSndHorn.get());
            dtCore::Transform xform;
            mSndHorn->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         dtCore::Transform ourTransform;
         GetTransform(ourTransform);

         // Create Physics model here!
         CreateBoatVehicle();

         if(!IsRemote())
         {
            GetGameActorProxy().GetGameManager()->CreateActor(
               *EntityActorRegistry::PORTAL_ACTOR_TYPE, mVehiclesPortal);
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(&GetGameActorProxy());
            portal->SetPortalName(GetName());
            portal->SetIsOpen(true);
            GetGameActorProxy().GetGameManager()->AddActor(*mVehiclesPortal.get(), false, true);
            mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_NORMAL | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
         }
         else
         {
            mPhysicsHelper->SetObjectAsKinematic();
            mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
         }

         dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>
            (GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"))->RegisterAgeiaHelper(*mPhysicsHelper.get());
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::CreateBoatVehicle()
      {
         // change matrix at top or down below, but its only made in ident currently
         dtCore::Transform ourTransform, lastTransform;
         GetTransform(ourTransform);

         NxMat34 sendInMatrix;
         sendInMatrix.id();

         SetTransform(lastTransform);

         mPhysicsHelper->SetCollisionGroup(26);
         mPhysicsHelper->SetIsKinematic(false);
         mPhysicsHelper->SetPhysicsModelTypeEnum(dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CONVEXMESH);
         mPhysicsHelper->InitializePrimitive(GetOSGNode(), sendInMatrix);
         dtPhysics::PhysicsObject* actor = mPhysicsHelper->GetPhysicsObject();

         SetTransform(ourTransform);

         if(actor == nullptr)
         {
            // should prob throw an exception
            return;
         }

         NxMaterialIndex materialIndex = 0;

         //////////////////////////////////////////////
         //Find the material, or make a new one
         std::string NameofMaterial = "WheelWithFrictionMaterial";
         dtAgeiaPhysX::NxAgeiaWorldComponent* worldComponent =
            dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"));
         if(worldComponent != nullptr)
         {
            worldComponent->RegisterMaterial(NameofMaterial, 0.75f, 0.5f, 0.2f);
            materialIndex = worldComponent->GetMaterialIndex(NameofMaterial, "Default", true);
         }

         NxVec3 positions[3];
         positions[0].set(0, 3, 0);
         positions[1].set(3, -5, 0);
         positions[2].set(-3, -5, 0);

         // fill in all the data for the wheel
         for(int i = 0 ; i < 3 ; ++i)
         {
            positions[i].set(positions[i].x,
                             positions[i].y,
                             positions[i].z);

            NxWheelShapeDesc wheelShapeDesc;
            wheelShapeDesc.materialIndex = materialIndex;
            wheelShapeDesc.localPose.t = positions[i];
            wheelShapeDesc.group = 26;

            /*
               wheelDesc.wheelApproximation  = 10;
               wheelDesc.wheelRadius         = 0.5;
               wheelDesc.wheelWidth          = 0.3;  // 0.1
               wheelDesc.wheelSuspension     = 1.0;
               wheelDesc.springRestitution   = 100;
               wheelDesc.springDamping       = 0.5;
               wheelDesc.springBias          = 0;
               wheelDesc.maxBrakeForce       = 1;

               NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED |
               NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT
            */

            NxReal mWheelSuspensionAmount = 1.5f;
            NxReal mWheelSizeRadius       = 0.75f;
            NxReal mWheelSpringRestitution= 9000;
            NxReal mWheelSpringDamping    = 10;
            NxReal mWheelSpringBias       = 0;
            NxReal mWheelInverseMass      = 0.075f;

            NxReal heightModifier                  = ((mWheelSuspensionAmount + mWheelSizeRadius) / mWheelSuspensionAmount);
            wheelShapeDesc.suspension.spring       = mWheelSpringRestitution*heightModifier;
            wheelShapeDesc.suspension.damper       = mWheelSpringDamping*heightModifier;
            wheelShapeDesc.suspension.targetValue  = mWheelSpringBias*heightModifier;
            wheelShapeDesc.radius                  = mWheelSizeRadius;
            wheelShapeDesc.suspensionTravel        = mWheelSuspensionAmount;
            wheelShapeDesc.inverseWheelMass        = mWheelInverseMass;

            mWheels[i] = static_cast<NxWheelShape*>(actor->createShape(wheelShapeDesc));

            if(mWheels[i] != nullptr)
            {
               //these flags are no longer in the SDK
               //mWheels[i]->setWheelFlags(NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF
               //| NX_WF_ACCELERATED | NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT);

               NxMat33 orient;
               orient.setRow(0, NxVec3(1,0,0));
               orient.setRow(1, NxVec3(0,0,-1));
               orient.setRow(2, NxVec3(0,1,0));

               NxMat33 mat = mWheels[i]->getGlobalOrientation();
               mat = orient*mat;
               mWheels[i]->setGlobalOrientation(mat);
            }
            else
            {
               // throw exception
            }
         }

         //actor->raiseBodyFlag(NX_BF_FROZEN_ROT_Y);
         actor->setCMassOffsetLocalPosition(NxVec3(0,-1, -3));

         osg::Vec3 xyz;
         ourTransform.GetTranslation(xyz);
         actor->setGlobalPosition(NxVec3(xyz[0],
                                         xyz[1],
                                         xyz[2]));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::OnTickLocal(const dtGame::TickMessage &tickMessage)
      {
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetPhysicsObject();
         if(physicsObject == nullptr)
         {
            // should probably throw an exception
            LOG_ERROR("BAD PHYSXOBJECT ON VEHICLE!");
            return;
         }

         float ElapsedTime = tickMessage.GetDeltaSimTime();

         if(mBobbingUp)
            mBobbingTimer += ElapsedTime;
         else
            mBobbingTimer -= ElapsedTime;

         if(mBobbingTimer > mMaxBobbingTimeAmount)
         {
            mBobbingUp = false;
            mBobbingTimer = mMaxBobbingTimeAmount;
         }
         else if(mBobbingTimer < -mMaxBobbingTimeAmount)
         {
            mBobbingUp = true;
            mBobbingTimer = -mMaxBobbingTimeAmount;
         }

         if(physicsObject->isSleeping())   physicsObject->wakeUp(1e30);

         // check and see if we are intersection with land, if so, we need to slow down drastically to no
         // movement left for the player.

         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         //                                          Update everything else                                      //
         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         UpdateVehicleTorquesAndAngles(ElapsedTime);
         UpdateRotationDOFS(ElapsedTime, true);
         UpdateSoundEffects(ElapsedTime);
         UpdateDeadReckoning(ElapsedTime);

         CheckForGroundCollision();

         // Allow the base class to handle expected base functionality.
         // NOTE: This is called last since the vehicle's position will be final.
         //       The base TickLocal currently queries the vehicle's position and orientation.
         Platform::OnTickLocal(tickMessage);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::UpdateSoundEffects(float deltaTime)
      {
         ////////////////////////////////////////////////////////////
         // do sound here
         // if the vehicle is moving
         if(mSndVehicleIdleLoop == nullptr)
            return;

         if(mSndVehicleIdleLoop->IsPlaying() == false)
            mSndVehicleIdleLoop->Play();

         if(GetMPH() > 1)
         {
            float minpitchBend   = 1.0f;
            float maxpitchBend   = 1.5f;
            float pitchBend      = 1.0f;

            pitchBend = minpitchBend + ((GetMPH() / mVehicleMaxMPH) * (maxpitchBend - minpitchBend));

            if(pitchBend > maxpitchBend)
               pitchBend = maxpitchBend;

            mSndVehicleIdleLoop->SetPitch(pitchBend);
         }
         else
            mSndVehicleIdleLoop->SetPitch(1.0f);
         ////////////////////////////////////////////////////////////
      }

      void NECCBoatActor::UpdateDeadReckoning(float deltaTime)
      {
         if(mTimesASecondYouCanSendOutAnUpdate == 0)
         {
            LOG_ERROR("Not sending out dead reckoning, the mTimesASecondYouCanSendOutAnUpdate is set to 0");
            return;
         }

         if(mPhysicsHelper->GetPhysicsObject() == nullptr)
         {
            LOG_WARNING("PhysX object null in update dead reckoning not sending out!");
            return;
         }

         mTimeForSendingDeadReckoningInfoOut += deltaTime;

         if(mTimeForSendingDeadReckoningInfoOut > 1.0f / mTimesASecondYouCanSendOutAnUpdate)
         {
            mTimeForSendingDeadReckoningInfoOut = 0.0f;
         }
         else
            return;

         float amountChange = 0.5f;
         float glmat[16];
         dtPhysics::PhysicsObject* physxObj = mPhysicsHelper->GetPhysicsObject();
         NxMat33 rotation = physxObj->getGlobalOrientation();
         rotation.getColumnMajorStride4(glmat);
         glmat[12] = physxObj->getGlobalPosition()[0];
         glmat[13] = physxObj->getGlobalPosition()[1];
         glmat[14] = physxObj->getGlobalPosition()[2];
         glmat[15] = 1.0f;
         float zoffset = 0.0;
         //float zoffset = 1.1;
         osg::Matrix currentMatrix(glmat);
         osg::Vec3 globalOrientation;
         dtUtil::MatrixUtil::MatrixToHpr(globalOrientation, currentMatrix);
         osg::Vec3 nxVecTemp;
         nxVecTemp.set(physxObj->getGlobalPosition().x, physxObj->getGlobalPosition().y, physxObj->getGlobalPosition().z + zoffset);

         // A full update may not be required. Prevent any further network updates.
         // Let the following code determine if this vehicle should be flagged
         // for a full actor update.
         mNotifyFullUpdate = false;
         mNotifyPartialUpdate = false;

         const osg::Vec3 &translationVec = GetDeadReckoningHelper().GetLastKnownTranslation();//GetCurrentDeadReckonedTranslation();
         const osg::Vec3 &orientationVec = GetDeadReckoningHelper().GetLastKnownRotation();//GetCurrentDeadReckonedRotation();

         bool changedTrans = dtUtil::Equivalent(nxVecTemp, translationVec, amountChange);//!dtUtil::Equivalent<osg::Vec3, float>(nxVecTemp, translationVec, 3, amountChange);
         bool changedOrient = !dtUtil::Equivalent(globalOrientation, orientationVec, osg::Vec3::value_type(3.0f));

         const osg::Vec3 &velocityVec = GetDeadReckoningHelper().GetLastKnownVelocity();

         osg::Vec3 AngularVelocity(physxObj->getAngularVelocity().x, physxObj->getAngularVelocity().y, physxObj->getAngularVelocity().z);
         osg::Vec3 linearVelocity(physxObj->getLinearVelocity().x, physxObj->getLinearVelocity().y, physxObj->getLinearVelocity().z);

         float velocityDiff = (velocityVec - linearVelocity).length();
         bool velocityNearZero = linearVelocity.length() < 0.1;
         bool changedVelocity = velocityDiff > 0.2 || (velocityNearZero && velocityVec.length() > 0.1 );

         if( changedTrans || changedOrient || changedVelocity )
         {
            SetLastKnownTranslation(nxVecTemp);
            SetLastKnownRotation(globalOrientation);
            SetLastKnownAngularVelocity(AngularVelocity);

            if( velocityNearZero )
            {
               linearVelocity.set(0.0,0.0,0.0);
            }
            SetLastKnownVelocity(linearVelocity);

            // do not send the update message here but rather flag this vehicle
            // to send the update via the base class though ShouldForceUpdate.
            mNotifyFullUpdate = true;
         }
         else if( GetArticulationHelper() != nullptr && GetArticulationHelper()->IsDirty() )
         {
            mNotifyPartialUpdate = true;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool NECCBoatActor::ShouldForceUpdate(
         const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate)
      {
         // UpdateDeadReckoning will have already determined if a full update is necessary.
         fullUpdate = mNotifyFullUpdate;

         // A full update or a dirty articulation will allow for at least a partial update.
         bool forceUpdate = fullUpdate
            || mNotifyPartialUpdate;

         return forceUpdate;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float NECCBoatActor::GetPercentageChangeDifference(float startValue, float newValue)
      {
         if(fabs(startValue) < 0.01f && fabs(newValue) < 0.01f)
            return 1.0;

         if(startValue == 0)
            startValue = 1.0f;

         return fabs((((newValue - startValue) / startValue) * 100.0f));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         float ElapsedTime = tickMessage.GetDeltaSimTime();
         UpdateSoundEffects(ElapsedTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::AgeiaPrePhysicsUpdate()
      {
         osg::Matrix rot = GetMatrixNode()->getMatrix();

         dtPhysics::PhysicsObject* toFillIn = GetPhysicsHelper()->GetPhysicsObject();

         if(toFillIn != nullptr)
         {
            toFillIn->setGlobalPosition(NxVec3(rot.operator ()(3,0), rot.operator ()(3,1), rot.operator ()(3,2)));
            toFillIn->setGlobalOrientation(
               NxMat33( NxVec3(rot.operator ()(0,0), rot.operator ()(0,1), rot.operator ()(0,2)),
                        NxVec3(rot.operator ()(1,0), rot.operator ()(1,1), rot.operator ()(1,2)),
                        NxVec3(rot.operator ()(2,0), rot.operator ()(2,1), rot.operator ()(2,2))));
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::AgeiaPostPhysicsUpdate()
      {
         mPhysicsHelper->SetTransform();

         dtCore::Transform ourTransform;
         GetTransform(ourTransform);

         // this will make the vehicle orientate left / right
         if(mWheels[0] != nullptr)
         {
            NxReal value = mWheels[0]->getSteerAngle();
            osg::Matrix ourMatrix;
            ourTransform.GetRotation(ourMatrix);

            osg::Vec3 hpr;
            dtUtil::MatrixUtil::MatrixToHpr(hpr, ourMatrix);
            hpr[2] = -value * 57.2957795;
            dtUtil::MatrixUtil::HprToMatrix(ourMatrix, hpr);

            ourTransform.SetRotation(ourMatrix);
         }

         // this will make the vehicle orientate forward / backwards
         NxReal value = GetMPH();
         if(value > mVehicleMaxMPH)
            value = mVehicleMaxMPH;

         osg::Matrix ourMatrix;
         ourTransform.GetRotation(ourMatrix);

         float mphScale = (value / mVehicleMaxMPH);

         // to prevent a divide by 0
         if(mphScale == 0)
            mphScale += 0.001f;

         // calculate our bobbing
         mDegreesDifference = (mphScale * MAX_BOBBING_AMOUNT) * (mBobbingTimer / mMaxBobbingTimeAmount);

         osg::Vec3 hpr;
         dtUtil::MatrixUtil::MatrixToHpr(hpr, ourMatrix);
         hpr[1] = mphScale * (8 - mDegreesDifference);
         dtUtil::MatrixUtil::HprToMatrix(ourMatrix, hpr);

         ourTransform.SetRotation(ourMatrix);

         SetTransform(ourTransform);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport&
         contactReport, dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit)
      {
         //printf("VehicleCollision\n");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
      {
         // do whatever dof transforms we have here. wheel / throttle?

         //osg::ref_ptr<osgSim::DOFTransform> Wheel[4];
         //osg::ref_ptr<osgSim::DOFTransform> steeringWheel;

         /*if(!insideVehicle)
         {
            Wheel[BACK_RIGHT] = GetNodeCollector()->GetDOFTransform("dof_wheel_rt_02");
            Wheel[FRONT_RIGHT]= GetNodeCollector()->GetDOFTransform("dof_wheel_rt_01");
            Wheel[BACK_LEFT]  = GetNodeCollector()->GetDOFTransform("dof_wheel_lt_02");
            Wheel[FRONT_LEFT] = GetNodeCollector()->GetDOFTransform("dof_wheel_lt_01");
         }
         else
         {
            std::vector<dtDAL::ActorProxy*> toFillin;
            GetGameActorProxy().GetGameManager()->FindActorsByType(*EntityActorRegistry::INTERIOR_ACTOR_TYPE.get() , toFillin);
            if(toFillin.size())
            {
               InteriorActor* ourInterior = dynamic_cast<InteriorActor*>(toFillin[0]->GetActor());
               if(ourInterior != nullptr)
               {
                  steeringWheel = ourInterior->GetSteeringWheelDOF("dof_steering_wheel");
                  if(steeringWheel != nullptr)
                  {
                     osg::Vec3 HPR = steeringWheel->getCurrentHPR();
                     HPR[0] = GetPhysicsHelper()->GetWheelShape(1)->getSteerAngle() * 4;
                     steeringWheel->setCurrentHPR(HPR);
                  }
               }
            }
         }*/

         /*if(!insideVehicle)
         {
            for(int i = 0 ; i < 4; i++)
            {
               osg::Vec3 HPR;
               HPR[0] = GetPhysicsHelper()->GetWheelShape(i)->getSteerAngle();
               if(i % 2)
               {
                  HPR[1] = GetPhysicsHelper()->GetAxleRotationOne();

               }
               else
               {
                  HPR[1] = GetPhysicsHelper()->GetAxleRotationTwo();
               }
               HPR[2] = 0.0f;
               Wheel[i]->setCurrentHPR(HPR);
            }
         }*/
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::ResetVehicle()
      {
         NxMat34 ourMatrix;
         ourMatrix.t = NxVec3(71815, 42518, 804);
         ourMatrix.M = NxMat33();
         ourMatrix.M.id();
         GetPhysicsHelper()->GetPhysicsObject()->setGlobalPosition(ourMatrix.t);
         GetPhysicsHelper()->GetPhysicsObject()->setGlobalOrientation(ourMatrix.M);

         // reset forces.
         GetPhysicsHelper()->ResetForces();

         for(int i = 0 ; i < 3; ++i)
         {
            if(mWheels[i] != nullptr)
            {
               mWheels[i]->setBrakeTorque(0);
               mWheels[i]->setMotorTorque(0);
            }
         }

         if(mSndIgnition != nullptr)
         {
            if(!mSndIgnition->IsPlaying())
               mSndIgnition->Play();
         }

         if(mSndVehicleIdleLoop != nullptr)
         {
            mSndVehicleIdleLoop->SetPitch(1.0f);
            if(!mSndVehicleIdleLoop->IsPlaying())
               mSndVehicleIdleLoop->Play();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::UpdateVehicleTorquesAndAngles(float deltaTime)
      {
         dtCore::Keyboard *keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
         if(keyboard == nullptr)
            return;

         if(mWheels[0] == nullptr || mWheels[1] == nullptr || mWheels[2] == nullptr )
            // log warning / throw exception
            return;

         static int CURRENT_DRIVING_FORCE_FOR_BOAT = 0;

         if( ! IsMobilityDisabled() && GetHasDriver() )
         {
            if (keyboard->GetKeyState('w') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
            {
               CURRENT_DRIVING_FORCE_FOR_BOAT = 200;
               //++CURRENT_DRIVING_FORCE_FOR_BOAT;
               mWheels[0]->setBrakeTorque(0);
               mWheels[1]->setBrakeTorque(0);
               mWheels[2]->setBrakeTorque(0);

               //GetPhysicsHelper()->ApplyAccel(GetMPH());
            }
            else if (keyboard->GetKeyState('s') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
            {
               CURRENT_DRIVING_FORCE_FOR_BOAT = -200;
               //--CURRENT_DRIVING_FORCE_FOR_BOAT;
               mWheels[0]->setBrakeTorque(0);
               mWheels[1]->setBrakeTorque(0);
               mWheels[2]->setBrakeTorque(0);

               //GetPhysicsHelper()->ApplyHandBrake(GetMPH());
            }
            else if (keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Shift_L))
            {
               CURRENT_DRIVING_FORCE_FOR_BOAT = 0;
               mWheels[1]->setMotorTorque(0);
               mWheels[2]->setMotorTorque(0);
               mWheels[0]->setBrakeTorque(500);
               mWheels[1]->setBrakeTorque(500);
               mWheels[2]->setBrakeTorque(500);

               //accelOrBrakePressedThisFrame = false;
            }

            if(keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               if(mSndHorn.valid())
               {
                  if(mSndHorn->IsPlaying() == false)
                     mSndHorn->Play();
               }
               //GetPhysicsHelper()->ApplyBrake(deltaTime);
            }

            NxReal mWheelTurnRadiusPerUpdate = 0.03f;
            NxReal mWheelTurnRadiusTurnBackPerUpdate = 0.015f;
            NxReal mWheelTurnRadius = 45.0f;

            if (keyboard->GetKeyState('a') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
            {
               if(mCurrentSteeringAngle * (180/osg::PI) < 0)
                  mCurrentSteeringAngle += NxPi*mWheelTurnRadiusPerUpdate*deltaTime*3;
               else
                  mCurrentSteeringAngle += NxPi*mWheelTurnRadiusPerUpdate*deltaTime;

               if(mCurrentSteeringAngle * (180/osg::PI) > mWheelTurnRadius)
                  mCurrentSteeringAngle = mWheelTurnRadius/(180/osg::PI);
               //GetPhysicsHelper()->SteerLeft(deltaTime);
            }
            else if(keyboard->GetKeyState('d') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
            {
               if(mCurrentSteeringAngle * (180/osg::PI) > 0)
                  mCurrentSteeringAngle -= NxPi*mWheelTurnRadiusPerUpdate*deltaTime*3;
               else
                  mCurrentSteeringAngle -= NxPi*mWheelTurnRadiusPerUpdate*deltaTime;

               if(mCurrentSteeringAngle * (180/osg::PI) < -mWheelTurnRadius)
                  mCurrentSteeringAngle = -mWheelTurnRadius/(180/osg::PI);
               //GetPhysicsHelper()->SteerRight(deltaTime);
            }
            else
            {
               if(mCurrentSteeringAngle >  NxPi*mWheelTurnRadiusTurnBackPerUpdate)
                  mCurrentSteeringAngle -= NxPi*mWheelTurnRadiusTurnBackPerUpdate;
               else if(mCurrentSteeringAngle <  -NxPi*mWheelTurnRadiusTurnBackPerUpdate)
                  mCurrentSteeringAngle += NxPi*mWheelTurnRadiusTurnBackPerUpdate;
               else
                  mCurrentSteeringAngle = 0;
               //steeredThisFrame = false;
            }
         }
         else
         {
            //steeredThisFrame = false;
            //accelOrBrakePressedThisFrame = true;
            //GetPhysicsHelper()->ApplyBrake(deltaTime);
         }

         if(GetMPH() > mVehicleMaxMPH && CURRENT_DRIVING_FORCE_FOR_BOAT > 0)
         {
            CURRENT_DRIVING_FORCE_FOR_BOAT *= -1;
         }

         else if(-GetMPH() < mVehicleMaxReverseMPH && CURRENT_DRIVING_FORCE_FOR_BOAT < 0)
         {
            CURRENT_DRIVING_FORCE_FOR_BOAT *= -1;
         }

        /* if(CURRENT_DRIVING_FORCE_FOR_BOAT < mVehicleMaxReverseMPH)
            CURRENT_DRIVING_FORCE_FOR_BOAT = mVehicleMaxReverseMPH;
         else if(CURRENT_DRIVING_FORCE_FOR_BOAT > mVehicleMaxMPH)
            CURRENT_DRIVING_FORCE_FOR_BOAT = mVehicleMaxMPH;*/

         mWheels[0]->setSteerAngle(mCurrentSteeringAngle);
         mWheels[1]->setMotorTorque(-CURRENT_DRIVING_FORCE_FOR_BOAT);
         mWheels[2]->setMotorTorque(-CURRENT_DRIVING_FORCE_FOR_BOAT);

         //GetPhysicsHelper()->UpdateVehicle(deltaTime, accelOrBrakePressedThisFrame, steeredThisFrame, GetMPH());
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float NECCBoatActor::GetMPH()
      {
         return GetLastKnownVelocity().length() * 2.236936291;
         //return GetPhysicsHelper()->GetMPH();
         return 0.0f;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::RepositionVehicle(float deltaTime)
      {
         //GetPhysicsHelper()->RepositionVehicle(deltaTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::CheckForGroundCollision()
      {
         // we could probably play a dirt sound effect if we hit the dirt.
         //
         //

         NxRay ourRay;
         ourRay.orig = GetPhysicsHelper()->GetPhysicsObject()->getGlobalPosition();
         ourRay.dir = NxVec3(0,0,-1);
         NxRaycastHit   mOurHit;

         // Drop a ray through the world to see what we hit. Make sure we don't hit ourselves.  And,
         // Make sure we DO hit hte terrain appropriatelys
         BoatToLandReport myReport(this);
         //NxShape* shape = ourActor->getScene().raycastClosestShape(ourRay, NX_ALL_SHAPES,  mOurHit, (1 << 0));
         NxU32 numHits = GetPhysicsHelper()->GetPhysicsObject()->getScene().raycastAllShapes(ourRay, myReport, NX_ALL_SHAPES,
            (1 << 0) | (1 << 23) | (1 << 26) | (1 << 30) | (1 << 31) );
         if(numHits > 0 && myReport.mGotAHit)
         {
            if(myReport.mClosestHit.shape->getGroup() == 0)
            {
               mWheels[0]->setBrakeTorque(500);
               mWheels[1]->setBrakeTorque(500);
               mWheels[2]->setBrakeTorque(500);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////
      NECCBoatActorProxy::NECCBoatActorProxy()
      {
         SetClassName("NECCBoatActorProxy");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActorProxy::BuildPropertyMap()
      {
         const std::string& SOUND_GROUP = "Sound Property Values";

         PlatformActorProxy::BuildPropertyMap();

         NECCBoatActor  &actor = static_cast<NECCBoatActor &>(GetGameActor());
         std::vector<std::shared_ptr<dtDAL::ActorProperty> >  toFillIn;
         actor.GetPhysicsHelper()->BuildPropertyMap(toFillIn);
         for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
            AddProperty(toFillIn[i].get());

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_IGNITION", "SFX Ignition Path", dtDAL::MakeFunctor(actor,
            &NECCBoatActor::SetSound_effect_ignition),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_VEHICLE_LOOP", "SFX Vehicle Idle Path", dtDAL::MakeFunctor(actor,
            &NECCBoatActor::SetSound_effect_vehicle_loop),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_COLLISION_HIT", "SFX Collision Hit Path", dtDAL::MakeFunctor(actor,
            &NECCBoatActor::SetSound_effect_collision_hit),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_HORN_SOUND", "SFX Horn Path", dtDAL::MakeFunctor(actor,
            &NECCBoatActor::SetSound_Effect_Horn),
            "What is the filepath / string of the sound effect", SOUND_GROUP));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      NECCBoatActorProxy::~NECCBoatActorProxy(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActorProxy::CreateActor()
      {
         SetActor(*new NECCBoatActor(*this));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActorProxy::OnEnteredWorld()
      {
         RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

         if (IsRemote())
            RegisterForMessages(dtGame::MessageType::TICK_REMOTE,
            dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);

         PlatformActorProxy::OnEnteredWorld();
      }

   } // namespace
}// namespace
#endif
