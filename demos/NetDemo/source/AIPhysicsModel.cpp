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

#include <AIPhysicsModel.h>
#include <dtPhysics/physicshelper.h>

#include <dtGame/gameactor.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>

#include <dtUtil/matrixutil.h>

namespace NetDemo
{

   AIPhysicsModel::AIPhysicsModel()
   {

   }

   AIPhysicsModel::~AIPhysicsModel()
   {

   }


   void AIPhysicsModel::SetKinematicState(const Kinematic& ko)
   {
     mKinematicState = ko;
   }

   void AIPhysicsModel::SetPhysicsHelper(dtPhysics::PhysicsHelper* newHelper)
   {
      mPhysicsHelper = newHelper;
   }

   dtPhysics::PhysicsHelper* AIPhysicsModel::GetPhysicsHelper()
   {
      return mPhysicsHelper.get();
   }

   void AIPhysicsModel::Init()
   {

   }

   void AIPhysicsModel::Update(const SteeringOutput& steeringOut, float dt)
   {
      if(mPhysicsHelper.valid())
      {
         //todo: all this should be a derivative of an interface
         //       it is currently specific to the EnemyMine
         //       also we need a generic way to set physical constraints

         //TODO: this doesnt seem to work!
         //osg::Vec3 right = dtUtil::MatrixUtil::GetRow3(mKinematicState.mTransform, 0);
         //osg::Vec3 up(0.0f, 0.0f, 1.0f); //= dtUtil::MatrixUtil::GetRow3(mKinematicState.mTransform, 2);
         //osg::Vec3 at = dtUtil::MatrixUtil::GetRow3(mKinematicState.mTransform, 1);

         //float maxLiftForce = 100.0f;
         //float maxThrustForce = 100.0f;
         //float maxYawForce = 100.0f;

         //osg::Vec3 force;

         //force += osg::Vec3(0.0f, 0.0f, 100.0f) + (up * (steeringOut.mLift * maxLiftForce));
         //force += at * (steeringOut.mThrust * maxThrustForce);
         //force += right * (steeringOut.mYaw * maxYawForce);
         //
         //GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->AddForce(force);
         GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->AddForce(steeringOut.mLinearVelocity);

      }
   }

}//namespace NetDemo
