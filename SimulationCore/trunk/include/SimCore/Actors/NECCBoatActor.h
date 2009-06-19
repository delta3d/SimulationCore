/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
* @author Allen Danklefsen
*/
#ifdef AGEIA_PHYSICS
#ifndef _NECC_BOAT_VEHICLE_
#define _NECC_BOAT_VEHICLE_

#include <SimCore/Export.h>

#include <NxAgeiaPrimitivePhysicsHelper.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/VehicleInterface.h>

#include <SimCore/PhysicsTypes.h>

namespace dtAudio
{
   class Sound;
}

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT NECCBoatActor : public Platform,
                                           public dtAgeiaPhysX::NxAgeiaPhysicsInterface,
                                           public VehicleInterface
      {
         public:
            /// Constructor
            NECCBoatActor (PlatformActorProxy &proxy);

         protected:
            /// Destructor
            virtual ~NECCBoatActor();

         // INHERITED PUBLIC
         public:
            /**
            * This method is an invokable called when an object is local and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void OnTickLocal(const dtGame::TickMessage &tickMessage);

            /**
            * This method is an invokable called when an object is remote and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void OnTickRemote(const dtGame::TickMessage &tickMessage);

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            virtual void AgeiaPrePhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport,
               dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit);

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const dtPhysics::PhysicsObject& ourSelf,
               const dtPhysics::PhysicsObject& whatWeHit){}

            /// Utility Methods
            virtual float GetMPH();

            /// Reset to starting position, good for when u crash ;p
            void ResetVehicle();

            /// Turns it up and moves up
            void RepositionVehicle(float deltaTime);

            virtual bool ShouldForceUpdate( const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper* GetPhysicsHelper() {return mPhysicsHelper.get();}

            void SetSound_effect_ignition(const std::string& value){SOUND_EFFECT_IGNITION=value;}
            void SetSound_effect_vehicle_loop(const std::string& value){SOUND_EFFECT_VEHICLE_LOOP=value;}
            void SetSound_effect_collision_hit(const std::string& value){SOUND_EFFECT_COLLISION_HIT=value;}
            void SetSound_Effect_Horn(const std::string& value) {SOUND_EFFECT_HORN_SOUND = value;}
            virtual void SetHasDriver( bool hasDriver )           { mHasDriver = hasDriver; }

            std::string GetSound_effect_ignition() const {return SOUND_EFFECT_IGNITION;}
            std::string GetSound_effect_vehicle_loop() const {return SOUND_EFFECT_VEHICLE_LOOP;}
            std::string GetSound_effect_collision_hit() const {return SOUND_EFFECT_COLLISION_HIT;}
            std::string GetSound_Effect_Horn() const {return SOUND_EFFECT_HORN_SOUND;}
            virtual bool GetHasDriver() const       { return mHasDriver; }

         // Private vars
         private:

            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper> mPhysicsHelper;

            ///////////////////////////////////////////////////
            // Sound effects
            dtCore::RefPtr<dtAudio::Sound> mSndIgnition;
            dtCore::RefPtr<dtAudio::Sound> mSndVehicleIdleLoop;
            dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
            dtCore::RefPtr<dtAudio::Sound> mSndHorn;
            ///////////////////////////////////////////////////

            ///////////////////////////////////////////////////
            // properties
            std::string SOUND_EFFECT_IGNITION;     /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_VEHICLE_LOOP; /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_COLLISION_HIT;/// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_HORN_SOUND;
            ///////////////////////////////////////////////////

            // kept here so we can play with the forces directly.
            NxWheelShape* mWheels[3];

            /// current speed of the vehicle.
            float mVehicleMPH;

            /// The max mph you want your vehicle to have
            float mVehicleMaxMPH;

            /// The max reverse mph you can go
            float mVehicleMaxReverseMPH;

            // for the front wheel
            float mCurrentSteeringAngle;

            // so we have a bob up and down while moving.
            float mBobbingTimer;
            float mDegreesDifference;
            bool mBobbingUp;

            ///////////////////////////////////////////////////
            // vehicles portal for the actor
            dtCore::RefPtr<dtGame::GameActorProxy> mVehiclesPortal;

            ///////////////////////////////////////////////////
            // sending out dead reckoning
            float mTimeForSendingDeadReckoningInfoOut;
            float mTimesASecondYouCanSendOutAnUpdate;

            ///////////////////////////////////////////////////
            // is there currently a driver inside?
            bool mHasDriver;

            ///////////////////////////////////////////////////
            // Should this vehicle send a full actor update when asked?
            bool mNotifyFullUpdate;
            bool mNotifyPartialUpdate;

            /// Called internally to update the dofs for the wheels to match ageias wheel counterpoints
            void UpdateRotationDOFS(float deltaTime, bool insideVehicle);

            /// Angles/ steering moving etc done here.
            void UpdateVehicleTorquesAndAngles(float deltaTime);

            /// called from tick.
            void UpdateSoundEffects(float deltaTime);

            /// called from tick, does local dead reckon, determines to send out updates.
            void UpdateDeadReckoning(float deltaTime);

            /// utility function for the UpdatedeadReckoning function
            float GetPercentageChangeDifference(float startValue, float newValue);

            void CreateBoatVehicle();
            void CheckForGroundCollision();

            static const int MAX_BOBBING_AMOUNT = 2;
            const float mMaxBobbingTimeAmount;
      };

      class SIMCORE_EXPORT NECCBoatActorProxy : public PlatformActorProxy
      {
         public:
            NECCBoatActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~NECCBoatActorProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
      };

   }
}

#endif
#endif
