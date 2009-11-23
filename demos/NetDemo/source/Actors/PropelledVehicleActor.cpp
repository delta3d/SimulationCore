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
#include <dtUtil/nodecollector.h>

#include <dtABC/application.h>
#include <dtCore/keyboard.h>

#include <osgSim/DOFTransform>
#include <dtUtil/mathdefines.h>

#include <dtPhysics/bodywrapper.h>

//For debug only.
#include <iostream>

namespace NetDemo
{

   ////////////////////////////////////////////////////////////////////////
   PropelledVehicleActor::PropelledVehicleActor(SimCore::Actors::PlatformActorProxy& proxy)
   : BaseClass(proxy)
//   , mStartBoost(false)
//   , mStartBoostForce(6500.0f)
//   , mMaximumBoostPerSecond(1000.0f)
//   , mCurrentBoostTime(0.0f)
//   , mTimeToResetBoost(5.0f)
//   , mBoostResetTimer(0.0f)
   , mHelper(new SimCore::FourWheelVehiclePhysicsHelper(proxy))
   {
      SetPhysicsHelper(mHelper.get());
      SetMunitionDamageTableName("StandardDamageTable");
//      SetMaxTranslationError(0.02f);
//      SetMaxRotationError(1.0f);
      SetMaxUpdateSendRate(5.0f);
      SetPublishLinearVelocity(true);
      SetPublishAngularVelocity(true);
   }

   ////////////////////////////////////////////////////////////////////////
   PropelledVehicleActor::~PropelledVehicleActor()
   {
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
      dtCore::Transform xform;
      GetTransform(xform);
      osg::Group* body = GetNodeCollector()->GetDOFTransform("dof_chassis");
      if (body == NULL)
      {
         body = GetNodeCollector()->GetGroup("Body");
         if (body == NULL)
         {
            LOG_ERROR("Unable to find the 'Body' node");
         }
      }

      osgSim::DOFTransform* wheels[4];
      wheels[0] = GetNodeCollector()->GetDOFTransform("dof_wheel_lt_01");
      wheels[1] = GetNodeCollector()->GetDOFTransform("dof_wheel_rt_01");
      wheels[2] = GetNodeCollector()->GetDOFTransform("dof_wheel_lt_02");
      wheels[3] = GetNodeCollector()->GetDOFTransform("dof_wheel_rt_02");

      for (size_t i = 0 ; i < 4; ++i)
      {
         if (wheels[i] == NULL)
         {
            LOG_ERROR("One of the wheel DOFs is NULL, unable to create vehicle");
            return;
         }
      }

      osg::Matrix bodyOffset;
      bodyOffset.makeIdentity();
      mHelper->GetLocalMatrix(*body, bodyOffset);
      bodyOffset.setTrans(bodyOffset.getTrans() - mHelper->GetMainPhysicsObject()->GetOriginOffset());
      dtCore::Transform offsetXform;
      offsetXform.Set(bodyOffset);

      mHelper->GetMainPhysicsObject()->SetVisualToBodyTransform(offsetXform);

      mHelper->CreateVehicle(xform, *body, wheels);
      mHelper->GetMainPhysicsObject()->SetTransformAsVisual(xform);
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::PostPhysicsUpdate()
   {
      BaseClass::PostPhysicsUpdate();
      mHelper->UpdateWheelTransforms();
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
   {
      BaseClass::OnTickLocal(tickMessage);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);
      dtCore::Keyboard* keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
      if(keyboard == NULL)
         return;
      //float currentMPH = GetMPH(); // speed, not a velocity with direction

      dtPhysics::PhysicsObject* po = GetPhysicsHelper()->GetMainPhysicsObject();

      float accelerator = 0.0;
      float brakes = 0.0;

      float steering = 0.0;

      if (!IsMobilityDisabled())
      {
         // FORWARD OR BACKWARD
         if (keyboard->GetKeyState('w') || (keyboard->GetKeyState('W')) ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
         {
            accelerator = 1.0;
         }

         if (keyboard->GetKeyState('s') || keyboard->GetKeyState('S') ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
         {
            accelerator = -1.0;
         }

         if (keyboard->GetKeyState(' '))
         {
            brakes = 1.0;
         }

         // LEFT OR RIGHT
         if (keyboard->GetKeyState('a') || keyboard->GetKeyState('A') ||
              keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
         {

            steering = 1.0;
            osg::Vec3 torqueDirection = osg::Vec3(0.0, -1.0, 0.0);
            float torqueMagnitude = 1000;
            //po->AddLocalTorque(torqueDirection * torqueMagnitude);
            //po->AddLocalForce(osg::Vec3(0.0, 0.0, -1000.0));
         }
         else if (keyboard->GetKeyState('d') || keyboard->GetKeyState('D') ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
         {

            steering = -1.0;
            osg::Vec3 torqueDirection = osg::Vec3(0.0, 1.0, 0.0);
            float torqueMagnitude = 1000;

            //po->AddLocalTorque(torqueDirection * torqueMagnitude);
            //po->AddLocalForce(osg::Vec3(0.0, 0.0, -1000.0));
         }

         if (keyboard->GetKeyState('f') || keyboard->GetKeyState('F'))
         {
            //dtPhysics::PhysicsObject* po = GetPhysicsHelper()->GetMainPhysicsObject();



            //po->AddLocalForce(boostDirection * boostForce);
         }
         else
         {
         }

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
         }

      }

      dtCore::Transform xform;
      po->GetTransform(xform);
      osg::Vec3 hpr;
      xform.GetRotation(hpr);

      osg::Vec3 angVel = po->GetAngularVelocity();

      osg::Vec3 dragVec = mHelper->ComputeAeroDynDrag(po->GetLinearVelocity());
      po->AddForce(dragVec);


//      osg::Vec3 up;
//      xform.GetRow(2, up);
//
//      osg::Vec3 zup(0.0, 0.0, 1.0);
//
//      float mag = (up * zup);
//
//      // If you are not upside down, that is, we apply a stranger downward force as you get
//      // more and more rotated.
//      if (mag > 0.0)
//      {
//         float force = -(po->GetMass() * (1.0 - mag) * 3.3);
//         osg::Vec3 forceVec = up * force;
//         po->AddForce(forceVec);
//      }

      //std::cout << "Angular Velocity: " << angVel << std::endl;
//      std::cout << "Linear Velocity: " << po->GetLinearVelocity() << std::endl;
//      std::cout << "Aero Drag: " << dragVec << std::endl;

      float pitchTorque = -(5.0f * (hpr.y()));
      pitchTorque -= angVel.x() * 30.0;
      dtUtil::Clamp(pitchTorque, -1000.0f, 1000.0f);

      //std::cout << "pitch Torque: " << pitchTorque << std::endl;

      float rollTorque = -(5.0f * (hpr.z()));
      rollTorque -= angVel.y() * 30.0;
      dtUtil::Clamp(rollTorque, -1000.0f, 1000.0f);

      //std::cout << "roll Torque: " << rollTorque << std::endl;

      //po->AddLocalTorque(osg::Vec3(pitchTorque, rollTorque, 0.0));
      mHelper->Control(accelerator, steering, brakes);

   }

   void PropelledVehicleActor::RegisterProperties( dtDAL::PropertyContainer& pc, const std::string& group )
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, PropelledVehicleActor> RegHelperType;
      RegHelperType propReg(pc, this, group);

//      REGISTER_PROPERTY(StartBoostForce, "The initial force of the boost in newtons.", RegHelperType, propReg);
//      REGISTER_PROPERTY(MaximumBoostPerSecond, "The maximum amount of boost to be applied over time.", RegHelperType, propReg);
//      REGISTER_PROPERTY(TimeToResetBoost, "How long it takes the booster to recharge.", RegHelperType, propReg);

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
      pEntity->RegisterProperties(*this, group);
   }

   ////////////////////////////////////////////////////////////////////////
   void PropelledVehicleActorProxy::BuildInvokables()
   {
      BaseClass::BuildInvokables();
   }

}
