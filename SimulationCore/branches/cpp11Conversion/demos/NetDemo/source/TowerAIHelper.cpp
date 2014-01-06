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



#include <TowerAIHelper.h>
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

#include <dtUtil/stringutils.h>
#include <dtUtil/mathdefines.h>

#include <AIEvent.h>

namespace NetDemo
{

   TowerAIHelper::TowerAIHelper()
   {
     
   }

   TowerAIHelper::~TowerAIHelper()
   {

   }

   void TowerAIHelper::OnInit(const EnemyDescriptionActor* desc)
   {
      BaseClass::OnInit(desc);

      //mGoalState.SetMaxPitch(osg::DegreesToRadians(180.0f));
      //mGoalState.SetMaxAngularVel(0.01f);

      //float lookAheadRot = 0.25f;
      //float timeToTargetRot = 1.25f;

      //GetSteeringModel()->AddSteeringBehavior(new TowerAlign(lookAheadRot, timeToTargetRot));

      //mTurretAI.mCurrentState.SetCurrentAngle(osg::Vec2(osg::PI_2, osg::PI_2));
   }

   void TowerAIHelper::Spawn()
   {
     BaseClass::Spawn();
   }

   void TowerAIHelper::PreSync(const dtCore::Transform& trans)
   {
      BaseClass::PreSync(trans);

      mTurretAI.mCurrentState.SetOrigin(trans.GetTranslation());
   }

   void TowerAIHelper::Update(float dt)
   {
      BaseClass::Update(dt);
   }

   void TowerAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();
      GetStateFactory()->RegisterType<AttackState>(AIStateType::AI_STATE_ATTACK.GetName());
   }

   void TowerAIHelper::CreateStates()
   {
      BaseClass::CreateStates();

      GetStateMachine().AddState(&AIStateType::AI_STATE_FIND_TARGET);
      GetStateMachine().AddState(&AIStateType::AI_STATE_ATTACK);
      GetStateMachine().AddState(&AIStateType::AI_STATE_FIRE_LASER);
   }

   void TowerAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();

      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_FIRE_LASER, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_FIRE_LASER);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_FIND_TARGET);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_FIRE_LASER, &AIStateType::AI_STATE_FIND_TARGET);
      
      BaseClass::AddTransition(&AIEvent::AI_EVENT_NO_TARGET_FOUND, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_IDLE);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_FIND_TARGET);

    }

   void TowerAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &TowerAIHelper::Attack));
   }

   void TowerAIHelper::SelectState(float dt)
   {
      BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_FIND_TARGET);
   }

   void TowerAIHelper::Attack(float dt)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != nullptr)
      {
         if(attackState->mStateData.mTarget.valid())
         {
            dtCore::Transform xform;
            attackState->mStateData.mTarget->GetTransform(xform);
            osg::Vec3 pos = xform.GetTranslation();
            
            //mGoalState.SetPos(pos);
            //mDefaultTargeter->Push(pos);

            osg::Vec3 vel = pos - attackState->mStateData.mLastPos;
            
            osg::Vec3 predictedPos = pos + (vel * 2.5);

            mTurretAI.GetTargeter().Push(predictedPos);

            mTurretAI.StepTurret(dt);

            attackState->mStateData.mLastPos = pos;

            if(mTurretAI.mCurrentState.GetTrigger())
            {
               BaseClass::GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_FIRE_LASER);
            }
         }
         else
         {
            BaseClass::GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TARGET_KILLED);
            //LOG_ALWAYS("Target Killed");
         }
      }
   }

   void TowerAIHelper::SetCurrentTarget(const dtCore::Transformable& target)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != nullptr)
      {
         //LOG_ALWAYS("FOUND TARGET: " + target.GetName());
         attackState->mStateData.mTarget = &target;
         //let the system know we have targeted a new entity
         BaseClass::GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_ENEMY_TARGETED);
      }
      else
      {
         LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      }
   }

   float TowerAIHelper::GetAngle( const osg::Vec3& pos )
   {
      osg::Vec3 dir = pos - mCurrentState.GetPos();
      dir.normalize();

      osg::Vec3 forward = mCurrentState.GetForward();
      forward.normalize();

      float dot = forward * dir;
      //LOG_ALWAYS(dtUtil::ToString(dot));
      return dot;
   }

   const osg::Vec2& TowerAIHelper::GetWeaponAngle() const
   {
      return mTurretAI.mCurrentState.GetCurrentAngle();
   }

   void TowerAIHelper::SetWeaponAngle(const osg::Vec2& angle)
   {
      mTurretAI.mCurrentState.SetCurrentAngle(angle);
   }

   bool TowerAIHelper::GetTriggerState() const
   {
      return mTurretAI.mCurrentState.GetTrigger();
   }
} //namespace NetDemo

