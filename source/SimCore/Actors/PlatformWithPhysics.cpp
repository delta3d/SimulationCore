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
#include <prefix/SimCorePrefix-src.h>

#ifdef AGEIA_PHYSICS
#include <NxAgeiaWorldComponent.h>
#else
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsobject.h>
#endif

#include <SimCore/Actors/PlatformWithPhysics.h>
#include <dtCore/scene.h>
#include <dtDAL/enginepropertytypes.h>

#include <osg/Matrix>

namespace SimCore
{
   namespace Actors
   {
      const std::string &PlatformWithPhysics::DEFAULT_NAME = "Default";
      const std::string &PlatformWithPhysics::BUILDING_DEFAULT_NAME = "Building - Terrain Actor Geometry";

      /////////////////////////////////////////////////////////////////////////
      PlatformWithPhysics::PlatformWithPhysics(PlatformActorProxy &proxy)
      : Platform(proxy)
      {
#ifdef AGEIA_PHYSICS
            mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy);
            mPhysicsHelper->SetBaseInterfaceClass(this);
#else
            mPhysicsHelper = new dtPhysics::PhysicsHelper(proxy);
#endif
         mLoadGeomFromNode = false;
      }

      /////////////////////////////////////////////////////////////////////////
      PlatformWithPhysics::~PlatformWithPhysics()
      {
      }

      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::SetDamageState(BaseEntityActorProxy::DamageStateEnum& damageState)
      {
         Platform::SetDamageState( damageState );
         if(mLoadGeomFromNode == true)
            return;
         LoadCollision();
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
#ifdef AGEIA_PHYSICS
            mPhysicsHelper->SetCollisionStaticMesh(mNodeForGeometry.get(), NxVec3(0,0,0), false, "");
#else
            dtCore::RefPtr<dtPhysics::PhysicsObject> physObj= new dtPhysics::PhysicsObject(DEFAULT_NAME);
            physObj->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
            physObj->CreateFromProperties(mNodeForGeometry.get());
            mPhysicsHelper->AddPhysicsObject(*physObj);
#endif
         }
         else // this is for objects moving around, in our case vehicles
         {
            LoadCollision();
         }

#ifdef AGEIA_PHYSICS
         dtGame::GMComponent *comp =
            GetGameActorProxy().GetGameManager()->GetComponentByName(dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_NAME);
         if(comp != NULL)
         {
            static_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(comp)->RegisterAgeiaHelper(*mPhysicsHelper);
         }
#else
         dtPhysics::PhysicsComponent *comp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME);
         if(comp != NULL)
         {
            comp->RegisterHelper(*mPhysicsHelper);
         }
#endif
      }


      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::LoadCollision()
      {
         std::string checkValue;
         unsigned int modelToLoad = 0;

         // release if something is already made for this actor
         mPhysicsHelper->RemovePhysicsObject(DEFAULT_NAME);

         BaseEntityActorProxy::DamageStateEnum& damState = GetDamageState();
         if (damState == BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
         {
            checkValue = GetGameActorProxy().GetProperty(PlatformActorProxy::PROPERTY_MESH_NON_DAMAGED_ACTOR)->ToString();
         }
         else if (damState == BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE
            || damState == BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE )
         {
            checkValue = GetGameActorProxy().GetProperty(PlatformActorProxy::PROPERTY_MESH_DAMAGED_ACTOR)->ToString();
            modelToLoad = 1;
         }
         else if (damState == BaseEntityActorProxy::DamageStateEnum::DESTROYED)
         {
            checkValue = GetGameActorProxy().GetProperty(PlatformActorProxy::PROPERTY_MESH_DESTROYED_ACTOR)->ToString();
            modelToLoad = 2;
         }

         if(checkValue.empty())
         {
            LOG_DEBUG("Unable to load file, resource was not valid! This is for actor \"" +
               GetUniqueId().ToString() + "\", damage state \"" + damState.GetName() + "\". However this is called from " +
               "setdamagestate, and model may not be valid yet.");
            return;
         }
         else
#ifdef AGEIA_PHYSICS
         {
            dtCore::Transform ourTransform, zeroTransform;
            GetTransform(ourTransform);
            osg::Matrix rot;
            ourTransform.GetRotation(rot);
            rot.invert(rot);

            NxMat34 sendInMatrix(NxMat33( NxVec3(rot(0,0), rot(0,1), rot(0,2)),
                                          NxVec3(rot(1,0), rot(1,1), rot(1,2)),
                                          NxVec3(rot(2,0), rot(2,1), rot(2,2))),
                                 NxVec3(0,0,0));
            mPhysicsHelper->SetAgeiaMass(5000);
            mPhysicsHelper->SetResourceName(checkValue);
            mPhysicsHelper->SetLoadAsCached(true);
            //I don't think we want this.
            mPhysicsHelper->SetIsKinematic(true);
            mPhysicsHelper->SetPhysicsModelTypeEnum(dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CONVEXMESH);
            switch(modelToLoad)
            {
               case 0:
               default:
               {
                  mPhysicsHelper->InitializePrimitive(GetNonDamagedFileNode(), sendInMatrix);
               }
               break;

               case 1:
               {
                  mPhysicsHelper->InitializePrimitive(GetDamagedFileNode(), sendInMatrix);
               }
               break;

               case 2:
               {
                  mPhysicsHelper->InitializePrimitive(GetDestroyedFileNode(),sendInMatrix);
               }
               break;
            }
         }
         mPhysicsHelper->SetAgeiaUserData(mPhysicsHelper.get());
         mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
#else
         {
            dtCore::RefPtr<dtPhysics::PhysicsObject> physObj= new dtPhysics::PhysicsObject(DEFAULT_NAME);
            physObj->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);

            switch(modelToLoad)
            {
               case 0:
               default:
               {
                  physObj->CreateFromProperties(GetNonDamagedFileNode());
               }
               break;

               case 1:
               {
                  physObj->CreateFromProperties(GetDamagedFileNode());
               }
               break;

               case 2:
               {
                  physObj->CreateFromProperties(GetDestroyedFileNode());
               }
               break;
            }

            dtCore::Transform xform;
            GetTransform(xform);

            physObj->SetMass(5000);
            physObj->SetTransform(xform);
            mPhysicsHelper->AddPhysicsObject(*physObj);
         }
         mPhysicsHelper->SetPrePhysicsCallback(dtPhysics::PhysicsHelper::UpdateCallback(this, &PlatformWithPhysics::PrePhysicsUpdate));
#endif
      }

#ifdef AGEIA_PHYSICS
      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysics::AgeiaPrePhysicsUpdate()
      {
         if(!mLoadGeomFromNode)
         {
            dtCore::Transform ourTransform;
            GetTransform(ourTransform);
            osg::Matrix rot;
            ourTransform.GetRotation(rot);

            dtPhysics::PhysicsObject* toFillIn = mPhysicsHelper->GetPhysicsObject();
            if(toFillIn != NULL)
            {
               osg::Vec3 translation;
               ourTransform.GetTranslation(translation);

               toFillIn->setGlobalPosition(NxVec3(translation[0], translation[1], translation[2]));
               toFillIn->setGlobalOrientation(
                  NxMat33( NxVec3(rot(0,0), rot(0,1), rot(0,2)),
                           -NxVec3(rot(1,0), rot(1,1), rot(1,2)),
                           NxVec3(rot(2,0), rot(2,1), rot(2,2))));
            }
         }
      }
#else
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
               physObj->GetTransform(xform);
               SetTransform(xform);
            }
         }
      }

#endif
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

         BaseEntity* entityActor = dynamic_cast<BaseEntity*> (GetActor());
         if( entityActor != NULL )
         {
            entityActor->InitDeadReckoningHelper();
         }
      }

      /////////////////////////////////////////////////////////////////////////
      void PlatformWithPhysicsActorProxy::OnEnteredWorld()
      {
         PlatformActorProxy::OnEnteredWorld();
      }

   } // namespace
} // namespace
