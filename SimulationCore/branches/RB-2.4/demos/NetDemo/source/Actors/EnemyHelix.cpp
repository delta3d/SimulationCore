/* Net Demo - EnemyHelix (.cpp & .h) - Using 'The MIT License'
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
#include <Actors/EnemyHelix.h>

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

#include <AISpaceShip.h> 
//all below are included from the above- #include <AISpaceShip.h> 
//#include <EnemyAIHelper.h>
//#include <AIUtility.h>
#include <AIEvent.h>
#include <AIState.h>

#include <ActorRegistry.h>
#include <Actors/FortActor.h>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyHelixActor::EnemyHelixActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : BaseEnemyActor(proxy)
   {
      mAIHelper = new SpaceShipAIHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyHelixActor::~EnemyHelixActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      if (!IsRemote()) //only run locally
      {
         //calling init on the AIHelper will setup the states and transitions
         //note: init is now called by the spawn component
         //mAIHelper->Init();

         //this will allow the AI to actually move us
         mAIHelper->GetPhysicsModel()->SetPhysicsHelper(GetPhysicsHelper());
         
         //redirecting the find target function
         dtAI::NPCState* state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyHelixActor::FindTarget));

         //redirecting the detonate function
         state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_DETONATE);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyHelixActor::DoExplosion));

         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_DIE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FOLLOW, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FLOCK, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_DETONATE);

         
         //calling spawn will start the AI
         mAIHelper->Spawn();
      }
   }

   void EnemyHelixActor::FindTarget(float)
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
   void EnemyHelixActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      //update the entities orientation
      //dtCore::Transform trans;
      //GetTransform(trans);

      //mAIHelper->PostSync(trans);
      //SetTransform(trans);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::PostPhysicsUpdate()
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
               
               //apply our own rotation
               mAIHelper->PostSync(currentXForm);
               
               SetTransform(currentXForm);
               SetPushTransformToPhysics(false);
            }
         }
      }
   }


   //////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
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


   ///////////////////////////////////////////////////////////////////////////////////
   //TODO- MAKE THIS A HELPER FUNCTION OR BASE, COPIED FROM ENEMYMINE.CPP
   void EnemyHelixActor::DoExplosion(float)
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
      msg->SetDetonationResultCode( 3 ); // TO BE DYNAMIC
      msg->SetMunitionType("Generic Explosive");
      msg->SetFuseType(0);
      msg->SetWarheadType(0);
      msg->SetQuantityFired(1);
      msg->SetSendingActorId(GetGameActorProxy().GetId());
      //msg->SetFinalVelocityVector( finalVelocity );
      msg->SetRateOfFire(1);

      gm->SendMessage( *msg );
      gm->SendNetworkMessage( *msg );

      GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::RespondToHit(const SimCore::DetonationMessage& message,
      const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force, 
      const osg::Vec3& location)
   {
      // the base class applies an impulse
      BaseClass::RespondToHit(message, munition, force, location);

      if(IsMobilityDisabled())
      {
         mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DIE);
      }

      if(GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
      {       
         //this lets the AI respond to being hit
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TOOK_DAMAGE);
      }
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   EnemyHelixActorProxy::EnemyHelixActorProxy()
   {
      SetClassName("EnemyHelixActor");
  }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActorProxy::BuildPropertyMap()
   {
      const std::string GROUP = "Enemy Props";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      EnemyHelixActor& actor = static_cast<EnemyHelixActor &>(GetGameActor());

   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyHelixActorProxy::~EnemyHelixActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActorProxy::CreateActor()
   {
      SetActor(*new EnemyHelixActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

   void EnemyHelixActorProxy::OnRemovedFromWorld()
   {
      EnemyHelixActor& actor = static_cast<EnemyHelixActor&>(GetGameActor());      
   }
} // namespace
