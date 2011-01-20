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

#include <AISteeringModel.h>
#include <AIState.h>
#include <AIEvent.h>

#include <dtCore/transform.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>

#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtPhysics/physicsobject.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/bodywrapper.h>

#include <dtGame/gameactor.h>
#include <osg/Vec2>
#include <osg/Vec3>

namespace NetDemo
{

   //////////////////////////////////////////////////////////////////////////
   //AISteeringModel
   //////////////////////////////////////////////////////////////////////////
   AISteeringModel::AISteeringModel()
      : BaseClass()
      , mCurrentBehavior(0)
      , mSteeringBehaviors()
   {
    
   }

   AISteeringModel::~AISteeringModel()
   {
      mSteeringBehaviors.clear();
   }

   unsigned AISteeringModel::AddSteeringBehavior(BaseAISteeringBehavior* steeringbehavior)
   {
      unsigned id = mSteeringBehaviors.size();
      mSteeringBehaviors.push_back(steeringbehavior);
      return id;
   }

   const AISteeringModel::SteeringBehaviorArray& AISteeringModel::GetSteeringBehaviors() const
   {
      return mSteeringBehaviors;
   }

   bool AISteeringModel::SetCurrentSteeringBehavior(unsigned id)
   {
     if(id < mSteeringBehaviors.size())
     {
        mCurrentBehavior = id;
        return true;
     }

     return false;
   }

   void AISteeringModel::Init()
   {
   }

   void AISteeringModel::OutputControl(const BaseAIControllable::PathType& pathToFollow, const BaseAIControllable::StateType& current_state, BaseAIControllable::ControlType& result) const
   {
      if (!pathToFollow.empty())
      {
         //GetSteeringModel()->Update(current_state.mTimeStep);
         if(mCurrentBehavior < mSteeringBehaviors.size())
         {
            BaseAIControllable::SteeringBehaviorType* behavior = mSteeringBehaviors[mCurrentBehavior];
            behavior->Think(current_state.GetTimeStep(), pathToFollow.front(), current_state, result);
         }
      }
      //else
      //{
      //   result = mDefaultControls;
      //}
   }


}//namespace NetDemo
