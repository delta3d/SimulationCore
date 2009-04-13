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
      class SIMCORE_EXPORT MunitionEffect : public dtCore::Transformable
      {
         public:
            MunitionEffect();

            void SetLightName( const std::string& lightName );
            const std::string& GetLightName() const;

            void SetMaxLifeTime( float maxLifeTime );
            float GetMaxLifeTime() const;

            void SetVelocity( const osg::Vec3& velocity );
            const osg::Vec3& GetVelocity() const;

            virtual void SetPosition( const osg::Vec3& position );
            const osg::Vec3& GetPosition() const;

            void SetVisible( bool visible );
            bool IsVisible() const;

            bool IsActive() const;

            virtual void Reset();

            void Execute( float maxLifeTime );

            virtual void Update( float deltaTime );

            void AddDynamicLight( const osg::Vec3& color );
            void RemoveDynamicLight();

            void SetGameManager( dtGame::GameManager* gameManager );

         protected:
            virtual ~MunitionEffect();

            bool      mDynamicLightEnabled;
            unsigned  mDynamicLightID;
            float     mLifeTime;
            float     mMaxLifeTime;
            osg::Vec3 mLastPosition;
            osg::Vec3 mPosition;
            osg::Vec3 mVelocity;
            dtUtil::RefString mTracerLightName;
            dtCore::RefPtr<dtGame::GameManager> mGM; // for accessing the rendering support component (safer using GM)
      };



      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT TracerEffect : public MunitionEffect
      {
         public:
            typedef MunitionEffect BaseClass;

            static const dtUtil::RefString DEFAULT_TRACER_LIGHT;
            static const dtUtil::RefString DEFAULT_TRACER_SHADER;
            static const dtUtil::RefString DEFAULT_TRACER_SHADER_GROUP;

            TracerEffect( float lineLength, float lineThickness,
               const std::string& shaderName = DEFAULT_TRACER_SHADER.Get(),
               const std::string& shaderGroup = DEFAULT_TRACER_SHADER_GROUP.Get() );

            void SetMaxLength( float maxLength );
            float GetMaxLength() const;

            virtual void SetPosition( const osg::Vec3& position );

            virtual void Reset();

            virtual void Update( float deltaTime );

         protected:
            virtual ~TracerEffect();

         private:
            float mMaxLength;
            dtCore::RefPtr<SimCore::Actors::VolumetricLine> mLine;
      };



      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Request Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionEffectRequest : public osg::Referenced
      {
         public:
            MunitionEffectRequest( unsigned totalEffects, float cycleTime,
               const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo );

            void SetOwner( SimCore::Actors::BaseEntity* owner );
            SimCore::Actors::BaseEntity* GetOwner();
            const SimCore::Actors::BaseEntity* GetOwner() const;

            void SetFirePoint( const osg::Vec3& firePoint );
            const osg::Vec3& GetFirePoint() const;

            void SetVelocity( const osg::Vec3& velocity );
            const osg::Vec3& GetVelocity() const;

            void SetMunitionModelFile( const std::string& modelFile );
            const std::string& GetMunitionModelFile() const;

            const SimCore::Actors::MunitionEffectsInfoActor& GetEffectsInfo() const;

            float GetCycleTime() const;

            unsigned GetTotalEffects() const;

            bool IsFirstEffect() const;

            bool IsEffectReady() const;

            void Update( float timeDelta );

            unsigned Decrement();
         
         protected:
            virtual ~MunitionEffectRequest();

         private:
            bool     mIsFirstEffect;
            unsigned mTotalEffects;
            float    mCycleTime;
            float    mCurrentTime;
            osg::Vec3 mFirePoint;
            osg::Vec3 mVelocity;
            dtUtil::RefString mMunitionModelFile;
            dtCore::RefPtr<const SimCore::Actors::MunitionEffectsInfoActor> mEffectsInfo;
            dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mOwner;
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
            void SetFlashProbability( float probability );
            float GetFlashProbability() const;

            // Setting the flash time will automatically make the flash visible.
            // @param flashTime The period of time the flash effect shall stay visible.
            void SetFlashTime( float flashTime );
            float GetFlashTime() const;

            void SetFlash( dtCore::ParticleSystem* flash );
            dtCore::ParticleSystem* GetFlash();
            const dtCore::ParticleSystem* GetFlash() const;

            // Assign the sound object.
            // @param sound The sound object
            // @param releaseOldSound Determines if this object should free
            //        the sound memory before assigning the new sound object.
            void SetSound( dtAudio::Sound* sound, bool releaseOldSound = true );
            dtAudio::Sound* GetSound();
            const dtAudio::Sound* GetSound() const;

            // Set the entity that will own this effect.
            // This MUST be set in order for a call to Attach to be successful.
            // @param owner The entity (usually remote) to which this effects 
            //        object will attach.
            void SetOwner( SimCore::Actors::BaseEntity* owner );
            SimCore::Actors::BaseEntity* GetOwner();
            const SimCore::Actors::BaseEntity* GetOwner() const;

            // Set the visibility of the flash effect.
            // @param visible The visibility state of the flash effect.
            void SetVisible( bool visible );
            bool IsVisible() const;

            // Access the referenced DOF that was set via Attach
            osgSim::DOFTransform* GetDOF();
            const osgSim::DOFTransform* GetDOF() const;

            // Get the time since the flash effect was executed.
            // This time is used by the WeaponEffectsManager to determine
            // if this effect object is old enough for recycling.
            // @return The time in seconds since the last flash execution.
            float GetTimeSinceFlash() const;

            // Determine if the sound part of the effect has been played.
            // @return TRUE if the sound had been executed.
            bool IsSoundPlayed() const;

            // Get the time in seconds that sound will be played after Execute is called.
            // @return seconds from the instant Execute is called to the point the
            // associated sound effect will played.
            //
            // NOTE: This function is mostly intended for testing purposes.
            //
            //       The delay is calculated after Execute is called in which the
            //       listener's world position is specified.
            //
            float GetSoundDelay() const;

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
            dtGame::GameManager* GetGameManager();
            const dtGame::GameManager* GetGameManager() const;

            // Return the isector responsible for collision detection of tracer effects.
            // NOTE: This is intended for testing purposes only.
            const dtCore::BatchIsector* GetIsector() const;

            // Set the maximum length of time that any WeaponEffect object should
            // live before being recycled.
            // @param effectTimeMax The max time in seconds that an effect is allowed to persist.
            void SetEffectTimeMax( float effectTimeMax );
            float GetEffectTimeMax() const;

            // Set the limit of effects allowed to exist at any one time.
            // @param maxEffectsAllowed The limit on effects allowed to be created.
            //        Negative values mean NO limit.
            void SetMaxWeaponEffects( int maxEffectsAllowed );
            int GetMaxWeaponEffects() const;

            // Set the limit of munition effects allowed to exist at any one time.
            // @param maxEffectsAllowed The limit on effects allowed to be created.
            //        Negative values mean NO limit.
            void SetMaxMunitionEffects( int maxEffectsAllowed );
            int GetMaxMunitionEffects() const;

            // Set the cycle length between each call to Recycle.
            // @param recycleTime The time in seconds between each automatic
            //        call to Recycle.
            void SetRecycleTime( float recycleTime );
            float GetRecycleTime() const;

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
            // @param effectRequest Object containing other parameters related to the effect,
            //        such as the owner entity that fired the munition.
            // @return The success of the operation; FALSE means that
            bool ApplyMunitionEffect(
               const osg::Vec3& weaponFirePoint,
               const osg::Vec3& intialVelocity,
               const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo,
               MunitionEffectRequest& effectRequest );

            bool AddMunitionEffectRequest( dtCore::RefPtr<MunitionEffectRequest>& effectRequest );

            unsigned ClearMunitionEffectRequests();
            
            void UpdateMunitionEffectRequests( float timeDelta );

            // Get the total of effect objects in existence.
            // @return The number of effects objects contained by this effects manager.
            unsigned GetWeaponEffectCount() const;

            // Get the number of munition effects allocated within this manager.
            // @return The total number of munition effects allocated within this manager.
            unsigned GetMunitionEffectCount() const;

            // Get the number of munition effects that are currently active in the simulation.
            // @return The number of active munition effects opposed to the total
            //         number of allocated munition effects.
            //
            // NOTE: This will return a number less than or equal to that returned
            //       by GetTracerEffectcount.
            unsigned GetMunitionEffectActiveCount() const;

            // Update time on all effects and execute a recycle when necessary.
            // @param deltaTime The time in seconds since the last call to Update
            void Update( float deltaTime );

            // Iterate through all weapon and tracer effects and remove all those
            // that are invalid or outdated.
            // @return The number of effects objects recycled, both weapon and tracer effects.
            unsigned Recycle();

            // Recycle only munition effects.
            // @return The number of munition effects that were recycled.
            unsigned RecycleMunitionEffects();

            // Recycle only weapon effects.
            // @return The number of weapon effects that were recycled.
            unsigned RecycleWeaponEffects();

            // Clear out all weapon effects objects and their resources.
            void ClearWeaponEffects();

            // Clear out all munition effects objects and their resources.
            void ClearMunitionEffects();

            // Clear all weapon and tracer effects
            void Clear();

            float CalcTimeToImpact( const osg::Vec3& weaponFirePoint, const osg::Vec3& initialVelocity,
               float maxTime = 10.0f, SimCore::Actors::BaseEntity* owner = NULL );

         protected:
           virtual ~WeaponEffectsManager();

         private:
            float mEffectTimeMax;
            float mRecycleTime;
            float mCurRecycleTime; // increases to mRefreshTime
            int mMaxWeaponEffects;
            int mMaxMunitionEffects;
            typedef std::map<std::string, dtCore::RefPtr<WeaponEffect> > EntityToWeaponEffectMap;
            EntityToWeaponEffectMap mEntityToEffectMap;
            typedef std::vector<dtCore::RefPtr<MunitionEffect> > MunitionEffectArray;
            MunitionEffectArray mMunitionEffects;
            dtCore::RefPtr<dtCore::BatchIsector> mIsector;
            dtCore::RefPtr<dtGame::GameManager> mGM;

            typedef std::vector<dtCore::RefPtr<MunitionEffectRequest> > MunitionEffectRequestList;
            MunitionEffectRequestList mTracerRequests;
            MunitionEffectRequestList mTracerRequestsDeletable;
      };

   }
}

#endif
