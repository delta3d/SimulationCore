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
#ifndef _NX_AGEIA_VEHICLE_
#define _NX_AGEIA_VEHICLE_

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
      class SIMCORE_EXPORT NxAgeiaFourWheelVehicleActor : public Platform, 
                                                       public dtAgeiaPhysX::NxAgeiaPhysicsInterface, 
                                                       public VehicleInterface
      {
         public:
            /// Constructor
            NxAgeiaFourWheelVehicleActor (PlatformActorProxy &proxy);

         protected:
            /// Destructor
            virtual ~NxAgeiaFourWheelVehicleActor();
         
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

         // PUBLIC CLASSES
         public:
            /// Utility Methods
            virtual float GetMPH();
            float GetBrakeTorque();

            /// Reset to starting position, good for when u crash ;p
            void ResetVehicle();

            /// Turns it up and moves up
            void RepositionVehicle(float deltaTime);

            virtual bool ShouldForceUpdate( const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            enum WHEEL_NAMES {FRONT_LEFT = 0, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT};
            
            dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper* GetPhysicsHelper() {return mPhysicsHelper.get();}

            void SetSound_brake_squeal_amount( float value)       {SOUND_BRAKE_SQUEAL_AMOUNT=value;}    
            void SetSound_gear_change_low( float value)           {SOUND_GEAR_CHANGE_LOW=value;}
            void SetSound_gear_change_medium( float value)        {SOUND_GEAR_CHANGE_MEDIUM=value;}        
            void SetSound_gear_change_high( float value)          {SOUND_GEAR_CHANGE_HIGH=value;}          
            void SetSound_effect_ignition(const std::string& value){SOUND_EFFECT_IGNITION=value;}     
            void SetSound_effect_vehicle_loop(const std::string& value){SOUND_EFFECT_VEHICLE_LOOP=value;} 
            void SetSound_effect_brake(const std::string& value)  {SOUND_EFFECT_BRAKE=value;}        
            void SetSound_effect_acceleration(const std::string& value){SOUND_EFFECT_ACCELERATION=value;} 
            void SetSound_effect_collision_hit(const std::string& value){SOUND_EFFECT_COLLISION_HIT=value;}
            void SetVehicleInsideModel(const std::string &value)  {VEHICLE_INSIDE_MODEL = value;}
            virtual void SetHasDriver( bool hasDriver )           { mHasDriver = hasDriver; }
               
            float  GetSound_brake_squeal_amount()   {return SOUND_BRAKE_SQUEAL_AMOUNT;}    
            float  GetSound_gear_change_low()       {return SOUND_GEAR_CHANGE_LOW;}
            float  GetSound_gear_change_medium()    {return SOUND_GEAR_CHANGE_MEDIUM;}        
            float  GetSound_gear_change_high()      {return SOUND_GEAR_CHANGE_HIGH;}          
            const std::string& GetSound_effect_ignition() {return SOUND_EFFECT_IGNITION;}     
            const std::string& GetSound_effect_vehicle_loop(){return SOUND_EFFECT_VEHICLE_LOOP;} 
            const std::string& GetSound_effect_brake()    {return SOUND_EFFECT_BRAKE;}        
            const std::string& GetSound_effect_acceleration(){return SOUND_EFFECT_ACCELERATION;} 
            const std::string& GetSound_effect_collision_hit(){return SOUND_EFFECT_COLLISION_HIT;}
            virtual bool GetHasDriver() const       { return mHasDriver; }
           
         // Private vars
         private:
            
            ///////////////////////////////////////////////////
            // for sound
            enum GearSoundLevel {FIRST_GEAR = 1, SECOND_GEAR, THIRD_GEAR, FOURTH_GEAR};
            GearSoundLevel    mLastGearChange;     /// So we know when to play a sound.
            ///////////////////////////////////////////////////

            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper> mPhysicsHelper;

            ///////////////////////////////////////////////////
            // Sound effects
            dtCore::RefPtr<dtAudio::Sound> mSndIgnition;
            dtCore::RefPtr<dtAudio::Sound> mSndVehicleIdleLoop;
            dtCore::RefPtr<dtAudio::Sound> mSndBrake;
            dtCore::RefPtr<dtAudio::Sound> mSndAcceleration;
            dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
            ///////////////////////////////////////////////////

            ///////////////////////////////////////////////////
            // properties
            std::string VEHICLE_INSIDE_MODEL;      /// for interior views
            float SOUND_BRAKE_SQUEAL_AMOUNT;       /// How many mph does the car have to go to squeal used with BRAKE_STOP_NOW_BRAKE_TIME
            float SOUND_GEAR_CHANGE_LOW;           /// At what speed play the acceleration sound effect / reset idle pitch // TODO IMPLEMENT
            float SOUND_GEAR_CHANGE_MEDIUM;        /// At what speed play the acceleration sound effect / reset idle pitch // TODO IMPLEMENT
            float SOUND_GEAR_CHANGE_HIGH;          /// At what speed play the acceleration sound effect / reset idle pitch // TODO IMPLEMENT
            std::string SOUND_EFFECT_IGNITION;     /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_VEHICLE_LOOP; /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_BRAKE;        /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_ACCELERATION; /// What is the filepath / string of the sound effect
            std::string SOUND_EFFECT_COLLISION_HIT;/// What is the filepath / string of the sound effect
            ///////////////////////////////////////////////////

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

            /// Check if the supplied NxActor is below ground, if so, move it above ground
            void KeepAboveGround( NxActor* physicsObject );

      };

      class SIMCORE_EXPORT NxAgeiaFourWheelVehicleActorProxy : public PlatformActorProxy
      {
         public:
            NxAgeiaFourWheelVehicleActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~NxAgeiaFourWheelVehicleActorProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
      };

   }
}

#endif
#endif
