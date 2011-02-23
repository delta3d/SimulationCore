/* -*-c++-*-
* Simulation Core
* Copyright 2007-2010, Alion Science and Technology
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
*
* @author Allen Danklefsen
* @author David Guthrie
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/FourWheelVehicleActor.h>
#include <dtDAL/propertymacros.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtDAL/exceptionenum.h>
#include <dtDAL/project.h>
#include <dtCore/batchisector.h>
#include <dtCore/keyboard.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/PortalActor.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/PhysicsTypes.h>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////////////////////////////////
      FourWheelVehicleActor ::FourWheelVehicleActor(BasePhysicsVehicleActorProxy& proxy)
      : BaseClass(proxy)
      , mNumUpdatesUntilFullSteeringAngle(25)
      , mCurrentSteeringAngleNormalized(0.0f)
      , mSoundBrakeSquealSpeed(0.0)
      , mGearChangeLow(10.0)
      , mGearChangeMedium(20.0)
      , mGearChangeHigh(35.0)
      , mLastGearChange(FIRST_GEAR)
      , mCruiseSpeed(0.0f)
      , mStopMode(true)
      , mCruiseMode(false)
      {
         SetEntityType("WheeledVehicle"); // Used for HLA mapping mostly
         SetMunitionDamageTableName("VehicleDamageTable"); // Used for Munitions Damage.
      }

      ///////////////////////////////////////////////////////////////////////////////////
      FourWheelVehicleActor::~FourWheelVehicleActor(void)
      {
         if (mSndBrake.valid())
         {
            mSndBrake->Stop();
            RemoveChild(mSndBrake.get());
            mSndBrake.release();
         }
         if (mSndVehicleIdleLoop.valid())
         {
            mSndVehicleIdleLoop->Stop();
            RemoveChild(mSndVehicleIdleLoop.get());
            mSndVehicleIdleLoop.release();
         }
         if (mSndIgnition.valid())
         {
            mSndIgnition->Stop();
            RemoveChild(mSndIgnition.get());
            mSndIgnition.release();
         }
         if (mSndAcceleration.valid())
         {
            mSndAcceleration->Stop();
            RemoveChild(mSndAcceleration.get());
            mSndAcceleration.release();
         }

         if (!IsRemote() && mVehiclesPortal.valid() )
         {
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(NULL);
            GetGameActorProxy().GetGameManager()->DeleteActor(*mVehiclesPortal.get());
            mVehiclesPortal = NULL;
         }

      }

      ///////////////////////////////////////////////////////////////////////////////////
      SimCore::FourWheelVehiclePhysicsActComp* FourWheelVehicleActor::GetFourWheelPhysicsActComp() const
      {
         return static_cast<SimCore::FourWheelVehiclePhysicsActComp*> (GetPhysicsActComp());
      }

      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, int,   NumUpdatesUntilFullSteeringAngle);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, float, CurrentSteeringAngleNormalized);

      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, float, SoundBrakeSquealSpeed);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, float, GearChangeLow);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, float, GearChangeMedium);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, float, GearChangeHigh);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, dtDAL::ResourceDescriptor, SoundEffectIgnition);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, dtDAL::ResourceDescriptor, SoundEffectIdleLoop);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, dtDAL::ResourceDescriptor, SoundEffectBrake);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, dtDAL::ResourceDescriptor, SoundEffectAcceleration);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, dtDAL::ResourceDescriptor, SoundEffectCollisionHit);
      DT_IMPLEMENT_ACCESSOR(FourWheelVehicleActor, dtDAL::ResourceDescriptor, VehicleInteriorModel);

      ///////////////////////////////////////////////////////////////////////////////////
      bool FourWheelVehicleActor::LoadSound(const dtDAL::ResourceDescriptor& rd, dtCore::RefPtr<dtAudio::Sound>& sound)
      {
         try
         {
            if (rd != dtDAL::ResourceDescriptor::NULL_RESOURCE)
            {
               sound = dtAudio::AudioManager::GetInstance().NewSound();
               sound->LoadFile(dtDAL::Project::GetInstance().GetResourcePath(rd).c_str());
               AddChild(sound);
               dtCore::Transform xform;
               sound->SetTransform(xform, dtCore::Transformable::REL_CS);
               return true;
            }
         }
         catch (const dtDAL::ProjectException& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }
         return false;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActor::OnEnteredWorld()
      {
         EnsureResourcesAreLoaded();

         LoadSound(mSoundEffectIgnition, mSndIgnition);

         if (LoadSound(mSoundEffectIdleLoop, mSndVehicleIdleLoop))
         {
            mSndVehicleIdleLoop->SetLooping(true);
            mSndVehicleIdleLoop->SetMaxDistance(35.0f);
         }

         if (LoadSound(mSoundEffectBrake, mSndBrake))
         {
            mSndBrake->SetListenerRelative(true);
            // max distance makes no sense on listener relative sounds.
            // mSndBrake->SetMaxDistance(160.0f);
         }

         LoadSound(mSoundEffectAcceleration, mSndAcceleration);

         std::vector<osgSim::DOFTransform*> wheels;

         dtUtil::NodeCollector* nodeCollector = GetNodeCollector();

         if (nodeCollector != NULL)
         {
            std::string indexString;

            unsigned axle = 1;
            bool found = true;
            while (found)
            {
               dtUtil::MakeIndexString(axle, indexString, 2);
               wheels.push_back(nodeCollector->GetDOFTransform(std::string("dof_wheel_lt_") + indexString));
               wheels.push_back(nodeCollector->GetDOFTransform(std::string("dof_wheel_rt_") + indexString));
               if (wheels[2 * (axle - 1) + 0] == NULL || wheels[2 * (axle - 1) + 1] == NULL)
               {
                  found = false;
                  // Pop off the last axle since it wasn't found.
                  wheels.pop_back();
                  wheels.pop_back();
               }
               ++axle;
            }
            if (wheels.empty())
            {
               LOGN_ERROR("FourWheelVehicleActor.cpp", "Couldn't find any wheels, unable to create vehicle.");
               return;
            }
         }

         dtCore::Transform ourTransform;
         GetTransform(ourTransform);

         osg::Node* chassis = NULL;
         if (nodeCollector != NULL)
         {
            chassis = nodeCollector->GetDOFTransform("dof_chassis");
            if (chassis == NULL)
            {
               chassis = nodeCollector->GetGroup("Body");
            }

         }

         if (chassis == NULL)
         {
            chassis = GetNonDamagedFileNode();

            if (chassis == NULL)
            {
               LOGN_ERROR("FourWheelVehicleActor.cpp",
                        "Unable to find either a dof_chassis node or an undamaged model node.  Vehicle will not be created.");
            }
            else
            {
               LOGN_WARNING("FourWheelVehicleActor.cpp",
                        "Unable to find either a dof_chassis node.  "
                        "Vehicle will created, but the wheels may be part of the collision geometry,"
                        "and it may not have a proper center of mass.");
            }
         }
         else
         {
            FourWheelVehiclePhysicsActComp* helper = GetFourWheelPhysicsActComp();
            osg::Matrix bodyOffset;
            bodyOffset.makeIdentity();
            helper->GetLocalMatrix(*chassis, bodyOffset);
            bodyOffset.setTrans(bodyOffset.getTrans() - helper->GetMainPhysicsObject()->GetOriginOffset());
            dtCore::Transform offsetXform;
            offsetXform.Set(bodyOffset);

            helper->GetMainPhysicsObject()->SetVisualToBodyTransform(offsetXform);

            GetFourWheelPhysicsActComp()->CreateVehicle(ourTransform, *chassis, wheels);
            helper->GetMainPhysicsObject()->SetTransformAsVisual(ourTransform);
         }

         if (!IsRemote())
         {

            // Create portals to get in and out of our vehicle
            GetGameActorProxy().GetGameManager()->CreateActor(
               *EntityActorRegistry::PORTAL_ACTOR_TYPE, mVehiclesPortal);
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(&GetGameActorProxy());
            portal->SetPortalName(GetName());
            portal->SetIsOpen(true);
            GetGameActorProxy().GetGameManager()->AddActor(*mVehiclesPortal.get(), false, true);
         }

         BaseClass::OnEnteredWorld();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActor::UpdateSoundEffects(float deltaTime)
      {
         ////////////////////////////////////////////////////////////
         // do sound here
         // if the vehicle is moving

         if (mSndVehicleIdleLoop == NULL)
         {
            return;
         }

         if (mSndVehicleIdleLoop->IsPlaying() == false)
         {
            mSndVehicleIdleLoop->Play();
         }

         if (GetMPH() > 1.0f)
         {
            float minpitchBend   = 1.0f;
            float maxpitchBend   = 1.5f;
            float pitchBend      = 1.0f;
            float dis            = 0.0f;
            float dif            = 0.0f;
            float tick           = 0.0f;

            if (GetMPH() < GetGearChangeLow())
            {
               if (mLastGearChange != FIRST_GEAR && mSndAcceleration != NULL)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = FIRST_GEAR;
               dis = GetGearChangeLow() - GetMPH();
               dif = GetGearChangeLow();
               tick = (maxpitchBend - minpitchBend) / dif;
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else if (GetMPH() < GetGearChangeMedium()  && mSndAcceleration != NULL)
            {
               if (mLastGearChange != SECOND_GEAR)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = SECOND_GEAR;
               dis = GetGearChangeMedium() - GetMPH();
               dif = GetGearChangeMedium() - GetGearChangeLow();
               tick = (maxpitchBend - minpitchBend) / dif;
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else if (GetMPH() < GetGearChangeHigh()  && mSndAcceleration != NULL)
            {
               if (mLastGearChange != THIRD_GEAR)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = THIRD_GEAR;
               dis = GetGearChangeHigh() - GetMPH();
               dif = GetGearChangeHigh() - GetGearChangeMedium();
               tick = (maxpitchBend - minpitchBend) / dif;
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else if (GetMPH() < GetFourWheelPhysicsActComp()->GetVehicleTopSpeed())
            {
               if (mLastGearChange != FOURTH_GEAR  && mSndAcceleration != NULL)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = FOURTH_GEAR;
               dis = GetFourWheelPhysicsActComp()->GetVehicleTopSpeed() - GetMPH();
               dif = GetFourWheelPhysicsActComp()->GetVehicleTopSpeed() - GetGearChangeHigh();
               tick = (maxpitchBend - minpitchBend) / dif;
               pitchBend = maxpitchBend  - (dis * tick) + mLastGearChange * .1;
            }
            else
            {
               pitchBend = maxpitchBend;
            }

            if (pitchBend > maxpitchBend)
            {
               pitchBend = maxpitchBend;
            }

            mSndVehicleIdleLoop->SetPitch(pitchBend);
         }
         else if (GetMPH() < -1 )
         {
            mSndVehicleIdleLoop->SetPitch(1.0f + (-GetMPH() * .005));
         }
         ////////////////////////////////////////////////////////////

         BaseClass::UpdateSoundEffects(deltaTime);
      }


     ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
      {
         osg::ref_ptr<osgSim::DOFTransform> steeringWheel;
         if (insideVehicle)
         {
            std::vector<dtDAL::ActorProxy*> toFillin;
            GetGameActorProxy().GetGameManager()->FindActorsByType(*EntityActorRegistry::INTERIOR_ACTOR_TYPE.get() , toFillin);
            if (toFillin.size())
            {
               InteriorActor* ourInterior = dynamic_cast<InteriorActor*>(toFillin[0]->GetActor());
               if (ourInterior != NULL)
               {
                  steeringWheel = ourInterior->GetSteeringWheelDOF("dof_steering_wheel");
                  if (steeringWheel != NULL)
                  {
                     osg::Vec3 HPR = steeringWheel->getCurrentHPR();
                     HPR[0] =  1.6f * osg::PI * mCurrentSteeringAngleNormalized;
                     steeringWheel->setCurrentHPR(HPR);
                  }
               }
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActor::ResetVehicle()
      {
         BaseClass::ResetVehicle();

         SetCurrentSteeringAngleNormalized(0.0f);

         if (mSndIgnition != NULL)
         {
            if (!mSndIgnition->IsPlaying())
            {
               mSndIgnition->Play();
            }
         }

         if (mSndBrake != NULL)
         {
            if (mSndBrake->IsPlaying())
            {
               mSndBrake->Stop();
            }
         }

         if (mSndVehicleIdleLoop != NULL)
         {
            mSndVehicleIdleLoop->SetPitch(1.0f);
            if (!mSndVehicleIdleLoop->IsPlaying())
            {
               mSndVehicleIdleLoop->Play();
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
      {
         BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);
         dtCore::Keyboard *keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
         if (keyboard == NULL)
         {
            return;
         }

         dtPhysics::PhysicsObject* po = GetPhysicsActComp()->GetMainPhysicsObject();
         if (po == NULL)
         {
            // We have no physics so return.  I assume this is because of an error in init, which
            // should have been reported, so no need to spam.
            return;
         }

         GetFourWheelPhysicsActComp()->CalcMPH();
         float curMPH = GetFourWheelPhysicsActComp()->GetMPH();

         float accelFactor = 1.0f/(float(mLastGearChange) * 0.6f + 0.4f);
         float accel = 0.0f;
         float brake = 0.0f;
         if (! IsMobilityDisabled() && GetHasDriver())
         {
            if (keyboard->GetKeyState('w') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
            {
               accel = accelFactor;
               mStopMode = false;
               if (curMPH > 0.0f)
               {
                  mCruiseMode = true;
                  mCruiseSpeed = curMPH;
               }
            }
            else if (keyboard->GetKeyState('s') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
            {
               accel = -accelFactor;
               mStopMode = false;
               mCruiseMode = false;
            }
            else if (mCruiseMode) 
            {

               // Just a way to shut off cruise control if we end up too slow (or hit something)
               if (mCruiseSpeed < 5.0f || curMPH < 5.0f)
               {
                  mCruiseMode = false;
               }
               else
               {
                  // To keep the accel from going on/off, we use a VERY simple approximation. 
                  // Essentially, we try to keep the accel between 1.0 and 0.0, centered around
                  // the target cruisespeed (+/- 1% speed). It means we will rarely be exactly 
                  // at our desired speed, but our vel won't keep changing. It settles pretty quickly.
                  float minTargetSpeed = mCruiseSpeed * 0.99f;
                  float maxTargetSpeed = mCruiseSpeed * 1.01f;                  
                  float cruiseAccelModifier = (maxTargetSpeed - curMPH) / (maxTargetSpeed - minTargetSpeed);                     
                  dtUtil::Clamp(cruiseAccelModifier, 0.0001f, 1.00f);
                  accel = accelFactor * cruiseAccelModifier;
               }
            }

            // If you hold the brake and the accelerator at the same time, it will make sure
            // you don't go into cruise mode, which it should not do.
            if (mStopMode || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               brake = 1.0f;
               mCruiseMode = false;
               if (curMPH < 0.1f)
               {
                  mStopMode = true;
               }
            }

            float updateCountInSecs = float(mNumUpdatesUntilFullSteeringAngle) / 60.00f;
            float angleChange = deltaTime / updateCountInSecs;
            float recoverAngleChange = 2.0f * angleChange;

            bool turnLeft = keyboard->GetKeyState('a') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left);
            bool turnRight = keyboard->GetKeyState('d') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right);

            // If you are turning the wheel or letting the wheel turn toward centered, then turn at double speed
            if (!turnRight && mCurrentSteeringAngleNormalized < -recoverAngleChange)
            {
               mCurrentSteeringAngleNormalized += recoverAngleChange;
            }
            else if (!turnLeft && mCurrentSteeringAngleNormalized > recoverAngleChange)
            {
               mCurrentSteeringAngleNormalized -= recoverAngleChange;
            }
            else if (turnLeft)
            {
               mCurrentSteeringAngleNormalized += angleChange;
            }
            else if (turnRight)
            {
               mCurrentSteeringAngleNormalized -= angleChange;
            }
            else
            {
               mCurrentSteeringAngleNormalized = 0.0f;
            }

            dtUtil::Clamp(mCurrentSteeringAngleNormalized, -1.0f, 1.0f);

            if (keyboard->GetKeyState('r') || keyboard->GetKeyState('R'))
            {
               dtCore::Transform xform;
               po->GetTransform(xform);
               osg::Vec3 trans;
               trans.z() += GetOSGNode()->getBound().radius() * 1.2;
               xform.GetTranslation(trans);
               xform.MakeIdentity();
               xform.SetTranslation(trans);
               po->SetTransform(xform);

               mStopMode = true;
               mCruiseMode = false;
            }

         }
         else
         {
            accel = 0.0f;
            brake = 1.0f;
            mStopMode = true;
            mCruiseMode = false;
            mCurrentSteeringAngleNormalized = 0.0f;
         }
         GetFourWheelPhysicsActComp()->Control(accel, mCurrentSteeringAngleNormalized, brake);

         osg::Vec3 dragVec = GetFourWheelPhysicsActComp()->ComputeAeroDynDrag(po->GetLinearVelocity());
         po->AddForce(dragVec);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActor::RepositionVehicle(float deltaTime)
      {
         BasePhysicsVehicleActor::RepositionVehicle(deltaTime);
         //GetFourWheelPhysicsActComp()->RepositionVehicle(deltaTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float FourWheelVehicleActor::GetMPH() const
      {
         float result = 0.0f;
         if (IsRemote())
         {
            static const float METERSPS_TO_MILESPH = 2.236936291;
            result = GetComponent<dtGame::DeadReckoningHelper>()->GetLastKnownVelocity().length() * METERSPS_TO_MILESPH;
         }
         else
         {
            result = dtUtil::Abs(GetFourWheelPhysicsActComp()->GetMPH());
         }
         return result;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float FourWheelVehicleActor::GetBrakeTorque()
      {
         // TODO this is wrong. it should return the current brake torque
         return GetFourWheelPhysicsActComp()->GetMaxBrakeTorque();
      }

      ////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActor::PostPhysicsUpdate()
      {
         BaseClass::PostPhysicsUpdate();
         GetFourWheelPhysicsActComp()->UpdateWheelTransforms();
      }

      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////
      FourWheelVehicleActorProxy::FourWheelVehicleActorProxy()
      {
         SetClassName("FourWheeledVehicleActor");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActorProxy::BuildPropertyMap()
      {
         const dtUtil::RefString VEH_GROUP   = "Vehicle";
         const dtUtil::RefString SOUND_GROUP = "Vehicle Sound";

         BasePhysicsVehicleActorProxy::BuildPropertyMap();

         FourWheelVehicleActor* actor = NULL;
         GetActor(actor);

         typedef dtDAL::PropertyRegHelper<FourWheelVehicleActorProxy&, FourWheelVehicleActor> PropRegType;
         PropRegType propRegHelper(*this, actor, VEH_GROUP);
         PropRegType propRegHelperSound(*this, actor, SOUND_GROUP);

         DT_REGISTER_PROPERTY_WITH_LABEL(NumUpdatesUntilFullSteeringAngle, "Num updates until full steering angle",
                  "Number of update cycles to go from straight to full turn angle when using the keyboard.",
                  PropRegType, propRegHelper);

         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(dtDAL::DataType::STATIC_MESH, VehicleInteriorModel,
                  "VEHICLE_INSIDE_MODEL", "Vehicle Interior Model",
                  "What is the resource of the interior model.", PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(SoundBrakeSquealSpeed,
                  "SOUND_BRAKE_SQUEAL_AMOUNT", "Sound brake squeal speed",
                  "How many mph does the car have to go to squeal used with BRAKE_STOP_NOW_BRAKE_TIME",
                  PropRegType, propRegHelperSound);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(GearChangeLow,
                  "SOUND_GEAR_CHANGE_LOW", "Low Gear change MPH",
                  "MPH When Gear Changes from 1st to 2nd",
                  PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(GearChangeMedium,
                  "SOUND_GEAR_CHANGE_MEDIUM", "Medium Gear change MPH",
                  "MPH When Gear Changes from 2nd to 3rd",
                  PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(GearChangeHigh,
                  "SOUND_GEAR_CHANGE_HIGH", "High Gear change MPH",
                  "MPH When Gear Changes from 3rd to 4th",
                  PropRegType, propRegHelper);


         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(dtDAL::DataType::SOUND, SoundEffectIgnition,
                  "SOUND_EFFECT_IGNITION", "Sound FX Ignition",
                  "Sound resource for the engine ignition sound.", PropRegType, propRegHelperSound);

         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(dtDAL::DataType::SOUND, SoundEffectIdleLoop,
                  "SOUND_EFFECT_VEHICLE_LOOP", "Sound FX Idle",
                  "Sound resource for the engine idle sound.", PropRegType, propRegHelperSound);

         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(dtDAL::DataType::SOUND, SoundEffectBrake,
                  "SOUND_EFFECT_BRAKE", "Sound FX Brake Squeal",
                  "Sound resource for the brake squeal sound.", PropRegType, propRegHelperSound);

         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(dtDAL::DataType::SOUND, SoundEffectAcceleration,
                  "SOUND_EFFECT_ACCELERATION", "Sound FX Acceleration",
                  "Sound resource for the acceleration sound.", PropRegType, propRegHelperSound);

         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(dtDAL::DataType::SOUND, SoundEffectCollisionHit,
                  "SOUND_EFFECT_COLLISION_HIT", "Sound FX Collision Hit",
                  "Sound resource for the collision hit sound.", PropRegType, propRegHelperSound);

      }

      ///////////////////////////////////////////////////////////////////////////////////
      FourWheelVehicleActorProxy::~FourWheelVehicleActorProxy(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActorProxy::CreateActor()
      {
         SetActor(*new FourWheelVehicleActor(*this));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActorProxy::OnEnteredWorld()
      {
         RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

         BasePhysicsVehicleActorProxy::OnEnteredWorld();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void FourWheelVehicleActorProxy::BuildActorComponents()
      {
         if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
         {
            AddComponent(*new SimCore::FourWheelVehiclePhysicsActComp());
         }

         BaseClass::BuildActorComponents();

         dtGame::DRPublishingActComp* drPublishingActComp = NULL;
         GetComponent(drPublishingActComp);
         if (drPublishingActComp == NULL)
         {
            LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
            return;
         }
         drPublishingActComp->SetMaxUpdateSendRate(3.0f);
         drPublishingActComp->SetMaxTranslationError(0.002f);
         drPublishingActComp->SetMaxRotationError(0.5f);
      }

   } // namespace
}// namespace
