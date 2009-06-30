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
#include <Actors/EnemyDescriptionActor.h>

#include <dtCore/transform.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>

#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtPhysics/physicsobject.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/bodywrapper.h>

#include <osg/Vec2>
#include <osg/Vec3>

namespace NetDemo
{

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipState
   //////////////////////////////////////////////////////////////////////////
   SpaceShipState::SpaceShipState()
   {
   }

   SpaceShipState::~SpaceShipState()
   {
   }

   void SpaceShipState::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, SpaceShipState> RegHelperType;
      RegHelperType propReg(pc, this, group);

      REGISTER_PROPERTY(Pos, "The world space position.", RegHelperType, propReg);
      REGISTER_PROPERTY(Forward, "The world space forward vector.", RegHelperType, propReg);
      REGISTER_PROPERTY(Up, "The world space up vector.", RegHelperType, propReg);
      
      REGISTER_PROPERTY(Vel, "The world space velocity vector.", RegHelperType, propReg);
      REGISTER_PROPERTY(Accel, "The world space velocity vector.", RegHelperType, propReg);
      
      REGISTER_PROPERTY(AngularVel, "The scalar angular velocity.", RegHelperType, propReg);
      REGISTER_PROPERTY(AngularAccel, "The scalar angular acceleration.", RegHelperType, propReg);

      REGISTER_PROPERTY(VerticalVel, "The scalar vertical velocity.", RegHelperType, propReg);
      REGISTER_PROPERTY(VerticalAccel, "The scalar vertical acceleration.", RegHelperType, propReg);

      REGISTER_PROPERTY(Pitch, "The current pitch.", RegHelperType, propReg);
      REGISTER_PROPERTY(Roll, "The current roll.", RegHelperType, propReg);

      REGISTER_PROPERTY(TimeStep, "The per frame time step.", RegHelperType, propReg);
   }

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipGoalState
   //////////////////////////////////////////////////////////////////////////
   SpaceShipGoalState::SpaceShipGoalState()
   {
   }

   SpaceShipGoalState::~SpaceShipGoalState()
   {
   }

   void SpaceShipGoalState::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, SpaceShipGoalState> RegHelperType;
      RegHelperType propReg(pc, this, group);

      REGISTER_PROPERTY(DragCoef, "The linear air resistance.", RegHelperType, propReg);
      REGISTER_PROPERTY(AngularDragCoef, "The angular air resistance.", RegHelperType, propReg);
      REGISTER_PROPERTY(VerticalDragCoef, "The vertical air resistance.", RegHelperType, propReg);

      REGISTER_PROPERTY(MaxVel, "The maximim scalar velocity.", RegHelperType, propReg);
      REGISTER_PROPERTY(MaxAccel, "The maximim scalar acceleration.", RegHelperType, propReg);

      REGISTER_PROPERTY(MaxAngularVel, "The maximim scalar angular velocity.", RegHelperType, propReg);
      REGISTER_PROPERTY(MaxAngularAccel, "The maximim scalar angular acceleration.", RegHelperType, propReg);

      REGISTER_PROPERTY(MaxVerticalVel, "The maximim scalar vertical velocity.", RegHelperType, propReg);
      REGISTER_PROPERTY(MaxVerticalAccel, "The maximim scalar vertical acceleration.", RegHelperType, propReg);

      REGISTER_PROPERTY(MaxPitch, "The maximim pitch.", RegHelperType, propReg);
      REGISTER_PROPERTY(MaxRoll, "The maximim roll.", RegHelperType, propReg);

      REGISTER_PROPERTY(MaxTiltPerSecond, "The maximim pitch.", RegHelperType, propReg);
      REGISTER_PROPERTY(MaxRollPerSecond, "The maximim roll.", RegHelperType, propReg);

      REGISTER_PROPERTY(MinElevation, "The minimum elevation we can fly.", RegHelperType, propReg);
      REGISTER_PROPERTY(MaxElevation, "The maximum elevation we can fly.", RegHelperType, propReg);

   }

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipControls
   //////////////////////////////////////////////////////////////////////////
   SpaceShipControls::SpaceShipControls()
      : mThrust(0.0f)
      , mLift(0.0f)
      , mYaw(0.0f)
   {
   }

   SpaceShipControls::~SpaceShipControls()
   {
   }
 
   void SpaceShipControls::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, SpaceShipControls> RegHelperType;
      RegHelperType propReg(pc, this, group);

      REGISTER_PROPERTY(Thrust, "Our current scalar thrust.", RegHelperType, propReg);
      REGISTER_PROPERTY(Lift, "Our current scalar lift.", RegHelperType, propReg);
      REGISTER_PROPERTY(Yaw, "Our current scalar yaw.", RegHelperType, propReg);
   }

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
      goalStateConstraint.mMaxAngularVel = osg::DegreesToRadians(1000.0f);
      //goalStateConstraint.mMaxAngularVel = osg::DegreesToRadians(10.0f);
      //goalStateConstraint.mMaxAngularAccel = 200.0f * osg::DegreesToRadians(6.0f);
      goalStateConstraint.mMaxAngularAccel = 50.0f;
      goalStateConstraint.mMaxVel = 77.1667f; //150 knots
      //goalStateConstraint.mMaxAccel = 200.0f * 8.77f;
      goalStateConstraint.mMaxAccel = 1000.0f;
      goalStateConstraint.mMaxPitch = osg::DegreesToRadians(15.0f);
      goalStateConstraint.mMaxRoll = osg::DegreesToRadians(30.0f);
      goalStateConstraint.mMaxTiltPerSecond = osg::DegreesToRadians(5.0f);
      goalStateConstraint.mMaxRollPerSecond = osg::DegreesToRadians(5.0f);
      //goalStateConstraint.mMaxVerticalVel = 15.0f;//7.62f; //1500 feet/min
      goalStateConstraint.mMaxVerticalVel = 50.0f;//7.62f; //1500 feet/min
      goalStateConstraint.mMaxVerticalAccel = 50.0f;

      goalStateConstraint.mMinElevation = 25.0f;
      goalStateConstraint.mMaxElevation = 200.0f;
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
      //we let the physics set our position, we just set the orientation
      //result(3,0) = currentState.mPos[0];
      //result(3,1) = currentState.mPos[1];
      //result(3,2) = currentState.mPos[2];

      osg::Vec3 rightVector = currentState.mForward ^ currentState.mUp;
      rightVector.normalize();

      dtUtil::MatrixUtil::SetRow(result, currentState.mForward, 1);
      dtUtil::MatrixUtil::SetRow(result, currentState.mUp, 2);
      dtUtil::MatrixUtil::SetRow(result, rightVector, 0);
   }


   void SpaceShipControllable::UpdateState(float dt, const SpaceShipControls& steeringOut)
   {
      mTimeStep = dt;

      osg::Vec3 tempUpVector(0.0f, 0.0f, 1.0f);

      UpdateTilt(steeringOut, tempUpVector);
      UpdateVelocity(steeringOut);

      UpdateRoll(steeringOut, tempUpVector);
      UpdateAngularVelocity(steeringOut);

      UpdateVerticalVelocity(steeringOut);

      UpdateHeading(steeringOut);
      UpdatePosition(steeringOut);

      SpaceShipControllable::OrthoNormalize(mCurrentState);
   }

   bool SpaceShipControllable::FindPath(const AIState& fromState, const AIGoal& goal, AIPath& resultingPath) const
   {
      resultingPath.push_back(goal);
      return true;
   }

   void SpaceShipControllable::UpdateHeading(const SpaceShipControls& controls)
   {
      float thetaAngle = mCurrentState.mAngularVel * mTimeStep; 

      osg::Matrix rotation = osg::Matrix::rotate(thetaAngle, osg::Vec3(0.0, 0.0, 1.0));

      mCurrentState.mForward = osg::Matrix::transform3x3(mCurrentState.mForward, rotation); 
   }

   void SpaceShipControllable::UpdateAngularVelocity(const SpaceShipControls& controls)
   {
      SpaceShipState& physicalState = mCurrentState;
      SpaceShipGoalState& physicalConstraint = mGoalState;

      float newVelocity = controls.mYaw * physicalConstraint.mMaxAngularVel;
      physicalState.mAngularVel = newVelocity;
      //float maxAccel = physicalConstraint.mMaxAngularAccel * physicalState.mTimeStep;
      //physicalState.mAngularVel += Clamp(newVelocity - physicalState.mAngularVel, -maxAccel, maxAccel);

      //physicalState.mAngularVel -= physicalState.mAngularVel * physicalConstraint.mAngularDragCoef;

      //this is necessary because we don't clamp the controls to 1.0 so technically we may need to clamp here
      //physicalState.mAngularVel = Clamp(physicalState.mAngularVel, -physicalConstraint.mMaxAngularVel, physicalConstraint.mMaxAngularVel);
   }

   void SpaceShipControllable::UpdateVerticalVelocity(const SpaceShipControls& controls)
   {
      SpaceShipState& physicalState = mCurrentState;
      SpaceShipGoalState& physicalConstraint = mGoalState;

      //update acceleration
      physicalState.mVerticalAccel = controls.mLift * physicalConstraint.mMaxVerticalAccel;

      //update velocity
      physicalState.mVerticalVel += physicalState.mVerticalAccel * physicalState.mTimeStep;
      physicalState.mVerticalVel -= (physicalState.mVerticalVel * physicalConstraint.mVerticalDragCoef);   
      physicalState.mVerticalVel = Clamp(physicalState.mVerticalVel, -physicalConstraint.mMaxVerticalVel, physicalConstraint.mMaxVerticalVel);    
   }

   void SpaceShipControllable::UpdatePosition(const SpaceShipControls& controls)
   {
      SpaceShipState& physicalState = mCurrentState;

      physicalState.mPos += physicalState.mVel * physicalState.mTimeStep;  
      physicalState.mPos[2] += physicalState.mVerticalVel * physicalState.mTimeStep;
   }

   void SpaceShipControllable::UpdateVelocity(const SpaceShipControls& controls)
   {
      SpaceShipState& physicalState = mCurrentState;
      SpaceShipGoalState& physicalConstraint = mGoalState;

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

   void SpaceShipControllable::UpdateTilt(const SpaceShipControls& controls, osg::Vec3& tilt)
   {
      SpaceShipState& physicalState = mCurrentState;
      SpaceShipGoalState& physicalConstraint = mGoalState;

      physicalState.mPitch = Dampen(physicalState.mPitch, (controls.mThrust * physicalConstraint.mMaxPitch), physicalConstraint.mMaxTiltPerSecond * physicalState.mTimeStep, (physicalState.mPitch / physicalConstraint.mMaxPitch));

      physicalState.mPitch = Clamp(physicalState.mPitch, -physicalConstraint.mMaxPitch, physicalConstraint.mMaxPitch);

      osg::Vec3 rightVec = (physicalState.mForward ^ physicalState.mUp);
      rightVec.normalize();

      osg::Matrix rotation = osg::Matrix::rotate( -physicalState.mPitch, rightVec);

      tilt = osg::Matrix::transform3x3(tilt, rotation); 

   }


   void SpaceShipControllable::UpdateRoll(const SpaceShipControls& controls, osg::Vec3& roll)
   {
      SpaceShipState& physicalState = mCurrentState;
      SpaceShipGoalState& physicalConstraint = mGoalState;

      physicalState.mRoll = Dampen(physicalState.mRoll, controls.mYaw * physicalConstraint.mMaxRoll, physicalConstraint.mMaxRollPerSecond * physicalState.mTimeStep, (physicalState.mRoll / physicalConstraint.mMaxRoll));

      //physicalState.mRoll *= ((0.1 + physicalState.mVel.length()) / physicalConstraint.mMaxVel);

      if(physicalState.mRoll > physicalConstraint.mMaxRoll) physicalState.mRoll = physicalConstraint.mMaxRoll;
      else if(physicalState.mRoll < -physicalConstraint.mMaxRoll) physicalState.mRoll = -physicalConstraint.mMaxRoll;

      osg::Matrix rotation = osg::Matrix::rotate(-physicalState.mRoll, physicalState.mForward);

      roll = osg::Matrix::transform3x3(roll, rotation); 

   }

   float SpaceShipControllable::Clamp(float x, float from, float to)
   {
      if(x < from)
         return from;
      else if(x > to)
         return to;
      else return x;
   }

   float SpaceShipControllable::Dampen(float last, float current, float pmax, float falloff)
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
   //SpaceShipTargeter
   //////////////////////////////////////////////////////////////////////////
   SpaceShipTargeter::SpaceShipTargeter()
   {
   }

   SpaceShipTargeter::~SpaceShipTargeter()
   {
   }

   bool SpaceShipTargeter::GetGoal(const SpaceShipState& current_state, SpaceShipGoalState& result)
   {
      if(!mPointOfInterest.empty())
      {
         result.mPos = Top();
         Pop();
      }

      return true;
   }

   void SpaceShipTargeter::Push(const osg::Vec3& pos)
   {
         mPointOfInterest.push(pos);
   }

   void SpaceShipTargeter::Pop()
   {
      mPointOfInterest.pop();
   }

   const osg::Vec3& SpaceShipTargeter::Top() const
   {
      return mPointOfInterest.top();
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

   bool SpaceShipConstraint::WillViolate(const BaseClass::PathType& pathToFollow) const
   {
      return false;
   }

   void SpaceShipConstraint::Suggest(const BaseClass::PathType& pathToFollow, const SpaceShipState& current_state, SpaceShipGoalState& result) const
   {
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

   void SpaceShipAIHelper::OnInit(const EnemyDescriptionActor& desc)
   {      
      dtCore::Transform xform;
      desc.GetTransform(xform);
      osg::Matrix mat;
      xform.Get(mat);

      SpaceShipControllable::InitControllable(mat, mAIControllable);

      mAIControllable.mTargeters.push_back(&mDefaultTargeter);
      mAIControllable.mOutputControlFunc = SpaceShipControllable::BaseClass::OutputControlFunctor(this, &SpaceShipAIHelper::OutputControl);

      float minSpeedPercent = 0.15f;
      float maxSpeedPercent = 0.85f;
      float lookAheadTime = 1.0f;
      float timeToTarget = 0.5f;
      float lookAheadRot = 2.5f;
      float timeToTargetRot = 1.0f;

      mDefaultBehavior = new FollowPath(minSpeedPercent, maxSpeedPercent, lookAheadTime, timeToTarget, lookAheadRot, timeToTargetRot);

      BaseClass::OnInit(desc);
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

      //osg::Vec3 linearVelocity = mAIControllable.mCurrentState.mVel * 0.25f;
      //linearVelocity[2] = 2.5f + mAIControllable.mCurrentState.mVerticalVel;

      //osg::Vec3 force = mAIControllable.mCurrentState.GetForward() * 10.0f * mAIControllable.mCurrentState.mVel.length();
      //GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->AddForce(force);
      //GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->ApplyImpulse(linearVelocity);

      osg::Vec3 up = mAIControllable.mCurrentState.mUp;
      osg::Vec3 at = mAIControllable.mCurrentState.mForward;

      osg::Vec3 right = at ^ up;
      right.normalize();

      float maxLiftForce = 500.0f;
      float maxThrustForce = 100.0f;
      float maxYawForce = 100.0f;

      osg::Vec3 force;

      force += osg::Vec3(0.0f, 0.0f, 1000.0f) + (up * (mAIControllable.mCurrentControls.mLift * maxLiftForce));
      //force += at * (mAIControllable.mCurrentControls.mThrust * maxThrustForce);
      //force += right * (mAIControllable.mCurrentControls.mYaw * maxYawForce);
      
      dtPhysics::PhysicsObject* physicsObject = GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject();
      mAIControllable.mCurrentState.mVel = physicsObject->GetLinearVelocity();

      force += mAIControllable.mCurrentState.GetForward() * (75.0f + (100.0f * mAIControllable.mCurrentControls.mThrust));
      physicsObject->GetBodyWrapper()->AddForce(force);
   }

   void SpaceShipAIHelper::PreSync(const dtCore::Transform& trans)
   {
      //updates the state of the physics model
      BaseClass::PreSync(trans);

      osg::Matrix xform;
      trans.Get(xform);
      SpaceShipControllable::SetState(xform, mAIControllable.mCurrentState);

      dtPhysics::PhysicsObject* physicsObject = GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject();
      mAIControllable.mCurrentState.mVel = physicsObject->GetLinearVelocity();

   }

   void SpaceShipAIHelper::PostSync(dtCore::Transform& trans) const
   {
      osg::Matrix xform;
      trans.Get(xform);

      //we will set our orientation
      SpaceShipControllable::SetMatrix(mAIControllable.mCurrentState, xform);
      trans.SetRotation(xform);
   }


   void SpaceShipAIHelper::OutputControl(const SpaceShipControllable::PathType& pathToFollow, const SpaceShipControllable::StateType& current_state, SpaceShipControllable::ControlType& result) const
   {
      if(!pathToFollow.empty())
      {
         //GetSteeringModel()->Update(current_state.mTimeStep);
         mDefaultBehavior->Think(current_state.mTimeStep,pathToFollow.front(), current_state, result);
      }
      else
      {
         result = mAIControllable.mDefaultControls;
      }
   }

   void SpaceShipAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();

      GetStateFactory()->RegisterType<GoToWaypointState>(AIStateType::AI_STATE_GO_TO_WAYPOINT.GetName());
      GetStateFactory()->RegisterType<AttackState>(AIStateType::AI_STATE_ATTACK.GetName());
   }

   void SpaceShipAIHelper::CreateStates()
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

   void SpaceShipAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();
   }

   void SpaceShipAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      if(state != NULL)
      {
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &SpaceShipAIHelper::Attack));
      }
   }

   void SpaceShipAIHelper::SelectState(float dt)
   {
      BaseClass::GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_FIND_TARGET);
   }

   void SpaceShipAIHelper::Attack(float dt)
   {
      dtAI::NPCState* npcState = BaseClass::GetStateMachine().GetCurrentState();
      AttackState* attackState = dynamic_cast<AttackState*>(npcState);
      if(attackState != NULL && attackState->mStateData.mTarget.valid())
      {
         dtCore::Transform xform;
         attackState->mStateData.mTarget->GetTransform(xform);
         osg::Vec3 pos = xform.GetTranslation();

         pos[2] += 25.0f;
         mDefaultTargeter.Push(pos);
      }
      else
      {
         LOG_ERROR("Invalid state type for state 'AI_STATE_ATTACK'");
      }
   }

   void SpaceShipAIHelper::SetCurrentTarget(dtCore::Transformable& target)
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

   //float SpaceShipAIHelper::GetDistance(const osg::Vec3& vec)
   //{
   //   osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(BaseClass::GetPhysicsModel()->GetKinematicState().mTransform, 3);
   //   return (vec - pos).length();
   //}





   //////////////////////////////////////////////////////////////////////////
   //SpaceShipSteeringBehavior
   //////////////////////////////////////////////////////////////////////////

   float Align::Sgn(float x)
   {
      if(x < 0.0f)
      {
         return -1.0f;
      }
      else
      {
         return 1.0f;
      }
   }

   osg::Vec3 Align::GetTargetPosition(float dt, const SpaceShipGoalState& goal)
   {
      //project our target forward in time if it has a velocity
      osg::Vec3 targetPos = goal.GetPos();
      //if(goal.HasLinearVelocity())
      //{
      //   targetPos += goal.GetLinearVelocity() * dt;
      //}
      return targetPos;
   }

   float Align::GetTargetForward(float dt, const osg::Vec3& targetPos, const SpaceShipGoalState& current_goal, const SpaceShipState& current_state, osg::Vec3& vec_in)
   {
      osg::Vec3 projectedPos = current_state.mPos + (current_state.mVel * dt);

      osg::Vec3 goalForward = targetPos - projectedPos;

      vec_in = goalForward;
      return vec_in.normalize();   
   }


   void Align::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   { 
      float lookAhead = mTimeToTarget * dt;
      osg::Vec3 targetPos = GetTargetPosition(lookAhead, current_goal);
      osg::Vec3 goalForward;
      float dist = GetTargetForward(lookAhead, targetPos, current_goal, current_state, goalForward);      

      osg::Vec3 currForward = current_state.mForward;

      float thetaAngle = (current_state.mAngularVel * lookAhead); 
      osg::Matrix rotation = osg::Matrix::rotate(thetaAngle, osg::Vec3(0.0, 0.0, 1.0));
      currForward = osg::Matrix::transform3x3(currForward, rotation); 
      currForward.normalize();

      float dot = goalForward * currForward;
      float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);

      float angle = acos(dot);
      if(angle > 0.1f)
      {     
         float yaw = angle / fabs(current_state.mAngularVel);
         dtUtil::Clamp(yaw, 0.0001f, mTimeToTarget);
         yaw /= mTimeToTarget;
         result.mYaw = Sgn(sign) * yaw;         
      }  
   }

   void FollowPath::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   {
      float lookAhead = mLookAhead * dt;
      BaseClass::Think(lookAhead, current_goal, current_state, result);

      osg::Vec3 targetPos = BaseClass::GetTargetPosition(lookAhead, current_goal);
      osg::Vec3 goalForward;
      float dist = GetTargetForward(lookAhead, targetPos, current_goal, current_state, goalForward);
      osg::Vec3 currForward = current_state.mForward;

      float angle = 0.0f;
      float dot = goalForward * currForward;
      float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);

      angle = acos(dot);

      float timeRemaining = dist / current_state.mVel.length();

      dtUtil::Clamp(timeRemaining, 0.00001f, mTimeToTarget);
      timeRemaining /= mTimeToTarget;

      result.mThrust = timeRemaining * dtUtil::MapRangeValue(angle, 0.0f, float(osg::PI), mMaxSpeed, mMinSpeed);

      //compute height
      osg::Vec3 pos = current_state.mPos;
      float heightDiff = current_goal.GetPos()[2] - (pos[2] + (current_state.mVel[2] * dt));
      heightDiff /= 100.0f;

      dtUtil::Clamp(heightDiff, -1.0f, 1.0f);
      float absHeightDiff = fabs(heightDiff);
      result.mLift = BaseClass::Sgn(heightDiff) * absHeightDiff * absHeightDiff;
   }

}//namespace NetDemo
