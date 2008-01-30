/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen
*/
#ifdef AGEIA_PHYSICS
#ifndef _NECC_BOAT_VEHICLE_
#define _NECC_BOAT_VEHICLE_

#include <SimCore/Export.h>

#include <NxAgeiaFourWheelVehiclePhysicsHelper.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/VehicleInterface.h>

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
            virtual void TickLocal(const dtGame::Message &tickMessage);

            /**
            * This method is an invokable called when an object is remote and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void TickRemote(const dtGame::Message &tickMessage);

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            virtual void AgeiaPrePhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, 
               NxActor& ourSelf, NxActor& whatWeHit);

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, 
               const NxActor& whatWeHit){}

            /**
            * Handle forces received from the environment, such as detonations and impacts
            */
            virtual void ApplyForce( const osg::Vec3& force, const osg::Vec3& location );

            bool CompareVectors( const osg::Vec3& op1, const osg::Vec3& op2, float epsilon );

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
            virtual void SetHasDriver( bool hasDriver )           { mHasDriver = hasDriver; }
               
            const std::string& GetSound_effect_ignition() {return SOUND_EFFECT_IGNITION;}     
            const std::string& GetSound_effect_vehicle_loop(){return SOUND_EFFECT_VEHICLE_LOOP;} 
            const std::string& GetSound_effect_collision_hit(){return SOUND_EFFECT_COLLISION_HIT;}
            virtual bool GetHasDriver() const       { return mHasDriver; }
           
         // Private vars
         private:
            
            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper> mPhysicsHelper;

            ///////////////////////////////////////////////////
            // Sound effects
            dtCore::RefPtr<dtAudio::Sound> mSndIgnition;
            dtCore::RefPtr<dtAudio::Sound> mSndVehicleIdleLoop;
            dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
            ///////////////////////////////////////////////////

            ///////////////////////////////////////////////////
            // properties
            std::string SOUND_EFFECT_IGNITION;     /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_VEHICLE_LOOP; /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_COLLISION_HIT;/// What is the filepath / string of the sound effect
            ///////////////////////////////////////////////////

            /// current speed of the vehicle.
            float mVehicleMPH;

            /// The max mph you want your vehicle to have
            float mVehicleMaxMPH;            

            /// The max reverse mph you can go 
            float mVehicleMaxReverseMPH;     

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
