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

#ifndef NETDEMO_AIUTILITY_H
#define NETDEMO_AIUTILITY_H

#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Matrix>

#include <dtAI/steeringutility.h>
#include <dtAI/steeringbehavior.h>


namespace NetDemo
{

   struct Kinematic 
   {
      osg::Matrix mTransform;
      osg::Vec3 mLinearVelocity;
      osg::Vec3 mAngularVelocity;
   };

   struct SteeringOutput
   { 
      //these are the control inputs
      //all are floats from 1 to -1 
      //which represents percentage of maximum
      float mThrust, mLift, mYaw;

      void Reset()
      {
         mThrust = 0.0f;
         mLift = 0.0f;
         mYaw = 0.0f;
      }
   };


   typedef dtAI::SteeringBehavior<dtAI::KinematicGoal, Kinematic, SteeringOutput> SteeringBehaviorType;

   //class ErrorCondition: public dtUtil::Enumeration
   //{
   //   DECLARE_ENUM(ErrorCondition);
   //public:
   //   ErrorCondition(const std::string&);
   //};


   /**
    * Align is used to align our orientation with the current dtAI::KinematicGoal's orientation (rotation) 
    */
   class Align: public SteeringBehaviorType
   {
   public:
     typedef SteeringBehaviorType BaseClass;
     Align(float lookAhead, float timeToTarget)
        : mLookAhead(lookAhead) 
        , mTimeToTarget(timeToTarget)
     {}

     /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   protected:
      float Sgn(float x);
      osg::Vec3 GetTargetPosition(float dt, BaseClass::ConstKinematicGoalParam goal);
      float GetTargetForward(float dt, const osg::Vec3& targetPos, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, osg::Vec3& vec_in);

      float mLookAhead, mTimeToTarget;
   };

   /**
   * DiveBomb is an AI Behavior to do a Kamikaze attack on a target
   */
   //class DiveBomb: public Align
   //{
   //public:
   //   typedef Align BaseClass;

   //   DiveBomb(){}

   //   /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);
   //};

   /**
    * Follow path can be used to follow waypoints
    */
   class FollowPath: public Align
   {
   public:
      typedef Align BaseClass;

      FollowPath(float minSpeed, float maxSpeed, float lookAhead, float timeToTarget, float lookAheadRot, float timeToTargetRot)
         : BaseClass(lookAheadRot, timeToTargetRot)
         , mMinSpeed(minSpeed)
         , mMaxSpeed(maxSpeed)
         , mLookAhead(lookAhead) 
         , mTimeToTarget(timeToTarget)
      {}

     /*virtual*/ void Think(float dt, Align::BaseClass::ConstKinematicGoalParam current_goal, BaseClass::BaseClass::ConstKinematicParam current_state, BaseClass::BaseClass::SteeringOutByRefParam result);

   private: 

      float mMinSpeed, mMaxSpeed, mLookAhead, mTimeToTarget;
   };

}//namespace NetDemo

#endif //NETDEMO_AIUTILITY_H
