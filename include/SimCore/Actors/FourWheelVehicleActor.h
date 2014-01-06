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

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();
            virtual void PostPhysicsUpdate();
         public:
            /// Utility Methods
            float GetBrakeTorque();

            /// Reset to starting position In additional to base behavior, it turns off sounds.
            virtual void ResetVehicle();

            SimCore::FourWheelVehiclePhysicsActComp* GetFourWheelPhysicsActComp() const;

            /// Steering angle normalized from -1 to 1
            DT_DECLARE_ACCESSOR(float, CurrentSteeringAngleNormalized);

            DT_DECLARE_ACCESSOR(float, SoundBrakeSquealSpeed);
            DT_DECLARE_ACCESSOR(float, GearChangeLow);
            DT_DECLARE_ACCESSOR(float, GearChangeMedium);
            DT_DECLARE_ACCESSOR(float, GearChangeHigh);
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, SoundEffectIgnition);
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, SoundEffectIdleLoop);
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, SoundEffectBrake);
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, SoundEffectAcceleration);
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, SoundEffectCollisionHit);
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, VehicleInteriorModel);

            /// Turns it up and moves up
            virtual void RepositionVehicle(float deltaTime);

            virtual float GetMPH() const;

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
            virtual bool LoadSound(const dtDAL::ResourceDescriptor& rd, std::shared_ptr<dtAudio::Sound>& soundOut);

         private:

            ///////////////////////////////////////////////////
            // for sound
            enum GearSoundLevel {FIRST_GEAR = 1, SECOND_GEAR, THIRD_GEAR, FOURTH_GEAR};
            GearSoundLevel    mLastGearChange;     /// So we know when to play a sound.
            ///////////////////////////////////////////////////

            ///////////////////////////////////////////////////
            // Sound effects
            std::shared_ptr<dtAudio::Sound> mSndIgnition;
            std::shared_ptr<dtAudio::Sound> mSndVehicleIdleLoop;
            std::shared_ptr<dtAudio::Sound> mSndBrake;
            std::shared_ptr<dtAudio::Sound> mSndAcceleration;
            std::shared_ptr<dtAudio::Sound> mSndCollisionHit;
            ///////////////////////////////////////////////////

            ///////////////////////////////////////////////////
            // vehicles portal for the actor
            std::shared_ptr<dtGame::GameActorProxy> mVehiclesPortal;
            float mCruiseSpeed;
            bool mStopMode, mCruiseMode;

      };

      class SIMCORE_EXPORT FourWheelVehicleActorProxy : public BasePhysicsVehicleActorProxy
      {
         public:
            FourWheelVehicleActorProxy();
            virtual void BuildPropertyMap();
            virtual void BuildActorComponents();

         protected:
            virtual ~FourWheelVehicleActorProxy();
            void CreateDrawable();
            virtual void OnEnteredWorld();
      };

   }
}

#endif
#endif
