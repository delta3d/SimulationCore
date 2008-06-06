/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008, Alion Science and Technology
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
 * David Guthrie
 */
#include "SpheroInputComponent.h"
#include <dtGame/messagetype.h>
#include <dtABC/application.h>
#include <osgGA/GUIEventAdapter>

#include <dtActors/engineactorregistry.h>
#include <dtActors/playerstartactorproxy.h>
#include <dtActors/gamemeshactor.h>

#include <dtPhysics/physicsmaterialactor.h>
#include <dtPhysics/physicscomponent.h>

namespace Sphero
{
   //////////////////////////////////////////////////////////////
   SpheroInputComponent::SpheroInputComponent(const std::string& name):
      BaseClass(name)
   {
      
   }

   //////////////////////////////////////////////////////////////
   SpheroInputComponent::~SpheroInputComponent()
   {
      
   }

   //////////////////////////////////////////////////////////////
   void  SpheroInputComponent::ProcessMessage(const dtGame::Message& message)
   {
      BaseClass::ProcessMessage(message);

      if (message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
         mMotionModel->SetTarget(GetGameManager()->GetApplication().GetCamera());
         dtActors::PlayerStartActorProxy* startPosProxy = NULL;
         GetGameManager()->FindActorByType(*dtActors::EngineActorRegistry::PLAYER_START_ACTOR_TYPE, startPosProxy);
         if (startPosProxy != NULL)
         {
            dtCore::Transformable* actor = NULL;
            startPosProxy->GetActor(actor);
            dtCore::Transform xform;
            actor->GetTransform(xform);
            GetGameManager()->GetApplication().GetCamera()->SetTransform(xform);
         }

         SetupMaterialsAndTerrain();

      }
   }

   //////////////////////////////////////////////////////////////
   void SpheroInputComponent::SetupMaterialsAndTerrain()
   {
      dtPhysics::MaterialActorProxy* terrainMaterial = NULL;
      static const dtUtil::RefString TERRAIN_MATERIAL_NAME("Terrain Material");
      GetGameManager()->FindActorByName(TERRAIN_MATERIAL_NAME, terrainMaterial);

      dtGame::GameActorProxy* terrain = NULL;
      static const dtUtil::RefString TERRAIN_NAME("Terrain");
      GetGameManager()->FindActorByName(TERRAIN_NAME, terrain);
      if (terrain == NULL)
      {
         LOG_ERROR("No Terrain was found, physics can't be loaded.");
         return;
      }

      dtPhysics::PhysicsComponent* physicsComponent = NULL;
      GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComponent);
      if (physicsComponent == NULL)
      {
         LOG_ERROR("No Physics Component was found.");
         return;
      }

      dtCore::RefPtr<dtPhysics::PhysicsHelper> helper = new dtPhysics::PhysicsHelper(*terrain);
      helper->SetMaterialActor(terrainMaterial);
      dtCore::RefPtr<dtPhysics::PhysicsObject> terrainObject = new dtPhysics::PhysicsObject();
      helper->AddPhysicsObject(*terrainObject);
      terrainObject->SetPrimitiveType(dtPhysics::PhysicsObject::PhysicsPrimitiveType::TERRAIN_MESH);
      terrainObject->SetMass(1e12f);
      terrainObject->SetMechanicsEnum(dtPhysics::PhysicsObject::MechanicsEnum::STATIC);
      terrainObject->CreateFromProperties(terrain->GetActor()->GetOSGNode());

      physicsComponent->RegisterHelper(*helper);

      helper->SetPostPhysicsCallback(dtUtil::MakeFunctor(&SpheroInputComponent::UpdateHelpers, this));
      helper->SetFlags(dtPhysics::PhysicsHelper::FLAGS_POST_UPDATE);
   }

   //////////////////////////////////////////////////////////////
   void SpheroInputComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();
      mMotionModel = new dtCore::FlyMotionModel(GetGameManager()->GetApplication().GetKeyboard(),
               GetGameManager()->GetApplication().GetMouse(), false, true, true);
   }

   //////////////////////////////////////////////////////////////
   void SpheroInputComponent::OnRemovedFromGM()
   {
      BaseClass::OnRemovedFromGM();
      mMotionModel = NULL;
   }

   //////////////////////////////////////////////////////////////
   bool SpheroInputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)
   {
      bool keyUsed = true;
      switch(key)
      {
         case '\n':
         case '\r':
         case 'p':
         {
            FireSomething();
            break;
         }
         default:
            keyUsed = false;
      }
      return keyUsed;
   }

   void SpheroInputComponent::UpdateHelpers()
   {
      HelperList::iterator i, iend;
      i = mHelpers.begin();
      iend = mHelpers.end();
      for (; i != iend; ++i)
      {
         dtPhysics::PhysicsHelper& helper = **i;
         dtPhysics::PhysicsObject* phyObj = helper.GetPhysicsObject(helper.GetGameActorProxy()->GetName());
         if (phyObj != NULL)
         {
            dtPhysics::TransformType xform;
            phyObj->GetTransform(xform);
            helper.GetGameActorProxy()->GetGameActor().SetTransform(xform);
         }
      }
   }

   void SpheroInputComponent::FireSomething()
   {
      dtPhysics::MaterialActorProxy* projectileMaterial = NULL;
      static const dtUtil::RefString PROJECTILE_MATERIAL_NAME("Projectile Material");
      GetGameManager()->FindActorByName(PROJECTILE_MATERIAL_NAME, projectileMaterial);

      if (projectileMaterial == NULL)
      {
         LOG_ERROR("Can't create a projectile, the material is NULL");
         return;
      }

      dtActors::GameMeshActorProxy* projectilePrototype = NULL;
      static const dtUtil::RefString PROJECTILE_CRATE_NAME("Crate");
      GetGameManager()->FindPrototypeByName(PROJECTILE_CRATE_NAME, projectilePrototype);

      if (projectilePrototype == NULL)
      {
         LOG_ERROR("Can't create a projectile, the prototype NULL");
         return;
      }

      dtCore::RefPtr<dtActors::GameMeshActorProxy> projectile = NULL;
      GetGameManager()->CreateActorFromPrototype(projectilePrototype->GetId(), projectile);

      projectile->SetName("Silly Crate");

      dtPhysics::PhysicsComponent* physicsComponent = NULL;
      GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComponent);
      if (physicsComponent == NULL)
      {
         LOG_ERROR("No Physics Component was found.");
         return;
      }

      dtCore::RefPtr<dtPhysics::PhysicsHelper> helper = new dtPhysics::PhysicsHelper(*projectile);
      mHelpers.push_back(helper);

      helper->SetMaterialActor(projectileMaterial);
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject();
      helper->AddPhysicsObject(*physicsObject);
      physicsObject->SetName(projectile->GetName());
      physicsObject->SetPrimitiveType(dtPhysics::PhysicsObject::PhysicsPrimitiveType::CUBE);
      physicsObject->SetMass(3.0f);
      physicsObject->SetMechanicsEnum(dtPhysics::PhysicsObject::MechanicsEnum::DYNAMIC);
      physicsObject->SetExtents(osg::Vec3(1.0f, 1.0f, 2.0f));
      physicsObject->CreateFromProperties(NULL);
      physicsObject->SetActive(true);
      physicsObject->GetPhysicsObjectImpl()->ApplyImpulse(dtPhysics::VectorType(0.0, 20.0, 0.0));

      dtCore::Transform xform;
      GetGameManager()->GetApplication().GetCamera()->GetTransform(xform);
      physicsObject->SetTransform(xform);


      GetGameManager()->AddActor(*projectile, false, true);

      physicsComponent->RegisterHelper(*helper);
   }

}
