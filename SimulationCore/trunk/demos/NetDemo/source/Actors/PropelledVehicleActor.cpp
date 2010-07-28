/* -*-c++-*-
 * SimulationCore
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
 *
 * Bradley Anderegg
 */

#include <Actors/PropelledVehicleActor.h>

#include <dtABC/application.h>
#include <dtCore/keyboard.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>
#include <dtUtil/mathdefines.h>
#include <dtPhysics/bodywrapper.h>

//For debug only.
#include <iostream>

#include <SimCore/ApplyShaderVisitor.h>
#include <SimCore/Actors/DRPublishingActComp.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messagetype.h>
#include <dtGame/message.h>
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

   ///////////////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::BuildActorComponents()
   {
      BaseClass::BuildActorComponents();

      SimCore::Actors::DRPublishingActComp* drPublishingActComp = GetDRPublishingActComp();
      if (drPublishingActComp == NULL)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(5.0f);
      drPublishingActComp->SetMaxTranslationError(0.0001f);
      drPublishingActComp->SetMaxRotationError(0.5f);
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
      //mTrailParticles = new dtCore::ParticleSystem;
      //mTrailParticles->LoadFile("Particles/SimpleSpotTrailGreen.osg", true);
      //mTrailParticles->SetEnabled(true);
      //AddChild(mTrailParticles.get());
   }

   ///////////////////////////////////////////////////////////////////////////////////
   /*
   void PropelledVehicleActor::ProcessMessage(const dtGame::Message& message)
   {

      if (message.GetMessageType() == dtGame::MessageType::TICK_REMOTE)
      {
         // Pull the real position for DR testing. The real loc is set by the physics at the end 
         // of the previous frame.  
         dtCore::Transform xform;
         GetTransform(xform);
         xform.GetTranslation(mDRTestingRealLocation);

         // Get the DR position. The DR pos is set in Tick Remote, so we pull it after that.
         osg::Vec3 testingDRLoc = GetDeadReckoningHelper().GetCurrentDeadReckonedTranslation();
         float testingDRSpeed = GetDeadReckoningHelper().GetLastKnownVelocity().length();

         // Compare the DR loc to the real loc.
         float difference = (mDRTestingRealLocation - testingDRLoc).length();
         mDRTestingAveragedError = mDRTestingAveragedError * 0.99f + difference * 0.01f; 

         // Every so often, print the cumulative averaged delta and the vel
         static int counter = 0;
         counter ++;
         if (counter >= 30)
         {
            printf("  Avg DR Error[%6.4f m], Last DR Speed[%6.4f m/s].\r\n", 
               mDRTestingAveragedError, testingDRSpeed);
            counter = 0;
         }
      }

      if (IsRemote())
      {
         BaseClass::ProcessMessage(message);
      }
   }
*/
   ///////////////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);

//      dtCore::Keyboard* keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
//      if(keyboard == NULL)
//         return;
//      //float currentMPH = GetMPH(); // speed, not a velocity with direction
//
//      dtPhysics::PhysicsObject* po = GetPhysicsHelper()->GetMainPhysicsObject();
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
//            //dtPhysics::PhysicsObject* po = GetPhysicsHelper()->GetMainPhysicsObject();
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
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActorProxy::OnEnteredWorld()
   {
      //RegisterForMessages(dtGame::MessageType::TICK_REMOTE);

      BaseClass::OnEnteredWorld();
   }
}
