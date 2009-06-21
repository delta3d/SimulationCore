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
#include <AISpaceShip.h>
#include <AIState.h>
#include <AIEvent.h>

#include <dtCore/transform.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <osg/Vec2>
#include <osg/Vec3>

namespace NetDemo
{

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipControllable
   //////////////////////////////////////////////////////////////////////////
   SpaceShipControllable::SpaceShipControllable()
   {
   }

   SpaceShipControllable::~SpaceShipControllable()
   {
   }

   void SpaceShipControllable::InitControllable(const osg::Matrix& matIn, SpaceShipControllable& stateIn)
   {
      stateIn.mCurrentControls.mYaw = 0.0;
      stateIn.mCurrentControls.mThrust = 0.0;
      stateIn.mCurrentControls.mLift = 0.0;

      SpaceShipGoalState& goalStateConstraint = stateIn.mGoalState;
      goalStateConstraint.mMaxAngularVel = osg::DegreesToRadians(36.0f);
      goalStateConstraint.mMaxAngularAccel = 200.0f * osg::DegreesToRadians(6.0f);
      goalStateConstraint.mMaxVel = 77.1667f; //150 knots
      goalStateConstraint.mMaxAccel = 200.0f * 8.77f;
      goalStateConstraint.mMaxPitch = osg::DegreesToRadians(15.0f);
      goalStateConstraint.mMaxRoll = osg::DegreesToRadians(30.0f);
      goalStateConstraint.mMaxTiltPerSecond = osg::DegreesToRadians(5.0f);
      goalStateConstraint.mMaxRollPerSecond = osg::DegreesToRadians(5.0f);
      goalStateConstraint.mMaxVerticalVel = 15.0f;//7.62f; //1500 feet/min
      goalStateConstraint.mMaxVerticalAccel = 50.0f;

      goalStateConstraint.mMinElevation = 40.0f;
      goalStateConstraint.mMaxElevation = 80.0f;
      goalStateConstraint.mDragCoef = 0.005f;
      goalStateConstraint.mAngularDragCoef = 0.005f;
      goalStateConstraint.mVerticalDragCoef = 0.005f;

      SetState(matIn, stateIn.mCurrentState);
      SetState(matIn, stateIn.mGoalState);
   }

   void SpaceShipControllable::ResetState(const osg::Matrix& matIn, SpaceShipState& spaceShipState)
   {
      spaceShipState.mVel.set(0.0f, 0.0f, 0.0f);
      spaceShipState.mAccel.set(0.0f, 0.0f, 0.0f);
      spaceShipState.mPitch = 0.0f;
      spaceShipState.mRoll = 0.0f;
      spaceShipState.mAngularAccel = 0.0f;
      spaceShipState.mAngularVel = 0.0f;
      spaceShipState.mVerticalVel = 0.0f;
      spaceShipState.mVerticalAccel = 0.0f;

      spaceShipState.mForward = dtUtil::MatrixUtil::GetRow3(matIn, 1);
      spaceShipState.mForward.normalize();

      spaceShipState.mUp = dtUtil::MatrixUtil::GetRow3(matIn, 2);
      spaceShipState.mUp.normalize();

      spaceShipState.mPos[0] = matIn(3,0); spaceShipState.mPos[1] = matIn(3,1); spaceShipState.mPos[2] = matIn(3,2);

      spaceShipState.mTimeStep = 0.0f;
      spaceShipState.mVel.set(0.0f, 0.0f, 0.0f);
   }

   void SpaceShipControllable::SetState(const osg::Matrix& matIn, SpaceShipState& spaceShipState)
   {
      spaceShipState.mForward = dtUtil::MatrixUtil::GetRow3(matIn, 1);
      spaceShipState.mForward.normalize();

      spaceShipState.mUp = dtUtil::MatrixUtil::GetRow3(matIn, 2);
      spaceShipState.mUp.normalize();

      spaceShipState.mPos[0] = matIn(3,0); spaceShipState.mPos[1] = matIn(3,1); spaceShipState.mPos[2] = matIn(3,2);
   }

   void SpaceShipControllable::OrthoNormalize(SpaceShipState& currentState)
   {
      osg::Vec3 tempUpVector(0.0f, 0.0f, 1.0f);

      currentState.mForward.normalize();

      osg::Vec3 rightVector = currentState.mForward ^ tempUpVector;
      rightVector.normalize();

      currentState.mUp = rightVector ^ currentState.mForward;
      currentState.mUp.normalize();
   }

   void SpaceShipControllable::SetMatrix(const SpaceShipState& currentState, osg::Matrix& result)
   {
      result.makeIdentity();

      result(3,0) = currentState.mPos[0];
      result(3,1) = currentState.mPos[1];
      result(3,2) = currentState.mPos[2];

      osg::Vec3 rightVector = currentState.mForward ^ currentState.mUp;
      rightVector.normalize();

      dtUtil::MatrixUtil::SetRow(result, currentState.mForward, 1);
      dtUtil::MatrixUtil::SetRow(result, currentState.mUp, 2);
      dtUtil::MatrixUtil::SetRow(result, rightVector, 0);
   }



   //////////////////////////////////////////////////////////////////////////
   //SpaceShipTargeter
   //////////////////////////////////////////////////////////////////////////
   SpaceShipTargeter::SpaceShipTargeter()
   {
   }

   SpaceShipTargeter::~SpaceShipTargeter()
   {
   }

   bool SpaceShipTargeter::GetGoal(const SpaceShipState& current_state, SpaceShipGoalState& result) const
   {
      return true;
   }

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipDecomposer
   //////////////////////////////////////////////////////////////////////////
   SpaceShipDecomposer::SpaceShipDecomposer()
   {
   }

   SpaceShipDecomposer::~SpaceShipDecomposer()
   {
   }

   void SpaceShipDecomposer::Decompose(const SpaceShipState& current_state, SpaceShipGoalState& result) const
   {
   }

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipConstraint
   //////////////////////////////////////////////////////////////////////////
   SpaceShipConstraint::SpaceShipConstraint()
   {
   }

   SpaceShipConstraint::~SpaceShipConstraint()
   {
   }

   bool SpaceShipConstraint::WillViolate(const BaseClass::PathType& pathToFollow)
   {
      return false;
   }

   void SpaceShipConstraint::Suggest(const BaseClass::PathType& pathToFollow, const SpaceShipState& current_state, SpaceShipGoalState& result)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipActuator
   //////////////////////////////////////////////////////////////////////////
   SpaceShipActuator::SpaceShipActuator()
   {
   }

   SpaceShipActuator::~SpaceShipActuator()
   {
   }

   bool SpaceShipActuator::GetPath(const SpaceShipState& current_state, const SpaceShipGoalState& goal, BaseClass::PathType& result)
   {
      return true;
   }

   void SpaceShipActuator::Output(const BaseClass::PathType& pathToFollow, const SpaceShipState& current_state, SpaceShipControls& result)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipUpdater
   //////////////////////////////////////////////////////////////////////////
   SpaceShipUpdater::SpaceShipUpdater()
   {
   }

   SpaceShipUpdater::~SpaceShipUpdater()
   {
   }



   void SpaceShipUpdater::Reset(const osg::Matrix& mat, SpaceShipControllable& stateIn)
   {
      SpaceShipControllable::InitControllable(mat, stateIn);
   }


   void SpaceShipUpdater::Update(float dt, const SpaceShipControls& steeringOut, SpaceShipControllable::BaseClass& stateIn)
   {
      stateIn.mCurrentState.mTimeStep = dt;

      osg::Vec3 tempUpVector(0.0f, 0.0f, 1.0f);

      UpdateTilt(steeringOut, tempUpVector, stateIn);
      UpdateVelocity(steeringOut, stateIn);

      UpdateRoll(steeringOut, tempUpVector, stateIn);
      UpdateAngularVelocity(steeringOut, stateIn);

      UpdateVerticalVelocity(steeringOut, stateIn);

      UpdateHeading(steeringOut, stateIn);
      UpdatePosition(steeringOut, stateIn);

      SpaceShipControllable::OrthoNormalize(stateIn.mCurrentState);
   }
   

   void SpaceShipUpdater::UpdateHeading(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn)
   {
      SpaceShipState& physicalState = stateIn.mCurrentState;

      float thetaAngle = physicalState.mAngularVel * physicalState.mTimeStep; 

      osg::Matrix rotation = osg::Matrix::rotate( thetaAngle, osg::Vec3(0.0, 0.0, 1.0));

      physicalState.mForward = osg::Matrix::transform3x3(physicalState.mForward, rotation); 
   }

   void SpaceShipUpdater::UpdateAngularVelocity(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn)
   {
      SpaceShipState& physicalState = stateIn.mCurrentState;
      SpaceShipGoalState& physicalConstraint = stateIn.mGoalState;

      float newVelocity = controls.mYaw * physicalConstraint.mMaxAngularVel;
      float maxAccel = physicalConstraint.mMaxAngularAccel * physicalState.mTimeStep;
      physicalState.mAngularVel += Clamp(newVelocity - physicalState.mAngularVel, -maxAccel, maxAccel);

      physicalState.mAngularVel -= physicalState.mAngularVel * physicalConstraint.mAngularDragCoef;

      //this is necessary because we don't clamp the controls to 1.0 so technically we may need to clamp here
      physicalState.mAngularVel = Clamp(physicalState.mAngularVel, -physicalConstraint.mMaxAngularVel, physicalConstraint.mMaxAngularVel);
   }

   void SpaceShipUpdater::UpdateVerticalVelocity(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn)
   {
      SpaceShipState& physicalState = stateIn.mCurrentState;
      SpaceShipGoalState& physicalConstraint = stateIn.mGoalState;

      //update acceleration
      physicalState.mVerticalAccel = controls.mLift * physicalConstraint.mMaxVerticalAccel;

      //update velocity
      physicalState.mVerticalVel += physicalState.mVerticalAccel * physicalState.mTimeStep;
      physicalState.mVerticalVel -= (physicalState.mVerticalVel * physicalConstraint.mVerticalDragCoef);   
      physicalState.mVerticalVel = Clamp(physicalState.mVerticalVel, -physicalConstraint.mMaxVerticalVel, physicalConstraint.mMaxVerticalVel);    
   }

   void SpaceShipUpdater::UpdatePosition(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn)
   {
      SpaceShipState& physicalState = stateIn.mCurrentState;

      physicalState.mPos += physicalState.mVel * physicalState.mTimeStep;  
      physicalState.mPos[2] += physicalState.mVerticalVel * physicalState.mTimeStep;
   }

   void SpaceShipUpdater::UpdateVelocity(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn)
   {
      SpaceShipState& physicalState = stateIn.mCurrentState;
      SpaceShipGoalState& physicalConstraint = stateIn.mGoalState;

      float newVelocity = (controls.mThrust * physicalConstraint.mMaxVel);  
      float maxAccel = physicalConstraint.mMaxAccel * physicalState.mTimeStep; 

      physicalState.mVel += physicalState.mForward * Clamp(newVelocity - physicalState.mVel.length(), -maxAccel, maxAccel);
      physicalState.mVel -= (physicalState.mVel * physicalConstraint.mDragCoef);

      //we don't clamp the controls so this is necessary
      if(physicalState.mVel.length() > physicalConstraint.mMaxVel)
      {
         physicalState.mVel.normalize();
         physicalState.mVel *= physicalConstraint.mMaxVel;
      }
   }

   void SpaceShipUpdater::UpdateTilt(const SpaceShipControls& controls, osg::Vec3& tilt, SpaceShipControllable::BaseClass& stateIn)
   {
      SpaceShipState& physicalState = stateIn.mCurrentState;
      SpaceShipGoalState& physicalConstraint = stateIn.mGoalState;

      physicalState.mPitch = Dampen(physicalState.mPitch, (controls.mThrust * physicalConstraint.mMaxPitch), physicalConstraint.mMaxTiltPerSecond * physicalState.mTimeStep, (physicalState.mPitch / physicalConstraint.mMaxPitch));

      physicalState.mPitch = Clamp(physicalState.mPitch, -physicalConstraint.mMaxPitch, physicalConstraint.mMaxPitch);

      osg::Vec3 rightVec = (physicalState.mForward ^ physicalState.mUp);
      rightVec.normalize();

      osg::Matrix rotation = osg::Matrix::rotate( -physicalState.mPitch, rightVec);

      tilt = osg::Matrix::transform3x3(tilt, rotation); 

   }


   void SpaceShipUpdater::UpdateRoll(const SpaceShipControls& controls, osg::Vec3& roll, SpaceShipControllable::BaseClass& stateIn)
   {
      SpaceShipState& physicalState = stateIn.mCurrentState;
      SpaceShipGoalState& physicalConstraint = stateIn.mGoalState;

      physicalState.mRoll = Dampen(physicalState.mRoll, controls.mYaw * physicalConstraint.mMaxRoll, physicalConstraint.mMaxRollPerSecond * physicalState.mTimeStep, (physicalState.mRoll / physicalConstraint.mMaxRoll));
      
      //physicalState.mRoll *= ((0.1 + physicalState.mVel.length()) / physicalConstraint.mMaxVel);

      if(physicalState.mRoll > physicalConstraint.mMaxRoll) physicalState.mRoll = physicalConstraint.mMaxRoll;
      else if(physicalState.mRoll < -physicalConstraint.mMaxRoll) physicalState.mRoll = -physicalConstraint.mMaxRoll;

      osg::Matrix rotation = osg::Matrix::rotate(-physicalState.mRoll, physicalState.mForward);

      roll = osg::Matrix::transform3x3(roll, rotation); 

   }

   float SpaceShipUpdater::Clamp(float x, float from, float to)
   {
      if(x < from)
         return from;
      else if(x > to)
         return to;
      else return x;
   }

   float SpaceShipUpdater::Dampen(float last, float current, float pmax, float falloff)
   {
      if(current > last)
      {
         float adjust = pmax * (1.0f - falloff);

         if(current - last >= adjust)
            return last + adjust;
         else return current;
      }
      else
      {
         float adjust = pmax * falloff;
         if(last - current >= pmax)
            return last - pmax;
         else return current;
      }
   }


   //////////////////////////////////////////////////////////////////////////
   //SpaceShipAIHelper
   //////////////////////////////////////////////////////////////////////////
   SpaceShipAIHelper::SpaceShipAIHelper()
   {

   }

   SpaceShipAIHelper::~SpaceShipAIHelper()
   {

   }

   void SpaceShipAIHelper::Init()
   {
      BaseClass::Init();
   }

   void SpaceShipAIHelper::Spawn()
   {
      BaseClass::Spawn();
   }


   SpaceShipAIModel& SpaceShipAIHelper::GetAIModel()
   {
      return mAIModel;
   }

   const SpaceShipAIModel& SpaceShipAIHelper::GetAIModel() const
   {
      return mAIModel;
   }

   SpaceShipControllable& SpaceShipAIHelper::GetAIControllable()
   {
      return mAIControllable;
   }

   const SpaceShipControllable& SpaceShipAIHelper::GetAIControllable() const
   {
      return mAIControllable;
   }


   void SpaceShipAIHelper::Update(float dt)
   {
      GetStateMachine().Update(dt);
      mAIModel.Step(dt, mAIControllable);
      //mPhysicsModel->Update(mAIControllable.mCurrentControls, dt);
   }

   void SpaceShipAIHelper::PreSync(const dtCore::Transform& trans)
   {
      osg::Matrix xform;
      trans.Get(xform);
      SpaceShipControllable::SetState(xform, mAIControllable.mCurrentState);
   }

   void SpaceShipAIHelper::PostSync(dtCore::Transform& trans) const
   {
      osg::Matrix xform;
      SpaceShipControllable::SetMatrix(mAIControllable.mCurrentState, xform);
      trans.Set(xform);
   }

   void SpaceShipAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();

      //GetStateFactory()->RegisterType<GoToWaypointState>(AIStateType::AI_STATE_GO_TO_WAYPOINT.GetName());
      //GetStateFactory()->RegisterType<AttackState>(AIStateType::AI_STATE_ATTACK.GetName());
   }

   void SpaceShipAIHelper::CreateStates()
   {
      BaseClass::CreateStates();

      //GetStateMachine().AddState(&AIStateType::AI_STATE_FIND_TARGET);
      //GetStateMachine().AddState(&AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //GetStateMachine().AddState(&AIStateType::AI_STATE_ATTACK);
      //GetStateMachine().AddState(&AIStateType::AI_STATE_EVADE);
      //GetStateMachine().AddState(&AIStateType::AI_STATE_FOLLOW);
      //GetStateMachine().AddState(&AIStateType::AI_STATE_FLOCK);
      //GetStateMachine().AddState(&AIStateType::AI_STATE_WANDER);
      //GetStateMachine().AddState(&AIStateType::AI_STATE_DETONATE);
   }

   void SpaceShipAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();

      ////our next waypoint is calculated on the transition into the AI_STATE_GO_TO_WAYPOINT, so this transition will keep us moving on a path
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_ARRIVED, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_GO_TO_WAYPOINT);

      //BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FOLLOW, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_FLOCK, &AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_GO_TO_POINT, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_GO_TO_WAYPOINT);

      //BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_ATTACK);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_ATTACK);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_ENEMY_TARGETED, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_ATTACK);

      //BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_FIND_TARGET);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_FIND_TARGET);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_FIND_TARGET);

      //BaseClass::AddTransition(&AIEvent::AI_EVENT_DAMAGE_CRITICAL, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_EVADE);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_DAMAGE_CRITICAL, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_EVADE);

      //BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_WANDER);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_WANDER);
      //BaseClass::AddTransition(&AIEvent::AI_EVENT_TARGET_KILLED, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_FIND_TARGET);

   }

   void SpaceShipAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      ////setting this will allow us to follow a path
      //dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::GoToWaypoint));
      ////on entry into the waypoint state we will calculate the next waypoint to go to
      //state->AddEntryCommand(new dtUtil::Command0<void>(dtUtil::Command0<void>::FunctorType(this, &SpaceShipAIHelper::CalculateNextWaypoint)));

      ////these below all setup the states to call the default update
      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_DIE);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_IDLE);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::Attack));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_EVADE);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_FOLLOW);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_FLOCK);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_WANDER);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_DETONATE);
      //state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::DefaultStateUpdate));


      ////this can be used to change steering behaviors when transitioning into a new state
      //typedef dtUtil::Command1<void, dtCore::RefPtr<SteeringBehaviorType> > ChangeSteeringBehaviorCommand;
      //typedef dtUtil::Functor<void, TYPELIST_1(dtCore::RefPtr<SteeringBehaviorType>)> ChangeSteeringBehaviorFunctor;

      //float minSpeedPercent = 0.15f;
      //float maxSpeedPercent = 0.85f;
      //float lookAheadTime = 1.0f;
      //float timeToTarget = 0.5f;
      //float lookAheadRot = 5.0f;
      //float timeToTargetRot = 5.0f;

      //SteeringBehaviorType* behavior = new FollowPath(minSpeedPercent, maxSpeedPercent, lookAheadTime, timeToTarget, lookAheadRot, timeToTargetRot);
      //ChangeSteeringBehaviorCommand* ctbc = new ChangeSteeringBehaviorCommand(ChangeSteeringBehaviorFunctor(this, &SpaceShipAIHelper::ChangeSteeringBehavior), behavior);

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_GO_TO_WAYPOINT);
      //state->AddEntryCommand(ctbc);

      //float speed = 1000.0f;
      //behavior = new BombDive(speed);
      //ctbc = new ChangeSteeringBehaviorCommand(ChangeSteeringBehaviorFunctor(this, &SpaceShipAIHelper::ChangeSteeringBehavior), behavior);

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      //state->AddEntryCommand(ctbc);

      ////for all the rest of the states currently lets do nothing by setting the default behavior
      //behavior = new DoNothing();
      //ctbc = new ChangeSteeringBehaviorCommand(ChangeSteeringBehaviorFunctor(this, &SpaceShipAIHelper::ChangeSteeringBehavior), behavior);

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_DIE);
      //state->AddEntryCommand(ctbc);

      //state = GetStateMachine().GetState(&AIStateType::AI_STATE_IDLE);
      //state->AddEntryCommand(ctbc);

   }

   void SpaceShipAIHelper::SelectState(float dt)
   {
      BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_FIND_TARGET);
   }

   void SpaceShipAIHelper::DefaultStateUpdate(float dt)
   {
   }

   //void SpaceShipAIHelper::ChangeSteeringBehavior(dtCore::RefPtr<SteeringBehaviorType> newBehavior)
   //{
   //   GetSteeringModel()->SetSteeringBehavior(newBehavior.get());
   //}

   void SpaceShipAIHelper::CalculateNextWaypoint()
   {
      ////just go straight until we can work with a bezier node
      //osg::Matrix mat = GetPhysicsModel()->GetKinematicState().mTransform;
      //osg::Vec3 forward = dtUtil::MatrixUtil::GetRow3(mat, 1);

      //osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(mat, 3) + (forward * 50.0f);

      //GoToWaypointState* waypointState = dynamic_cast<GoToWaypointState*>(BaseClass::GetStateMachine().GetState(&AIStateType::AI_STATE_GO_TO_WAYPOINT)); 
      //if(waypointState != NULL)
      //{
      //   waypointState->mStateData = pos;
      //}
   }

   void SpaceShipAIHelper::GoToWaypoint(float dt)
   {
      //dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      //GoToWaypointState* waypointState = dynamic_cast<GoToWaypointState*>(npcState);
      //if(waypointState != NULL)
      //{
      //   osg::Vec3 pos = waypointState->mStateData.mCurrentWaypoint;

      //   dtAI::KinematicGoal kg; 
      //   kg.SetPosition(pos);
      //   BaseClass::GetSteeringModel()->SetKinematicGoal(kg);
      //}
      //else
      //{
      //   LOG_ERROR("Invalid state type for state 'AI_STATE_GO_TO_WAYPOINT'");
      //}
   }

   void SpaceShipAIHelper::Attack(float dt)
   {
      //dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      //AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      //if(attackState != NULL && attackState->mStateData.mTarget.valid())
      //{
      //   dtCore::Transform xform;
      //   attackState->mStateData.mTarget->GetTransform(xform);
      //   osg::Vec3 pos = xform.GetTranslation();

      //   //NOTE: HACK!!!! -The fort target is below the ground, I am adding an offset here
      //   //todo: find the bounding box of the object and use that to determine a good target point
      //   pos[2] += 5.0f;

      //   //if we are within distance, detonate
      //   //this is only for the enemy mine, and should be refactored
      //   float dist = GetDistance(pos);
      //   if(dist < 4.5)
      //   {
      //      BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DETONATE);
      //      return;
      //   }

      //   dtAI::KinematicGoal kg;
      //   kg.SetPosition(pos);
      //   BaseClass::GetSteeringModel()->SetKinematicGoal(kg);
      //}
      //else
      //{
      //   LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      //}
   }

   void SpaceShipAIHelper::SetCurrentTarget(dtCore::Transformable& target)
   {
      //dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      //AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      //if(attackState != NULL)
      //{
      //   attackState->mStateData.mTarget = &target;
      //   //let the system know we have targeted a new entity
      //   BaseClass::GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_ENEMY_TARGETED);
      //}
      //else
      //{
      //   LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      //}
   }

   //float SpaceShipAIHelper::GetDistance(const osg::Vec3& vec)
   //{
   //   osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(BaseClass::GetPhysicsModel()->GetKinematicState().mTransform, 3);
   //   return (vec - pos).length();
   //}


}//namespace NetDemo
