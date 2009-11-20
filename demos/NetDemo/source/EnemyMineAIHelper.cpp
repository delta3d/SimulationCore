/* -*-c++-*-
* Using 'The MIT License'
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
* @author Bradley Anderegg
*/



#include <EnemyMineAIHelper.h>
#include <AIEvent.h>
#include <AIState.h>
#include <AIUtility.h>
#include <Actors/EnemyDescriptionActor.h>

#include <dtAI/npcstate.h>
#include <dtAI/npcevent.h>

#include <dtCore/transformable.h>

#include <dtUtil/command.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/log.h>

#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>

namespace NetDemo
{

   EnemyMineAIHelper::EnemyMineAIHelper()
      : mMaxVelocity(150.0f)
   {
     
   }

   EnemyMineAIHelper::~EnemyMineAIHelper()
   {

   }

   void EnemyMineAIHelper::OnInit(const EnemyDescriptionActor* desc)
   {
      BaseClass::OnInit(desc);
      
      if(desc != NULL)
      {
         mMaxVelocity = desc->GetSpawnInfo().GetMaxVelocity();
         
         //this sets our physical constraint
         mGoalState.SetMaxVel(mMaxVelocity);
      }

      GetSteeringModel()->AddSteeringBehavior(new BombDive(mMaxVelocity));
   }

   void EnemyMineAIHelper::Spawn()
   {
     BaseClass::Spawn();
   }

   void EnemyMineAIHelper::Update(float dt)
   {
      const float MAX_TICK = 0.1f;
      if(dt > MAX_TICK) dt = MAX_TICK;

      mStateMachine.Update(dt);
      mSteeringModel->Step(dt, *this);
      
      if(GetPhysicsModel()->GetPhysicsHelper() != NULL)
      {
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject();

         if(physicsObject != NULL)
         {
            osg::Vec3 at = mGoalState.GetPos() - mCurrentState.GetPos();
            at.normalize();

            float maxThrustForce = 10.0f;

            osg::Vec3 force;
            float mass = physicsObject->GetBodyWrapper()->GetMass();

            force += at * (mCurrentControls.GetThrust() * maxThrustForce);

            mCurrentState.SetVel(physicsObject->GetLinearVelocity());

            physicsObject->GetBodyWrapper()->ApplyImpulse(force);
         }
      }
   }

   void EnemyMineAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();
   }

   void EnemyMineAIHelper::CreateStates()
   {
      BaseClass::CreateStates();
   }

   void EnemyMineAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();
    }

   void EnemyMineAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      if(state != NULL)
      {
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyMineAIHelper::Attack));
      }

      ////this can be used to change steering behaviors when transitioning into a new state
      //typedef dtUtil::Command1<void, dtCore::RefPtr<SteeringBehaviorType> > ChangeSteeringBehaviorCommand;
      //typedef dtUtil::Functor<void, TYPELIST_1(dtCore::RefPtr<SteeringBehaviorType>)> ChangeSteeringBehaviorFunctor;
      //    
      //SteeringBehaviorType* behavior = new BombDive(mMaxVelocity);
      //ChangeSteeringBehaviorCommand* ctbc = new ChangeSteeringBehaviorCommand(ChangeSteeringBehaviorFunctor(this, &EnemyAIHelper::ChangeSteeringBehavior), behavior);
      
      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      //state->AddEntryCommand(ctbc);   
   }


   void EnemyMineAIHelper::Attack(float dt)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != NULL && attackState->mStateData.mTarget.valid())
      {
         dtCore::Transform xform;
         attackState->mStateData.mTarget->GetTransform(xform);
         osg::Vec3 pos = xform.GetTranslation();

         //if we are within distance, detonate
         //this is only for the enemy mine, and should be refactored
         float dist = GetDistance(pos);
         if(dist < 25.0f)
         {
            BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DETONATE);
            return;
         }
         
         mGoalState.SetPos(pos);
         mDefaultTargeter->Push(pos);
      }
      else
      {
         LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      }
   }

} //namespace NetDemo
