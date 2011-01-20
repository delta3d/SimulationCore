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
#include <EnemyHelixAIHelper.h>
#include <AIState.h>
#include <AIEvent.h>
#include <Actors/EnemyDescriptionActor.h>

#include <dtCore/transform.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>

#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>

#include <osg/Vec2>
#include <osg/Vec3>

namespace NetDemo
{

   //////////////////////////////////////////////////////////////////////////
   //EnemyHelixAIHelper
   //////////////////////////////////////////////////////////////////////////
   EnemyHelixAIHelper::EnemyHelixAIHelper()
      : mTargetOffset(0.0f, 0.0f, 5.0f)
   {

   }

   //////////////////////////////////////////////////////////////////////////
   EnemyHelixAIHelper::~EnemyHelixAIHelper()
   {

   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::OnInit(const EnemyDescriptionActor* desc)
   {
      BaseClass::OnInit(desc);

      float minSpeedPercent = 0.0f;
      float maxSpeedPercent = 2.5f;
      float lookAheadTime = 2.0f;
      float timeToTarget = 10.0f;
      float timeToTargetHeight = 25.0f;
      float lookAheadRot = 2.5f;
      float timeToTargetRot = 1.0f;

      GetSteeringModel()->AddSteeringBehavior(new FollowPath(minSpeedPercent, maxSpeedPercent, lookAheadTime, timeToTarget, timeToTargetHeight, lookAheadRot, timeToTargetRot));

      ComputeTargetOffset();
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::Spawn()
   {
      BaseClass::Spawn();
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::ComputeTargetOffset()
   {
      mTargetOffset = osg::Vec3(
         dtUtil::RandFloat(0.0f, 0.0f),
         dtUtil::RandFloat(0.0f, 0.0f),
         dtUtil::RandFloat(10.0f, 15.0f) );
   }


   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::Update(float dt)
   {
      BaseClass::Update(dt);
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::PreSync(const dtCore::Transform& trans)
   {
      //updates the state of the physics model
      BaseClass::PreSync(trans);
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::PostSync(dtCore::Transform& trans) const
   {
      BaseClass::PostSync(trans);
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::CreateStates()
   {
      BaseClass::CreateStates();
      
      //GetStateMachine().AddState(&AIStateType::AI_STATE_FIRE_LASER);
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();

      //BaseClass::AddTransition(&AIEvent::AI_EVENT_FIRE_LASER, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_FIRE_LASER);

   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      if(state != NULL)
      {
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyHelixAIHelper::Attack));
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::SelectState(float dt)
   {
      BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_FIND_TARGET);
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::Attack(float dt)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != NULL && attackState->mStateData.mTarget.valid())
      {
         dtCore::Transform xform;
         attackState->mStateData.mTarget->GetTransform(xform);
         osg::Vec3 pos = xform.GetTranslation();

         pos += mTargetOffset;
         mDefaultTargeter->Push(pos);
         mGoalState.SetPos(pos);

         //osg::Vec3 vel = pos - attackState->mStateData.mLastPos;
         //osg::Vec3 predictedPos = pos + (vel * 2.5);
         mTurretAI.mCurrentState.SetOrigin(mCurrentState.GetPos());
         mTurretAI.GetTargeter().Push(pos);
         mTurretAI.StepTurret(dt);
         //attackState->mStateData.mLastPos = pos;

         //if(mTurretAI.mCurrentState.GetTrigger())
         //{
         //   BaseClass::GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_FIRE_LASER);
         //}

         /*if(GetDistance(pos) < 20.0f)
         {
            ComputeTargetOffset();
         }*/
      }
      else
      {
         LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::SetCurrentTarget(dtCore::Transformable& target)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != NULL)
      {
         attackState->mStateData.mTarget = &target;
         //let the system know we have targeted a new entity
         BaseClass::GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_ENEMY_TARGETED);
      }
      else
      {
         LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      }
   }


   //////////////////////////////////////////////////////////////////////////
   const osg::Vec2& EnemyHelixAIHelper::GetWeaponAngle() const
   {
      return mTurretAI.mCurrentState.GetCurrentAngle();
   }

   //////////////////////////////////////////////////////////////////////////
   void EnemyHelixAIHelper::SetWeaponAngle(const osg::Vec2& angle)
   {
      mTurretAI.mCurrentState.SetCurrentAngle(angle);
   }

   //////////////////////////////////////////////////////////////////////////
   bool EnemyHelixAIHelper::GetTriggerState() const
   {
      return mTurretAI.mCurrentState.GetTrigger();
   }

}//namespace NetDemo
