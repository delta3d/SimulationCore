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
 * @author Chris Rodgers, Allen Danklefsen
 */
#ifndef _WEAPON_ACTOR_H_
#define _WEAPON_ACTOR_H_

#include <SimCore/Actors/IGActor.h>
#include <dtGame/message.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/Platform.h>

#include <SimCore/Actors/MunitionParticlesActor.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/collisioncontact.h>

namespace dtAudio
{
   class Sound;
}

namespace SimCore
{
   namespace Actors
   {
      class WeaponFlashActor;
      class WeaponFlashActorProxy;

      //////////////////////////////////////////////////////////
      // Actor Code
      //////////////////////////////////////////////////////////
      class WeaponActorProxy;
      class SIMCORE_EXPORT WeaponActor : public Platform
      {
         public:

            typedef Platform BaseClass;

            enum WeaponEffect
            {
               // formatting like opengl push / pop examples make indentions for enums
               // more easily readible for min and max. For others initializing your first variable
               // is also helpful, this allows the programmer not to have to guess what the initial
               // writer was trying to use.
               //
               // having MAX enums help for iterations / for loops if you ever have to cycle, or if
               // someone went over what you intended you can just check for >= MAX, so your enum
               // can be somewhat safer.
               WEAPON_EFFECT_NONE = 0,
               WEAPON_EFFECT_FIRE,
               WEAPON_EFFECT_DRY_FIRE,
               WEAPON_EFFECT_JAM,
               WEAPON_EFFECT_BROKEN,
               WEAPON_EFFECT_MAX
            };

            // Constructor
            WeaponActor(WeaponActorProxy& proxy);

            // Inherited Functions:
            virtual void OnEnteredWorld();

            virtual void OnRemovedFromWorld();

            // This override function updates time variables in the weapon that
            // cause it to re-fire or go into sleep mode (unregistering from TickMessages)
            virtual void OnTickLocal( const dtGame::TickMessage& tickMessage );

            // Set whether this weapon should send detonation messages for
            // rounds when they impact.
            // @param useBulletPhysics The flag that determines if detonation messages
            //        should be sent by bullets.
            //
            // NOTE: This is ignored if the munition type assigned to this weapon
            // is part of family GRENADE or EXPLOSIVE_ROUND; these family types
            // automatically trigger a detonation message to be sent.
            void SetUsingBulletPhysics( bool useBulletPhysics ) { mUseBulletPhysics = useBulletPhysics; }
            bool IsUsingBulletPhysics() const { return mUseBulletPhysics; }

            // Set the average scalar velocity of munitions leaving this weapon.
            void SetFireVelocity( float velocity ) { mFireVelocity = velocity; }
            float GetFireVelocity() const { return mFireVelocity; }

            // The distance in meters that the weapon "kicks" backward along its firing direction.
            void SetRecoilDistance( float distance ) { mRecoilDistance = distance; }
            float GetRecoilDistance() const { return mRecoilDistance; }

            // Set the time in seconds that it takes this weapon to travel back
            // to its resting position from a recoil.
            // @param restTime Time in seconds it takes the weapon to travel
            //        back to its resting position from a recoil event.
            void SetRecoilRestTime( float restTime ) { mRecoilRestTime = restTime; }
            float GetRecoilRestTime() const { return mRecoilRestTime; }

            // Set the time period for this weapon actor to wait before unregistering from TickLocal.
            // @param sleepTime Time in seconds that this weapon will wait before
            //        unregistering its TickLocal function.
            void SetAutoSleepTime( float sleepTime ) { mAutoSleepTime = sleepTime; }
            float GetAutoSleepTime() const { return mAutoSleepTime; }

            // Set the trigger state of this weapon. While in held state, this
            // weapon will re-fire as long as it has a FireRate greater than 0.0.
            // @param held The held state of the trigger.
            //
            // NOTE: Setting the held state to true will automatically "wake" the
            //       weapon by registering its TickLocal functions.
            void SetTriggerHeld( bool hold );
            bool IsTriggerHeld() const { return mTriggerHeld; }

            /**
             * Set the weapon's state to either jammed or unjammed.
             * @param jammed The jammed state to be set on this weapon.
             *
             * NOTE: This will need to be called manually to set the
             *       jammed state to FALSE before attempting another shot.
             */
            void SetJammed( bool jammed ) { mJammed = jammed; }
            bool IsJammed() const { return mJammed; }

            // Implement the NxAgeiaPhysicsInterface to receive contact report data.
            //
            // Called by the shooter for every contact a bullet makes
            // with another entity or terrain.
            // @param report The structure of PhysX data describing the physical contact.
            // @param target The object that was hit by the physics bullet
            //
            // NOTE: It is this weapons responsibility for collecting and preparing
            //       data for a network message, as well as forwarding a local
            //       message through the application.
            virtual void ReceiveContactReport(dtPhysics::CollisionContact& report, dtGame::GameActorProxy* target);

            /**
             * Set the shooter that this weapon uses to display tracers and/or
             * spawn physical objects representing rounds that must collide realistically
             * with the environment.
             * @param shooter The physics particle system that spawns tracers and physical rounds.
             */
            void SetShooter(MunitionParticlesActorProxy* shooter) { mShooter = shooter; }
            MunitionParticlesActorProxy* GetShooter() { return mShooter.get(); }
            const MunitionParticlesActorProxy* GetShooter() const  { return mShooter.get(); }

            // Set the name of the munition type that this shooter unit will be using.
            // @param munitionTypeName The name of the MunitionTypeActor to be retrieved
            //        from the MunitionsComponent.
            //
            // NOTE: This does not automatically access the MunitionTypeActor from
            //       MunitionsComponent. A WeaponActor loads
            void SetMunitionTypeName( const std::string& munitionTypeName ) { mMunitionTypeName = munitionTypeName; }
            std::string GetMunitionTypeName() const { return mMunitionTypeName; }

            // Set the munition type directly that this weapon will reference for effects.
            void SetMunitionType( MunitionTypeActor* munitionType );
            const MunitionTypeActor* GetMunitionType() const { return mMunitionType.get(); }

            // The following two functions are similar to SetMunitionType and GetMunitionType.
            // However, these functions are used by ActorActorProperties so that the
            // MunitionTypeActor can be set from STAGE or by playback mode in AAR.
            // @param proxy The MunitionTypeActorProxy to be referenced.
            void SetMunitionTypeProxy( dtDAL::ActorProxy* proxy );

            // This function helps to load the MunitionTypeActor that this weapon
            // will need for messaging. The actor will attempt to access the
            // MunitionsComponent that contains the MunitionTypeActor.
            // @param munitionTypeName The name of the MunitionTypeActor to be
            //        loaded from the MunitionsComponent.
            // @return TRUE if a new MunitionTypeActor was assigned to this weapon.
            //         FALSE if not able to access a valid MuntionTypeActor of
            //         specified name munitionTypeName
            //
            // NOTE: The weapon's original reference to a MunitionTypeActor will
            //       not change if this function returns FALSE. This is behavior
            //       is also the same for munition name stored by this weapon.
            bool LoadMunitionType( const std::string& munitionTypeName );

            // The actor that owns this object and that is published on the network.
            // The actor's ID will be sent as the SendingID for weapon fire messages.
            // @param proxy The owner actor proxy
            //
            // NOTE: If an owner is not specified, the weapon will supply its own ID
            // as the sender for weapon fire messages.
            void SetOwner( dtDAL::ActorProxy* proxy );
            dtDAL::ActorProxy* GetOwner();

            // The flash actor responsible for timing and rendering the flash effects
            // produced by this weapon.
            // @param flashActor The flash actor to be assigned to this weapon.
            void SetFlashActor( WeaponFlashActor* flashActor );
            WeaponFlashActor* GetFlashActor();
            const WeaponFlashActor* GetFlashActor() const;

            // These functions are the same as Set/GetFlashActor except that
            // they utilize base pointers.
            // This function is used for the actor property mapping.
            void SetFlashActorProxy( dtCore::BaseActorObject* flashProxy );

            // Set the re-fire rate measured in seconds.
            // @param rate The time period to wait before firing another shot
            void SetFireRate( float rate );
            float GetFireRate() const { return mFireRate; }

            // Set the frequency of tracers.
            // @param frequency Every nth round that will be a tracer.
            //
            // Examples:
            //    0 (no tracers)
            //    1 (every round fired will be a tracer)
            //   10 (every 10th round fired will be a tracer)
            void SetTracerFrequency( int frequency ) { mTracerFrequency = frequency; }
            int GetTracerFrequency() const { return mTracerFrequency; }

            // Set the current ammo this shooter contains
            // @param count The total amount of ammo to load into this shooter
            //
            // NOTE: This function caps the value to the shooter's AmmoMax
            void SetAmmoCount( int count );
            int GetAmmoCount() const;

            // Set the maximum number of ammo that this shooter can hold
            // @param ammoMax The limit of ammo that the shooter can hold
            void SetAmmoMax( int ammoMax );
            int GetAmmoMax() const;

            // Set the probability that this shooter will jam.
            // This function will clamp the supplied value between 0.0 and 1.0.
            // @param probability The probability ranging from 0.0 to 1.0; 1.0 is 100%
            void SetJamProbability( float probability )
            {
               mJamProbability = probability < 0.0f ? 0.0f : probability > 1.0f ? 1.0f : probability;
            }
            float GetJamProbability() const { return mJamProbability; }

            // Set the probability that this weapon will produce a flash when fired.
            // This function will clamp the supplied value between 0.0 and 1.0.
            // @param probability The probability ranging from 0.0 to 1.0; 1.0 is 100%
            void SetFlashProbability( float probability )
            {
               mFlashProbability = probability < 0.0f ? 0.0f : probability > 1.0f ? 1.0f : probability;
            }
            float GetFlashProbability() const { return mFlashProbability; }

            // Set the amount of time that a flash effect will remain visible when created.
            // @param flashTime The time in seconds that a flash effect remains visible.
            void SetFlashTime( float flashTime ) { mFlashTime = flashTime; }
            float GetFlashTime() const { return mFlashTime; }

            // Set the minimum amount of time between sending messages hitting
            // the same or no target.
            // @param time The time in seconds
            //
            // NOTE: The time is ignored if a hit target changes during a message wait cycle;
            //       A message wait cycle is the period of time between the last message and
            //       the next time a message is allowed to be sent.
            void SetTimeBetweenMessages( float time ) { mMessageCycleTime = time < 0.0f ? 0.0f : time; }
            float GetTimeBetweenMessages() const { return mMessageCycleTime; }

            // Determine if the weapon has unregistered from TickLocal.
            bool IsSleeping() const { return mSleeping; }

            // This function calls upon the current shooter unit to fire a round and
            // execute a weapon effect based on the fire success and the munition
            // type that was used.
            //
            // NOTE: This together with TickLocal will cause this weapon to automatically
            // send shot fired messages when appropriate.
            void Fire();

            // Attach a drawable object to a DOF of the weapon's model.
            // @param object The drawable object that needs to be attached to a DOF on the weapon.
            // @param dofName The name of the DOF to which the object must attach.
            // @return TRUE if attachment was successful.
            //
            // NOTE: This function requires that the model to be loaded.
            bool AttachObject( dtCore::Transformable& object, const std::string& dofName );

            // This function will send the message that represents multiple rounds fired.
            // @param quantity The number of rounds fired.
            // @param initialVelocity The velocity of the round when leaving the weapon.
            // @param target The object that was targeted and that should receive a
            //               Direct Fire message. Specify NULL if this is Indirect Fire.
            void SendFireMessage( unsigned short quantity, const osg::Vec3& initialVelocity,
               const dtCore::Transformable* target = NULL );

            // This function is used for sending messages about grenade type rounds
            // @param quantity The number of rounds fired.
            // @param location The world location of the detonation.
            // @param finalVelocity The velocity of the round on impact.
            // @param target The object that was targeted and that should receive a
            //               Direct Fire message. Specify NULL if this is Indirect Fire.
            //
            // NOTE: proximity type munitions like missiles and mines who
            //       have actual Entity representations are responsible
            //       for sending their own detonation messages.
            void SendDetonationMessage( unsigned short quantity, const osg::Vec3& finalVelocity,
               const osg::Vec3& location, const dtCore::Transformable* target = NULL );

            void LoadSoundFire( const std::string& filePath );
            void LoadSoundDryFire( const std::string& filePath );
            void LoadSoundJammed( const std::string& filePath );

            // Reset counters and timers on this weapon. This funtion will
            // be useful in unit tests.
            void Reset()
            {
               mTriggerTime = 0.0f;
               mFireMessageTime = 0.0f;
               mDetMessageTime = 0.0f;
               mCurRecoilRestTime = 0.0f;
               mCurSleepTime = 0.0f;
               mLastTargetObject = NULL;
            }

            void AddDynamicLight();

         protected:
            // Destructor
            virtual ~WeaponActor();

            void SoundLoad( const std::string& filePath, dtCore::RefPtr<dtAudio::Sound>& sound );

            void SoundPlay( dtCore::RefPtr<dtAudio::Sound>& sound );

            void SoundRelease( dtCore::RefPtr<dtAudio::Sound>& sound );

         private:
            // Determines if normal bullets should send detonation messages.
            bool mUseBulletPhysics;

            // Tracks if the weapon has unregistered from TickLocal.
            bool mSleeping;

            // State that can prevent a weapon from firing.
            bool mJammed;

            bool mTriggerHeld;

            // lets TickLocal know if the weapon has fired to help determine if a message should be sent.
            bool mFired;

            // Triggers a message to be sent prematurely in TickLocal, ignoring message time.
            bool mTargetChanged;

            // Time before another shot can be fired.
            // This time grows from 0 to the target time delta equal or greater than fire rate.
            float mTriggerTime;

            // The distance the weapon back up when fired (in meters)
            float mRecoilDistance;

            // Time in seconds for the weapon to travel back to rest position.
            float mRecoilRestTime;

            // Time in seconds remaining for the weapon to return to the rest position
            // from a recoil.
            float mCurRecoilRestTime;

            // Time in seconds from that last fire event before the weapon will unregister from TickLocal.
            float mAutoSleepTime;

            // Time in seconds left before unregistering from TickLocal.
            float mCurSleepTime;

            // The number of rounds per second; can be fractional for odd number over even seconds.
            float mFireRate;

            // The chance that the weapon will jam, ranging between 0.0 and 1.0
            float mJamProbability;

            // The probability that the weapon will produce a flash effect.
            float mFlashProbability;

            // The length of time a flash will remain visible
            float mFlashTime;

            // Every nth (mTracerFrequency) round will be a tracer.
            int mTracerFrequency;

            // The current amount of ammunition contained in this weapon
            int mAmmoCount;

            // The maximum amount of ammunition that this weapon can hold.
            int mAmmoMax;

            // Tracks the total shots fired from the last message sent
            unsigned mShotsFired;

            // Time increases to a time value when a message must be sent.
            float mFireMessageTime;
            float mDetMessageTime;

            // The minimum amount of time allowed between messages hitting the same target.
            float mMessageCycleTime;

            // The running count of successful hits on the last detected target.
            unsigned short mHitCount;

            // The running total of weapon fire messages made with this weapon.
            // This is used for the event identifier parameter in weapon fire
            // and detonation messages.
            unsigned short mMessageCount;

            // The exit velocity of munitions leaving the weapon.
            float mFireVelocity;

            //this is the id that corresponds to our dynamic light effect
            unsigned mDynamicLightID;

            //keeps track of our dynamic light so we know if we should disable it
            bool mDynamicLightEnabled;

            // The world location where the impact occurred.
            osg::Vec3 mLastHitLocation;

            // The velocity recorded from the last contact report.
            // This is used as the final velocity for detonation messages.
            osg::Vec3 mLastVelocity;

            // The target id that was recorded on the last hit report.
            // Changes to this will trigger a message being sent.
            std::string mLastTargetID;

            // The name of the currently referenced munition type.
            // This can be used to automatically acquire the
            // MunitionTypeActor during OnEnteredWorld
            std::string mMunitionTypeName;

            dtCore::ObserverPtr<dtCore::Transformable> mLastTargetObject;

            // The MunitionTypeActor that contains all data describing the
            // munition that this weapon fires.
            dtCore::RefPtr<MunitionTypeActor> mMunitionType;

            // The actor that owns this weapon and that is published
            // on the network. This object's ID will be sent in
            // the weapon fire messages.
            dtCore::ObserverPtr<dtDAL::ActorProxy> mOwner;

            // The flash actor that is responsible for rendering
            // and the timing of flash effects.
            dtCore::RefPtr<WeaponFlashActor> mFlash;

            // The sound effects that this weapon will use
            dtCore::RefPtr<dtAudio::Sound> mSoundFire;
            dtCore::RefPtr<dtAudio::Sound> mSoundDryFire;
            dtCore::RefPtr<dtAudio::Sound> mSoundJammed;

            // Reference to the shooter that shoots the visual representations of the rounds.
            dtCore::RefPtr<MunitionParticlesActorProxy>  mShooter;

      };



      //////////////////////////////////////////////////////////
      // Actor Proxy code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT WeaponActorProxy : public PlatformActorProxy
      {
         public:
            // Constructor
            WeaponActorProxy();

            // Creates the actor
            virtual void CreateDrawable();

            // Adds the properties associated with this actor
            virtual void BuildPropertyMap();

            virtual void OnEnteredWorld();

         protected:
            // Destructor
            virtual ~WeaponActorProxy();
      };

   }
}

#endif
