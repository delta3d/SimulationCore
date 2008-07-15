/* -*-c++-*-
* Driver Demo
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*
* @author Curtiss Murphy
*/
#ifdef AGEIA_PHYSICS
#ifndef _HOVER_TARGET_ACTOR_
#define _HOVER_TARGET_ACTOR_

#include <DriverExport.h>

#include <HoverVehiclePhysicsHelper.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

namespace dtAudio
{
   class Sound;
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
         HoverTargetActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

      protected:
         /// Destructor
         virtual ~HoverTargetActor();
      
      // INHERITED PUBLIC
      public:
         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
         //virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, 
         //   NxActor& ourSelf, NxActor& whatWeHit);

         // You would have to make a new raycast to get this report,
         // so no flag associated with it.
         //virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, 
         //   const NxActor& whatWeHit){}

         /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
         virtual void AgeiaPrePhysicsUpdate();

         /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
         virtual void AgeiaPostPhysicsUpdate();

         //////////////// PROPERTIES

         float GetVehicleBaseWeight() {return mVehicleBaseWeight;}        
         void SetVehicleBaseWeight(float value)  {mVehicleBaseWeight = value;}        

         float GetSphereRadius() {return mSphereRadius;}        
         void SetSphereRadius(float value)  {mSphereRadius = value;}        

         float GetGroundClearance() {return mGroundClearance;}        
         void SetGroundClearance(float value)  {mGroundClearance = value;}        


      // PUBLIC METHODS
      public:
         float ComputeEstimatedForceCorrection(const osg::Vec3 &location, 
            const osg::Vec3 &direction, float &distanceToHit);

         /// Reset to starting position In additional to base behavior, it turns off sounds.
         //virtual void ResetVehicle();

         //HoverVehiclePhysicsHelper* GetHoverPhysicsHelper() {
         //   return static_cast<HoverVehiclePhysicsHelper*> (GetPhysicsHelper());}

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


      // Private vars
      private:
         
         ///////////////////////////////////////////////////
         // Sound effects
         dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
         ///////////////////////////////////////////////////

         float  mVehicleBaseWeight;     /// How much does the vehicle weight
         float  mSphereRadius;          /// The radius (meters) of the hover sphere. Used to calculate lots of things...
         float  mGroundClearance;       /// How far above the ground we should be.

         osg::Vec3 mGoalLocation;       /// The general location we want to be.

         float mTimeSinceKilled;        /// How long it's been since the target was killed, delete after like 20 seconds
         float mTimeSinceBorn;          /// How long we've been alive - so we can time out after a while.


         dtCore::RefPtr<VehicleShield> mShield;
   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class DRIVER_DEMO_EXPORT HoverTargetActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         HoverTargetActorProxy();
         virtual void BuildPropertyMap();

      protected:
         virtual ~HoverTargetActorProxy();
         void CreateActor();
         virtual void OnEnteredWorld();
   };

}

#endif
#endif
