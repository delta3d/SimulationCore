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



#include <EnemyAIHelper.h>
#include <AIEvent.h>
#include <AIState.h>

#include <dtUtil/log.h>

namespace NetDemo
{

   EnemyAIHelper::EnemyAIHelper()
   {
     
   }

   EnemyAIHelper::~EnemyAIHelper()
   {

   }

   void EnemyAIHelper::Init()
   {
      BaseClass::Init();
   }

   void EnemyAIHelper::Spawn()
   {
     BaseClass::Spawn();
   }

   void EnemyAIHelper::Update(float dt)
   {
      BaseClass::Update(dt);
   }

   void EnemyAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();

      GetStateFactory()->RegisterType<GoToWaypointState>(AIStateType::AI_STATE_GO_TO_WAYPOINT.GetName());
   }

   void EnemyAIHelper::CreateStates()
   {
      BaseClass::CreateStates();

      GetStateMachine().AddState(&AIStateType::AI_STATE_FIND_TARGET);
      GetStateMachine().AddState(&AIStateType::AI_STATE_GO_TO_WAYPOINT);
      GetStateMachine().AddState(&AIStateType::AI_STATE_ATTACK);
      GetStateMachine().AddState(&AIStateType::AI_STATE_EVADE);
      GetStateMachine().AddState(&AIStateType::AI_STATE_FOLLOW);
      GetStateMachine().AddState(&AIStateType::AI_STATE_FLOCK);
      GetStateMachine().AddState(&AIStateType::AI_STATE_WANDER);
      GetStateMachine().AddState(&AIStateType::AI_STATE_DETONATE);
   }

   void EnemyAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();

      BaseClass::AddTransition(&AIEvent::AI_EVENT_ARRIVED, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_GO_TO_WAYPOINT);

      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FOLLOW, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FLOCK, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_GO_TO_WAYPOINT);

      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_SIGHTED, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_SIGHTED, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_SIGHTED, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_ATTACK);

      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_FIND_TARGET);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_FIND_TARGET);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_FIND_TARGET);

      BaseClass::AddTransition(&AIEvent::AI_EVENT_DAMAGE_CRITICAL, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_EVADE);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_DAMAGE_CRITICAL, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_EVADE);
      
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_WANDER);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_WANDER);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_FIND_TARGET);

    }

   void EnemyAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();
   }

   void EnemyAIHelper::SelectState(float dt)
   {
      BaseClass::SelectState(dt);
   }


} //namespace NetDemo
