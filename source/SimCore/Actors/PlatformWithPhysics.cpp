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

#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsobject.h>

#include <SimCore/PhysicsTypes.h>

#include <SimCore/Actors/PlatformWithPhysics.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtCore/observerptr.h>
#include <dtDAL/enginepropertytypes.h>

#include <osg/Matrix>
#include <osg/MatrixTransform>

namespace SimCore
{
   namespace Actors
   {
      const std::string PlatformWithPhysics::DEFAULT_NAME = "Default";
      const std::string PlatformWithPhysics::BUILDING_DEFAULT_NAME = "Building - Terrain Actor Geometry";

      /////////////////////////////////////////////////////////////////////////
      PlatformWithPhysics::PlatformWithPhysics(PlatformActorProxy& proxy)
      : Platform(proxy)
      {
         mPhysicsHelper = new dtPhysics::PhysicsHelper(proxy);
         dtCore::RefPtr<dtPhysics::PhysicsObject> physObj= new dtPhysics::PhysicsObject(DEFAULT_NAME);
         physObj->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
         physObj->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
         physObj->SetMass(500.0f);
         mPhysicsHelper->AddPhysicsObject(*physObj);
         mPhysicsHelper->SetPrePhysicsCallback(dtPhysics::PhysicsHelper::UpdateCallback(this, &PlatformWithPhysics::PrePhysicsUpdate));
         mLoadGeomFromNode = false;
      }

      /////////////////////////////////////////////////////////////////////////
      PlatformWithPhysics::~PlatformWithPhysics()
      {
      }

      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::SetDamageState(BaseEntityActorProxy::DamageStateEnum& damageState)
      {
         bool reloadCollision = false;
         if (damageState != GetDamageState())
         {
            reloadCollision = true;
         }

         Platform::SetDamageState( damageState );
         if(mLoadGeomFromNode == true)
            return;
         if (reloadCollision)
         {
            LoadCollision();
         }
      }

      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         // this is so buildings dont get entities tick local method.
         if(!mLoadGeomFromNode)
         {
            Platform::OnTickLocal(tickMessage);
         }
      }

      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::OnEnteredWorld()
      {
         // this is for static world geometry
         Platform::OnEnteredWorld();
         if(mLoadGeomFromNode)
         {
            SetName(BUILDING_DEFAULT_NAME);
            mPhysicsHelper->GetMainPhysicsObject()->CreateFromProperties(mNodeForGeometry.get());
            mPhysicsHelper->GetMainPhysicsObject()->SetMechanicsType(dtPhysics::MechanicsType::STATIC);

            osg::Matrix bodyOffset;
            bodyOffset.setTrans(-mPhysicsHelper->GetMainPhysicsObject()->GetOriginOffset());
            dtCore::Transform offsetXform;
            offsetXform.Set(bodyOffset);

            mPhysicsHelper->GetMainPhysicsObject()->SetVisualToBodyTransform(offsetXform);
         }
         else // this is for objects moving around, in our case vehicles
         {
            LoadCollision();
         }

         dtPhysics::PhysicsComponent *comp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, comp);
         if(comp != NULL)
         {
            comp->RegisterHelper(*mPhysicsHelper);
         }
      }


      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::LoadCollision()
      {
         std::string checkValue;

         BaseEntityActorProxy::DamageStateEnum& damState = GetDamageState();
         if (damState == BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
         {
            checkValue = GetGameActorProxy().GetProperty(PlatformActorProxy::PROPERTY_MESH_NON_DAMAGED_ACTOR)->ToString();
         }
         else if (damState == BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE
            || damState == BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE )
         {
            checkValue = GetGameActorProxy().GetProperty(PlatformActorProxy::PROPERTY_MESH_DAMAGED_ACTOR)->ToString();
         }
         else if (damState == BaseEntityActorProxy::DamageStateEnum::DESTROYED)
         {
            checkValue = GetGameActorProxy().GetProperty(PlatformActorProxy::PROPERTY_MESH_DESTROYED_ACTOR)->ToString();
         }

         if (checkValue.empty())
         {
            LOG_DEBUG("Unable to load file, resource was not valid! This is for actor \"" +
               GetUniqueId().ToString() + ". However this is called from " +
               "setdamagestate, and model may not be valid yet.");
            return;
         }
         else
         {
            dtCore::RefPtr<dtPhysics::PhysicsObject> physObj = mPhysicsHelper->GetPhysicsObject(DEFAULT_NAME);

            if (physObj.valid())
            {
               // it's re-added below, don't worry.
               mPhysicsHelper->RemovePhysicsObject(*physObj);
            }
            else
            {
               LOG_ERROR("The Physics object should already be created, attempting to create a another physics object.");
               physObj = new dtPhysics::PhysicsObject(DEFAULT_NAME);
               physObj->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
               physObj->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
               physObj->SetMass(500.0f);
            }

            // TODO, if it changes to local again, it should figure out what the property was configured to be.
            // users also probably want to be able to configure what remote does.
            if (IsRemote())
            {
               physObj->SetMechanicsType(dtPhysics::MechanicsType::KINEMATIC);
            }

            dtCore::Transform xform;
            GetTransform(xform);

            physObj->SetTransform(xform);
            mPhysicsHelper->AddPhysicsObject(*physObj);

            osg::Matrix bodyOffset;
            bodyOffset.setTrans(-mPhysicsHelper->GetMainPhysicsObject()->GetOriginOffset());
            dtCore::Transform offsetXform;
            offsetXform.Set(bodyOffset);

            mPhysicsHelper->GetMainPhysicsObject()->SetVisualToBodyTransform(offsetXform);

            physObj->CreateFromProperties(&GetScaleMatrixTransform());

         }
      }

      ////////////////////////////////////////////////////////////////////
      dtPhysics::PhysicsHelper* PlatformWithPhysics::GetPhysicsHelper()
      {
         return mPhysicsHelper.get();
      }

      ////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::PrePhysicsUpdate()
      {
         if(!mLoadGeomFromNode)
         {
            dtPhysics::PhysicsObject* physObj = mPhysicsHelper->GetPhysicsObject(DEFAULT_NAME);
            if (physObj != NULL)
            {
               dtCore::Transform xform;
               GetTransform(xform);
               physObj->SetTransformAsVisual(xform);
            }
         }
      }

      ////////////////////////////////////////////////////////////////////
      // Actor Proxy Below here
      ////////////////////////////////////////////////////////////////////
      PlatformWithPhysicsActorProxy::PlatformWithPhysicsActorProxy()
      {
         SetClassName("PlatformWithPhysics");
      }

      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysicsActorProxy::BuildPropertyMap()
      {
         PlatformActorProxy::BuildPropertyMap();
         PlatformWithPhysics* actor = NULL;
         GetActor(actor);
         std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
         actor->GetPhysicsHelper()->BuildPropertyMap(toFillIn);
         for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
            AddProperty(toFillIn[i].get());
      }

      /////////////////////////////////////////////////////////////////////////
      PlatformWithPhysicsActorProxy::~PlatformWithPhysicsActorProxy()
      {
      }
      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysicsActorProxy::CreateActor()
      {
         SetActor(*new PlatformWithPhysics(*this));
      }

      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysicsActorProxy::OnEnteredWorld()
      {
         PlatformActorProxy::OnEnteredWorld();
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::ActorProperty> PlatformWithPhysicsActorProxy::GetDeprecatedProperty(const std::string& name)
      {
         PlatformWithPhysics* actor = NULL;
         GetActor(actor);
         return actor->GetPhysicsHelper()->GetDeprecatedProperty(name);
      }

   } // namespace
} // namespace
