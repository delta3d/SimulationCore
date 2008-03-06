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
#include <prefix/SimCorePrefix-src.h>
#ifdef AGEIA_PHYSICS
#include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
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
#include <dtGame/basemessages.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/PortalActor.h>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////////////////////////////////
      NxAgeiaFourWheelVehicleActor ::NxAgeiaFourWheelVehicleActor(PlatformActorProxy &proxy) 
         : Platform(proxy)
      , mLastGearChange(FIRST_GEAR)
      , SOUND_BRAKE_SQUEAL_AMOUNT(0.0f)     
      , SOUND_GEAR_CHANGE_LOW(0.0f)         
      , SOUND_GEAR_CHANGE_MEDIUM(0.0f)      
      , SOUND_GEAR_CHANGE_HIGH(0.0f)        
      , mHasDriver(false)
      , mNotifyFullUpdate(true)
      , mNotifyPartialUpdate(true)
      , mPerformAboveGroundSafetyCheck(true)
      {
         mTimeForSendingDeadReckoningInfoOut = 0.0f;
         mTimesASecondYouCanSendOutAnUpdate  = 3.0f;
         mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper(proxy);
         mPhysicsHelper->SetBaseInterfaceClass(this);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      NxAgeiaFourWheelVehicleActor::~NxAgeiaFourWheelVehicleActor(void)
      {
         if(mSndBrake.valid())
         {
            mSndBrake->Stop();
            RemoveChild(mSndBrake.get());
            mSndBrake.release();
         }
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
         if(mSndAcceleration.valid())
         {
            mSndAcceleration->Stop();
            RemoveChild(mSndAcceleration.get());
            mSndAcceleration.release();
         }
         
         if(!IsRemote() && mVehiclesPortal.valid() )
         {
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(NULL);
            GetGameActorProxy().GetGameManager()->DeleteActor(*mVehiclesPortal.get());
            mVehiclesPortal = NULL;
         }
         
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::OnEnteredWorld()
      {
         Platform::OnEnteredWorld();

         if(SOUND_EFFECT_IGNITION != "")
         {
            mSndIgnition       = dtAudio::AudioManager::GetInstance().NewSound();
            mSndIgnition->LoadFile(SOUND_EFFECT_IGNITION.c_str());
            mSndIgnition->ListenerRelative(false);
            AddChild(mSndIgnition.get());
            dtCore::Transform xform;
            mSndIgnition->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_VEHICLE_LOOP != "")
         {
            mSndVehicleIdleLoop= dtAudio::AudioManager::GetInstance().NewSound();
            mSndVehicleIdleLoop->LoadFile(SOUND_EFFECT_VEHICLE_LOOP.c_str());
            mSndVehicleIdleLoop->ListenerRelative(false);
            mSndVehicleIdleLoop->SetLooping(true);
            mSndVehicleIdleLoop->SetMinDistance(8.0f);
            mSndVehicleIdleLoop->SetMaxDistance(35.0f);
            AddChild(mSndVehicleIdleLoop.get());
            dtCore::Transform xform;
            mSndVehicleIdleLoop->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_BRAKE != "")
         {
            mSndBrake          = dtAudio::AudioManager::GetInstance().NewSound();
            mSndBrake->LoadFile(SOUND_EFFECT_BRAKE.c_str());
            mSndBrake->ListenerRelative(true);
            mSndBrake->SetMinDistance(20.0f);
            mSndBrake->SetMaxDistance(160.0f);
            AddChild(mSndBrake.get());
            dtCore::Transform xform;
            mSndBrake->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_ACCELERATION != "")
         {
            mSndAcceleration          = dtAudio::AudioManager::GetInstance().NewSound();
            mSndAcceleration->LoadFile(SOUND_EFFECT_ACCELERATION.c_str());
            mSndAcceleration->ListenerRelative(false);
            AddChild(mSndAcceleration.get());
            dtCore::Transform xform;
            mSndAcceleration->SetTransform(xform, dtCore::Transformable::REL_CS);
         }
         
         osgSim::DOFTransform* Wheel[4];

         Wheel[BACK_LEFT]  = GetNodeCollector()->GetDOFTransform("dof_wheel_lt_02"); 
         Wheel[BACK_RIGHT] = GetNodeCollector()->GetDOFTransform("dof_wheel_rt_02");
         Wheel[FRONT_LEFT] = GetNodeCollector()->GetDOFTransform("dof_wheel_lt_01");
         Wheel[FRONT_RIGHT]= GetNodeCollector()->GetDOFTransform("dof_wheel_rt_01");
         
         dtCore::Transform ourTransform;
         GetTransform(ourTransform);

         GetPhysicsHelper()->SetLocalOffSet(osg::Vec3(0,0,1.1));
         GetPhysicsHelper()->CreateVehicle(ourTransform, GetNodeCollector()->GetDOFTransform("dof_chassis"), Wheel, !IsRemote());
         GetPhysicsHelper()->SetLocalOffSet(osg::Vec3(0,0,0));

         if(!IsRemote())
         {
            GetPhysicsHelper()->TurnObjectsGravityOff("Default");

            GetGameActorProxy().GetGameManager()->CreateActor(
               *EntityActorRegistry::PORTAL_ACTOR_TYPE, mVehiclesPortal);
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(&GetGameActorProxy());
            portal->SetPortalName(GetName());
            portal->SetIsOpen(true);
            GetGameActorProxy().GetGameManager()->AddActor(*mVehiclesPortal.get(), false, true);
         }

         GetPhysicsHelper()->SetAgeiaUserData(mPhysicsHelper.get());

         dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"))->RegisterAgeiaHelper(*mPhysicsHelper.get());

         if(IsRemote()) 
         {
            GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
            GetPhysicsHelper()->SetObjectAsKinematic();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::ApplyForce( const osg::Vec3& force, const osg::Vec3& location )
      {
         GetPhysicsHelper()->GetPhysXObject()->addForce( NxVec3(force[0],force[1],force[2]) );
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::TickLocal(const dtGame::Message &tickMessage)
      {
         NxActor* physicsObject = GetPhysicsHelper()->GetPhysXObject();
         if(physicsObject == NULL)
         {
            LOG_ERROR("BAD PHYSXOBJECT ON VEHICLE!");
            return;
         }

         if(physicsObject->readBodyFlag(NX_BF_DISABLE_GRAVITY))
         {
            std::vector<dtDAL::ActorProxy*> toFill;
            GetGameActorProxy().GetGameManager()->FindActorsByClassName("NxAgeiaTerraPageLand", toFill);
            if(toFill.size())
            {
               NxAgeiaTerraPageLandActor* landActor = dynamic_cast<NxAgeiaTerraPageLandActor*>((*toFill.begin())->GetActor());
               if(landActor != NULL)
               {
                  if(landActor->HasSomethingBeenLoaded())
                     GetPhysicsHelper()->TurnObjectsGravityOn("Default");
               }
            }
         }

         if(physicsObject->isSleeping())   physicsObject->wakeUp(1e30);

         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();

         // Check to see if we are currently up under the earth, if so, snap them back up.
         if (GetPerformAboveGroundSafetyCheck() == true)
         {
            KeepAboveGround(physicsObject);
         }

         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         //                                          Update everything else                                      //
         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         UpdateVehicleTorquesAndAngles(ElapsedTime);
         UpdateRotationDOFS(ElapsedTime, true);
         UpdateSoundEffects(ElapsedTime);
         UpdateDeadReckoning(ElapsedTime);

         // Allow the base class to handle expected base functionality.
         // NOTE: This is called last since the vehicle's position will be final.
         //       The base TickLocal currently queries the vehicle's position and orientation.
         Platform::TickLocal(tickMessage);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::UpdateSoundEffects(float deltaTime)
      {
         ////////////////////////////////////////////////////////////
         // do sound here
         // if the vehicle is moving

         if(mSndVehicleIdleLoop == NULL)
            return;

         if(mSndVehicleIdleLoop->IsPlaying() == false)
            mSndVehicleIdleLoop->Play();

         if(GetMPH() > 1)
         {
            float minpitchBend   = 1.0f;
            float maxpitchBend   = 1.5f;
            float pitchBend      = 1.0f;
            float dis            = 0.0f;
            float dif            = 0.0f;
            float tick           = 0.0f;
            if(GetMPH() < GetSound_gear_change_low())
            {
               if(mLastGearChange != FIRST_GEAR && mSndAcceleration != NULL)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = FIRST_GEAR;
               dis = GetSound_gear_change_low() - GetMPH();
               dif = GetSound_gear_change_low() - 0.0f;
               tick = (maxpitchBend - minpitchBend) / dif;
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else if(GetMPH() < GetSound_gear_change_medium()  && mSndAcceleration != NULL)
            {
               if(mLastGearChange != SECOND_GEAR)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = SECOND_GEAR;
               dis = GetSound_gear_change_medium() - GetMPH();
               dif = GetSound_gear_change_medium() - GetSound_gear_change_low();
               tick = (maxpitchBend - minpitchBend) / dif;
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else if(GetMPH() < GetSound_gear_change_high()  && mSndAcceleration != NULL)
            {
               if(mLastGearChange != THIRD_GEAR)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = THIRD_GEAR;
               dis = GetSound_gear_change_high() - GetMPH();
               dif = GetSound_gear_change_high() - GetSound_gear_change_medium();
               tick = (maxpitchBend - minpitchBend) / dif;
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else if(GetMPH() < GetPhysicsHelper()->GetVehicleMaxMPH())
            {
               if(mLastGearChange != FOURTH_GEAR  && mSndAcceleration != NULL)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = FOURTH_GEAR;
               dis = GetPhysicsHelper()->GetVehicleMaxMPH() - GetMPH();
               dif = GetPhysicsHelper()->GetVehicleMaxMPH() - GetSound_gear_change_high();
               tick = (maxpitchBend - minpitchBend) / dif;               
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else
               pitchBend = maxpitchBend;

            if(pitchBend > maxpitchBend)
               pitchBend = maxpitchBend;

            mSndVehicleIdleLoop->SetPitch(pitchBend);
         }
         else if(GetMPH() < -1 )
         {
            mSndVehicleIdleLoop->SetPitch(1.0f + (-GetMPH() * .005));
         }
         ////////////////////////////////////////////////////////////
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool CompareVectors( const osg::Vec3& op1, const osg::Vec3& op2, float epsilon )
      {
         return std::abs(op1.x() - op2.x()) < epsilon
            && std::abs(op1.y() - op2.y()) < epsilon
            && std::abs(op1.z() - op2.z()) < epsilon;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::UpdateDeadReckoning(float deltaTime)
      {
         if(mTimesASecondYouCanSendOutAnUpdate == 0)
         {
            LOG_ERROR("Not sending out dead reckoning, the mTimesASecondYouCanSendOutAnUpdate is set to 0");
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
         NxActor* physxObj = mPhysicsHelper->GetPhysXObject();

         if(physxObj == NULL)
         {
            LOG_ERROR("No physics object on the hmmwv, no doing dead reckoning");
            return;
         }

         NxMat33 rotation = physxObj->getGlobalOrientation();
         rotation.getColumnMajorStride4(glmat);
         glmat[12] = physxObj->getGlobalPosition()[0];
         glmat[13] = physxObj->getGlobalPosition()[1];
         glmat[14] = physxObj->getGlobalPosition()[2];
         glmat[15] = 1.0f;
         //float zoffset = 0.0;
         float zoffset = 1.1;
         osg::Matrix currentMatrix(glmat);
         osg::Vec3 globalOrientation;
         dtUtil::MatrixUtil::MatrixToHpr(globalOrientation, currentMatrix);
         osg::Vec3 physTranslationVec;
         physTranslationVec.set(physxObj->getGlobalPosition().x, physxObj->getGlobalPosition().y, physxObj->getGlobalPosition().z + zoffset);

         // A full update may not be required. Prevent any further network updates.
         // Let the following code determine if this vehicle should be flagged
         // for a full actor update.
         mNotifyFullUpdate = false;
         mNotifyPartialUpdate = false;

         const osg::Vec3 &drTranslationVec = GetDeadReckoningHelper().GetLastKnownTranslation();//GetCurrentDeadReckonedTranslation();
         const osg::Vec3 &drOrientationVec = GetDeadReckoningHelper().GetLastKnownRotation();//GetCurrentDeadReckonedRotation();

         bool changedTrans = CompareVectors(physTranslationVec, drTranslationVec, amountChange);//!dtUtil::Equivalent<osg::Vec3, float>(physTranslationVec, translationVec, 3, amountChange);
         bool changedOrient = !dtUtil::Equivalent<osg::Vec3, float>(globalOrientation, drOrientationVec, 3, 3.0f);

         //const osg::Vec3 &drAngularVelocity = GetDeadReckoningHelper().GetAngularVelocityVector();
         const osg::Vec3 &drLinearVelocity = GetDeadReckoningHelper().GetVelocityVector();

         osg::Vec3 physAngularVelocity(physxObj->getAngularVelocity().x, physxObj->getAngularVelocity().y, physxObj->getAngularVelocity().z);
         osg::Vec3 physLinearVelocity(physxObj->getLinearVelocity().x, physxObj->getLinearVelocity().y, physxObj->getLinearVelocity().z);

         float linearVelocityDiff = (drLinearVelocity - physLinearVelocity).length();
         bool physVelocityNearZero = physLinearVelocity.length() < 0.1;
         bool changedVelocity = linearVelocityDiff > 0.2 || (physVelocityNearZero && drLinearVelocity.length() > 0.1 );

         if( changedTrans || changedOrient || changedVelocity )
         {
            SetLastKnownTranslation(physTranslationVec);
            SetLastKnownRotation(globalOrientation);
            SetAngularVelocityVector(physAngularVelocity);

            if( physVelocityNearZero )
            {
               physLinearVelocity.set(0.0,0.0,0.0);
            }
            SetVelocityVector(physLinearVelocity);

            // do not send the update message here but rather flag this vehicle
            // to send the update via the base class though ShouldForceUpdate.
            mNotifyFullUpdate = true; 
         }
         else if( GetArticulationHelper() != NULL && GetArticulationHelper()->IsDirty() )
         {
            mNotifyPartialUpdate = true;
            // DEBUG: std::cout << "Articulation Update\n" << std::endl;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool NxAgeiaFourWheelVehicleActor::ShouldForceUpdate(
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
      float NxAgeiaFourWheelVehicleActor::GetPercentageChangeDifference(float startValue, float newValue)
      {
         if(std::abs(startValue) < 0.01f && std::abs(newValue) < 0.01f)
            return 1.0;

         if(startValue == 0)
            startValue = 1.0f;

         return std::abs((((newValue - startValue) / startValue) * 100.0f));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::TickRemote(const dtGame::Message &tickMessage)
      {
         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         UpdateSoundEffects(ElapsedTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::AgeiaPrePhysicsUpdate()
      {
         osg::Matrix rot = GetMatrixNode()->getMatrix();
         
         NxActor* toFillIn = GetPhysicsHelper()->GetPhysXObject();
         if(toFillIn != NULL)
         {
            toFillIn->setGlobalPosition(NxVec3(rot.operator ()(3,0), rot.operator ()(3,1), rot.operator ()(3,2)));
            toFillIn->setGlobalOrientation(
               NxMat33( NxVec3(rot.operator ()(0,0), rot.operator ()(0,1), rot.operator ()(0,2)),
                        NxVec3(rot.operator ()(1,0), rot.operator ()(1,1), rot.operator ()(1,2)),
                        NxVec3(rot.operator ()(2,0), rot.operator ()(2,1), rot.operator ()(2,2))));
         }

      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::AgeiaPostPhysicsUpdate(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& 
         contactReport, NxActor& ourSelf, NxActor& whatWeHit)
      {
         //printf("VehicleCollision\n");
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
      {
         osg::ref_ptr<osgSim::DOFTransform> Wheel[4];
         osg::ref_ptr<osgSim::DOFTransform> steeringWheel;
         if(!insideVehicle)
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
               if(ourInterior != NULL)
               {
                  steeringWheel = ourInterior->GetSteeringWheelDOF("dof_steering_wheel");
                  if(steeringWheel != NULL)
                  {
                     osg::Vec3 HPR = steeringWheel->getCurrentHPR();
                     HPR[0] = GetPhysicsHelper()->GetWheelShape(1)->getSteerAngle() * 4;
                     steeringWheel->setCurrentHPR(HPR);
                  }
               }
            }
         }

         if(!insideVehicle)
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
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::ResetVehicle()
      {
         GetPhysicsHelper()->ResetVehicle();
         
         if(mSndIgnition != NULL)
         {
            if(!mSndIgnition->IsPlaying())
               mSndIgnition->Play();
         }

         if(mSndBrake != NULL)
         {
            if(mSndBrake->IsPlaying())
               mSndBrake->Stop();
         }

         if(mSndVehicleIdleLoop != NULL)
         {
            mSndVehicleIdleLoop->SetPitch(1.0f);
            if(!mSndVehicleIdleLoop->IsPlaying())
               mSndVehicleIdleLoop->Play();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
      {
         dtCore::Keyboard *keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
         if(keyboard == NULL)
            return;

         bool steeredThisFrame = true;
         bool accelOrBrakePressedThisFrame = true;

         if( ! IsMobilityDisabled() && GetHasDriver() )
         {
            if (keyboard->GetKeyState('w') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
            {
               GetPhysicsHelper()->ApplyAccel(GetMPH());
            }
            else if (keyboard->GetKeyState('s') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
            {
               GetPhysicsHelper()->ApplyHandBrake(GetMPH());
            }
            else if (!keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               accelOrBrakePressedThisFrame = false;
            }

            if(keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               GetPhysicsHelper()->ApplyBrake(deltaTime);
            }

            if (keyboard->GetKeyState('a') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
            {
               GetPhysicsHelper()->SteerLeft(deltaTime);
            }
            else if(keyboard->GetKeyState('d') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
            {
               GetPhysicsHelper()->SteerRight(deltaTime);
            }
            else
            {
               steeredThisFrame = false;
            }
         }
         else
         {
            steeredThisFrame = false;
            accelOrBrakePressedThisFrame = true;
            GetPhysicsHelper()->ApplyBrake(deltaTime);
         }
         GetPhysicsHelper()->UpdateVehicle(deltaTime, accelOrBrakePressedThisFrame, steeredThisFrame, GetMPH());
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float NxAgeiaFourWheelVehicleActor::GetMPH()
      {
         return GetVelocityVector().length() * 2.236936291;
         //return GetPhysicsHelper()->GetMPH();
         return 0.0f;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float NxAgeiaFourWheelVehicleActor::GetBrakeTorque()
      {
      return GetPhysicsHelper()->GetBrakeTorque();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::RepositionVehicle(float deltaTime)
      {
         GetPhysicsHelper()->RepositionVehicle(deltaTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::KeepAboveGround( NxActor* physicsObject )
      {
         bool underearth = false;
         std::vector<dtDAL::ActorProxy*> toFill;
         GetGameActorProxy().GetGameManager()->FindActorsByName("Terrain", toFill);
         dtDAL::ActorProxy* terrainNode = NULL;
         if(!toFill.empty())
            terrainNode = (dynamic_cast<dtDAL::ActorProxy*>(&*toFill[0]));

         NxVec3 position = physicsObject->getGlobalPosition();

         osg::Vec3 hp;
         dtCore::RefPtr<dtCore::BatchIsector> iSector = new dtCore::BatchIsector();
         iSector->SetScene( &GetGameActorProxy().GetGameManager()->GetScene() );
         iSector->SetQueryRoot(terrainNode->GetActor());
         dtCore::BatchIsector::SingleISector& SingleISector = iSector->EnableAndGetISector(0);
         osg::Vec3 pos( position.x, position.y, position.z );
         osg::Vec3 endPos = pos;
         pos[2] += 100; 
         endPos[2] -= 100;
         float offsettodo = 5.0f;
         SingleISector.SetSectorAsLineSegment(pos, endPos);
         if( iSector->Update(osg::Vec3(0,0,0), true) )
         {
            if( SingleISector.GetNumberOfHits() > 0 ) 
            {
               SingleISector.GetHitPoint(hp);

               if(position[2] + offsettodo < hp[2])
               {
                  underearth = true;
               }
            }
         }

         if(underearth)
         {
            physicsObject->setGlobalPosition(
               NxVec3(position[0], position[1], hp[2] + offsettodo));
         }
      }


      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////
      NxAgeiaFourWheelVehicleActorProxy::NxAgeiaFourWheelVehicleActorProxy()
      {
         SetClassName("NxAgeiaFourWheeledVehicleActor");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActorProxy::BuildPropertyMap()
      {
         const std::string& VEH_GROUP   = "Vehicle Property Values";
         const std::string& SOUND_GROUP = "Sound Property Values";

         PlatformActorProxy::BuildPropertyMap();

         NxAgeiaFourWheelVehicleActor  &actor = static_cast<NxAgeiaFourWheelVehicleActor &>(GetGameActor());
         std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
         actor.GetPhysicsHelper()->BuildPropertyMap(toFillIn);
         for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
            AddProperty(toFillIn[i].get());

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "VEHICLE_INSIDE_MODEL", "VEHICLE_INSIDE_MODEL_PATH", dtDAL::MakeFunctor(actor, 
            &NxAgeiaFourWheelVehicleActor::SetVehicleInsideModel),
            "What is the filepath / string of the inside model", VEH_GROUP));
         
         AddProperty(new dtDAL::FloatActorProperty("SOUND_BRAKE_SQUEAL_AMOUNT", "How much MPH for Squeal Brake",
            dtDAL::MakeFunctor(actor, &NxAgeiaFourWheelVehicleActor ::SetSound_brake_squeal_amount),
            dtDAL::MakeFunctorRet(actor, &NxAgeiaFourWheelVehicleActor ::GetSound_brake_squeal_amount),
            "How many mph does the car have to go to squeal used with BRAKE_STOP_NOW_BRAKE_TIME", SOUND_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("SOUND_GEAR_CHANGE_LOW", "MPH When Gear Change Low",
            dtDAL::MakeFunctor(actor, &NxAgeiaFourWheelVehicleActor ::SetSound_gear_change_low),
            dtDAL::MakeFunctorRet(actor, &NxAgeiaFourWheelVehicleActor ::GetSound_gear_change_low),
            "At what speed play the acceleration sound effect / reset idle pitch", SOUND_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("SOUND_GEAR_CHANGE_MEDIUM", "MPH When Gear Change Medium",
            dtDAL::MakeFunctor(actor, &NxAgeiaFourWheelVehicleActor ::SetSound_gear_change_medium),
            dtDAL::MakeFunctorRet(actor, &NxAgeiaFourWheelVehicleActor ::GetSound_gear_change_medium),
            "At what speed play the acceleration sound effect / reset idle pitch", SOUND_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("SOUND_GEAR_CHANGE_HIGH", "MPH When Gear Change High",
            dtDAL::MakeFunctor(actor, &NxAgeiaFourWheelVehicleActor ::SetSound_gear_change_high),
            dtDAL::MakeFunctorRet(actor, &NxAgeiaFourWheelVehicleActor ::GetSound_gear_change_high),
            "At what speed play the acceleration sound effect / reset idle pitch", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_IGNITION", "SFX Ignition Path", dtDAL::MakeFunctor(actor, 
            &NxAgeiaFourWheelVehicleActor::SetSound_effect_ignition),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_VEHICLE_LOOP", "SFX Vehicle Idle Path", dtDAL::MakeFunctor(actor, 
            &NxAgeiaFourWheelVehicleActor::SetSound_effect_vehicle_loop),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_BRAKE", "SFX Brake Squeal Path", dtDAL::MakeFunctor(actor, 
            &NxAgeiaFourWheelVehicleActor::SetSound_effect_brake),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_ACCELERATION", "SFX Acceleration Path", dtDAL::MakeFunctor(actor, 
            &NxAgeiaFourWheelVehicleActor::SetSound_effect_acceleration),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SOUND_EFFECT_COLLISION_HIT", "SFX Collision Hit Path", dtDAL::MakeFunctor(actor, 
            &NxAgeiaFourWheelVehicleActor::SetSound_effect_collision_hit),
            "What is the filepath / string of the sound effect", SOUND_GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("PERFORM_ABOVE_GROUND_SAFETY_CHECK",
            "Perform above ground safety check",
            dtDAL::MakeFunctor(actor, &NxAgeiaFourWheelVehicleActor::SetPerformAboveGroundSafetyCheck),
            dtDAL::MakeFunctorRet(actor, &NxAgeiaFourWheelVehicleActor::GetPerformAboveGroundSafetyCheck),
            "Use an Isector as a safety check to keep the vehicle above ground if the collision detection fails.",
            VEH_GROUP));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      NxAgeiaFourWheelVehicleActorProxy::~NxAgeiaFourWheelVehicleActorProxy(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActorProxy::CreateActor()
      {
         SetActor(*new NxAgeiaFourWheelVehicleActor(*this));

         BaseEntity* entityActor = dynamic_cast<BaseEntity*> (GetActor());
         if( entityActor != NULL )
         {
            entityActor->InitDeadReckoningHelper();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActorProxy::OnEnteredWorld()
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
