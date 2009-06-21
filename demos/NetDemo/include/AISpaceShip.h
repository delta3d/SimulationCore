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

#include <osg/Matrix>
#include <osg/Vec3>

namespace NetDemo
{
   struct SpaceShipState
   {
      osg::Vec3 mPos;
      osg::Vec3 mForward, mUp;
      osg::Vec3 mVel, mAccel;
      float mAngularVel, mAngularAccel;
      float mVerticalVel, mVerticalAccel;
      float mPitch, mRoll;
      float mTimeStep;
   };

   struct SpaceShipGoalState: public SpaceShipState
   {
      float mDragCoef, mAngularDragCoef, mVerticalDragCoef;
      float mMaxVel, mMaxAccel;
      float mMaxAngularVel, mMaxAngularAccel;
      float mMaxVerticalVel, mMaxVerticalAccel;
      float mMaxPitch, mMaxRoll;
      float mMaxTiltPerSecond, mMaxRollPerSecond;
      float mMinElevation, mMaxElevation;
   };

   struct SpaceShipControls
   { 
      //these are the control inputs
      //all are floats from 1 to -1 
      //which represents percentage of maximum
      float mThrust, mLift, mYaw;
   };

   struct SpaceShipControllable: public Controllable<SpaceShipState, SpaceShipGoalState, SpaceShipControls>
   {
      public:
         typedef Controllable<SpaceShipState, SpaceShipGoalState, SpaceShipControls> BaseClass;
   
         SpaceShipControllable();
         ~SpaceShipControllable();

         static void InitControllable(const osg::Matrix& matIn, SpaceShipControllable& stateIn);
         static void ResetState(const osg::Matrix& matIn, SpaceShipState& spaceShipState);
         static void SetState(const osg::Matrix& matIn, SpaceShipState& spaceShipState);
         static void OrthoNormalize(SpaceShipState& stateIn);
         static void SetMatrix(const SpaceShipState& stateIn, osg::Matrix& result);


         //derived members
         //typename BaseClass::ControlType mCurrentControls;

         //typename BaseClass::StateType mCurrentState;
         //typename BaseClass::StateType mNextPredictedState;

         //typename BaseClass::GoalStateType mGoalState;

         //typename BaseClass::StateType mStateConstraints;
         //typename BaseClass::ControlType mControlConstraints;

         //typename BaseClass::PathType mPathToFollow;
         //typename BaseClass::PathType mPredictedPath;

   };

   class SpaceShipTargeter: public Targeter<SpaceShipState, SpaceShipGoalState, SpaceShipControls>
   {
      public:
         typedef Targeter<SpaceShipState, SpaceShipGoalState, SpaceShipControls> BaseClass;

         SpaceShipTargeter();
         virtual ~SpaceShipTargeter();

         /*virtual*/ bool GetGoal(const SpaceShipState& current_state, SpaceShipGoalState& result) const;

      private:
   };

   class SpaceShipDecomposer: public Decomposer<SpaceShipState, SpaceShipGoalState, SpaceShipControls>
   {
   public:
      typedef Decomposer<SpaceShipState, SpaceShipGoalState, SpaceShipControls> BaseClass;

      SpaceShipDecomposer();
      ~SpaceShipDecomposer();

      /*virtual*/ void Decompose(const SpaceShipState& current_state, SpaceShipGoalState& result) const;
   };

   class SpaceShipConstraint: public Constraint<SpaceShipState, SpaceShipGoalState, SpaceShipControls>
   {
   public:
      typedef Constraint<SpaceShipState, SpaceShipGoalState, SpaceShipControls> BaseClass;

      SpaceShipConstraint();
      ~SpaceShipConstraint();

      /*virtual*/ bool WillViolate(const BaseClass::PathType& pathToFollow);
      /*virtual*/ void Suggest(const BaseClass::PathType& pathToFollow, const SpaceShipState& current_state, SpaceShipGoalState& result);
   };


   class SpaceShipActuator: public Actuator<SpaceShipState, SpaceShipGoalState, SpaceShipControls>
   {
   public:
      typedef Actuator<SpaceShipState, SpaceShipGoalState, SpaceShipControls> BaseClass;

      SpaceShipActuator();
      ~SpaceShipActuator();

      /*virtual*/ bool GetPath(const SpaceShipState& current_state, const SpaceShipGoalState& goal, BaseClass::PathType& result);
      /*virtual*/ void Output(const BaseClass::PathType& pathToFollow, const SpaceShipState& current_state, SpaceShipControls& result);
   };



   class SpaceShipUpdater: public Updater<SpaceShipState, SpaceShipGoalState, SpaceShipControls>
   {
      public:
         typedef Updater<SpaceShipState, SpaceShipGoalState, SpaceShipControls> BaseClass;

         SpaceShipUpdater();
         virtual ~SpaceShipUpdater();

         //const osg::Matrix& Update(const SpaceShipControls& control, double dt);
         /*virtual*/ void Update(float dt, const SpaceShipControls& steeringOut, SpaceShipControllable::BaseClass& stateIn);

         static void Reset(const osg::Matrix& matIn, SpaceShipControllable& stateIn);

      private:

         void UpdateHeading(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn);
         void UpdatePosition(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn);
         void UpdateVelocity(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn);
         void UpdateAngularVelocity(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn);
         void UpdateVerticalVelocity(const SpaceShipControls& controls, SpaceShipControllable::BaseClass& stateIn);

         void UpdateTilt(const SpaceShipControls& controls, osg::Vec3& tilt, SpaceShipControllable::BaseClass& stateIn);
         void UpdateRoll(const SpaceShipControls& controls, osg::Vec3& roll, SpaceShipControllable::BaseClass& stateIn);

         float Clamp(float x, float from, float to);
         float Dampen(float last, float current, float max, float falloff);         

   };


   typedef SteeringPipeline<SpaceShipControllable, SpaceShipTargeter, SpaceShipDecomposer,
                            SpaceShipConstraint, SpaceShipActuator, SpaceShipUpdater> SpaceShipAIModel;


   class NETDEMO_EXPORT SpaceShipAIHelper: public EnemyAIHelper
   {
   public:
      typedef EnemyAIHelper BaseClass;

      SpaceShipAIHelper();

      /*virtual*/ void Init();
      /*virtual*/ void Spawn();
      /*virtual*/ void Update(float dt);

      /*virtual*/ void PreSync(const dtCore::Transform& trans);
      /*virtual*/ void PostSync(dtCore::Transform& trans) const;

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
      virtual void CalculateNextWaypoint();
      virtual void GoToWaypoint(float dt);
      virtual void DefaultStateUpdate(float dt);

      //void ChangeSteeringBehavior(dtCore::RefPtr<SteeringBehaviorType> newBehavior);

   private:

      //float GetDistance(const osg::Vec3& pos);

      SpaceShipControllable mAIControllable;
      SpaceShipAIModel mAIModel;
   };

}//namespace NetDemo

#endif // DELTA_AISPACESHIP_H
