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
//#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

namespace dtAudio
{
   class Sound;
}

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////
      /* This class extends BasePhysicsVehicle and has behavior specific to just the 4 wheeled
       * variety of vehicle. It provides for sounds, brakes, etc...
       */
      class SIMCORE_EXPORT NxAgeiaFourWheelVehicleActor : public BasePhysicsVehicleActor
      {
         public:
            /// Constructor
            NxAgeiaFourWheelVehicleActor (BasePhysicsVehicleActorProxy &proxy);

         protected:
            /// Destructor
            virtual ~NxAgeiaFourWheelVehicleActor();

         // INHERITED PUBLIC
         public:

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            //virtual void AgeiaPrePhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            //virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            //virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport,
            //   dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit);

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            //virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const dtPhysics::PhysicsObject& ourSelf,
            //   const dtPhysics::PhysicsObject& whatWeHit){}

            /**
            * Handle forces received from the environment, such as detonations and impacts
            */
            //virtual void ApplyForce( const osg::Vec3& force, const osg::Vec3& location );

         // PUBLIC CLASSES
         public:
            /// Utility Methods
            //virtual float GetMPH();
            float GetBrakeTorque();

            /// Reset to starting position In additional to base behavior, it turns off sounds.
            virtual void ResetVehicle();

            /// Turns it up and moves up
            //void RepositionVehicle(float deltaTime);

            //virtual bool ShouldForceUpdate( const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            enum WHEEL_NAMES {FRONT_LEFT = 0, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT};

            dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper* GetFourWheelPhysicsHelper() {
               return static_cast<dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper*> (GetPhysicsHelper());}

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

            float  GetSound_brake_squeal_amount()   {return SOUND_BRAKE_SQUEAL_AMOUNT;}
            float  GetSound_gear_change_low()       {return SOUND_GEAR_CHANGE_LOW;}
            float  GetSound_gear_change_medium()    {return SOUND_GEAR_CHANGE_MEDIUM;}
            float  GetSound_gear_change_high()      {return SOUND_GEAR_CHANGE_HIGH;}
            const std::string& GetSound_effect_ignition() {return SOUND_EFFECT_IGNITION;}
            const std::string& GetSound_effect_vehicle_loop(){return SOUND_EFFECT_VEHICLE_LOOP;}
            const std::string& GetSound_effect_brake()    {return SOUND_EFFECT_BRAKE;}
            const std::string& GetSound_effect_acceleration(){return SOUND_EFFECT_ACCELERATION;}
            const std::string& GetSound_effect_collision_hit(){return SOUND_EFFECT_COLLISION_HIT;}


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

            ///////////////////////////////////////////////////
            // for sound
            enum GearSoundLevel {FIRST_GEAR = 1, SECOND_GEAR, THIRD_GEAR, FOURTH_GEAR};
            GearSoundLevel    mLastGearChange;     /// So we know when to play a sound.
            ///////////////////////////////////////////////////

            //std::shared_ptr<dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper> mFourWheelPhysicsHelper;

            ///////////////////////////////////////////////////
            // Sound effects
            std::shared_ptr<dtAudio::Sound> mSndIgnition;
            std::shared_ptr<dtAudio::Sound> mSndVehicleIdleLoop;
            std::shared_ptr<dtAudio::Sound> mSndBrake;
            std::shared_ptr<dtAudio::Sound> mSndAcceleration;
            std::shared_ptr<dtAudio::Sound> mSndCollisionHit;
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
            std::shared_ptr<dtGame::GameActorProxy> mVehiclesPortal;

      };

      class SIMCORE_EXPORT NxAgeiaFourWheelVehicleActorProxy : public BasePhysicsVehicleActorProxy
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
