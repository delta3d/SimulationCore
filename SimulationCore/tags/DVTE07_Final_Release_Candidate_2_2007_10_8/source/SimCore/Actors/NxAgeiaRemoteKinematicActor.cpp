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
   #include <NxAgeiaWorldComponent.h>
#endif

#include <SimCore/Actors/NxAgeiaRemoteKinematicActor.h>
#include <dtCore/scene.h>
#include <dtDAL/enginepropertytypes.h>

#include <osg/matrix>

namespace SimCore
{
   namespace Actors
   {
      const std::string &NxAgeiaRemoteKinematicActor::DEFAULT_NAME = "Default";
      const std::string &NxAgeiaRemoteKinematicActor::BUILDING_DEFAULT_NAME = "Building - Terrain Actor Geometry";
      
      /////////////////////////////////////////////////////////////////////////
      NxAgeiaRemoteKinematicActor::NxAgeiaRemoteKinematicActor(PlatformActorProxy &proxy) 
      : Platform(proxy)
      {
         #ifdef AGEIA_PHYSICS
            mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy);
            mPhysicsHelper->SetBaseInterfaceClass(this);
         #endif
         mLoadGeomFromNode = false;
      }

      /////////////////////////////////////////////////////////////////////////
      NxAgeiaRemoteKinematicActor::~NxAgeiaRemoteKinematicActor()
      {
      }

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActor::SetDamageState(BaseEntityActorProxy::DamageStateEnum &damageState)
      {
         Platform::SetDamageState( damageState );
         #ifdef AGEIA_PHYSICS
            if(mLoadGeomFromNode == true)
               return;

            // release if something is already made for this actor
            mPhysicsHelper->ReleasePhysXObject(DEFAULT_NAME);

            // load the physics to it
            LoadCollision();

         #endif
      }

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActor::TickLocal(const dtGame::Message& tickMessage)
      {
         // this is so buildings dont get entities tick local method.
         if(!mLoadGeomFromNode)
         {
            Platform::TickLocal(tickMessage);
         }
      }

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActor::OnEnteredWorld()
      {
         // this is for static world geometry
         Platform::OnEnteredWorld();
         #ifdef AGEIA_PHYSICS
            if(mLoadGeomFromNode)
            {
               SetName(BUILDING_DEFAULT_NAME);
               mPhysicsHelper->SetCollisionStaticMesh(mNodeForGeometry.get(), NxVec3(0,0,0), false, "");
            }
            else // this is for objects moving around, in our case vehicles
            {
               LoadCollision();
            }

            dtGame::GMComponent *comp = 
               GetGameActorProxy().GetGameManager()->GetComponentByName(dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_NAME);
            if(comp != NULL)
            {
               static_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(comp)->RegisterAgeiaHelper(*mPhysicsHelper.get());
            }
         #endif
      }

      #ifdef AGEIA_PHYSICS

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActor::LoadCollision()
      {
         std::string checkValue;
         unsigned int modelToLoad = 0;

         if(GetDamageState() == BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
         {
            checkValue = GetGameActorProxy().GetProperty("Non-damaged actor")->ToString();
         }
         else if(GetDamageState() == BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE
            || GetDamageState() == BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE )
         {
            checkValue = GetGameActorProxy().GetProperty("Damaged actor")->ToString();
            modelToLoad = 1;
         }
         else if(GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED)
         {
            checkValue = GetGameActorProxy().GetProperty("Destroyed actor")->ToString();
            modelToLoad = 2;
         }
         else
         {
            LOG_WARNING("No damage state found on remote kinematic actor, bailing out");
            return;
         }

         if(checkValue.empty())
         {
            LOG_DEBUG("Unable to load file, resource was not valid! This is for actor [" + 
               GetUniqueId().ToString() + "], damage state[" + GetDamageState().GetName() + "]. However this is called from " +
               "setdamagestate, and model may not be valid yet.");
            return;
         }
         else
         {
            dtCore::Transform ourTransform, zeroTransform;
            GetTransform(ourTransform);
            osg::Matrix rot = ourTransform.GetRotation();
            rot.invert(rot);

            NxMat34 sendInMatrix(NxMat33( NxVec3(rot.operator ()(0,0), rot.operator ()(0,1), rot.operator ()(0,2)),
                                          NxVec3(rot.operator ()(1,0), rot.operator ()(1,1), rot.operator ()(1,2)),
                                          NxVec3(rot.operator ()(2,0), rot.operator ()(2,1), rot.operator ()(2,2))), 
                                 NxVec3(0,0,0));
            mPhysicsHelper->SetAgeiaMass(5000);
            mPhysicsHelper->SetResourceName(checkValue);
            mPhysicsHelper->SetLoadAsCached(true);
            mPhysicsHelper->SetIsKinematic(true);
            mPhysicsHelper->SetPhysicsModelTypeEnum(dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CONVEXMESH);
            switch(modelToLoad)
            {
               case 0:
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
               default:
               {
                  mPhysicsHelper->InitializePrimitive(GetDestroyedFileNode(),sendInMatrix);
               }
               break;
            }
         }
         mPhysicsHelper->SetAgeiaUserData(mPhysicsHelper.get());
         mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
      }

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, NxActor& ourSelf, NxActor& whatWeHit){}
      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActor::AgeiaPrePhysicsUpdate()
      {
         if(!mLoadGeomFromNode)
         {
            dtCore::Transform ourTransform;
            GetTransform(ourTransform);
            osg::Matrix *rot = &ourTransform.GetRotation();

            NxActor* toFillIn = mPhysicsHelper->GetPhysXObject();
            if(toFillIn != NULL)
            {
               toFillIn->setGlobalPosition(NxVec3(ourTransform.GetTranslation()[0], ourTransform.GetTranslation()[1], ourTransform.GetTranslation()[2]));
               toFillIn->setGlobalOrientation(
                  NxMat33( NxVec3(rot->operator ()(0,0), rot->operator ()(0,1), rot->operator ()(0,2)),
                           -NxVec3(rot->operator ()(1,0), rot->operator ()(1,1), rot->operator ()(1,2)),
                           NxVec3(rot->operator ()(2,0), rot->operator ()(2,1), rot->operator ()(2,2))));
            }
         }
      }

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActor::AgeiaPostPhysicsUpdate(){}
      #endif
      ////////////////////////////////////////////////////////////////////
      // Actor Proxy Below here
      ////////////////////////////////////////////////////////////////////
      NxAgeiaRemoteKinematicActorProxy::NxAgeiaRemoteKinematicActorProxy()
      {
         SetClassName("NxAgeiaRemoteKinematicActor");
      }

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActorProxy::BuildPropertyMap()
      {
         PlatformActorProxy::BuildPropertyMap();
      #ifdef AGEIA_PHYSICS
         NxAgeiaRemoteKinematicActor* actor = dynamic_cast<NxAgeiaRemoteKinematicActor*> (GetActor());
         actor->GetPhysicsHelper()->BuildPropertyMap();
      #endif
      }

      /////////////////////////////////////////////////////////////////////////
      NxAgeiaRemoteKinematicActorProxy::~NxAgeiaRemoteKinematicActorProxy()
      {
      }
      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActorProxy::CreateActor()
      {
         SetActor(*new NxAgeiaRemoteKinematicActor(*this));

         BaseEntity* entityActor = dynamic_cast<BaseEntity*> (GetActor());
         if( entityActor != NULL )
         {
            entityActor->InitDeadReckoningHelper();
         }
      }

      /////////////////////////////////////////////////////////////////////////
      void NxAgeiaRemoteKinematicActorProxy::OnEnteredWorld()
      {
         PlatformActorProxy::OnEnteredWorld();
      }

   } // namespace
} // namespace
