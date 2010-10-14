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
#include <AIUtility.h>
#include <Actors/EnemyDescriptionActor.h>

#include <dtAI/npcstate.h>
#include <dtAI/npcevent.h>

#include <dtCore/transformable.h>

#include <dtUtil/command.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/log.h>

#include <iostream>

namespace NetDemo
{

   EnemyAIHelper::EnemyAIHelper()
   {
     
   }

   EnemyAIHelper::~EnemyAIHelper()
   {

   }

   void EnemyAIHelper::OnInit(const EnemyDescriptionActor* desc)
   {
      BaseClass::OnInit(desc);
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
      GetStateFactory()->RegisterType<AttackState>(AIStateType::AI_STATE_ATTACK.GetName());
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

      //our next waypoint is calculated on the transition into the AI_STATE_GO_TO_WAYPOINT, so this transition will keep us moving on a path
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ARRIVED, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_GO_TO_WAYPOINT);

      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FOLLOW, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FLOCK, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_GO_TO_WAYPOINT);

      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_ATTACK);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_DIE, &AIStateType::AI_STATE_DIE);

      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_FIND_TARGET);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_FIND_TARGET);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_FIND_TARGET);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_DETONATE, &AIStateType::AI_STATE_IDLE);


      BaseClass::AddTransition(&AIEvent::AI_EVENT_DAMAGE_CRITICAL, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_EVADE);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_DAMAGE_CRITICAL, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_EVADE);
      
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_WANDER);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_WANDER);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_FIND_TARGET);
      BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_DIE, &AIStateType::AI_STATE_DIE);

    }

   void EnemyAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      //setting this will allow us to follow a path
      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_GO_TO_WAYPOINT);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::GoToWaypoint));
      //on entry into the waypoint state we will calculate the next waypoint to go to
      state->AddEntryCommand(new dtUtil::Command0<void>(dtUtil::Command0<void>::FunctorType(this, &EnemyAIHelper::CalculateNextWaypoint)));

      //these below all setup the states to call the default update
      state = GetStateMachine().GetState(&AIStateType::AI_STATE_DIE);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_IDLE);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_EVADE);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_FOLLOW);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_FLOCK);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_WANDER);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_DETONATE);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyAIHelper::DefaultStateUpdate));


      //this can be used to change steering behaviors when transitioning into a new state
      typedef dtUtil::Command1<void, dtCore::RefPtr<SteeringBehaviorType> > ChangeSteeringBehaviorCommand;
      typedef dtUtil::Functor<void, TYPELIST_1(dtCore::RefPtr<SteeringBehaviorType>)> ChangeSteeringBehaviorFunctor;    
  
      SteeringBehaviorType* behavior = new DoNothing();
      ChangeSteeringBehaviorCommand* ctbc = new ChangeSteeringBehaviorCommand(ChangeSteeringBehaviorFunctor(this, &EnemyAIHelper::ChangeSteeringBehavior), behavior);

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_DIE);
      state->AddEntryCommand(ctbc);

      state = GetStateMachine().GetState(&AIStateType::AI_STATE_IDLE);
      state->AddEntryCommand(ctbc);

   }

   void EnemyAIHelper::SelectState(float dt)
   {
      BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_FIND_TARGET);
   }

   void EnemyAIHelper::DefaultStateUpdate(float dt)
   {
   }

   void EnemyAIHelper::ChangeSteeringBehavior(dtCore::RefPtr<SteeringBehaviorType> newBehavior)
   {
      unsigned num = GetSteeringModel()->AddSteeringBehavior(newBehavior.get());
      GetSteeringModel()->SetCurrentSteeringBehavior(num);
   }

   void EnemyAIHelper::CalculateNextWaypoint()
   {
      //just do a bee line until we can work with a bezier node
      osg::Matrix mat;
      GetPhysicsModel()->GetState(mCurrentState, mat);

      osg::Vec3 forward = dtUtil::MatrixUtil::GetRow3(mat, 1);

      osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(mat, 3) + (forward * 50.0f);

      GoToWaypointState* waypointState = dynamic_cast<GoToWaypointState*>(BaseClass::GetStateMachine().GetState(&AIStateType::AI_STATE_GO_TO_WAYPOINT)); 
      if(waypointState != NULL)
      {
         waypointState->mStateData = pos;
      }
   }

   void EnemyAIHelper::GoToWaypoint(float dt)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      GoToWaypointState* waypointState = dynamic_cast<GoToWaypointState*>(npcState);
      if(waypointState != NULL)
      {
         osg::Vec3 pos = waypointState->mStateData.mCurrentWaypoint;

         mGoalState.SetPos(pos);
      }
      else
      {
         LOG_ERROR("Invalid state type for state 'AI_STATE_GO_TO_WAYPOINT'");
      }
   }


   void EnemyAIHelper::SetCurrentTarget(dtCore::Transformable& target)
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

   float EnemyAIHelper::GetDistance(const osg::Vec3& vec)
   {      
      return (vec - mCurrentState.GetPos()).length();
   }


} //namespace NetDemo
