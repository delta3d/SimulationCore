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
#include <Actors/EnemyDescriptionActor.h>

#include <dtAI/npcstate.h>
#include <dtUtil/log.h>

//these headers should be refactored out of here
#include <dtGame/gameactor.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>

namespace NetDemo
{

   BaseAIHelper::BaseAIHelper()
      : mFactory(new dtAI::FSM::FactoryType())
      , mStateMachine(mFactory.get())
      , mSteeringModel(new AISteeringModel())
      , mPhysicsModel(new AIPhysicsModel())
      , mDefaultTargeter(new BaseSteeringTargeter())
   {
      mTargeters.push_back(mDefaultTargeter);
   }

   BaseAIHelper::~BaseAIHelper()
   {

   }

   void BaseAIHelper::Init(const EnemyDescriptionActor* desc)
   {
      RegisterStates();
      CreateStates();
      SetupTransitions();
      SetupFunctors();

      mSteeringModel->Init();
      mPhysicsModel->Init();

      OnInit(desc);
   }

   void BaseAIHelper::OnInit(const EnemyDescriptionActor* desc)
   {
      osg::Matrix mat;
      mat.makeIdentity();

      if(desc != NULL)
      {
         dtCore::Transform xform;
         desc->GetTransform(xform);
         xform.Get(mat);

         mPhysicsModel->SetState(mCurrentState, mat);
         mPhysicsModel->SetState(mGoalState, mat);
         
         //our goal state actually derives the game state so we call both
         mPhysicsModel->SetDefaultState(mat, mGoalState);
         mPhysicsModel->SetDefaultConstraints(mGoalState);
      }
      else
      {
         mPhysicsModel->SetDefaultState(mat, mCurrentState);
         
         //our goal state actually derives the game state so we call both
         mPhysicsModel->SetDefaultState(mat, mGoalState);
         mPhysicsModel->SetDefaultConstraints(mGoalState);
      }
   }


   void BaseAIHelper::PreSync(const dtCore::Transform& trans)
   {
      if(mPhysicsModel.valid())
      {
         //update the position and orientation
         osg::Matrix mat;
         trans.Get(mat);
         mPhysicsModel->SetState(mCurrentState, mat);

         //update the linear and angular velocities
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsModel()->GetPhysicsActComp()->GetMainPhysicsObject();
         mCurrentState.SetVel(physicsObject->GetLinearVelocity());
         //mCurrentState.SetAngularVel(physicsObject->GetAngularVelocity());
      }
   }

   void BaseAIHelper::PostSync(dtCore::Transform& trans) const
   {
      if(mPhysicsModel.valid())
      {
         osg::Matrix mat;
         mPhysicsModel->GetState(mCurrentState, mat);
         
         //we are currently only updating the rotation
         trans.SetRotation(mat);

         //check to make sure we haven't exceeded our max speed
         osg::Vec3 suggestedPos;
         trans.GetTranslation(suggestedPos);

         float dist = (mCurrentState.GetPos() - suggestedPos).length();
         if(dist > (mGoalState.GetMaxVel() * mPhysicsModel->GetCurrentTimeStep()))
         {
            //we could try to move you as far as you can go but we may cause a collision
            //instead we will have to just stay put, investigate a collision query or second ai try (tick)
            trans.SetTranslation(mCurrentState.GetPos());
            
            //LOG_ALWAYS("Clamping entity range");
            //we changed the actual position, we had better notify physics
            if(mPhysicsModel->GetPhysicsActComp() != NULL && mPhysicsModel->GetPhysicsActComp()->GetMainPhysicsObject() != NULL)
            {
               dtPhysics::PhysicsObject* physicsObject = mPhysicsModel->GetPhysicsActComp()->GetMainPhysicsObject();
               physicsObject->SetTransform(trans);

               physicsObject->GetBodyWrapper()->ResetForces();
            }
            
         }
      }
   }

   void BaseAIHelper::Spawn()
   {
      mStateMachine.MakeCurrent(&AIStateType::AI_STATE_SPAWN);
   }

   void BaseAIHelper::Update(float dt)
   {
      const float MAX_TICK = 0.1f;
      if(dt > MAX_TICK) dt = MAX_TICK;

      mStateMachine.Update(dt);
      mSteeringModel->Step(dt, *this);
      mPhysicsModel->Update(dt, *this);
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

   bool BaseAIHelper::FindPath(const BaseClass::AIState& fromState, const BaseClass::AIGoal& goal, BaseClass::AIPath& resultingPath) const
   {
      resultingPath.push_back(goal);
      return true;
   }
   
   void BaseAIHelper::OutputControl(const BaseClass::AIPath& pathToFollow, const BaseClass::AIState& current_state, BaseClass::AIControlState& result) const
   {
      mSteeringModel->OutputControl(pathToFollow, current_state, result);
   }

   void BaseAIHelper::UpdateState(float dt, const BaseClass::AIControlState& steerData)
   {
   }

   void BaseAIHelper::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      BaseClass::RegisterProperties(pc, group);
   }

   void BaseAIHelper::GetTransform( dtCore::Transform& transIn ) const
   {
      osg::Matrix mat;
      mPhysicsModel->GetState(mCurrentState, mat);

      transIn.Set(mat);
   }

   void BaseAIHelper::SetTransform( const dtCore::Transform& trans )
   {
      osg::Matrix mat;
      trans.Get(mat);

      mPhysicsModel->SetState(mCurrentState, mat);
      //if(mPhysicsModel->GetPhysicsActComp() != NULL
      //   && mPhysicsModel->GetPhysicsActComp()->GetMainPhysicsObject() != NULL)
      //{
      //   mPhysicsModel->GetPhysicsActComp()->GetMainPhysicsObject()->SetTransform(trans);
      //}
   }

} //namespace NetDemo
