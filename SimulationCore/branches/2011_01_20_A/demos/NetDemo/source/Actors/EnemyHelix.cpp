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
#include <iostream>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
//#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/DefaultFlexibleArticulationHelper.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/CollisionGroupEnum.h>

#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palphysicsworld.h>

#include <dtUtil/nodecollector.h>

#include <EnemyHelixAIHelper.h> 
//all below are included from the above- #include <AISpaceShip.h> 
//#include <EnemyAIHelper.h>
//#include <AIUtility.h>
#include <AIEvent.h>
#include <AIState.h>
#include <Actors/TowerActor.h>

#include <ActorRegistry.h>
#include <Actors/FortActor.h>
#include <Actors/FireBallActor.h>

#include <Components/WeaponComponent.h>

#include <dtPhysics/physicshelper.h>

#include <osgSim/DOFTransform>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyHelixActor::EnemyHelixActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : BaseEnemyActor(proxy)
   {
      mAIHelper = new EnemyHelixAIHelper();
      
      SetTimeToExistAfterDead(4.0f);
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
         mAIHelper->GetPhysicsModel()->SetPhysicsActComp(GetPhysicsActComp());
         
         //redirecting the find target function
         dtAI::NPCState* state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyHelixActor::FindTarget));

         //redirecting the detonate function
         state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_DETONATE);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyHelixActor::DoExplosion));

         //redirecting the shoot 
         //state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIRE_LASER);
         //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyHelixActor::Shoot));


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

         //creates the weapon actor
         InitWeapon();

      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::OnRemovedFromWorld()
   {
      if(mWeaponProxy.valid())
      {
         GetGameActorProxy().GetGameManager()->DeleteActor(*mWeaponProxy);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::InitWeapon()
   {
      // Get the weapon component that is used to create weapons.
      WeaponComponent* weaponComp = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(WeaponComponent::DEFAULT_NAME, weaponComp);

      if(weaponComp != NULL)
      {
         SimCore::Actors::WeaponActor* weapon = NULL;

         // Create the primary weapon.
         // --- This method will automatically add the created weapon to the world.
         weaponComp->CreateWeapon("Weapon_MachineGun",
            "Particle_System_Weapon_GunWithTracer",
            "weapon_gun_flash.osg", weapon);

         // Customize the new weapon.
         if(weapon != NULL)
         {
            // Attach the weapon to this object.
            weapon->SetOwner(&GetGameActorProxy());
            AddChild(weapon, "dof_hotspot_01");

            // Maintain references to the weapon and its proxy.
            mWeapon = weapon;
            mWeaponProxy = static_cast<SimCore::Actors::WeaponActorProxy*>(&weapon->GetGameActorProxy());

            //slow down the rate of fire
            mWeapon->SetFireRate(0.65f);
         }
      }
      else
      {
         LOG_ERROR("Could not find Weapon Component to create weapon.");
      }
   }

   //////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::FindTarget(float)
   {
      //randomly choose a tower to attack
      int attackTower = dtUtil::RandRange(0, 3);

      if(attackTower > 1)
      {
         dtCore::Transformable* t = GetClosestTower();
         if(t != NULL)
         {
            mTarget = t;
            mAIHelper->SetCurrentTarget(*t);
            return;
         }
      }

      FortActor* fort = GetCurrentFortUnderAttack();
      if(fort != NULL)
      {
         mTarget = fort;
         mAIHelper->SetCurrentTarget(*fort);
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

      BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::PostPhysicsUpdate()
   {
      // Mostly copied from BasePhysicsVehicleActor - we do NOT want want our vehicle to 'roll', so we
      // take the position and throw away the rotation.

      // This is ONLY called if we are LOCAL (we put the check here just in case... )
      if (!IsRemote() && GetPhysicsActComp() != NULL)
      {
         // The base behavior is that we want to pull the translation and rotation off the object
         // in our physics scene and apply it to our 3D object in the visual scene.
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsActComp()->GetMainPhysicsObject();

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
   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActor::Shoot(float timeLeft)
   {
      if(mWeapon.valid())
      {

         //TODO- THIS DOESNT WORK, HOW DO WE ORIENT THE LASER
         dtUtil::NodeCollector* nodes = GetNodeCollector();
         osgSim::DOFTransform* dof = nodes->GetDOFTransform("dof_hotspot_01");
         if (dof != NULL)
         {
            mTimeSinceLastFire = 0.0f;

            //create a fireball actor
            dtCore::RefPtr<FireBallActorProxy> proxy;
            GetGameActorProxy().GetGameManager()->CreateActor(*NetDemoActorRegistry::FIREBALL_ACTOR_TYPE, proxy);
            if(proxy.valid())
            {
               FireBallActor* fireball = NULL;
               proxy->GetActor(fireball);
               if(fireball != NULL)
               {

                  float fireBallSpeed = 37.0f;

                  dtCore::Transform xform;
                  osg::Vec3 pos, enemyPos; 
                  GetTransform(xform);
                  xform.GetTranslation(pos);

                  osg::Vec3 dir;
                  xform.GetRow(1, dir);
                  dir.normalize();

                  fireball->SetVelocity(fireBallSpeed);
                  fireball->SetMaxTime(2.5f);
                  fireball->SetPosition(pos + (dir * 10.5f));

                  dtPhysics::PhysicsActComp* physAC = NULL;
                  GetComponent(physAC);
                  if (physAC != NULL && physAC->GetMainPhysicsObject() != NULL)
                  {
                     fireball->AddForce(physAC->GetMainPhysicsObject()->GetLinearVelocity());
                  }

                  if (mTarget.valid())
                  {
                     fireball->SetTarget(*mTarget);
                  }

                  GetGameActorProxy().GetGameManager()->AddActor(*proxy, false, true);
               }
            }
         }         
         //mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_ATTACK);
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


      EnemyHelixAIHelper* helix = dynamic_cast<EnemyHelixAIHelper*>(mAIHelper.get());
      if(mTimeSinceLastFire > 1.866f &&
         helix != NULL && helix->GetTriggerState())
      {
         Shoot(mTimeSinceLastFire);
      }
      else
      {
         osg::Vec3 angleToTarget = mAIHelper->mGoalState.GetPos() - mAIHelper->mCurrentState.GetPos();
         angleToTarget.normalize();
         float angle = angleToTarget * mAIHelper->mCurrentState.GetForward();
         if(mTimeSinceLastFire > 1.36f && angle > 0.975f)
         {
            Shoot(mTimeSinceLastFire);
         }
      }

      //randomly switch targets
      int switchTarget = dtUtil::RandRange(0, 500);
      if(switchTarget == 0)
      {
         FindTarget(0.0f);
      }
      

      BaseClass::OnTickLocal(tickMessage);

      mTimeSinceLastFire += tickMessage.GetDeltaSimTime();
      mWeapon->SetTriggerHeld(false);
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
      BaseClass::BuildPropertyMap();

      const std::string GROUP = "Enemy Props";

      //EnemyHelixActor& actor = static_cast<EnemyHelixActor &>(GetGameActor());

   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyHelixActorProxy::~EnemyHelixActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyHelixActorProxy::CreateActor()
   {
      SetActor(*new EnemyHelixActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////

} // namespace
