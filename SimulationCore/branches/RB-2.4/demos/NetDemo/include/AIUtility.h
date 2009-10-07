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

#ifndef NETDEMO_AIUTILITY_H
#define NETDEMO_AIUTILITY_H

#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Matrix>

#include <dtAI/steeringutility.h>
#include <dtAI/steeringbehavior.h>

//for the steering pipeline
#include <dtUtil/objectfactory.h>

//include needed just for getting to PathData internal to BezierController
//todo- move PathData out of BezierController
#include <dtABC/beziercontroller.h>

//used for the waypoint array typedef WaypointPath
//todo- move WaypointArray out of AIPluginInterface
#include <dtAI/aiplugininterface.h>

//use a property container to hold ai data
#include <dtDAL/propertymacros.h>

#include <dtUtil/log.h>

#include <dtAI/steeringpipeline.h>

namespace NetDemo
{

   struct Kinematic
   {
      osg::Matrix mTransform;
      osg::Vec3 mLinearVelocity;
      osg::Vec3 mAngularVelocity;
   };

   struct SteeringOutput
   {
      //these are the control inputs
      //all are floats from 1 to -1
      //which represents percentage of maximum
      float mThrust, mLift, mYaw;

      osg::Vec3 mLinearVelocity;

      SteeringOutput()
      {
         Reset();
      }

      void Reset()
      {
         mThrust = 0.0f;
         mLift = 0.0f;
         mYaw = 0.0f;
         mLinearVelocity.set(0.0f, 0.0f, 0.0f);
      }
   };

   typedef std::vector<dtABC::BezierController::PathData>  BezierPath;
   typedef dtAI::AIPluginInterface::WaypointArray WaypointPath;
   typedef dtAI::SteeringBehavior<dtAI::KinematicGoal, Kinematic, SteeringOutput> SteeringBehaviorType;

   //class ErrorCondition: public dtUtil::Enumeration
   //{
   //   DECLARE_ENUM(ErrorCondition);
   //public:
   //   ErrorCondition(const std::string&);
   //};

   //this allows for a default behavior that doesn't do anything... idle?
   class DoNothing: public SteeringBehaviorType
   {
   public:
      typedef SteeringBehaviorType BaseClass;
      DoNothing(){}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:
   };

   class BombDive: public SteeringBehaviorType
   {
   public:
      typedef SteeringBehaviorType BaseClass;
      BombDive(float speed): mSpeed(speed){}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:
      float mSpeed;
   };


}//namespace NetDemo

#endif //NETDEMO_AIUTILITY_H
