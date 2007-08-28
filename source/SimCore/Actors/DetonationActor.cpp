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
#include <prefix/dvteprefix-src.h>

#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/NxAgeiaParticleSystemActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/invokable.h>

#include <dtAudio/audiomanager.h>

#include <dtCore/camera.h>

#include <dtDAL/enginepropertytypes.h>

#include <dtABC/application.h>

#include <SimCore/Components/ParticleManagerComponent.h>

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
      DetonationActorProxy::DetonationActorProxy()
      {
         SetClassName("SimCore::Actors::DetonationActor");
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
            dtDAL::MakeFunctor(*this, &DetonationActorProxy::HandleDetonationActorTimers)));

         RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_TIMER_ELAPSED, "HandleDetonationActorTimers");
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         DetonationActor &da = static_cast<DetonationActor&>(GetGameActor());

         AddProperty(new dtDAL::BooleanActorProperty("Enable Physics","Enable Physics", 
            dtDAL::MakeFunctor(da, &DetonationActor::SetPhysicsEnabled), 
            dtDAL::MakeFunctorRet(da, &DetonationActor::IsPhysicsEnabled), 
            "Sets whether the detonation actor should have physics particles."));

         AddProperty(new dtDAL::FloatActorProperty("Lingering Shot Seconds", "Lingering Shot Seconds", 
            dtDAL::MakeFunctor(da, &DetonationActor::SetLingeringSmokeSecs), 
            dtDAL::MakeFunctorRet(da, &DetonationActor::GetLingeringSmokeSecs), 
            "Sets the number of seconds that smoke will linger around an impact area"));

         AddProperty(new dtDAL::FloatActorProperty("Explosion Timer Seconds", "Explosion Timer Seconds", 
            dtDAL::MakeFunctor(da, &DetonationActor::SetExplosionTimerSecs), 
            dtDAL::MakeFunctorRet(da, &DetonationActor::GetExplosionTimerSecs), 
            "Sets the number of seconds an explosion will render"));
         
         AddProperty(new dtDAL::FloatActorProperty("Delete Actor Timer Seconds", "Delete Actor Timer Seconds", 
            dtDAL::MakeFunctor(da, &DetonationActor::SetDeleteActorTimerSecs), 
            dtDAL::MakeFunctorRet(da, &DetonationActor::GetDeleteActorTimerSecs), 
            "Sets the number of seconds after smoke is rendered for an actor to be deleted"));

         AddProperty(new dtDAL::FloatActorProperty("Max Sound Distance", "Max Sound Distance", 
            dtDAL::MakeFunctor(da, &DetonationActor::SetMaximumSoundDistance), 
            dtDAL::MakeFunctorRet(da, &DetonationActor::GetMaximumSoundDistance), 
            "Sets the maximum number of meters that a sound will clip"));

         AddProperty(new dtDAL::FloatActorProperty("Min Sound Distance", "Min Sound Distance", 
            dtDAL::MakeFunctor(da, &DetonationActor::SetMinimumSoundDistance), 
            dtDAL::MakeFunctorRet(da, &DetonationActor::GetMinimumSoundDistance), 
            "Sets the minimum number of meters that a sound will clip"));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, 
            "Smoke Particle System", "Smoke Particle System", dtDAL::MakeFunctor(da, &DetonationActor::LoadSmokeFile), 
            "Loads the particle system for this detonation to use for smoke"));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, 
            "Detonation Particle System", "Detonation Particle System", dtDAL::MakeFunctor(da, &DetonationActor::LoadDetonationFile), 
            "Loads the particle system for this detonation to use for the explosion"));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND, 
            "Explosion Sound", "Explosion Sound", dtDAL::MakeFunctor(da, &DetonationActor::LoadSoundFile), 
            "Loads the sound for this detonation to use"));
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::OnEnteredWorld()
      {
         DetonationActor &da = static_cast<DetonationActor&>(GetGameActor());
         float time = da.GetDelayTime();
         GetGameManager()->SetTimer("PlayDetonationSoundTimer", this, time);

         dtCore::ParticleSystem* explosionSystem = da.GetExplosionParticleSystem();
         if( explosionSystem != NULL )
         {
            IG::ParticleInfo::AttributeFlags attrs = { true, false };
            da.RegisterParticleSystem( *explosionSystem, &attrs );
         }
               
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::HandleDetonationActorTimers(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::INFO_TIMER_ELAPSED)
         {
            const dtGame::TimerElapsedMessage &timeMsg = static_cast<const dtGame::TimerElapsedMessage&>(msg);
            if(timeMsg.GetTimerName() == "PlayDetonationSoundTimer")
            {
                static_cast<DetonationActor&>(GetGameActor()).PlaySound();
            }
            else if(timeMsg.GetTimerName() == "ExplosionRendered")
            {
               static_cast<DetonationActor&>(GetGameActor()).RenderSmoke();
            }
            else if(timeMsg.GetTimerName() == "SmokeRendered")
            {
               static_cast<DetonationActor&>(GetGameActor()).StopRenderingSmoke();
            } 
            else if(timeMsg.GetTimerName() == "DeleteActor")
            {
               DetonationActor &da = static_cast<DetonationActor&>(GetGameActor());
               if(da.GetSound() != NULL && da.GetSound()->IsPlaying())
                  da.GetSound()->Stop();

               dtCore::RefPtr< dtCore::ParticleSystem > ps = da.GetExplosionParticleSystem();
               if(ps.valid())
                  da.UnregisterParticleSystem(*ps);

               ps = da.GetSmokeParticleSystem();
               if(ps.valid())
                  da.UnregisterParticleSystem(*ps);

               GetGameManager()->DeleteActor(*this);
            }
            else
               LOG_ERROR("Received a timer message of the correct type, but wrong name");
         }
         else
            LOG_ERROR("The invokable received a message that wasn't the correct type");
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::SetDetonationProperties(
         float lingerTime,
         float minSoundDistance,
         float maxSoundDistance,
         const std::string& detonationFile,
         const std::string& soundFile,
         const std::string& smokeFile
         )
      {
         dtDAL::FloatActorProperty* floatProp = NULL;

         GetProperty("Lingering Shot Seconds", floatProp);
         if(floatProp != NULL )
            floatProp->SetValue(lingerTime);

         GetProperty("Max Sound Distance", floatProp);
         if(floatProp != NULL )
            floatProp->SetValue(maxSoundDistance);

         GetProperty("Min Sound Distance", floatProp);
         if(floatProp != NULL )
            floatProp->SetValue(minSoundDistance);

         dtDAL::StringActorProperty* stringProp =  NULL;
         GetProperty("Detonation Particle System", stringProp);
         if(stringProp != NULL && !detonationFile.empty() )
            stringProp->SetValue(detonationFile);

         GetProperty("Explosion Sound", stringProp);
         if(stringProp != NULL && !soundFile.empty() )
            stringProp->FromString(soundFile);

         GetProperty("Smoke Particle System");
         if(stringProp != NULL && !smokeFile.empty() )
            stringProp->FromString(smokeFile);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActorProxy::ClearTimers()
      {
         GetGameManager()->ClearTimer("PlayDetonationSoundTimer", this);
         GetGameManager()->ClearTimer("ExplosionRendered", this);
         GetGameManager()->ClearTimer("SmokeRendered", this);
         GetGameManager()->ClearTimer("DeleteActor", this);
      }

      //////////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////////

      DetonationActor::DetonationActor(dtGame::GameActorProxy &proxy) : 
         IGActor(proxy), 
         mExplosionSystem(new dtCore::ParticleSystem),
         mSmokeSystem(new dtCore::ParticleSystem),
         mDelayTime(3.0f),
         mLingeringSmokeSecs(0.0f), 
         mRenderExplosionTimerSecs(2.0f), 
         mDeleteActorTimerSecs(20.0f),
         mUsesPhysics(false)
      {
         AddChild(mExplosionSystem.get());
         AddChild(mSmokeSystem.get());
      }

      ///////////////////////////////////////////////////////////////////////
      DetonationActor::~DetonationActor()
      {
         if(mSound.valid())
         {
            RemoveChild(mSound.get());
            mSound.release();
//            dtAudio::Sound *sound = mSound.release();
//            dtAudio::AudioManager::GetInstance().FreeSound(sound);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      dtCore::ParticleSystem* DetonationActor::GetExplosionParticleSystem()
      {
         return mExplosionSystem.get();
      }

      ///////////////////////////////////////////////////////////////////////
      const dtCore::ParticleSystem* DetonationActor::GetExplosionParticleSystem() const
      {
         return mExplosionSystem.get();
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

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::OnEnteredWorld()
      {
         if( mExplosionSystem.valid() )
            RegisterParticleSystem(*mExplosionSystem);

         if( mSmokeSystem.valid() )
            RegisterParticleSystem(*mSmokeSystem);

         ///////////////////////////////////////////////////////////////////////
         // Add physics particle systems to the detonation
#ifdef AGEIA_PHYSICS        
         if(mUsesPhysics && mCollidedMaterial != NULL)
         {
            std::string particleSystems[5];
            particleSystems[0] = mCollidedMaterial->GetPhysicsParticleLookupStringOne();
            particleSystems[1] = mCollidedMaterial->GetPhysicsParticleLookupStringTwo();
            particleSystems[2] = mCollidedMaterial->GetPhysicsParticleLookupStringThr();
            particleSystems[3] = mCollidedMaterial->GetPhysicsParticleLookupStringFour();
            particleSystems[4] = mCollidedMaterial->GetPhysicsParticleLookupStringFive();
            for(int i = 0 ; i < 5 ; ++i)
            {
               if(!particleSystems[i].empty())
               {
                  std::vector<dtDAL::ActorProxy*> toFill;
                  GetGameActorProxy().GetGameManager()->FindPrototypesByName( particleSystems[i] ,toFill);
                  if(toFill.size())
                  {
                     dtCore::RefPtr<dtDAL::ActorProxy> ourActualActorProxy = GetGameActorProxy().GetGameManager()->CreateActorFromPrototype(toFill.front()->GetId());
                     if( ourActualActorProxy != NULL )
                     {
                        dtCore::Transform detonationTransform;
                        GetTransform(detonationTransform);
                        NxAgeiaParticleSystemActor* ourSpewingParticleSystemOfDoom = dynamic_cast<NxAgeiaParticleSystemActor*>(ourActualActorProxy->GetActor());                           
                        ourSpewingParticleSystemOfDoom->SetTransform(detonationTransform);
                        ourSpewingParticleSystemOfDoom->ToggleEmitter(true);
                        GetGameActorProxy().GetGameManager()->AddActor(ourSpewingParticleSystemOfDoom->GetGameActorProxy(), false, false);
                     }
                  }
               }
            }
         }
#endif     
         ///////////////////////////////////////////////////////////////////////

         RenderDetonation();
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

            xform.SetLookAt(xform.GetTranslation(), cform.GetTranslation(), osg::Vec3(0.0f, 0.0f, 1.0f));//ur position, what u look at,up vector 001
            mSound->SetTransform(xform);
            //////////////////////////////////////////////
            mSound->Play();
         }
         //else
         //   LOG_WARNING("Cannot play sound. No detonation sound file loaded.");
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::LoadSoundFile(const std::string &fileName)
      {
         if(fileName.empty())
         {
            //LOG_ERROR("Cannot load an empty sound file. Ignoring.");
            return;
         }
         
         if(mSound != NULL && mSound->GetFilename() != NULL)
            mSound->UnloadFile();
         
         mSound = NULL;
         mSound = dtAudio::AudioManager::GetInstance().NewSound();
         
         if(!mSound.valid())
            throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
            "Failed to create the detonation sound pointer", __FILE__, __LINE__);

         mSound->LoadFile(fileName.c_str());
         mSound->ListenerRelative(true);
         AddChild(mSound.get());
         dtCore::Transform xform;
         mSound->SetTransform(xform, dtCore::Transformable::REL_CS);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::LoadDetonationFile(const std::string &fileName)
      {
         if(fileName.empty())
         {
            LOG_ERROR("Cannot load an empty detonation file. Ignoring.");
            return;
         }

         mExplosionSystem->LoadFile(fileName);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::LoadSmokeFile(const std::string &fileName)
      {
         if(fileName.empty())
         {
            //LOG_ERROR("Cannot load an empty smoke file. Ignoring.");
            return;
         }

         mSmokeSystem->LoadFile(fileName);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::RenderDetonation()
      {
         mExplosionSystem->SetEnabled(true);
         GetGameActorProxy().GetGameManager()->SetTimer("ExplosionRendered", &GetGameActorProxy(), mRenderExplosionTimerSecs);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::RenderSmoke()
      {
         if (mLingeringSmokeSecs > 0.00001)
         {
            mSmokeSystem->SetEnabled(true);

            if(mUsesPhysics && mCollidedMaterial != NULL)
            {
               std::list<dtCore::ParticleLayer> ourList = mSmokeSystem->GetAllLayers();               
               std::list<dtCore::ParticleLayer>::iterator iter = ourList.begin();
               for(; iter != ourList.end(); ++iter)
               {
                  (*iter).GetParticleSystem().getDefaultParticleTemplate().setColorRange(osgParticle::rangev4(mCollidedMaterial->GetBaseColorvalue(), 
                                                                                                              mCollidedMaterial->GetHighlighteColorvalue()));
               }
            }
            
            GetGameActorProxy().GetGameManager()->SetTimer("SmokeRendered", &GetGameActorProxy(), mLingeringSmokeSecs);
         }
		   else 
		   {
	         GetGameActorProxy().GetGameManager()->SetTimer("DeleteActor", &GetGameActorProxy(), mDeleteActorTimerSecs);
		   }
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::StopRenderingSmoke()
      {
         mSmokeSystem->SetEnabled(false);
         GetGameActorProxy().GetGameManager()->SetTimer("DeleteActor", &GetGameActorProxy(), mDeleteActorTimerSecs);
      }

      ///////////////////////////////////////////////////////////////////////
      void DetonationActor::CalculateDelayTime(const osg::Vec3 &position)
      {
         dtCore::Transform xform;
         GetTransform(xform);
         osg::Vec3 detonationPos = xform.GetTranslation();

         float distance = 
            sqrtf(fabs((position[0] - detonationPos[0]) * (position[0] - detonationPos[0]) +
                       (position[1] - detonationPos[1]) * (position[1] - detonationPos[1]) +
                       (position[2] - detonationPos[2]) * (position[2] - detonationPos[2])));

         mDelayTime = distance / 350.0f;
      }
   }
}
