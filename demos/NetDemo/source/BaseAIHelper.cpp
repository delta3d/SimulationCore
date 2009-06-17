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

#include <dtUtil/log.h>

namespace NetDemo
{

   BaseAIHelper::BaseAIHelper()
   : mFactory(new dtAI::FSM::FactoryType())
   , mStateMachine(mFactory.get())
   {
     
   }

   BaseAIHelper::~BaseAIHelper()
   {

   }

   void BaseAIHelper::Init()
   {

   }

   void BaseAIHelper::PreSync(const Kinematic& ko)
   {
     if(mPhysicsModel.valid())
     {
       mPhysicsModel->SetKinematicState(ko);
     }
   }

   void BaseAIHelper::PostSync(Kinematic& ko) const
   {
     if(mPhysicsModel.valid())
     {
       ko = mPhysicsModel->GetKinematicState();
     }
   }

   void BaseAIHelper::Spawn()
   {
     mStateMachine.MakeCurrent(&AIState::AI_STATE_SPAWN);
     dtAI::NPCState* state = mStateMachine.GetState(&AIState::AI_STATE_SPAWN);
   }

   void BaseAIHelper::Update(float dt)
   {
      mStateMachine.Update(dt);
     
     //NOTE: the state machine update will call update on the current state
     //      but the output of that state must set the kinematic goal and the appropriate 
     //      steering behavior on the steering model

     if(mSteeringModel.valid() && mPhysicsModel.valid() && mSteeringModel->GetSteeringBehavoir() != NULL)
     {    
         mSteeringModel->Update(mPhysicsModel->GetKinematicState(), dt);
         mPhysicsModel->Update(mSteeringModel->GetOutput(), dt);
     }
   }


   void BaseAIHelper::AddTransition(const AIEvent* eventToTriggerTransition, const AIState* fromState, const AIState* toState)
   {
     mStateMachine.AddTransition(eventToTriggerTransition, fromState, toState);
   }

} //namespace NetDemo
