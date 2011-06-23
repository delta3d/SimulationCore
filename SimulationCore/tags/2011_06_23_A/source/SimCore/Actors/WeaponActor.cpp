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
 * @author Chris Rodgers, editor Allen Danklefsen
 */
#include <prefix/SimCorePrefix.h>

#include <dtUtil/mswin.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>

#include <dtUtil/mathdefines.h>

#include <dtUtil/nodecollector.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/project.h>

#include <dtGame/basemessages.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/messagefactory.h>
#include <dtGame/deadreckoninghelper.h>


#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/WeaponFlashActor.h>

#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/MunitionsComponent.h>

#include <SimCore/Components/RenderingSupportComponent.h> //for dynamic lights

#include <osg/Geode>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////////////////////
      WeaponActor::WeaponActor( WeaponActorProxy &proxy )
         : Platform(proxy)
         , mUseBulletPhysics(false)
         , mSleeping(false)
         , mJammed(false)
         , mTriggerHeld(false)
         , mFired(false)
         , mTargetChanged(false)
         , mTriggerTime(0.0f)
         , mRecoilDistance(0.0f)
         , mRecoilRestTime(0.0f)
         , mCurRecoilRestTime(0.0f)
         , mAutoSleepTime(0.0f)
         , mCurSleepTime(0.0f)
         , mFireRate(0.0f) // 0 , means single fire
         , mJamProbability(0.0f)
         , mFlashProbability(1.0f)
         , mFlashTime(0.15f)
         , mTracerFrequency(0)
         , mAmmoCount(0)
         , mAmmoMax(100000)
         , mShotsFired(0)
         , mFireMessageTime(0.0f)
         , mDetMessageTime(0.0f)
         , mMessageCycleTime(0.5f)
         , mHitCount(0)
         , mMessageCount(0)
         , mFireVelocity(1000.0f)
         , mDynamicLightID(0)
      {
         SetAutoRegisterWithMunitionsComponent(false);
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponActor::~WeaponActor()
      {
         SoundRelease( mSoundFire );
         SoundRelease( mSoundDryFire );
         SoundRelease( mSoundJammed );
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
      {
         float timeDelta = tickMessage.GetDeltaSimTime();

         // Update trigger timer
         if( mTriggerTime < mFireRate )
         {
            mTriggerTime += timeDelta;
         }

         if( mTriggerHeld && mTriggerTime >= mFireRate )
         {
            Fire();
         }

         // Update message timer
         if( mFireMessageTime < mMessageCycleTime )
         {
            mFireMessageTime += timeDelta;
         }
         if( mDetMessageTime < mMessageCycleTime )
         {
            mDetMessageTime += timeDelta;
         }

         // Update auto-sleep timer
         if( mAutoSleepTime > 0.0f && mCurSleepTime > 0.0f )
         {
            mCurSleepTime -= timeDelta;
            if( mCurSleepTime <= 0.0f )
            {
               if( !mSleeping )
               {
                  GetGameActorProxy().UnregisterForMessages(
                     dtGame::MessageType::TICK_LOCAL,
                     dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );
                  mSleeping = true;
                  LOG_DEBUG( "Weapon unregistered from TickLocal (INACTIVE)" );
               }
            }
         }

         // Send a message if a target has changed or the time allows.
         if( mFired && ( mTargetChanged || mFireMessageTime >= mMessageCycleTime ) )
         {
            if( mUseBulletPhysics ) // Indirect Fire (hit by physics)
            {
               // Get the weapons direction
               dtCore::Transform xform;
               GetTransform(xform);
               osg::Matrix mtx;
               xform.GetRotation(mtx);
               osg::Vec3 initialVelocity( mtx.ptr()[4], mtx.ptr()[5], mtx.ptr()[6] );
               initialVelocity *= mFireVelocity;

               // Send the fire message
               SendFireMessage(
                  // if no hit reports were received and not using bullet physics, send the shots that were fired
                  mShotsFired,
                  initialVelocity,
                  mLastTargetObject.get() );
            }
            else // Direct Fire (instant fire & hit)
            {
               bool directHit = mLastTargetObject.valid();
               // Send the fire message
               SendFireMessage(
                  // if no hit reports were received and not using bullet physics, send the shots that were fired
                  directHit ? mHitCount : mShotsFired,
                  mLastVelocity,
                  mLastTargetObject.get() );

               // Send a detonation as well
               SendDetonationMessage(
                  directHit ? mHitCount : mShotsFired,
                  mLastVelocity,
                  mLastHitLocation,
                  mLastTargetObject.get() );
            }

            mHitCount = 0;
            mShotsFired = 0;
            mFireMessageTime = 0.0f;
            mTargetChanged = false;
            mFired = false;
         }

      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetAmmoCount( int count )
      {
         mAmmoCount = count > mAmmoMax ? mAmmoMax : count;
      }

      //////////////////////////////////////////////////////////////////////////
      int WeaponActor::GetAmmoCount() const
      {
         return mAmmoCount;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetAmmoMax( int ammoMax )
      {
         mAmmoMax = ammoMax;
      }

      //////////////////////////////////////////////////////////////////////////
      int WeaponActor::GetAmmoMax() const
      {
         return mAmmoMax;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetFireRate( float rate )
      {
         // Determine if this is a transition from automatic mode
         // to single-shot mode.
         if( rate != mFireRate && rate == 0.0f )
         {
            mTriggerHeld = false;
            mTriggerTime = 0.0f; // prevents another shot on next tick
         }
         mFireRate = rate;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetTriggerHeld( bool hold )
      {
         if( ! mTriggerHeld && hold && mSleeping )
         {
            if( GetGameActorProxy().IsInGM() )
            {
               mSleeping = false;
               GetGameActorProxy().RegisterForMessages(
                  dtGame::MessageType::TICK_LOCAL,
                  dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );
               LOG_DEBUG( "Weapon registered from SetTriggerHeld (ACTIVE)" );
            }
         }

         // execute instant fire
         // Removed this - Fire() accounts for how long it's been.  By resetting the timer, it
         // allows you to 'click' fast fire.
         /*if( ! mTriggerHeld && hold )
         {
            // Set time variable to allow an instant fire
            mTriggerTime = mFireRate;
            mTriggerHeld = true;
            Fire();
         }
         else
         {
            mTriggerHeld = hold;
         }*/
         mTriggerHeld = hold;

      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::AddDynamicLight()
      {
         //this creates a dynamic light
         SimCore::Components::RenderingSupportComponent* renderComp;
         GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                  renderComp);

         if(renderComp != NULL)
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl =
                  renderComp->GetDynamicLight(mDynamicLightID);
            if(dl == NULL)
            {
               dl = renderComp->AddDynamicLightByPrototypeName("Light-WeaponFlash");
               //dl = new SimCore::Components::RenderingSupportComponent::DynamicLight();
               mDynamicLightID = dl->GetId();//renderComp->AddDynamicLight(dl);
            }

            //dl->mColor.set(0.97f, 0.98f, 0.482f);//a bright yellow
            //dl->mAttenuation.set(0.1, 0.05, 0.0002);
            //dl->mIntensity = 1.0f;
            //dl->mSaturationIntensity = 0.0f; //no saturation
            dtCore::Transformable* transformable = this;
            if(mFlash.valid())
            {
               transformable = mFlash.get();
            }
            dl->mTarget = transformable;
            //dl->mAutoDeleteAfterMaxTime = true;
            //dl->mMaxTime = 0.5f;

         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::Fire()
      {
         // Avoid firing again if weapon is in the middle of a re-fire cycle
         if( ! mTriggerHeld || mTriggerTime < mFireRate ) { return; }

         if( mSleeping )
         {
            GetGameActorProxy().RegisterForMessages(
               dtGame::MessageType::TICK_LOCAL,
               dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );
            mSleeping = false;
            LOG_DEBUG( "Weapon registered from TickLocal (ACTIVE)" );
         }

         // Reset re-fire timer
         mTriggerHeld = mFireRate > 0.0f; // allow re-fire if fire rate does not specify single-shot mode (rate of 0.0)
         mTriggerTime = mTriggerHeld && mFireRate < mTriggerTime
            ? mTriggerTime - ((int)(mTriggerTime/mFireRate)) * mFireRate // float mod; leave the difference over fire rate
            : 0.0f;

         WeaponEffect effectType = WEAPON_EFFECT_FIRE;

         if( GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED )
         {
            effectType = WEAPON_EFFECT_BROKEN;
         }
         else if( mAmmoCount == 0 )
         {
            effectType = WEAPON_EFFECT_DRY_FIRE;
         }

         // Determine if the weapon is jammed or should become jammed.
         // The jamming probability should only be used with local simulations.
         if( mJammed || ( ! IsRemote() && mJamProbability >= dtUtil::RandFloat(0.0f,1.0f) ) )
         {
            effectType = WEAPON_EFFECT_JAM;
            mJammed = true;
         }

         // Execute the shooter before applying weapon effects
         if( effectType == WEAPON_EFFECT_FIRE )
         {
            /*/ --- DEBUG --- START --- //
            std::cout << "Fired round: " << mAmmoCount << std::endl;
            std::cout << "DEBUG Weapon/Shooter Orientation:" << std::endl;
            dtCore::Transform xform;
            GetTransform(xform);
            const osg::Matrix& mtx = xform.GetRotation();
            osg::Vec3 normal( mtx.ptr()[4], mtx.ptr()[5], mtx.ptr()[6] );
            if( normal.length2() > 0.0 ) { normal.normalize(); }
            std::cout << "\tWeapon: " << normal[0] << ", " << normal[1] << ", " << normal[2] << std::endl;
            // --- DEBUG --- END --- /*/

            if( mShooter.valid() )
            {
               MunitionParticlesActor* particleSystem = NULL;
               mShooter->GetActor(particleSystem);

               // Add the vehicles current velocity to the weapon.
               osg::Vec3 vehicleVelocity;
               BaseEntityActorProxy* entityProxy = dynamic_cast<BaseEntityActorProxy*>(mOwner.get());
               //std::cout << "Weapon Actor shooting. Attempting to set parent velocity." << std::endl;
               if (entityProxy != NULL)
               {
                  vehicleVelocity = entityProxy->GetComponent<dtGame::DeadReckoningHelper>()->GetLastKnownVelocity();
                  //std::cout << "      NOW SETTING PARENT VELOCITY TO [" << vehicleVelocity << "]." << std::endl;
                  particleSystem->SetParentsWorldRelativeVelocityVector(vehicleVelocity);
               }
               particleSystem->Fire();
            }

            if(mAmmoCount > 0)
            {
               mAmmoCount--;
            }
            mShotsFired++;
         }



         switch( effectType )
         {
            case WEAPON_EFFECT_FIRE:
            {
               mFired = true;
               float flashProb = dtUtil::RandFloat(0.0f,1.0f);
               if( mFlash.valid() && mFlashProbability >= flashProb )
               {
                  // Setting visible to TRUE will cause the flash
                  // to restart its age time. The age time will progress
                  // and cause the flash to automatically turn invisible when
                  // it passes its life time, if life time has been set
                  // greater than 0.
                  mFlash->SetVisible( true );
                  mFlash->SetFlashTime( GetFlashTime() );

                  // Randomly roll the flash effect to a different orientation
                  // so that the spawned particles do not spawn at the same orientation
                  // every time a weapon flash is executed. Currently, OSG particles
                  // systems within Delta3D spawn all particles at the same orientation, 0.0;
                  // all particle rotations are derived by age and spin rate starting at
                  // angle 0.0 from time 0.0.
                  dtCore::Transform flashXform;
                  mFlash->GetTransform( flashXform, dtCore::Transformable::REL_CS );
                  flashXform.SetRotation( osg::Vec3(dtUtil::RandFloat(-180.0f,180.0f),0.0,0.0) );
                  mFlash->SetTransform( flashXform, dtCore::Transformable::REL_CS );

                  AddDynamicLight();
               }

               SoundPlay( mSoundFire );
               break;
            }

            case WEAPON_EFFECT_DRY_FIRE:
            {
               SoundPlay( mSoundDryFire );
               break;
            }

            case WEAPON_EFFECT_JAM:
            {
               SoundPlay( mSoundJammed );
               break;
            }

            case WEAPON_EFFECT_BROKEN:
            {
               break;
            }

            default: // Do nothing
               break;
         }

         // Reset current sleep time, the time until this weapon will unregister from TickLocal
         mCurSleepTime = mAutoSleepTime;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         if( ! mMunitionType.valid() && ! mMunitionTypeName.empty() )
         {
            // Attempt a reload
            LoadMunitionType( mMunitionTypeName );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::OnRemovedFromWorld()
      {
         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();

         if (mShooter.valid())
         {
            gm->DeleteActor(*mShooter);
         }

         BaseClass::OnRemovedFromWorld();
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponActor::LoadMunitionType( const std::string& munitionTypeName )
      {
         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();

         if ( gm != NULL )
         {
            // Try to access the munitions component in order
            // to obtain the munition types of shooters
            SimCore::Components::MunitionsComponent* comp;
            gm->GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME, comp);

            if( NULL == comp )
            {
               LOG_ERROR( "WeaponActor could not access the MunitionsComponent." );
               return false;
            }

            SimCore::Components::MunitionTypeTable* table = comp->GetMunitionTypeTable();

            if( NULL == table )
            {
               //this is no longer an error because the map is no longer loaded before the other maps
               //we now log an error if this returns false when we try to lazily get our munitition type
               //LOG_ERROR( "WeaponActor could not access the MunitionsComponent's munition type table." );
               return false;
            }

            // Get a reference to the weapon's munition type
            MunitionTypeActor* munitionType = NULL;

            // Get the munition type from the component by the munition type name
            munitionType = table->GetMunitionType( munitionTypeName );
            if( NULL != munitionType )
            {
               // Assign the accessed munition type
               SetMunitionType( munitionType );
            }
            return NULL != munitionType && mMunitionType.valid();
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetMunitionType( MunitionTypeActor* munitionType )
      {
         mMunitionType = munitionType;

         if( mMunitionType.valid() )
         {
            mMunitionTypeName = mMunitionType->GetName();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetMunitionTypeProxy( dtDAL::ActorProxy* proxy )
      {
         mMunitionType = proxy != NULL ?
            dynamic_cast<MunitionTypeActor*> (proxy->GetActor()) : NULL;
         GetGameActorProxy().SetLinkedActor("Munition Type", proxy);

         if( mMunitionType.valid() )
         {
            mMunitionTypeName = mMunitionType->GetName();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::DeltaDrawable* WeaponActor::GetMunitionTypeDrawable()
      {
         dtDAL::ActorProxy* proxy = GetGameActorProxy().GetLinkedActor("Munition Type");
         return proxy != NULL ? proxy->GetActor() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetOwner( dtDAL::ActorProxy* proxy )
      {
         mOwner = proxy;
         GetGameActorProxy().SetLinkedActor("Owner", proxy);
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::DeltaDrawable* WeaponActor::GetOwner()
      {
         dtDAL::ActorProxy* proxy = GetGameActorProxy().GetLinkedActor("Owner");
         return proxy != NULL ? proxy->GetActor() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetFlashActor( WeaponFlashActor* flashActor )
      {
         mFlash = flashActor;
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponFlashActor* WeaponActor::GetFlashActor()
      {
         return mFlash.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const WeaponFlashActor* WeaponActor::GetFlashActor() const
      {
         return mFlash.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SetFlashActorProxy( dtDAL::ActorProxy* flashProxy )
      {
         mFlash = flashProxy != NULL ?
            dynamic_cast<WeaponFlashActor*> (flashProxy->GetActor()) : NULL;
         GetGameActorProxy().SetLinkedActor("FlashActor", flashProxy );
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::DeltaDrawable* WeaponActor::GetFlashActorDrawable()
      {
         dtDAL::ActorProxy* proxy = GetGameActorProxy().GetLinkedActor("FlashActor");
         return proxy != NULL ? proxy->GetActor() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::ReceiveContactReport(dtPhysics::CollisionContact& report, dtGame::GameActorProxy* target)
      {
         ++mHitCount;

         mLastVelocity.set(report.mNormal);
         mLastVelocity *= mFireVelocity;

         mLastHitLocation = report.mPosition;

         // Get the target ID
         mLastTargetObject = target != NULL ? &target->GetGameActor() : NULL;
         std::string targetID( mLastTargetObject.valid() ?
            mLastTargetObject->GetUniqueId().ToString() : "" );

         // Send a message if a target has changed or the time allows.
         if( mLastTargetID != targetID )
         {
            mTargetChanged = true;
            mLastTargetID = targetID;
         }

         if(!mMunitionType.valid() && !mMunitionTypeName.empty())
         {
            // Attempt a reload
            bool result = LoadMunitionType(mMunitionTypeName);
            if(!result)
            {
               LOG_ERROR( "WeaponActor could not access the MunitionsComponent's munition type table." );
            }
         }

         // Check type to see if it is grenade - Indirect Fire type
         if( ( mMunitionType.valid()
             && (  mMunitionType->GetFamily() == MunitionFamily::FAMILY_GRENADE
                || mMunitionType->GetFamily() == MunitionFamily::FAMILY_EXPLOSIVE_ROUND) )
             || ( mUseBulletPhysics && ( mTargetChanged || mDetMessageTime >= mMessageCycleTime ) ) )
         {
            SendDetonationMessage( mHitCount, mLastVelocity, mLastHitLocation, mLastTargetObject.get() );
            mDetMessageTime = 0.0f;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SendFireMessage( unsigned short quantity,
         const osg::Vec3& initialVelocity, const dtCore::Transformable* target )
      {
         //printf("Sending SHOT FIRED\r\n");

         if(!mMunitionType.valid() && !mMunitionTypeName.empty())
         {
            // Attempt a reload
            bool result = LoadMunitionType(mMunitionTypeName);
            if(!result)
            {
               LOG_ERROR( "WeaponActor could not access the MunitionsComponent's munition type table." );
               return;
            }
         }

         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();

         // Prepare a shot fired message
         dtCore::RefPtr<SimCore::ShotFiredMessage> msg;
         gm->GetMessageFactory().CreateMessage( SimCore::MessageType::SHOT_FIRED, msg );

         // Get the location of this weapon
         dtCore::Transform xform;
         GetTransform( xform );
         osg::Vec3 thisPos;
         xform.GetTranslation(thisPos);

         // Address the message to the targeted entity

         // Required Parameters:
         // --- EventIdentifier
         msg->SetEventIdentifier( mMessageCount++ );
         // --- FiringLocation
         msg->SetFiringLocation( thisPos );
         // --- FiringObjectIdentifier
         msg->SetSendingActorId( mOwner.valid() ? mOwner->GetId() : GetUniqueId() );
         // --- FuseType
         msg->SetFuseType( (unsigned short) mMunitionType->GetFuseType() );
         // --- WarheadType
         msg->SetWarheadType( (unsigned short) mMunitionType->GetWarheadType() );
         // --- MunitionType
         msg->SetMunitionType( mMunitionType->GetName() );
         // --- InitialVelocityVector
         msg->SetInitialVelocityVector( initialVelocity );

         // Optional Parameters:
         // --- FireControlSolutionRange
         // ???
         // --- FireMunitionIndex
         // ???
         // --- MunitionObjectIdentifier
         // ? for Missiles ?
         // --- QuantityFired
         msg->SetQuantityFired( quantity < 1 ? 1 : quantity );
         // --- RateOfFire (rounds per minute)
         unsigned rate = mFireRate <= 0.0f ? 0 : (unsigned)(60.0f / mFireRate + 0.5f);
         msg->SetRateOfFire( rate < 1 ? 1 : rate );
         // --- TargetObjectIdentifier - for Direct Fire
         if( target != NULL ) { msg->SetAboutActorId( target->GetUniqueId() ); }

         gm->SendMessage(*msg);
         gm->SendNetworkMessage(*msg);
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SendDetonationMessage( unsigned short quantity,
         const osg::Vec3& finalVelocity, const osg::Vec3& location, const dtCore::Transformable* target )
      {
         //printf("Sending DETONATION\r\n");

         if(!mMunitionType.valid() && !mMunitionTypeName.empty())
         {
            // Attempt a reload
            bool result = LoadMunitionType(mMunitionTypeName);
            if(!result)
            {
               LOG_ERROR( "WeaponActor could not access the MunitionsComponent's munition type table." );
               return;
            }
         }

         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();

         // Prepare a detonation message
         dtCore::RefPtr<SimCore::DetonationMessage> msg;
         gm->GetMessageFactory().CreateMessage(SimCore::MessageType::DETONATION, msg);

         // Required Parameters:
         // --- EventIdentifier
         msg->SetEventIdentifier( mMessageCount++ );
         // --- DetonationLocation
         msg->SetDetonationLocation( location );
         // --- DetonationResultCode
            // 1 == Entity Impact
            // 3 == Ground Impact
            // 5 == Detonation
            msg->SetDetonationResultCode( target != NULL ? 1 : 3 ); // TO BE DYNAMIC
         // --- MunitionType
         msg->SetMunitionType( mMunitionType->GetName() );
         // --- FuseType
         msg->SetFuseType( (unsigned short) mMunitionType->GetFuseType() );
         // --- WarheadType
         msg->SetWarheadType( (unsigned short) mMunitionType->GetWarheadType() );
         // --- QuantityFired - number of rounds in a burst (ICM munitions)
         msg->SetQuantityFired( quantity );
         // FiringObjectIdentifier
         msg->SetSendingActorId( mOwner.valid() ? mOwner->GetId() : GetUniqueId() );

         // Direct Fire Parameters:
         if( target != NULL )
         {
            // TargetObjectIdentifier
            msg->SetAboutActorId( target->GetUniqueId() );
            // RelativeDetonationLocation
            dtCore::Transform xform;
            target->GetTransform( xform );
            osg::Vec3 targetTrans;
            xform.GetTranslation(targetTrans);
            msg->SetRelativeDetonationLocation( location - targetTrans);
         }

         // Optional Parameters:
         // FinalVelocityVector
         msg->SetFinalVelocityVector( finalVelocity );
         // MunitionObjectIdentifier
         // ? for Missiles ?
         // RateOfFire (rounds per minute)
         unsigned rate = mFireRate <= 0.0f ? 0 : (unsigned)(60.0f / mFireRate + 0.5f);
         msg->SetRateOfFire( rate < 1 ? 1 : rate );

         gm->SendMessage( *msg );
         gm->SendNetworkMessage( *msg );
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponActor::AttachObject( dtCore::Transformable& object, const std::string& dofName )
      {
         return AddChild(&object, dofName);
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::LoadSoundFire( const std::string& filePath )
      {
         SoundLoad( filePath, mSoundFire );

         if( ! mSoundFire.valid() ) { return; }

         mSoundFire->SetListenerRelative(false);
         AddChild(mSoundFire.get());
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::LoadSoundDryFire( const std::string& filePath )
      {
         SoundLoad( filePath, mSoundDryFire );

         if( ! mSoundDryFire.valid() ) { return; }

         mSoundDryFire->SetListenerRelative(true);
         AddChild( mSoundDryFire.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::LoadSoundJammed( const std::string& filePath )
      {
         SoundLoad( filePath, mSoundJammed );

         if( ! mSoundJammed.valid() ) { return; }

         mSoundJammed->SetListenerRelative(true);
         AddChild(mSoundJammed.get());
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SoundLoad( const std::string& filePath, dtCore::RefPtr<dtAudio::Sound>& sound )
      {
         if(sound != NULL && sound->GetFilename() != NULL)
         {
            dtAudio::AudioManager::GetInstance().FreeSound(sound.get());
            sound->UnloadFile();
         }

         sound = NULL;

         if (!filePath.empty())
         {
            sound = dtAudio::AudioManager::GetInstance().NewSound();

            if( ! sound.valid())
            {
               throw dtGame::InvalidParameterException(
                        "Failed to create the sound object", __FILE__, __LINE__);
            }

            sound->LoadFile(filePath.c_str());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SoundPlay( dtCore::RefPtr<dtAudio::Sound>& sound )
      {
         if( sound.valid() && ! sound->IsPlaying() ) { sound->Play(); }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActor::SoundRelease( dtCore::RefPtr<dtAudio::Sound>& sound )
      {
         if( sound.valid() )
         {
            if( sound->IsPlaying() ) { sound->Stop(); }

            RemoveChild( sound.get() );
            dtAudio::AudioManager::GetInstance().FreeSound(sound.get());
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////////////////////
      WeaponActorProxy::WeaponActorProxy()
      {
         SetClassName("SimCore::Actors::WeaponActor");
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponActorProxy::~WeaponActorProxy()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorProxy::CreateActor()
      {
         WeaponActor& actor = *new WeaponActor(*this);
         SetActor(actor);
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorProxy::BuildPropertyMap()
      {
         PlatformActorProxy::BuildPropertyMap();

         WeaponActor* actor = NULL;
         GetActor(actor);

         static const dtUtil::RefString groupResources("Resources");
         static const dtUtil::RefString groupMunitions("Munitions");
         static const dtUtil::RefString groupBehaviors("Behaviors");
         static const dtUtil::RefString groupStatus("Status");
         static const dtUtil::RefString groupActors("Associate Actors");

         // --- RESOURCE PROPERTIES --- //

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "Fire Sound", "Fire Sound",
            dtDAL::ResourceActorProperty::SetFuncType(actor, &WeaponActor::LoadSoundFire),
            "The sound produced when this weapon fires.",
            groupResources));

         // --- PRIMITIVE PROPERTIES --- //

         AddProperty(new dtDAL::BooleanActorProperty("Use Bullet Physics","Use Bullet Physics",
            dtDAL::BooleanActorProperty::SetFuncType( actor, &WeaponActor::SetUsingBulletPhysics),
            dtDAL::BooleanActorProperty::GetFuncType( actor, &WeaponActor::IsUsingBulletPhysics),
            "Enables the weapon to send detonation messages for rounds when they impact.",
            groupStatus));

         AddProperty(new dtDAL::BooleanActorProperty("Trigger Held","Trigger Held",
            dtDAL::BooleanActorProperty::SetFuncType( actor, &WeaponActor::SetTriggerHeld),
            dtDAL::BooleanActorProperty::GetFuncType( actor, &WeaponActor::IsTriggerHeld),
            "Sets the trigger's state to held or not held.",
            groupStatus));

         AddProperty(new dtDAL::BooleanActorProperty("Jammed","Jammed",
            dtDAL::BooleanActorProperty::SetFuncType( actor, &WeaponActor::SetJammed),
            dtDAL::BooleanActorProperty::GetFuncType( actor, &WeaponActor::IsJammed),
            "Sets the weapon's state to jammed/unjammed",
            groupStatus));

         AddProperty(new dtDAL::FloatActorProperty("Recoil Distance","Recoil Distance",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetRecoilDistance),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetRecoilDistance),
            "The distance in meters that the weapon jumps backward along the line of fire.",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Recoil Rest Time","Recoil Rest Time",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetRecoilRestTime),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetRecoilRestTime),
            "The time in seconds that it take the weapon to return to it resting position from recoil",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Auto Sleep Time","Auto Sleep Time",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetAutoSleepTime),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetAutoSleepTime),
            "The time in seconds from the last fired shot to the instant the weapon will unregister from TickLocal.",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Fire Velocity", "Fire Velocity",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetFireVelocity ),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetFireVelocity ),
            "The average scalar velocity of munitions leaving this weapon.",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Fire Rate","Fire Rate",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetFireRate),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetFireRate),
            "Number of rounds that can be fired per second; fractional numbers can be used for odd or slow re-fire cycles.",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Jam Probability","Jam Probability",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetJamProbability),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetJamProbability),
            "Probability from 0.0 to 1.0 that this weapon will jam when firing a round.",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Flash Probability","Flash Probability",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetFlashProbability),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetFlashProbability),
            "Probability from 0.0 to 1.0 that this weapon will produce a flash effect when firing a round.",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Flash Time","Flash Time",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetFlashTime),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetFlashTime),
            "time in seconds that a weapon flash will remain visible.",
            groupBehaviors));

         AddProperty(new dtDAL::FloatActorProperty("Time Between Messages","Time Between Messages",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponActor::SetTimeBetweenMessages),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponActor::GetTimeBetweenMessages),
            "The minimum amount of time allowed between fire and detonation messages.",
            groupBehaviors));

         AddProperty(new dtDAL::IntActorProperty( "Tracer Frequency", "Tracer Frequency",
            dtDAL::IntActorProperty::SetFuncType( actor, &WeaponActor::SetTracerFrequency),
            dtDAL::IntActorProperty::GetFuncType( actor, &WeaponActor::GetTracerFrequency),
            "The number of rounds that must be fired to reveal one tracer round.",
            groupBehaviors));

         AddProperty(new dtDAL::IntActorProperty( "Ammo Count", "Ammo Count",
            dtDAL::IntActorProperty::SetFuncType( actor, &WeaponActor::SetAmmoCount),
            dtDAL::IntActorProperty::GetFuncType( actor, &WeaponActor::GetAmmoCount),
            "The current count of ammo this weapon is holding (this clamped by Ammo Max).",
            groupStatus));

         AddProperty(new dtDAL::IntActorProperty( "Ammo Max", "Ammo Max",
            dtDAL::IntActorProperty::SetFuncType( actor, &WeaponActor::SetAmmoMax),
            dtDAL::IntActorProperty::GetFuncType( actor, &WeaponActor::GetAmmoMax),
            "The maximum count of ammo this weapon can hold.",
            groupStatus));

         AddProperty(new dtDAL::StringActorProperty( "Munition Type Name", "Munition Type Name",
            dtDAL::StringActorProperty::SetFuncType( actor, &WeaponActor::SetMunitionTypeName),
            dtDAL::StringActorProperty::GetFuncType( actor, &WeaponActor::GetMunitionTypeName),
            "Set the munition type name of the MunitionTypeActor to be loaded from the MunitionsComponent.",
            groupMunitions));

         // --- ACTOR PROPERTIES --- //

         AddProperty(new dtDAL::ActorActorProperty( *this, "Munition Type", "Munition Type",
            dtDAL::ActorActorProperty::SetFuncType( actor, &WeaponActor::SetMunitionTypeProxy ),
            dtDAL::ActorActorProperty::GetFuncType( actor, &WeaponActor::GetMunitionTypeDrawable ),
            MunitionTypeActorProxy::CLASS_NAME,
            "A reference to the MunitionTypeActor that will have data related to the munition this weapon will fire.",
            groupMunitions));

         AddProperty(new dtDAL::ActorActorProperty( *this, "Owner", "Owner",
            dtDAL::ActorActorProperty::SetFuncType( actor, &WeaponActor::SetOwner ),
            dtDAL::ActorActorProperty::GetFuncType( actor, &WeaponActor::GetOwner ),
            "", // anything might be able to own a weapon
            "A reference to the owning actor that is published on the network and that will need to send its ID in weapon fire messages.",
            groupActors));

         AddProperty(new dtDAL::ActorActorProperty( *this, "Flash Actor", "Flash Actor",
            dtDAL::ActorActorProperty::SetFuncType( actor, &WeaponActor::SetFlashActorProxy ),
            dtDAL::ActorActorProperty::GetFuncType( actor, &WeaponActor::GetFlashActorDrawable ),
            WeaponFlashActorProxy::CLASS_NAME,
            "A reference to the flash actor responsible for timing and rendering flash effects.",
            groupActors));
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorProxy::OnEnteredWorld()
      {
         PlatformActorProxy::OnEnteredWorld();
      }

   }
}
