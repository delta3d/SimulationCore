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



#include <BaseAIHelper.h>
#include <AIEvent.h>
#include <AIState.h>

#include <dtAI/npcstate.h>
#include <dtUtil/log.h>

namespace NetDemo
{

   BaseAIHelper::BaseAIHelper()
   : mFactory(new dtAI::FSM::FactoryType())
   , mStateMachine(mFactory.get())
   , mSteeringModel(new AISteeringModel())
   , mPhysicsModel(new AIPhysicsModel())
   {
     
   }

   BaseAIHelper::~BaseAIHelper()
   {

   }

   void BaseAIHelper::Init()
   {
      RegisterStates();
      CreateStates();
      SetupTransitions();
      SetupFunctors();
   }


   void BaseAIHelper::PreSync(const dtCore::Transform& trans)
   {
     if(mPhysicsModel.valid())
     {
       Kinematic k = mPhysicsModel->GetKinematicState();
       trans.Get(k.mTransform);

       mPhysicsModel->SetKinematicState(k);
     }
   }

   void BaseAIHelper::PostSync(dtCore::Transform& trans) const
   {
     if(mPhysicsModel.valid())
     {
       const Kinematic& k = mPhysicsModel->GetKinematicState();
       trans.Set(k.mTransform);
     }
   }

   void BaseAIHelper::Spawn()
   {
     mStateMachine.MakeCurrent(&AIStateType::AI_STATE_SPAWN);
   }

   void BaseAIHelper::Update(float dt)
   {
      mStateMachine.Update(dt);
     
     if(mSteeringModel.valid() && mPhysicsModel.valid())
     {    
         mSteeringModel->Update(mPhysicsModel->GetKinematicState(), dt);
         mPhysicsModel->Update(mSteeringModel->GetOutput(), dt);
     }
   }

   void BaseAIHelper::RegisterStates()
   {
   }

   void BaseAIHelper::CreateStates()
   {
      mStateMachine.AddState(&AIStateType::AI_STATE_SPAWN);
      mStateMachine.AddState(&AIStateType::AI_STATE_DIE);
      mStateMachine.AddState(&AIStateType::AI_STATE_IDLE);
   }

   void BaseAIHelper::SetupTransitions()
   {
   }

   void BaseAIHelper::SetupFunctors()
   {
      //on spawn we will call the virtual select state to allow derivatives to choose a default state
      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_SPAWN);
      state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &BaseAIHelper::SelectState));
   }

   void BaseAIHelper::SelectState(float dt)
   {
      //by default we will go into idle
      mStateMachine.MakeCurrent(&AIStateType::AI_STATE_IDLE);
   }

   void BaseAIHelper::AddTransition(const AIEvent* eventToTriggerTransition, const AIStateType* fromState, const AIStateType* toState)
   {
     mStateMachine.AddTransition(eventToTriggerTransition, fromState, toState);
   }

} //namespace NetDemo
