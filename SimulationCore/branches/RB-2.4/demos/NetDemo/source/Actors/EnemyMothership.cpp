/* Net Demo - EnemyMothership (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
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
* Bradley Anderegg
*/
#include <dtUtil/mswin.h>
#include <Actors/EnemyMothership.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palphysicsworld.h>

#include <EnemyMothershipAIHelper.h> 
#include <AIEvent.h>
#include <AIState.h>

#include <ActorRegistry.h>
#include <Actors/FortActor.h>

#include <SimCore/Components/RenderingSupportComponent.h>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMothershipActor::EnemyMothershipActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : BaseEnemyActor(proxy)
   {
      mAIHelper = new EnemyMothershipAIHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMothershipActor::~EnemyMothershipActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   osg::Vec3 EnemyMothershipActor::GetSpawnPoint() const
   {
      dtCore::Transform xform;

      GetTransform(xform);
      osg::Vec3 pos = xform.GetTranslation();
      pos[2] -= 10.0f;
      return pos;
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMothershipActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      if (!IsRemote()) //only run locally
      {
         mAIHelper->Init(NULL);

         //this will allow the AI to actually move us
         mAIHelper->GetPhysicsModel()->SetPhysicsHelper(GetPhysicsHelper());

         //we set our transform for the first time so the AI knows what it is
         //the other enemy helpers get created with an enemy description actor
         //so there position doesnt come from the actor prototype.
         dtCore::Transform trans;
         GetTransform(trans);
         mAIHelper->SetTransform(trans);

         //redirecting the find target function
         dtAI::NPCState* state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyMothershipActor::FindTarget));

         //calling spawn will start the AI
         mAIHelper->Spawn();

         //adding a blue light to us
         AddDynamicLight();
      }
   }

   void EnemyMothershipActor::FindTarget(float)
   {
      //temporarily lets just look for a fort to destroy
      FortActorProxy* fortProxy = NULL;
      GetGameActorProxy().GetGameManager()->FindActorByType(*NetDemoActorRegistry::FORT_ACTOR_TYPE, fortProxy);
      if (fortProxy != NULL)
      {
         FortActor& fort = *static_cast<FortActor*>(fortProxy->GetActor());
         mAIHelper->SetCurrentTarget(fort);
      }

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMothershipActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      //update the entities orientation
      //dtCore::Transform trans;
      //GetTransform(trans);

      //mAIHelper->PostSync(trans);
      //SetTransform(trans);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMothershipActor::PostPhysicsUpdate()
   {
      // Mostly copied from BasePhysicsVehicleActor - we do NOT want want our vehicle to 'roll', so we
      // take the position and throw away the rotation.

      // This is ONLY called if we are LOCAL (we put the check here just in case... )
      if (!IsRemote() && GetPhysicsHelper() != NULL)
      {
         // The base behavior is that we want to pull the translation and rotation off the object
         // in our physics scene and apply it to our 3D object in the visual scene.
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();

         //TODO: Ask if the object is activated.  If not, the transform should not be pushed.
         if (!GetPushTransformToPhysics())
         {
            if(physicsObject != NULL)
            {
               // Take rotation from physics and apply to current xform - IE NO ROTATION!
               dtCore::Transform currentXForm;
               GetTransform(currentXForm);
               dtCore::Transform physicsXForm;
               physicsObject->GetTransform(physicsXForm);
               currentXForm.SetTranslation(physicsXForm.GetTranslation());

               //apply our own rotation, and verify the position is ok for us
               //... or allow us to apply our own physical constraints
               mAIHelper->PostSync(currentXForm);

               SetTransform(currentXForm);
               SetPushTransformToPhysics(false);
            }
         }
      }
   }


   //////////////////////////////////////////////////////////////////////
   void EnemyMothershipActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      //Tick the AI
      //update the AI's position and orientation
      dtCore::Transform trans;
      GetTransform(trans);
      mAIHelper->PreSync(trans);

      ////////let the AI do its thing
      mAIHelper->Update(tickMessage.GetDeltaSimTime());

      BaseClass::OnTickLocal(tickMessage);
   }

   void EnemyMothershipActor::AddDynamicLight()
   {
      SimCore::Components::RenderingSupportComponent* rsc = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsc);

      if(rsc != NULL)
      {
         SimCore::Components::RenderingSupportComponent::SpotLight* light = new SimCore::Components::RenderingSupportComponent::SpotLight();
         light->mTarget = this;
         light->mAutoDeleteLightOnTargetNull = true;
         light->mIntensity = 1.0f;        
         light->mAttenuation.set(0.000025f, 0.00005f, 0.00005f);
         light->mColor.set(0.25f, 0.35f, 0.65f);
         light->mRadius = 150.0f;
         light->mFlicker = true;
         light->mFlickerScale = 0.25f;
         light->mUseAbsoluteDirection = true;
         light->mDirection.set(0.0f, 0.0f, -1.0f);
         light->mSpotExponent = 3.5f;
         light->mSpotCosCutoff = 0.85f;
         
         rsc->AddDynamicLight(light);
      }

   }
  
   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   EnemyMothershipActorProxy::EnemyMothershipActorProxy()
   {
      SetClassName("EnemyMothershipActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMothershipActorProxy::BuildPropertyMap()
   {
      const std::string GROUP = "Enemy Props";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      EnemyMothershipActor& actor = static_cast<EnemyMothershipActor &>(GetGameActor());

   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMothershipActorProxy::~EnemyMothershipActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMothershipActorProxy::CreateActor()
   {
      SetActor(*new EnemyMothershipActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMothershipActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

   void EnemyMothershipActorProxy::OnRemovedFromWorld()
   {
      EnemyMothershipActor& actor = static_cast<EnemyMothershipActor&>(GetGameActor());      
   }
} // namespace
