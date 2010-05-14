/* -*-c++-*-
* Simulation Core
* Copyright 2010, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>

#include <SimCore/Actors/DRPublishingActComp.h>

#include <dtUtil/mathdefines.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/propertymacros.h>
#include <dtGame/basemessages.h>
#include <dtGame/gameactor.h>
#include <dtCore/transform.h>
#include <osg/Geode>


namespace SimCore
{
   namespace Actors
   {
      const dtGame::ActorComponent::ACType DRPublishingActComp::TYPE("DRPublishingActComp");

      const float DRPublishingActComp::TIME_BETWEEN_UPDATES(10.0f);

      ////////////////////////////////////////////////////////////////////////////////
      DRPublishingActComp::DRPublishingActComp()
         : ActorComponent(TYPE)
         , mTimeUntilNextFullUpdate(0.0f)
         , mVelocityAverageFrameCount(1U)
         , mMaxUpdateSendRate(5.0f)
         , mPublishLinearVelocity(true)
         , mPublishAngularVelocity(true)
         , mSecsSinceLastUpdateSent(0.0f)
         , mVelocityMagThreshold(1.0f)
         , mVelocityDotThreshold(0.9f)
         , mForceUpdateNextChance(false)
         , mUseVelocityInDRUpdateDecision(false)
         , mMaxRotationError(1.0f) // 2.0
         , mMaxRotationError2(1.0f) // 4.0
         , mMaxTranslationError(0.02f)//(0.15f)
         , mMaxTranslationError2(0.0004f)//(0.0225f)
      {
      }

      ////////////////////////////////////////////////////////////////////////////////
      DRPublishingActComp::~DRPublishingActComp()
      {
      }


      ////////////////////////////////////////////////////////////////////////////////
      // PROPERTY MACROS
      // These macros define the Getter and Setter method body for each property
      ////////////////////////////////////////////////////////////////////////////////

      IMPLEMENT_PROPERTY(DRPublishingActComp, int, VelocityAverageFrameCount);

      IMPLEMENT_PROPERTY_GETTER(DRPublishingActComp, float, MaxUpdateSendRate); // Setter is implemented below

      IMPLEMENT_PROPERTY(DRPublishingActComp, bool, PublishLinearVelocity);

      IMPLEMENT_PROPERTY(DRPublishingActComp, bool, PublishAngularVelocity);


      ////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::OnAddedToActor(dtGame::GameActor& actor)
      {
         // The base class may have overwritten our update rate values - or they may not have been set yet due to Init order
         SetMaxUpdateSendRate(GetMaxUpdateSendRate());

      }

      ////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::OnRemovedFromActor(dtGame::GameActor& actor)
      {
      }


      ////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {

         // We can't do anything without a helper, and it's possible it is going to get set later or
         // because it's an observer pointer, maybe it has gone away.
         if (!IsDeadReckoningHelperValid())
         {
            return; 
         }

         double elapsedTime = tickMessage.GetDeltaSimTime();

         // UPDATE OUR DR VALUES         
         mTimeUntilNextFullUpdate -= elapsedTime;
         mSecsSinceLastUpdateSent += elapsedTime; // We can only send out an update so many times a second.

         dtGame::GameActor* actor;
         GetOwner(actor);

         bool forceUpdate = false;
         bool fullUpdate = false;

         dtCore::Transform xform;
         actor->GetTransform(xform);
         osg::Vec3 rot;
         xform.GetRotation(rot);
         osg::Vec3 pos;
         xform.GetTranslation(pos);

         ComputeCurrentVelocity(elapsedTime, pos, rot);

         if (mTimeUntilNextFullUpdate <= 0.0f)
         {
            mTimeUntilNextFullUpdate = 1.05f * TIME_BETWEEN_UPDATES;
            fullUpdate = true;
            forceUpdate = true;
         }

         // Check for update
         if (mForceUpdateNextChance)
         {
            forceUpdate = true;
            mForceUpdateNextChance = false;
         }
         else if (!fullUpdate)
         {
            forceUpdate = ShouldForceUpdate(pos, rot);
         }


         if (forceUpdate)
         {
            // Previously, this was done at the start of the frame, before applying physics.
            // Because the behavior was pulled into the Actor Component, the behavior now happens 
            // after the owner actor is ticked. Theoretically, this should be OK, because the physics
            // forces do not apply until after this frame. However, if you are manually setting velocity
            // or position, it might behave differently than it did before.
            SetLastKnownValuesBeforePublish(pos, rot);

            // Since we are about to publish, set our time since last publish back to 0.
            // This allows us to immediately send out a change the exact moment it happens (ex if we
            // were moving slowly and hadn't updated recently).
            mSecsSinceLastUpdateSent = 0.0f;

            // If it is almost time to do a full update and our entity wants to do a partial update anyway, 
            // then go ahead and do a full update now. This prevents the heart beat from causing 
            // discontinuities in the update rate - mainly for vehicles that publish quickly and regularly
            // The logic should cause an update at between 9.5 - 10.5 seconds assuming a 10s heart beat
            if (!fullUpdate && mTimeUntilNextFullUpdate < TIME_BETWEEN_UPDATES * 0.1f)
            {
               mTimeUntilNextFullUpdate = 1.05f * TIME_BETWEEN_UPDATES;
               fullUpdate = true;
            }

            if (fullUpdate)
            {
               actor->GetGameActorProxy().NotifyFullActorUpdate();
            }
            else
            {
               actor->GetGameActorProxy().NotifyPartialActorUpdate();
            }
         }

      }

      ////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::OnEnteredWorld()
      {
         dtGame::GameActor* actor;
         GetOwner(actor);

         // LOCAL ACTOR -  do our setup
         if (!actor->IsRemote())
         {
            RegisterForTicks();

            // We publish full updates periodically, but we want to randomize their start point, 
            // so all actors loaded in a map don't do full updates on the same frame. 
            mTimeUntilNextFullUpdate = TIME_BETWEEN_UPDATES - 0.5f * dtUtil::RandFloat(0.0f, TIME_BETWEEN_UPDATES);
         }

      }

      ////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::OnRemovedFromWorld()
      {
         dtGame::GameActor* actor;
         GetOwner(actor);

         // LOCAL ACTOR - cleanup
         if (!actor->IsRemote())
         {
            UnregisterForTicks();
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::BuildPropertyMap()
      {
         //static const dtUtil::RefString GROUPNAME = "DR Publishing";

         typedef dtDAL::PropertyRegHelper<DRPublishingActComp&, DRPublishingActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "DR Publishing");

         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(VelocityAverageFrameCount, "VelocityAveragingFrameCount", "Velocity Averaging Frame Count",
            "This actor computes it's current velocity by averaging the change in position over the given number of frames.", PropRegType, propRegHelper);
         //static const dtUtil::RefString VEL_AVG_FRAME_COUNT_DESC("This actor computes it's current velocity by averaging the change in position over the given number of frames.");
         //AddProperty(new dtDAL::IntActorProperty(PROP_VEL_AVG_FRAME_COUNT, PROP_VEL_AVG_FRAME_COUNT,
         //   dtDAL::IntActorProperty::SetFuncType(this &DRPublishingActComp::SetVelocityAverageFrameCount),
         //   dtDAL::IntActorProperty::GetFuncType(this, &DRPublishingActComp::GetVelocityAverageFrameCount),
         //   VEL_AVG_FRAME_COUNT_DESC,
         //   GROUPNAME));

         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(MaxUpdateSendRate, "DesiredNumUpdatesPerSec", "Desired Number of Updates Per Second",
            "The desired number of updates per second - the actual frequently may be less if vehicle doesn't change much.", PropRegType, propRegHelper);
         //AddProperty(new dtDAL::FloatActorProperty("DesiredNumUpdatesPerSec",
         //   ,
         //   dtDAL::FloatActorProperty::SetFuncType(this, &DRPublishingActComp::SetMaxUpdateSendRate),
         //  dtDAL::FloatActorProperty::GetFuncType(this, &DRPublishingActComp::GetMaxUpdateSendRate),
         //   ,
         //   GROUPNAME));

      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::ForceUpdateAtNextOpportunity()
      {
         mForceUpdateNextChance = true;
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::ForceFullUpdateAtNextOpportunity()
      {
         mTimeUntilNextFullUpdate = 0.0f;
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetSecsSinceLastUpdateSent(float secsSinceLastUpdateSent)
      {
         mSecsSinceLastUpdateSent = secsSinceLastUpdateSent;
      }

      //////////////////////////////////////////////////////////////////////
      float DRPublishingActComp::GetSecsSinceLastUpdateSent() const
      {
         return mSecsSinceLastUpdateSent;
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetMaxUpdateSendRate(float maxUpdateSendRate)
      {
         mMaxUpdateSendRate = maxUpdateSendRate;

         // The DR helper should be kept in the loop about the max send rate. 
         if (IsDeadReckoningHelperValid() && maxUpdateSendRate > 0.0f)
         {
            // If we are setting our smoothing time, then we need to force the DR helper
            // to ALWAYS use that, instead of using the avg update rate.
            GetDeadReckoningHelper().SetAlwaysUseMaxSmoothingTime(true);
            float transUpdateRate = dtUtil::Max(0.01f, dtUtil::Min(1.0f, 1.00f/maxUpdateSendRate));
            GetDeadReckoningHelper().SetMaxTranslationSmoothingTime(transUpdateRate);
            float rotUpdateRate = dtUtil::Max(0.01f, dtUtil::Min(1.0f, 1.00f/maxUpdateSendRate));
            GetDeadReckoningHelper().SetMaxRotationSmoothingTime(rotUpdateRate);
         }

      }

      //////////////////////////////////////////////////////////////////////
      //float DRPublishingActComp::GetMaxUpdateSendRate()
      //{
      //   return mMaxUpdateSendRate;
      //}

      //////////////////////////////////////////////////////////////////////
      //void DRPublishingActComp::SetVelocityMagnitudeUpdateThreshold(float thresh)
      //{
      //   mVelocityMagThreshold = thresh;
      //}

      //////////////////////////////////////////////////////////////////////
      float DRPublishingActComp::GetVelocityMagnitudeUpdateThreshold() const
      {
         return mVelocityMagThreshold;
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetVelocityDotProductUpdateThreshold(float thresh)
      {
         mVelocityDotThreshold = thresh;
      }

      //////////////////////////////////////////////////////////////////////
      float DRPublishingActComp::GetVelocityDotProductUpdateThreshold() const
      {
         return mVelocityMagThreshold;
      }

      //////////////////////////////////////////////////////////////////////
      //void DRPublishingActComp::SetVelocityAverageFrameCount(int count)
      //{
      //   mVelocityAverageFrameCount = dtUtil::Max(1, count);
      //}

      //////////////////////////////////////////////////////////////////////
      //int DRPublishingActComp::GetVelocityAverageFrameCount() const
      //{
      //   return mVelocityAverageFrameCount;
      //}


      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetUseVelocityInDRUpdateDecision(bool value)
      {
         mUseVelocityInDRUpdateDecision = value;
      }

      //////////////////////////////////////////////////////////////////////
      bool DRPublishingActComp::GetUseVelocityInDRUpdateDecision() const
      {
         return mUseVelocityInDRUpdateDecision;
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetCurrentVelocity(const osg::Vec3& vec) 
      { 
         mCurrentVelocity = vec; 
      }

      //////////////////////////////////////////////////////////////////////
      osg::Vec3 DRPublishingActComp::GetCurrentVelocity() const 
      { 
         return mCurrentVelocity; 
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetCurrentAcceleration(const osg::Vec3& vec) 
      { 
         mCurrentAcceleration = vec; 
      }

      //////////////////////////////////////////////////////////////////////
      osg::Vec3 DRPublishingActComp::GetCurrentAcceleration() const 
      { 
         return mCurrentAcceleration; 
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetCurrentAngularVelocity(const osg::Vec3& vec) 
      { 
         mCurrentAngularVelocity = vec; 
      }

      //////////////////////////////////////////////////////////////////////
      osg::Vec3 DRPublishingActComp::GetCurrentAngularVelocity() const 
      { 
         return mCurrentAngularVelocity; 
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetMaxTranslationError(float distance)
      {
         mMaxTranslationError = distance;
         mMaxTranslationError2 = distance * distance;
      }

      //////////////////////////////////////////////////////////////////////
      float DRPublishingActComp::GetMaxTranslationError() const 
      { 
         return mMaxTranslationError; 
      }

      //////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetMaxRotationError(float rotation)
      {
         mMaxRotationError = rotation;
         mMaxRotationError2 = rotation * rotation;
      }

      //////////////////////////////////////////////////////////////////////
      float DRPublishingActComp::GetMaxRotationError() const 
      { 
         return mMaxRotationError; 
      }

      //////////////////////////////////////////////////////////////////////
      float DRPublishingActComp::GetTimeUntilNextFullUpdate() const 
      { 
         return mTimeUntilNextFullUpdate; 
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetAccumulatedAcceleration(const osg::Vec3 &newValue)
      { 
         mAccumulatedAcceleration = newValue; 
      }

      ///////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 DRPublishingActComp::GetAccumulatedAcceleration() const
      { 
         return mAccumulatedAcceleration; 
      }

      ///////////////////////////////////////////////////////////////////////////////////
      dtGame::DeadReckoningHelper& DRPublishingActComp::GetDeadReckoningHelper() 
      { 
         return *mDeadReckoningHelper; 
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetDeadReckoningHelper(dtGame::DeadReckoningHelper* drHelper) 
      { 
         mDeadReckoningHelper = drHelper; 
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool DRPublishingActComp::IsDeadReckoningHelperValid() const
      { 
         return (mDeadReckoningHelper.valid()); 
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::SetLastKnownValuesBeforePublish(const osg::Vec3& pos, const osg::Vec3& rot)
      {

         GetDeadReckoningHelper().SetLastKnownTranslation(pos);
         GetDeadReckoningHelper().SetLastKnownRotation(rot);



         // Linear Velocity & acceleration - push the current value to the Last Known
         if (mPublishLinearVelocity)
         {
            // VELOCITY 
            osg::Vec3 velocity = GetCurrentVelocity();            
            if (velocity.length() < 0.0001) // If close to 0, set to 0 to prevent wiggling/shaking
            {
               velocity = osg::Vec3(0.f, 0.f, 0.f);
            }
            GetDeadReckoningHelper().SetLastKnownVelocity(velocity);

            // ACCELERATION

            /// DAMPEN THE ACCELERATION TO PREVENT WILD SWINGS WITH DEAD RECKONING
            // Dampen the acceleration before publication if the vehicle is making drastic changes 
            // in direction. With drastic changes, the acceleration will cause the Dead Reckoned 
            // pos to oscillate wildly. Whereas it will improve DR on smooth curves such as a circle.
            // The math is: take the current accel and the non-scaled accel from the last publish;
            // normalize them; dot them and use the product to scale our current Acceleration. 
            osg::Vec3 curAccel = GetCurrentAcceleration();
            curAccel.normalize();
            float accelDotProduct = curAccel * mAccelerationCalculatedForLastPublish; // dot product
            SetCurrentAcceleration(GetCurrentAcceleration() * dtUtil::Max(0.0f, accelDotProduct));
            mAccelerationCalculatedForLastPublish = curAccel; // Hold for next time (pre-normalized)

            // Acceleration is paired with velocity
            GetDeadReckoningHelper().SetLastKnownAcceleration(GetCurrentAcceleration());
         }

         // Angular Velocity - push the current value to the Last Known
         if (mPublishAngularVelocity)
         {
            osg::Vec3 angularVelocity = GetCurrentAngularVelocity();
            if (angularVelocity.length() < 0.001)  // If close to 0, set to 0 to prevent wiggling/shaking
            {
               angularVelocity = osg::Vec3(0.f, 0.f, 0.f);
            }
            GetDeadReckoningHelper().SetLastKnownAngularVelocity(angularVelocity);
         }

      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRPublishingActComp::ComputeCurrentVelocity(float deltaTime, const osg::Vec3& pos, const osg::Vec3& rot)
      {
         if (mPublishLinearVelocity) // If not publishing, then don't do anything.
         {
            // Note - we used to grab the velocity from the physics engines, but there were sometimes 
            // discontinuities reported by the various engines, so that was removed in favor of a simple
            // differential of position. 
            //dtGame::GameActor* actor;
            //GetOwner(actor);
            //dtCore::Transform xform;
            //actor->GetTransform(xform);
            //osg::Vec3 pos;
            //xform.GetTranslation(pos);
            if (deltaTime > 0.0f && mLastPos.length2() > 0.0) // ignore first time.
            {
               osg::Vec3 distanceMoved = pos - mLastPos;
               osg::Vec3 instantVelocity = distanceMoved / deltaTime;

               // Compute our acceleration as the instantaneous differential of the velocity
               // Acceleration is dampened before publication - see SetLastKnownValuesBeforePublish().
               // Note - if you know your REAL acceleration due to vehicle dynamics, override the method
               // and make your own call to SetCurrentAcceleration().
               osg::Vec3 changeInVelocity = instantVelocity - mAccumulatedLinearVelocity;
               mAccumulatedAcceleration = changeInVelocity / deltaTime;

               // Compute Vel - either the instant Vel or a blended value over a couple of frames. Blended Velocities tend to make acceleration less useful
               if (mVelocityAverageFrameCount == 1)
               {
                  mAccumulatedLinearVelocity = instantVelocity;
               }
               else 
               {
                  float instantVelWeight = 1.0f / float(mVelocityAverageFrameCount);
                  mAccumulatedLinearVelocity = instantVelocity * instantVelWeight + mAccumulatedLinearVelocity * (1.0f - instantVelWeight);
               }

               // Many vehicles get a slight jitter up/down while running. If you allow the z acceleration to 
               // be published, the vehicle will go all over the place nutty. So, we zero it out. 
               // This is not an ideal solution, but is workable because vehicles that really do have a lot of
               // z acceleration are probably flying and by definition are not as close to other objects so the z accel
               // is less visually apparent.
               mAccumulatedAcceleration.z() = 0.0f; 

               SetCurrentAcceleration(mAccumulatedAcceleration);
               SetCurrentVelocity(mAccumulatedLinearVelocity);
            }

            mLastPos = pos; 
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool DRPublishingActComp::ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot)
      {
         bool forceUpdateResult = false; // if full update set, we assume we will publish
         bool enoughTimeHasPassed = (mMaxUpdateSendRate > 0.0f &&
            (mSecsSinceLastUpdateSent >= 1.0f / mMaxUpdateSendRate));

         if (enoughTimeHasPassed)
         {
            //// PREVIOUSLY THIS PART WAS IN BASE ENTITY
            //forceUpdateResult = Platform::ShouldForceUpdate(pos, rot, fullUpdate);

            // If no DR is occuring, then we don't want to check.
            if (GetDeadReckoningHelper().GetDeadReckoningAlgorithm() != dtGame::DeadReckoningAlgorithm::NONE)
            {
               // check to see if it's moved or turned enough to warrant one.
               osg::Vec3 distanceMoved = pos - GetDeadReckoningHelper().GetCurrentDeadReckonedTranslation();
               osg::Vec3 distanceTurned = rot - GetDeadReckoningHelper().GetCurrentDeadReckonedRotation();
               if (distanceMoved.length2() > mMaxTranslationError2 || distanceTurned.length2() > mMaxRotationError2)
               {
                  // Note the rotation check isn't perfect (ie, not a quaternion), so you might get
                  // an extra update, but it's very close and is very cheap processor wise.
                  forceUpdateResult = true;
               }
               // We passed pos/rot check, now check velocity
               else if (GetUseVelocityInDRUpdateDecision())
               {
                  osg::Vec3 oldVel = GetDeadReckoningHelper().GetLastKnownVelocity();
                  osg::Vec3 curVel = GetCurrentVelocity();
                  float oldMag = oldVel.normalize();
                  float curMag = curVel.normalize();
                  float velMagChange = dtUtil::Abs(curMag - oldMag);
                  if (velMagChange > mVelocityMagThreshold)
                  {
                     forceUpdateResult = true;
                     LOGN_DEBUG("DRPublishingActComp.cpp", "Forcing update based on velocity magnitude.")
                  }
                  else
                  {
                     float dotProd = oldVel * curVel;
                     if (dotProd < mVelocityDotThreshold)
                     {
                        forceUpdateResult = true;
                        LOGN_DEBUG("DRPublishingActComp.cpp", "Forcing update based on velocity angle.")
                     }
                  }
               }
            }
         }

         return forceUpdateResult;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float DRPublishingActComp::GetPercentageChangeDifference(float startValue, float newValue) const
      {
         if(std::abs(startValue) < 0.01f && std::abs(newValue) < 0.01f)
            return 1.0;

         if(startValue == 0)
            startValue = 1.0f;

         return std::abs((((newValue - startValue) / startValue) * 100.0f));
      }

   } // Actors Namespace
} // #SimCore Namespace


