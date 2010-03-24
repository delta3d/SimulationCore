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
#include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaRaycastReport.h>
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
#include <dtGame/messagetype.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/PortalActor.h>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////////////////////////////////
      NxAgeiaFourWheelVehicleActor ::NxAgeiaFourWheelVehicleActor(BasePhysicsVehicleActorProxy &proxy)
         : BasePhysicsVehicleActor(proxy)
      , mLastGearChange(FIRST_GEAR)
      , SOUND_BRAKE_SQUEAL_AMOUNT(0.0f)
      , SOUND_GEAR_CHANGE_LOW(0.0f)
      , SOUND_GEAR_CHANGE_MEDIUM(0.0f)
      , SOUND_GEAR_CHANGE_HIGH(0.0f)
      {
         SetMaxUpdateSendRate(3.0f);
         dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper *helper =
            new dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper(proxy);
         helper->SetBaseInterfaceClass(this);
         SetPhysicsHelper(helper);
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
         if(SOUND_EFFECT_IGNITION != "")
         {
            mSndIgnition       = dtAudio::AudioManager::GetInstance().NewSound();
            mSndIgnition->LoadFile(SOUND_EFFECT_IGNITION.c_str());
            AddChild(mSndIgnition.get());
            dtCore::Transform xform;
            mSndIgnition->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_VEHICLE_LOOP != "")
         {
            mSndVehicleIdleLoop= dtAudio::AudioManager::GetInstance().NewSound();
            mSndVehicleIdleLoop->LoadFile(SOUND_EFFECT_VEHICLE_LOOP.c_str());
            mSndVehicleIdleLoop->SetLooping(true);
            mSndVehicleIdleLoop->SetMaxDistance(35.0f);
            AddChild(mSndVehicleIdleLoop.get());
            dtCore::Transform xform;
            mSndVehicleIdleLoop->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_BRAKE != "")
         {
            mSndBrake          = dtAudio::AudioManager::GetInstance().NewSound();
            mSndBrake->LoadFile(SOUND_EFFECT_BRAKE.c_str());
            mSndBrake->SetListenerRelative(true);
            mSndBrake->SetMaxDistance(160.0f);
            AddChild(mSndBrake.get());
            dtCore::Transform xform;
            mSndBrake->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         if(SOUND_EFFECT_ACCELERATION != "")
         {
            mSndAcceleration          = dtAudio::AudioManager::GetInstance().NewSound();
            mSndAcceleration->LoadFile(SOUND_EFFECT_ACCELERATION.c_str());
            mSndAcceleration->SetListenerRelative(false);
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

         GetFourWheelPhysicsHelper()->CreateVehicle(ourTransform, GetNodeCollector()->GetDOFTransform("dof_chassis"), Wheel, !IsRemote());

         if(!IsRemote())
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

         BasePhysicsVehicleActor::OnEnteredWorld();
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
            else if(GetMPH() < GetFourWheelPhysicsHelper()->GetVehicleMaxMPH())
            {
               if(mLastGearChange != FOURTH_GEAR  && mSndAcceleration != NULL)
               {
                  mSndAcceleration->Play();
               }
               mLastGearChange = FOURTH_GEAR;
               dis = GetFourWheelPhysicsHelper()->GetVehicleMaxMPH() - GetMPH();
               dif = GetFourWheelPhysicsHelper()->GetVehicleMaxMPH() - GetSound_gear_change_high();
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

         BasePhysicsVehicleActor::UpdateSoundEffects(deltaTime);
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
                     HPR[0] = GetFourWheelPhysicsHelper()->GetWheelShape(1)->getSteerAngle() * 4;
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
               HPR[0] = GetFourWheelPhysicsHelper()->GetWheelShape(i)->getSteerAngle();
               if(i % 2)
               {
                  HPR[1] = GetFourWheelPhysicsHelper()->GetAxleRotationOne();

               }
               else
               {
                  HPR[1] = GetFourWheelPhysicsHelper()->GetAxleRotationTwo();
               }
               HPR[2] = 0.0f;
               Wheel[i]->setCurrentHPR(HPR);
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::ResetVehicle()
      {
         BasePhysicsVehicleActor::ResetVehicle();

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
               GetFourWheelPhysicsHelper()->ApplyAccel(GetMPH());
            }
            else if (keyboard->GetKeyState('s') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
            {
               GetFourWheelPhysicsHelper()->ApplyHandBrake(GetMPH());
            }
            else if (!keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               accelOrBrakePressedThisFrame = false;
            }

            if(keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               GetFourWheelPhysicsHelper()->ApplyBrake(deltaTime);
            }

            if (keyboard->GetKeyState('a') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
            {
               GetFourWheelPhysicsHelper()->SteerLeft(deltaTime);
            }
            else if(keyboard->GetKeyState('d') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
            {
               GetFourWheelPhysicsHelper()->SteerRight(deltaTime);
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
            GetFourWheelPhysicsHelper()->ApplyBrake(deltaTime);
         }
         GetFourWheelPhysicsHelper()->UpdateVehicle(deltaTime, accelOrBrakePressedThisFrame, steeredThisFrame, GetMPH());
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActor::RepositionVehicle(float deltaTime)
      {
         // Note - this should be refactored. There should be a base physics vehicle HELPER.
         // See nxageiaFourWheelActor::RepositionVehicle() for more info.

         BasePhysicsVehicleActor::RepositionVehicle(deltaTime);
         GetFourWheelPhysicsHelper()->RepositionVehicle(deltaTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float NxAgeiaFourWheelVehicleActor::GetBrakeTorque()
      {
         return GetFourWheelPhysicsHelper()->GetBrakeTorque();
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

         BasePhysicsVehicleActorProxy::BuildPropertyMap();

         NxAgeiaFourWheelVehicleActor  &actor = static_cast<NxAgeiaFourWheelVehicleActor &>(GetGameActor());

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

      }

      ///////////////////////////////////////////////////////////////////////////////////
      NxAgeiaFourWheelVehicleActorProxy::~NxAgeiaFourWheelVehicleActorProxy(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActorProxy::CreateActor()
      {
         SetActor(*new NxAgeiaFourWheelVehicleActor(*this));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NxAgeiaFourWheelVehicleActorProxy::OnEnteredWorld()
      {
         RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

         BasePhysicsVehicleActorProxy::OnEnteredWorld();
      }

   } // namespace
}// namespace
#endif
