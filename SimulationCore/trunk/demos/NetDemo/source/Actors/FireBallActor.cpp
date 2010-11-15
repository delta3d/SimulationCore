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
#include <dtDAL/enginepropertytypes.h>
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
#include <SimCore/Actors/DRPublishingActComp.h>
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
   FireBallActor::FireBallActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   {
      SetTerrainPresentDropHeight(0.0);
      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      dtPhysics::PhysicsHelper* helper = new dtPhysics::PhysicsHelper(proxy);
      SetPhysicsHelper(helper);

      // Add our initial body.
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("FireBallBody");
      helper->AddPhysicsObject(*physicsObject);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::SPHERE);
      physicsObject->SetMass(3.0f);
      physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
      physicsObject->SetNotifyCollisions(true);

      SetEntityType("Fort");
      SetMunitionDamageTableName("StandardDamageType");
   }

   //////////////////////////////////////////////////////////
   FireBallActor::~FireBallActor()
   {

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallActor::BuildActorComponents()
   {
      BaseClass::BuildActorComponents();

      SimCore::Actors::DRPublishingActComp* drPublishingActComp = GetDRPublishingActComp();
      if (drPublishingActComp == NULL)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(2.0f);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      dtPhysics::PhysicsObject *physObj = GetPhysicsHelper()->GetMainPhysicsObject();
      physObj->SetTransform(ourTransform);     
      physObj->CreateFromProperties(NULL);

      BaseClass::OnEnteredWorld();

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
         mDynamicLight->mAttenuation.set(0.0025, 0.00015, 0.008);
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
         mShapeVolume->mRadius.set(0.1f, 0.0f, 0.0f);
         mShapeVolume->mNumParticles = 10;
         mShapeVolume->mParticleRadius = 1.5f;
         mShapeVolume->mDensity = 0.25f;
         mShapeVolume->mNoiseScale = 0.85f;
         mShapeVolume->mVelocity = 0.15f;
         mShapeVolume->mTarget = this;
         mShapeVolume->mAutoDeleteOnTargetNull = true;
         mShapeVolume->mShaderName = "FireVolumeShader";
         mShapeVolume->mRenderMode = SimCore::Components::VolumeRenderingComponent::PARTICLE_VOLUME;

         vrc->CreateShapeVolume(mShapeVolume);
      }

      if(GetPhysicsHelper() != NULL && GetPhysicsHelper()->GetMainPhysicsObject() != NULL)
      {
         osg::Vec3 up(0.0, 0.0, 150.0);
         GetPhysicsHelper()->GetMainPhysicsObject()->ApplyImpulse(mVelocity);
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
         vrc->RemoveShapeVolume(mShapeVolume->mId);

         mShapeVolume->mParentNode->removeChildren(0, mShapeVolume->mParentNode->getNumChildren());
      }

      GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
   }

   //////////////////////////////////////////////////////////////////////
   void FireBallActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );


      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      ourTransform.Move(mVelocity * tickMessage.GetDeltaSimTime());

      if(GetPhysicsHelper() != NULL && GetPhysicsHelper()->GetMainPhysicsObject() != NULL)
      {
         GetPhysicsHelper()->GetMainPhysicsObject()->ApplyImpulse(mVelocity);

         std::vector<dtPhysics::CollisionContact> contacts;
         dtPhysics::PhysicsWorld::GetInstance().GetContacts(*GetPhysicsHelper()->GetMainPhysicsObject(), contacts);
         if(!contacts.empty())
         {
            DoExplosion(0.0f);
         }
      }

      //SimCore::Components::VolumeRenderingComponent* vrc = NULL;
      //GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 
      //if(vrc != NULL && mShapeVolume.valid())
      //{
      //   osg::Vec3 vel(2.10f, 0.0f, 2.0f);
      //   vel *= tickMessage.GetDeltaSimTime();

      //   vrc->ExpandVolume(*mShapeVolume, vel);
      //   vrc->ComputeParticleRadius(*mShapeVolume);
      //}
   }

   //////////////////////////////////////////////////////////////////////
   void FireBallActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );
   }

   //////////////////////////////////////////////////////////
   void FireBallActor::SetVelocity( const osg::Vec3& vel )
   {
      mVelocity = vel;
   }

   //////////////////////////////////////////////////////////
   const osg::Vec3& FireBallActor::GetVelocity() const
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
   const osg::Vec3& FireBallActor::GetPosition() const
   {
      dtCore::Transform xform;
      GetTransform(xform);
      return xform.GetTranslation();
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
      GetActor(fa);

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();
   }
}
