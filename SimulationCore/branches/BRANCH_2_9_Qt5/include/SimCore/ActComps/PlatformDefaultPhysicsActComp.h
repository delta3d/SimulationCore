/* -*-c++-*-
* Simulation Core
* Copyright 2011, Alion Science and Technology
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
* David Guthrie
*/

#include <dtPhysics/physicsactcomp.h>

#include <dtGame/gameactorproxy.h>
#include <SimCore/Actors/Platform.h>
#include <dtUtil/log.h>
#include <SimCore/CollisionGroupEnum.h>
#include <dtPhysics/physicsmaterialactor.h>

namespace SimCore
{
   namespace ActComps
   {
      static const dtUtil::RefString PLATFORM_BODY_NAME("Default");

      class PlatformDefaultPhysicsActComp : public dtPhysics::PhysicsActComp
      {
      public:
         PlatformDefaultPhysicsActComp()
         : mLastLoadDamageState(NULL)
         {
            dtCore::RefPtr<dtPhysics::PhysicsObject> physObj = dtPhysics::PhysicsObject::CreateNew(PLATFORM_BODY_NAME);
            physObj->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
            physObj->SetMechanicsType(dtPhysics::MechanicsType::KINEMATIC);
            physObj->SetMass(500.0f);
            physObj->SetCollisionGroup(SimCore::CollisionGroup::GROUP_VEHICLE_GROUND);
            AddPhysicsObject(*physObj, true);
         }

         virtual void PrePhysicsUpdate()
         {
            dtPhysics::PhysicsObject* physObj = GetMainPhysicsObject();
            if (physObj != NULL)
            {
               dtCore::Transform xform;
               dtGame::GameActorProxy* owner = NULL;
               GetOwner(owner);
               dtCore::Transformable* xformable;
               owner->GetDrawable(xformable);
               if (xformable != NULL)
               {
                  xformable->GetTransform(xform);
               }

               physObj->SetTransformAsVisual(xform);
            }
         }

         virtual void OnRemovedFromActor(dtCore::BaseActorObject& actor)
         {
            dtPhysics::PhysicsActComp::OnRemovedFromActor(actor);
            SetPrePhysicsCallback(dtPhysics::PhysicsActComp::UpdateCallback());
         }

         virtual void OnEnteredWorld()
         {
            dtPhysics::PhysicsActComp::OnEnteredWorld();
            LoadCollision(false);
         }

         virtual void OnRemovedFromWorld()
         {
            dtPhysics::PhysicsActComp::OnRemovedFromWorld();
            mLastLoadDamageState = NULL;
         }

         void LoadCollision(bool reloadPhysics)
         {
            SimCore::Actors::Platform* plat = NULL;
            dtGame::GameActorProxy* actor = NULL;
            GetOwner(actor);
            if (actor != NULL)
               actor->GetDrawable(plat);
            if (plat == NULL)
            {
               LOG_ERROR("The drawable of the owner of the platform physics is not a platform drawable.");
               return;
            }

            if (!reloadPhysics && (mLastLoadDamageState != NULL && *mLastLoadDamageState == plat->GetDamageState()))
            {
               return;
            }

            std::string checkValue = GetModelResourceString(plat->GetGameActorProxy(), plat->GetDamageState());

            if (checkValue.empty())
            {
               return;
            }

            dtCore::RefPtr<dtPhysics::PhysicsObject> physObj = GetMainPhysicsObject();

            if (physObj.valid())
            {
               // it's re-added below, don't worry.
               RemovePhysicsObject(*physObj);
            }
            else
            {
               LOG_ERROR("The Physics object should already be created, attempting to create a another physics object.");
               physObj = dtPhysics::PhysicsObject::CreateNew(PLATFORM_BODY_NAME);
               physObj->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
               physObj->SetMechanicsType(dtPhysics::MechanicsType::KINEMATIC);
               physObj->SetCollisionGroup(SimCore::CollisionGroup::GROUP_VEHICLE_GROUND);
               physObj->SetMass(500.0f);
            }

            // TODO, if it changes to local again, it should figure out what the property was configured to be.
            // users also probably want to be able to configure what remote does.
            if (plat->IsRemote())
            {
               physObj->SetMechanicsType(dtPhysics::MechanicsType::KINEMATIC);
            }

            if (physObj->GetMechanicsType() != dtPhysics::MechanicsType::DYNAMIC)
            {
               if (!IsPrePhysicsCallbackValid())
               {
                  // If it's kinematic or static, the prephysics callback should be applied.
                  SetPrePhysicsCallback(dtPhysics::PhysicsActComp::UpdateCallback(this, &PlatformDefaultPhysicsActComp::PrePhysicsUpdate));
               }
            }

            AddPhysicsObject(*physObj, true);

            dtCore::Transform offsetXform;
            offsetXform.SetTranslation(-physObj->GetOriginOffset());

            physObj->SetVisualToBodyTransform(offsetXform);

            dtCore::Transform xform;
            plat->GetTransform(xform);

            physObj->SetTransformAsVisual(xform);

            osg::Node* node = NULL;
            osg::Group* drawNode = plat->GetOSGNode()->asGroup();
            if (drawNode != NULL && drawNode->getNumChildren() > 0)
            {
               node = drawNode->getChild(0);
            }

            if (physObj->GetMaterial() == NULL)
            {
               const dtPhysics::MaterialActor* matActor = LookupMaterialActor();
               if (matActor != NULL)
               {
                  physObj->SetMaterial(matActor->GetMaterial());
               }
            }

            // Note, this must be done after the visual to body transform is set because the origin
            // offset may be adjusted if the primitive type is a box, sphere, or cylinder and the center
            // of the bounding box doesn't match the center of the mesh
            physObj->Create(node, true, checkValue);

            mLastLoadDamageState = &plat->GetDamageState();
         }

      private:

         // Recursive function for getting right model resource.  It does what the platform does with displaying a model
         // by allowing destroyed to choose damaged or non damaged and Damaged to choose non damaged in the event
         // the proper model isn't assigned.
         std::string GetModelResourceString(dtGame::GameActorProxy& actor, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum& damState)
         {
            std::string checkValue;

            if (damState == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
            {
               checkValue = actor.GetProperty(SimCore::Actors::PlatformActorProxy::PROPERTY_MESH_NON_DAMAGED_ACTOR)->ToString();
            }
            else if (damState == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE
               || damState == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE )
            {
               checkValue = actor.GetProperty(SimCore::Actors::PlatformActorProxy::PROPERTY_MESH_DAMAGED_ACTOR)->ToString();
               if (checkValue.empty())
               {
                  checkValue = GetModelResourceString(actor, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE);
               }
            }
            else if (damState == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
            {
               checkValue = actor.GetProperty(SimCore::Actors::PlatformActorProxy::PROPERTY_MESH_DESTROYED_ACTOR)->ToString();
               if (checkValue.empty())
               {
                  checkValue = GetModelResourceString(actor, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE);
               }
            }
            return checkValue;
         }

         SimCore::Actors::BaseEntityActorProxy::DamageStateEnum* mLastLoadDamageState;
      };

   }

}

