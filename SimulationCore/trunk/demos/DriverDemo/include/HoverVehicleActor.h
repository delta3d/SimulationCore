/* -*-c++-*-
* Driver Demo - HoverVehicleActor (.cpp & .h) - Using 'The MIT License'
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
#ifdef AGEIA_PHYSICS
#ifndef _HOVER_VEHICLE_ACTOR_
#define _HOVER_VEHICLE_ACTOR_

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
   /* This class extends BasePhysicsVehicle and has hover tank behavior.
    */
   class DRIVER_DEMO_EXPORT HoverVehicleActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         /// Constructor
         HoverVehicleActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

      protected:
         /// Destructor
         virtual ~HoverVehicleActor();
      
      // INHERITED PUBLIC
      public:
         //virtual void TickLocal(const dtGame::Message &tickMessage);

         //virtual void TickRemote(const dtGame::Message &tickMessage);

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


      // PUBLIC METHODS
      public:

         /// Reset to starting position In additional to base behavior, it turns off sounds.
         virtual void ResetVehicle();

         /// Turns it up and moves up
         //void RepositionVehicle(float deltaTime);
         
         HoverVehiclePhysicsHelper* GetHoverPhysicsHelper() {
            return static_cast<HoverVehiclePhysicsHelper*> (GetPhysicsHelper());}

         //void SetSound_effect_collision_hit(const std::string& value){SOUND_EFFECT_COLLISION_HIT=value;}
         //void SetVehicleInsideModel(const std::string &value)  {VEHICLE_INSIDE_MODEL = value;}
         //const std::string& GetSound_effect_collision_hit(){return SOUND_EFFECT_COLLISION_HIT;}

         /// Turns it up and moves up
         virtual void RepositionVehicle(float deltaTime);

         /// These methods are kind of odd. Some vehicles have a distinct turret (no up/down, just rotate) that is 
         /// separate from the vehicle. On others, the turret is hard attached to the vehicle. 
         /// In this case, turning the motion model will rotate the vehicle.
         void SetVehicleIsTurret( bool vehicleIsTurret ) { mVehicleIsTurret = vehicleIsTurret; }
         bool GetVehicleIsTurret() const { return mVehicleIsTurret; }

         virtual void ApplyForce( const osg::Vec3& force, const osg::Vec3& location );

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
         
         //GearSoundLevel    mLastGearChange;     /// So we know when to play a sound.

         ///////////////////////////////////////////////////
         // Sound effects
         dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
         ///////////////////////////////////////////////////

         ///////////////////////////////////////////////////
         // properties
         //std::string VEHICLE_INSIDE_MODEL;      /// for interior views
         //std::string SOUND_EFFECT_COLLISION_HIT;/// What is the filepath / string of the sound effect
         ///////////////////////////////////////////////////

         float mTimeTillJumpReady;
         bool mVehicleIsTurret;

         dtCore::RefPtr<VehicleShield> mShield;
   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class DRIVER_DEMO_EXPORT HoverVehicleActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         HoverVehicleActorProxy();
         virtual void BuildPropertyMap();

      protected:
         virtual ~HoverVehicleActorProxy();
         void CreateActor();
         virtual void OnEnteredWorld();
   };

}

#endif
#endif
