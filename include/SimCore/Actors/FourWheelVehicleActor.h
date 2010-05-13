/* -*-c++-*-
* Simulation Core
* Copyright 2007-2010, Alion Science and Technology
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
* Allen Danklefsen
* David Guthrie
*/
#ifndef AGEIA_PHYSICS
#ifndef FOUR_WHEEL_VEHICLE_
#define FOUR_WHEEL_VEHICLE_

#include <SimCore/Export.h>

#include <SimCore/FourWheelVehiclePhysicsHelper.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <dtUtil/getsetmacros.h>
#include <dtDAL/resourcedescriptor.h>

// must include because of the refptrs
#include <dtAudio/sound.h>

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////
      /* This class extends BasePhysicsVehicle and has behavior specific to just the 4 wheeled
       * variety of vehicle. It provides for sounds, brakes, etc...
       */
      class SIMCORE_EXPORT FourWheelVehicleActor : public BasePhysicsVehicleActor
      {
         public:
            typedef BasePhysicsVehicleActor BaseClass;
            /// Constructor
            FourWheelVehicleActor (BasePhysicsVehicleActorProxy& proxy);

         protected:
            /// Destructor
            virtual ~FourWheelVehicleActor();

         // INHERITED PUBLIC
         public:
            /// Override this to add your own components or to init values on the ones that are already added.
            virtual void BuildActorComponents();

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();
            virtual void PostPhysicsUpdate();
         public:
            /// Utility Methods
            float GetBrakeTorque();

            /// Reset to starting position In additional to base behavior, it turns off sounds.
            virtual void ResetVehicle();

            SimCore::FourWheelVehiclePhysicsHelper* GetFourWheelPhysicsHelper();

            /// The number of updates to go from straight to full turn angle
            DECLARE_PROPERTY(int, NumUpdatesUntilFullSteeringAngle);
            /// Steering angle normalized from -1 to 1
            DECLARE_PROPERTY(float, CurrentSteeringAngleNormalized);

            DECLARE_PROPERTY(float, SoundBrakeSquealSpeed);
            DECLARE_PROPERTY(float, GearChangeLow);
            DECLARE_PROPERTY(float, GearChangeMedium);
            DECLARE_PROPERTY(float, GearChangeHigh);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, SoundEffectIgnition);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, SoundEffectIdleLoop);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, SoundEffectBrake);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, SoundEffectAcceleration);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, SoundEffectCollisionHit);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, VehicleInteriorModel);

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

            /**
             * Loads a sound from a resource descriptor and adds it as child
             * @param rd the resource to load
             * @param sound  Output, the sound ref pointer to assign.
             * @return true if the sound was loaded successfully.
             */
            virtual bool LoadSound(const dtDAL::ResourceDescriptor& rd, dtCore::RefPtr<dtAudio::Sound>& soundOut);

         private:

            ///////////////////////////////////////////////////
            // for sound
            enum GearSoundLevel {FIRST_GEAR = 1, SECOND_GEAR, THIRD_GEAR, FOURTH_GEAR};
            GearSoundLevel    mLastGearChange;     /// So we know when to play a sound.
            ///////////////////////////////////////////////////

            ///////////////////////////////////////////////////
            // Sound effects
            dtCore::RefPtr<dtAudio::Sound> mSndIgnition;
            dtCore::RefPtr<dtAudio::Sound> mSndVehicleIdleLoop;
            dtCore::RefPtr<dtAudio::Sound> mSndBrake;
            dtCore::RefPtr<dtAudio::Sound> mSndAcceleration;
            dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
            ///////////////////////////////////////////////////

            ///////////////////////////////////////////////////
            // vehicles portal for the actor
            dtCore::RefPtr<dtGame::GameActorProxy> mVehiclesPortal;

      };

      class SIMCORE_EXPORT FourWheelVehicleActorProxy : public BasePhysicsVehicleActorProxy
      {
         public:
            FourWheelVehicleActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~FourWheelVehicleActorProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
      };

   }
}

#endif
#endif
