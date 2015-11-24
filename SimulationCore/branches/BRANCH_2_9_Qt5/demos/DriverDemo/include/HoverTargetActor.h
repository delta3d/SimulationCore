/* -*-c++-*-
* Driver Demo - HoverTargetActor (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
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
//#ifdef AGEIA_PHYSICS
#ifndef _HOVER_TARGET_ACTOR_
#define _HOVER_TARGET_ACTOR_

#include <DriverExport.h>

#include <HoverTargetPhysicsHelper.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

namespace dtAudio
{
   class Sound;
}

namespace SimCore
{
   class DetonationMessage;

   namespace Actors
   {
      class MunitionTypeActor;
   }
}

namespace DriverDemo
{
   class VehicleShield;


   ////////////////////////////////////////////////////////////////////////////////
   /* This class extends BasePhysicsVehicle. It is intended to be a simple target
    * that you can shoot at. It will publish itself and can be shot by remote players.
    */
   class DRIVER_DEMO_EXPORT HoverTargetActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         /// Constructor
         HoverTargetActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy);

      protected:
         /// Destructor
         virtual ~HoverTargetActor();

      // INHERITED PUBLIC
      public:

         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         //////////////// PROPERTIES


      // PUBLIC METHODS
      public:
         float ComputeEstimatedForceCorrection(const osg::Vec3& location,
            const osg::Vec3& direction, float& distanceToHit);

         HoverTargetPhysicsActComp* GetTargetPhysicsActComp() {
            return static_cast<HoverTargetPhysicsActComp*> (GetPhysicsActComp());
         }

         /// Overridden from Base Entity. Allows us to take more or less damage from a hit.
         virtual float ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message,
            const SimCore::Actors::MunitionTypeActor& munition);


      protected:
         /// Angles/ steering moving etc done here. Of the updates, this is called first.
         /// This does nothing by default.
         virtual void UpdateVehicleTorquesAndAngles(float deltaTime);

         /// Called update the dofs for your vehicle. Wheels or whatever. Of the updates, this is called second
         /// By default, this does nothing.
         virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);

         /// called from tick. Do your sounds. Of the updates, this is called third.
         /// Does nothing by default.
         virtual void UpdateSoundEffects(float deltaTime);

         bool CheckAndUpdatePerformanceThrottle(float deltaTime);

      // Private vars
      private:

         ///////////////////////////////////////////////////
         // Sound effects
         dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
         ///////////////////////////////////////////////////

         //float  mVehicleBaseWeight;     /// How much does the vehicle weight
         //float  mSphereRadius;          /// The radius (meters) of the hover sphere. Used to calculate lots of things...
         float  mGroundClearance;       /// How far above the ground we should be.

         osg::Vec3 mGoalLocation;       /// The general location we want to be.

         float mTimeSinceKilled;        /// How long it's been since the target was killed, delete after like 20 seconds
         float mTimeSinceBorn;          /// How long we've been alive - so we can time out after a while.

         int mPerfThrottleCountDown; // num of frames we will skip if we had to slow down for perf reasons


         dtCore::RefPtr<VehicleShield> mShield;
   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class DRIVER_DEMO_EXPORT HoverTargetActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         HoverTargetActorProxy();
         virtual void BuildPropertyMap();
         /// Override this to add your own components or to init values on the ones that are already added.
         virtual void BuildActorComponents();

      protected:
         virtual ~HoverTargetActorProxy();
         void CreateDrawable();
         virtual void OnEnteredWorld();
   };

}

#endif
//#endif
