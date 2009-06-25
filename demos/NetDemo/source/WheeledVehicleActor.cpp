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
 * David Guthrie
 */

#include <WheeledVehicleActor.h>
#include <dtUtil/nodecollector.h>

#include <dtABC/application.h>
#include <dtCore/keyboard.h>

#include <osgSim/DOFTransform>

namespace NetDemo
{

   ////////////////////////////////////////////////////////////////////////
   WheeledVehicleActor::WheeledVehicleActor(SimCore::Actors::PlatformActorProxy& proxy)
   : BaseClass(proxy)
   , mHelper(new SimCore::FourWheelVehiclePhysicsHelper(proxy))
   {
      SetPhysicsHelper(mHelper.get());
      SetMunitionDamageTableName("StandardDamageTable");
   }

   ////////////////////////////////////////////////////////////////////////
   WheeledVehicleActor::~WheeledVehicleActor()
   {
   }

   ////////////////////////////////////////////////////////////////////////
   void WheeledVehicleActor::OnEnteredWorld()
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
   void WheeledVehicleActor::PostPhysicsUpdate()
   {
      BaseClass::PostPhysicsUpdate();
      mHelper->UpdateWheelTransforms();
   }

   ////////////////////////////////////////////////////////////////////////
   void WheeledVehicleActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
   {
      BaseClass::OnTickLocal(tickMessage);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void WheeledVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);
      dtCore::Keyboard* keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
      if(keyboard == NULL)
         return;
      //float currentMPH = GetMPH(); // speed, not a velocity with direction

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
         }
         else if (keyboard->GetKeyState('d') || keyboard->GetKeyState('D') ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
         {
            steering = -1.0;
         }

         if (keyboard->GetKeyState('f') || keyboard->GetKeyState('F'))
         {
            dtPhysics::PhysicsObject* po = GetPhysicsHelper()->GetMainPhysicsObject();
            po->AddTorque(osg::Vec3(0.0, 8000.0, 0.0));
         }

         if (keyboard->GetKeyState('r') || keyboard->GetKeyState('R'))
         {
            dtPhysics::PhysicsObject* po = GetPhysicsHelper()->GetMainPhysicsObject();
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

      mHelper->Control(accelerator, steering, brakes);

   }


   ////////////////////////////////////////////////////////////////////////
   ///////////////// PROXY ////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////
   WheeledVehicleActorProxy::WheeledVehicleActorProxy()
   {

   }

   ////////////////////////////////////////////////////////////////////////
   WheeledVehicleActorProxy::~WheeledVehicleActorProxy()
   {

   }

   ////////////////////////////////////////////////////////////////////////
   void WheeledVehicleActorProxy::CreateActor()
   {
      WheeledVehicleActor* pEntity = new WheeledVehicleActor(*this);
      SetActor(*pEntity);
   }

   ////////////////////////////////////////////////////////////////////////
   void WheeledVehicleActorProxy::BuildPropertyMap()
   {
      BaseClass::BuildPropertyMap();

      WheeledVehicleActor* pEntity = NULL;
      GetActor(pEntity);

   }

   ////////////////////////////////////////////////////////////////////////
   void WheeledVehicleActorProxy::BuildInvokables()
   {
      BaseClass::BuildInvokables();
   }

}
