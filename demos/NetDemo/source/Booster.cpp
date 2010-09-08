/* -*-c++-*-
 * SimulationCore
 * Copyright 2007-2008, Alion Science and Technology
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
 * david
 */

#include <Booster.h>

namespace NetDemo
{

   Booster::Booster()
   : mStartBoost(false)
   , mStartBoostAccel(7.8f)
   , mMaxBoostTime(3.0f)
   , mCurrentBoostTime(0.0f)
   , mTimeToResetBoost(7.0f)
   , mBoostResetTimer(0.0f)
   {
   }

   Booster::~Booster()
   {
   }

   DT_IMPLEMENT_ACCESSOR(Booster, bool, StartBoost);
   DT_IMPLEMENT_ACCESSOR(Booster, float, StartBoostAccel);
   DT_IMPLEMENT_ACCESSOR(Booster, float, MaxBoostTime);
   DT_IMPLEMENT_ACCESSOR(Booster, float, CurrentBoostTime);
   DT_IMPLEMENT_ACCESSOR(Booster, float, TimeToResetBoost);
   DT_IMPLEMENT_ACCESSOR(Booster, float, BoostResetTimer);

   void Booster::Start()
   {

   }

   void Booster::Stop()
   {

   }

   void Booster::Update(float dt)
   {
      //            osg::Vec3 boostDirection(0.0f, 1.0f, 0.0f);
      //            float boostForce = 0.0f;
      //
      //            if(mStartBoost)
      //            {
      //               mCurrentBoostTime += deltaTime;
      //
      //               //note: we are ramping down the boost since it
      //               //       is being held down and we dont want to fly into space :)
      //               boostForce = (mMaximumBoostPerSecond / mCurrentBoostTime);
      //            }
      //            else
      //            {
      //               mStartBoost = true;
      //
      //               //note: we just started boosting so lets boost with a large force
      //               boostForce = mStartBoostForce;
      //            }



      //            if(mStartBoost)
      //            {
      //               mBoostResetTimer += deltaTime;
      //               if(mBoostResetTimer >= mTimeToResetBoost)
      //               {
      //                  mStartBoost = false;
      //                  mCurrentBoostTime = 0.0f;
      //                  mBoostResetTimer = 0.0f;
      //               }
      //            }
}

   float Booster::GetCurrentBoostForce()
   {
      return 1.0f;

   }

}
