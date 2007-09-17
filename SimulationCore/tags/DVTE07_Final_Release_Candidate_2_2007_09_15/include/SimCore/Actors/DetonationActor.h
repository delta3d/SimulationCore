/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#ifndef _DETONATION_ACTOR_H_
#define _DETONATION_ACTOR_H_

#include <dtCore/particlesystem.h>

#include <dtAudio/sound.h>

#include <SimCore/Actors/IGActor.h>
//#include <SimCore/Actors/ViewerMaterialActor.h>

namespace SimCore
{
   namespace Actors
   {
      class ViewerMaterialActor;

      /**
       * Type of Detonations - used for different particle effects and sounds.
       */
      class SIMCORE_EXPORT DetonationMunitionType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(DetonationMunitionType);
         public:
            static DetonationMunitionType UNKNOWN;
            static DetonationMunitionType LARGE_EXPLOSION;
            static DetonationMunitionType MEDIUM_EXPLOSION;
            static DetonationMunitionType SMALL_EXPLOSION;
            static DetonationMunitionType LARGE_BULLET;
            static DetonationMunitionType SMALL_BULLET;
            static DetonationMunitionType SHORT_SMOKE;
            static DetonationMunitionType LONG_SMOKE;
            static DetonationMunitionType DPICM;
            static DetonationMunitionType ILLUMINATION;
         private:
            DetonationMunitionType(const std::string &name) : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
      };


      class SIMCORE_EXPORT DetonationActor : public IGActor
      {
         public:

            /// Constructor
            DetonationActor(dtGame::GameActorProxy &proxy);

            /**
             * Loads in a specified sound file
             * @param fileName The name of the sound file
             */
            void LoadSoundFile(const std::string &fileName);
            
            /**
             * Loads in a particle system file for the detonation
             * @param fileName The name of the file to load
             */
            void LoadDetonationFile(const std::string &fileName);

            /**
             * Loads a smoke file for this detonation actor to use
             * @param fileName The name of the file to use
             */
            void LoadSmokeFile(const std::string &fileName);

            // Invoked when a actor is added to the Game Manager
            virtual void OnEnteredWorld(); 

            /**
             * Gets the delay time to be used for flash bang
             * @return mDelayTime
             */
            float GetDelayTime() const { return mDelayTime; }

            /**
             * Calculates the delay time from a player's position
             * @param position The current position of the player
             * @return The delay in seconds
             * @note This function sets the mDelayTime member as a post condition
             */
            void CalculateDelayTime(const osg::Vec3 &position);

            /**
             * Sets the amount of shot lingered seconds on the detonation
             * @param secs The number of secs to linger
             */
            //static void SetLingeringShotSecs(const float secs) { mLingeringShotSecs = secs; }

            /**
             * Gets the length that the smoke particle system stays alive.  Zero means do NOT 
             * use smoke particle. Default is zero.
             * @return mLingeringSmokeSecs
             */
            float GetLingeringSmokeSecs() const { return mLingeringSmokeSecs; }

            /**
             * Sets the length that the smoke particle system stays alive.  Zero means do NOT 
             * use smoke particle. Default is zero.
             * @param lingeringSmokeSecs the length, in seconds that the smoke stays up.
             */
            void SetLingeringSmokeSecs(float lingeringSmokeSecs) { mLingeringSmokeSecs = lingeringSmokeSecs; }

            /**
             * Sets the explosion render seconds 
             * @param secs The timer seconds
             */
            void SetExplosionTimerSecs(float secs) { mRenderExplosionTimerSecs = secs; }

            /**
             * Sets the delete actor seconds 
             * @param secs The timer seconds
             */
            void SetDeleteActorTimerSecs(float secs) { mDeleteActorTimerSecs = secs; }
      
            /**
             * Sets the explosion render seconds 
             * @param secs The timer seconds
             */
            float GetExplosionTimerSecs() const { return mRenderExplosionTimerSecs; }

            /**
             * Sets the delete actor seconds 
             * @param secs The timer seconds
             */
            float GetDeleteActorTimerSecs() const { return mDeleteActorTimerSecs; }

            /**
             * Sets the maximum distance on the sound
             * @param distance The new distance
             */
            void SetMaximumSoundDistance(float distance) { if(mSound.valid()) mSound->SetMaxDistance(distance); }

            /**
             * Sets the minimum distance on the sound
             * @param distance The new distance
             */
            void SetMinimumSoundDistance(float distance) { if(mSound.valid()) mSound->SetMinDistance(distance); }

            /**
             * Sets the roll off factor of the sound
             * @param the distance to which is starts dropping off
             */
            void SetRollOffFactor(float distance) {if(mSound.valid()) mSound->SetRolloffFactor(distance);}

            /**
             * Gets the maximum distance on the sound
             * @param distance The new distance
             */
            float GetMaximumSoundDistance() const { return mSound.valid() ? mSound->GetMaxDistance() : 0; }

            /**
             * Gets the minimum distance on the sound
             * @param distance The new distance
             */
            float GetMinimumSoundDistance() const { return mSound.valid() ? mSound->GetMinDistance() : 0; }
            
            /**
             * Get the explosion particle system. This allows the proxy
             * to register the particle system in HandleDetonationActorTimers.
             */
            dtCore::ParticleSystem* GetExplosionParticleSystem();
            const dtCore::ParticleSystem* GetExplosionParticleSystem() const;

            /**
             * Get the smoke particle system. This allows the proxy
             * to register the particle system in HandleDetonationActorTimers.
             */
            dtCore::ParticleSystem* GetSmokeParticleSystem();
            const dtCore::ParticleSystem* GetSmokeParticleSystem() const;

            /**
             * Set whether this detonation actor will apply physics to its particles
             */
            void SetPhysicsEnabled( bool usesPhysics ) { mUsesPhysics = usesPhysics; }
            bool IsPhysicsEnabled() const { return mUsesPhysics; }

            void SetMaterialCollidedWith(ViewerMaterialActor& material)
            {
               mCollidedMaterial = &material;
            }

         protected:
            /// Destructor
            virtual ~DetonationActor();

         private:

            /**
             * Gets the sound member of this detonation actor
             * @note If no sound file is loaded, the sound member will be NULL
             * @return mSound
             */
            dtAudio::Sound* GetSound() { return mSound.get(); }

            friend class DetonationActorProxy;
            /// Plays the sound associated with this Detonation Actor
            void PlaySound();

            /// Enables the explosion particle system of this Detonation Actor
            void RenderDetonation();

            /// Enables the smoke particle system of this Detonation Actor
            void RenderSmoke();

            /// Disables the rendering of smoke
            void StopRenderingSmoke();

            ///adds a light for the explosion effect
            void AddDynamicLight();

            dtCore::RefPtr<dtCore::ParticleSystem> mExplosionSystem, mSmokeSystem;
            dtCore::RefPtr<dtAudio::Sound> mSound;
            float mDelayTime;
            //static float mLingeringShotSecs;
            float mLingeringSmokeSecs;
            float mRenderExplosionTimerSecs;
            float mDeleteActorTimerSecs;

            bool mUsesPhysics;

            dtCore::ObserverPtr<ViewerMaterialActor> mCollidedMaterial;
      };

      class SIMCORE_EXPORT DetonationActorProxy : public dtGame::GameActorProxy
      {
         public:

            /// Constructor
            DetonationActorProxy();

            /// Creates the actor
            void CreateActor() { SetActor(*new DetonationActor(*this)); }

            /// Builds the properties this actor has
            void BuildPropertyMap();

            /// Builds the invokables of this actor
            void BuildInvokables();

            /// Invokable that tells the game manager to delete ourselves and play our sound
            void HandleDetonationActorTimers(const dtGame::Message &msg);

            /// Set common property values with one function call
            void SetDetonationProperties(
               float lingerTime,
               float minSoundDistance,
               float maxSoundDistance,
               const std::string& detonationFile,
               const std::string& soundFile,
               const std::string& smokeFile = ""
               );

            /// Clear all timers from the GameManager
            void ClearTimers();
         
         protected:

            /// Destructor
            virtual ~DetonationActorProxy();
            virtual void OnEnteredWorld();
      };
   }
}
#endif
