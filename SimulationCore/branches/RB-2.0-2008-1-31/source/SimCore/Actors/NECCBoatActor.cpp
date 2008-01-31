/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen
*/
#include <prefix/SimCorePrefix-src.h>
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
#include <dtGame/basemessages.h>

#include <osg/MatrixTransform>

#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PortalActor.h>

namespace SimCore
{
   namespace Actors
   {
      ///////////////////////////////////////////////////////////////////////////////////
      NECCBoatActor ::NECCBoatActor(PlatformActorProxy &proxy) 
         : Platform(proxy)
      , mVehicleMPH(0.0f)
      , mVehicleMaxMPH(60.0f)
      , mVehicleMaxReverseMPH(-20.0f)
      , mHasDriver(false)
      , mNotifyFullUpdate(true)
      , mNotifyPartialUpdate(true)
      {
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
         
         if(!IsRemote() && mVehiclesPortal.valid() )
         {
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(NULL);
            GetGameActorProxy().GetGameManager()->DeleteActor(*mVehiclesPortal.get());
            mVehiclesPortal = NULL;
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

         dtCore::Transform ourTransform;
         GetTransform(ourTransform);

         // Create Physics model here!

         //GetPhysicsHelper()->SetLocalOffSet(osg::Vec3(0,0,1.1));
         //GetPhysicsHelper()->CreateVehicle(ourTransform, GetNodeCollector()->GetDOFTransform("dof_chassis"), Wheel, !IsRemote());
         //GetPhysicsHelper()->SetLocalOffSet(osg::Vec3(0,0,0));

         if(!IsRemote())
         {
            //GetPhysicsHelper()->TurnObjectsGravityOff("Default");

            GetGameActorProxy().GetGameManager()->CreateActor(
               *EntityActorRegistry::PORTAL_ACTOR_TYPE, mVehiclesPortal);
            Portal* portal = dynamic_cast<Portal*>(mVehiclesPortal->GetActor());
            portal->SetActorLink(&GetGameActorProxy());
            portal->SetPortalName(GetName());
            portal->SetIsOpen(true);
            GetGameActorProxy().GetGameManager()->AddActor(*mVehiclesPortal.get(), false, true);
         }

         GetPhysicsHelper()->SetObjectAsKinematic();
         GetPhysicsHelper()->SetAgeiaUserData(mPhysicsHelper.get());

         dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>
            (GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"))->RegisterAgeiaHelper(*mPhysicsHelper.get());

         if(IsRemote()) 
         {
            GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::ApplyForce( const osg::Vec3& force, const osg::Vec3& location )
      {
         GetPhysicsHelper()->GetPhysXObject()->addForce( NxVec3(force[0],force[1],force[2]) );
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::TickLocal(const dtGame::Message &tickMessage)
      {
         NxActor* physicsObject = GetPhysicsHelper()->GetPhysXObject();
         if(physicsObject == NULL)
         {
            // should probably throw an exception
            LOG_ERROR("BAD PHYSXOBJECT ON VEHICLE!");
            return;
         }

         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();

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
      void NECCBoatActor::UpdateSoundEffects(float deltaTime)
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
            
            pitchBend = minpitchBend + (mVehicleMPH / mVehicleMaxMPH * (maxpitchBend - minpitchBend));

            if(pitchBend > maxpitchBend)
               pitchBend = maxpitchBend;

            mSndVehicleIdleLoop->SetPitch(pitchBend);
         }
         else if(GetMPH() < -1 )
         {
            mSndVehicleIdleLoop->SetPitch(1.0f + (-GetMPH() * .005));
         }
         else
            mSndVehicleIdleLoop->SetPitch(1.0f);
         ////////////////////////////////////////////////////////////
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool NECCBoatActor::CompareVectors( const osg::Vec3& op1, const osg::Vec3& op2, float epsilon )
      {
         return fabs(op1.x() - op2.x()) < epsilon
            && fabs(op1.y() - op2.y()) < epsilon
            && fabs(op1.z() - op2.z()) < epsilon;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::UpdateDeadReckoning(float deltaTime)
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
         osg::Vec3 nxVecTemp;
         nxVecTemp.set(physxObj->getGlobalPosition().x, physxObj->getGlobalPosition().y, physxObj->getGlobalPosition().z + zoffset);

         // A full update may not be required. Prevent any further network updates.
         // Let the following code determine if this vehicle should be flagged
         // for a full actor update.
         mNotifyFullUpdate = false;
         mNotifyPartialUpdate = false;

         const osg::Vec3 &translationVec = GetDeadReckoningHelper().GetLastKnownTranslation();//GetCurrentDeadReckonedTranslation();
         const osg::Vec3 &orientationVec = GetDeadReckoningHelper().GetLastKnownRotation();//GetCurrentDeadReckonedRotation();

         bool changedTrans = CompareVectors(nxVecTemp, translationVec, amountChange);//!dtUtil::Equivalent<osg::Vec3, float>(nxVecTemp, translationVec, 3, amountChange);
         bool changedOrient = !dtUtil::Equivalent<osg::Vec3, float>(globalOrientation, orientationVec, 3, 3.0f);

         const osg::Vec3 &turnVec = GetDeadReckoningHelper().GetAngularVelocityVector();
         const osg::Vec3 &velocityVec = GetDeadReckoningHelper().GetVelocityVector();

         osg::Vec3 AngularVelocity(physxObj->getAngularVelocity().x, physxObj->getAngularVelocity().y, physxObj->getAngularVelocity().z);
         osg::Vec3 linearVelocity(physxObj->getLinearVelocity().x, physxObj->getLinearVelocity().y, physxObj->getLinearVelocity().z);

         float velocityDiff = (velocityVec - linearVelocity).length();
         bool velocityNearZero = linearVelocity.length() < 0.1;
         bool changedVelocity = velocityDiff > 0.2 || (velocityNearZero && velocityVec.length() > 0.1 );

         if( changedTrans || changedOrient || changedVelocity )
         {
            SetLastKnownTranslation(nxVecTemp);
            SetLastKnownRotation(globalOrientation);
            SetAngularVelocityVector(AngularVelocity);

            if( velocityNearZero )
            {
               linearVelocity.set(0.0,0.0,0.0);
            }
            SetVelocityVector(linearVelocity);

            // do not send the update message here but rather flag this vehicle
            // to send the update via the base class though ShouldForceUpdate.
            mNotifyFullUpdate = true; 
         }
         else if( GetArticulationHelper() != NULL && GetArticulationHelper()->IsDirty() )
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
      void NECCBoatActor::TickRemote(const dtGame::Message &tickMessage)
      {
         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         UpdateSoundEffects(ElapsedTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::AgeiaPrePhysicsUpdate()
      {
         osg::Matrix rot = GetMatrixNode()->getMatrix();
         
         NxActor* toFillIn = GetPhysicsHelper()->GetPhysXObject();
         toFillIn->setGlobalPosition(NxVec3(rot.operator ()(3,0), rot.operator ()(3,1), rot.operator ()(3,2)));
         toFillIn->setGlobalOrientation(
            NxMat33( NxVec3(rot.operator ()(0,0), rot.operator ()(0,1), rot.operator ()(0,2)),
                     NxVec3(rot.operator ()(1,0), rot.operator ()(1,1), rot.operator ()(1,2)),
                     NxVec3(rot.operator ()(2,0), rot.operator ()(2,1), rot.operator ()(2,2))));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::AgeiaPostPhysicsUpdate(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& 
         contactReport, NxActor& ourSelf, NxActor& whatWeHit)
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
         // TODO Change physical location
         //GetPhysicsHelper()->ResetVehicle();
         
         if(mSndIgnition != NULL)
         {
            if(!mSndIgnition->IsPlaying())
               mSndIgnition->Play();
         }

         if(mSndVehicleIdleLoop != NULL)
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
         if(keyboard == NULL)
            return;

         bool steeredThisFrame = true;
         bool accelOrBrakePressedThisFrame = true;

         if( ! IsMobilityDisabled() && GetHasDriver() )
         {
            if (keyboard->GetKeyState('w') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
            {
               //GetPhysicsHelper()->ApplyAccel(GetMPH());
            }
            else if (keyboard->GetKeyState('s') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
            {
               //GetPhysicsHelper()->ApplyHandBrake(GetMPH());
            }
            else if (!keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               //accelOrBrakePressedThisFrame = false;
            }

            if(keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            {
               //GetPhysicsHelper()->ApplyBrake(deltaTime);
            }

            if (keyboard->GetKeyState('a') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
            {
               //GetPhysicsHelper()->SteerLeft(deltaTime);
            }
            else if(keyboard->GetKeyState('d') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
            {
               //GetPhysicsHelper()->SteerRight(deltaTime);
            }
            else
            {
               //steeredThisFrame = false;
            }
         }
         else
         {
            //steeredThisFrame = false;
            //accelOrBrakePressedThisFrame = true;
            //GetPhysicsHelper()->ApplyBrake(deltaTime);
         }
         //GetPhysicsHelper()->UpdateVehicle(deltaTime, accelOrBrakePressedThisFrame, steeredThisFrame, GetMPH());
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float NECCBoatActor::GetMPH()
      {
         return GetVelocityVector().length() * 2.236936291;
         //return GetPhysicsHelper()->GetMPH();
         return 0.0f;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActor::RepositionVehicle(float deltaTime)
      {
         //GetPhysicsHelper()->RepositionVehicle(deltaTime);
      }

      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////
      NECCBoatActorProxy::NECCBoatActorProxy()
      {
         SetClassName("NxAgeiaFourWheeledVehicleActor");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActorProxy::BuildPropertyMap()
      {
         const std::string& VEH_GROUP   = "Vehicle Property Values";
         const std::string& SOUND_GROUP = "Sound Property Values";

         PlatformActorProxy::BuildPropertyMap();

         NECCBoatActor  &actor = static_cast<NECCBoatActor &>(GetGameActor());
         std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
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
      }

      ///////////////////////////////////////////////////////////////////////////////////
      NECCBoatActorProxy::~NECCBoatActorProxy(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void NECCBoatActorProxy::CreateActor()
      {
         SetActor(*new NECCBoatActor(*this));

         BaseEntity* entityActor = dynamic_cast<BaseEntity*> (GetActor());
         if( entityActor != NULL )
         {
            entityActor->InitDeadReckoningHelper();
         }
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
