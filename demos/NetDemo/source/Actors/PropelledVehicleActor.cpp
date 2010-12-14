/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* David Guthrie
* Curtiss Murphy
*/


#include <Actors/PropelledVehicleActor.h>
#include <Components/GameLogicComponent.h>
#include <NetDemoUtils.h>
#include <NetDemoMessageTypes.h>

#include <dtABC/application.h>
#include <dtCore/keyboard.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>
#include <dtUtil/mathdefines.h>
#include <dtPhysics/bodywrapper.h>

//For debug only.
#include <iostream>

#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/ApplyShaderVisitor.h>
#include <dtGame/drpublishingactcomp.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messagetype.h>
#include <dtGame/message.h>
#include <dtGame/invokable.h>
#include <dtCore/shaderprogram.h>

namespace NetDemo
{

   ////////////////////////////////////////////////////////////////////////
   PropelledVehicleActor::PropelledVehicleActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
   : BaseClass(proxy)
     , mDRTestingAveragedError(0.0f)
   {
      SetEntityType("WheeledVehicle"); // Used for HLA mapping mostly
      SetMunitionDamageTableName("VehicleDamageTable"); // Used for Munitions Damage.
   }

   ////////////////////////////////////////////////////////////////////////
   PropelledVehicleActor::~PropelledVehicleActor()
   {
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      // TEMP:
      // Find the fuel line geometry and apply the pulse effect to it.
      dtCore::RefPtr<SimCore::ApplyShaderVisitor> visitor = new SimCore::ApplyShaderVisitor();
      visitor->AddNodeName("FuelLines-GEODE");
      visitor->SetShaderName("ColorPulseShader");
      visitor->SetShaderGroup("CustomizableVehicleShaderGroup");
      GetOSGNode()->accept(*visitor);

      // Add a particle system to see where the vehicle has been.
      mTrailParticles = new dtCore::ParticleSystem;
      mTrailParticles->LoadFile("Particles/SimpleSpotTrailGreen.osg", true);
      mTrailParticles->SetEnabled(true);
      AddChild(mTrailParticles.get());
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::OnTickRemoteTest(const dtGame::TickMessage& tickMessage)
   {

      // The following behavior is left here because it was used for the statistics published
      // with the Game Engine Gems 2 article, 'Believable Dead Reckoning for Networked Games'.
      // It computes the difference between the dead reckoned location and the real location
      // every frame and passes along the avg value to the debug values.

      // compute DR debug values on Remote message, but only if we are a local actor
      // Pull the real position for DR testing. The real loc is set by the physics at the end
      // of the previous frame.
      dtCore::Transform xform;
      GetTransform(xform);
      xform.GetTranslation(mDRTestingRealLocation);

      dtGame::DeadReckoningHelper* drHelper = NULL;
      GetComponent(drHelper);

      // Get the DR position. The DR pos is set in Tick Remote, so we pull it after that.
      osg::Vec3 testingDRLoc = drHelper->GetCurrentDeadReckonedTranslation();
      float testingDRSpeed = drHelper->GetLastKnownVelocity().length();

      // Compare the DR loc to the real loc.
      float difference = (mDRTestingRealLocation - testingDRLoc).length();
      mDRTestingAveragedError = mDRTestingAveragedError * 0.99f + difference * 0.01f;

      // Every so often, print the cumulative averaged delta and the vel
      static int counter = 0;
      counter ++;
      if (counter >= 60)
      {
         GameLogicComponent* comp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(GameLogicComponent::DEFAULT_NAME, comp);
         if (comp != NULL)
         {
            comp->GetDebugInfo().mDRAvgSpeed = testingDRSpeed;
            comp->GetDebugInfo().mDRAvgError = mDRTestingAveragedError;
            MessageUtils::SendSimpleMessage(NetDemo::MessageType::UI_DEBUGINFO_UPDATED,
               *GetGameActorProxy().GetGameManager());
         }
         //printf("  Avg DR Error[%6.4f m], Last DR Speed[%6.4f m/s], NumPubs[%5.4f].\r\n",
         //   mDRTestingAveragedError, testingDRSpeed, GetDRPublishingActComp()->GetMaxUpdateSendRate());
         counter = 0;
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);

//      dtCore::Keyboard* keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
//      if(keyboard == NULL)
//         return;
//      //float currentMPH = GetMPH(); // speed, not a velocity with direction
//
//      dtPhysics::PhysicsObject* po = GetPhysicsActComp()->GetMainPhysicsObject();
//      osg::Vec3 dragVec = mHelper->ComputeAeroDynDrag(po->GetLinearVelocity());
//      po->AddForce(dragVec);
//
//      float accelerator = 0.0;
//      float brakes = 0.0;
//
//      float steering = 0.0;
//
//      if (!IsMobilityDisabled())
//      {
//         // FORWARD OR BACKWARD
//         if (keyboard->GetKeyState('w') || (keyboard->GetKeyState('W')) ||
//               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
//         {
//            accelerator = 1.0;
//         }
//
//         if (keyboard->GetKeyState('s') || keyboard->GetKeyState('S') ||
//               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
//         {
//            accelerator = -1.0;
//         }
//
//         if (keyboard->GetKeyState(' '))
//         {
//            brakes = 1.0;
//         }
//
//         // LEFT OR RIGHT
//         if (keyboard->GetKeyState('a') || keyboard->GetKeyState('A') ||
//              keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
//         {
//
//            steering = 1.0;
//            osg::Vec3 torqueDirection = osg::Vec3(0.0, -1.0, 0.0);
//            //float torqueMagnitude = 1000;
//            //po->AddLocalTorque(torqueDirection * torqueMagnitude);
//            //po->AddLocalForce(osg::Vec3(0.0, 0.0, -1000.0));
//         }
//         else if (keyboard->GetKeyState('d') || keyboard->GetKeyState('D') ||
//               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
//         {
//
//            steering = -1.0;
//            osg::Vec3 torqueDirection = osg::Vec3(0.0, 1.0, 0.0);
//            //float torqueMagnitude = 1000;
//
//            //po->AddLocalTorque(torqueDirection * torqueMagnitude);
//            //po->AddLocalForce(osg::Vec3(0.0, 0.0, -1000.0));
//         }
//
//         if (keyboard->GetKeyState('f') || keyboard->GetKeyState('F'))
//         {
//            //dtPhysics::PhysicsObject* po = GetPhysicsActComp()->GetMainPhysicsObject();
//
//
//
//            //po->AddLocalForce(boostDirection * boostForce);
//         }
//         else
//         {
//         }
//
//         if (keyboard->GetKeyState('r') || keyboard->GetKeyState('R'))
//         {
//            dtCore::Transform xform;
//            po->GetTransform(xform);
//            osg::Vec3 trans;
//            trans.z() += GetOSGNode()->getBound().radius() * 1.2;
//            xform.GetTranslation(trans);
//            xform.MakeIdentity();
//            xform.SetTranslation(trans);
//            po->SetTransform(xform);
//         }
//
//      }
//
//      dtCore::Transform xform;
//      po->GetTransform(xform);
//      osg::Vec3 hpr;
//      xform.GetRotation(hpr);
//
//      osg::Vec3 angVel = po->GetAngularVelocity();
//
//
////      osg::Vec3 up;
////      xform.GetRow(2, up);
////
////      osg::Vec3 zup(0.0, 0.0, 1.0);
////
////      float mag = (up * zup);
////
////      // If you are not upside down, that is, we apply a stranger downward force as you get
////      // more and more rotated.
////      if (mag > 0.0)
////      {
////         float force = -(po->GetMass() * (1.0 - mag) * 3.3);
////         osg::Vec3 forceVec = up * force;
////         po->AddForce(forceVec);
////      }
//
//      //std::cout << "Angular Velocity: " << angVel << std::endl;
////      std::cout << "Linear Velocity: " << po->GetLinearVelocity() << std::endl;
////      std::cout << "Aero Drag: " << dragVec << std::endl;
//
//      float pitchTorque = -(5.0f * (hpr.y()));
//      pitchTorque -= angVel.x() * 30.0;
//      dtUtil::Clamp(pitchTorque, -1000.0f, 1000.0f);
//
//      //std::cout << "pitch Torque: " << pitchTorque << std::endl;
//
//      float rollTorque = -(5.0f * (hpr.z()));
//      rollTorque -= angVel.y() * 30.0;
//      dtUtil::Clamp(rollTorque, -1000.0f, 1000.0f);
//
//      //std::cout << "roll Torque: " << rollTorque << std::endl;
//
//      //po->AddLocalTorque(osg::Vec3(pitchTorque, rollTorque, 0.0));
//      mHelper->Control(accelerator, steering, brakes);

   }
   ////////////////////////////////////////////////////////////////////////
   ///////////////// PROXY ////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////
   PropelledVehicleActorProxy::PropelledVehicleActorProxy()
   {

   }

   ////////////////////////////////////////////////////////////////////////
   PropelledVehicleActorProxy::~PropelledVehicleActorProxy()
   {

   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActorProxy::CreateActor()
   {
      PropelledVehicleActor* pEntity = new PropelledVehicleActor(*this);
      SetActor(*pEntity);
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActorProxy::BuildPropertyMap()
   {
      BaseClass::BuildPropertyMap();

      PropelledVehicleActor* pEntity = NULL;
      GetActor(pEntity);

      std::string group("Propelled Vehicle");
      //pEntity->RegisterProperties(*this, group);
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActorProxy::BuildInvokables()
   {
      BaseClass::BuildInvokables();

      PropelledVehicleActor* pEntity = NULL;
      GetActor(pEntity);

      AddInvokable(*new dtGame::Invokable("TickRemoteTest",
               dtUtil::MakeFunctor(&PropelledVehicleActor::OnTickRemoteTest, pEntity)));
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActorProxy::OnEnteredWorld()
   {
      if (!IsRemote())
      {
         RegisterForMessages(dtGame::MessageType::TICK_REMOTE, "TickRemoteTest");
      }

      BaseClass::OnEnteredWorld();

      PropelledVehicleActor* propelledActor = NULL;
      GetActor(propelledActor);

      // Add a dynamic light to match the propulsion color
      ////TODO- take in user specified color
      SimCore::Components::RenderingSupportComponent* renderComp;
      GetGameManager()->GetComponentByName(
         SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);
      if(renderComp != NULL)
      {
      //   //Add a spot light
         SimCore::Components::RenderingSupportComponent::DynamicLight* sl =
            new SimCore::Components::RenderingSupportComponent::DynamicLight();

      //   //we should always see the light from our base... more user friendly in finding it
         sl->mRadius = 10.0f;
         sl->mIntensity = 1.5f;
         sl->mColor.set(0.05f, 0.5f, 0.1f);
         sl->mAttenuation.set(0.05f, 0.25f, 0.2f);
         sl->mTarget = propelledActor;
         sl->mAutoDeleteLightOnTargetNull = true;
         renderComp->AddDynamicLight(sl);
      }
   }
   ///////////////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActorProxy::BuildActorComponents()
   {
      BaseClass::BuildActorComponents();

      dtGame::DRPublishingActComp* drPublishingActComp = NULL;
      GetComponent(drPublishingActComp);
      if (drPublishingActComp == NULL)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(5.0f);
      drPublishingActComp->SetMaxTranslationError(0.0001f);
      drPublishingActComp->SetMaxRotationError(0.5f);
   }
}
