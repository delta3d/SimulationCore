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
#include <dtPhysics/physicsactcomp.h>

#include <dtGame/gameactor.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/matrixutil.h>

namespace NetDemo
{

   AIPhysicsModel::AIPhysicsModel()
      : mTimeStep(0.0f)
      , mCurrentState(NULL)
      , mGoalState(NULL)
   {

   }

   AIPhysicsModel::~AIPhysicsModel()
   {

   }

   void AIPhysicsModel::SetPhysicsActComp(dtPhysics::PhysicsActComp* newHelper)
   {
      mPhysicsActComp = newHelper;
   }

   dtPhysics::PhysicsActComp* AIPhysicsModel::GetPhysicsActComp()
   {
      return mPhysicsActComp.get();
   }

   void AIPhysicsModel::Init()
   {

   }

   void AIPhysicsModel::Update(float dt, BaseAIControllable& aiAgent)
   {
      mTimeStep = dt;
      ClampTimeStep();

      osg::Vec3 tempUpVector(0.0f, 0.0f, 1.0f);

      mCurrentState = &aiAgent.mCurrentState;
      mGoalState = &aiAgent.mGoalState;

      const BaseAIControls& steeringOut = aiAgent.mCurrentControls;

      UpdateTilt(steeringOut, tempUpVector);
      UpdateVelocity(steeringOut);

      UpdateRoll(steeringOut, tempUpVector);
      UpdateAngularVelocity(steeringOut);

      UpdateVerticalVelocity(steeringOut);

      UpdateHeading(steeringOut);
      UpdatePosition(steeringOut);

      OrthoNormalize(*mCurrentState);

      if(mPhysicsActComp.valid())
      {
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsActComp()->GetMainPhysicsObject();

         if(physicsObject != NULL)
         {
            osg::Vec3 up = mCurrentState->GetUp();
            osg::Vec3 at = mCurrentState->GetForward();
            
            float maxLiftForce = 20.0f;
            float maxThrustForce = 10.0f;

            osg::Vec3 force;
            float mass = physicsObject->GetBodyWrapper()->GetMass();

            float liftForce =(steeringOut.GetLift() * mass * maxLiftForce);
            dtUtil::ClampMin(liftForce, 0.0f);

            force += up * liftForce;
            force += at * (steeringOut.GetThrust() * mass * maxThrustForce);

            physicsObject->GetBodyWrapper()->AddForce(force);
         }
      }
   }


   void AIPhysicsModel::SetDefaultState(const osg::Matrix& matIn, BaseAIGameState& BaseAIGameState)
   {
      BaseAIGameState.SetVel(osg::Vec3(0.0f, 0.0f, 0.0f));
      BaseAIGameState.SetAccel(osg::Vec3(0.0f, 0.0f, 0.0f));
      BaseAIGameState.SetPitch(0.0f);
      BaseAIGameState.SetRoll(0.0f);
      BaseAIGameState.SetAngularAccel(0.0f);
      BaseAIGameState.SetAngularVel(0.0f);
      BaseAIGameState.SetVerticalVel(0.0f);
      BaseAIGameState.SetVerticalAccel(0.0f);

      osg::Vec3 fwd = dtUtil::MatrixUtil::GetRow3(matIn, 1);
      fwd.normalize();
      BaseAIGameState.SetForward(fwd);

      osg::Vec3 up = dtUtil::MatrixUtil::GetRow3(matIn, 2);
      up.normalize();
      BaseAIGameState.SetUp(up);

      BaseAIGameState.SetPos(osg::Vec3(matIn(3,0), matIn(3,1), matIn(3,2)));

      BaseAIGameState.SetTimeStep(0.0f);
      BaseAIGameState.SetVel(osg::Vec3());
   }

   void AIPhysicsModel::SetDefaultConstraints(BaseAIGoalState& goalStateIn) const
   {
      goalStateIn.SetMaxAngularVel(osg::DegreesToRadians(1000.0f));
      //goalStateIn.mMaxAngularVel = osg::DegreesToRadians(10.0f);
      //goalStateIn.mMaxAngularAccel = 200.0f * osg::DegreesToRadians(6.0f);
      goalStateIn.SetMaxAngularAccel(50.0f);
      goalStateIn.SetMaxVel(1000.0f);
      //goalStateIn.mMaxAccel = 200.0f * 8.77f;
      goalStateIn.SetMaxAccel(1000.0f);
      goalStateIn.SetMaxPitch(osg::DegreesToRadians(15.0f));
      goalStateIn.SetMaxRoll(osg::DegreesToRadians(30.0f));
      goalStateIn.SetMaxTiltPerSecond(osg::DegreesToRadians(5.0f));
      goalStateIn.SetMaxRollPerSecond(osg::DegreesToRadians(5.0f));
      //goalStateIn.mMaxVerticalVel(15.0f;//7.62f; //1500 feet/min
      goalStateIn.SetMaxVerticalVel(50.0f);//7.62f; //1500 feet/min
      goalStateIn.SetMaxVerticalAccel(50.0f);

      goalStateIn.SetMinElevation(25.0f);
      goalStateIn.SetMaxElevation(200.0f);
      goalStateIn.SetDragCoef(0.005f);
      goalStateIn.SetAngularDragCoef(0.005f);
      goalStateIn.SetVerticalDragCoef(200.005f);
   }

   void AIPhysicsModel::SetState(BaseAIGameState& state, const osg::Matrix& matIn)
   {
      osg::Vec3 fwd = dtUtil::MatrixUtil::GetRow3(matIn, 1);
      fwd.normalize();
      state.SetForward(fwd);

      osg::Vec3 up = dtUtil::MatrixUtil::GetRow3(matIn, 2);
      up.normalize();
      state.SetUp(up);

      state.SetPos(osg::Vec3(matIn(3,0), matIn(3,1),  matIn(3,2)));
   }

   void AIPhysicsModel::GetState(const BaseAIGameState& currentState, osg::Matrix& result) const
   {
      result(3,0) = currentState.GetPos()[0];
      result(3,1) = currentState.GetPos()[1];
      result(3,2) = currentState.GetPos()[2];

      osg::Vec3 rightVector = currentState.GetForward() ^ currentState.GetUp();
      rightVector.normalize();

      dtUtil::MatrixUtil::SetRow(result, currentState.GetForward(), 1);
      dtUtil::MatrixUtil::SetRow(result, currentState.GetUp(), 2);
      dtUtil::MatrixUtil::SetRow(result, rightVector, 0);
   }


   void AIPhysicsModel::OrthoNormalize(BaseAIGameState& currentState)
   {
      osg::Vec3 tempUpVector(0.0f, 0.0f, 1.0f);

      osg::Vec3 fwd = currentState.GetForward();
      fwd.normalize();
      currentState.SetForward(fwd);

      osg::Vec3 rightVector = currentState.GetForward() ^ tempUpVector;
      rightVector.normalize();
      osg::Vec3 up = rightVector ^ currentState.GetForward();
      up.normalize();
      currentState.SetUp(up);
   }

   void AIPhysicsModel::UpdateHeading(const BaseAIControls& controls)
   {
      float thetaAngle = mCurrentState->GetAngularVel() * mTimeStep;

      osg::Matrix rotation = osg::Matrix::rotate(thetaAngle, osg::Vec3(0.0, 0.0, 1.0));

      //since the AI does not currently use acceleration (just simplifies things quite a bit)
      //we are doing this to smooth out the ability to change heading
      osg::Vec3 deltaForward = osg::Matrix::transform3x3(mCurrentState->GetForward(), rotation);
      osg::Vec3 newForward = mCurrentState->GetForward() + mCurrentState->GetForward() + deltaForward;
      newForward /= 3.0f;
      mCurrentState->SetForward(newForward);
   }

   void AIPhysicsModel::UpdateAngularVelocity(const BaseAIControls& controls)
   {
      float newVelocity = controls.GetYaw() * mGoalState->GetMaxAngularVel();
      mCurrentState->SetAngularVel(newVelocity);
   }

   void AIPhysicsModel::UpdateVerticalVelocity(const BaseAIControls& controls)
   {
      //update acceleration
      mCurrentState->SetVerticalAccel(controls.GetLift() * mGoalState->GetMaxVerticalAccel());

      //update velocity
      mCurrentState->SetVerticalVel(mCurrentState->GetVerticalVel() + (mCurrentState->GetVerticalAccel() * mCurrentState->GetTimeStep()));
      mCurrentState->SetVerticalVel(mCurrentState->GetVerticalVel() - (mCurrentState->GetVerticalVel() * mGoalState->GetVerticalDragCoef()));
      mCurrentState->SetVerticalVel(Clamp(mCurrentState->GetVerticalVel(), -mGoalState->GetMaxVerticalVel(), mGoalState->GetMaxVerticalVel()));
   }

   void AIPhysicsModel::UpdatePosition(const BaseAIControls& controls)
   {
      osg::Vec3 newPos = mCurrentState->GetPos() + (mCurrentState->GetVel() * mCurrentState->GetTimeStep());
      newPos[2] += mCurrentState->GetVerticalVel() * mCurrentState->GetTimeStep();
      mCurrentState->SetPos(newPos);
   }

   void AIPhysicsModel::UpdateVelocity(const BaseAIControls& controls)
   {
      float newVelocity = (controls.GetThrust() * mGoalState->GetMaxVel());
      float maxAccel = mGoalState->GetMaxAccel() * mCurrentState->GetTimeStep();

      osg::Vec3 newVel = mCurrentState->GetVel();
      newVel += mCurrentState->GetForward() * Clamp(newVelocity - mCurrentState->GetVel().length(), -maxAccel, maxAccel);
      newVel -= (mCurrentState->GetVel() * mGoalState->GetDragCoef());


      //we don't clamp the controls so this is necessary
      if (newVel.length() > mGoalState->GetMaxVel())
      {
         newVel.normalize();
         newVel *= mGoalState->GetMaxVel();
      }
      mCurrentState->SetVel(newVel);
   }

   void AIPhysicsModel::UpdateTilt(const BaseAIControls& controls, osg::Vec3& tilt)
   {
      mCurrentState->SetPitch(Dampen(mCurrentState->GetPitch(), (controls.GetThrust() * mGoalState->GetMaxPitch()), mGoalState->GetMaxTiltPerSecond() * mCurrentState->GetTimeStep(), (mCurrentState->GetPitch() / mGoalState->GetMaxPitch())));

      mCurrentState->SetPitch(Clamp(mCurrentState->GetPitch(), -mGoalState->GetMaxPitch(), mGoalState->GetMaxPitch()));

      osg::Vec3 rightVec = (mCurrentState->GetForward() ^ mCurrentState->GetUp());
      rightVec.normalize();

      osg::Matrix rotation = osg::Matrix::rotate( -mCurrentState->GetPitch(), rightVec);

      tilt = osg::Matrix::transform3x3(tilt, rotation); 

   }


   void AIPhysicsModel::UpdateRoll(const BaseAIControls& controls, osg::Vec3& roll)
   {
      mCurrentState->SetRoll(Dampen(mCurrentState->GetRoll(), controls.GetYaw() * mGoalState->GetMaxRoll(), mGoalState->GetMaxRollPerSecond() * mCurrentState->GetTimeStep(), (mCurrentState->GetRoll() / mGoalState->GetMaxRoll())));

      //mCurrentState->mRoll *= ((0.1 + mCurrentState->GetVel().length()) / mGoalState->mMaxVel);

      if (mCurrentState->GetRoll() > mGoalState->GetMaxRoll()) mCurrentState->SetRoll(mGoalState->GetMaxRoll());
      else if (mCurrentState->GetRoll() < -mGoalState->GetMaxRoll()) mCurrentState->SetRoll(-mGoalState->GetMaxRoll());

      osg::Matrix rotation = osg::Matrix::rotate(-mCurrentState->GetRoll(), mCurrentState->GetForward());

      roll = osg::Matrix::transform3x3(roll, rotation); 

   }

   float AIPhysicsModel::Clamp(float x, float from, float to)
   {
      if(x < from)
         return from;
      else if(x > to)
         return to;
      else return x;
   }

   float AIPhysicsModel::Dampen(float last, float current, float pmax, float falloff)
   {
      if(current > last)
      {
         float adjust = pmax * (1.0f - falloff);

         if(current - last >= adjust)
            return last + adjust;
         else return current;
      }
      else
      {
         // float adjust = pmax * falloff;
         if(last - current >= pmax)
            return last - pmax;
         else return current;
      }
   }

   float AIPhysicsModel::GetCurrentTimeStep()
   {
      ClampTimeStep();
      return mTimeStep;
   }

   void AIPhysicsModel::ClampTimeStep()
   {
      //we will allow ticking from 10fps to 100fps
      const float MAX_TICK = 0.1f;
      const float MIN_TICK = 0.01f;

      if(mTimeStep > MAX_TICK) mTimeStep = MAX_TICK;

      if(mTimeStep > MAX_TICK) mTimeStep = MAX_TICK;
      else if(mTimeStep < MIN_TICK) mTimeStep = MIN_TICK;
   }


}//namespace NetDemo
