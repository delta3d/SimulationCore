/* -*-c++-*-
 * Driver Demo - HoverTargetPhysicsActComp (.cpp & .h) - Using 'The MIT License'
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
 * @author Curtiss Murphy
 */
#ifndef _HOVER_TARGET_PHYSICS_HELPER_
#define _HOVER_TARGET_PHYSICS_HELPER_

#include <DriverExport.h>
#include <dtPhysics/physicsactcomp.h>
//#include <NxAgeiaPhysicsActComp.h>
//#include <NxAgeiaWorldComponent.h>
#include <SimCore/PhysicsTypes.h>
#include <osgSim/DOFTransform>

namespace DriverDemo
{
   //#define NUM_BRAKE_LEVELS_FOR_TABLE 10
   class DRIVER_DEMO_EXPORT HoverTargetPhysicsActComp : public dtPhysics::PhysicsActComp
   {
   public:
      typedef dtPhysics::PhysicsActComp BaseClass;

      HoverTargetPhysicsActComp();

   protected:
      virtual ~HoverTargetPhysicsActComp();

   private:


      //////////////////////////////////
      // Properties
      float  mGroundClearance;       /// How far above the ground we should be.

      osg::Vec3 mTotalForceAppliedLastTime;

   public:

      float GetCurentMPH();

      /**
       * /brief Purpose : To create the target's physics structure
       */
      //bool CreateTarget(osg::Vec3 &startVec, bool isRemote);
      bool CreateTarget(const dtCore::Transform& transformForRot, osg::Node* bodyNode);

      float ComputeEstimatedForceCorrection(const osg::Vec3& location,
               const osg::Vec3& direction, float& distanceToHit);

      void ApplyTargetHoverForces(float deltaTime, osg::Vec3& goalLocation);
      void ApplyForceFromLastFrame(float deltaTime);

      //////////////////////////////////////////////////////////////////////////////////////
      //                                    Properties                                    //
      //////////////////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////
      // Build the property list for the actor
      virtual void BuildPropertyMap();

      float GetSphereRadius();

      float GetGroundClearance() const;
      void SetGroundClearance(float value);

   };
}

#endif
