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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <osgDB/ReadFile>
#include <dtAudio/audiomanager.h>

#include <dtCore/particlesystem.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtGame/exceptionenum.h>

#include <dtUtil/mathdefines.h>
#include <dtUtil/matrixutil.h>
#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/WeaponEffectsManager.h>



using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Utility Code
      //////////////////////////////////////////////////////////////////////////
      namespace WeaponEffectUtils
      {
         class EntityNodeVisitor : public osg::NodeVisitor
         {
            public:
               EntityNodeVisitor()
                  : mFoundNode(false)
                  , mNode(NULL)
               {
                  setTraversalMode( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
               }

               void SetGeodeToFind( const osg::Geode& node )
               {
                  mNode = &node;
               }

               bool HasFoundNode() const
               {
                  return mFoundNode;
               }

               virtual void apply( osg::Geode& node )
               {
                  if( ! mFoundNode )
                  {
                     mFoundNode = &node == mNode;
                  }
               }

               void Reset()
               {
                  mFoundNode = false;
                  mNode = NULL;
               }

            protected:
               virtual ~EntityNodeVisitor()
               {
               }

            private:
               bool mFoundNode;
               const osg::Geode* mNode;
         };
      }



      //////////////////////////////////////////////////////////////////////////
      // Munition Effect Code
      //////////////////////////////////////////////////////////////////////////
      MunitionEffect::MunitionEffect()
      : dtCore::Transformable("MunitionEffect")
      , mDynamicLightEnabled(false)
      , mDynamicLightID(0)
      , mLifeTime(0.0f)
      , mMaxLifeTime(1.0f)
      {
         SetVisible( true ); // ensures node mask is set to the proper value.
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionEffect::~MunitionEffect()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::SetLightName( const std::string& lightName )
      {
         mTracerLightName = lightName;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& MunitionEffect::GetLightName() const
      {
         return mTracerLightName.Get();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::SetMaxLifeTime( float maxLifeTime )
      {
         mMaxLifeTime = maxLifeTime;
      }

      //////////////////////////////////////////////////////////////////////////
      float MunitionEffect::GetMaxLifeTime() const
      {
         return mMaxLifeTime;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::SetVelocity( const osg::Vec3& velocity )
      {
         // If an external munition doesn't send the correct velocity vector, then we
         // just change it to be some sort of minimum. Otherwise, the tracer moves at like
         // 1 m/s which is awful.
         if( velocity.length() < 5.0f )
         {
            osg::Vec3 velocityNormal(velocity);
            velocityNormal.normalize();
            mVelocity = velocityNormal * 100.0f;
         }
         else
         {
            mVelocity = velocity;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec3& MunitionEffect::GetVelocity() const
      {
         return mVelocity;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::SetPosition( const osg::Vec3& position )
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
            mLastPosition.set( mPosition );
         }

         mPosition.set(position);

         dtCore::Transform xform;
         GetTransform(xform,dtCore::Transformable::REL_CS);
         xform.SetTranslation(position);
         SetTransform(xform,dtCore::Transformable::REL_CS);
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec3& MunitionEffect::GetPosition() const
      {
         return mPosition;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::SetVisible( bool visible )
      {
         GetOSGNode()->setNodeMask(visible?0xFFFFFFFF:0);
         if( visible && ! mTracerLightName.Get().empty() )
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
      bool MunitionEffect::IsVisible() const
      {
         return 0 != GetOSGNode()->getNodeMask();
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionEffect::IsActive() const
      {
         return mLifeTime < mMaxLifeTime;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::Reset()
      {
         mLifeTime = 0.0f;
         SetVisible( true );

         // Start the tracer line with zero stretch
         mLastPosition = mPosition;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::Execute( float maxLifeTime )
      {
         SetMaxLifeTime( maxLifeTime );

         Reset();

         // Physically adjust orientation to match direction
         dtCore::Transform xform;
         GetTransform(xform,dtCore::Transformable::REL_CS);
         osg::Matrix mtx;

         osg::Vec3 velocityNormal(mVelocity);
         if( velocityNormal.length2() > 0.0f )
         {
            velocityNormal.normalize();
         }
         else
         {
            velocityNormal.set(0.0f, 1.0f, 0.0f);
         }

         osg::Vec3 right = velocityNormal ^ osg::Vec3(0.0,0.0,1.0);
         right.normalize();
         osg::Vec3 up = right ^ velocityNormal;
         up.normalize();
         mtx.ptr()[0] = right[0];
         mtx.ptr()[1] = right[1];
         mtx.ptr()[2] = right[2];
         mtx.ptr()[4] = velocityNormal[0];
         mtx.ptr()[5] = velocityNormal[1];
         mtx.ptr()[6] = velocityNormal[2];
         mtx.ptr()[8] = up[0];
         mtx.ptr()[9] = up[1];
         mtx.ptr()[10] = up[2];
         xform.SetRotation(mtx);
         SetTransform(xform,dtCore::Transformable::REL_CS);
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::Update( float deltaTime )
      {
         if( IsActive() )
         {
            mVelocity += osg::Vec3( 0.0f, 0.0f, -9.8f ) * deltaTime;
            SetPosition( mPosition + (mVelocity * deltaTime) );
            mLifeTime += deltaTime;
         }
         else if( IsVisible() )
         {
            SetVisible( false );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::AddDynamicLight( const osg::Vec3& color )
      {
         if( ! mGM.valid() )
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp;
         mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);

         if( renderComp != NULL )
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = NULL;
            if( ! mDynamicLightEnabled )
            {
               dl = renderComp->AddDynamicLightByPrototypeName( mTracerLightName.Get() );
               dl->mTarget = this;
               
               //removing hard coded color on light, not sure why this is here considering it is created via prototype
               //dl->mColor = color;

               mDynamicLightID = dl->GetId();
               mDynamicLightEnabled = true;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffect::RemoveDynamicLight()
      {
         if( ! mGM.valid() )
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp;
         mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);

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
      void MunitionEffect::SetGameManager( dtGame::GameManager* gameManager )
      {
         mGM = gameManager;
      }



      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Code
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString TracerEffect::DEFAULT_TRACER_LIGHT("Light-Tracer");
      const dtUtil::RefString TracerEffect::DEFAULT_TRACER_SHADER("VolumetricLines");
      const dtUtil::RefString TracerEffect::DEFAULT_TRACER_SHADER_GROUP("TracerGroup");

      //////////////////////////////////////////////////////////////////////////
      TracerEffect::TracerEffect( float lineLength, float lineThickness,
         const std::string& shaderName, const std::string& shaderGroup )
         : MunitionEffect()
         , mMaxLength(10.0f)
         , mLine(new SimCore::Actors::VolumetricLine(lineLength,lineThickness,shaderName,shaderGroup))
      {
         AddChild( mLine.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      TracerEffect::~TracerEffect()
      {
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
      void TracerEffect::Reset()
      {
         BaseClass::Reset();
         mLine->SetLength(0.001f);
      }

      //////////////////////////////////////////////////////////////////////////
      void TracerEffect::Update( float deltaTime )
      {
         if( IsActive() )
         {
            SetPosition( mPosition + (mVelocity * deltaTime) );
            mLifeTime += deltaTime;

            // Update the tracer length for stretching
            float length = (mLastPosition-mPosition).length();
            if( length > mMaxLength )
            {
               mLine->SetLength( mMaxLength );
            }
            else if( length > 0.0 && length < mMaxLength )
            {
               mLine->SetLength( length );
            }
         }
         else if( IsVisible() )
         {
            SetVisible( false );
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // Tracer Effect Request Code
      //////////////////////////////////////////////////////////////////////////
      MunitionEffectRequest::MunitionEffectRequest( unsigned totalEffects, float cycleTime,
         const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo )
         : mIsFirstEffect(true)
         , mTotalEffects(totalEffects)
         , mCycleTime(cycleTime)
         , mCurrentTime(0.0f)
         , mEffectsInfo(&effectsInfo)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionEffectRequest::~MunitionEffectRequest()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectRequest::SetOwner( SimCore::Actors::BaseEntity* owner )
      {
         mOwner = owner;
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::BaseEntity* MunitionEffectRequest::GetOwner()
      {
         return mOwner.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::BaseEntity* MunitionEffectRequest::GetOwner() const
      {
         return mOwner.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectRequest::SetFirePoint( const osg::Vec3& firePoint )
      {
         mFirePoint = firePoint;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec3& MunitionEffectRequest::GetFirePoint() const
      {
         return mFirePoint;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectRequest::SetVelocity( const osg::Vec3& velocity )
      {
         mVelocity = velocity;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec3& MunitionEffectRequest::GetVelocity() const
      {
         return mVelocity;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectRequest::SetMunitionModelFile( const std::string& modelFile )
      {
         mMunitionModelFile = modelFile;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& MunitionEffectRequest::GetMunitionModelFile() const
      {
         return mMunitionModelFile.Get();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionEffectsInfoActor& MunitionEffectRequest::GetEffectsInfo() const
      {
         return *mEffectsInfo;
      }

      //////////////////////////////////////////////////////////////////////////
      float MunitionEffectRequest::GetCycleTime() const
      {
         return mCycleTime;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned MunitionEffectRequest::GetTotalEffects() const
      {
         return mTotalEffects;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionEffectRequest::IsFirstEffect() const
      {
         return mIsFirstEffect;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionEffectRequest::IsEffectReady() const
      {
         return mTotalEffects > 0 && mCurrentTime > mCycleTime;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectRequest::Update( float timeDelta )
      {
         mCurrentTime += timeDelta;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned MunitionEffectRequest::Decrement()
      {
         if( mTotalEffects > 0 )
         {
            --mTotalEffects;
         }
         mCurrentTime = 0.0f;
         mIsFirstEffect = false;
         return mTotalEffects;
      }



      //////////////////////////////////////////////////////////////////////////
      // Weapon Effect Code
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString WeaponEffect::CLASS_NAME("WeaponEffect");

      //////////////////////////////////////////////////////////////////////////
      WeaponEffect::WeaponEffect()
         : dtCore::Transformable(WeaponEffect::CLASS_NAME.Get())
         , mDynamicLightID(0)
         , mDynamicLightEnabled(false)
         , mVisible(true)
         , mSoundPlayed(false)
         , mSoundStartTime(0.0f)
         , mFlashProbability(1.0f)
         , mFlashTime(0.0f)
         , mTimeSinceFlash(0.0f)
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
            dtCore::Transform soundXform;
            mSound->GetTransform( soundXform );
            osg::Vec3 soundPos;
            soundXform.GetTranslation(soundPos);
            mSoundStartTime = CalculateSoundDelayTime(soundPos , listenerPosition );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::SetFlashProbability( float probability )
      {
         mFlashProbability = probability < 0.0f ? 0.0f : probability > 1.0f ? 1.0f : probability;
      }

      //////////////////////////////////////////////////////////////////////////
      float WeaponEffect::GetFlashProbability() const
      {
         return mFlashProbability;
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
      float WeaponEffect::GetFlashTime() const
      {
         return mFlashTime;
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
      dtAudio::Sound* WeaponEffect::GetSound()
      {
         return mSound.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtAudio::Sound* WeaponEffect::GetSound() const
      {
         return mSound.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::SetOwner( SimCore::Actors::BaseEntity* owner )
      {
         mOwner = owner;
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::BaseEntity* WeaponEffect::GetOwner()
      {
         return mOwner.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::BaseEntity* WeaponEffect::GetOwner() const
      {
         return mOwner.get();
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
      bool WeaponEffect::IsVisible() const
      {
         return mVisible;
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Group* WeaponEffect::GetParentNode()
      {
         return mParentNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Group* WeaponEffect::GetParentNode() const
      {
         return mParentNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      float WeaponEffect::GetTimeSinceFlash() const
      {
         return mTimeSinceFlash;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::IsSoundPlayed() const
      {
         return mSoundPlayed;
      }

      //////////////////////////////////////////////////////////////////////////
      float WeaponEffect::GetSoundDelay() const
      {
         return mSoundStartTime;
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

         if(mFlash.valid())
         {
            ParticleManagerComponent::AttachShaders(*mFlash);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::ParticleSystem* WeaponEffect::GetFlash()
      {
         return mFlash.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtCore::ParticleSystem* WeaponEffect::GetFlash() const
      {
         return mFlash.get();
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::Attach( osg::Group* parent )
      {
         if( ! mOwner.valid() ) { return false; }

         // Remove this from the owner in case it is already attached
         mOwner->RemoveChild( this );

         bool success = mOwner->AddChild( this );

         if( success )
         {
            mParentNode = parent;

            if( mParentNode.valid() )
            {
               // Set the parent of the flash explicitly.
               // This will be used like a boolean to determine if the flash has
               // already been attached to the DOF.
               if( mFlash.valid() && mFlash->GetParent() == NULL )
               {
                  mFlash->SetParent( this );
                  mParentNode->addChild( mFlash->GetOSGNode() );
               }
               if( mSound.valid() && mSound->GetParent() == NULL )
               {
                  mSound->SetParent( this );
                  mParentNode->addChild( mSound->GetOSGNode() );
               }
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::Detach()
      {
         if( mParentNode.valid() )
         {
            if( mFlash.valid() && mFlash->GetParent() != NULL )
            {
               mParentNode->removeChild( mFlash->GetOSGNode() );
               mFlash->SetParent( NULL );
            }
            if( mSound.valid() && mSound->GetParent() != NULL )
            {
               mParentNode->removeChild( mSound->GetOSGNode() );
               mSound->SetParent( NULL );
            }
         }
         mParentNode = NULL;

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

         if (! mSound.valid())
         {
            throw dtGame::InvalidParameterException(
                     "Failed to create the sound object for WeaponEffect", __FILE__, __LINE__);
         }

         mSound->LoadFile(filePath.c_str());

         return mSound.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffect::LoadFlash( const std::string& filePath )
      {
         dtCore::RefPtr<dtCore::ParticleSystem> flash = new dtCore::ParticleSystem;
         flash->SetParentRelative(true);

         if (! flash.valid())
         {
            throw dtGame::InvalidParameterException(
                     "Failed to create the particles object for WeaponEffect", __FILE__, __LINE__);
         }

         flash->LoadFile(filePath.c_str());

         SetFlash( flash.get() );

         return mFlash.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::AddDynamicLight( const osg::Vec3& color )
      {
         if (! mGM.valid())
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp;
         mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);

         if (renderComp != NULL)
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = NULL;
            if (! mDynamicLightEnabled)
            {
               //dl = new SimCore::Components::RenderingSupportComponent::DynamicLight();
               dl = renderComp->AddDynamicLightByPrototypeName(TracerEffect::DEFAULT_TRACER_LIGHT.Get());
               dl->mColor = color;//a bright yellow
               //dl->mAttenuation.set(0.1, 0.05, 0.0002);
               //dl->mIntensity = 1.0f;
               //dl->mSaturationIntensity = 0.0f; //no saturation
               dl->mTarget = mFlash.get();

               //mDynamicLightID = renderComp->AddDynamicLight(dl);
               mDynamicLightID = dl->GetId();
               mDynamicLightEnabled = true;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffect::RemoveDynamicLight()
      {
         if (! mGM.valid())
         {
            return;
         }

         SimCore::Components::RenderingSupportComponent* renderComp;
         mGM->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);

         if (renderComp != NULL)
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = renderComp->GetDynamicLight(mDynamicLightID);
            if (dl != NULL && mDynamicLightEnabled)
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
         : mEffectTimeMax(5.0f)
         , mRecycleTime(1.0f)
         , mCurRecycleTime(0.0f)
         , mMaxWeaponEffects(-1) // no limit
         , mMaxMunitionEffects(-1) // no limit
         , mIsector(new dtCore::BatchIsector)
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
      dtGame::GameManager* WeaponEffectsManager::GetGameManager()
      {
         return mGM.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtGame::GameManager* WeaponEffectsManager::GetGameManager() const
      {
         return mGM.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtCore::BatchIsector* WeaponEffectsManager::GetIsector() const
      {
         return mIsector.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::SetEffectTimeMax( float effectTimeMax )
      {
         mEffectTimeMax = effectTimeMax;
      }

      //////////////////////////////////////////////////////////////////////////
      float WeaponEffectsManager::GetEffectTimeMax() const
      {
         return mEffectTimeMax;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::SetMaxWeaponEffects( int maxEffectsAllowed )
      {
         mMaxWeaponEffects = maxEffectsAllowed;
      }

      //////////////////////////////////////////////////////////////////////////
      int WeaponEffectsManager::GetMaxWeaponEffects() const
      {
         return mMaxWeaponEffects;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::SetMaxMunitionEffects( int maxEffectsAllowed )
      {
         mMaxMunitionEffects = maxEffectsAllowed;
      }

      //////////////////////////////////////////////////////////////////////////
      int WeaponEffectsManager::GetMaxMunitionEffects() const
      {
         return mMaxMunitionEffects;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::SetRecycleTime( float recycleTime )
      {
         mRecycleTime = recycleTime;
      }

      //////////////////////////////////////////////////////////////////////////
      float WeaponEffectsManager::GetRecycleTime() const
      {
         return mRecycleTime;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffectsManager::ApplyWeaponEffect( SimCore::Actors::BaseEntity& owner,
         osg::Group* ownerNode,
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
                  sound->SetMaxDistance( effectsInfo.GetFireSoundMaxDistance() );
               }
            }

            // Put new effect into list before attaching it, in case addition fails.
            mEntityToEffectMap.insert( std::make_pair( objectId, newEffect.get() ) );
         }

         bool success = newEffect->Attach( ownerNode );

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
      bool WeaponEffectsManager::ApplyMunitionEffect(
         const osg::Vec3& weaponFirePoint,
         const osg::Vec3& intialVelocity,
         const SimCore::Actors::MunitionEffectsInfoActor& effectsInfo,
         MunitionEffectRequest& effectRequest )
      {
         // Avoid any unnecessary processing.
         if( ! mGM.valid()
            || (mMaxMunitionEffects > -1 && int(mMunitionEffects.size()) >= mMaxMunitionEffects) )
         {
            return false;
         }

         // Tracers are special case, since they rely on shaders.
         bool isTracerEffect = ! effectsInfo.GetTracerShaderName().empty();

         // Find a recyclable effect.
         MunitionEffect* effect = NULL;
         MunitionEffect* curEffect = NULL;
         MunitionEffectArray::iterator curEffectIter = mMunitionEffects.begin();
         MunitionEffectArray::iterator endEffectArray = mMunitionEffects.end();
         for( ; curEffectIter != endEffectArray; ++curEffectIter )
         {
            curEffect = curEffectIter->get();
            if( curEffect != NULL && ! curEffect->IsActive() )
            {
               if( isTracerEffect == (dynamic_cast<TracerEffect*>(curEffect) != NULL) )
               {
                  effect = curEffect;
                  break;
               }
            }
         }

         // Create a new effect if one could not be recycled.
         if( effect == NULL )
         {
            if( isTracerEffect )
            {
               TracerEffect* tracerEffect = new TracerEffect(
                  0.001, // spawn tracer at a near-zero size; it will be stretched to max size during updates.
                  effectsInfo.GetTracerThickness(),
                  effectsInfo.GetTracerShaderName(),
                  effectsInfo.GetTracerShaderGroup() );
               tracerEffect->SetMaxLength( effectsInfo.GetTracerLength() );

               effect = tracerEffect;
            }
            else
            {
               dtCore::RefPtr<osg::Node> modelNode
                  = osgDB::readNodeFile( effectRequest.GetMunitionModelFile() );

               if( modelNode.valid() )
               {
                  effect = new MunitionEffect;

                  osg::Group* group = dynamic_cast<osg::Group*>(effect->GetOSGNode());
                  if( group != NULL )
                  {
                     group->addChild( modelNode.get() );
                  }
               }
               else
               {
                  LOG_WARNING( ("Could not load munition effect model \""
                     + effectRequest.GetMunitionModelFile() + "\"") );
               }
            }

            if( effect != NULL )
            {
               effect->SetGameManager( mGM.get() );

               // Make sure the munition is visible in the scene
               mGM->GetScene().AddChild( effect );

               // Track this effect though out the life of the application
               mMunitionEffects.push_back( effect );
            }
         }

         bool success = effect != NULL;

         // Set position and velocity on the new/recycled tracer effect.
         if( success )
         {
            effect->SetPosition( weaponFirePoint );
            effect->SetVelocity( intialVelocity );

            // Determine if the munition has a light effect.
            if( effectRequest.IsFirstEffect() )
            {
               effect->SetLightName( effectsInfo.GetTracerLight() );

               // Default the tracer light in case the map did not set the light name.
               if( isTracerEffect && effect->GetLightName().empty() )
               {
                  effect->SetLightName( TracerEffect::DEFAULT_TRACER_LIGHT.Get() );
               }
            }
            else
            {
               effect->SetLightName( "" );
            }

            // Execute the effect.
            if( isTracerEffect )
            {

               // Determine how far the tracer has to go before it hits an obstacle.
               float tracerLifeTime = CalcTimeToImpact(
                  weaponFirePoint, intialVelocity,
                  effectsInfo.GetTracerLifeTime(), effectRequest.GetOwner() );

               // This also sets the initial orientation of the tracer.
               // Setup tracer to update and render.
               effect->Execute( tracerLifeTime );
            }
            else
            {
               // This may be a grenade effect of some kind.
               effect->Execute( 10.0f );
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponEffectsManager::AddMunitionEffectRequest( dtCore::RefPtr<MunitionEffectRequest>& effectRequest )
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
      unsigned WeaponEffectsManager::ClearMunitionEffectRequests()
      {
         unsigned lastSize = mTracerRequests.size();
         mTracerRequests.clear();
         return lastSize;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::UpdateMunitionEffectRequests( float timeDelta )
      {
         if( mTracerRequests.empty() )
         {
            return;
         }

         bool deleteEffectRequest = false;
         bool firstEffectRequestProcessed = false;
         MunitionEffectRequest* curRequest = NULL;
         MunitionEffectRequestList::iterator iter(--mTracerRequests.end());
         while( ! firstEffectRequestProcessed )
         {
            firstEffectRequestProcessed = iter == mTracerRequests.begin();
            deleteEffectRequest = false;
            curRequest = iter->get();

            if( curRequest == NULL ) // This should not happen
            {
               deleteEffectRequest = true;
               LOG_WARNING( "WeaponEffectsManager acquired a NULL MunitionEffectRequest" );
            }
            else
            {
               curRequest->Update( timeDelta );
               if( curRequest->IsEffectReady() )
               {
                  ApplyMunitionEffect(
                     curRequest->GetFirePoint(),
                     curRequest->GetVelocity(),
                     curRequest->GetEffectsInfo(),
                     *curRequest );

                  curRequest->Decrement();
               }

               if( curRequest->GetTotalEffects() < 1 )
               {
                  deleteEffectRequest = true;
               }
            }

            if( deleteEffectRequest )
            {
               MunitionEffectRequestList::iterator deleteIter = iter;
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
      unsigned WeaponEffectsManager::GetMunitionEffectCount() const
      {
         return mMunitionEffects.size();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::GetMunitionEffectActiveCount() const
      {
         unsigned activeCount = 0;
         unsigned limit = mMunitionEffects.size();
         for( unsigned i = 0; i < limit; ++i )
         {
            if( mMunitionEffects[i].valid() && mMunitionEffects[i]->IsActive() )
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

         UpdateMunitionEffectRequests( deltaTime );

         unsigned limit = mMunitionEffects.size();
         for( unsigned effect = 0; effect < limit; ++effect )
         {
            if( mMunitionEffects[effect].valid() )
            {
               mMunitionEffects[effect]->Update(deltaTime);
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
      unsigned WeaponEffectsManager::RecycleMunitionEffects()
      {
         unsigned recycleCount = 0;

         // Remove any unused, extraneous effects
         if( ! mMunitionEffects.empty() && mMaxMunitionEffects > -1
            && mMunitionEffects.size() > unsigned(mMaxMunitionEffects) )
         {
            MunitionEffectArray::iterator iter(--mMunitionEffects.end());
            MunitionEffectArray::iterator begin = mMunitionEffects.begin();

            for( ; iter != begin; )
            {
               // Remove all NULLs and unused effects that make the effects
               // vector longer than necessary.
               if( ! iter->valid()
                  || ( ! (*iter)->IsActive()
                     && mMunitionEffects.size() > unsigned(mMaxMunitionEffects) ) )
               {
                  // Ensure that the munition is still not held by the scene.
                  (*iter)->Emancipate();

                  MunitionEffectArray::iterator toDelete = iter;

                  --iter;
                  mMunitionEffects.erase(toDelete);
               }
               else
               {
                  --iter;
               }
            }
         }

         // DEBUG: std::cout << "Munition Effects:\t" << mMunitionEffects.size() << std::endl;

         return recycleCount;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponEffectsManager::Recycle()
      {
         return RecycleWeaponEffects() + RecycleMunitionEffects();
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
      void WeaponEffectsManager::ClearMunitionEffects()
      {
         if( mGM.valid() )
         {
            unsigned limit = mMunitionEffects.size();
            for( unsigned tracer = 0; tracer < limit; ++tracer )
            {
               mMunitionEffects[tracer]->Emancipate();
               mGM->GetScene().RemoveChild( mMunitionEffects[tracer].get() );
            }
         }

         if( ! mMunitionEffects.empty() )
         {
            mMunitionEffects.clear();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponEffectsManager::Clear()
      {
         ClearWeaponEffects();
         ClearMunitionEffects();
      }

      //////////////////////////////////////////////////////////////////////////
      float WeaponEffectsManager::CalcTimeToImpact(
         const osg::Vec3& weaponFirePoint, const osg::Vec3& initialVelocity,
         float maxTime, SimCore::Actors::BaseEntity* owner )
      {
         float speed = initialVelocity.length();
         if( speed == 0.0f )
         {
            return maxTime;
         }

         dtCore::RefPtr<WeaponEffectUtils::EntityNodeVisitor> subNodeVisitor
            = new WeaponEffectUtils::EntityNodeVisitor;
         dtCore::BatchIsector::SingleISector& isector = mIsector->EnableAndGetISector(0);
         osg::Vec3 endPoint( weaponFirePoint + (initialVelocity*maxTime) );
         isector.SetSectorAsLineSegment( weaponFirePoint, endPoint);

         if( mIsector->Update( osg::Vec3(0,0,0), true ) )
         {
            if( isector.GetNumberOfHits() > 0 )
            {
               // Check for self-collision?
               if( owner != NULL )
               {
                  const osg::Geode* geode = NULL;
                  dtCore::BatchIsector::HitList& hitList = isector.GetHitList();
                  dtCore::BatchIsector::HitList::iterator curHitIter = hitList.begin();
                  dtCore::BatchIsector::HitList::iterator endHitList = hitList.end();
                  for (int index = 0; curHitIter != endHitList; ++curHitIter, ++index)
                  {
                     const dtCore::BatchIsector::Hit& hit = *curHitIter;

                     // Get the drawable of the current hit.
                     const osg::NodePath& nodePath = hit.getNodePath();
                     geode = dynamic_cast<osg::Geode*>(nodePath.back());
                     if (geode == NULL)
                     {
                        continue;
                     }

                     // Determine if the hit drawable object is part of the
                     // entity that generated this tracer effect.
                     subNodeVisitor->SetGeodeToFind( *geode );
                     owner->GetOSGNode()->traverse( *subNodeVisitor );
                     bool foundNode = subNodeVisitor->HasFoundNode();
                     subNodeVisitor->Reset();

                     // If the object is part of the entity hierarchy, then
                     // continue searching for other hit points that are not
                     // hits on the entity itself.
                     if( foundNode )
                     {
                        continue;
                     }

                     // Point was found. Get it and exit the loop.
                     isector.GetHitPoint( endPoint, index );
                     break;
                  }
               }
               else
               {
                  // Get the first point, not checking for self-collision.
                  isector.GetHitPoint( endPoint );
               }
            }

            maxTime = (endPoint-weaponFirePoint).length()/speed;

            // Clear for next use
            mIsector->Reset();
         }

         return maxTime;
      }

   }
}
