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
#include <prefix/dvteprefix-src.h>
#include <dtActors/particlesystemactorproxy.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/nodecollector.h>
#include <dtCore/particlesystem.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <SimCore/Actors/MissileActor.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>

#include <osg/MatrixTransform>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      MissileActorProxy::MissileActorProxy()
      {
         SetClassName("SimCore::Actors::MissileActor");
      }

      //////////////////////////////////////////////////////////
      MissileActorProxy::~MissileActorProxy()
      {

      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::BuildInvokables()
      {
         PlatformActorProxy::BuildInvokables();

         MissileActor &actor = static_cast<MissileActor&>(GetGameActor());

         AddInvokable(*new dtGame::Invokable("ScheduleSmokeTrailDelete", 
            dtDAL::MakeFunctor(actor, &MissileActor::ScheduleSmokeTrailDelete)));

         RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_ACTOR_DELETED, "ScheduleSmokeTrailDelete");
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::BuildPropertyMap()
      {
         PlatformActorProxy::BuildPropertyMap();

         AddProperty(new dtDAL::BooleanActorProperty("Smoke Trail Enabled", "Smoke Trail Enabled", 
            dtDAL::MakeFunctor(static_cast<MissileActor&>(GetGameActor()), &MissileActor::SetSmokeTrailEnabled), 
            dtDAL::MakeFunctorRet(static_cast<MissileActor&>(GetGameActor()), &MissileActor::IsSmokeTrailEnabled), 
            "Turns the smoke trail particle system on or off"));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, 
            "Smoke Trail File", "Smoke Trail File", dtDAL::MakeFunctor(*this, &MissileActorProxy::LoadSmokeTrailFile), 
            "Loads the file of the missile smoke trail", "Particles"));

         AddProperty(new dtDAL::BooleanActorProperty("Flame Enabled", "Flame Enabled", 
            dtDAL::MakeFunctor(static_cast<MissileActor&>(GetGameActor()), &MissileActor::SetFlameEnabled), 
            dtDAL::MakeFunctorRet(static_cast<MissileActor&>(GetGameActor()), &MissileActor::IsFlameEnabled), 
            "Turns the flame particle system on or off"));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, 
            "Flame File", "Flame File", dtDAL::MakeFunctor(*this, &MissileActorProxy::LoadFlameFile), 
            "Loads the file of the missile thruster flame", "Particles"));
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::LoadSmokeTrailFile(const std::string &fileName)
      {
         MissileActor *ma = static_cast<MissileActor*>(GetActor());

         ma->LoadSmokeTrailFile(fileName); 
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::LoadFlameFile(const std::string &fileName)
      {
         MissileActor *ma = static_cast<MissileActor*>(GetActor());

         ma->LoadFlameFile(fileName); 
      }
      
      //////////////////////////////////////////////////////////
      void MissileActorProxy::CreateActor()
      {
         MissileActor* pActor = new MissileActor(*this);
         SetActor(*pActor); 
         pActor->InitDeadReckoningHelper();
      }


      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      MissileActor::MissileActor(dtGame::GameActorProxy &proxy) : 
         Platform(proxy),
         mLastTranslationSet(false),
         mLastRotationSet(false)
      {
      }

      //////////////////////////////////////////////////////////
      MissileActor::~MissileActor()
      {
         SetFlameEnabled(false);
         mFlame = NULL;
         SetSmokeTrailEnabled(false);
         mSmokeTrail = NULL;
      }

      //////////////////////////////////////////////////////////
      void MissileActor::LoadFlameFile(const std::string& fileName) 
      { 
         if(!mFlame.valid())
         {
            mFlame = new dtCore::ParticleSystem("Flame");
            mFlame->SetParentRelative(true);
            mFlame->LoadFile(fileName);
         }
      }

      //////////////////////////////////////////////////////////
      dtCore::ParticleSystem* MissileActor::GetFlame()
      {
         return mFlame.get();
      }

      //////////////////////////////////////////////////////////
      void MissileActor::SetFlame(dtCore::ParticleSystem* particles )
      {
         mFlame = particles;
      }

      //////////////////////////////////////////////////////////
      void MissileActor::SetFlameEnabled( bool enable )
      {
         if(mFlame.valid())
         {
            mFlame->SetEnabled(true);
         }
      }

      //////////////////////////////////////////////////////////
      bool MissileActor::IsFlameEnabled()
      {
         if(mFlame.valid())
         {
            return mFlame->IsEnabled();
         }
         return false;
      }

      //////////////////////////////////////////////////////////
      void MissileActor::LoadSmokeTrailFile(const std::string& fileName) 
      { 
         if(!mSmokeTrail.valid())
         {
            GetGameActorProxy().GetGameManager()->CreateActor(
               *dtActors::EngineActorRegistry::PARTICLE_SYSTEM_ACTOR_TYPE,
               mSmokeTrail);
            mSmokeTrail->GetProperty("Particle(s) File")->FromString(fileName);
         }

         if(mSmokeTrail->GetProperty("Particle(s) File")->ToString().empty())
         {
            LOG_ERROR("Failed to load the missile smoke trail particle system file: " + fileName);
         }

         //LoadFlameFile("Particles/thrustflame.osg");
      }

      //////////////////////////////////////////////////////////
      dtActors::ParticleSystemActorProxy* MissileActor::GetSmokeTrail()
      {
         return mSmokeTrail.get();
      }

      //////////////////////////////////////////////////////////
      void MissileActor::SetSmokeTrail(dtActors::ParticleSystemActorProxy* particleProxy )
      {
         mSmokeTrail = particleProxy;
      }

      //////////////////////////////////////////////////////////
      void MissileActor::SetSmokeTrailEnabled( bool enable )
      {
         if(mSmokeTrail.valid())
         {
            dtDAL::BooleanActorProperty* prop = NULL; 
            mSmokeTrail->GetProperty("Enable", prop);
            prop->SetValue(enable);
         }
      }

      //////////////////////////////////////////////////////////
      bool MissileActor::IsSmokeTrailEnabled()
      {
         if(mSmokeTrail.valid())
         {
            const dtDAL::BooleanActorProperty* prop = NULL;
            mSmokeTrail->GetProperty("Enable", prop);
            return prop->GetValue();
         }
         return false;
      }

      //////////////////////////////////////////////////////////
      void MissileActor::OnEnteredWorld()
      {
         Platform::OnEnteredWorld();


         // Set the flame and smoke position by "hotspot_01"
         osg::MatrixTransform* relTransform =
            const_cast<osg::MatrixTransform*> (GetNodeCollector()->GetMatrixTransform("hotspot_01"));



         // If thrust flame exists
         if( mFlame.valid() )
         {

            // Attach the particles to the parent missile
            AddChild(mFlame.get());

            if( relTransform != NULL )
            {
               // Offset the particles to a target point
               dtCore::Transform transform;
               transform.SetTranslation(relTransform->getMatrix().getTrans());
               mFlame->SetTransform(transform,dtCore::Transformable::REL_CS);//*/
            }
            RegisterParticleSystem(*mFlame);
         }


         if( mSmokeTrail.valid() )
         {
            // Set the new emitter program
            dtCore::ParticleSystem* smokeps = dynamic_cast<dtCore::ParticleSystem*> (mSmokeTrail->GetActor());

            if( smokeps != NULL )
            {
               // Add the smoke trail to the GameManager so that
               // it can exist after this missile is deleted.
               GetGameActorProxy().GetGameManager()->AddActor( *mSmokeTrail );

               // The GameManager added it to the scene in AddActor, so remove it 
               smokeps->Emancipate();
               AddChild(smokeps);
               if( relTransform != NULL )
               {
                  dtCore::Transform xform;
                  smokeps->GetTransform(xform, dtCore::Transformable::REL_CS);
                  xform.SetTranslation(relTransform->getMatrix().getTrans());
                  smokeps->SetTransform(xform, dtCore::Transformable::REL_CS);
               }
               // Register the particle systems with the particle manager component
               IG::ParticleInfo::AttributeFlags attrs = {true,true};
               RegisterParticleSystem(*smokeps, &attrs );
            }
         }

         IG::ParticleManagerComponent* comp =
            dynamic_cast<IG::ParticleManagerComponent*> 
            (GetGameActorProxy().GetGameManager()
            ->GetComponentByName(IG::ParticleManagerComponent::DEFAULT_NAME) );

         if( comp != NULL )
         {
            comp->RegisterActor(GetGameActorProxy());
         }
      }


      //////////////////////////////////////////////////////////
      void MissileActor::ScheduleSmokeTrailDelete( const dtGame::Message& message )
      {
         if( mFlame != NULL ) { mFlame->SetEnabled(false); }

         if( !mSmokeTrail.valid() ) { return; }

         dtGame::GMComponent* comp = 
            GetGameActorProxy().GetGameManager()->GetComponentByName(IG::TimedDeleterComponent::DEFAULT_NAME);

         if( comp != NULL )
         {
            IG::TimedDeleterComponent* deleterComp = static_cast<IG::TimedDeleterComponent*> (comp);

            dtCore::ParticleSystem* ps = static_cast<dtCore::ParticleSystem*> (mSmokeTrail->GetActor());
            dtCore::ParticleLayer* smokeLayer = ps->GetSingleLayer("Smoke");
            float deleteTime = 0.0f;
            if( smokeLayer != NULL )
            {
               // Set the delete time based on the particle life span.
               deleteTime = smokeLayer->GetParticleSystem()
                  .getDefaultParticleTemplate().getLifeTime();
            }
            
            if( deleteTime < 1.0f )
            {
               deleteTime = 1.0f;
            }

            deleterComp->AddId( mSmokeTrail->GetId(), deleteTime );
         }
      }


      //////////////////////////////////////////////////////////
      void MissileActor::SetLastKnownTranslation(const osg::Vec3& vec)
      {
         if(!mLastTranslationSet)
         {
            dtCore::Transform trans;
            GetTransform(trans);
            trans.SetTranslation(vec);
            SetTransform(trans, dtCore::Transformable::REL_CS);
            mLastTranslationSet = true;
         }
         Platform::SetLastKnownTranslation(vec);
      }


      //////////////////////////////////////////////////////////
      void MissileActor::SetLastKnownRotation(const osg::Vec3& vec)
      {
         if(!mLastRotationSet)
         {
            dtCore::Transform trans;
            GetTransform(trans);
            trans.SetRotation(vec);
            SetTransform(trans, dtCore::Transformable::REL_CS);
            mLastRotationSet = true;
         }
         Platform::SetLastKnownRotation(vec);
      }

   }
}

