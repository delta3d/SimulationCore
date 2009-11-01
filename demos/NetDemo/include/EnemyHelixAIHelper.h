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

#ifndef DELTA_ENEMYHELIXAIHELPER_H
#define DELTA_ENEMYHELIXAIHELPER_H

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
   struct NETDEMO_EXPORT EnemyHelixState
   {
      EnemyHelixState();
      ~EnemyHelixState();
      
      DECLARE_PROPERTY_INLINE(osg::Vec3, Pos);
      DECLARE_PROPERTY_INLINE(osg::Vec3, Forward);
      DECLARE_PROPERTY_INLINE(osg::Vec3, Up);

      DECLARE_PROPERTY_INLINE(osg::Vec3, Vel);
      DECLARE_PROPERTY_INLINE(osg::Vec3, Accel);

      DECLARE_PROPERTY_INLINE(float, AngularVel);
      DECLARE_PROPERTY_INLINE(float, AngularAccel);

      DECLARE_PROPERTY_INLINE(float, VerticalVel);
      DECLARE_PROPERTY_INLINE(float, VerticalAccel);

      DECLARE_PROPERTY_INLINE(float, Pitch);
      DECLARE_PROPERTY_INLINE(float, Roll);
      
      DECLARE_PROPERTY_INLINE(float, TimeStep);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   struct NETDEMO_EXPORT EnemyHelixGoalState: public EnemyHelixState
   {
      EnemyHelixGoalState();
      ~EnemyHelixGoalState();

      DECLARE_PROPERTY_INLINE(float, DragCoef);
      DECLARE_PROPERTY_INLINE(float, AngularDragCoef);
      DECLARE_PROPERTY_INLINE(float, VerticalDragCoef);

      DECLARE_PROPERTY_INLINE(float, MaxVel);
      DECLARE_PROPERTY_INLINE(float, MaxAccel);
      
      DECLARE_PROPERTY_INLINE(float, MaxAngularVel);
      DECLARE_PROPERTY_INLINE(float, MaxAngularAccel);
      
      DECLARE_PROPERTY_INLINE(float, MaxVerticalVel);
      DECLARE_PROPERTY_INLINE(float, MaxVerticalAccel);

      DECLARE_PROPERTY_INLINE(float, MaxPitch);
      DECLARE_PROPERTY_INLINE(float, MaxRoll);
      
      DECLARE_PROPERTY_INLINE(float, MaxTiltPerSecond);
      DECLARE_PROPERTY_INLINE(float, MaxRollPerSecond);
      
      DECLARE_PROPERTY_INLINE(float, MinElevation);
      DECLARE_PROPERTY_INLINE(float, MaxElevation);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   struct EnemyHelixControls
   {
      EnemyHelixControls();
      ~EnemyHelixControls();

      //these are the control inputs
      //all are floats from 1 to -1 
      //which represents percentage of maximum
      DECLARE_PROPERTY_INLINE(float, Thrust);
      DECLARE_PROPERTY_INLINE(float, Lift);
      DECLARE_PROPERTY_INLINE(float, Yaw);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   //////////////////////////////////////////////////////////////////////////
   //EnemyHelixSteeringBehavior
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::SteeringBehavior<EnemyHelixGoalState, EnemyHelixState, EnemyHelixControls> EnemyHelixSteeringBehavior;


   class EnemyHelixTargeter: public dtAI::Targeter<EnemyHelixState, EnemyHelixGoalState>
   {
      public:
         typedef dtAI::Targeter<EnemyHelixState, EnemyHelixGoalState> BaseClass;

         EnemyHelixTargeter();
         virtual ~EnemyHelixTargeter();

         void Pop();
         const osg::Vec3& Top() const;
         void Push(const osg::Vec3& pos);

         /*virtual*/ bool GetGoal(const EnemyHelixState& current_state, EnemyHelixGoalState& result);

      private:

         std::stack<osg::Vec3> mPointOfInterest;

   };

   class EnemyHelixDecomposer: public dtAI::Decomposer<EnemyHelixState, EnemyHelixGoalState>
   {
   public:
      typedef dtAI::Decomposer<EnemyHelixState, EnemyHelixGoalState> BaseClass;

      EnemyHelixDecomposer();
      ~EnemyHelixDecomposer();

      /*virtual*/ void Decompose(const EnemyHelixState& current_state, EnemyHelixGoalState& result) const;
   };

   class EnemyHelixConstraint: public dtAI::Constraint<EnemyHelixState, EnemyHelixGoalState>
   {
   public:
      typedef dtAI::Constraint<EnemyHelixState, EnemyHelixGoalState> BaseClass;

      EnemyHelixConstraint();
      ~EnemyHelixConstraint();

      /*virtual*/ bool WillViolate(const BaseClass::PathType& pathToFollow) const;
      /*virtual*/ void Suggest(const BaseClass::PathType& pathToFollow, const EnemyHelixState& current_state, EnemyHelixGoalState& result) const;
   };


   class EnemyHelixControllable: public dtAI::Controllable<EnemyHelixState, EnemyHelixGoalState, EnemyHelixControls>
   {
   public:
      typedef dtAI::Controllable<EnemyHelixState, EnemyHelixGoalState, EnemyHelixControls> BaseClass;

      EnemyHelixControllable();
      ~EnemyHelixControllable();

      /*virtual*/ bool FindPath(const AIState& fromState, const AIGoal& goal, AIPath& resultingPath) const;
      /*virtual*/ void UpdateState(float dt, const AIControlState& steerData);

      /*virtual*/ void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
      {
         BaseClass::RegisterProperties(pc, group);
      };

      static void InitControllable(const osg::Matrix& matIn, EnemyHelixControllable& stateIn);
      static void ResetState(const osg::Matrix& matIn, EnemyHelixState& EnemyHelixState);
      static void SetState(const osg::Matrix& matIn, EnemyHelixState& EnemyHelixState);
      static void OrthoNormalize(EnemyHelixState& stateIn);
      static void SetMatrix(const EnemyHelixState& stateIn, osg::Matrix& result);

   protected:


      void UpdateHeading(const EnemyHelixControls& controls);
      void UpdatePosition(const EnemyHelixControls& controls);
      void UpdateVelocity(const EnemyHelixControls& controls);
      void UpdateAngularVelocity(const EnemyHelixControls& controls);
      void UpdateVerticalVelocity(const EnemyHelixControls& controls);
      void UpdateTilt(const EnemyHelixControls& controls, osg::Vec3& tilt);
      void UpdateRoll(const EnemyHelixControls& controls, osg::Vec3& roll);
      float Clamp(float x, float from, float to);
      float Dampen(float last, float current, float max, float falloff);

      DECLARE_PROPERTY_INLINE(float, TimeStep)

   };

   typedef dtAI::SteeringPipeline<EnemyHelixControllable> EnemyHelixAIModel;

   class NETDEMO_EXPORT EnemyHelixAIHelper: public EnemyAIHelper
   {
   public:
      typedef EnemyAIHelper BaseClass;
      EnemyHelixAIHelper();

      /*virtual*/ void OnInit(const EnemyDescriptionActor* desc);
      /*virtual*/ void Spawn();
      /*virtual*/ void Update(float dt);

      /*virtual*/ void PreSync(const dtCore::Transform& trans);
      /*virtual*/ void PostSync(dtCore::Transform& trans) const;

      void OutputControl(const EnemyHelixControllable::PathType& pathToFollow, const EnemyHelixControllable::StateType& current_state, EnemyHelixControllable::ControlType& result) const;

      void SetCurrentTarget(dtCore::Transformable& target);

      EnemyHelixAIModel& GetAIModel();
      const EnemyHelixAIModel& GetAIModel() const;

      EnemyHelixControllable& GetAIControllable();
      const EnemyHelixControllable& GetAIControllable() const;

   protected:
      EnemyHelixAIHelper(const EnemyHelixAIHelper&);  //not implemented by design
      EnemyHelixAIHelper& operator=(const EnemyHelixAIHelper&);  //not implemented by design
      ~EnemyHelixAIHelper();

      /*virtual*/ void RegisterStates();
      /*virtual*/ void CreateStates();
      /*virtual*/ void SetupTransitions();
      /*virtual*/ void SetupFunctors();

      /*virtual*/ void SelectState(float dt);

      virtual void Attack(float dt);

   private:

      //float GetDistance(const osg::Vec3& pos);

      EnemyHelixTargeter mDefaultTargeter;
      
      EnemyHelixSteeringBehavior* mDefaultBehavior;

      EnemyHelixControllable mAIControllable;
      EnemyHelixAIModel mAIModel;
   };



   /**
   * HelixAlign is used to Align our orientation with the current dtAI::KinematicGoal's orientation (rotation)
   */
   class HelixAlign: public EnemyHelixSteeringBehavior
   {
   public:
      typedef EnemyHelixSteeringBehavior BaseClass;

      HelixAlign(float lookAhead, float timeToTarget)
         : mLookAhead(lookAhead)
         , mTimeToTarget(timeToTarget)
      {}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   protected:
      float Sgn(float x);
      osg::Vec3 GetTargetPosition(float dt, const EnemyHelixGoalState& goal);
      float GetTargetForward(float dt, const osg::Vec3& targetPos, const EnemyHelixGoalState& current_goal, const EnemyHelixState& current_state, osg::Vec3& vec_in);

      float mLookAhead, mTimeToTarget;
   };

   /**
   * Follow path can be used to follow waypoints
   */
   class HelixFollowPath: public HelixAlign
   {
   public:
      typedef HelixAlign BaseClass;

      HelixFollowPath(float minSpeed, float maxSpeed, float lookAhead, float timeToTarget, float lookAheadRot, float timeToTargetRot)
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

#endif // DELTA_ENEMYHELIXAIHELPER_H
