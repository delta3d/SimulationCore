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
#ifndef NETDEMO_AISTEERINGMODEL_H
#define NETDEMO_AISTEERINGMODEL_H

#include <DemoExport.h>
#include <AIUtility.h>

#include <dtCore/propertymacros.h>
#include <dtAI/controllable.h>
#include <dtAI/steeringbehavior.h>
#include <dtAI/steeringpipeline.h>
#include <osg/Matrix>
#include <osg/Vec3>

#include <stack>
#include <osg/Referenced>
#include <dtCore/refptr.h>


namespace NetDemo
{

   class NETDEMO_EXPORT AISteeringModel:  public dtAI::SteeringPipeline<BaseAIControllable>,
                                          public osg::Referenced
   {
   public:
      typedef dtAI::SteeringPipeline<BaseAIControllable> BaseClass;
      typedef std::vector<BaseAISteeringBehavior*> SteeringBehaviorArray; 

     AISteeringModel();

     virtual void Init();

     void OutputControl(const BaseAIControllable::PathType& pathToFollow, const BaseAIControllable::StateType& current_state, BaseAIControllable::ControlType& result) const;

     const SteeringBehaviorArray& GetSteeringBehaviors() const;
     bool SetCurrentSteeringBehavior(unsigned id);

     unsigned AddSteeringBehavior(BaseAISteeringBehavior* steeringbehavior);

   protected:
     AISteeringModel(const AISteeringModel&);  //not implemented by design
     AISteeringModel& operator=(const AISteeringModel&);  //not implemented by design
     ~AISteeringModel();


     DT_DECLARE_ACCESSOR_INLINE(float, TimeStep)

     unsigned mCurrentBehavior;
     SteeringBehaviorArray mSteeringBehaviors;

     //BaseSteeringTargeter* mTargeter;

   };

} //namespace NetDemo

#endif //NETDEMO_AISTEERINGMODEL_H
