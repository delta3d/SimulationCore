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

#ifndef DELTA_AIEnemyMothership_H
#define DELTA_AIEnemyMothership_H

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
   struct NETDEMO_EXPORT EnemyMothershipState
   {
      EnemyMothershipState();
      ~EnemyMothershipState();
      
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

   struct NETDEMO_EXPORT EnemyMothershipGoalState: public EnemyMothershipState
   {
      EnemyMothershipGoalState();
      ~EnemyMothershipGoalState();

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

   struct EnemyMothershipControls
   {
      EnemyMothershipControls();
      ~EnemyMothershipControls();

      //these are the control inputs
      //all are floats from 1 to -1 
      //which represents percentage of maximum
      DECLARE_PROPERTY_INLINE(float, Thrust);
      DECLARE_PROPERTY_INLINE(float, Lift);
      DECLARE_PROPERTY_INLINE(float, Yaw);

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
   };

   //////////////////////////////////////////////////////////////////////////
   //EnemyMothershipSteeringBehavior
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::SteeringBehavior<EnemyMothershipGoalState, EnemyMothershipState, EnemyMothershipControls> EnemyMothershipSteeringBehavior;


   class EnemyMothershipTargeter: public dtAI::Targeter<EnemyMothershipState, EnemyMothershipGoalState>
   {
      public:
         typedef dtAI::Targeter<EnemyMothershipState, EnemyMothershipGoalState> BaseClass;

         EnemyMothershipTargeter();
         virtual ~EnemyMothershipTargeter();

         void Pop();
         const osg::Vec3& Top() const;
         void Push(const osg::Vec3& pos);

         /*virtual*/ bool GetGoal(const EnemyMothershipState& current_state, EnemyMothershipGoalState& result);

      private:

         std::stack<osg::Vec3> mPointOfInterest;

   };

   class EnemyMothershipDecomposer: public dtAI::Decomposer<EnemyMothershipState, EnemyMothershipGoalState>
   {
   public:
      typedef dtAI::Decomposer<EnemyMothershipState, EnemyMothershipGoalState> BaseClass;

      EnemyMothershipDecomposer();
      ~EnemyMothershipDecomposer();

      /*virtual*/ void Decompose(const EnemyMothershipState& current_state, EnemyMothershipGoalState& result) const;
   };

   class EnemyMothershipConstraint: public dtAI::Constraint<EnemyMothershipState, EnemyMothershipGoalState>
   {
   public:
      typedef dtAI::Constraint<EnemyMothershipState, EnemyMothershipGoalState> BaseClass;

      EnemyMothershipConstraint();
      ~EnemyMothershipConstraint();

      /*virtual*/ bool WillViolate(const BaseClass::PathType& pathToFollow) const;
      /*virtual*/ void Suggest(const BaseClass::PathType& pathToFollow, const EnemyMothershipState& current_state, EnemyMothershipGoalState& result) const;
   };


   class EnemyMothershipControllable: public dtAI::Controllable<EnemyMothershipState, EnemyMothershipGoalState, EnemyMothershipControls>
   {
   public:
      typedef dtAI::Controllable<EnemyMothershipState, EnemyMothershipGoalState, EnemyMothershipControls> BaseClass;

      EnemyMothershipControllable();
      ~EnemyMothershipControllable();

      /*virtual*/ bool FindPath(const AIState& fromState, const AIGoal& goal, AIPath& resultingPath) const;
      /*virtual*/ void UpdateState(float dt, const AIControlState& steerData);

      /*virtual*/ void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group)
      {
         BaseClass::RegisterProperties(pc, group);
      };

      static void InitControllable(const osg::Matrix& matIn, EnemyMothershipControllable& stateIn);
      static void ResetState(const osg::Matrix& matIn, EnemyMothershipState& EnemyMothershipState);
      static void SetState(const osg::Matrix& matIn, EnemyMothershipState& EnemyMothershipState);
      static void OrthoNormalize(EnemyMothershipState& stateIn);
      static void SetMatrix(const EnemyMothershipState& stateIn, osg::Matrix& result);

   protected:


      void UpdateHeading(const EnemyMothershipControls& controls);
      void UpdatePosition(const EnemyMothershipControls& controls);
      void UpdateVelocity(const EnemyMothershipControls& controls);
      void UpdateAngularVelocity(const EnemyMothershipControls& controls);
      void UpdateVerticalVelocity(const EnemyMothershipControls& controls);
      void UpdateTilt(const EnemyMothershipControls& controls, osg::Vec3& tilt);
      void UpdateRoll(const EnemyMothershipControls& controls, osg::Vec3& roll);
      float Clamp(float x, float from, float to);
      float Dampen(float last, float current, float max, float falloff);

      DECLARE_PROPERTY_INLINE(float, TimeStep)

   };

   typedef dtAI::SteeringPipeline<EnemyMothershipControllable> EnemyMothershipAIModel;

   class NETDEMO_EXPORT EnemyMothershipAIHelper: public EnemyAIHelper
   {
   public:
      typedef EnemyAIHelper BaseClass;
      EnemyMothershipAIHelper();

      /*virtual*/ void OnInit(const EnemyDescriptionActor* desc);
      /*virtual*/ void Spawn();
      /*virtual*/ void Update(float dt);

      /*virtual*/ void PreSync(const dtCore::Transform& trans);
      /*virtual*/ void PostSync(dtCore::Transform& trans) const;

      void OutputControl(const EnemyMothershipControllable::PathType& pathToFollow, const EnemyMothershipControllable::StateType& current_state, EnemyMothershipControllable::ControlType& result) const;

      void SetCurrentTarget(dtCore::Transformable& target);

      EnemyMothershipAIModel& GetAIModel();
      const EnemyMothershipAIModel& GetAIModel() const;

      EnemyMothershipControllable& GetAIControllable();
      const EnemyMothershipControllable& GetAIControllable() const;

   protected:
      EnemyMothershipAIHelper(const EnemyMothershipAIHelper&);  //not implemented by design
      EnemyMothershipAIHelper& operator=(const EnemyMothershipAIHelper&);  //not implemented by design
      ~EnemyMothershipAIHelper();

      /*virtual*/ void RegisterStates();
      /*virtual*/ void CreateStates();
      /*virtual*/ void SetupTransitions();
      /*virtual*/ void SetupFunctors();

      /*virtual*/ void SelectState(float dt);

      virtual void Attack(float dt);

      void ComputeTargetOffset();

   private:

      //float GetDistance(const osg::Vec3& pos);

      osg::Vec3 mTargetOffset;
      EnemyMothershipTargeter mDefaultTargeter;
      
      EnemyMothershipSteeringBehavior* mDefaultBehavior;

      EnemyMothershipControllable mAIControllable;
      EnemyMothershipAIModel mAIModel;
   };



   /**
   * Align is used to align our orientation with the current dtAI::KinematicGoal's orientation (rotation)
   */
   class Align: public EnemyMothershipSteeringBehavior
   {
   public:
      typedef EnemyMothershipSteeringBehavior BaseClass;

      Align(float lookAhead, float timeToTarget)
         : mLookAhead(lookAhead)
         , mTimeToTarget(timeToTarget)
      {}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   protected:
      float Sgn(float x);
      osg::Vec3 GetTargetPosition(float dt, const EnemyMothershipGoalState& goal);
      float GetTargetForward(float dt, const osg::Vec3& targetPos, const EnemyMothershipGoalState& current_goal, const EnemyMothershipState& current_state, osg::Vec3& vec_in);

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

#endif // DELTA_AIEnemyMothership_H
