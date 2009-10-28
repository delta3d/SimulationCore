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

#include <AIUtility.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <osg/Vec2>


namespace NetDemo
{
   void DoNothing::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   { 
   }

   void BombDive::Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result)
   { 
      osg::Vec3 targetPos = current_goal.GetPosition();
      osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(current_state.mTransform, 3);
      pos = (targetPos - pos);
      pos.normalize();
      
      result.mLinearVelocity = pos * mSpeed;
      
      //add some randomization
      //osg::Vec3 randVector(dtUtil::RandFloat(-1.0f, 1.0f), dtUtil::RandFloat(-1.0f, 1.0f), dtUtil::RandFloat(0.0f, 2.0f));
      //randVector.normalize();
      //result.mLinearVelocity += randVector * (mSpeed * 0.25f);
   }

} //namespace NetDemo
