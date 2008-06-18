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
