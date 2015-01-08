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
* @author Bradley Anderegg 
*/
#include <prefix/SimCorePrefix.h>
#include <Actors/FireBallActor.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
#include <dtCore/enginepropertytypes.h>
//#include <dtABC/application.h>
//#include <dtAudio/audiomanager.h>
//#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
//#include <dtCore/keyboard.h>
#include <dtGame/basemessages.h>
//#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/DefaultFlexibleArticulationHelper.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Actors/BaseEntity.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <dtPhysics/collisioncontact.h>

//#include <dtUtil/nodeprintout.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>

#include <Components/GameLogicComponent.h>

namespace NetDemo
{
   //////////////////////////////////////////////////////////
   // Actor code
   //////////////////////////////////////////////////////////
   FireBallActor::FireBallActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mMaxTime(3.0f)
      , mCurrentTime(0.0f)
      , mVelocity(10.0f)
   {
      SetTerrainPresentDropHeight(0.0);

      SetEntityType("Fort");
      SetMunitionDamageTableName("StandardDamageType");
   }

   //////////////////////////////////////////////////////////
   FireBallActor::~FireBallActor()
   {

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();
      physObj->SetTransform(ourTransform);     
      physObj->Create(NULL);

      BaseClass::OnEnteredWorld();

      mCurrentTime = mMaxTime;

      // Add a dynamic light to our fort
      SimCore::Components::RenderingSupportComponent* renderComp;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);
      if(renderComp != NULL)
      {
         //Add a spot light
         mDynamicLight = new SimCore::Components::RenderingSupportComponent::DynamicLight();

         mDynamicLight->mRadius = 5.0f;
         mDynamicLight->mIntensity = 1.0f;
         mDynamicLight->mColor.set(1.0f, 1.0f, 0.0f);
         mDynamicLight->mAttenuation.set(0.0015, 0.000075, 0.005);
         mDynamicLight->mTarget = this;
         mDynamicLight->mFlicker = true;
         mDynamicLight->mFlickerScale = 1.0f;
         mDynamicLight->mFadeOut = true;
         mDynamicLight->mFadeOutTime = 0.5f;
         mDynamicLight->mAutoDeleteLightOnTargetNull = true;
         renderComp->AddDynamicLight(mDynamicLight.get());
      } 

      //add a shape volume for the beam
      SimCore::Components::VolumeRenderingComponent* vrc = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 
      if(vrc != NULL)
      {
         mShapeVolume = new SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord();
         mShapeVolume->mPosition.set(0.0f, 0.0f, 0.0f);
         mShapeVolume->mColor.set(1.0f, 0.2f, 0.15f, 1.0f);
         mShapeVolume->mShapeType = SimCore::Components::VolumeRenderingComponent::SPHERE;
         mShapeVolume->mRadius.set(0.15f, 0.0f, 0.0f);
         mShapeVolume->mNumParticles = 5;
         mShapeVolume->mParticleRadius = 2.15f;
         mShapeVolume->mDensity = 0.25f;
         mShapeVolume->mNoiseScale = 0.85f;
         mShapeVolume->mVelocity = 1.15f;
         mShapeVolume->mTarget = GetOSGNode();
         mShapeVolume->mAutoDeleteOnTargetNull = true;
         mShapeVolume->mAutoDeleteAfterMaxTime = true;
         mShapeVolume->mMaxTime = GetMaxTime();
         mShapeVolume->mFadeOut = true;
         mShapeVolume->mFadeOutTime = 0.5f;
         mShapeVolume->mShaderName = "FireVolumeShader";
         mShapeVolume->mRenderMode = SimCore::Components::VolumeRenderingComponent::PARTICLE_VOLUME;

         vrc->CreateShapeVolume(mShapeVolume);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallActor::DoExplosion(float)
   {
      //const osg::Vec3& finalVelocity, const osg::Vec3& location, const dtCore::Transformable* target )
      //printf("Sending DETONATION\r\n");

      dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      osg::Vec3 trans = ourTransform.GetTranslation();

      // Prepare a detonation message
      dtCore::RefPtr<SimCore::DetonationMessage> msg;
      gm->GetMessageFactory().CreateMessage( SimCore::MessageType::DETONATION, msg );

      // Required Parameters:
      msg->SetEventIdentifier( 1 );
      msg->SetDetonationLocation(trans);
      // --- DetonationResultCode 1 == Entity Impact, 3 == Ground Impact, 5 == Detonation
      msg->SetDetonationResultCode( 5 ); // TO BE DYNAMIC
      //msg->SetMunitionType("Generic Explosive");
      msg->SetMunitionType("Grenade");  // Other example options are "Bullet" and "High Explosive"
      msg->SetFuseType(0);
      msg->SetWarheadType(0);
      msg->SetQuantityFired(dtUtil::RandRange(1, 3));
      msg->SetSendingActorId(GetGameActorProxy().GetId());
      //msg->SetFinalVelocityVector( finalVelocity );
      msg->SetRateOfFire(5);

      gm->SendMessage( *msg );
      gm->SendNetworkMessage( *msg );

      SimCore::Components::VolumeRenderingComponent* vrc = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 
      if(vrc != NULL)
      {
         vrc->RemoveShapeVolume(mShapeVolume.get());
      }

      GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
   }

   //////////////////////////////////////////////////////////////////////
   void FireBallActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

      mCurrentTime -= tickMessage.GetDeltaSimTime();
      if(mCurrentTime <= 0.0f)
      {
         DoExplosion(0.0f);
      }
      else
      {
         dtCore::Transform ourTransform;
         dtCore::Transform targetTransform;
         osg::Vec3 currentDirection, currentPosition;

         GetTransform(ourTransform);
         ourTransform.GetTranslation(currentPosition);
         
         if(mTarget.valid())
         {
            osg::Vec3 targetPos;
            mTarget->GetTransform(targetTransform);
            targetTransform.GetTranslation(targetPos);
            currentDirection = targetPos - currentPosition;
            currentDirection.normalize();
         }
         else //if our target has been deleted just go straight
         {
            ourTransform.GetRow(1, currentDirection);
         }

         dtPhysics::PhysicsActComp* physAC = NULL;
         GetComponent(physAC);
         if (physAC != NULL && physAC->GetMainPhysicsObject() != NULL)
         {
            physAC->GetMainPhysicsObject()->ApplyImpulse(currentDirection * mVelocity);

            physAC->GetMainPhysicsObject()->AddForce(mForces);
            
            //these get reset every frame
            mForces.set(0.0f, 0.0f, 0.0f);

            std::vector<dtPhysics::CollisionContact> contacts;
            dtPhysics::PhysicsWorld::GetInstance().GetContacts(*physAC->GetMainPhysicsObject(), contacts);
            if(!contacts.empty())
            {
               DoExplosion(0.0f);
            }
         }
      }

      SimCore::Components::VolumeRenderingComponent* vrc = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 
      if(vrc != NULL && mShapeVolume.valid() && mShapeVolume->mParticleDrawable.valid())
      {
         osg::Vec3 vel(2.475f, 0.0f, 0.0f);
         vel[0] *= tickMessage.GetDeltaSimTime();

         mShapeVolume->mRadius[0] += vel[0];
         mShapeVolume->mDirtyParams = true;
         //vrc->ExpandVolume(*mShapeVolume, vel);
         //vrc->ComputeParticleRadius(*mShapeVolume);
      }
   }

   //////////////////////////////////////////////////////////////////////
   void FireBallActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );
   }

   //////////////////////////////////////////////////////////
   void FireBallActor::SetVelocity(float vel)
   {
      mVelocity = vel;
   }

   //////////////////////////////////////////////////////////
   float FireBallActor::GetVelocity() const
   {
      return mVelocity;
   }
   
   //////////////////////////////////////////////////////////
   void FireBallActor::SetPosition( const osg::Vec3& pos )
   {
      dtCore::Transform xform;
      GetTransform(xform);
      xform.SetTranslation(pos);

      SetTransform(xform);
   }

   //////////////////////////////////////////////////////////
   const osg::Vec3 FireBallActor::GetPosition() const
   {
      dtCore::Transform xform;
      GetTransform(xform);
      return xform.GetTranslation();
   }

   //////////////////////////////////////////////////////////
   void FireBallActor::AddForce(const osg::Vec3& f)
   {
      mForces += f;
   }

   //////////////////////////////////////////////////////////
   void FireBallActor::SetTarget(dtCore::Transformable& t)
   {
      mTarget = &t;
   }

   //////////////////////////////////////////////////////////
   void FireBallActor::SetMaxTime(float t)
   {
      mMaxTime = t;
   }

   //////////////////////////////////////////////////////////
   float FireBallActor::GetMaxTime() const
   {
      return mMaxTime;
   }

   //////////////////////////////////////////////////////////
   // Proxy code
   //////////////////////////////////////////////////////////
   FireBallActorProxy::FireBallActorProxy()
   {
      SetClassName("SimCore::Actors::FireBallActor");
   }

   //////////////////////////////////////////////////////////
   FireBallActorProxy::~FireBallActorProxy()
   {

   }


   //////////////////////////////////////////////////////////
   void FireBallActorProxy::BuildPropertyMap()
   {
      FireBallActor* fa = NULL;
      GetDrawable(fa);

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallActorProxy::BuildActorComponents()
   {
      BaseClass::BuildActorComponents();


      dtPhysics::PhysicsActComp* physAC = NULL;
      GetComponent(physAC);
      // Add our initial body.
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = dtPhysics::PhysicsObject::CreateNew("FireBallBody");
      physAC->AddPhysicsObject(*physicsObject);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::SPHERE);
      physicsObject->SetMass(30.0f);
      physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
      physicsObject->SetNotifyCollisions(true);

      dtGame::DRPublishingActComp* drPublishingActComp = NULL;
      GetComponent(drPublishingActComp);
      if (drPublishingActComp == NULL)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(2.0f);
      //drPublishingActComp->SetPublishLinearVelocity(false);
      //drPublishingActComp->SetPublishAngularVelocity(false);
   }
}
