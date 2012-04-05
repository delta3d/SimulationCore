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
 * @author Eddie Johnson
 */
#ifndef _DETONATION_ACTOR_H_
#define _DETONATION_ACTOR_H_

#include <dtCore/particlesystem.h>

#include <dtAudio/sound.h>

#include <SimCore/Actors/IGActor.h>
//#include <SimCore/Actors/ViewerMaterialActor.h>
#include <dtUtil/getsetmacros.h>
#include <dtDAL/resourcedescriptor.h>

namespace dtGame
{
   class TimerElapsedMessage;
}

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
            ///currently these are the supported impact types
            ///each one has specific particle systems, sounds, and lights
            enum IMPACT_TYPE{IMPACT_TERRAIN, IMPACT_ENTITY, IMPACT_HUMAN};

            /// Constructor
            DetonationActor(dtGame::GameActorProxy& proxy);

            /**
             *  The impact type determines which particle system and sounds will be played.
             */
            void SetImpactType(IMPACT_TYPE impact);
            IMPACT_TYPE GetImpactType() const;

            virtual void OnRemovedFromWorld();

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
         
            //Ground Impact Properties
            //note: if no other impact properties are set it defaults to using the ground impact properties
            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, GroundImpactEffect);
            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, GroundImpactSound);
            void SetGroundImpactLight( const std::string& lightName );
            const std::string& GetGroundImpactLight() const;

            //Entity Impact Properties
            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, EntityImpactEffect);
            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, EntityImpactSound);
            void SetEntityImpactLight( const std::string& lightName );
            const std::string& GetEntityImpactLight() const;

            //Human Impact Properties
            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, HumanImpactEffect);
            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, HumanImpactSound);
            void SetHumanImpactLight( const std::string& lightName );
            const std::string& GetHumanImpactLight() const;

            //smoke effect
            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, SmokeEffect);
            void LoadSmokeFile(const dtDAL::ResourceDescriptor& resource);

            DT_DECLARE_ACCESSOR_INLINE(float, SmokeLifeTime);

            /**
             * Gets the sound member of this detonation actor
             * @note If no sound file is loaded, the sound member will be NULL
             * @return mSound
             */
            dtAudio::Sound* GetSound() { return mSound.get(); }

         protected:
            /// Destructor
            virtual ~DetonationActor();

            /// Plays the sound associated with this Detonation Actor
            virtual void PlaySound();

            /// Enables the explosion particle system of this Detonation Actor
            virtual void RenderDetonation();

            /// Enables the smoke particle system of this Detonation Actor
            virtual void RenderSmoke();

            /// Disables the rendering of smoke
            virtual void StopRenderingSmoke();


            /// Enables the smoke particle system of this Detonation Actor
            void StartSmokeEffect(dtCore::ParticleSystem& particles);
            void LoadSoundFile(const dtDAL::ResourceDescriptor& resource, dtCore::RefPtr<dtAudio::Sound>& soundIn);
            void LoadParticleSystem(const dtDAL::ResourceDescriptor& resource, dtCore::RefPtr<dtCore::ParticleSystem>& particleSysIn);

            ///adds a light for the explosion effect
            void AddDynamicLight(const std::string& lightName);


         private:

            friend class DetonationActorProxy;
           
            void SetImpactEffects();

            IMPACT_TYPE mCurrentImpact;

            float mDelayTime;
            float mRenderExplosionTimerSecs;
            float mDeleteActorTimerSecs;

            bool mUsesPhysics;
            
            std::string mCurrentLightName;
            std::string mLightImpactGround;
            std::string mLightImpactEntity;
            std::string mLightImpactHuman;

            dtCore::ObserverPtr<ViewerMaterialActor> mCollidedMaterial; 
            dtCore::RefPtr<dtCore::ParticleSystem> mExplosionSystem;
            dtCore::RefPtr<dtCore::ParticleSystem> mSmokeSystem;
            dtCore::RefPtr<dtAudio::Sound> mSound;
      };

      class SIMCORE_EXPORT DetonationActorProxy : public dtGame::GameActorProxy
      {
         public:
            static const std::string CLASS_NAME;

            /// Constructor
            DetonationActorProxy();

            /// Creates the actor
            void CreateActor() { SetActor(*new DetonationActor(*this)); }

            /// Builds the properties this actor has
            void BuildPropertyMap();

            /// Builds the invokables of this actor
            void BuildInvokables();

            /// Invokable that tells the game manager to delete ourselves and play our sound
            void HandleDetonationActorTimers(const dtGame::TimerElapsedMessage& timeMsg);

            /// Clear all timers from the GameManager
            void ClearTimers();

            virtual void OnRemovedFromWorld()
            {
               DetonationActor* actor = NULL;
               GetActor(actor);
               if (actor != NULL)
               {
                  actor->OnRemovedFromWorld();
               }
            }

         protected:

            /// Destructor
            virtual ~DetonationActorProxy();
            virtual void OnEnteredWorld();
      };
   }
}
#endif
