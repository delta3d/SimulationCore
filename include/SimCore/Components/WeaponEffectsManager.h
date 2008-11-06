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
 * @author Chris Rodgers
 */

#ifndef WEAPON_EFFECTS_MANAGER_H
#define WEAPON_EFFECTS_MANAGER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/Referenced>
#include <osgSim/DOFTransform>
#include <dtCore/batchisector.h>
#include <dtCore/observerptr.h>
#include <dtUtil/refstring.h>
#include <SimCore/Actors/VolumetricLine.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtAudio
{
   class Sound;
}

namespace dtCore
{
   class ParticleSystem;
}

namespace dtGame
{
   class GameManager;
}

namespace SimCore
{
   namespace Actors
   {
      class BaseEntity;
      class MunitionEffectsInfoActor;
   }

   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT TracerEffect : public SimCore::Actors::VolumetricLine
      {
         public:
            static const dtUtil::RefString DEFAULT_TRACER_SHADER;
            static const dtUtil::RefString DEFAULT_TRACER_SHADER_GROUP;

            TracerEffect( float lineLength, float lineThickness,
               const std::string& shaderName = DEFAULT_TRACER_SHADER.Get(),
               const std::string& shaderGroup = DEFAULT_TRACER_SHADER_GROUP.Get() );

            void SetHasLight( bool hasLight ) { mHasLight = hasLight; }
            bool HasLight() const { return mHasLight; }

            void SetMaxLifeTime( float maxLifeTime ) { mMaxLifeTime = maxLifeTime; }
            float GetMaxLifeTime() const { return mMaxLifeTime; }

            void SetSpeed( float speed ) { mSpeed = speed; }
            float GetSpeed() const { return mSpeed; }

            void SetDirection( const osg::Vec3& direction );
            const osg::Vec3& GetDirection() const { return mDirection; }

            void SetVelocity( const osg::Vec3& velocity );
            osg::Vec3 GetVelocity() const;
            void GetVelocity( osg::Vec3& outVelocity ) const;

            void SetPosition( const osg::Vec3& position );
            const osg::Vec3& GetPosition() const { return mPosition; }

            void SetVisible( bool visible );
            bool IsVisible() const;

            void SetMaxLength( float maxLength );
            float GetMaxLength() const;

            bool IsActive() const;

            void Execute( float maxLifeTime );

            void Update( float deltaTime );

            void AddDynamicLight( const osg::Vec3& color );
            void RemoveDynamicLight();

            void SetGameManager( dtGame::GameManager* gameManager );

         protected:
            virtual ~TracerEffect();

         private:
            bool      mHasLight;
            bool      mDynamicLightEnabled;
            unsigned  mDynamicLightID;
            float     mLifeTime;
            float     mMaxLifeTime;
            float     mSpeed; // aka Velocity Magnitude
            float     mMaxLength;
            osg::Vec3 mLastPosition;
            osg::Vec3 mPosition;
            osg::Vec3 mDirection;
            dtCore::RefPtr<dtGame::GameManager> mGM; // for accessing the rendering support component (safer using GM)
      };



      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Request Code
      //////////////////////////////////////////////////////////////////////////
      class TracerEffectRequest : public osg::Referenced
      {
         public:
            TracerEffectRequest( unsigned totalEffects, float cycleTime,
               const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo )
               : mIsFirstEffect(true),
               mTotalEffects(totalEffects),
               mCycleTime(cycleTime),
               mCurrentTime(0.0),
               mEffectsInfo(&effectsInfo)
            {
            }

            void SetFirePoint( const osg::Vec3& firePoint ) { mFirePoint = firePoint; }
            const osg::Vec3& GetFirePoint() const { return mFirePoint; }

            void SetVelocity( const osg::Vec3& velocity ) { mVelocity = velocity; }
            const osg::Vec3& GetVelocity() const { return mVelocity; }

            const SimCore::Actors::MunitionEffectsInfoActor& GetEffectsInfo() const { return *mEffectsInfo; }

            float GetCycleTime() const { return mCycleTime; }

            unsigned GetTotalEffects() const { return mTotalEffects; }

            bool IsFirstEffect() const { return mIsFirstEffect; }

            bool IsEffectReady() const
            {
               return mTotalEffects > 0 && mCurrentTime > mCycleTime;
            }

            void Update( float timeDelta )
            {
               mCurrentTime += timeDelta;
            }

            unsigned Decrement()
            {
               if( mTotalEffects > 0 )
               {
                  --mTotalEffects;
               }
               mCurrentTime = 0.0f;
               mIsFirstEffect = false;
               return mTotalEffects;
            }
         
         protected:
            virtual ~TracerEffectRequest() {}

         private:
            bool     mIsFirstEffect;
            unsigned mTotalEffects;
            float    mCycleTime;
            float    mCurrentTime;
            osg::Vec3 mFirePoint;
            osg::Vec3 mVelocity;
            dtCore::RefPtr<const SimCore::Actors::MunitionEffectsInfoActor> mEffectsInfo;
      };



      //////////////////////////////////////////////////////////////////////////
      // Weapon Effect Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT WeaponEffect : public dtCore::Transformable
      {
         public:
            static const dtUtil::RefString CLASS_NAME;

            WeaponEffect();

            // Set the probability that this object will execute a visible flash effect.
            // @param probability The probability (0.0 to 1.0) that a flash will
            //        be produced by this object. This value will be clamped automatically.
            void SetFlashProbability( float probability )
            { 
               mFlashProbability = probability < 0.0f ? 0.0f : probability > 1.0f ? 1.0f : probability; 
            }
            float GetFlashProbability() const { return mFlashProbability; }

            // Setting the flash time will automatically make the flash visible.
            // @param flashTime The period of time the flash effect shall stay visible.
            void SetFlashTime( float flashTime );
            float GetFlashTime() const { return mFlashTime; }

            void SetFlash( dtCore::ParticleSystem* flash );
            dtCore::ParticleSystem* GetFlash() { return mFlash.get(); }
            const dtCore::ParticleSystem* GetFlash() const { return mFlash.get(); }

            // Assign the sound object.
            // @param sound The sound object
            // @param releaseOldSound Determines if this object should free
            //        the sound memory before assigning the new sound object.
            void SetSound( dtAudio::Sound* sound, bool releaseOldSound = true );
            dtAudio::Sound* GetSound() { return mSound.get(); }
            const dtAudio::Sound* GetSound() const { return mSound.get(); }

            // Set the entity that will own this effect.
            // This MUST be set in order for a call to Attach to be successful.
            // @param owner The entity (usually remote) to which this effects 
            //        object will attach.
            void SetOwner( SimCore::Actors::BaseEntity* owner ) { mOwner = owner; }
            SimCore::Actors::BaseEntity* GetOwner() { return mOwner.get(); }
            const SimCore::Actors::BaseEntity* GetOwner() const { return mOwner.get(); }

            // Set the visibility of the flash effect.
            // @param visible The visibility state of the flash effect.
            void SetVisible( bool visible );
            bool IsVisible() const { return mVisible; }

            // Access the referenced DOF that was set via Attach
            osgSim::DOFTransform* GetDOF() { return mDOF.get(); }
            const osgSim::DOFTransform* GetDOF() const { return mDOF.get(); }

            // Get the time since the flash effect was executed.
            // This time is used by the WeaponEffectsManager to determine
            // if this effect object is old enough for recycling.
            // @return The time in seconds since the last flash execution.
            float GetTimeSinceFlash() const { return mTimeSinceFlash; }

            // Determine if the sound part of the effect has been played.
            // @return TRUE if the sound had been executed.
            bool IsSoundPlayed() const { return mSoundPlayed; }

            // Get the time in seconds that sound will be played after Execute is called.
            // @return seconds from the instant Execute is called to the point the
            // associated sound effect will played.
            //
            // NOTE: This function is mostly intended for testing purposes.
            //
            //       The delay is calculated after Execute is called in which the
            //       listener's world position is specified.
            //
            float GetSoundDelay() const { return mSoundStartTime; }

            // Determine the time delay of sound between to points.
            // @param sourcePosition World position at which the sound is emitted.
            // @param listenerPosition World position of the listener to hear the sound.
            // @return distance in meters between the two specified points.
            static float CalculateSoundDelayTime( const osg::Vec3& sourcePosition, const osg::Vec3& listenerPosition );

            // Show the weapon flash and play the weapon sound.
            // @param flashTime The time in seconds that the flash should remain visible.
            // @param playerPosition World position of the listener. This is used
            //        attenuate the sound of any sound effects created by this effect.
            void Execute( float flashTime, const osg::Vec3& listenerPosition );

            // Update the flash timer and make the flash invisible if flash timer
            // exceeds the flash life time.
            // @param timeDelta The simulation time slice in seconds since the
            //        last call to Update.
            void Update( float timeDelta );

            // Attach this effects object to the owner that was set.
            // If a DOF is specified, this object will attempt to attach directly
            // to the DOF rather than the owner.
            // @param dof The DOF of the owner's geometry to which this effects
            //        object should attach. If the dof is NULL, direct attachment
            //        to the owner will be attempted.
            // @return TRUE if the attachment process was successful.
            bool Attach( osgSim::DOFTransform* dof );

            // Detach this object from the owner and its associated DOF.
            // @return TRUE if the detachment process was successful.
            bool Detach();

            // Clear this object's references to other objects.
            // Detach will be called automatically.
            // @param releaseOldSound Determines if this object has permission
            //        to free the associated sound object's memory.
            void Clear( bool releaseOldSound = true );

            // Load and assign the sound resource.
            // @param filePath The path that points to the file to be loaded.
            // @return TRUE if load and assignment was successful.
            bool LoadSound( const std::string& filePath );

            // Load and assign the flash (particle effect) resource.
            // @param filePath The path that points to the file to be loaded.
            // @return TRUE if load and assignment was successful.
            bool LoadFlash( const std::string& filePath );

            void AddDynamicLight( const osg::Vec3& color );
            void RemoveDynamicLight();

            void SetGameManager( dtGame::GameManager* gameManager );

         protected:
            virtual ~WeaponEffect();

         private:
            unsigned  mDynamicLightID;
            bool mDynamicLightEnabled;
            bool mVisible;
            bool mSoundPlayed;
            float mSoundStartTime;
            float mFlashProbability;
            float mFlashTime;
            float mTimeSinceFlash;
            dtCore::RefPtr<dtAudio::Sound> mSound;
            dtCore::RefPtr<dtCore::ParticleSystem> mFlash;
            dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mOwner;
            osg::observer_ptr<osgSim::DOFTransform> mDOF;
            dtCore::RefPtr<dtGame::GameManager> mGM; // for accessing the rendering support component (safer using GM)
      };



      //////////////////////////////////////////////////////////////////////////
      // Weapon Effect Manager Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT WeaponEffectsManager : public osg::Referenced
      {
         public:
            WeaponEffectsManager();

            // Set the game manager that has a scene to which tracer effects may be added.
            // @param gameManager The main scene that will render tracer effects.
            void SetGameManager( dtGame::GameManager* gameManager );
            dtGame::GameManager* GetGameManager() { return mGM.get(); }
            const dtGame::GameManager* GetGameManager() const { return mGM.get(); }

            // Return the isector responsible for collision detection of tracer effects.
            // NOTE: This is intended for testing purposes only.
            const dtCore::BatchIsector* GetIsector() const { return mIsector.get(); }

            // Set the maximum length of time that any WeaponEffect object should
            // live before being recycled.
            // @param effectTimeMax The max time in seconds that an effect is allowed to persist.
            void SetEffectTimeMax( float effectTimeMax ) { mEffectTimeMax = effectTimeMax; }
            float GetEffectTimeMax() const { return mEffectTimeMax; }

            // Set the limit of effects allowed to exist at any one time.
            // @param maxEffectsAllowed The limit on effects allowed to be created.
            //        Negative values mean NO limit.
            void SetMaxWeaponEffects( int maxEffectsAllowed ) { mMaxWeaponEffects = maxEffectsAllowed; }
            int GetMaxWeaponEffects() const { return mMaxWeaponEffects; }

            // Set the limit of tracer effects allowed to exist at any one time.
            // @param maxEffectsAllowed The limit on effects allowed to be created.
            //        Negative values mean NO limit.
            void SetMaxTracerEffects( int maxEffectsAllowed ) { mMaxTracerEffects = maxEffectsAllowed; }
            int GetMaxTracerEffects() const { return mMaxTracerEffects; }

            // Set the cycle length between each call to Recycle.
            // @param recycleTime The time in seconds between each automatic
            //        call to Recycle.
            void SetRecycleTime( float recycleTime ) { mRecycleTime = recycleTime; }
            float GetRecycleTime() const { return mRecycleTime; }

            // Update or create a WeaponEffect for the the specified owner entity.
            // @param owner The entity that will have the effects attached
            // @param ownerDOF The DOF of the entity's geometry to which the effects
            //        object should be attached directly.
            // @param effectsInfo The effects info that contains file paths to
            //        the sound and flash effects needed by a new effects object.
            // @param listenerLocation World position of the listener. This will
            //        be used to attenuate any sound effects created by the new effect.
            // @return TRUE if the effect object was successfully created and attached.
            bool ApplyWeaponEffect(
               SimCore::Actors::BaseEntity& owner,
               osgSim::DOFTransform* ownerDOF, 
               const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo,
               const osg::Vec3& listenerLocation );

            // Update or create tracer effects.
            // This updates the positions and visibilities of all the allocated tracers.
            // @param weaponFirePoint The world position at which the tracer is spawned.
            // @param intialVelocity The speed and direction in which the tracer travels.
            // @param effectsInfo The structure that holds parameters used in tracer creation,
            //        such as the tracer shader name, dimensions, and life span
            // @param useLight Determines if the effect should use a dynamic light; this is
            //        normally used for the first tracer in a group of tracer effects spawned
            //        by a tracer effect request.
            // @return The success of the operation; FALSE means that
            bool ApplyTracerEffect(
               const osg::Vec3& weaponFirePoint,
               const osg::Vec3& intialVelocity,
               const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo,
               bool useLight = false );

            bool AddTracerEffectRequest( dtCore::RefPtr<TracerEffectRequest>& effectRequest );

            unsigned ClearTracerEffectRequests();
            
            void UpdateTracerEffectRequests( float timeDelta );

            // Get the total of effect objects in existence.
            // @return The number of effects objects contained by this effects manager.
            unsigned GetWeaponEffectCount() const;

            // Get the number of tracer effects allocated within this manager.
            // @return The total number of tracers allocated within this manager.
            unsigned GetTracerEffectCount() const;

            // Get the number of tracer effects that are currently active in the simulation.
            // @return The number of active tracers opposed to the total
            //         number of allocated tracers.
            //
            // NOTE: This will return a number less than or equal to that returned
            //       by GetTracerEffectcount.
            unsigned GetTracerEffectActiveCount() const;

            // Update time on all effects and execute a recycle when necessary.
            // @param deltaTime The time in seconds since the last call to Update
            void Update( float deltaTime );

            // Iterate through all weapon and tracer effects and remove all those
            // that are invalid or outdated.
            // @return The number of effects objects recycled, both weapon and tracer effects.
            unsigned Recycle();

            // Recycle only tracer effects.
            // @return The number of tracer effects that were recycled.
            unsigned RecycleTracerEffects();

            // Recycle only weapon effects.
            // @return The number of weapon effects that were recycled.
            unsigned RecycleWeaponEffects();

            // Clear out all weapon effects objects and their resources.
            void ClearWeaponEffects();

            // Clear out all tracer effects objects and their resources.
            void ClearTracerEffects();

            // Clear all weapon and tracer effects
            void Clear();

            float CalcTimeToImpact( const osg::Vec3& weaponFirePoint, const osg::Vec3& initialVelocity, float maxTime = 10.0f );

         protected:
           virtual ~WeaponEffectsManager();

         private:
            float mEffectTimeMax;
            float mRecycleTime;
            float mCurRecycleTime; // increases to mRefreshTime
            int mMaxWeaponEffects;
            int mMaxTracerEffects;
            std::map<std::string, dtCore::RefPtr<WeaponEffect> > mEntityToEffectMap;
            std::vector<dtCore::RefPtr<TracerEffect> > mTracerEffects;
            dtCore::RefPtr<dtCore::BatchIsector> mIsector;
            dtCore::RefPtr<dtGame::GameManager> mGM;

            typedef std::vector<dtCore::RefPtr<TracerEffectRequest> > TracerEffectRequestList;
            TracerEffectRequestList mTracerRequests;
            TracerEffectRequestList mTracerRequestsDeletable;
      };

   }
}

#endif
