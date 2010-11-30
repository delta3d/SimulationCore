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
#ifndef NETDEMO_AIPHYSICSMODEL_H
#define NETDEMO_AIPHYSICSMODEL_H

#include <DemoExport.h>
#include <AIUtility.h>
#include <osg/Referenced>
#include <dtCore/refptr.h>

namespace dtPhysics
{
   class PhysicsActComp;
}

namespace NetDemo
{

   class NETDEMO_EXPORT AIPhysicsModel: public osg::Referenced
   {
   public:
     AIPhysicsModel();

     virtual void Init();

     virtual void Update(float dt, BaseAIControllable& aiAgent);

     void SetPhysicsActComp(dtPhysics::PhysicsActComp* newHelper);
     dtPhysics::PhysicsActComp* GetPhysicsActComp();

     void SetState(BaseAIGameState& BaseAIGameState, const osg::Matrix& matIn);
     void GetState(const BaseAIGameState& stateIn, osg::Matrix& result) const;
     void SetDefaultState(const osg::Matrix& matIn, BaseAIGameState& BaseAIGameState);

     void SetDefaultConstraints(BaseAIGoalState& goalStateIn) const;

     //not const because it may clamp timestep
     float GetCurrentTimeStep();

   protected:
     AIPhysicsModel(const AIPhysicsModel&);  //not implemented by design
     AIPhysicsModel& operator=(const AIPhysicsModel&);  //not implemented by design
     ~AIPhysicsModel();

     void ClampTimeStep();
     void UpdateHeading(const BaseAIControls& controls);
     void UpdatePosition(const BaseAIControls& controls);
     void UpdateVelocity(const BaseAIControls& controls);
     void UpdateAngularVelocity(const BaseAIControls& controls);
     void UpdateVerticalVelocity(const BaseAIControls& controls);
     void UpdateTilt(const BaseAIControls& controls, osg::Vec3& tilt);
     void UpdateRoll(const BaseAIControls& controls, osg::Vec3& roll);
     void OrthoNormalize(BaseAIGameState& stateIn);
     float Clamp(float x, float from, float to);
     float Dampen(float last, float current, float max, float falloff);

     float mTimeStep;

     BaseAIGameState* mCurrentState;
     BaseAIGoalState* mGoalState;
     dtCore::RefPtr<dtPhysics::PhysicsActComp> mPhysicsActComp;

   };

}//namespace NetDemo

#endif //NETDEMO_AIPHYSICSMODEL_H
