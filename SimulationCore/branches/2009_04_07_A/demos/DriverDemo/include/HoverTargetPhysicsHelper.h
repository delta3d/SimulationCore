/* -*-c++-*-
* Driver Demo - HoverTargetPhysicsHelper (.cpp & .h) - Using 'The MIT License'
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
//#include <Vehicles/NxWheelDesc.h>
#include <NxAgeiaPhysicsHelper.h>
#include <NxAgeiaWorldComponent.h>
#include <osgSim/DOFTransform>

namespace DriverDemo
{
   //#define NUM_BRAKE_LEVELS_FOR_TABLE 10
   class DRIVER_DEMO_EXPORT HoverTargetPhysicsHelper : public dtAgeiaPhysX::NxAgeiaPhysicsHelper
   {
      public:
         HoverTargetPhysicsHelper(dtGame::GameActorProxy &proxy);

      protected:
         virtual ~HoverTargetPhysicsHelper();

      private:


         //////////////////////////////////
         // Properties
         //float  mVehicleMaxForwardMPH;  /// The max mph you want your vehicle to have - conceptual, not actual. Multiplied times mForceBoostFactor
         //float  mVehicleMaxStrafeMPH;   /// The max reverse mph you can go - conceptual, not actual. Multiplied times mForceBoostFactor
         float  mVehicleBaseWeight;     /// How much does the vehicle weight
         float  mSphereRadius;          /// The radius (meters) of the hover sphere. Used to calculate lots of things...
         float  mGroundClearance;       /// How far above the ground we should be.
         //float  mForceBoostFactor;      /// Force boost factor is multiplied time max speeds to determine impulse force

      public:

         float GetCurentMPH();

         /// Turns it up and moves up
         //void RepositionVehicle(float deltaTime);

         /**
          * Call this method each frame from your actor after you have determined which direction to accelerate. 
          * Note, the method may not correct you if you say you accelerated forward and back, left and right 
          * at the same time - if it does check, left wins over right & forward wins over reverse.
          */
         //void UpdateVehicle(float deltaTime, bool accelForward, bool accelReverse, bool accelLeft, bool accelRight);

         /**
          * /brief Purpose : To create the target's physics structure
          */ 
         bool CreateTarget(osg::Vec3 &startVec, bool isRemote); 

         float ComputeEstimatedForceCorrection(const osg::Vec3 &location, 
            const osg::Vec3 &direction, float &distanceToHit);

         void ApplyTargetHoverForces(float deltaTime, osg::Vec3 &goalLocation);

         //////////////////////////////////////////////////////////////////////////////////////
         //                                    Properties                                    //
         //////////////////////////////////////////////////////////////////////////////////////

         //////////////////////////////////////////////////////////////////
         // Build the property list for the actor
         virtual void BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn);

         //float GetVehicleMaxForwardMPH() {return mVehicleMaxForwardMPH;}            
         //void SetVehicleMaxForwardMPH(float value) {mVehicleMaxForwardMPH = value;}            

         //float GetVehicleMaxStrafeMPH() {return mVehicleMaxStrafeMPH;}     
         //void SetVehicleMaxStrafeMPH(float value)  {mVehicleMaxStrafeMPH = value;}     

         float GetVehicleBaseWeight() {return mVehicleBaseWeight;}        
         void SetVehicleBaseWeight(float value)  {mVehicleBaseWeight = value;}        

         float GetSphereRadius() {return mSphereRadius;}        
         void SetSphereRadius(float value)  {mSphereRadius = value;}        

         float GetGroundClearance() {return mGroundClearance;}        
         void SetGroundClearance(float value)  {mGroundClearance = value;}        

         //float GetForceBoostFactor() {return mForceBoostFactor;}        
         //void SetForceBoostFactor(float value)  {mForceBoostFactor = value;}        
   };
}

#endif
