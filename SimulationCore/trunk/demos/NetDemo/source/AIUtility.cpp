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
   void DoNothing::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   { 
   }

   void BombDive::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   { 
      osg::Vec3 targetPos = current_goal.GetPosition();
      osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(current_state.mTransform, 3);
      pos = (targetPos - pos);
      pos.normalize();
      result.mLinearVelocity = pos * mSpeed;
   }


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

   osg::Vec3 Align::GetTargetPosition(float dt, BaseClass::ConstKinematicGoalParam goal)
   {
      //project our target forward in time if it has a velocity
      osg::Vec3 targetPos = goal.GetPosition();
      if(goal.HasLinearVelocity())
      {
         targetPos += goal.GetLinearVelocity() * dt;
      }
      return targetPos;
   }

   float Align::GetTargetForward(float dt, const osg::Vec3& targetPos, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, osg::Vec3& vec_in)
   {
      osg::Vec3 projectedPos = dtUtil::MatrixUtil::GetRow3(current_state.mTransform, 3) + (current_state.mLinearVelocity * dt);   
         
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

      osg::Vec3 currForward = dtUtil::MatrixUtil::GetRow3(current_state.mTransform, 1);
      currForward.normalize();

      float thetaAngle = (current_state.mAngularVelocity[0] * lookAhead); 
      osg::Matrix rotation = osg::Matrix::rotate( thetaAngle, osg::Vec3(0.0, 0.0, 1.0));
      currForward = osg::Matrix::transform3x3(currForward, rotation); 
      currForward.normalize();

      float dot = goalForward * currForward;
      float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);
      
      float angle = acos(dot);
      if(angle > 0.1f)
      {     
         float yaw = angle / fabs(current_state.mAngularVelocity[0]);
         dtUtil::Clamp(yaw, 0.0001f, mTimeToTarget);
         yaw /= mTimeToTarget;
         result.mYaw = Sgn(sign) * yaw;         
      }  
   }

   void FollowPath::Think(float dt, Align::BaseClass::ConstKinematicGoalParam current_goal, BaseClass::BaseClass::ConstKinematicParam current_state, BaseClass::BaseClass::SteeringOutByRefParam result)
   {
      float lookAhead = mLookAhead * dt;
      BaseClass::Think(lookAhead, current_goal, current_state, result);

      osg::Vec3 targetPos = BaseClass::GetTargetPosition(lookAhead, current_goal);
      osg::Vec3 goalForward;
      float dist = GetTargetForward(lookAhead, targetPos, current_goal, current_state, goalForward);
      osg::Vec3 currForward = dtUtil::MatrixUtil::GetRow3(current_state.mTransform, 1);
      currForward.normalize();

      float angle = 0.0f;

      float dot = goalForward * currForward;
      float sign = (currForward[0] * goalForward[1]) - (currForward[1] * goalForward[0]);
      
      angle = acos(dot);

      float timeRemaining = dist / current_state.mLinearVelocity.length();

      dtUtil::Clamp(timeRemaining, 0.00001f, mTimeToTarget);
      timeRemaining /= mTimeToTarget;

      result.mThrust = timeRemaining * dtUtil::MapRangeValue(angle, 0.0f, float(osg::PI), mMaxSpeed, mMinSpeed);

      //compute height
      osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(current_state.mTransform, 3);
      float heightDiff = current_goal.GetPosition()[2] - (pos[2] + (current_state.mLinearVelocity[2] * dt));

      dtUtil::Clamp(heightDiff, -1.0f, 1.0f);
      float absHeightDiff = fabs(heightDiff);
      result.mLift = BaseClass::Sgn(heightDiff) * absHeightDiff * absHeightDiff;
   }

} //namespace NetDemo
