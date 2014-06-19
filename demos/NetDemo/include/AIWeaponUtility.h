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

#ifndef DELTA_AIWEAPONUTILITY_H
#define DELTA_AIWEAPONUTILITY_H

#include "AIUtility.h"

#include <osg/Vec2>


namespace NetDemo
{
   //////////////////////////////////////////////////////////////////////////
   //TurretWeaponState
   //////////////////////////////////////////////////////////////////////////
   struct TurretWeaponState
   {
      TurretWeaponState();
      ~TurretWeaponState();

      DT_DECLARE_ACCESSOR_INLINE(bool, Trigger);
      DT_DECLARE_ACCESSOR_INLINE(float, TimeStep);

      //using polar coordinates
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec2, CurrentAngle);
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec2, CurrentVel);
      
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec3, Origin);

      void RegisterProperties(dtCore::PropertyContainer& pc, const std::string& group)
      {}

   };

   //////////////////////////////////////////////////////////////////////////
   //TurretGoalState
   //////////////////////////////////////////////////////////////////////////
   struct TurretGoalState
   {
      TurretGoalState();
      ~TurretGoalState();

      //using polar coordinates
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec2, MinAngle);
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec2, MaxAngle);
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec2, MaxVel);

      DT_DECLARE_ACCESSOR_INLINE(osg::Vec2, AngleToTarget);
      DT_DECLARE_ACCESSOR_INLINE(osg::Vec3, TargetPos);

      void RegisterProperties(dtCore::PropertyContainer& pc, const std::string& group){}

   };

   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringBehavior
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::SteeringBehavior<TurretGoalState, TurretWeaponState, TurretWeaponState> TurretSteeringBehavior;


   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringTargeter
   //////////////////////////////////////////////////////////////////////////
   class TurretSteeringTargeter: public dtAI::Targeter<TurretWeaponState, TurretGoalState>
   {
   public:
      typedef dtAI::Targeter<TurretWeaponState, TurretGoalState> BaseClass;

      TurretSteeringTargeter();
      virtual ~TurretSteeringTargeter();

      void Pop();
      const osg::Vec3& Top() const;
      void Push(const osg::Vec3& pos);

      /*virtual*/ bool GetGoal(const TurretWeaponState& current_state, TurretGoalState& result);

   private:

      std::stack<osg::Vec3> mTargets;

   };


   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringDecomposer
   //////////////////////////////////////////////////////////////////////////
   class TurretSteeringDecomposer: public dtAI::Decomposer<TurretWeaponState, TurretWeaponState>
   {
   public:
      typedef dtAI::Decomposer<TurretWeaponState, TurretWeaponState> BaseClass;

      TurretSteeringDecomposer();
      ~TurretSteeringDecomposer();

      /*virtual*/ void Decompose(const TurretWeaponState& current_state, TurretWeaponState& result) const;
   };


   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringConstraint
   //////////////////////////////////////////////////////////////////////////
   class TurretSteeringConstraint: public dtAI::Constraint<TurretWeaponState, TurretWeaponState>
   {
   public:
      typedef dtAI::Constraint<TurretWeaponState, TurretWeaponState> BaseClass;

      TurretSteeringConstraint();
      ~TurretSteeringConstraint();

      /*virtual*/ bool WillViolate(const BaseClass::PathType& pathToFollow) const;
      /*virtual*/ void Suggest(const BaseClass::PathType& pathToFollow, const TurretWeaponState& current_state, TurretWeaponState& result) const;
   };


   //////////////////////////////////////////////////////////////////////////
   //TurretDoNothing
   //////////////////////////////////////////////////////////////////////////
   class TurretDoNothing: public TurretSteeringBehavior
   {
   public:

      typedef TurretSteeringBehavior BaseClass;

      /*virtual*/ void Think(float dt, const TurretGoalState& current_goal, const TurretWeaponState& current_state, TurretWeaponState& result)
      {
      }

   };

   //////////////////////////////////////////////////////////////////////////
   //TurretAlign
   //////////////////////////////////////////////////////////////////////////
   class TurretAlign: public TurretSteeringBehavior
   {
   public:
      typedef TurretSteeringBehavior BaseClass;

      TurretAlign(float lookAhead, float timeToTarget);

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:

      float ComputeDOF(float dt, float currAngle, float goalAngle, float minAngle, float maxAngle);

      float mLookAhead, mTimeToTarget;
   };



   //////////////////////////////////////////////////////////////////////////
   //TurretControllable
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::Controllable<TurretWeaponState, TurretGoalState, TurretWeaponState> TurretControllable;

   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringPipeline
   //////////////////////////////////////////////////////////////////////////
   typedef dtAI::SteeringPipeline<TurretControllable> TurretSteeringPipeline;

   //////////////////////////////////////////////////////////////////////////
   //AITurret
   //////////////////////////////////////////////////////////////////////////
   class AITurret: public TurretControllable
   {
      public:
         typedef TurretControllable BaseClass;

         typedef std::vector<TurretSteeringBehavior*> SteeringBehaviorArray; 


         AITurret();
         /*virtual*/ ~AITurret();

         void StepTurret(float dt);

         //derived from Controllable and allows the steering pipeline to operate on us
         /*virtual*/ bool FindPath(const TurretWeaponState& fromState, TurretGoalState& goal, BaseClass::AIPath& resultingPath) const;
         /*virtual*/ void UpdateState(float dt, const TurretWeaponState& steerData);
         /*virtual*/ void OutputControl(const BaseClass::AIPath& pathToFollow, const TurretWeaponState& current_state, TurretWeaponState& result) const;

         /*virtual*/ void RegisterProperties(dtCore::PropertyContainer& pc, const std::string& group){}

         TurretSteeringTargeter& GetTargeter();

         void AddSteeringBehavior(TurretSteeringBehavior* steeringbehavior);
         //void RemoveSteeringBehavior(BaseAISteeringBehavior* steeringbehavior);
         const SteeringBehaviorArray& GetSteeringBehaviors() const;

      private:
         TurretSteeringTargeter* mTargeter;
         TurretSteeringPipeline mSteeringPipeline;
         SteeringBehaviorArray mBehaviors;
   };


} //namespace NetDemo

#endif //DELTA_AIWEAPONUTILITY_H
