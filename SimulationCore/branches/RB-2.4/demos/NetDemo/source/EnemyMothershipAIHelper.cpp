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
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/bodywrapper.h>

#include <osg/Vec2>
#include <osg/Vec3>

namespace NetDemo
{

   //////////////////////////////////////////////////////////////////////////
   //EnemyMothershipState
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipState::EnemyMothershipState()
   {
   }

   EnemyMothershipState::~EnemyMothershipState()
   {
   }

   void EnemyMothershipState::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, EnemyMothershipState> RegHelperType;
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
   //EnemyMothershipGoalState
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipGoalState::EnemyMothershipGoalState()
   {
   }

   EnemyMothershipGoalState::~EnemyMothershipGoalState()
   {
   }

   void EnemyMothershipGoalState::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, EnemyMothershipGoalState> RegHelperType;
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
   //EnemyMothershipControls
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipControls::EnemyMothershipControls()
      : mThrust(0.0f)
      , mLift(0.0f)
      , mYaw(0.0f)
   {
   }

   EnemyMothershipControls::~EnemyMothershipControls()
   {
   }
 
   void EnemyMothershipControls::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, EnemyMothershipControls> RegHelperType;
      RegHelperType propReg(pc, this, group);

      REGISTER_PROPERTY(Thrust, "Our current scalar thrust.", RegHelperType, propReg);
      REGISTER_PROPERTY(Lift, "Our current scalar lift.", RegHelperType, propReg);
      REGISTER_PROPERTY(Yaw, "Our current scalar yaw.", RegHelperType, propReg);
   }

   //////////////////////////////////////////////////////////////////////////
   //EnemyMothershipControllable
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipControllable::EnemyMothershipControllable()
   {
   }

   EnemyMothershipControllable::~EnemyMothershipControllable()
   {
   }

   void EnemyMothershipControllable::InitControllable(const osg::Matrix& matIn, EnemyMothershipControllable& stateIn)
   {
      stateIn.mCurrentControls.SetYaw(0.0);
      stateIn.mCurrentControls.SetThrust(0.0);
      stateIn.mCurrentControls.SetLift(0.0);

      EnemyMothershipGoalState& goalStateConstraint = stateIn.mGoalState;
      goalStateConstraint.SetMaxAngularVel(osg::DegreesToRadians(1000.0f));
      //goalStateConstraint.mMaxAngularVel = osg::DegreesToRadians(10.0f);
      //goalStateConstraint.mMaxAngularAccel = 200.0f * osg::DegreesToRadians(6.0f);
      goalStateConstraint.SetMaxAngularAccel(50.0f);
      goalStateConstraint.SetMaxVel(77.1667f); //150 knots
      //goalStateConstraint.mMaxAccel = 200.0f * 8.77f;
      goalStateConstraint.SetMaxAccel(1000.0f);
      goalStateConstraint.SetMaxPitch(osg::DegreesToRadians(15.0f));
      goalStateConstraint.SetMaxRoll(osg::DegreesToRadians(30.0f));
      goalStateConstraint.SetMaxTiltPerSecond(osg::DegreesToRadians(5.0f));
      goalStateConstraint.SetMaxRollPerSecond(osg::DegreesToRadians(5.0f));
      //goalStateConstraint.mMaxVerticalVel(15.0f;//7.62f; //1500 feet/min
      goalStateConstraint.SetMaxVerticalVel(50.0f);//7.62f; //1500 feet/min
      goalStateConstraint.SetMaxVerticalAccel(50.0f);

      goalStateConstraint.SetMinElevation(25.0f);
      goalStateConstraint.SetMaxElevation(200.0f);
      goalStateConstraint.SetDragCoef(0.005f);
      goalStateConstraint.SetAngularDragCoef(0.005f);
      goalStateConstraint.SetVerticalDragCoef(0.005f);

      SetState(matIn, stateIn.mCurrentState);
      SetState(matIn, stateIn.mGoalState);
   }

   void EnemyMothershipControllable::ResetState(const osg::Matrix& matIn, EnemyMothershipState& EnemyMothershipState)
   {
      EnemyMothershipState.SetVel(osg::Vec3(0.0f, 0.0f, 0.0f));
      EnemyMothershipState.SetAccel(osg::Vec3(0.0f, 0.0f, 0.0f));
      EnemyMothershipState.SetPitch(0.0f);
      EnemyMothershipState.SetRoll(0.0f);
      EnemyMothershipState.SetAngularAccel(0.0f);
      EnemyMothershipState.SetAngularVel(0.0f);
      EnemyMothershipState.SetVerticalVel(0.0f);
      EnemyMothershipState.SetVerticalAccel(0.0f);

      osg::Vec3 fwd = dtUtil::MatrixUtil::GetRow3(matIn, 1);
      fwd.normalize();
      EnemyMothershipState.SetForward(fwd);

      osg::Vec3 up = dtUtil::MatrixUtil::GetRow3(matIn, 2);
      up.normalize();
      EnemyMothershipState.SetUp(up);

      EnemyMothershipState.SetPos(osg::Vec3(matIn(3,0), matIn(3,1), matIn(3,2)));

      EnemyMothershipState.SetTimeStep(0.0f);
      EnemyMothershipState.SetVel(osg::Vec3());
   }

   void EnemyMothershipControllable::SetState(const osg::Matrix& matIn, EnemyMothershipState& EnemyMothershipState)
   {
      osg::Vec3 fwd = dtUtil::MatrixUtil::GetRow3(matIn, 1);
      fwd.normalize();
      EnemyMothershipState.SetForward(fwd);

      osg::Vec3 up = dtUtil::MatrixUtil::GetRow3(matIn, 2);
      up.normalize();
      EnemyMothershipState.SetUp(up);

      EnemyMothershipState.SetPos(osg::Vec3(matIn(3,0), matIn(3,1),  matIn(3,2)));
   }

   void EnemyMothershipControllable::OrthoNormalize(EnemyMothershipState& currentState)
   {
      osg::Vec3 tempUpVector(0.0f, 0.0f, 1.0f);

      osg::Vec3 fwd = currentState.GetForward();
      fwd.normalize();
      currentState.SetForward(fwd);

      osg::Vec3 rightVector = currentState.GetForward() ^ tempUpVector;
      rightVector.normalize();
      osg::Vec3 up = rightVector ^ currentState.GetForward();
      up.normalize();
      currentState.SetUp(up);
   }

   void EnemyMothershipControllable::SetMatrix(const EnemyMothershipState& currentState, osg::Matrix& result)
   {
      //we let the physics set our position, we just set the orientation
      //result(3,0) = currentState.GetPos()[0];
      //result(3,1) = currentState.GetPos()[1];
      //result(3,2) = currentState.GetPos()[2];

      osg::Vec3 rightVector = currentState.GetForward() ^ currentState.GetUp();
      rightVector.normalize();

      dtUtil::MatrixUtil::SetRow(result, currentState.GetForward(), 1);
      dtUtil::MatrixUtil::SetRow(result, currentState.GetUp(), 2);
      dtUtil::MatrixUtil::SetRow(result, rightVector, 0);
   }


   void EnemyMothershipControllable::UpdateState(float dt, const EnemyMothershipControls& steeringOut)
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

      EnemyMothershipControllable::OrthoNormalize(mCurrentState);
   }

   bool EnemyMothershipControllable::FindPath(const AIState& fromState, const AIGoal& goal, AIPath& resultingPath) const
   {
      resultingPath.push_back(goal);
      return true;
   }

   void EnemyMothershipControllable::UpdateHeading(const EnemyMothershipControls& controls)
   {
      float thetaAngle = mCurrentState.GetAngularVel() * mTimeStep;

      osg::Matrix rotation = osg::Matrix::rotate(thetaAngle, osg::Vec3(0.0, 0.0, 1.0));

      mCurrentState.SetForward(osg::Matrix::transform3x3(mCurrentState.GetForward(), rotation));
   }

   void EnemyMothershipControllable::UpdateAngularVelocity(const EnemyMothershipControls& controls)
   {
      EnemyMothershipState& physicalState = mCurrentState;
      EnemyMothershipGoalState& physicalConstraint = mGoalState;

      float newVelocity = controls.GetYaw() * physicalConstraint.GetMaxAngularVel();
      physicalState.SetAngularVel(newVelocity);
      //float maxAccel = physicalConstraint.mMaxAngularAccel * physicalState.mTimeStep;
      //physicalState.GetAngularVel() += Clamp(newVelocity - physicalState.GetAngularVel(), -maxAccel, maxAccel);

      //physicalState.GetAngularVel() -= physicalState.GetAngularVel() * physicalConstraint.mAngularDragCoef;

      //this is necessary because we don't clamp the controls to 1.0 so technically we may need to clamp here
      //physicalState.GetAngularVel() = Clamp(physicalState.GetAngularVel(), -physicalConstraint.mMaxAngularVel, physicalConstraint.mMaxAngularVel);
   }

   void EnemyMothershipControllable::UpdateVerticalVelocity(const EnemyMothershipControls& controls)
   {
      EnemyMothershipState& physicalState = mCurrentState;
      EnemyMothershipGoalState& physicalConstraint = mGoalState;

      //update acceleration
      physicalState.SetVerticalAccel(controls.GetLift() * physicalConstraint.GetMaxVerticalAccel());

      //update velocity
      physicalState.SetVerticalVel(physicalState.GetVerticalVel() + (physicalState.GetVerticalAccel() * physicalState.GetTimeStep()));
      physicalState.SetVerticalVel(physicalState.GetVerticalVel() - (physicalState.GetVerticalVel() * physicalConstraint.GetVerticalDragCoef()));
      physicalState.SetVerticalVel(Clamp(physicalState.GetVerticalVel(), -physicalConstraint.GetMaxVerticalVel(), physicalConstraint.GetMaxVerticalVel()));
   }

   void EnemyMothershipControllable::UpdatePosition(const EnemyMothershipControls& controls)
   {
      EnemyMothershipState& physicalState = mCurrentState;

      osg::Vec3 newPos = physicalState.GetPos() + (physicalState.GetVel() * physicalState.GetTimeStep());
      newPos[2] += physicalState.GetVerticalVel() * physicalState.GetTimeStep();
      physicalState.SetPos(newPos);
   }

   void EnemyMothershipControllable::UpdateVelocity(const EnemyMothershipControls& controls)
   {
      EnemyMothershipState& physicalState = mCurrentState;
      EnemyMothershipGoalState& physicalConstraint = mGoalState;

      float newVelocity = (controls.GetThrust() * physicalConstraint.GetMaxVel());
      float maxAccel = physicalConstraint.GetMaxAccel() * physicalState.GetTimeStep();

      osg::Vec3 newVel = physicalState.GetVel();
      newVel += physicalState.GetForward() * Clamp(newVelocity - physicalState.GetVel().length(), -maxAccel, maxAccel);
      newVel -= (physicalState.GetVel() * physicalConstraint.GetDragCoef());


      //we don't clamp the controls so this is necessary
      if (newVel.length() > physicalConstraint.GetMaxVel())
      {
         newVel.normalize();
         newVel *= physicalConstraint.GetMaxVel();
      }
      physicalState.SetVel(newVel);
   }

   void EnemyMothershipControllable::UpdateTilt(const EnemyMothershipControls& controls, osg::Vec3& tilt)
   {
      EnemyMothershipState& physicalState = mCurrentState;
      EnemyMothershipGoalState& physicalConstraint = mGoalState;

      physicalState.SetPitch(Dampen(physicalState.GetPitch(), (controls.GetThrust() * physicalConstraint.GetMaxPitch()), physicalConstraint.GetMaxTiltPerSecond() * physicalState.GetTimeStep(), (physicalState.GetPitch() / physicalConstraint.GetMaxPitch())));

      physicalState.SetPitch(Clamp(physicalState.GetPitch(), -physicalConstraint.GetMaxPitch(), physicalConstraint.GetMaxPitch()));

      osg::Vec3 rightVec = (physicalState.GetForward() ^ physicalState.GetUp());
      rightVec.normalize();

      osg::Matrix rotation = osg::Matrix::rotate( -physicalState.GetPitch(), rightVec);

      tilt = osg::Matrix::transform3x3(tilt, rotation); 

   }


   void EnemyMothershipControllable::UpdateRoll(const EnemyMothershipControls& controls, osg::Vec3& roll)
   {
      EnemyMothershipState& physicalState = mCurrentState;
      EnemyMothershipGoalState& physicalConstraint = mGoalState;

      physicalState.SetRoll(Dampen(physicalState.GetRoll(), controls.GetYaw() * physicalConstraint.GetMaxRoll(), physicalConstraint.GetMaxRollPerSecond() * physicalState.GetTimeStep(), (physicalState.GetRoll() / physicalConstraint.GetMaxRoll())));

      //physicalState.mRoll *= ((0.1 + physicalState.GetVel().length()) / physicalConstraint.mMaxVel);

      if (physicalState.GetRoll() > physicalConstraint.GetMaxRoll()) physicalState.SetRoll(physicalConstraint.GetMaxRoll());
      else if (physicalState.GetRoll() < -physicalConstraint.GetMaxRoll()) physicalState.SetRoll(-physicalConstraint.GetMaxRoll());

      osg::Matrix rotation = osg::Matrix::rotate(-physicalState.GetRoll(), physicalState.GetForward());

      roll = osg::Matrix::transform3x3(roll, rotation); 

   }

   float EnemyMothershipControllable::Clamp(float x, float from, float to)
   {
      if(x < from)
         return from;
      else if(x > to)
         return to;
      else return x;
   }

   float EnemyMothershipControllable::Dampen(float last, float current, float pmax, float falloff)
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
   //EnemyMothershipTargeter
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipTargeter::EnemyMothershipTargeter()
   {
   }

   EnemyMothershipTargeter::~EnemyMothershipTargeter()
   {
   }

   bool EnemyMothershipTargeter::GetGoal(const EnemyMothershipState& current_state, EnemyMothershipGoalState& result)
   {
      if(!mPointOfInterest.empty())
      {
         result.SetPos(Top());
         Pop();
      }

      return true;
   }

   void EnemyMothershipTargeter::Push(const osg::Vec3& pos)
   {
         mPointOfInterest.push(pos);
   }

   void EnemyMothershipTargeter::Pop()
   {
      mPointOfInterest.pop();
   }

   const osg::Vec3& EnemyMothershipTargeter::Top() const
   {
      return mPointOfInterest.top();
   }


   //////////////////////////////////////////////////////////////////////////
   //EnemyMothershipDecomposer
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipDecomposer::EnemyMothershipDecomposer()
   {
   }

   EnemyMothershipDecomposer::~EnemyMothershipDecomposer()
   {
   }

   void EnemyMothershipDecomposer::Decompose(const EnemyMothershipState& current_state, EnemyMothershipGoalState& result) const
   {
   }

   //////////////////////////////////////////////////////////////////////////
   //EnemyMothershipConstraint
   //////////////////////////////////////////////////////////////////////////
   EnemyMothershipConstraint::EnemyMothershipConstraint()
   {
   }

   EnemyMothershipConstraint::~EnemyMothershipConstraint()
   {
   }

   bool EnemyMothershipConstraint::WillViolate(const BaseClass::PathType& pathToFollow) const
   {
      return false;
   }

   void EnemyMothershipConstraint::Suggest(const BaseClass::PathType& pathToFollow, const EnemyMothershipState& current_state, EnemyMothershipGoalState& result) const
   {
   }


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
      mTargetOffset = osg::Vec3(
         dtUtil::RandFloat(-100.0f, 100.0f),
         dtUtil::RandFloat(50.0f, 350.0f),
         dtUtil::RandFloat(75.0f, 150.0f) );
   }

   void EnemyMothershipAIHelper::OnInit(const EnemyDescriptionActor* desc)
   {
      osg::Matrix mat;

      EnemyMothershipControllable::InitControllable(mat, mAIControllable);

      mAIControllable.mTargeters.push_back(&mDefaultTargeter);
      mAIControllable.mOutputControlFunc = EnemyMothershipControllable::BaseClass::OutputControlFunctor(this, &EnemyMothershipAIHelper::OutputControl);

      float minSpeedPercent = 0.0f;
      float maxSpeedPercent = 1.0f;
      float lookAheadTime = 2.0f;
      float timeToTarget = 30.0f;
      float lookAheadRot = 2.5f;
      float timeToTargetRot = 1.0f;

      mDefaultBehavior = new FollowPath(minSpeedPercent, maxSpeedPercent, lookAheadTime, timeToTarget, lookAheadRot, timeToTargetRot);

      BaseClass::OnInit(desc);
   }

   void EnemyMothershipAIHelper::Spawn()
   {
      BaseClass::Spawn();
   }


   EnemyMothershipAIModel& EnemyMothershipAIHelper::GetAIModel()
   {
      return mAIModel;
   }

   const EnemyMothershipAIModel& EnemyMothershipAIHelper::GetAIModel() const
   {
      return mAIModel;
   }

   EnemyMothershipControllable& EnemyMothershipAIHelper::GetAIControllable()
   {
      return mAIControllable;
   }

   const EnemyMothershipControllable& EnemyMothershipAIHelper::GetAIControllable() const
   {
      return mAIControllable;
   }


   void EnemyMothershipAIHelper::Update(float dt)
   {
      GetStateMachine().Update(dt);
      mAIModel.Step(dt, mAIControllable);

      //osg::Vec3 linearVelocity = mAIControllable.mCurrentState.GetVel() * 0.25f;
      //linearVelocity[2] = 2.5f + mAIControllable.mCurrentState.mVerticalVel;

      //osg::Vec3 force = mAIControllable.mCurrentState.GetForward() * 10.0f * mAIControllable.mCurrentState.GetVel().length();
      //GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->AddForce(force);
      //GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->ApplyImpulse(linearVelocity);

      osg::Vec3 up = mAIControllable.mCurrentState.GetUp();
      osg::Vec3 at = mAIControllable.mCurrentState.GetForward();

      osg::Vec3 right = at ^ up;
      right.normalize();

      float maxLiftForce = 500000.0f;
      float maxThrustForce = 10000.0f;

      osg::Vec3 force;

      force += up * (mAIControllable.mCurrentControls.GetLift() * maxLiftForce);
      force += at * (mAIControllable.mCurrentControls.GetThrust() * maxThrustForce);
      //force += right * (mAIControllable.mCurrentControls.mYaw * maxYawForce);
      
      dtPhysics::PhysicsObject* physicsObject = GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject();
      mAIControllable.mCurrentState.SetVel(physicsObject->GetLinearVelocity());

      //force += mAIControllable.mCurrentState.GetForward() * (75.0f + (100.0f * mAIControllable.mCurrentControls.GetThrust()));
      physicsObject->GetBodyWrapper()->AddForce(force);
   }

   void EnemyMothershipAIHelper::PreSync(const dtCore::Transform& trans)
   {
      //updates the state of the physics model
      BaseClass::PreSync(trans);

      osg::Matrix xform;
      trans.Get(xform);
      EnemyMothershipControllable::SetState(xform, mAIControllable.mCurrentState);

      dtPhysics::PhysicsObject* physicsObject = GetPhysicsModel()->GetPhysicsHelper()->GetMainPhysicsObject();
      mAIControllable.mCurrentState.SetVel(physicsObject->GetLinearVelocity());

   }

   void EnemyMothershipAIHelper::PostSync(dtCore::Transform& trans) const
   {
      osg::Matrix xform;
      trans.Get(xform);

      //we will set our orientation
      EnemyMothershipControllable::SetMatrix(mAIControllable.mCurrentState, xform);
      trans.SetRotation(xform);
   }


   void EnemyMothershipAIHelper::OutputControl(const EnemyMothershipControllable::PathType& pathToFollow, const EnemyMothershipControllable::StateType& current_state, EnemyMothershipControllable::ControlType& result) const
   {
      if (!pathToFollow.empty())
      {
         //GetSteeringModel()->Update(current_state.mTimeStep);
         mDefaultBehavior->Think(current_state.GetTimeStep(), pathToFollow.front(), current_state, result);
      }
      else
      {
         result = mAIControllable.mDefaultControls;
      }
   }

   void EnemyMothershipAIHelper::RegisterStates()
   {
      BaseClass::RegisterStates();

      GetStateFactory()->RegisterType<GoToWaypointState>(AIStateType::AI_STATE_GO_TO_WAYPOINT.GetName());
      GetStateFactory()->RegisterType<AttackState>(AIStateType::AI_STATE_ATTACK.GetName());
   }

   void EnemyMothershipAIHelper::CreateStates()
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

   void EnemyMothershipAIHelper::SetupTransitions()
   {
      BaseClass::SetupTransitions();
   }

   void EnemyMothershipAIHelper::SetupFunctors()
   {
      BaseClass::SetupFunctors();

      dtAI::NPCState* state = GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK);
      if(state != NULL)
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
      if(attackState != NULL && attackState->mStateData.mTarget.valid())
      {
         dtCore::Transform xform;
         attackState->mStateData.mTarget->GetTransform(xform);
         osg::Vec3 pos = xform.GetTranslation();

         pos += mTargetOffset;
         mDefaultTargeter.Push(pos);

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

   //float EnemyMothershipAIHelper::GetDistance(const osg::Vec3& vec)
   //{
   //   osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(BaseClass::GetPhysicsModel()->GetKinematicState().mTransform, 3);
   //   return (vec - pos).length();
   //}





   //////////////////////////////////////////////////////////////////////////
   //EnemyMothershipSteeringBehavior
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

   osg::Vec3 Align::GetTargetPosition(float dt, const EnemyMothershipGoalState& goal)
   {
      //project our target forward in time if it has a velocity
      osg::Vec3 targetPos = goal.GetPos();
      //if(goal.HasLinearVelocity())
      //{
      //   targetPos += goal.GetLinearVelocity() * dt;
      //}
      return targetPos;
   }

   float Align::GetTargetForward(float dt, const osg::Vec3& targetPos, const EnemyMothershipGoalState& current_goal, const EnemyMothershipState& current_state, osg::Vec3& vec_in)
   {
      osg::Vec3 projectedPos = current_state.GetPos() + (current_state.GetVel() * dt);

      osg::Vec3 goalForward = targetPos - projectedPos;

      vec_in = goalForward;
      return vec_in.normalize();   
   }


   void Align::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   { 
      dtUtil::Clamp(dt, 0.0167f, 0.1f);

      float lookAhead = mTimeToTarget * dt;
      osg::Vec3 targetPos = GetTargetPosition(lookAhead, current_goal);
      osg::Vec3 goalForward;
      float dist = GetTargetForward(lookAhead, targetPos, current_goal, current_state, goalForward);      

      osg::Vec3 currForward = current_state.GetForward();

      float thetaAngle = (current_state.GetAngularVel() * lookAhead);
      osg::Matrix rotation = osg::Matrix::rotate(thetaAngle, osg::Vec3(0.0, 0.0, 1.0));
      currForward = osg::Matrix::transform3x3(currForward, rotation); 
      currForward.normalize();

      float dot = goalForward * currForward;
      float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);

      float angle = acos(dot);
      if(angle > 0.1f)
      {     
         float yaw = angle / fabs(current_state.GetAngularVel());
         dtUtil::Clamp(yaw, 0.0001f, mTimeToTarget);
         yaw /= mTimeToTarget;
         result.SetYaw(Sgn(sign) * yaw);
      }  
   }

   void FollowPath::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   {
      dtUtil::Clamp(dt, 0.0167f, 0.1f);

      float lookAhead = mLookAhead * dt;
      BaseClass::Think(lookAhead, current_goal, current_state, result);

      const osg::Vec3 goalPos = current_goal.GetPos();
      const osg::Vec3 pos = current_state.GetPos();

      osg::Vec3 goalForward;
      float dist = GetTargetForward(mTimeToTarget, goalPos, current_goal, current_state, goalForward);
      osg::Vec3 currForward = current_state.GetForward();

      float angle = 0.0f;
      float dot = goalForward * currForward;
      float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);

      angle = acos(dot);

      float timeRemaining = dist / current_state.GetVel().length();

      dtUtil::Clamp(timeRemaining, 0.00001f, mTimeToTarget);
      timeRemaining /= mTimeToTarget;

      osg::Vec3 velNorm = current_state.GetVel();
      velNorm.normalize();

      float thrustSign = velNorm * goalForward;
      if(thrustSign < -0.75f && dot < -0.75f) result.SetThrust(-1.0f);
      else 
         result.SetThrust(timeRemaining * dtUtil::MapRangeValue(angle, 0.0f, float(osg::PI), mMaxSpeed, mMinSpeed));

      //compute height
      float zVel = current_state.GetVel()[2];
      if(fabs(zVel) < 0.0001) zVel = BaseClass::Sgn(zVel) * 0.0001;

      float heightLookAhead = zVel * mTimeToTarget;

      float heightDiff = goalPos[2] - (pos[2] + heightLookAhead);
      float remainingFallTime = fabs(heightDiff / zVel);
      dtUtil::Clamp(remainingFallTime, 0.00001f, mTimeToTarget);
      remainingFallTime /= mTimeToTarget;

      float sgnHeightDiff = BaseClass::Sgn(heightDiff);
      result.SetLift(remainingFallTime * sgnHeightDiff);
   }

}//namespace NetDemo
