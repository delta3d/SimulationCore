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

#include <AIUtility.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <osg/Vec2>


namespace NetDemo
{

   //////////////////////////////////////////////////////////////////////////
   //BaseAIGameState
   //////////////////////////////////////////////////////////////////////////
   BaseAIGameState::BaseAIGameState()
   : mPos()
   , mForward()
   , mUp()
   , mVel()
   , mAccel()
   , mAngularVel(0.0f)
   , mAngularAccel(0.0f)
   , mVerticalVel(0.0f)
   , mVerticalAccel(0.0f)
   , mPitch(0.0f)
   , mRoll(0.0f)
   , mTimeStep(0.0f)
   , mThrusters(0.0f)
   {
   }

   BaseAIGameState::~BaseAIGameState()
   {
   }

   void BaseAIGameState::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, BaseAIGameState> RegHelperType;
      RegHelperType propReg(pc, this, group);

      DT_REGISTER_PROPERTY(Pos, "The world space position.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(Forward, "The world space forward vector.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(Up, "The world space up vector.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(Vel, "The world space velocity vector.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(Accel, "The world space velocity vector.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(AngularVel, "The scalar angular velocity.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(AngularAccel, "The scalar angular acceleration.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(VerticalVel, "The scalar vertical velocity.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(VerticalAccel, "The scalar vertical acceleration.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(Pitch, "The current pitch.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(Roll, "The current roll.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(TimeStep, "The per frame time step.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(Thrusters, "Boosters", RegHelperType, propReg);

   }

   //////////////////////////////////////////////////////////////////////////
   //BaseAIGoalState
   //////////////////////////////////////////////////////////////////////////
   BaseAIGoalState::BaseAIGoalState()
   : mDragCoef(0.0f)
   , mAngularDragCoef(0.0f)
   , mVerticalDragCoef(0.0f)
   , mMaxVel(0.0f)
   , mMaxAccel(0.0f)
   , mMaxAngularVel(0.0f)
   , mMaxAngularAccel(0.0f)
   , mMaxVerticalVel(0.0f)
   , mMaxVerticalAccel(0.0f)
   , mMaxPitch(0.0f)
   , mMaxRoll(0.0f)
   , mMaxTiltPerSecond(0.0f)
   , mMaxRollPerSecond(0.0f)
   , mMinElevation(0.0f)
   , mMaxElevation(0.0f)
   {
   }

   BaseAIGoalState::~BaseAIGoalState()
   {
   }

   void BaseAIGoalState::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, BaseAIGoalState> RegHelperType;
      RegHelperType propReg(pc, this, group);

      DT_REGISTER_PROPERTY(DragCoef, "The linear air resistance.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(AngularDragCoef, "The angular air resistance.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(VerticalDragCoef, "The vertical air resistance.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(MaxVel, "The maximim scalar velocity.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxAccel, "The maximim scalar acceleration.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(MaxAngularVel, "The maximim scalar angular velocity.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxAngularAccel, "The maximim scalar angular acceleration.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(MaxVerticalVel, "The maximim scalar vertical velocity.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxVerticalAccel, "The maximim scalar vertical acceleration.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(MaxPitch, "The maximim pitch.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxRoll, "The maximim roll.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(MaxTiltPerSecond, "The maximim pitch.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxRollPerSecond, "The maximim roll.", RegHelperType, propReg);

      DT_REGISTER_PROPERTY(MinElevation, "The minimum elevation we can fly.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxElevation, "The maximum elevation we can fly.", RegHelperType, propReg);

 }

   //////////////////////////////////////////////////////////////////////////
   //BaseAIControls
   //////////////////////////////////////////////////////////////////////////
   BaseAIControls::BaseAIControls()
      : mThrust(0.0f)
      , mLift(0.0f)
      , mYaw(0.0f)
   {
   }

   BaseAIControls::~BaseAIControls()
   {
   }

   void BaseAIControls::RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
   {
      typedef dtDAL::PropertyRegHelper<dtDAL::PropertyContainer&, BaseAIControls> RegHelperType;
      RegHelperType propReg(pc, this, group);

      DT_REGISTER_PROPERTY(Thrust, "Our current scalar thrust.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(Lift, "Our current scalar lift.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(Yaw, "Our current scalar yaw.", RegHelperType, propReg);
      
    }

   //////////////////////////////////////////////////////////////////////////
   //BaseSteeringTargeter
   //////////////////////////////////////////////////////////////////////////
   BaseSteeringTargeter::BaseSteeringTargeter()
   {
   }

   BaseSteeringTargeter::~BaseSteeringTargeter()
   {
   }

   bool BaseSteeringTargeter::GetGoal(const BaseAIGameState& current_state, BaseAIGoalState& result)
   {
      if(!mPointOfInterest.empty())
      {
         result.SetPos(Top());
         Pop();
      }

      return true;
   }

   void BaseSteeringTargeter::Push(const osg::Vec3& pos)
   {
      mPointOfInterest.push(pos);
   }

   void BaseSteeringTargeter::Pop()
   {
      mPointOfInterest.pop();
   }

   const osg::Vec3& BaseSteeringTargeter::Top() const
   {
      return mPointOfInterest.top();
   }


   //////////////////////////////////////////////////////////////////////////
   //BaseSteeringDecomposer
   //////////////////////////////////////////////////////////////////////////
   BaseSteeringDecomposer::BaseSteeringDecomposer()
   {
   }

   BaseSteeringDecomposer::~BaseSteeringDecomposer()
   {
   }

   void BaseSteeringDecomposer::Decompose(const BaseAIGameState& current_state, BaseAIGoalState& result) const
   {
   }

   //////////////////////////////////////////////////////////////////////////
   //BaseSteeringConstraint
   //////////////////////////////////////////////////////////////////////////
   BaseSteeringConstraint::BaseSteeringConstraint()
   {
   }

   BaseSteeringConstraint::~BaseSteeringConstraint()
   {
   }

   bool BaseSteeringConstraint::WillViolate(const BaseClass::PathType& pathToFollow) const
   {
      return false;
   }

   void BaseSteeringConstraint::Suggest(const BaseClass::PathType& pathToFollow, const BaseAIGameState& current_state, BaseAIGoalState& result) const
   {
   }


   //////////////////////////////////////////////////////////////////////////
   //BombDive
   //////////////////////////////////////////////////////////////////////////
   void BombDive::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   { 

      osg::Vec3 vel = current_state.GetVel();
      vel.normalize();
      osg::Vec3 dirToTarget = current_goal.GetPos() - current_state.GetPos();
      dirToTarget.normalize();
      float dot = dirToTarget * vel;
      dtUtil::Clamp(dot, 0.0f, 1.0f);

      if(current_state.GetVel().length() < mSpeed)
      {
         result.SetThrust(2.0f * dot);
      }
      else
      {
         result.SetThrust(0.0f);
      }
   }



   //////////////////////////////////////////////////////////////////////////
   //Align
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

   osg::Vec3 Align::GetTargetPosition(float dt, const BaseAIGoalState& goal)
   {
      //project our target forward in time if it has a velocity
      osg::Vec3 targetPos = goal.GetPos();
      //if(goal.HasLinearVelocity())
      //{
      //   targetPos += goal.GetLinearVelocity() * dt;
      //}
      return targetPos;
   }

   float Align::GetTargetForward(float dt, const osg::Vec3& targetPos, const BaseAIGoalState& current_goal, const BaseAIGameState& current_state, osg::Vec3& vec_in)
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
      //float dist = GetTargetForward(lookAhead, targetPos, current_goal, current_state, goalForward);

      osg::Vec3 currForward = current_state.GetForward();

      float thetaAngle = (current_state.GetAngularVel() * lookAhead);
      osg::Matrix rotation = osg::Matrix::rotate(thetaAngle, osg::Vec3(0.0, 0.0, 1.0));
      currForward = osg::Matrix::transform3x3(currForward, rotation); 
      currForward.normalize();

      float dot = goalForward * currForward;
      float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);

      float angle = acos(dot);
      if(angle > 0.05f)
      {     
         float yaw = angle / fabs(current_state.GetAngularVel());
         dtUtil::Clamp(yaw, 0.0001f, mTimeToTarget);
         yaw /= mTimeToTarget;
         yaw = Sgn(sign) * yaw;

         if(!dtUtil::IsFinite(yaw)) 
         {
            yaw = 0.0f;
         }

         result.SetYaw(yaw);
      }  
      else
      {
         result.SetYaw(0.0f);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   //Follow Path
   //////////////////////////////////////////////////////////////////////////
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
      //float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);

      angle = acos(dot);

      float timeRemaining = dist / current_state.GetVel().length();

      dtUtil::Clamp(timeRemaining, 0.00001f, mTimeToTarget);
      timeRemaining /= mTimeToTarget;

      osg::Vec3 velNorm = current_state.GetVel();
      velNorm.normalize();

      float thrustSign = velNorm * goalForward;
      float thrust = 0.0f;

      if(thrustSign < -0.75f && dot < -0.75f) 
      {
         thrust = -1.0f;
      }
      else 
      {
         thrust = timeRemaining * dtUtil::MapRangeValue(angle, 0.0f, float(osg::PI), mMaxSpeed, mMinSpeed);
      }

      if(!dtUtil::IsFinite(thrust)) 
      {
         thrust = 0.0f;
      }

      result.SetThrust(thrust);

      //compute height
      float zVel = current_state.GetVel()[2];
      if(fabs(zVel) < 0.0001) zVel = BaseClass::Sgn(zVel) * 0.0001;

      float heightLookAhead = zVel * mTimeToTargetHeight;

      float heightDiff = goalPos[2] - (pos[2] + heightLookAhead);
      float remainingFallTime = fabs(heightDiff / zVel);
      dtUtil::Clamp(remainingFallTime, 0.00001f, mTimeToTarget);
      remainingFallTime /= mTimeToTarget;

      float gConstant = 0.5f;
      float sgnHeightDiff = BaseClass::Sgn(heightDiff);
      float lift = remainingFallTime * sgnHeightDiff;
      if(!dtUtil::IsFinite(lift)) 
      {
         lift = 0.0f;
      }
      
      //dtUtil::Clamp(lift, 0.25f, 1.0f);

      result.SetLift(gConstant + lift);
   }


} //namespace NetDemo
