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
 * @author Bradley Anderegg
 */
#include <prefix/SimCorePrefix-src.h>
#include <dtActors/particlesystemactorproxy.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/shadermanager.h>
#include <dtCore/particlesystem.h>
#include <dtCore/transform.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <dtGame/basemessages.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/Actors/SurfaceVesselActor.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/DynamicParticleSystem.h>

#include <osg/MatrixTransform>
#include <osg/Geode>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      SurfaceVesselActorProxy::SurfaceVesselActorProxy()
      {
         SetClassName("SimCore::Actors::SurfaceVesselActor");
      }

      //////////////////////////////////////////////////////////
      SurfaceVesselActorProxy::~SurfaceVesselActorProxy()
      {

      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActorProxy::BuildInvokables()
      {
         PlatformActorProxy::BuildInvokables();
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActorProxy::BuildPropertyMap()
      {
         PlatformActorProxy::BuildPropertyMap();

         dtUtil::RefString group = "SurfaceVessel";

         SurfaceVesselActor& actor = static_cast<SurfaceVesselActor&>(GetGameActor());


         AddProperty(new dtDAL::BooleanActorProperty("WaterSpray Enabled", "WaterSpray Enabled",
            dtDAL::MakeFunctor(static_cast<SurfaceVesselActor&>(GetGameActor()), &SurfaceVesselActor::SetWaterSprayEnabled),
            dtDAL::MakeFunctorRet(static_cast<SurfaceVesselActor&>(GetGameActor()), &SurfaceVesselActor::GetWaterSprayEnabled),
            "Turns the WaterSpray particle system on or off", group));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "WaterSprayFrontFile", "Water Spray Front File", dtDAL::MakeFunctor(actor, &SurfaceVesselActor::LoadWaterSprayFrontFile),
            "Loads the particle system for the water spray effect on the front of the ship", group));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "WaterSpraySideFile", "Water Spray Side File", dtDAL::MakeFunctor(actor, &SurfaceVesselActor::LoadWaterSpraySideFile),
            "Loads the particle system for the water spray effect on the side of the ship", group));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "WaterSprayBackFile", "Water Spray Back File", dtDAL::MakeFunctor(actor, &SurfaceVesselActor::LoadWaterSprayBackFile),
            "Loads the particle system for the water spray effect on the back of the ship", group));

         
         AddProperty(new dtDAL::Vec3ActorProperty("WaterSprayFrontOffsetStarboard", "Water Spray Front Offset Starboard", 
            dtDAL::Vec3ActorProperty::SetFuncType(&actor, &SurfaceVesselActor::SetWaterSprayFrontOffsetStarboard),
            dtDAL::Vec3ActorProperty::GetFuncType(&actor, &SurfaceVesselActor::GetWaterSprayFrontOffsetStarboard),
            "The local offset on the starboard side to apply the water spray particle effect on the front of the ship.", group));

         AddProperty(new dtDAL::Vec3ActorProperty("WaterSprayFrontOffsetPort", "Water Spray Front Offset Port", 
            dtDAL::Vec3ActorProperty::SetFuncType(&actor, &SurfaceVesselActor::SetWaterSprayFrontOffsetPort),
            dtDAL::Vec3ActorProperty::GetFuncType(&actor, &SurfaceVesselActor::GetWaterSprayFrontOffsetPort),
            "The local offset on the port side to apply the water spray particle effect on the front of the ship.", group));

         AddProperty(new dtDAL::Vec3ActorProperty("WaterSpraySideOffsetStarboard", "Water Spray Side Offset Starboard", 
            dtDAL::Vec3ActorProperty::SetFuncType(&actor, &SurfaceVesselActor::SetWaterSpraySideOffsetStarboard),
            dtDAL::Vec3ActorProperty::GetFuncType(&actor, &SurfaceVesselActor::GetWaterSpraySideOffsetStarboard),
            "The local offset on the starboard side to apply the water spray particle effect on the side of the ship.", group));

         AddProperty(new dtDAL::Vec3ActorProperty("WaterSpraySideOffsetPort", "Water Spray Side Offset Port", 
            dtDAL::Vec3ActorProperty::SetFuncType(&actor, &SurfaceVesselActor::SetWaterSpraySideOffsetPort),
            dtDAL::Vec3ActorProperty::GetFuncType(&actor, &SurfaceVesselActor::GetWaterSpraySideOffsetPort),
            "The local offset on the port side to apply the water spray particle effect on the side of the ship.", group));

         AddProperty(new dtDAL::Vec3ActorProperty("WaterSprayBackOffset", "Water Spray Back Offset", 
            dtDAL::Vec3ActorProperty::SetFuncType(&actor, &SurfaceVesselActor::SetWaterSprayBackOffset),
            dtDAL::Vec3ActorProperty::GetFuncType(&actor, &SurfaceVesselActor::GetWaterSprayBackOffset),
            "The local offset on the back side to apply the water spray particle effect on the back of the ship.", group));

         AddProperty(new dtDAL::FloatActorProperty("WaterSprayStartSpeed", "Water Spray Start Speed", 
            dtDAL::FloatActorProperty::SetFuncType(&actor, &SurfaceVesselActor::SetWaterSprayStartSpeed),
            dtDAL::FloatActorProperty::GetFuncType(&actor, &SurfaceVesselActor::GetWaterSprayStartSpeed),
            "The speed at which to start the water spray particle effect.", group));

      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActorProxy::CreateActor()
      {
         SurfaceVesselActor* pActor = new SurfaceVesselActor(*this);
         SetActor(*pActor);
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActorProxy::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         // Remote?
         if(IsRemote())
         {
            RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         }
         else // Local
         {
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
         }
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActorProxy::OnRemovedFromWorld()
      {

      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      SurfaceVesselActor::SurfaceVesselActor(dtGame::GameActorProxy& proxy)
         : Platform(proxy)
         , mWaterSprayStartSpeed(0.0f)
         , mLastEffectRatio(0.0f)
         , mEffectVelocityMin(1.0f)
         , mEffectVelocityMax(8.0f)
         , mEffectUpdateTimer(0.0f)
      {
      }

      //////////////////////////////////////////////////////////
      SurfaceVesselActor::~SurfaceVesselActor()
      {
         SetWaterSprayEnabled(false);
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::OnEnteredWorld()
      {
         Platform::OnEnteredWorld();

         if(mWaterSprayFrontStarboard.valid() && mWaterSprayFrontPort.valid())
         {
            BindShaderToParticleSystem(mWaterSprayFrontStarboard->GetParticleSystem(), "WaterSprayParticle");
            BindShaderToParticleSystem(mWaterSprayFrontPort->GetParticleSystem(), "WaterSprayParticle");

            // Attach the particles to the parent
            GetOSGNode()->asGroup()->addChild(mWaterSprayFrontStarboard->GetOSGNode());
            GetOSGNode()->asGroup()->addChild(mWaterSprayFrontPort->GetOSGNode());

            // Offset the particles 
            dtCore::Transform transform;

            transform.SetTranslation(mWaterSprayFrontOffsetStarboard);
            mWaterSprayFrontStarboard->SetTransform(transform, dtCore::Transformable::REL_CS);

            transform.SetTranslation(mWaterSprayFrontOffsetPort);
            mWaterSprayFrontPort->SetTransform(transform, dtCore::Transformable::REL_CS);            
         }
    
         if(mWaterSpraySideStarboard.valid() && mWaterSpraySidePort.valid())
         {
            BindShaderToParticleSystem(mWaterSpraySideStarboard->GetParticleSystem(), "WaterSprayParticle");
            BindShaderToParticleSystem(mWaterSpraySidePort->GetParticleSystem(), "WaterSprayParticle");

            // Attach the particles to the parent
            GetOSGNode()->asGroup()->addChild(mWaterSpraySideStarboard->GetOSGNode());
            GetOSGNode()->asGroup()->addChild(mWaterSpraySidePort->GetOSGNode());

            // Offset the particles 
            dtCore::Transform transform;

            transform.SetTranslation(mWaterSpraySideOffsetStarboard);
            mWaterSpraySideStarboard->SetTransform(transform, dtCore::Transformable::REL_CS);

            transform.SetTranslation(mWaterSpraySideOffsetPort);
            mWaterSpraySidePort->SetTransform(transform, dtCore::Transformable::REL_CS);
         }

         if(mWaterSprayBack.valid())
         {
            BindShaderToParticleSystem(mWaterSprayBack->GetParticleSystem(), "WaterSprayParticle");

            // Attach the particles to the parent
            GetOSGNode()->asGroup()->addChild(mWaterSprayBack->GetOSGNode());

            // Offset the particles 
            dtCore::Transform transform;

            transform.SetTranslation(mWaterSprayBackOffset);
            mWaterSprayBack->SetTransform(transform, dtCore::Transformable::REL_CS);
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
      void SurfaceVesselActor::BindShaderToParticleSystem(dtCore::ParticleSystem& particles, const std::string& shaderName)
      {
         dtCore::ParticleSystem::LayerList& layers = particles.GetAllLayers();
         dtCore::ParticleSystem::LayerList::iterator iter = layers.begin();
         for( ; iter != layers.end(); ++iter )
         {
            osgParticle::ParticleSystem* ref = &iter->GetParticleSystem();
            dtCore::ParticleLayer& pLayer = *iter;
            BindShaderToNode(shaderName, pLayer.GetGeode());
         }
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::BindShaderToNode(const std::string& shaderName, osg::Node& node)
      {
         dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
         dtCore::ShaderGroup* sg = sm.FindShaderGroupPrototype("ParticleShaderGroup");
         if(sg)
         {
            dtCore::ShaderProgram* sp = sg->FindShader(shaderName);

            if(sp != NULL)
            {
               sm.AssignShaderFromPrototype(*sp, node);
            }
            else
            {
               LOG_ERROR("Unable to find a particle system shader with the name '." + shaderName + "' ");
            }
         }
         else
         {
            LOG_ERROR("Unable to find shader group for particle system manager.");
         }
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSprayStartSpeed( float speed )
      {
         mWaterSprayStartSpeed = speed;
      }

      //////////////////////////////////////////////////////////
      float SurfaceVesselActor::GetWaterSprayStartSpeed() const
      {
         return mWaterSprayStartSpeed;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSprayFrontOffsetPort( const osg::Vec3& vec )
      {
         mWaterSprayFrontOffsetPort = vec;
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 SurfaceVesselActor::GetWaterSprayFrontOffsetPort() const
      {
         return mWaterSprayFrontOffsetPort;
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 SurfaceVesselActor::GetWaterSprayFrontOffsetStarboard() const
      {
         return mWaterSprayFrontOffsetStarboard;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSprayFrontOffsetStarboard( const osg::Vec3& vec )
      {
         mWaterSprayFrontOffsetStarboard = vec;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSpraySideOffsetPort( const osg::Vec3& vec )
      {
         mWaterSpraySideOffsetPort = vec;
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 SurfaceVesselActor::GetWaterSpraySideOffsetPort() const
      {
         return mWaterSpraySideOffsetPort;
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 SurfaceVesselActor::GetWaterSpraySideOffsetStarboard() const
      {
         return mWaterSpraySideOffsetStarboard;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSpraySideOffsetStarboard( const osg::Vec3& vec )
      {
         mWaterSpraySideOffsetStarboard = vec;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSprayBackOffset( const osg::Vec3& vec )
      {
         mWaterSprayBackOffset = vec;
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 SurfaceVesselActor::GetWaterSprayBackOffset() const
      {
         return mWaterSprayBackOffset;
      }

      //////////////////////////////////////////////////////////
      dtCore::RefPtr<SurfaceVesselActor::DynamicParticlesProxy> SurfaceVesselActor::CreatDynamicParticleSystemProxy(
         const std::string& filename, const std::string& actorName)
      {
         dtCore::RefPtr<DynamicParticlesProxy> proxy;
         
         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
         if(gm != NULL)
         {
            // Create the actor.
            DynamicParticlesActor* actor = NULL;
            gm->CreateActor(*SimCore::Actors::EntityActorRegistry::DYNAMIC_PARTICLE_SYSTEM_ACTOR_TYPE, proxy);
            proxy->GetActor(actor);
            actor->SetName(actorName);
            actor->SetParticleSystemFile(filename);

            // Set default settings.
            typedef DynamicParticlesActor::InterpolatorArray InterpolatorArray;
            InterpolatorArray interpArray;
            actor->GetAllInterpolators(interpArray);

            InterpolatorArray::iterator curInterp = interpArray.begin();
            InterpolatorArray::iterator endInterpArray = interpArray.end();
            for( ; curInterp != endInterpArray; ++curInterp)
            {         
               ParticleSystemSettings& settings = (*curInterp)->GetStartSettings();
               settings.mRangeRate *= 0.0f;
               settings.mRangeSpeed *= 0.0f;
            }
         }

         return proxy;
      }

      //////////////////////////////////////////////////////////
      SurfaceVesselActor::DynamicParticlesActor* SurfaceVesselActor::GetParticlesActor(DynamicParticlesProxy* proxy)
      {
         DynamicParticlesActor* actor = NULL;
         if(proxy != NULL)
         {
            proxy->GetActor(actor);
         }
         return actor;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::LoadWaterSprayFrontFile(const std::string& fileName)
      {
         if(!mWaterSprayFrontStarboard.valid())
         {
            mWaterSprayFrontStarboardProxy = CreatDynamicParticleSystemProxy(fileName, "WaterSprayFrontStarboard");
            mWaterSprayFrontStarboard = GetParticlesActor(mWaterSprayFrontStarboardProxy.get());
         }

         if(!mWaterSprayFrontPort.valid())
         {
            mWaterSprayFrontPortProxy = CreatDynamicParticleSystemProxy(fileName, "WaterSprayFrontPort");
            mWaterSprayFrontPort = GetParticlesActor(mWaterSprayFrontPortProxy.get());
         }
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::LoadWaterSpraySideFile(const std::string& fileName)
      {
         if(!mWaterSpraySideStarboard.valid())
         {
            mWaterSpraySideStarboardProxy = CreatDynamicParticleSystemProxy(fileName, "WaterSpraySideStarboard");
            mWaterSpraySideStarboard = GetParticlesActor(mWaterSpraySideStarboardProxy.get());
         }

         if(!mWaterSpraySidePort.valid())
         {
            mWaterSpraySidePortProxy = CreatDynamicParticleSystemProxy(fileName, "WaterSpraySidePort");
            mWaterSpraySidePort = GetParticlesActor(mWaterSpraySidePortProxy.get());
         }
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::LoadWaterSprayBackFile(const std::string& fileName)
      {
         if(!mWaterSprayBack.valid())
         {
            mWaterSprayBackProxy = CreatDynamicParticleSystemProxy(fileName, "WaterSprayBack");
            mWaterSprayBack = GetParticlesActor(mWaterSprayBackProxy.get());
         }

      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSprayEnabled( bool enable )
      {
         if(mWaterSprayFrontStarboard.valid() && mWaterSprayFrontPort.valid())
         {
            mWaterSprayFrontStarboard->SetEnabled(enable);
            mWaterSprayFrontPort->SetEnabled(enable);
         }

         if(mWaterSpraySideStarboard.valid() && mWaterSpraySidePort.valid())
         {
            mWaterSpraySideStarboard->SetEnabled(enable);
            mWaterSpraySidePort->SetEnabled(enable);
         }

         if(mWaterSprayBack.valid())
         {
            mWaterSprayBack->SetEnabled(enable);
         }
      }

      //////////////////////////////////////////////////////////
      bool SurfaceVesselActor::GetWaterSprayEnabled()
      {
         if(mWaterSprayFrontStarboard.valid())
         {
            return mWaterSprayFrontStarboard->IsEnabled();
         }
         return false;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetEffectMinVelocity(float minVelocity)
      {
         mEffectVelocityMin = minVelocity;
      }

      //////////////////////////////////////////////////////////
      float SurfaceVesselActor::GetEffectMinVelocity() const
      {
         return mEffectVelocityMin;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetEffectMaxVelocity(float maxVelocity)
      {
         mEffectVelocityMax = maxVelocity;
      }
      
      //////////////////////////////////////////////////////////
      float SurfaceVesselActor::GetEffectMaxVelocity() const
      {
         return mEffectVelocityMax;
      }

      //////////////////////////////////////////////////////////
      float SurfaceVesselActor::GetVelocityRatio() const
      {
         float ratio = 0.0f;

         if(mEffectVelocityMax != 0.0f)
         {
            osg::Vec3 velocity(GetLastKnownVelocity());
            if(velocity.length2() >= mEffectVelocityMin && mEffectVelocityMax > mEffectVelocityMin)
            {
               ratio = (velocity.length() - mEffectVelocityMin) / (mEffectVelocityMax - mEffectVelocityMin);
            }
         }

         dtUtil::Clamp(ratio, 0.0f, 1.0f);
         
         return ratio;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::UpdateEffects(float simTimeDelta)
      {
         float ratio = GetVelocityRatio();
         float interpTime = 0.01f;

         mEffectUpdateTimer += simTimeDelta;

         bool allowInterpolation = mEffectUpdateTimer > simTimeDelta || dtUtil::Abs(mLastEffectRatio - ratio) > 0.1f;

         if(allowInterpolation)
         {
            // Reset control variables that determine the next update should occur.
            mLastEffectRatio = ratio;
            mEffectUpdateTimer = 0.0f;

            // DEBUG:
            //std::cout << "\n\tUpdating particles ("<< GetLastKnownVelocity().length2() <<" / "<< GetEffectMaxVelocity() <<" = "<< ratio <<")\n\n";
         }

         // Update the particle systems.
         if(mWaterSprayFrontStarboard.valid() && mWaterSprayFrontPort.valid())
         {
            mWaterSprayFrontStarboard->Update(simTimeDelta);
            mWaterSprayFrontPort->Update(simTimeDelta);

            if(allowInterpolation)
            {
               mWaterSprayFrontStarboard->InterpolateAllLayers(interpTime, ratio);
               mWaterSprayFrontPort->InterpolateAllLayers(interpTime, ratio);
            }
         }

         if(mWaterSpraySideStarboard.valid() && mWaterSpraySidePort.valid())
         {
            mWaterSpraySideStarboard->Update(simTimeDelta);
            mWaterSpraySidePort->Update(simTimeDelta);

            if(allowInterpolation)
            {
               mWaterSpraySideStarboard->InterpolateAllLayers(interpTime, ratio);
               mWaterSpraySidePort->InterpolateAllLayers(interpTime, ratio);
            }
         }

         if(mWaterSprayBack.valid())
         {
            mWaterSprayBack->Update(simTimeDelta);

            if(allowInterpolation)
            {
               mWaterSprayBack->InterpolateAllLayers(interpTime, ratio);
            }
         }
      }
      
      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::TickLocal(const dtGame::Message& tickMessage)
      {
         BaseClass::TickLocal(tickMessage);

         float simTimeDelta = static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         UpdateEffects(simTimeDelta);
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::TickRemote(const dtGame::Message& tickMessage)
      {
         BaseClass::TickRemote(tickMessage);

         float simTimeDelta = static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         UpdateEffects(simTimeDelta);
      }

   }
}

