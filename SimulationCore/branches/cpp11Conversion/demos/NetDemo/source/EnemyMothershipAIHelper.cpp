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
#include <EnemyMothershipAIHelper.h>
#include <AIState.h>
#include <AIEvent.h>
#include <Actors/EnemyDescriptionActor.h>

#include <dtCore/transform.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>

#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtPhysics/physicsobject.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/bodywrapper.h>

#include <osg/Vec2>
#include <osg/Vec3>

namespace NetDemo
{
   //////////////////////////////////////////////////////////////////////////
   //EnemyMothershipAIHelper
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipAIHelper::EnemyMothershipAIHelper()
   {  
      ComputeTargetOffset();
   }

   EnemyMothershipAIHelper::~EnemyMothershipAIHelper()
   {

   }

   void EnemyMothershipAIHelper::ComputeTargetOffset()
   {
      static bool side = true;
      side = !side;

      mTargetOffset = osg::Vec3(
         dtUtil::RandFloat(15.0f, 25.0f),
         dtUtil::RandFloat(90.0f, 170.0f),
         dtUtil::RandFloat(50.0f, 100.0f) );

      if(side) mTargetOffset[0] = -mTargetOffset[0];
   }

   void EnemyMothershipAIHelper::OnInit(const EnemyDescriptionActor* desc)
   {
      BaseClass::OnInit(desc);

      float minSpeedPercent = 0.0f;
      float maxSpeedPercent = 0.8f;
      float lookAheadTime = 10.0f;
      float timeToTarget = 35.0f;
      float timeToTargetHeight = 20.0f;
      float lookAheadRot = 1.5f;
      float timeToTargetRot = 1.0f;

      GetSteeringModel()->AddSteeringBehavior(new FollowPath(minSpeedPercent, maxSpeedPercent, lookAheadTime, timeToTarget, timeToTargetHeight, lookAheadRot, timeToTargetRot));

   }

   void EnemyMothershipAIHelper::Spawn()
   {
      BaseClass::Spawn();
   }

   void EnemyMothershipAIHelper::Update(float dt)
   {
      BaseClass::Update(dt);
   }

   void EnemyMothershipAIHelper::PreSync(const dtCore::Transform& trans)
   {
      //updates the state of the physics model
      BaseClass::PreSync(trans);

   }

   void EnemyMothershipAIHelper::PostSync(dtCore::Transform& trans) const
   {
      BaseClass::PostSync(trans);
   }


   void EnemyMothershipAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();
   }

   void EnemyMothershipAIHelper::CreateStates()
   {
      BaseClass::CreateStates();
   }

   void EnemyMothershipAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();
   }

   void EnemyMothershipAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      if(state != nullptr)
      {
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyMothershipAIHelper::Attack));
      }
   }

   void EnemyMothershipAIHelper::SelectState(float dt)
   {
      BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_FIND_TARGET);
   }

   void EnemyMothershipAIHelper::Attack(float dt)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != nullptr && attackState->mStateData.mTarget.valid())
      {
         dtCore::Transform xform;
         attackState->mStateData.mTarget->GetTransform(xform);
         osg::Vec3 pos = xform.GetTranslation();

         pos += mTargetOffset;
         mDefaultTargeter->Push(pos);

         if(GetDistance(pos) < 50.0f)
         {
            ComputeTargetOffset();
         }
      }
      else
      {
         LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      }
   }

   void EnemyMothershipAIHelper::SetCurrentTarget(dtCore::Transformable& target)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != nullptr)
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

}//namespace NetDemo
