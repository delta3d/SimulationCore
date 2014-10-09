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
#include <prefix/SimCorePrefix.h>

#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/invokable.h>

#include <dtAudio/audiomanager.h>

#include <dtCore/camera.h>
#include <dtCore/transform.h>

#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/propertymacros.h>

#include <dtABC/application.h>

#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/TimedDeleterComponent.h>

#include <cmath>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(DetonationMunitionType);
      DetonationMunitionType DetonationMunitionType::UNKNOWN("UNKNOWN");
      DetonationMunitionType DetonationMunitionType::LARGE_EXPLOSION("LARGE EXPLOSION");
      DetonationMunitionType DetonationMunitionType::MEDIUM_EXPLOSION("MEDIUM EXPLOSION");
      DetonationMunitionType DetonationMunitionType::SMALL_EXPLOSION("SMALL EXPLOSION");
      DetonationMunitionType DetonationMunitionType::LARGE_BULLET("LARGE BULLET");
      DetonationMunitionType DetonationMunitionType::SMALL_BULLET("SMALL BULLET");
      DetonationMunitionType DetonationMunitionType::SHORT_SMOKE("SHORT SMOKE");
      DetonationMunitionType DetonationMunitionType::LONG_SMOKE("LONG SMOKE");
      DetonationMunitionType DetonationMunitionType::DPICM("DPICM");
      DetonationMunitionType DetonationMunitionType::ILLUMINATION("ILLUMINATION");

      //////////////////////////////////////////////////////////////
      // Actor Proxy code
      //////////////////////////////////////////////////////////////
      const std::string DetonationActorProxy::CLASS_NAME("SimCore::Actors::DetonationActor");

      DetonationActorProxy::DetonationActorProxy()
      {
         SetClassName(CLASS_NAME);
      }

      ///////////////////////////////////////////////////////////////////////
      DetonationActorProxy::~DetonationActorProxy()
      {

      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::BuildInvokables()
      {
         dtGame::GameActorProxy::BuildInvokables();

         AddInvokable(*new dtGame::Invokable("HandleDetonationActorTimers",
            dtUtil::MakeFunctor(&DetonationActorProxy::HandleDetonationActorTimers, this)));

         RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_TIMER_ELAPSED, "HandleDetonationActorTimers");
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         DetonationActor* da = NULL;
         GetDrawable(da);

         static const dtUtil::RefString groupImpactEffects("Impact Effects");

         AddProperty(new dtCore::BooleanActorProperty("Enable Physics","Enable Physics",
            dtCore::BooleanActorProperty::SetFuncType(da, &DetonationActor::SetPhysicsEnabled),
            dtCore::BooleanActorProperty::GetFuncType(da, &DetonationActor::IsPhysicsEnabled),
            "Sets whether the detonation actor should have physics particles."));

         AddProperty(new dtCore::FloatActorProperty("Explosion Timer Seconds", "Explosion Timer Seconds",
            dtCore::FloatActorProperty::SetFuncType(da, &DetonationActor::SetExplosionTimerSecs),
            dtCore::FloatActorProperty::GetFuncType(da, &DetonationActor::GetExplosionTimerSecs),
            "Sets the number of seconds an explosion will render"));

         AddProperty(new dtCore::FloatActorProperty("Delete Actor Timer Seconds", "Delete Actor Timer Seconds",
            dtCore::FloatActorProperty::SetFuncType(da, &DetonationActor::SetDeleteActorTimerSecs),
            dtCore::FloatActorProperty::GetFuncType(da, &DetonationActor::GetDeleteActorTimerSecs),
            "Sets the number of seconds after smoke is rendered for an actor to be deleted"));

         typedef dtCore::PropertyRegHelper<DetonationActorProxy&, DetonationActor> RegHelperType;
         RegHelperType propReg(*this, da, groupImpactEffects);

         // GROUND IMPACT PROPERTIES
         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::PARTICLE_SYSTEM, GroundImpactEffect, "Ground Impact Effect", 
                                          "The particle system to spawn on ground impact.", RegHelperType, propReg);

         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::SOUND, GroundImpactSound, "Ground Impact Sound", 
                                          "The sound to play on ground impact.", RegHelperType, propReg);

         AddProperty(new dtCore::StringActorProperty("Ground Impact Light", "Ground Impact Light",
            dtCore::StringActorProperty::SetFuncType(da, &DetonationActor::SetGroundImpactLight ),
            dtCore::StringActorProperty::GetFuncType(da, &DetonationActor::GetGroundImpactLight ),
            "The name of light effect for the ground impact effect", groupImpactEffects));

         // ENTITY IMPACT PROPERTIES

         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::PARTICLE_SYSTEM, EntityImpactEffect, "Entity Impact Effect", 
            "The particle system to spawn on entity impact.", RegHelperType, propReg);

         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::SOUND, EntityImpactSound, "Entity Impact Sound", 
            "The sound to play on entity impact.", RegHelperType, propReg);

         AddProperty(new dtCore::StringActorProperty("Entity Impact Light", "Entity Impact Light",
            dtCore::StringActorProperty::SetFuncType(da, &DetonationActor::SetEntityImpactLight ),
            dtCore::StringActorProperty::GetFuncType(da, &DetonationActor::GetEntityImpactLight ),
            "The name of light effect for the entity impact effect", groupImpactEffects));

         // HUMAN IMPACT PROPERTIES

         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::PARTICLE_SYSTEM, HumanImpactEffect, "Human Impact Effect", 
            "The particle system to spawn on human impact.", RegHelperType, propReg);

         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::SOUND, HumanImpactSound, "Human Impact Sound", 
            "The sound to play on human impact.", RegHelperType, propReg);

         AddProperty(new dtCore::StringActorProperty("Human Impact Light", "Human Impact Light",
            dtCore::StringActorProperty::SetFuncType(da, &DetonationActor::SetHumanImpactLight ),
            dtCore::StringActorProperty::GetFuncType(da, &DetonationActor::GetHumanImpactLight ),
            "The name of light effect for the human impact effect", groupImpactEffects));

         //smoke effect
         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::PARTICLE_SYSTEM, SmokeEffect, "Smoke Effect", 
            "The particle system used for the detonation smoke effect.", RegHelperType, propReg);

         DT_REGISTER_PROPERTY(SmokeLifeTime, "The lifetime in seconds of the smoke effect.", RegHelperType, propReg);

      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::OnEnteredWorld()
      {
         DetonationActor* da = GetDrawable<DetonationActor>();
         float time = da->GetDelayTime();
         GetGameManager()->SetTimer("PlayDetonationSoundTimer", this, time);

      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::HandleDetonationActorTimers(const dtGame::TimerElapsedMessage& timeMsg)
      {
         if(timeMsg.GetTimerName() == "PlayDetonationSoundTimer")
         {
            GetDrawable<DetonationActor>()->PlaySound();
         }
         else if(timeMsg.GetTimerName() == "ExplosionRendered")
         {
            DetonationActor* detonation = NULL;
            GetDrawable(detonation);
            detonation->RenderSmoke();
            GetGameManager()->SetTimer("SmokeRendered", this, detonation->GetSmokeLifeTime());
         }
         else if(timeMsg.GetTimerName() == "SmokeRendered")
         {
            GetDrawable<DetonationActor>()->StopRenderingSmoke();
         }
         else
            LOG_ERROR("Received a timer message of the correct type, but wrong name");
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::ClearTimers()
      {
         GetGameManager()->ClearTimer("PlayDetonationSoundTimer", this);
         GetGameManager()->ClearTimer("ExplosionRendered", this);
         GetGameManager()->ClearTimer("SmokeRendered", this);
         //GetGameManager()->ClearTimer("DeleteActor", this);
      }

      //////////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////////

      DetonationActor::DetonationActor(dtGame::GameActorProxy& owner)
      : IGActor(owner)
      , mSmokeLifeTime(0.0f)
      , mCurrentImpact(IMPACT_TERRAIN)
      , mDelayTime(3.0f)
      , mRenderExplosionTimerSecs(2.0f)
      , mDeleteActorTimerSecs(5.0f)
      , mUsesPhysics(false)
      , mCurrentLightName()
      , mLightImpactGround()
      , mLightImpactEntity()
      , mLightImpactHuman()
      , mCollidedMaterial(NULL)
      , mExplosionSystem(new dtCore::ParticleSystem())
      , mSmokeSystem(new dtCore::ParticleSystem())
      , mSound()
      {
         AddChild(mExplosionSystem.get());
         AddChild(mSmokeSystem.get());
      }

      ///////////////////////////////////////////////////////////////////////
      DetonationActor::~DetonationActor()
      {
      }

      ///////////////////////////////////////////////////////////////////////
      dtCore::ParticleSystem* DetonationActor::GetSmokeParticleSystem()
      {
         return mSmokeSystem.get();
      }

      ///////////////////////////////////////////////////////////////////////
      const dtCore::ParticleSystem* DetonationActor::GetSmokeParticleSystem() const
      {
         return mSmokeSystem.get();
      }

      void DetonationActor::OnRemovedFromWorld()
      {
         if(mSound.valid())
         {
            dtAudio::AudioManager::GetInstance().FreeSound(mSound.get());
            RemoveChild(mSound.get());
            mSound = NULL;
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::OnEnteredWorld()
      {
         SetImpactEffects();

         // Register explosion particle effects
         if( mExplosionSystem.valid() )
         {
            Components::ParticleInfoAttributeFlags attrs = { true, false };
            RegisterParticleSystem( *mExplosionSystem, &attrs );
         }

         // Register smoke particle effects
         if( mSmokeSystem.valid() )
         {
            Components::ParticleInfoAttributeFlags attrs = { true, false };
            RegisterParticleSystem( *mSmokeSystem, &attrs );
         }

         if(!mCurrentLightName.empty())
         {
            AddDynamicLight(mCurrentLightName);
         }

         ///////////////////////////////////////////////////////////////////////
         // Add physics particle systems to the detonation
         if(mUsesPhysics && mCollidedMaterial != NULL)
         {
            std::string particleSystems[5];
            particleSystems[0] = mCollidedMaterial->GetPhysicsParticleLookupStringOne();
            particleSystems[1] = mCollidedMaterial->GetPhysicsParticleLookupStringTwo();
            particleSystems[2] = mCollidedMaterial->GetPhysicsParticleLookupStringThree();
            particleSystems[3] = mCollidedMaterial->GetPhysicsParticleLookupStringFour();
            particleSystems[4] = mCollidedMaterial->GetPhysicsParticleLookupStringFive();
            for(int i = 0 ; i < 5 ; ++i)
            {
               if(!particleSystems[i].empty())
               {
                  std::vector<dtCore::ActorProxy*> toFill;
                  GetGameActorProxy().GetGameManager()->FindPrototypesByName( particleSystems[i] ,toFill);
                  if(toFill.size())
                  {
                     dtCore::RefPtr<dtCore::ActorProxy> ourActualActorProxy = GetGameActorProxy().GetGameManager()->CreateActorFromPrototype(toFill.front()->GetId());
                     if( ourActualActorProxy != NULL )
                     {
                        dtCore::Transform detonationTransform;
                        GetTransform(detonationTransform);
                        PhysicsParticleSystemActor* ourSpewingParticleSystemOfDoom = dynamic_cast<PhysicsParticleSystemActor*>(ourActualActorProxy->GetDrawable());
                        ourSpewingParticleSystemOfDoom->SetTransform(detonationTransform);
                        ourSpewingParticleSystemOfDoom->ToggleEmitter(true);
                        GetGameActorProxy().GetGameManager()->AddActor(ourSpewingParticleSystemOfDoom->GetGameActorProxy(), false, false);
                     }
                  }
               }
            }
         }
         
         ///////////////////////////////////////////////////////////////////////

         RenderDetonation();
         GetGameActorProxy().GetGameManager()->SetTimer("ExplosionRendered", &GetGameActorProxy(), mRenderExplosionTimerSecs);

         // Register to delete after X time to make sure the detonation goes away when it should.
         SimCore::Components::TimedDeleterComponent* timeDeleteComp;
         GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::TimedDeleterComponent::DEFAULT_NAME,timeDeleteComp);
         if(timeDeleteComp != NULL)
         {
            timeDeleteComp->AddId(GetUniqueId(), mRenderExplosionTimerSecs + mSmokeLifeTime + mDeleteActorTimerSecs);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::SetImpactEffects()
      {
         //we first check what we collided with and select the appropriate particle system
         if(mCurrentImpact == IMPACT_ENTITY && !GetEntityImpactEffect().IsEmpty())
         {
            LoadParticleSystem(GetEntityImpactEffect(), mExplosionSystem);
            LoadSoundFile(GetEntityImpactSound(), mSound);
            mCurrentLightName = mLightImpactEntity;
         }
         else if (mCurrentImpact == IMPACT_HUMAN && !GetHumanImpactEffect().IsEmpty())
         {
            LoadParticleSystem(GetHumanImpactEffect(), mExplosionSystem);
            LoadSoundFile(GetHumanImpactSound(), mSound); 
            mCurrentLightName = mLightImpactHuman;
         }
         //we default to the terrain impact
         else if (!GetGroundImpactEffect().IsEmpty())
         {
            LoadParticleSystem(GetGroundImpactEffect(), mExplosionSystem);
            LoadSoundFile(GetGroundImpactSound(), mSound);
            mCurrentLightName = mLightImpactGround;
         }

         if(!mSmokeEffect.IsEmpty())
         {
            LoadSmokeFile(mSmokeEffect);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::PlaySound()
      {
         if(mSound.valid())
         {
            //////////////////////////////////////////////
            // ALL OF THIS IS DONE IN LOAD SOUND FILE
            // BY ADDING SOUND FILE AS A CHILD
            dtCore::Transform cform;
            GetGameActorProxy().GetGameManager()->GetApplication().GetCamera()->GetTransform(cform);

            dtCore::Transform xform;
            mSound->GetTransform(xform);

            osg::Vec3 trans;
            xform.GetTranslation(trans);
            osg::Vec3 childTrans;
            cform.GetTranslation(childTrans);
            xform.Set(trans, childTrans, osg::Vec3(0.0f, 0.0f, 1.0f));//ur position, what u look at,up vector 001
            mSound->SetTransform(xform);
            //////////////////////////////////////////////
            mSound->Play();
         }
         //else
         //   LOG_WARNING("Cannot play sound. No detonation sound file loaded.");
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::LoadSoundFile( const dtCore::ResourceDescriptor& resource, dtCore::RefPtr<dtAudio::Sound>& soundIn)
      {
         if(!resource.IsEmpty())
         {
            try 
            {
               dtCore::Project& proj = dtCore::Project::GetInstance();
               std::string filename = proj.GetResourcePath(resource);

               if(soundIn != NULL)
               {
                  dtAudio::AudioManager::GetInstance().FreeSound(soundIn.get());
               }

               soundIn = NULL;
               soundIn = dtAudio::AudioManager::GetInstance().NewSound();

               if(!soundIn.valid())
                  throw dtGame::InvalidParameterException(
                 "Failed to create the detonation sound pointer", __FILE__, __LINE__);

               soundIn->LoadFile(filename.c_str());
               AddChild(soundIn.get());
               dtCore::Transform xform;
               soundIn->SetTransform(xform, dtCore::Transformable::REL_CS);
            }
            catch(dtUtil::Exception& e)
            {
               e.LogException();
            }
         }
         else
         {
            LOG_ERROR("Attempting to load invalid sound file.");
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::LoadParticleSystem(const dtCore::ResourceDescriptor& resource, dtCore::RefPtr<dtCore::ParticleSystem>& particleSysIn)
      {
         if(!resource.IsEmpty())
         {
            try 
            {
               dtCore::Project& proj = dtCore::Project::GetInstance();
               std::string filename = proj.GetResourcePath(resource);
               particleSysIn->LoadFile(filename);
            }
            catch(dtUtil::Exception& e)
            {
               e.LogException();
            }
         }
         else
         {
            LOG_ERROR("Attempting to load invalid detonation particle effect.");
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::LoadSmokeFile(const dtCore::ResourceDescriptor& resource)
      {
         LoadParticleSystem(resource, mSmokeSystem);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::RenderDetonation()
      {
         mExplosionSystem->SetEnabled(true);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::RenderSmoke()
      {
         if(mSmokeSystem.valid())
         {
            StartSmokeEffect(*mSmokeSystem);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::StartSmokeEffect(dtCore::ParticleSystem& particles)
      {
         if(mSmokeLifeTime > 0.00001)
         {
            particles.SetEnabled(true);

            if(mUsesPhysics && mCollidedMaterial != NULL)
            {
               dtCore::ParticleSystem::LayerList ourList = particles.GetAllLayers();
               dtCore::ParticleSystem::LayerList::iterator iter = ourList.begin();
               for(; iter != ourList.end(); ++iter)
               {
                  (*iter).GetParticleSystem().getDefaultParticleTemplate().setColorRange(osgParticle::rangev4(mCollidedMaterial->GetBaseColorvalue(),
                                                                                                              mCollidedMaterial->GetHighlighteColorvalue()));
               }
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::StopRenderingSmoke()
      {
         mSmokeSystem->SetEnabled(false);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::CalculateDelayTime(const osg::Vec3 &position)
      {
         dtCore::Transform xform;
         GetTransform(xform);
         osg::Vec3 detonationPos;
         xform.GetTranslation(detonationPos);

         float distance =
            sqrtf(std::abs((position[0] - detonationPos[0]) * (position[0] - detonationPos[0]) +
                       (position[1] - detonationPos[1]) * (position[1] - detonationPos[1]) +
                       (position[2] - detonationPos[2]) * (position[2] - detonationPos[2])));

         //a rough approximation to the speed of sound in meters per second
         mDelayTime = distance / 350.0f;
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::AddDynamicLight(const std::string& lightName)
      {
         if(!mCurrentLightName.empty())
         {
            SimCore::Components::RenderingSupportComponent* renderComp;
            GetGameActorProxy().GetGameManager()->GetComponentByName(
                  SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                  renderComp);

            if(renderComp != NULL)
            {
               SimCore::Components::RenderingSupportComponent::DynamicLight* dl =
                  renderComp->AddDynamicLightByPrototypeName(lightName);
               dl->mTarget = this;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void DetonationActor::SetImpactType(IMPACT_TYPE impact)
      {
         mCurrentImpact = impact;
      }

      //////////////////////////////////////////////////////////////////////////
      DetonationActor::IMPACT_TYPE DetonationActor::GetImpactType() const
      {
         return mCurrentImpact;
      }

      //////////////////////////////////////////////////////////////////////////
      void DetonationActor::SetGroundImpactLight( const std::string& lightName ) { mLightImpactGround = lightName; }
      const std::string& DetonationActor::GetGroundImpactLight() const { return mLightImpactGround; }

      //////////////////////////////////////////////////////////////////////////
      void DetonationActor::SetEntityImpactLight( const std::string& lightName ) { mLightImpactEntity = lightName; }
      const std::string& DetonationActor::GetEntityImpactLight() const { return mLightImpactEntity; }

      //////////////////////////////////////////////////////////////////////////
      void DetonationActor::SetHumanImpactLight( const std::string& lightName ) { mLightImpactHuman = lightName; }
      const std::string& DetonationActor::GetHumanImpactLight() const { return mLightImpactHuman; }

   }
}
