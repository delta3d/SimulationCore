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



#include <TowerAIHelper.h>
#include <AIEvent.h>
#include <AIState.h>
#include <AIUtility.h>
#include <Actors/EnemyDescriptionActor.h>

#include <dtAI/npcstate.h>
#include <dtAI/npcevent.h>

#include <dtCore/transformable.h>

#include <dtUtil/command.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/log.h>

#include <dtUtil/stringutils.h>
#include <dtUtil/mathdefines.h>

#include <AIEvent.h>

namespace NetDemo
{
   //////////////////////////////////////////////////////////////////////////
   //TurretWeaponState
   //////////////////////////////////////////////////////////////////////////
   TurretWeaponState::TurretWeaponState()
      : mTrigger(false)
      , mTimeStep(0.0f)
      , mCurrentAngle(osg::PI_2, osg::PI_2)
      , mCurrentVel()
      , mOrigin()
   {
   }
   
   TurretWeaponState::~TurretWeaponState()
   {
   }


   //////////////////////////////////////////////////////////////////////////
   //TurretGoalState
   //////////////////////////////////////////////////////////////////////////
   TurretGoalState::TurretGoalState()
   : mMinAngle(-180.0f, -30.0f)
   , mMaxAngle(180.0f, 30.0f)
   , mMaxVel(1.5f, 1.5f)
   , mAngleToTarget()
   {
   }
   
   TurretGoalState::~TurretGoalState()
   {
   }

   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringTargeter
   //////////////////////////////////////////////////////////////////////////
   TurretSteeringTargeter::TurretSteeringTargeter()
   {
   }

   TurretSteeringTargeter::~TurretSteeringTargeter()
   {
   }

   bool TurretSteeringTargeter::GetGoal(const TurretWeaponState& current_state, TurretGoalState& result)
   {
      if(!mTargets.empty())
      {
         osg::Vec3 pos = current_state.GetOrigin();
         osg::Vec3 targetPos = Top();

         osg::Vec3 vToTarget = targetPos - pos;

         float length = sqrtf((vToTarget[0] * vToTarget[0]) + (vToTarget[1] * vToTarget[1]));
         osg::Vec2 angle;         
         angle[0] = atan2f(vToTarget[1], vToTarget[0]);
         angle[1] = atan2f(length, vToTarget[2]);

         result.SetTargetPos(targetPos);
         result.SetAngleToTarget(angle);
         Pop();
      }

      return true;
   }

   void TurretSteeringTargeter::Push(const osg::Vec3& pos)
   {
      mTargets.push(pos);
   }

   void TurretSteeringTargeter::Pop()
   {
      mTargets.pop();
   }

   const osg::Vec3& TurretSteeringTargeter::Top() const
   {
      return mTargets.top();
   }


   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringDecomposer
   //////////////////////////////////////////////////////////////////////////
   TurretSteeringDecomposer::TurretSteeringDecomposer()
   {
   }

   TurretSteeringDecomposer::~TurretSteeringDecomposer()
   {
   }

   void TurretSteeringDecomposer::Decompose(const TurretWeaponState& current_state, TurretWeaponState& result) const
   {
   }

   //////////////////////////////////////////////////////////////////////////
   //TurretSteeringConstraint
   //////////////////////////////////////////////////////////////////////////
   TurretSteeringConstraint::TurretSteeringConstraint()
   {
   }

   TurretSteeringConstraint::~TurretSteeringConstraint()
   {
   }

   bool TurretSteeringConstraint::WillViolate(const BaseClass::PathType& pathToFollow) const
   {
      return false;
   }

   void TurretSteeringConstraint::Suggest(const BaseClass::PathType& pathToFollow, const TurretWeaponState& current_state, TurretWeaponState& result) const
   {
   }


   TurretAlign::TurretAlign(float lookAhead, float timeToTarget)
      : BaseClass()
      , mLookAhead(lookAhead)
      , mTimeToTarget(timeToTarget)
   {
   }

   void TurretAlign::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   {
      dtUtil::Clamp(dt, 0.01f, 0.1f);

      const float maxVelTheta = current_goal.GetMaxVel()[0];
      const float maxVelPhi = current_goal.GetMaxVel()[1];

      float theta = ComputeDOF(dt, 
                               current_state.GetCurrentAngle()[0],
                               current_goal.GetAngleToTarget()[0],
                                 -1.0f * maxVelTheta, maxVelTheta);

      float phi = ComputeDOF(dt, 
                                current_state.GetCurrentAngle()[1],
                                current_goal.GetAngleToTarget()[1],
                                -1.0f * maxVelPhi, maxVelPhi);


      osg::Vec2 angle(current_state.GetCurrentAngle());
      angle[0] = theta;
      angle[1] = phi;

      result.SetCurrentAngle(angle);

      //todo- move to firing behavior
      osg::Vec2 from = current_goal.GetAngleToTarget();
      osg::Vec2 to = current_state.GetCurrentAngle();
     
      float diff = (to - from).length();
      //LOG_ALWAYS("Angle: " + dtUtil::ToString(diff));
      if(diff < 0.05f)
      {
         result.SetTrigger(true);
      }
      else
      {
         result.SetTrigger(false);
      }

   }

   float TurretAlign::ComputeDOF( float dt, float currAngle, float goalAngle, float minAngle, float maxAngle )
   {
      float result = currAngle;

      float vel = goalAngle - currAngle;      
      vel /= mTimeToTarget;

      dtUtil::Clamp(vel, minAngle, maxAngle);

      if(dtUtil::IsFinite(vel)) 
      {
         result += dt * vel;

         while(result > osg::PI) result -= (2.0 * osg::PI);
      }

      return result;
   }

   //////////////////////////////////////////////////////////////////////////
   //AITurret
   //////////////////////////////////////////////////////////////////////////

   AITurret::AITurret()
      : BaseClass()
      , mTargeter(new TurretSteeringTargeter())
      , mSteeringPipeline()
   {
      mTargeters.push_back(mTargeter);
      
      //NOTE: THIS DOES NOT WORK BECAUSE THE ObtainGoal in SteeringPipeline
      //MUST COPY THE GOAL
      //TODO- FIX STEERINGPIPELINE.H 
      //mGoalState.SetMaxVel(osg::Vec2(1.0f, 1.5f));

      float lookAhead = 0.65f;
      float timeToTarget = 0.05f;
      TurretAlign* t_align = new TurretAlign(lookAhead, timeToTarget);
      mBehaviors.push_back(t_align);
   }

   AITurret::~AITurret()
   {
   }

   void AITurret::StepTurret(float dt)
   {
      mCurrentState.SetTimeStep(dt);
      mSteeringPipeline.Step(dt, *this);
   }

   bool AITurret::FindPath( const TurretWeaponState& fromState, TurretGoalState& goal, BaseClass::AIPath& resultingPath ) const
   {
      resultingPath.push_back(goal);
      return true;
   }

   void AITurret::UpdateState( float dt, const TurretWeaponState& steerData )
   {
      mCurrentState.SetTrigger(steerData.GetTrigger());
      mCurrentState.SetCurrentAngle(steerData.GetCurrentAngle());
      mCurrentState.SetCurrentVel(steerData.GetCurrentVel());
      mCurrentState.SetOrigin(steerData.GetOrigin());
   }

   void AITurret::OutputControl( const BaseClass::AIPath& pathToFollow, const TurretWeaponState& current_state, TurretWeaponState& result ) const
   {
      if(!pathToFollow.empty() && !mBehaviors.empty())
      {
         TurretSteeringBehavior* behavior = mBehaviors.front();
         behavior->Think(current_state.GetTimeStep(), pathToFollow.front(), current_state, result);
      }
   }

   void AITurret::AddSteeringBehavior( TurretSteeringBehavior* steeringbehavior )
   {
      mBehaviors.push_back(steeringbehavior);
   }

   const AITurret::SteeringBehaviorArray& AITurret::GetSteeringBehaviors() const
   {
      return mBehaviors;
   }

   TurretSteeringTargeter& AITurret::GetTargeter()
   {
      return *mTargeter;
   }
} //namespace NetDemo

