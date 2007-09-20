/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */
#include <prefix/SimCorePrefix-src.h>

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
// DELTA 3D
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtCore/camera.h>
#include <dtCore/transformable.h>
#include <dtCore/scene.h>
#include <dtDAL/project.h>
#include <dtDAL/map.h>
#include <dtGame/basemessages.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/gamemanager.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/matrixutil.h>

// SIM Core
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
// Components
#include <SimCore/Components/DamageHelper.h>
#include <SimCore/Components/MunitionDamage.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/MunitionsConfig.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/ViewerMaterialComponent.h>
// Actors
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Munition Type Table Code
      //////////////////////////////////////////////////////////////////////////
      MunitionTypeTable::MunitionTypeTable()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionTypeTable::~MunitionTypeTable()
      {
         Clear();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionTypeTable::GetCount() const
      {
         return mNameToMunitionMap.size();
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionTypeTable::AddMunitionType( const dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy>& newType )
      {
         if( ! newType.valid() ) { return false; }

         std::string munitionName( newType->GetName() );
         if( HasMunitionType( munitionName ) ) { return false; }

         SimCore::Actors::MunitionTypeActor* actor = 
            dynamic_cast<SimCore::Actors::MunitionTypeActor*>(newType->GetActor());

         if( actor == NULL )
         {
            std::stringstream ss;
            ss << "MunitionType \""
               << munitionName.c_str()
               <<"\" does not have a valid actor."
               << std::endl;
            LOG_ERROR( ss.str() );
            return false;
         }

         // Ensure the actor has the same name
         actor->SetName( munitionName );

         bool success = mNameToMunitionMap.insert( 
            std::make_pair( munitionName, actor ) 
            ).second;

         if( !success )
         {
            std::stringstream ss;
            ss << "Failure adding MunitionType "
               << munitionName.c_str()
               <<". MunitionType may have already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
         }
         else
         {
            InsertMunitionTypeToOrderedList( *actor );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionTypeTable::RemoveMunitionType( const std::string& name )
      {
         std::map< std::string, dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >::iterator iter = 
            mNameToMunitionMap.find( name );

         if( iter != mNameToMunitionMap.end() )
         {
            RemoveMunitionTypeFromOrderedList( *(iter->second) );
            mNameToMunitionMap.erase( iter );
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionTypeTable::HasMunitionType( const std::string& name ) const
      {
         return mNameToMunitionMap.find( name ) != mNameToMunitionMap.end();
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionType( const std::string& name )
      {
         std::map< std::string, dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >::const_iterator iter = 
            mNameToMunitionMap.find( name );

         return iter != mNameToMunitionMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionType( const std::string& name ) const
      {
         std::map< std::string, dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >::const_iterator iter = 
            mNameToMunitionMap.find( name );

         return iter != mNameToMunitionMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionTypeByDIS( 
         const std::string& dis, bool exactMatch ) const
      {
         // Convert the string to a comparable DIS object
         SimCore::Actors::DISIdentifier matchDis;
         matchDis.SetByString( dis );
         return GetMunitionTypeByDIS( matchDis, exactMatch );
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionTypeByDIS( 
         const SimCore::Actors::DISIdentifier& dis, bool exactMatch ) const
      {
         // Define variables to be used in the loop
         const SimCore::Actors::MunitionTypeActor* closestType = NULL;
         unsigned int matchLevel = 0;
         unsigned int curMatchLevel = 0;
         std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >::const_iterator iter 
            = mOrderedList.begin();

         // Iterate through DIS levels
         for( ; iter != mOrderedList.end(); ++iter )
         {
            // Match current level
            curMatchLevel = dis.GetDegreeOfMatch( (*iter)->GetDISIdentifier() );

            // Return immediately if this is a full match
            if( curMatchLevel == 7 ) { return iter->get(); }

            // Capture the first closest match
            if( matchLevel <= curMatchLevel ) // "=" has been put in for a just-in-case situation, though it should not be necessary
            {
               matchLevel = curMatchLevel; // "raise the bar" for matching

               // Only capture a close match if the current munition's DIS
               // has a zero for the next
               if( (*iter)->GetDISIdentifier().GetNumber( matchLevel ) == 0 )
               {
                  closestType = iter->get();
               }
            }
            // If match level is decreasing, no further checks will produce a higher match.
            // NOTE: This is under heavy assumption that the list is properly sorted.
            else if( matchLevel > curMatchLevel )
            {
               break;
            }
         }

         // If an exact match is requested and the match level is not 7
         // (there are 7 numbers in a DIS identifier), then NULL should be returned.
         if( closestType == NULL || ( exactMatch && matchLevel < 7 ) )
         {
            std::ostringstream ss;
            ss << "Could not find a munition that matches DIS " << dis.ToString();
            LOG_WARNING( ss.str() );
            return NULL;
         }

         return closestType;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeTable::Clear()
      {
         mNameToMunitionMap.clear();
         mOrderedList.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeTable::InsertMunitionTypeToOrderedList( SimCore::Actors::MunitionTypeActor& newType )
      {
         if( mOrderedList.empty() )
         {
            mOrderedList.push_back( &newType );
            return;
         }

         bool inserted = false;
         std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >::iterator iter = mOrderedList.begin();
         const SimCore::Actors::DISIdentifier& dis = newType.GetDISIdentifier();
         for( ; iter != mOrderedList.end(); ++iter )
         {
            if( (*iter)->GetDISIdentifier() >= dis )
            {
               mOrderedList.insert(iter,&newType);
               inserted = true;
               return;
            }
         }

         if( ! inserted )
         {
            mOrderedList.push_back( &newType );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeTable::RemoveMunitionTypeFromOrderedList( const SimCore::Actors::MunitionTypeActor& oldType )
      {
         std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >::iterator iter = mOrderedList.begin();
         const SimCore::Actors::DISIdentifier& dis = oldType.GetDISIdentifier();
         const std::string& name = oldType.GetName();
         for( ; iter != mOrderedList.end(); ++iter )
         {
            if( (*iter)->GetName() == name && (*iter)->GetDISIdentifier() == dis )
            {
               mOrderedList.erase(iter);
               return;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionTypeTable::GetOrderedListSize() const
      {
         return mOrderedList.size();
      }



      //////////////////////////////////////////////////////////////////////////
      // Munition Damage Table Code
      //////////////////////////////////////////////////////////////////////////
      MunitionDamageTable::MunitionDamageTable( const std::string& name )
         : dtCore::Base(name)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionDamageTable::~MunitionDamageTable()
      {
         Clear();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionDamageTable::GetCount() const
      {
         return mNameToMunitionMap.size();
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionDamageTable::AddMunitionDamage( const dtCore::RefPtr<MunitionDamage>& newInfo )
      {
         if( ! newInfo.valid() ) { return false; }
         if( HasMunitionDamage( newInfo->GetName() ) ) { return false; }

         bool success = mNameToMunitionMap.insert( 
            std::make_pair( newInfo->GetName(), newInfo.get() ) 
            ).second;

         if( !success )
         {
            std::stringstream ss;
            ss << "Failure adding MunitionDamage "
               << newInfo->GetName().c_str()
               <<". MunitionDamage may have already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionDamageTable::RemoveMunitionDamage( const std::string& name )
      {
         std::map< std::string, dtCore::RefPtr<MunitionDamage> >::iterator iter = 
            mNameToMunitionMap.find( name );

         if( iter != mNameToMunitionMap.end() )
         {
            mNameToMunitionMap.erase( iter );
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionDamageTable::HasMunitionDamage( const std::string& name ) const
      {
         return mNameToMunitionMap.find( name ) != mNameToMunitionMap.end();
      }

      //////////////////////////////////////////////////////////////////////////
      const MunitionDamage* MunitionDamageTable::GetMunitionDamage( const std::string& name ) const
      {
         std::map< std::string, dtCore::RefPtr<MunitionDamage> >::const_iterator iter = 
            mNameToMunitionMap.find( name );

         return iter != mNameToMunitionMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionDamageTable::Clear()
      {
         mNameToMunitionMap.clear();
      }


      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Code
      //////////////////////////////////////////////////////////////////////////
      TracerEffect::TracerEffect( float lineLength, float lineThickness,
         const std::string& shaderName, const std::string& shaderGroup )
         : SimCore::Actors::VolumetricLine(lineLength,lineThickness,shaderName,shaderGroup),
         mHasLight(false),
         mDynamicLightEnabled(false),
         mDynamicLightID(0),
         mLifeTime(0.0f),
         mMaxLifeTime(1.0f),
         mSpeed(0.0f),
         mMaxLength(10.0f)
      {
         SetVisible( true ); // ensures node mask is set to the proper value.
      }

      //////////////////////////////////////////////////////////////////////////
      TracerEffect::~TracerEffect()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::SetDirection( const osg::Vec3& direction )
      {
         mDirection = direction;
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::SetVelocity( const osg::Vec3& velocity )
      {
         if( velocity.length2() > 0.0 )
         { 
            mSpeed = velocity.length();
            // If an external munition doesn't send the correct velocity vector, then we 
            // just change it to be some sort of minimum. Otherwise, the tracer moves at like
            // 1 m/s which is awful.
            if (mSpeed < 5)
               mSpeed = 100;
            mDirection = velocity; 
            mDirection.normalize();
         }
         else
         {
            mSpeed = 0.0;
            mDirection.set(0.0,0.0,0.0);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Vec3 TracerEffect::GetVelocity() const
      {
         return mDirection * mSpeed;
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::GetVelocity( osg::Vec3& outVelocity ) const
      {
         outVelocity.set(mDirection);
         outVelocity *= mSpeed;
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::SetPosition( const osg::Vec3& position )
      {
         // Set the last position...
         if( ! IsActive() )
         {
            // ...to the specified point if this effect is about to be executed again
            mLastPosition.set(position);
         }
         else
         {
            // ...to remember the last point
            osg::Vec3 diff(mPosition-position);
            if( diff.length() >= mMaxLength )
            {
               diff.normalize();
               mLastPosition.set( diff * mMaxLength );
            }
         }

         mPosition.set(position);

         dtCore::Transform xform;
         GetTransform(xform,dtCore::Transformable::REL_CS);
         xform.SetTranslation(position);
         SetTransform(xform,dtCore::Transformable::REL_CS);
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::SetVisible( bool visible )
      {
         GetOSGNode()->setNodeMask(visible?0xFFFFFFFF:0);
         if( visible && mHasLight )
         {
            // TODO: Make tracer colors dynamic.
            AddDynamicLight(osg::Vec3(1.0f, 0.2f, 0.2f)); // default color to red
         }
         else
         {
            RemoveDynamicLight();
         }
      }
      
      //////////////////////////////////////////////////////////////////////////
      bool TracerEffect::IsVisible() const
      {
         return 0 != GetOSGNode()->getNodeMask();
      }

      //////////////////////////////////////////////////////////////////////////
      bool TracerEffect::IsActive() const
      {
         return mLifeTime < mMaxLifeTime;
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::Execute( float maxLifeTime )
      {
         SetMaxLifeTime( maxLifeTime );
         mLifeTime = 0.0f;
         SetVisible( true );

         // Start the tracer line with zero stretch
         mLastPosition = mPosition;
         SetLength(0.001f);

         // Physically adjust orientation to match direction
         dtCore::Transform xform;
         GetTransform(xform,dtCore::Transformable::REL_CS);
         osg::Matrix mtx;
         osg::Vec3 right = mDirection ^ osg::Vec3(0.0,0.0,1.0);
         right.normalize();
         osg::Vec3 up = right ^ mDirection;
         up.normalize();
         mtx.ptr()[0] = right[0];
         mtx.ptr()[1] = right[1];
         mtx.ptr()[2] = right[2];
         mtx.ptr()[4] = mDirection[0];
         mtx.ptr()[5] = mDirection[1];
         mtx.ptr()[6] = mDirection[2];
         mtx.ptr()[8] = up[0];
         mtx.ptr()[9] = up[1];
         mtx.ptr()[10] = up[2];
         xform.SetRotation(mtx);
         SetTransform(xform,dtCore::Transformable::REL_CS);
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::Update( float deltaTime )
      {
         if( IsActive() )
         {
            SetPosition( mPosition + (mDirection * mSpeed * deltaTime) );
            mLifeTime += deltaTime;

            // Update the tracer length for stretching
            float length = (mLastPosition-mPosition).length();
            if( length > mMaxLength )
            {
               SetLength( mMaxLength );
            }
            else if( length > 0.0 && length < mMaxLength )
            {
               SetLength( length );
            }
         }
         else if( IsVisible() )
         {
            SetVisible( false );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::AddDynamicLight( const osg::Vec3& color )
      {
         if( ! mGM.valid() )
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp
            = dynamic_cast<SimCore::Components::RenderingSupportComponent*>
            (mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME));

         if( renderComp != NULL )
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = NULL;
            if( ! mDynamicLightEnabled )
            {
               dl = renderComp->AddDynamicLightByPrototypeName("Light-Tracer");
               dl->mTarget = this;
               dl->mColor = color;

               mDynamicLightID = dl->mID;
               mDynamicLightEnabled = true;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::RemoveDynamicLight()
      {
         if( ! mGM.valid() )
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp
            = dynamic_cast<SimCore::Components::RenderingSupportComponent*>
            (mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME));

         if( renderComp != NULL )
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = renderComp->GetDynamicLight(mDynamicLightID);
            if( dl != NULL && mDynamicLightEnabled )
            {
               dl->mIntensity = 0.0f;
               renderComp->RemoveDynamicLight(mDynamicLightID);

               mDynamicLightEnabled = false;
               mDynamicLightID = 0;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::SetGameManager( dtGame::GameManager* gameManager )
      {
         mGM = gameManager;
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::SetMaxLength( float maxLength )
      {
         mMaxLength = maxLength;
      }

      //////////////////////////////////////////////////////////////////////////
      float TracerEffect::GetMaxLength() const
      {
         return mMaxLength;
      }



      //////////////////////////////////////////////////////////////////////////
      // Weapon Effect Code
      //////////////////////////////////////////////////////////////////////////
      const std::string WeaponEffect::CLASS_NAME("WeaponEffect");

      //////////////////////////////////////////////////////////////////////////
      WeaponEffect::WeaponEffect()
         : dtCore::Transformable(WeaponEffect::CLASS_NAME),
         mDynamicLightID(0),
         mDynamicLightEnabled(false),
         mVisible(true),
         mSoundPlayed(false),
         mSoundStartTime(0.0f),
         mFlashProbability(1.0f),
         mFlashTime(0.0f),
         mTimeSinceFlash(0.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponEffect::~WeaponEffect()
      {
         // NOTE: pass in FALSE if sound clean up should be the responsibility
         // of a higher level object.
         Clear();
      }

      ///////////////////////////////////////////////////////////////////////
      float WeaponEffect::CalculateSoundDelayTime( const osg::Vec3& sourcePosition, const osg::Vec3& listenerPosition )
      {
         return (listenerPosition - sourcePosition).length() / 350.0f;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::Execute( float flashTime, const osg::Vec3& listenerPosition )
      {
         SetFlashTime( flashTime );

         mSoundPlayed = false;
         mSoundStartTime = 0.0f;
         if( mSound.valid() )
         {
            dtCore::Transform xform;
            mSound->GetTransform( xform );

            mSoundStartTime = CalculateSoundDelayTime( xform.GetTranslation(), listenerPosition );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::SetFlashTime( float flashTime )
      {
         mFlashTime = flashTime;

         if( mFlash.valid() && mFlashTime > 0.0f )
         {
            if( mFlashProbability > 0.0f && dtUtil::RandFloat( 0.0f, 1.0f ) < mFlashProbability )
            {
               SetVisible( true );
            }
         }

         mTimeSinceFlash = 0.0f;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::SetSound( dtAudio::Sound* sound, bool releaseOldSound )
      {
         if( mSound.get() == sound ) { return; }

         if( releaseOldSound && mSound.valid() )
         {
            if( mSound->IsPlaying() ) { mSound->Stop(); }

            RemoveChild( mSound.get() );
            dtAudio::Sound* soundPointer = mSound.release();
            if( soundPointer != NULL )
            {
//               dtAudio::AudioManager::GetInstance().FreeSound( soundPointer );
            }
         }
         mSound = sound;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::SetVisible( bool visible )
      {
         mVisible = visible;
         if( mFlash.valid() )
         {
            mFlash->GetOSGNode()->setNodeMask( mVisible ? 0xFFFFFFFF : 0 );

            if( mVisible )
            {
               // TODO: Make tracer colors dynamic.
               AddDynamicLight(osg::Vec3(1.0f, 0.5f, 0.0f)); // yellow
            }
            else
            {
               RemoveDynamicLight();
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::Update( float timeDelta )
      {
         if( mFlashTime > 0.0f ) { mFlashTime -= timeDelta; }

         if( mVisible && mFlashTime <= 0.0f )
         {
            SetVisible( false );
         }

         mTimeSinceFlash += timeDelta;

         if( ! mSoundPlayed && mSoundStartTime <= mTimeSinceFlash )
         {
            mSoundPlayed = true;
            if( mSound.valid() && ! mSound->IsPlaying() )
            {
               mSound->Play();
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::SetFlash( dtCore::ParticleSystem* flash )
      {
         if( mFlash.valid() ) { mFlash->SetParent( NULL ); }

         mFlash = flash;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::Attach( osgSim::DOFTransform* dof )
      {
         if( ! mOwner.valid() ) { return false; }

         // Remove this from the owner in case it is already attached
         mOwner->RemoveChild( this );

         bool success = false;

         success = mOwner->AddChild( this );

         if( success )
         {
            mDOF = dof;

            if( mDOF.valid() )
            {
               // Set the parent of the flash explicitly.
               // This will be used like a boolean to determine if the flash has
               // already been attached to the DOF.
               if( mFlash.valid() && mFlash->GetParent() == NULL )
               {
                  mFlash->SetParent( this );
                  mDOF->addChild( mFlash->GetOSGNode() );
               }
               if( mSound.valid() && mSound->GetParent() == NULL )
               {
                  mSound->SetParent( this );
                  mDOF->addChild( mSound->GetOSGNode() );
               }
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::Detach()
      {
         if( mDOF.valid() )
         {
            if( mFlash.valid() && mFlash->GetParent() != NULL )
            {
               mDOF->removeChild( mFlash->GetOSGNode() );
               mFlash->SetParent( NULL );
            }
            if( mSound.valid() && mSound->GetParent() != NULL )
            {
               mDOF->removeChild( mSound->GetOSGNode() );
               mSound->SetParent( NULL );
            }
         }
         mDOF = NULL;

         if( ! mOwner.valid() ) { return false; }

         mOwner->RemoveChild( this );
         mOwner = NULL;

         return true;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::Clear( bool releaseOldSound )
      {
         Detach(); // sets the DOF and owner to NULL
         mFlash = NULL;
         SetSound( NULL, releaseOldSound );
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::LoadSound( const std::string& filePath )
      {
         mSound = NULL;
         mSound = dtAudio::AudioManager::GetInstance().NewSound();

         if( ! mSound.valid())
            throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
            "Failed to create the sound object for WeaponEffect", __FILE__, __LINE__);

         mSound->LoadFile(filePath.c_str());

         return mSound.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::LoadFlash( const std::string& filePath )
      {
         dtCore::RefPtr<dtCore::ParticleSystem> flash = new dtCore::ParticleSystem;
         flash->SetParentRelative(true);

         if( ! flash.valid())
            throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
            "Failed to create the particles object for WeaponEffect", __FILE__, __LINE__);

         flash->LoadFile(filePath.c_str());

         SetFlash( flash.get() );

         return mFlash.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::AddDynamicLight( const osg::Vec3& color )
      {
         if( ! mGM.valid() )
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp
            = dynamic_cast<SimCore::Components::RenderingSupportComponent*>
            (mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME));

         if( renderComp != NULL )
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = NULL;
            if( ! mDynamicLightEnabled )
            {
               //dl = new SimCore::Components::RenderingSupportComponent::DynamicLight();
               dl = renderComp->AddDynamicLightByPrototypeName("Light-Tracer");
               dl->mColor = color;//a bright yellow
               //dl->mAttenuation.set(0.1, 0.05, 0.0002);
               //dl->mIntensity = 1.0f;
               //dl->mSaturationIntensity = 0.0f; //no saturation
               dl->mTarget = mFlash.get();

               //mDynamicLightID = renderComp->AddDynamicLight(dl);
               mDynamicLightID = dl->mID;
               mDynamicLightEnabled = true;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::RemoveDynamicLight()
      {
         if( ! mGM.valid() )
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp
            = dynamic_cast<SimCore::Components::RenderingSupportComponent*>
            (mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME));

         if( renderComp != NULL )
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = renderComp->GetDynamicLight(mDynamicLightID);
            if( dl != NULL && mDynamicLightEnabled )
            {
               dl->mIntensity = 0.0f;
               renderComp->RemoveDynamicLight(mDynamicLightID);

               mDynamicLightEnabled = false;
               mDynamicLightID = 0;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::SetGameManager( dtGame::GameManager* gameManager )
      {
         mGM = gameManager;
      }



      //////////////////////////////////////////////////////////////////////////
      // Weapon Effect Manager Code
      //////////////////////////////////////////////////////////////////////////
      WeaponEffectsManager::WeaponEffectsManager()
         : dtCore::Base("WeaponEffectsManager"),
         mEffectTimeMax(5.0f),
         mRecycleTime(1.0f),
         mCurRecycleTime(0.0f),
         mMaxWeaponEffects(-1), // no limit
         mMaxTracerEffects(-1), // no limit
         mIsector(new dtCore::BatchIsector)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponEffectsManager::~WeaponEffectsManager()
      {
         Clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::SetGameManager( dtGame::GameManager* gameManager )
      {
         if( gameManager == NULL )
         {
            mIsector->Reset();
            mIsector->SetScene( NULL );
         }

         mGM = gameManager;

         if( mGM.valid() )
         {
            mIsector->SetScene( &mGM->GetScene() );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffectsManager::ApplyWeaponEffect( SimCore::Actors::BaseEntity& owner, 
         osgSim::DOFTransform* ownerDOF, 
         const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo, const osg::Vec3& listenerLocation )
      {
         // If a limit on effects has been set, avoid adding more if the
         // effects list is at or over the limit.
         if( mMaxWeaponEffects > -1 && mMaxWeaponEffects <= (int)mEntityToEffectMap.size() ) { return false; }

         // Use a key to find or insert an entry in the effects mapping.
         const std::string& objectId = owner.GetUniqueId().ToString();

         // Determine if an effect has already been assigned to the specified entity.
         std::map<std::string, dtCore::RefPtr<WeaponEffect> >::iterator foundEffectIter
            = mEntityToEffectMap.find( objectId );
         bool foundEffect = foundEffectIter != mEntityToEffectMap.end();

         // Ready a new effect
         dtCore::RefPtr<WeaponEffect> newEffect = ! foundEffect 
            ? new WeaponEffect : foundEffectIter->second.get();

         // Set the other effect data if this is a new weapon effect.
         if( ! foundEffect )
         {
            newEffect->SetOwner( &owner );
            newEffect->SetGameManager( mGM.get() );

            if( ! effectsInfo.GetFireEffect().empty() )
            {
               if( newEffect->LoadFlash( effectsInfo.GetFireEffect() ) )
               {
                  newEffect->SetFlashProbability( effectsInfo.GetFireFlashProbability() );
               }
            }
            if( ! effectsInfo.GetFireSound().empty() )
            {
               if( newEffect->LoadSound( effectsInfo.GetFireSound() ) )
               {
                  dtAudio::Sound* sound = newEffect->GetSound();
                  sound->ListenerRelative(true);
                  sound->SetMinDistance( effectsInfo.GetFireSoundMinDistance() );
                  sound->SetMaxDistance( effectsInfo.GetFireSoundMaxDistance() );
               }
            }

            // Put new effect into list before attaching it, in case addition fails.
            mEntityToEffectMap.insert( std::make_pair( objectId, newEffect.get() ) );
         }

         bool success = newEffect->Attach( ownerDOF );

         // --- DEBUG --- START --- //
         //std::cout << "\nWeaponEffectsManager " << (foundEffect?"Inserting":"Updating") 
         //   << "\n\tWeaponEffect: " << newEffect->GetUniqueId().ToString() 
         //   << " (Attach " << (success?"Success":"Failure") << ")" << std::endl;
         // --- DEBUG --- END --- //

         if( success )
         {
            newEffect->Execute( effectsInfo.GetFireFlashTime(), listenerLocation );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffectsManager::ApplyTracerEffect(
         const osg::Vec3& weaponFirePoint,
         const osg::Vec3& intialVelocity,
         const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo,
         bool useLight )
      {
         if( ! mGM.valid() 
            || (mMaxTracerEffects > -1 && (int)mTracerEffects.size() >= mMaxTracerEffects) )
         {
            return false;
         }

         // Find a recyclable effect.
         TracerEffect* effect = NULL;
         unsigned limit = mTracerEffects.size();
         for( unsigned i = 0; i < limit; ++i )
         {
            if( mTracerEffects[i].valid() && ! mTracerEffects[i]->IsActive() )
            {
               effect = mTracerEffects[i].get();
            }
         }

         if( effect == NULL
            && ( mMaxTracerEffects < 0 || (int)limit <= mMaxTracerEffects ) )
         {
            effect = new TracerEffect( 
               0.001, // spawn tracer at a near-zero size; it will be stretched to max size during updates.
               effectsInfo.GetTracerThickness(),
               effectsInfo.GetTracerShaderName(),
               effectsInfo.GetTracerShaderGroup() );
            effect->SetMaxLength( effectsInfo.GetTracerLength() );
            effect->SetGameManager( mGM.get() );

            // Make sure the tracer is visible in the scene
            mGM->GetScene().AddDrawable( effect );

            // Track this tracer effect though out the life of the application
            mTracerEffects.push_back( effect );
         }

         bool success = effect != NULL;

         // Set position and velocity on the new/recycled tracer effect.
         if( success )
         {
            effect->SetPosition( weaponFirePoint );
            effect->SetVelocity( intialVelocity );

            // Setup tracer to update and render.
            // This also sets the initial orientation of the tracer.
            float tracerLifeTime = CalcTimeToImpact( 
               weaponFirePoint, intialVelocity, 
               effectsInfo.GetTracerLifeTime() );
            effect->SetHasLight( useLight );
            effect->Execute( tracerLifeTime );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffectsManager::AddTracerEffectRequest( dtCore::RefPtr<TracerEffectRequest>& effectRequest )
      {
         if( ! effectRequest.valid() )
         {
            return false;
         }
         unsigned lastSize = mTracerRequests.size();
         mTracerRequests.push_back( effectRequest.get() );

         return mTracerRequests.size() > lastSize;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::ClearTracerEffectRequests()
      {
         unsigned lastSize = mTracerRequests.size();
         mTracerRequests.clear();
         return lastSize;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::UpdateTracerEffectRequests( float timeDelta )
      {
         if( mTracerRequests.empty() )
         {
            return;
         }

         bool deleteEffectRequest = false;
         bool firstEffectRequestProcessed = false;
         TracerEffectRequest* curRequest = NULL;
         TracerEffectRequestList::iterator iter(--mTracerRequests.end());
         while( ! firstEffectRequestProcessed )
         {
            firstEffectRequestProcessed = iter == mTracerRequests.begin();
            deleteEffectRequest = false;
            curRequest = iter->get();

            if( curRequest == NULL ) // This should not happen
            {
               deleteEffectRequest = true;
               LOG_WARNING( "WeaponEffectsManager acquired a NULL TracerEffectRequest" );
            }
            else
            {
               curRequest->Update( timeDelta );
               if( curRequest->IsEffectReady() )
               {
                  ApplyTracerEffect( curRequest->GetFirePoint(),
                     curRequest->GetVelocity(), curRequest->GetEffectsInfo(), curRequest->IsFirstEffect() );

                  curRequest->Decrement();
               }

               if( curRequest->GetTotalEffects() < 1 )
               {
                  deleteEffectRequest = true;
               }
            }

            if( deleteEffectRequest )
            {
               TracerEffectRequestList::iterator deleteIter = iter;
               if( ! firstEffectRequestProcessed )
                  --iter;
               mTracerRequests.erase(deleteIter);
            }
            else
            {
               if( ! firstEffectRequestProcessed )
                  --iter;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::GetWeaponEffectCount() const
      {
         return mEntityToEffectMap.size();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::GetTracerEffectCount() const
      {
         return mTracerEffects.size();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::GetTracerEffectActiveCount() const
      {
         unsigned activeCount = 0;
         unsigned limit = mTracerEffects.size();
         for( unsigned i = 0; i < limit; ++i )
         {
            if( mTracerEffects[i].valid() && mTracerEffects[i]->IsActive() )
            {
               ++activeCount;
            }
         }
         return activeCount;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::Update( float deltaTime )
      {
         if( mCurRecycleTime < mRecycleTime )
         {
            mCurRecycleTime += deltaTime;
         }

         if( mCurRecycleTime >= mRecycleTime )
         {
            Recycle();
            mCurRecycleTime = 0.0f;
         }

         // Iterate through all effects and update their times.
         std::map<std::string, dtCore::RefPtr<WeaponEffect> >::iterator iter 
            = mEntityToEffectMap.begin();
         for( ; iter != mEntityToEffectMap.end(); ++iter )
         {
            // Whole vector is assumed to be valid, provided that it adds and deletes
            // elements properly.
            iter->second->Update( deltaTime );
         }

         UpdateTracerEffectRequests( deltaTime );

         unsigned limit = mTracerEffects.size();
         for( unsigned effect = 0; effect < limit; ++effect )
         {
            if( mTracerEffects[effect].valid() )
            {
               mTracerEffects[effect]->Update(deltaTime);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::RecycleWeaponEffects()
      {
         unsigned recycleCount = 0;
         std::vector<std::string> deleteList;
         std::map<std::string, dtCore::RefPtr<WeaponEffect> >::iterator iter 
            = mEntityToEffectMap.begin();

         // Find any effects that need to be freed
         WeaponEffect* curEffect = NULL;
         for( ; iter != mEntityToEffectMap.end(); ++iter )
         {
            curEffect = iter->second.get();
            if( curEffect == NULL || curEffect->GetOwner() == NULL 
               || curEffect->GetTimeSinceFlash() >= mEffectTimeMax )
            {
               if( curEffect != NULL ) { curEffect->Clear( true ); }
               deleteList.push_back( iter->first );
            }
         }

         // Delete all effects associated with the keys in the delete list.
         // --- DEBUG --- START --- //
         //if( ! deleteList.empty() )
         //   std::cout << "\nWeaponEffectsManager Recycling" << std::endl;
         // --- DEBUG --- END --- //
         std::vector<std::string>::iterator deleteIter = deleteList.begin();
         for( ; deleteIter != deleteList.end(); ++deleteIter )
         {
            std::map<std::string, dtCore::RefPtr<WeaponEffect> >::iterator foundIter
               = mEntityToEffectMap.find( *deleteIter );

            if( foundIter != mEntityToEffectMap.end() )
            {
               // --- DEBUG --- START --- //
               //WeaponEffect* curEffect = foundIter->second.get();
               //if( curEffect )
               //{
               //   std::cout << "\tWeaponEffect: " << curEffect->GetUniqueId() << std::endl;
               //}
               // --- DEBUG --- END --- //
               mEntityToEffectMap.erase( foundIter );
               recycleCount++;
            }
         }

         return recycleCount;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::RecycleTracerEffects()
      {
         unsigned recycleCount = 0;

         // Remove any unused, extraneous effects
         if( ! mTracerEffects.empty() && mMaxTracerEffects > -1 
            && mTracerEffects.size() > unsigned(mMaxTracerEffects) )
         {
            std::vector<dtCore::RefPtr<TracerEffect> >::iterator iter(--mTracerEffects.end());
            std::vector<dtCore::RefPtr<TracerEffect> >::iterator begin = 
				mTracerEffects.begin();

            for( ; iter != begin; )
            {
               // Remove all NULLs and unused effects that make the effects
               // vector longer than necessary.
               if( ! iter->valid() || ( ! (*iter)->IsActive() && mTracerEffects.size() > unsigned(mMaxTracerEffects) ) )
               {
                  // Ensure that the tracer is still not held by the scene.
                  if( mGM.valid() )
                  {
                     mGM->GetScene().RemoveDrawable( iter->get() );
                  }

                  std::vector<dtCore::RefPtr<TracerEffect> >::iterator toDelete = 
                     iter;

                  --iter;
                  mTracerEffects.erase(toDelete);
               }
               else
               {
                  --iter;
               }
            }
         }

         // DEBUG: std::cout << "Tracers:\t" << mTracerEffects.size() << std::endl;

         return recycleCount;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::Recycle()
      {
         return RecycleWeaponEffects() + RecycleTracerEffects();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::ClearWeaponEffects()
      {
         std::map<std::string, dtCore::RefPtr<WeaponEffect> >::iterator iter 
            = mEntityToEffectMap.begin();

         for( ; iter != mEntityToEffectMap.end(); ++iter )
         {
            iter->second->Clear( true );
            iter->second = NULL;
         }

         if( ! mEntityToEffectMap.empty() ) { mEntityToEffectMap.clear(); }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::ClearTracerEffects()
      {
         if( mGM.valid() )
         {
            unsigned limit = mTracerEffects.size();
            for( unsigned tracer = 0; tracer < limit; ++tracer )
            {
               mGM->GetScene().RemoveDrawable( mTracerEffects[tracer].get() );
            }
         }

         if( ! mTracerEffects.empty() )
         {
            mTracerEffects.clear();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::Clear()
      {
         ClearWeaponEffects();
         ClearTracerEffects();
      }

      //////////////////////////////////////////////////////////////////////////
      float WeaponEffectsManager::CalcTimeToImpact( 
         const osg::Vec3& weaponFirePoint, const osg::Vec3& initialVelocity, float maxTime )
      {
         float speed = initialVelocity.length();
         if( speed == 0.0f )
         {
            return maxTime;
         }

         dtCore::BatchIsector::SingleISector& SingleISector = mIsector->EnableAndGetISector(0);
         osg::Vec3 endPoint( weaponFirePoint + (initialVelocity*maxTime) );
         SingleISector.SetSectorAsLineSegment( weaponFirePoint, endPoint);

         if( mIsector->Update( osg::Vec3(0,0,0), true ) )
         {
            if( SingleISector.GetNumberOfHits() > 0 ) 
            {
               SingleISector.GetHitPoint( endPoint );

               maxTime = (endPoint-weaponFirePoint).length()/speed;

               // Clear for next use
               mIsector->Reset();
            }
         }

         return maxTime;
      }



      //////////////////////////////////////////////////////////////////////////
      // Munitions Component Code
      //////////////////////////////////////////////////////////////////////////
      const std::string MunitionsComponent::DEFAULT_NAME("MunitionsComponent");

      //////////////////////////////////////////////////////////////////////////
      MunitionsComponent::MunitionsComponent( const std::string& name )
         :dtGame::GMComponent(name),
         mMunitionConfigPath("Configs:MunitionsConfig.xml"),
         mIsector(new dtCore::BatchIsector),
         mLastDetonationTime(0.0f),
         mEffectsManager(new WeaponEffectsManager)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionsComponent::~MunitionsComponent()
      {
         ClearRegisteredEntities();
         ClearTables();
         if( mMunitionTypeTable.valid() ) { mMunitionTypeTable->Clear(); }
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper* MunitionsComponent::CreateDamageHelper( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork )
      {
         return new DamageHelper( entity, autoNotifyNetwork );
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::Register( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork )
      {
         if( HasRegistered( entity.GetUniqueId() ) )
         {
            std::stringstream ss;
            ss << "Entity "
               << entity.GetUniqueId().ToString().c_str()
               <<" has already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
            return false;
         }

         DamageHelper* newHelper = CreateDamageHelper( entity, autoNotifyNetwork );

         if( newHelper == NULL ) { return false; }

         bool success = mIdToHelperMap.insert( 
               std::make_pair( entity.GetUniqueId(), newHelper ) 
            ).second;

         if( !success )
         {
            std::stringstream ss;
            ss << "FAILURE: Munition Component registering entity \""
               << entity.GetUniqueId().ToString().c_str()
               << "\" of class type \"" << entity.GetGameActorProxy().GetClassName() 
               <<"\" for damage tracking. Entity may have already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
         }
         else
         {
            std::stringstream ss;
            ss << "Munition Component registered entity \"" 
               << entity.GetUniqueId().ToString().c_str() 
               << "\" of class type \"" << entity.GetGameActorProxy().GetClassName() 
               << "\"" << std::endl;
            LOG_DEBUG( ss.str() );
         }

         // Load the munition table that the helper will need to reference, if one exists
         std::string tableName(entity.GetMunitionDamageTableName());

         if( ! tableName.empty() )
         {
            // If the table exists, link it to the newly created helper
            dtCore::RefPtr<MunitionDamageTable> table = GetMunitionDamageTable( tableName );
            newHelper->SetMunitionDamageTable( table );
         
            std::stringstream ss;
            if( table.valid() )
            {
               ss << "\tLoaded munition damage table \"" << tableName.c_str() 
                  << "\"" << std::endl;
               LOG_DEBUG( ss.str() );
            }
            else
            {
               ss << "FAILURE: Munition damage table \"" << tableName.c_str() 
                  << "\" could not be found." << std::endl;
               LOG_ERROR( ss.str() );
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::Unregister( const dtCore::UniqueId& entityId )
      {
         std::map< dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator itor = 
            mIdToHelperMap.find( entityId );

         if( itor != mIdToHelperMap.end() )
         {
            mIdToHelperMap.erase( itor );
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::HasRegistered( const dtCore::UniqueId& entityId ) const
      {
         return mIdToHelperMap.find( entityId ) != mIdToHelperMap.end();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ClearRegisteredEntities()
      {
         mIdToHelperMap.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionsComponent::LoadMunitionDamageTables( const std::string& munitionConfigPath )
      {
         // Find the specified file
         std::string resourcePath = dtDAL::Project::GetInstance()
            .GetResourcePath(dtDAL::ResourceDescriptor( munitionConfigPath ));
         if( resourcePath.empty() )
         {
            std::stringstream ss;
            ss << "Failure: MunitionsComponent.LoadMunitionDamageTables could not locate \"" 
               << munitionConfigPath.c_str() << "\"" << std::endl;
            LOG_ERROR( ss.str() );
            return 0;
         }

         // Capture new tables in a vector
         std::vector<dtCore::RefPtr<MunitionDamageTable> > tables;

         // Parse the new table data
         dtCore::RefPtr<MunitionsConfig> mParser = new MunitionsConfig();
         unsigned int successes = 
            mParser->LoadMunitionTables( resourcePath, tables );
         mParser = NULL;

         // Iterate through new tables and add/replace them into the table map
         MunitionDamageTable* existingTable = NULL;
         std::vector<dtCore::RefPtr<MunitionDamageTable> >::iterator iter = tables.begin();
         for( ; iter != tables.end(); ++iter )
         {
            // Check for existing table
            existingTable = GetMunitionDamageTable( (*iter)->GetName() );

            // Replace the existing table with the new one; reduce success if insert fails
            if( existingTable != NULL && ! RemoveMunitionDamageTable( existingTable->GetName() ) )
            {
               std::stringstream ss;
               ss << "Failure: MunitionsComponent.LoadMunitionDamageTables could not remove existing munition table \"" 
                  << existingTable->GetName() << "\"" << std::endl;
               LOG_WARNING( ss.str() );
            }

            // Simply add the new table; reduce success if insert fails
            if( ! AddMunitionDamageTable( *iter ) )
            {
               // Reduce successes because insert failed
               successes--;
               std::stringstream ss;
               ss << "Failure: MunitionsComponent.LoadMunitionDamageTables could insert new munition table \"" 
                  << (*iter)->GetName() << "\"" << std::endl;
               LOG_WARNING( ss.str() );
            }
         }

         return successes;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionsComponent::LoadMunitionTypeTable( const std::string& mapName )
      {
         // Prepare the table for a fresh load of data.
         if( mMunitionTypeTable.valid() )
         {
            mMunitionTypeTable->Clear();
         }
         else
         {
            mMunitionTypeTable = new MunitionTypeTable;
         }

         // Load the map file
         dtDAL::Map *map = NULL;
         try
         {
            map = &dtDAL::Project::GetInstance().GetMap(mapName);
         }
         catch(const dtUtil::Exception &e)
         {
            std::ostringstream oss;
            oss << "ERROR! Failed to load the munitions type table named: " << mapName << 
               " because: " << e.What() << ". You will not be able to see detonations.";

            LOG_ERROR(oss.str());
            return 0;
         }
         dtDAL::Map &actorMap = *map;
         std::vector<dtCore::RefPtr<dtDAL::ActorProxy> > proxies;
         actorMap.GetAllProxies( proxies );

         // Declare variable for the loop
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> curProxy = NULL;
         unsigned int munitions = 0;

         // Populate the table with valid MunitionTypeActors
         std::vector<dtCore::RefPtr<dtDAL::ActorProxy> >::iterator iter = proxies.begin();
         for( ; iter != proxies.end(); ++iter )
         {
            curProxy = dynamic_cast<SimCore::Actors::MunitionTypeActorProxy*> (iter->get());
            if( curProxy.valid() )
            {
               if( mMunitionTypeTable->AddMunitionType( curProxy ) )
               {
                  ++munitions;
               }
            }
         }

         proxies.clear();

         return munitions;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ProcessMessage( const dtGame::Message& message )
      {
         const dtGame::MessageType& type = message.GetMessageType();

         // Update the effects manager
         if( type == dtGame::MessageType::TICK_LOCAL )
         {
            const dtGame::TickMessage& tickMessage 
               = static_cast<const dtGame::TickMessage&> (message);

            mEffectsManager->Update( tickMessage.GetDeltaSimTime() );

            return;
         }
         // Avoid the most common messages that will not need to be processed
         if( type == dtGame::MessageType::TICK_REMOTE )
         { return; }

         // Process special messages...

         if( type == SimCore::MessageType::DETONATION )
         {
            const DetonationMessage& detMessage = 
               dynamic_cast<const DetonationMessage&> (message);

            // Is this Direct Fire?
            if( ! message.GetAboutActorId().ToString().empty() )
            {
               DamageHelper* helper = GetHelperByEntityId( message.GetAboutActorId() );
               if( helper != NULL )
               {
                  helper->ProcessDetonationMessage( detMessage, 
                     mMunitionTypeTable->GetMunitionType( detMessage.GetMunitionType() ), true );
               }
            }
            else // this is Indirect Fire
            {
               std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator iter = 
                  mIdToHelperMap.begin();

               for( ; iter != mIdToHelperMap.end(); ++iter )
               {
                  iter->second->ProcessDetonationMessage( detMessage, 
                     mMunitionTypeTable->GetMunitionType( detMessage.GetMunitionType() ), false );
               }
            }

            // Create the particle systems and sound effects
            ApplyDetonationEffects( detMessage );
         }
         else if( type == SimCore::MessageType::SHOT_FIRED )
         {
            const ShotFiredMessage& shotMessage = 
               dynamic_cast<const ShotFiredMessage&> (message);

            DamageHelper* helper = NULL;
            // Is this Direct Fire?
            if( ! message.GetAboutActorId().ToString().empty() )
            {
               helper = GetHelperByEntityId( message.GetAboutActorId() );
               if( helper != NULL )
               {
                  helper->ProcessShotMessage( shotMessage, 
                     mMunitionTypeTable->GetMunitionType( shotMessage.GetMunitionType() ), true );
               }
            }
            else // this is Indirect Fire
            {
               std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator iter = 
                  mIdToHelperMap.begin();

               for( ; iter != mIdToHelperMap.end(); ++iter )
               {
                  iter->second->ProcessShotMessage( shotMessage, 
                     mMunitionTypeTable->GetMunitionType( shotMessage.GetMunitionType() ), false );
               }
            }

            // Apply gun flash effects only to remote entities
            if(message.GetSource() != GetGameManager()->GetMachineInfo())
            {
               ApplyShotfiredEffects( shotMessage );
            }
         }
         // Capture the player
         else if(message.GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD)
         {
            dtGame::GameActorProxy* proxy 
               = GetGameManager()->FindGameActorById(message.GetAboutActorId());

            if ( proxy == NULL || proxy->IsRemote() ) { return; }

            mPlayer = dynamic_cast<SimCore::Actors::StealthActor*>(proxy->GetActor());

            if( ! mPlayer.valid() )
            {
               LOG_ERROR("Received a player entered world message from an actor that is not a player");
               return;
            }
         }
         else if( type == dtGame::MessageType::INFO_ACTOR_DELETED )
         {
            if( mPlayer.valid() && message.GetAboutActorId() == mPlayer->GetUniqueId() )
            {
               mPlayer = NULL;
            }

            Unregister( message.GetAboutActorId() );
         }
         else if( type == dtGame::MessageType::INFO_TIME_CHANGED )
         {
            mLastDetonationTime = 0.0f;
         }
         else if( type == dtGame::MessageType::INFO_MAP_LOADED )
         {
            std::vector<dtDAL::ActorProxy*> terrains;
            GetGameManager()->FindActorsByType( *SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, terrains );

            if( ! terrains.empty() )
            {
               mIsector->SetQueryRoot( terrains[0]->GetActor() );
            }
         }
         else if( type == dtGame::MessageType::INFO_RESTARTED
            || type == dtGame::MessageType::INFO_MAP_UNLOADED )
         {
            ClearRegisteredEntities();
            ClearTables();
            mLastDetonationTime = 0.0f;

            if( type == dtGame::MessageType::INFO_RESTARTED )
            {
               LoadMunitionDamageTables( mMunitionConfigPath );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper* MunitionsComponent::GetHelperByEntityId( const dtCore::UniqueId id )
      {
         std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator iter =
            mIdToHelperMap.find( id );

         return iter != mIdToHelperMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionDamageTable* MunitionsComponent::GetMunitionDamageTable( const std::string& entityClassName )
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::iterator iter =
            mNameToMunitionDamageTableMap.find( entityClassName );

         return iter != mNameToMunitionDamageTableMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const MunitionDamageTable* MunitionsComponent::GetMunitionDamageTable( const std::string& entityClassName ) const
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::const_iterator iter =
            mNameToMunitionDamageTableMap.find( entityClassName );

         return iter != mNameToMunitionDamageTableMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::AddMunitionDamageTable( dtCore::RefPtr<MunitionDamageTable>& table )
      {
         if( ! table.valid() ) { return false; }

         return mNameToMunitionDamageTableMap.insert(
               std::make_pair( table->GetName(), table.get() ) 
            ).second;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::RemoveMunitionDamageTable( const std::string& entityClassName )
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::iterator iter =
            mNameToMunitionDamageTableMap.find( entityClassName );

         if( iter != mNameToMunitionDamageTableMap.end() )
         {
            mNameToMunitionDamageTableMap.erase(iter);
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ClearTables()
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::iterator iter = 
            mNameToMunitionDamageTableMap.begin();

         for( ; iter != mNameToMunitionDamageTableMap.end(); ++iter )
         {
            if( iter->second.valid() )
            {
               iter->second->Clear();
            }
         }

         mNameToMunitionDamageTableMap.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::SetDamage( SimCore::Actors::BaseEntity& entity, DamageType& damage )
      {
         DamageHelper* helper = GetHelperByEntityId( entity.GetUniqueId() );
         if( helper != NULL )
         {
            helper->SetDamage( damage );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ApplyShotfiredEffects( const ShotFiredMessage& message )
      {
         const std::string& munitionName = message.GetMunitionType();
         if ( munitionName.empty() )
         {
            LOG_WARNING("Ignoring munition with type UNKNOWN.");
            return;
         }

         if( ! mMunitionTypeTable.valid() )
         {
            LOG_ERROR("Ignoring munition, no MunitionTypeTable exists.");
            return;
         }

         // Obtain the closest matching registered munition type.
         const SimCore::Actors::MunitionTypeActor* munitionType 
            = mMunitionTypeTable->GetMunitionType( munitionName );

         if( munitionType == NULL )
         {
            LOG_WARNING("Received a weapon fire with an invalid munition type. Ignoring");
            return;
         }

         // Get the munition effects info so that the detonation actor can be set
         // with relevant data.
         const SimCore::Actors::MunitionEffectsInfoActor* effects = 
            dynamic_cast<const SimCore::Actors::MunitionEffectsInfoActor*> 
            (munitionType->GetEffectsInfoActor());

         if( effects == NULL )
         {
            std::ostringstream ss;
            ss << "Munition \"" << munitionName << "\" does not have effects defined for it."
               << std::endl;
            LOG_ERROR( ss.str() );
            return;
         }

/*         // Prepare a new detonation actor to be placed into the world.
         RefPtr<SimCore::Actors::DetonationActorProxy> detProxy;
         GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::DETONATION_ACTOR_TYPE, detProxy);
         if( ! detProxy.valid() )
         {
            LOG_ERROR("Failed to create the detonation proxy");
            return;
         }
         // Obtain the new detonation actor proxy's actor
         SimCore::Actors::DetonationActor &da = static_cast<SimCore::Actors::DetonationActor&>(detProxy->GetGameActor());

         // Pre-calculate the offset of sound based on the distance from the player.
         if(mPlayer.valid())
         {
            dtCore::Transform xform;
            mPlayer->GetTransform(xform);
            da.CalculateDelayTime(xform.GetTranslation());
         }

         // Populate detonation actor with effects
         std::string curValue = effects->GetFireSound();
         if( ! curValue.empty() ) { da.LoadSoundFile( curValue ); }
         curValue = effects->GetFireEffect();
         if( ! curValue.empty() ) { da.LoadDetonationFile( curValue ); }

         // Place the detonation actor into the scene
         GetGameManager()->AddActor(da.GetGameActorProxy(), true, false);
*/
         // Find the remote actor who sent the fire message so that flash and
         // sound effects can be applied to it.
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy
            = dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>
            (GetGameManager()->FindActorById( message.GetSendingActorId() ));

         // Prevent further processing if no visible entity was found to have
         // fired the shot.
         if( ! proxy.valid() ) { return; }

         // Access the entity directly instead of though a proxy
         SimCore::Actors::Platform* entity = dynamic_cast<SimCore::Actors::Platform*> (&proxy->GetGameActor());

         if( entity == NULL ) { return; }

         // Access the node collector that will be used for finding DOF attach
         // points on the entity's model geometry.
         dtCore::NodeCollector* nodeCollector = entity->GetNodeCollector();

         if( nodeCollector == NULL ) { return; }

         // Prepare variable to be used in the weapons cycling loop
         float curDotProd = -1.0f;
         float lastDotProd = -1.0f;
         osg::Vec3 trajectoryNormal( message.GetInitialVelocityVector() );
         trajectoryNormal.normalize();
         osgSim::DOFTransform* curDof = NULL;
         osgSim::DOFTransform* bestDof = NULL;

         // Loop though and find a weapon on the entity's model that closely matches
         // the fired munition's initial fired direction. The weapon that matches
         // the direction the closest will be assigned the flash and sound effects.
         int limit = nodeCollector->GetTransformNodeMap().size();
         for( int weapon = 1; weapon <= limit; ++weapon )
         {
            // DOF's will have an assumed nomenclature in the format of
            // "hotspot_" followed by a 2 digit number, starting at one.
            std::stringstream ss;
            ss << "hotspot_" << (weapon < 10 ? "0" : "") << weapon;
            curDof = nodeCollector->GetDOFTransform(ss.str());
            if( curDof == NULL ) { continue; }

            // If there is more than one weapon, compare current the weapon's
            // direction with the munition's trajectory
            if( limit > 1 )
            {
               osg::Matrix mtx;
               osg::Vec3 hpr;
               hpr = curDof->getCurrentHPR();
               dtUtil::MatrixUtil::HprToMatrix( mtx, hpr );
               osg::Vec3 weaponDirection( mtx.ptr()[8], mtx.ptr()[9], mtx.ptr()[10] );
               weaponDirection.normalize();
               curDotProd = weaponDirection * trajectoryNormal;
            }

            if( bestDof == NULL || curDotProd > lastDotProd )
            { 
               bestDof = curDof; 
               lastDotProd = curDotProd;
            }
         }

         // Attach a flash effect if a DOF was found.
         if( bestDof != NULL )
         {
            // Get the player's location in the world so that sound effects can
            // be attenuated when created in 3D space.
            dtCore::Transform playerXform;
            GetGameManager()->GetApplication().GetCamera()->GetTransform(playerXform);

            // Initiate the weapon effect
            mEffectsManager->ApplyWeaponEffect( 
               *entity, bestDof, *effects, playerXform.GetTranslation() );
         }

         // Place tracers into the scene
         if( ! effects->GetTracerShaderName().empty() )
         {
            unsigned quantity = message.GetQuantityFired();

            // Determine if enough rounds have been shot to justify production
            // of a new tracer effect.
            int tracerFrequency = munitionType->GetTracerFrequency();

            // If tracers are allowable...
            if( tracerFrequency > 0 )
            {
               float probability = 0.0f;
               if( int(quantity) >= tracerFrequency ) // ...with 100% probability...
               {
                  probability = 1.0f;
               }
               else // ...determine a probability of a tracer effect...
               {
                  probability = float(quantity) / float(tracerFrequency);
               }

               // ...and if the probability of a tracer is within the probable area...
               if( probability == 1.0f || dtUtil::RandFloat( 0.0f, 1.0 ) < probability )
               {
                  // ...generate a tracer effect request...
                  dtCore::RefPtr<TracerEffectRequest> effectRequest
                     = new TracerEffectRequest( quantity, 0.05f, *effects );
                  effectRequest->SetVelocity( message.GetInitialVelocityVector() );

                  if( entity != NULL && bestDof != NULL )
                  {
                     // ...from the fire point on the firing entity.
                     osg::Matrix mtx;
                     entity->GetAbsoluteMatrix( bestDof, mtx );

                     effectRequest->SetFirePoint( mtx.getTrans() );
                     mEffectsManager->AddTracerEffectRequest( effectRequest );

                     //mEffectsManager->ApplyTracerEffect( 
                     //   mtx.getTrans(), message.GetInitialVelocityVector(), *effects );
                  }
                  else
                  {
                     // ...from the firing location specified in the message.
                     effectRequest->SetFirePoint( message.GetFiringLocation() );
                     mEffectsManager->AddTracerEffectRequest( effectRequest );

                     //mEffectsManager->ApplyTracerEffect( 
                     //   message.GetFiringLocation(), message.GetInitialVelocityVector(), *effects );
                  }
               }
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ApplyDetonationEffects( const DetonationMessage& message )
      {
         //Sort of a hack.  Detonations cannot be drawn if they come so quickly.
         double simTime = GetGameManager()->GetSimulationTime();
         if ( simTime > mLastDetonationTime && simTime - mLastDetonationTime <= 0.05f )
         {
            LOG_DEBUG("Skipping detonations that occur too close together.");
            return;
         }
         else 
            mLastDetonationTime = simTime;

         const std::string& munitionName = message.GetMunitionType();
         if ( munitionName.empty() )
         {
            LOG_WARNING("Ignoring munition with type UNKNOWN.");
            return;
         }

         if( ! mMunitionTypeTable.valid() )
         {
            LOG_ERROR("Ignoring munition, no MunitionTypeTable exists.");
            return;
         }

         // Obtain the closest matching registered munition type.
         const SimCore::Actors::MunitionTypeActor* munitionType 
            = mMunitionTypeTable->GetMunitionType( munitionName );

         if( munitionType == NULL )
         {
            LOG_WARNING("Received a detonation with an invalid detonation munition type. Ignoring");
            return;
         }

         // Get the munition effects info so that the detonation actor can be set
         // with relevant data.
         const SimCore::Actors::MunitionEffectsInfoActor* effects = 
            dynamic_cast<const SimCore::Actors::MunitionEffectsInfoActor*> 
            (munitionType->GetEffectsInfoActor());

         if( effects == NULL )
         {
            std::ostringstream ss;
            ss << "Munition \"" << munitionName << "\" does not have effects defined for it."
               << std::endl;
            LOG_ERROR( ss.str() );
            return;
         }

         // Figure out if we hit an entity or if we hit the ground!
         bool hitEntity = ! message.GetAboutActorId().ToString().empty();
         if( hitEntity )
         {
            dtDAL::ActorProxy * targetProxy = GetGameManager()->FindActorById(message.GetAboutActorId());
            // Note - the mIsector here is kinda wonky cause it holds the terrain Drawable.
            // Change hitEntity to false if this is found to be the terrain actor.
            hitEntity = ! (targetProxy != NULL 
               && ( dynamic_cast<NxAgeiaTerraPageLandActor*>(targetProxy->GetActor()) != NULL
               || dynamic_cast<SimCore::Actors::TerrainActor*>(targetProxy->GetActor()) != NULL ) );
         }

         // Set the position and ground clamp
         osg::Vec3 pos( message.GetDetonationLocation() );
         osg::Vec3 endPos = pos;

         // If an entity was hit, use clamping along the impact velocity vector
         if( hitEntity )
         {
            osg::Vec3 normal( message.GetFinalVelocityVector() );
            if( normal.length2() >= 0.0f )
            {
               normal.normalize();
               normal *= 0.5f;
               pos -= normal;
               endPos += normal;
            }
         }
         else // use straight up and down clamping
         {
            pos.z() -= 500.0f;
            endPos.z() += 500.0f;
         }

         dtCore::BatchIsector::SingleISector& SingleISector = mIsector->EnableAndGetISector(0);
         SingleISector.SetSectorAsLineSegment(pos, endPos);
         int fidID = 0;

         if(mIsector->Update( osg::Vec3(0,0,0), true ) )
         {
            osg::Vec3 hp;
            
            // Make sure the isector actually has hit points on the terrain.
            // This component will not assume the isector to return an unsigned value
            // when its function return is a signed value; thus failing everything from
            // 0 and everything to the left of 0.
            if( SingleISector.GetNumberOfHits() <= 0 ) 
            {
               LOG_WARNING( "Munition Component could not place a detonation actor because the BatchIsector has an empty hit list." );
               return;
            }

            SingleISector.GetHitPoint(hp);
            const osg::Drawable* drawable = SingleISector.GetIntersectionHit(0).getDrawable();
            if( drawable != NULL && drawable->getStateSet() != NULL)
            {
               osg::ref_ptr<const osg::IntArray> mOurList = dynamic_cast<const osg::IntArray*>(drawable->getStateSet()->getUserData());
               if( mOurList.valid() ) 
               {
                  if( ! mOurList->empty() )
                  {
                     int value[4];
                     int iter = 0;
                     osg::IntArray::const_iterator listiter = mOurList->begin();
                     for(; listiter != mOurList->end(); listiter++)
                     {
                        value[iter] = *listiter;
                        ++iter;
                     }
                     fidID = value[0];
                  }
               }
            }

            if (dtUtil::Log::GetInstance().IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            {
               std::ostringstream ss;
               ss << "Found a hit - old z " << pos.z() << " new z " << hp.z();
               LOG_DEBUG(ss.str());
            }

            mIsector->Reset();

            // --- DEBUG --- START --- //
            //std::cout << "MunComp. Det. (clamped): " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
            // --- DEBUG --- END --- //
         }

         // Prepare a detonation actor to be placed into the scene
         RefPtr<SimCore::Actors::DetonationActorProxy> proxy;
         GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::DETONATION_ACTOR_TYPE, proxy);

         if(!proxy.valid())
         {
            LOG_ERROR("Failed to create the detonation proxy");
            return;
         }

         SimCore::Actors::DetonationActor *da = dynamic_cast<SimCore::Actors::DetonationActor*>(proxy->GetActor());
         if(da == NULL)
         {
            LOG_ERROR("Received a detonation actor proxy that did not contain a detonation actor. Ignoring.");
            return;
         }

         // Set the detonation's position
         pos = message.GetDetonationLocation();
         dtCore::Transform xform(pos[0], pos[1], pos[2]);
         da->SetTransform(xform, dtCore::Transformable::REL_CS);

         SimCore::Components::ViewerMaterialComponent* materialComponent = dynamic_cast<SimCore::Components::ViewerMaterialComponent*>(GetGameManager()->GetComponentByName("ViewerMaterialComponent"));
         if(materialComponent != NULL)
         {
            SimCore::Actors::ViewerMaterialActor& viewerMaterial = materialComponent->CreateOrChangeMaterialByFID(fidID);
            da->SetMaterialCollidedWith(viewerMaterial);
         }

         // Delay particles to ensure that sound reaches the player at the time
         // particles start spawning.
         if( mPlayer.valid() )
         {
            dtCore::Transform xform;
            mPlayer->GetTransform(xform);
            da->CalculateDelayTime(xform.GetTranslation());
         }  

         // Set properties on the detonation actor
         std::string curValue;

         // Load the particle effect
         if( hitEntity && effects->HasEntityImpactEffect() )
         {
            curValue = effects->GetEntityImpactEffect();
            if( ! curValue.empty() ) { da->LoadDetonationFile( curValue ); }
         }
         else
         {
            curValue = effects->GetGroundImpactEffect();
            if( ! curValue.empty() ) { da->LoadDetonationFile( curValue ); }
         }

         // Load the sound
         if( hitEntity && effects->HasEntityImpactSound() )
         {
            curValue = effects->GetEntityImpactSound();
            if( ! curValue.empty() ) 
            {
               da->LoadSoundFile( curValue );
               da->SetMinimumSoundDistance( effects->GetEntityImpactSoundMinDistance() );
               da->SetMaximumSoundDistance( effects->GetEntityImpactSoundMaxDistance() );
            }
         }
         else
         {
            curValue = effects->GetGroundImpactSound();
            if( ! curValue.empty() )
            { 
               da->LoadSoundFile( curValue );
               da->SetMinimumSoundDistance( effects->GetGroundImpactSoundMinDistance() );
               da->SetMaximumSoundDistance( effects->GetGroundImpactSoundMaxDistance() );
            }
         }

         // Load smoke effect
         curValue = effects->GetSmokeEffect();
         if( ! curValue.empty() ) { da->LoadSmokeFile( curValue ); }
         da->SetLingeringSmokeSecs( effects->GetSmokeLifeTime() );

         // Determine if the detonation should have physics applied to its particles.
         bool avoidPhysics = munitionType->GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_ROUND 
            || munitionType->GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_UNKNOWN;
         da->SetPhysicsEnabled( ! avoidPhysics );

         // Prepare the reference light effect type
         curValue = effects->GetEntityImpactLight();
         if( ! hitEntity || curValue.empty() )
         {
            curValue = effects->GetGroundImpactLight();
         }
         da->SetLightName( curValue );

         // Add the newly created detonation to the scene
         GetGameManager()->AddActor(da->GetGameActorProxy(), false, false);  
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::OnAddedToGM()
      {
         dtGame::GMComponent::OnAddedToGM();
         mIsector->SetScene( &GetGameManager()->GetScene() );
         mEffectsManager->SetGameManager( GetGameManager() );
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& MunitionsComponent::GetMunitionDamageTypeName( const std::string& munitionTypeName ) const
      {
         // Static constant proposed by David so that this function can return a
         // string reference.
         static const std::string EMPTY("");

         if( ! mMunitionTypeTable.valid() ) { return EMPTY; }

         const SimCore::Actors::MunitionTypeActor* munitionType = 
            GetMunitionTypeTable()->GetMunitionType( munitionTypeName );

         if( munitionType == NULL ) { return EMPTY; }

         return munitionType->GetDamageType();
      }

   }
}
