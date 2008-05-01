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

#ifndef _MUNITIONS_COMPONENT_H_
#define _MUNITIONS_COMPONENT_H_

#include <SimCore/Export.h>

#include <dtCore/base.h>
#include <dtCore/batchisector.h>
#include <dtCore/observerptr.h>

#include <SimCore/Actors/VolumetricLine.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/MunitionTypeActor.h>

#include <SimCore/Components/MunitionDamage.h>

#include <dtGame/gmcomponent.h>

#include <osgSim/DOFTransform>

// High Explosives (HE) use: Carleton Damage Model
// Improved Conventional Munition (ICM) use: Cookie Cutter Model

namespace dtAudio
{
   class Sound;
}

namespace dtCore
{
   class UniqueId;
   class ParticleSystem;
}
namespace dtGame
{
   class Message;
}

namespace SimCore
{
   class DetonationMessage;
   class ShotFiredMessage;

   namespace Actors
   {
      class DISIdentifier;
      class MunitionTypeActorProxy;
      class MunitionEffectsInfoActor;
   }

   namespace Components
   {
      class DamageType;
      class DamageHelper;

      //////////////////////////////////////////////////////////////////////////
      // Munition Type Table Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionTypeTable : public dtCore::Base
      {
      public:

         // Constructor
         MunitionTypeTable();

         // Access the number of MunitionTypeActors contained in this table.
         // @return Total MunitionTypeActors contained in this table
         unsigned int GetCount() const;

         // Add a new MunitionTypeActor to this table.
         // @param newType The new MunitionTypeActor to be added to this table.
         // @return TRUE if addition was successful. FALSE usually means another
         //         entry with the same name exists in this table.
         bool AddMunitionType( const dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy>& newType );

         // Remove a MunitionTypeActor by name.
         // @param name The name of the MunitionTypeActor to be removed.
         // @return TRUE if the entry existed and was successfully removed.
         bool RemoveMunitionType( const std::string& name );

         // Determine if a MunitionTypeActor exists within this table.
         // @param name The name of the MunitionTypeActor in question.
         // @return TRUE if this table contains a MunitionTypeActor with the specified name.
         bool HasMunitionType( const std::string& name ) const;

         // Access a MunitionTypeActor that is identified by the specified name.
         // @param name The name of the MunitionTypeActor to be found.
         // @return The MunitionTypeActor that has the specified name.
         //         NULL if no MunitionTypeActor was found with the name.
         SimCore::Actors::MunitionTypeActor* GetMunitionType( const std::string& name );
         const SimCore::Actors::MunitionTypeActor* GetMunitionType( const std::string& name ) const;

         // This function makes a closest match. DIS is not guaranteed to be exact.
         // @param dis The DIS identifier used to find a MunitionTypeActor with
         //        the closest matching DIS identifier
         // @param exactMatch Determine if this function should perform an exact match.
         // @return The MunitionTypeActor with the closest or exactly matching DIS
         //         identifier as that specified by the parameter dis.
         //         NULL if there was no match.
         const SimCore::Actors::MunitionTypeActor* GetMunitionTypeByDIS( const SimCore::Actors::DISIdentifier& dis, bool exactMatch = false ) const;
         const SimCore::Actors::MunitionTypeActor* GetMunitionTypeByDIS( const std::string& dis, bool exactMatch = false ) const;

         // Access this table's sorted list directly.
         // @return The list of MunitionTypeActors sorted by DIS identifiers,
         //         from low to high numbers (identifier numbers are read from left to right).
         const std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >& GetOrderedList() const { return mOrderedList; }

         // Query the number of entries in this table with in a separate sorted list.
         // The list is sorted by DIS identifier.
         // @return Total entries in the sorted list.
         //
         // NOTE: If the table is valid, the value returned should match the total
         //       returned by GetCount.
         unsigned int GetOrderedListSize() const;

         // Remove all entries from this table.
         void Clear();

      protected:

         // Destructor
         virtual ~MunitionTypeTable();

         // This function inserts the Munition Type Actor in the ordered list,
         // sorted by DIS identifier.
         // @param newType The new MunitionTypeActor that was recently added to this
         //        table and that needs to be entered in a sorted list.
         //
         // NOTE: The sorted list is important in the quick comparison of DIS identifiers.
         void InsertMunitionTypeToOrderedList( SimCore::Actors::MunitionTypeActor& newType );

         // Remove a MunitionTypeActor from this table's sorted list before removal
         // from the table itself.
         // @param oldType The MunitionTypeActor to be removed from the sorted list.
         void RemoveMunitionTypeFromOrderedList( const SimCore::Actors::MunitionTypeActor& oldType );

      private:
         std::map<std::string, dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> > mNameToMunitionMap;
         std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> > mOrderedList; // ordered list
      };



      //////////////////////////////////////////////////////////////////////////
      // Munition Damage Table Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionDamageTable : public dtCore::Base
      {
         public:

            // Constructor 
            // Note that munition data is specific to the interaction of certain
            // munitions to a specific entity type; in other words, the same
            // munition can be specified for multiple entity classes but the
            // munition damage probabilities may not be the same, and usually
            // are not.
            // @param entityClassName The name of the table ought to be the name
            //        of the entity class that has munition damage probability data.
            //        NOTE: this name will be used for mapping in a MunitionsComponent.
            MunitionDamageTable( const std::string& entityClassName );

            unsigned int GetCount() const;

            bool AddMunitionDamage( const dtCore::RefPtr<MunitionDamage>& newInfo );

            bool RemoveMunitionDamage( const std::string& name );

            bool HasMunitionDamage( const std::string& name ) const;

            const MunitionDamage* GetMunitionDamage( const std::string& name ) const;

            void Clear();

         protected:

            // Destructor
            virtual ~MunitionDamageTable();

         private:
            std::map<std::string, dtCore::RefPtr<MunitionDamage> > mNameToMunitionMap;
      };



      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT TracerEffect : public SimCore::Actors::VolumetricLine
      {
         public:
            TracerEffect( float lineLength, float lineThickness,
               const std::string& shaderName = "VolumetricLines",
               const std::string& shaderGroup = "TracerGroup" );

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
      class TracerEffectRequest : public dtCore::Base
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
            static const std::string CLASS_NAME;

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
      class SIMCORE_EXPORT WeaponEffectsManager : public dtCore::Base
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



      //////////////////////////////////////////////////////////////////////////
      // Munitions Component Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionsComponent : public dtGame::GMComponent
      {
         public:

            static const std::string DEFAULT_NAME;

            /// Constructor
            MunitionsComponent( const std::string& name = DEFAULT_NAME );

            bool Register( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork = true );

            bool Unregister( const dtCore::UniqueId& entityId );

            bool HasRegistered( const dtCore::UniqueId& entityId ) const;

            void ClearRegisteredEntities();

            // Calls upon a MunitionConfig parser to pull in data about munitions
            // per entity specified in entityClassName.
            // @param munitionConfigPath The file path to the MunitionsConfig.xml
            // @return number of successfully loaded tables.
            unsigned int LoadMunitionDamageTables( const std::string& munitionConfigPath );

            // Searches for a table by name of an entity class.
            // @param entityClassName The name of the entity class that should have
            //        a table of munition data that was loaded from the MunitionsConfig.xml
            // @return a pointer to the munition table; NULL if it does not exist.
            MunitionDamageTable* GetMunitionDamageTable( const std::string& entityClassName );
            const MunitionDamageTable* GetMunitionDamageTable( const std::string& entityClassName ) const;

            // NOTE: Takes a ref pointer to ensure that there is an object
            // tracking the table's memory, in case addition of the table fails.
            bool AddMunitionDamageTable( dtCore::RefPtr<MunitionDamageTable>& table );

            // Remove a damage table associated with an entity type.
            // @param entityClassName The name of the damage table to be removed.
            //        The damage table shares the same name as the entity class
            //        declared within the MunitionsConfig.xml
            bool RemoveMunitionDamageTable( const std::string& entityClassName );

            // This function loads the table that contains all munition type data.
            // @param mapName The name of the map within the MunitionTypesMap.xml
            // @return the number of munition types successfully loaded to the table.
            unsigned int LoadMunitionTypeTable( const std::string& mapName );

            MunitionTypeTable* GetMunitionTypeTable() { return mMunitionTypeTable.get(); }
            const MunitionTypeTable* GetMunitionTypeTable() const { return mMunitionTypeTable.get(); }

            void ClearTables();

            virtual void ProcessMessage( const dtGame::Message& msg );
            
            DamageHelper* GetHelperByEntityId( const dtCore::UniqueId id );

            void SetDamage( SimCore::Actors::BaseEntity& entity, DamageType& damage );
            
            void ApplyShotfiredEffects( const ShotFiredMessage& message,
               const SimCore::Actors::MunitionTypeActor& munitionType );

            void ApplyDetonationEffects( const DetonationMessage& message,
               const SimCore::Actors::MunitionTypeActor& munitionType );

            virtual void OnAddedToGM();

            // This function retrieves the name of the MunitionDamage object that
            // is mapped to the MunitionTypeActor of name munitionTypeName
            // @param munitionTypeName The name of the MunitionTypeActor
            // @return The name of the MunitionDamage that the MunitionTypeActor references.
            const std::string& GetMunitionDamageTypeName( const std::string& munitionTypeName ) const;

            // Set the default munition that will be used in place of any detonations
            // for which the associated munition is not found in the munition table.
            // @param munitionName Name of the munition to use for the default detonation effect.
            void SetDefaultMunitionName( const std::string& munitionName );
            const std::string& GetDefaultMunitionName() const;

            // Convenience method for directly retrieving a munition from the contained munition table.
            // @param munitionName Name of the munition to be retrieved from the munition type table.
            // @param defaultMunitionName Name of a default munition to be returned if the first munition could not be found.
            // @return the specified munition OR a default munition if default munition name has been specified
            //         OR NULL if neither a match or a default could be found.
            const SimCore::Actors::MunitionTypeActor* GetMunition(
               const std::string& munitionName, const std::string& defaultMunitionName = "" ) const;

            // Convenience method for directly retrieving munition effects info from the contained munition table.
            // @param munition Munition with effects to be verified and returned.
            // @param defaultMunitionName Name of a default munition to be returned if the first munition could not be found.
            // @return effects from the specified munition OR default munition effects if default munition name has been specified
            //         OR NULL if neither munition has a valid effect nor a default could be found.
            const SimCore::Actors::MunitionEffectsInfoActor* GetMunitionEffectsInfo(
               const SimCore::Actors::MunitionTypeActor& munition, const std::string& defaultMunitionName = "" ) const;

         protected:

            // Destructor
            virtual ~MunitionsComponent();

            void SetMunitionTypeTable( dtCore::RefPtr<MunitionTypeTable>& table ) { mMunitionTypeTable = table.get(); }

            virtual DamageHelper* CreateDamageHelper( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork );

         private:

            // The path to the munition config file that contains all the damage
            // tables to be loaded.
            std::string mMunitionConfigPath;

            // The name of the default munition to be used in place of munition detonations
            // that have not been defined in the munition table. This also points to the 
            // default munition effects info object used to setup the detonation effect.
            std::string mDefaultMunitionName;

            // This map holds onto all damages helpers. Each damage helper is mapped
            // to the entity's ID of the entity that is needing the damage help.
            std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> > mIdToHelperMap;

            // This map is responsible for holding onto all munition tables.
            // Damage helpers will reference these tables only by observer pointers
            // so that they are not left holding the table and its memory.
            // This helps munitions tables stay "alive" and not fall out of scope.
            std::map<std::string, dtCore::RefPtr<MunitionDamageTable> > mNameToMunitionDamageTableMap;

            // This table is responsible for holding onto all data related to
            // a munition, such as DIS identifier, name and resource file paths
            // for visual and sound effects.
            //
            // NOTE: This table does NOT hold damage information, since damage
            // information is specific to the interaction between a certain entity
            // type and the munition. Such data is contained in MunitionDamageTables,
            // per entity type.
            //
            // A MunitionDamage and a MunitionTypeActor should share the same name
            // of the munition to which they refer. With this, one can obtain
            // all the data pertaining to a certain munition
            dtCore::RefPtr<MunitionTypeTable> mMunitionTypeTable;

            // This isector is used for ground clamping detonation particle effects.
            dtCore::RefPtr<dtCore::BatchIsector> mIsector;
            float mLastDetonationTime;

            // A reference to the player is needed so that the sound from detonations
            // can be offset properly; there is a sound delay for which to account.
            dtCore::ObserverPtr<SimCore::Actors::StealthActor> mPlayer;

            // The effects manager is used for placing weapon effects on remote
            // entities that identify themselves as firing objects.
            // Effects include gun flash, fire sounds and tracers.
            dtCore::RefPtr<WeaponEffectsManager> mEffectsManager;
      };

   }
}

#endif
