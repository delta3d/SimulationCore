/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2014, Caper Holdings LLC
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
* Bradley Anderegg
*/


#include <Actors/SurfaceVesselActor.h>
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
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/ActComps/AbstractWheeledVehicleInputActComp.h>
#include <SimCore/ActComps/KeyboardWheeledVehicleInputActComp.h>
#include <SimCore/ActComps/WeaponInventoryActComp.h>
#include <dtGame/drpublishingactcomp.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messagetype.h>
#include <dtGame/message.h>
#include <dtGame/invokable.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/librarymanager.h>

namespace NetDemo
{

   ////////////////////////////////////////////////////////////////////////
   SurfaceVessel::SurfaceVessel(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
   : BaseClass(proxy)
   {
      SetEntityType("SurfaceVessel"); // Used for HLA mapping mostly
      SetMunitionDamageTableName("VehicleDamageTable"); // Used for Munitions Damage.
   }

   ////////////////////////////////////////////////////////////////////////
   SurfaceVessel::~SurfaceVessel()
   {
   }

   ////////////////////////////////////////////////////////////////////////
   void SurfaceVessel::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void SurfaceVessel::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);
   }

   ////////////////////////////////////////////////////////////////////////
   ///////////////// PROXY ////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////
   SurfaceVesselActor::SurfaceVesselActor()
   {

   }

   ////////////////////////////////////////////////////////////////////////
   SurfaceVesselActor::~SurfaceVesselActor()
   {

   }

   ////////////////////////////////////////////////////////////////////////
   void SurfaceVesselActor::CreateDrawable()
   {
      SurfaceVessel* pEntity = new SurfaceVessel(*this);
      SetDrawable(*pEntity);
   }

   ////////////////////////////////////////////////////////////////////////
   void SurfaceVesselActor::BuildPropertyMap()
   {
      BaseClass::BuildPropertyMap();

      SurfaceVessel* pEntity = NULL;
      GetDrawable(pEntity);

      std::string group("Surface Vehicle");
      //pEntity->RegisterProperties(*this, group);
   }

   ////////////////////////////////////////////////////////////////////////
   void SurfaceVesselActor::BuildInvokables()
   {
      BaseClass::BuildInvokables();

      SurfaceVessel* pEntity = NULL;
      GetDrawable(pEntity);

   }

   ////////////////////////////////////////////////////////////////////////
   void SurfaceVesselActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      SurfaceVessel* propelledActor = NULL;
      GetDrawable(propelledActor);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void SurfaceVesselActor::BuildActorComponents()
   {

      dtCore::RefPtr<dtCore::BaseActorObject> comp = dtCore::LibraryManager::GetInstance().CreateActor("ActorComponents", "SurfaceVessel");
      if(comp.valid())
      {
         AddComponent(*dynamic_cast<dtGame::ActorComponent*>(comp.get()));
      }

      if (!HasComponent(SimCore::ActComps::AbstractWheeledVehicleInputActComp::TYPE))
      {
         AddComponent(*new SimCore::ActComps::KeyboardWheeledVehicleInputActComp());
      }

      if (!HasComponent(SimCore::ActComps::WeaponInventoryActComp::TYPE))
      {
         AddComponent(*new SimCore::ActComps::WeaponInventoryActComp());
      }


      if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
      {
         dtCore::RefPtr<dtPhysics::PhysicsActComp> physAC = new dtPhysics::PhysicsActComp();

         dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("Body");
         physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::BOX);
         physicsObject->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
         //physicsObject->SetCollisionGroup(SimCore::CollisionGroup::GROUP_VEHICLE_WATER);
         //physicsObject->SetMass(500000.0f);
         //physicsObject->SetExtents(osg::Vec3(300.0f, 50.0f, 15.0f));
         physAC->AddPhysicsObject(*physicsObject);

         AddComponent(*physAC);
      }

      BaseClass::BuildActorComponents();

   }
}

