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

#ifndef DELTA_AISPACESHIP_H
#define DELTA_AISPACESHIP_H

#include <AIUtility.h>
#include <DemoExport.h>
#include <EnemyAIHelper.h>
 
#include <dtDAL/propertymacros.h>
#include <dtAI/controllable.h>
#include <dtAI/steeringbehavior.h>
#include <dtAI/steeringpipeline.h>
#include <osg/Matrix>
#include <osg/Vec3>

#include <stack>

namespace NetDemo
{
   struct NETDEMO_EXPORT SpaceShipState
   {
      SpaceShipState();
      ~SpaceShipState();
      
      DECLARE_PROPERTY(osg::Vec3, Pos);
      DECLARE_PROPERTY(osg::Vec3, Forward);
      DECLARE_PROPERTY(osg::Vec3, Up);

      DECLARE_PROPERTY(osg::Vec3, Vel);
      DECLARE_PROPERTY(osg::Vec3, Accel);

      DECLARE_PROPERTY(float, AngularVel);
      DECLARE_PROPERTY(float, AngularAccel);

      DECLARE_PROPERTY(float, VerticalVel);
      DECLARE_PROPERTY(float, VerticalAccel);

      DECLARE_PROPERTY(float, Pitch);
      DECLARE_PROPERTY(float, Roll);
      
      DECLARE_PROPERTY(float, TimeStep);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   struct NETDEMO_EXPORT SpaceShipGoalState: public SpaceShipState
   {
      SpaceShipGoalState();
      ~SpaceShipGoalState();

      DECLARE_PROPERTY(float, DragCoef);
      DECLARE_PROPERTY(float, AngularDragCoef);
      DECLARE_PROPERTY(float, VerticalDragCoef);

      DECLARE_PROPERTY(float, MaxVel);
      DECLARE_PROPERTY(float, MaxAccel);
      
      DECLARE_PROPERTY(float, MaxAngularVel);
      DECLARE_PROPERTY(float, MaxAngularAccel);
      
      DECLARE_PROPERTY(float, MaxVerticalVel);
      DECLARE_PROPERTY(float, MaxVerticalAccel);

      DECLARE_PROPERTY(float, MaxPitch);
      DECLARE_PROPERTY(float, MaxRoll);
      
      DECLARE_PROPERTY(float, MaxTiltPerSecond);
      DECLARE_PROPERTY(float, MaxRollPerSecond);
      
      DECLARE_PROPERTY(float, MinElevation);
      DECLARE_PROPERTY(float, MaxElevation);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   struct SpaceShipControls
   {
      SpaceShipControls();
      ~SpaceShipControls();

      //these are the control inputs
      //all are floats from 1 to -1 
      //which represents percentage of maximum
      DECLARE_PROPERTY(float, Thrust);
      DECLARE_PROPERTY(float, Lift);
      DECLARE_PROPERTY(float, Yaw);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   //////////////////////////////////////////////////////////////////////////
   //SpaceShipSteeringBehavior
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::SteeringBehavior<SpaceShipGoalState, SpaceShipState, SpaceShipControls> SpaceShipSteeringBehavior;


   class SpaceShipTargeter: public dtAI::Targeter<SpaceShipState, SpaceShipGoalState>
   {
      public:
         typedef dtAI::Targeter<SpaceShipState, SpaceShipGoalState> BaseClass;

         SpaceShipTargeter();
         virtual ~SpaceShipTargeter();

         void Pop();
         const osg::Vec3& Top() const;
         void Push(const osg::Vec3& pos);

         /*virtual*/ bool GetGoal(const SpaceShipState& current_state, SpaceShipGoalState& result);

      private:

         std::stack<osg::Vec3> mPointOfInterest;

   };

   class SpaceShipDecomposer: public dtAI::Decomposer<SpaceShipState, SpaceShipGoalState>
   {
   public:
      typedef dtAI::Decomposer<SpaceShipState, SpaceShipGoalState> BaseClass;

      SpaceShipDecomposer();
      ~SpaceShipDecomposer();

      /*virtual*/ void Decompose(const SpaceShipState& current_state, SpaceShipGoalState& result) const;
   };

   class SpaceShipConstraint: public dtAI::Constraint<SpaceShipState, SpaceShipGoalState>
   {
   public:
      typedef dtAI::Constraint<SpaceShipState, SpaceShipGoalState> BaseClass;

      SpaceShipConstraint();
      ~SpaceShipConstraint();

      /*virtual*/ bool WillViolate(const BaseClass::PathType& pathToFollow) const;
      /*virtual*/ void Suggest(const BaseClass::PathType& pathToFollow, const SpaceShipState& current_state, SpaceShipGoalState& result) const;
   };


   class SpaceShipControllable: public dtAI::Controllable<SpaceShipState, SpaceShipGoalState, SpaceShipControls>
   {
   public:
      typedef dtAI::Controllable<SpaceShipState, SpaceShipGoalState, SpaceShipControls> BaseClass;

      SpaceShipControllable();
      ~SpaceShipControllable();

      /*virtual*/ bool FindPath(const AIState& fromState, const AIGoal& goal, AIPath& resultingPath) const;
      /*virtual*/ void UpdateState(float dt, const AIControlState& steerData);

      /*virtual*/ void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
      {
         BaseClass::RegisterProperties(pc, group);
      };

      static void InitControllable(const osg::Matrix& matIn, SpaceShipControllable& stateIn);
      static void ResetState(const osg::Matrix& matIn, SpaceShipState& spaceShipState);
      static void SetState(const osg::Matrix& matIn, SpaceShipState& spaceShipState);
      static void OrthoNormalize(SpaceShipState& stateIn);
      static void SetMatrix(const SpaceShipState& stateIn, osg::Matrix& result);

   protected:


      void UpdateHeading(const SpaceShipControls& controls);
      void UpdatePosition(const SpaceShipControls& controls);
      void UpdateVelocity(const SpaceShipControls& controls);
      void UpdateAngularVelocity(const SpaceShipControls& controls);
      void UpdateVerticalVelocity(const SpaceShipControls& controls);
      void UpdateTilt(const SpaceShipControls& controls, osg::Vec3& tilt);
      void UpdateRoll(const SpaceShipControls& controls, osg::Vec3& roll);
      float Clamp(float x, float from, float to);
      float Dampen(float last, float current, float max, float falloff);

      DECLARE_PROPERTY(float, TimeStep)

   };

   typedef dtAI::SteeringPipeline<SpaceShipControllable> SpaceShipAIModel;

   class NETDEMO_EXPORT SpaceShipAIHelper: public EnemyAIHelper
   {
   public:
      typedef EnemyAIHelper BaseClass;
      SpaceShipAIHelper();

      /*virtual*/ void OnInit(const EnemyDescriptionActor& desc);
      /*virtual*/ void Spawn();
      /*virtual*/ void Update(float dt);

      /*virtual*/ void PreSync(const dtCore::Transform& trans);
      /*virtual*/ void PostSync(dtCore::Transform& trans) const;

      void OutputControl(const SpaceShipControllable::PathType& pathToFollow, const SpaceShipControllable::StateType& current_state, SpaceShipControllable::ControlType& result) const;

      void SetCurrentTarget(dtCore::Transformable& target);

      SpaceShipAIModel& GetAIModel();
      const SpaceShipAIModel& GetAIModel() const;

      SpaceShipControllable& GetAIControllable();
      const SpaceShipControllable& GetAIControllable() const;

   protected:
      SpaceShipAIHelper(const SpaceShipAIHelper&);  //not implemented by design
      SpaceShipAIHelper& operator=(const SpaceShipAIHelper&);  //not implemented by design
      ~SpaceShipAIHelper();

      /*virtual*/ void RegisterStates();
      /*virtual*/ void CreateStates();
      /*virtual*/ void SetupTransitions();
      /*virtual*/ void SetupFunctors();

      /*virtual*/ void SelectState(float dt);

      virtual void Attack(float dt);

   private:

      //float GetDistance(const osg::Vec3& pos);

      SpaceShipTargeter mDefaultTargeter;
      
      SpaceShipSteeringBehavior* mDefaultBehavior;

      SpaceShipControllable mAIControllable;
      SpaceShipAIModel mAIModel;
   };



   /**
   * Align is used to align our orientation with the current dtAI::KinematicGoal's orientation (rotation)
   */
   class Align: public SpaceShipSteeringBehavior
   {
   public:
      typedef SpaceShipSteeringBehavior BaseClass;

      Align(float lookAhead, float timeToTarget)
         : mLookAhead(lookAhead)
         , mTimeToTarget(timeToTarget)
      {}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   protected:
      float Sgn(float x);
      osg::Vec3 GetTargetPosition(float dt, const SpaceShipGoalState& goal);
      float GetTargetForward(float dt, const osg::Vec3& targetPos, const SpaceShipGoalState& current_goal, const SpaceShipState& current_state, osg::Vec3& vec_in);

      float mLookAhead, mTimeToTarget;
   };

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

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:

      float mMinSpeed, mMaxSpeed, mLookAhead, mTimeToTarget;
   };

}//namespace NetDemo

#endif // DELTA_AISPACESHIP_H
