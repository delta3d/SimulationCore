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

#include <stack>

#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Matrix>

#include <DemoExport.h>

#include <dtAI/steeringutility.h>
#include <dtAI/steeringbehavior.h>
#include <dtAI/controllable.h>
#include <dtAI/steeringpipeline.h>

//for the steering pipeline
#include <dtUtil/objectfactory.h>

//include needed just for getting to PathData internal to BezierController
//todo- move PathData out of BezierController
#include <dtABC/beziercontroller.h>

//used for the waypoint array typedef WaypointPath
//todo- move WaypointArray out of AIPluginInterface
#include <dtAI/aiplugininterface.h>

//use a property container to hold ai data
#include <dtUtil/getsetmacros.h>

#include <dtUtil/log.h>


namespace NetDemo
{
   struct BaseAIGameState
   {
      BaseAIGameState();
      ~BaseAIGameState();

      DT_DECLARE_ACCESSOR_INLINE(osg::Vec3, Pos);
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec3, Forward);
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec3, Up);

      DT_DECLARE_ACCESSOR_INLINE(osg::Vec3, Vel);
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec3, Accel);

      DT_DECLARE_ACCESSOR_INLINE(float, AngularVel);
      DT_DECLARE_ACCESSOR_INLINE(float, AngularAccel);

      DT_DECLARE_ACCESSOR_INLINE(float, VerticalVel);
      DT_DECLARE_ACCESSOR_INLINE(float, VerticalAccel);

      DT_DECLARE_ACCESSOR_INLINE(float, Pitch);
      DT_DECLARE_ACCESSOR_INLINE(float, Roll);

      DT_DECLARE_ACCESSOR_INLINE(float, TimeStep);

      DT_DECLARE_ACCESSOR_INLINE(float, Thrusters);
      

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   struct BaseAIGoalState: public BaseAIGameState
   {
      BaseAIGoalState();
      ~BaseAIGoalState();

      //todo- make properties for using each constraint and apply constraints 
      //       in BaseAIHelper::PostSync()

      //DT_DECLARE_ACCESSOR_INLINE(bool, UseDrag);
      DT_DECLARE_ACCESSOR_INLINE(float, DragCoef);
      DT_DECLARE_ACCESSOR_INLINE(float, AngularDragCoef);
      DT_DECLARE_ACCESSOR_INLINE(float, VerticalDragCoef);

      //DT_DECLARE_ACCESSOR_INLINE(bool, UseMaxVel);
      DT_DECLARE_ACCESSOR_INLINE(float, MaxVel);
      
      //DT_DECLARE_ACCESSOR_INLINE(bool, UseMaxAccel);
      DT_DECLARE_ACCESSOR_INLINE(float, MaxAccel);

      DT_DECLARE_ACCESSOR_INLINE(float, MaxAngularVel);
      DT_DECLARE_ACCESSOR_INLINE(float, MaxAngularAccel);

      DT_DECLARE_ACCESSOR_INLINE(float, MaxVerticalVel);
      DT_DECLARE_ACCESSOR_INLINE(float, MaxVerticalAccel);

      DT_DECLARE_ACCESSOR_INLINE(float, MaxPitch);
      DT_DECLARE_ACCESSOR_INLINE(float, MaxRoll);

      DT_DECLARE_ACCESSOR_INLINE(float, MaxTiltPerSecond);
      DT_DECLARE_ACCESSOR_INLINE(float, MaxRollPerSecond);

      DT_DECLARE_ACCESSOR_INLINE(float, MinElevation);
      DT_DECLARE_ACCESSOR_INLINE(float, MaxElevation);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   struct BaseAIControls
   {
      BaseAIControls();
      ~BaseAIControls();

      //these are the control inputs
      //all are floats from 1 to -1 
      //which represents percentage of maximum
      DT_DECLARE_ACCESSOR_INLINE(float, Thrust);
      DT_DECLARE_ACCESSOR_INLINE(float, Lift);
      DT_DECLARE_ACCESSOR_INLINE(float, Yaw);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   //////////////////////////////////////////////////////////////////////////
   //BaseAIControllable
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::Controllable<BaseAIGameState, BaseAIGoalState, BaseAIControls> BaseAIControllable;

   //////////////////////////////////////////////////////////////////////////
   //BaseAISteeringBehavior
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::SteeringBehavior<BaseAIGoalState, BaseAIGameState, BaseAIControls> BaseAISteeringBehavior;


   //////////////////////////////////////////////////////////////////////////
   //BaseSteeringTargeter
   //////////////////////////////////////////////////////////////////////////
   class BaseSteeringTargeter: public dtAI::Targeter<BaseAIGameState, BaseAIGoalState>
   {
   public:
      typedef dtAI::Targeter<BaseAIGameState, BaseAIGoalState> BaseClass;

      BaseSteeringTargeter();
      virtual ~BaseSteeringTargeter();

      void Pop();
      const osg::Vec3& Top() const;
      void Push(const osg::Vec3& pos);

      /*virtual*/ bool GetGoal(const BaseAIGameState& current_state, BaseAIGoalState& result);

   private:

      std::stack<osg::Vec3> mPointOfInterest;

   };

   class BaseSteeringDecomposer: public dtAI::Decomposer<BaseAIGameState, BaseAIGoalState>
   {
   public:
      typedef dtAI::Decomposer<BaseAIGameState, BaseAIGoalState> BaseClass;

      BaseSteeringDecomposer();
      ~BaseSteeringDecomposer();

      /*virtual*/ void Decompose(const BaseAIGameState& current_state, BaseAIGoalState& result) const;
   };

   class BaseSteeringConstraint: public dtAI::Constraint<BaseAIGameState, BaseAIGoalState>
   {
   public:
      typedef dtAI::Constraint<BaseAIGameState, BaseAIGoalState> BaseClass;

      BaseSteeringConstraint();
      ~BaseSteeringConstraint();

      /*virtual*/ bool WillViolate(const BaseClass::PathType& pathToFollow) const;
      /*virtual*/ void Suggest(const BaseClass::PathType& pathToFollow, const BaseAIGameState& current_state, BaseAIGoalState& result) const;
   };


   //our default behavior
   class DoNothing: public BaseAISteeringBehavior
   {
   public:

      typedef BaseAISteeringBehavior BaseClass;

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
      {
      }

   };

   /**
   * Align is used to align our orientation with the current dtAI::KinematicGoal's orientation (rotation)
   */
   class Align: public BaseAISteeringBehavior
   {
   public:
      typedef BaseAISteeringBehavior BaseClass;

      Align(float lookAhead, float timeToTarget)
         : mLookAhead(lookAhead)
         , mTimeToTarget(timeToTarget)
      {}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   protected:
      float Sgn(float x);
      osg::Vec3 GetTargetPosition(float dt, const BaseAIGoalState& goal);
      float GetTargetForward(float dt, const osg::Vec3& targetPos, const BaseAIGoalState& current_goal, const BaseAIGameState& current_state, osg::Vec3& vec_in);

      float mLookAhead, mTimeToTarget;
   };

   //this is currently used by the enemy mine
   class BombDive: public BaseAISteeringBehavior
   {
   public:
      typedef BaseAISteeringBehavior BaseClass;

      BombDive(float maxVel): mSpeed(maxVel){}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:
      float mSpeed;
   };

   /**
   * Follow path can be used to follow waypoints
   */
   class FollowPath: public Align
   {
   public:
      typedef Align BaseClass;

      FollowPath(float minSpeed, float maxSpeed, float lookAhead, float timeToTarget, float timeToTargetHeight, float lookAheadRot, float timeToTargetRot)
         : BaseClass(lookAheadRot, timeToTargetRot)
         , mMinSpeed(minSpeed)
         , mMaxSpeed(maxSpeed)
         , mLookAhead(lookAhead)
         , mTimeToTarget(timeToTarget)
         , mTimeToTargetHeight(timeToTargetHeight)
      {}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:

      float mMinSpeed, mMaxSpeed, mLookAhead, mTimeToTarget, mTimeToTargetHeight;
   };


}//namespace NetDemo

#endif //NETDEMO_AIUTILITY_H
