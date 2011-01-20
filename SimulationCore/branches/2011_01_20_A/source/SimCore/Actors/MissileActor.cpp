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
#include <prefix/SimCorePrefix.h>
#include <dtActors/particlesystemactorproxy.h>
#include <dtActors/engineactorregistry.h>
#include <dtUtil/nodecollector.h>
#include <dtCore/particlesystem.h>
#include <dtCore/transform.h>
#include <dtDAL/booleanactorproperty.h>
#include <dtDAL/resourceactorproperty.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <SimCore/Actors/MissileActor.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>

#include <dtDAL/propertymacros.h>

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
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::BuildPropertyMap()
      {
         PlatformActorProxy::BuildPropertyMap();

         static const dtUtil::RefString GROUPNAME("MissileActor");
         static const dtUtil::RefString GROUPNAME_PARTICLES("Particles");

         MissileActor* drawable = NULL;
         GetActor(drawable);

         typedef dtDAL::PropertyRegHelper<MissileActorProxy&, MissileActor> PropRegHelperType;
         PropRegHelperType propRegHelper(*this, drawable, GROUPNAME);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(SmokeTrailEnabled, "Smoke Trail Enabled", "Smoke Trail Enabled",
                  "Turns the smoke trail particle system on or off", PropRegHelperType, propRegHelper);

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Smoke Trail File", "Smoke Trail File", dtUtil::MakeFunctor(&MissileActorProxy::LoadSmokeTrailFile, this),
            "Loads the file of the missile smoke trail", "Particles"));

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(FlameEnabled, "Flame Enabled", "Flame Enabled",
                  "Turns the flame particle system on or off", PropRegHelperType, propRegHelper);

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Flame File", "Flame File", dtUtil::MakeFunctor(&MissileActorProxy::LoadFlameFile, this),
            "Loads the file of the missile thruster flame", "Particles"));
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::LoadSmokeTrailFile(const std::string& fileName)
      {
         MissileActor* ma = static_cast<MissileActor*>(GetActor());

         ma->LoadSmokeTrailFile(fileName);
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::LoadFlameFile(const std::string& fileName)
      {
         MissileActor* ma = static_cast<MissileActor*>(GetActor());

         ma->LoadFlameFile(fileName);
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::CreateActor()
      {
         MissileActor* pActor = new MissileActor(*this);
         SetActor(*pActor);
      }

      //////////////////////////////////////////////////////////
      void MissileActorProxy::OnRemovedFromWorld()
      {
         MissileActor* missile = NULL;
         GetActor(missile);
         missile->ScheduleSmokeTrailDelete();
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      MissileActor::MissileActor(dtGame::GameActorProxy& proxy) :
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
      bool MissileActor::GetFlameEnabled()
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
         dtCore::ParticleSystem* ps = NULL;
         if (!mSmokeTrail.valid())
         {
            GetGameActorProxy().GetGameManager()->CreateActor(
               *dtActors::EngineActorRegistry::PARTICLE_SYSTEM_ACTOR_TYPE,
               mSmokeTrail);

            mSmokeTrail->GetActor(ps);
            ps->LoadFile(fileName);
         }

         if (ps != NULL && ps->GetFilename().empty())
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
      bool MissileActor::GetSmokeTrailEnabled()
      {
         if (mSmokeTrail.valid())
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
               SimCore::Components::ParticleInfoAttributeFlags attrs = {true,true};
               RegisterParticleSystem(*smokeps, &attrs );
            }
         }

         SimCore::Components::ParticleManagerComponent* comp;
         GetGameActorProxy().GetGameManager()->
            GetComponentByName(SimCore::Components::ParticleManagerComponent::DEFAULT_NAME, comp);

         if( comp != NULL )
         {
            comp->RegisterActor(GetGameActorProxy());
         }
      }


      //////////////////////////////////////////////////////////
      void MissileActor::ScheduleSmokeTrailDelete()
      {
         if( mFlame != NULL ) { mFlame->SetEnabled(false); }

         if( !mSmokeTrail.valid() ) { return; }

         Components::TimedDeleterComponent* deleterComp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(Components::TimedDeleterComponent::DEFAULT_NAME, deleterComp);

         if( deleterComp != NULL )
         {
            dtCore::ParticleSystem* ps = NULL;
            mSmokeTrail->GetActor(ps);
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
         else
         {
            LOG_ERROR("Could not find the timed deleter component.  Smoke trails will not be deleted properly.");
         }
      }

   }
}

