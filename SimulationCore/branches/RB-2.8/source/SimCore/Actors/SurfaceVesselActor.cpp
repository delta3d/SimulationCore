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
#include <prefix/SimCorePrefix.h>
#include <dtActors/particlesystemactorproxy.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/shadermanager.h>
#include <dtCore/particlesystem.h>
#include <dtCore/transform.h>
#include <dtCore/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
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
      const dtUtil::RefString SurfaceVesselActorProxy::CLASS_NAME("SimCore::Actors::SurfaceVesselActor");
      const dtUtil::RefString SurfaceVesselActorProxy::PROPERTY_SPRAY_VELOCITY_MIN("Spray Velocity Min");
      const dtUtil::RefString SurfaceVesselActorProxy::PROPERTY_SPRAY_VELOCITY_MAX("Spray Velocity Max");

      //////////////////////////////////////////////////////////
      SurfaceVesselActorProxy::SurfaceVesselActorProxy()
      {
         SetClassName(SurfaceVesselActorProxy::CLASS_NAME);
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

         SurfaceVesselActor* drawable = NULL;
         GetDrawable(drawable);


         AddProperty(new dtCore::BooleanActorProperty("WaterSpray Enabled", "WaterSpray Enabled",
            dtCore::BooleanActorProperty::SetFuncType(drawable, &SurfaceVesselActor::SetWaterSprayEnabled),
            dtCore::BooleanActorProperty::GetFuncType(drawable, &SurfaceVesselActor::GetWaterSprayEnabled),
            "Turns the WaterSpray particle system on or off", group));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM,
            "WaterSprayFrontFile", "Water Spray Front File", dtCore::ResourceActorProperty::SetFuncType(drawable, &SurfaceVesselActor::LoadWaterSprayFrontFile),
            "Loads the particle system for the water spray effect on the front of the ship", group));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM,
            "WaterSpraySideFile", "Water Spray Side File", dtCore::ResourceActorProperty::SetFuncType(drawable, &SurfaceVesselActor::LoadWaterSpraySideFile),
            "Loads the particle system for the water spray effect on the side of the ship", group));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM,
            "WaterSprayBackFile", "Water Spray Back File", dtCore::ResourceActorProperty::SetFuncType(drawable, &SurfaceVesselActor::LoadWaterSprayBackFile),
            "Loads the particle system for the water spray effect on the back of the ship", group));

         
         AddProperty(new dtCore::Vec3ActorProperty("WaterSprayFrontOffset", "Water Spray Front Offset", 
            dtCore::Vec3ActorProperty::SetFuncType(drawable, &SurfaceVesselActor::SetWaterSprayFrontOffset),
            dtCore::Vec3ActorProperty::GetFuncType(drawable, &SurfaceVesselActor::GetWaterSprayFrontOffset),
            "The local offset on the starboard side to apply the water spray particle effect on the front of the ship.", group));

         AddProperty(new dtCore::Vec3ActorProperty("WaterSpraySideOffsetStarboard", "Water Spray Side Offset Starboard", 
            dtCore::Vec3ActorProperty::SetFuncType(drawable, &SurfaceVesselActor::SetWaterSpraySideOffsetStarboard),
            dtCore::Vec3ActorProperty::GetFuncType(drawable, &SurfaceVesselActor::GetWaterSpraySideOffsetStarboard),
            "The local offset on the starboard side to apply the water spray particle effect on the side of the ship.", group));

         AddProperty(new dtCore::Vec3ActorProperty("WaterSpraySideOffsetPort", "Water Spray Side Offset Port", 
            dtCore::Vec3ActorProperty::SetFuncType(drawable, &SurfaceVesselActor::SetWaterSpraySideOffsetPort),
            dtCore::Vec3ActorProperty::GetFuncType(drawable, &SurfaceVesselActor::GetWaterSpraySideOffsetPort),
            "The local offset on the port side to apply the water spray particle effect on the side of the ship.", group));

         AddProperty(new dtCore::Vec3ActorProperty("WaterSprayBackOffset", "Water Spray Back Offset", 
            dtCore::Vec3ActorProperty::SetFuncType(drawable, &SurfaceVesselActor::SetWaterSprayBackOffset),
            dtCore::Vec3ActorProperty::GetFuncType(drawable, &SurfaceVesselActor::GetWaterSprayBackOffset),
            "The local offset on the back side to apply the water spray particle effect on the back of the ship.", group));

         AddProperty(new dtCore::FloatActorProperty(PROPERTY_SPRAY_VELOCITY_MIN, PROPERTY_SPRAY_VELOCITY_MIN, 
            dtCore::FloatActorProperty::SetFuncType(drawable, &SurfaceVesselActor::SetSprayVelocityMin),
            dtCore::FloatActorProperty::GetFuncType(drawable, &SurfaceVesselActor::GetSprayVelocityMin),
            "The speed at which to start the water spray particle effect.", group));

         AddProperty(new dtCore::FloatActorProperty(PROPERTY_SPRAY_VELOCITY_MAX, PROPERTY_SPRAY_VELOCITY_MAX, 
            dtCore::FloatActorProperty::SetFuncType(drawable, &SurfaceVesselActor::SetSprayVelocityMax),
            dtCore::FloatActorProperty::GetFuncType(drawable, &SurfaceVesselActor::GetSprayVelocityMax),
            "The speed at which to clamp the water spray particle systems' maximum effect.", group));

      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActorProxy::CreateDrawable()
      {
         SurfaceVesselActor* pActor = new SurfaceVesselActor(*this);
         SetDrawable(*pActor);
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
      // Actor code
      //////////////////////////////////////////////////////////
      SurfaceVesselActor::SurfaceVesselActor(dtGame::GameActorProxy& owner)
         : Platform(owner)
         , mLastSprayRatio(0.0f)
         , mSprayVelocityMin(1.0f)
         , mSprayVelocityMax(8.0f)
         , mSprayUpdateTimer(0.0f)
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

         if(mWaterSprayFront.valid())
         {
            BindShaderToParticleSystem(mWaterSprayFront->GetParticleSystem(), "WaterSprayParticle");

            // Attach the particles to the parent
            GetOSGNode()->asGroup()->addChild(mWaterSprayFront->GetOSGNode());

            // Offset the particles 
            dtCore::Transform transform;

            transform.SetTranslation(mWaterSprayFrontOffset);
            mWaterSprayFront->SetTransform(transform, dtCore::Transformable::REL_CS);          
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
            //osgParticle::ParticleSystem* ref = &iter->GetParticleSystem();
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
      osg::Vec3 SurfaceVesselActor::GetWaterSprayFrontOffset() const
      {
         return mWaterSprayFrontOffset;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetWaterSprayFrontOffset( const osg::Vec3& vec )
      {
         mWaterSprayFrontOffset = vec;
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
            proxy->GetDrawable(actor);
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
            proxy->GetDrawable(actor);
         }
         return actor;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::LoadWaterSprayFrontFile(const std::string& fileName)
      {
         if(!mWaterSprayFront.valid())
         {
            mWaterSprayFrontProxy = CreatDynamicParticleSystemProxy(fileName, "WaterSprayFrontStarboard");
            mWaterSprayFront = GetParticlesActor(mWaterSprayFrontProxy.get());
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
         if(mWaterSprayFront.valid())
         {
            mWaterSprayFront->SetEnabled(enable);
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
         if(mWaterSprayFront.valid())
         {
            return mWaterSprayFront->IsEnabled();
         }
         return false;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetSprayVelocityMin(float minVelocity)
      {
         mSprayVelocityMin = minVelocity;
      }

      //////////////////////////////////////////////////////////
      float SurfaceVesselActor::GetSprayVelocityMin() const
      {
         return mSprayVelocityMin;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::SetSprayVelocityMax(float maxVelocity)
      {
         mSprayVelocityMax = maxVelocity;
      }
      
      //////////////////////////////////////////////////////////
      float SurfaceVesselActor::GetSprayVelocityMax() const
      {
         return mSprayVelocityMax;
      }

      //////////////////////////////////////////////////////////
      float SurfaceVesselActor::GetVelocityRatio(float velocity) const
      {
         float ratio = 0.0f;

         if(mSprayVelocityMax != 0.0f)
         {
            if(velocity >= mSprayVelocityMin && mSprayVelocityMax > mSprayVelocityMin)
            {
               ratio = (velocity - mSprayVelocityMin) / (mSprayVelocityMax - mSprayVelocityMin);
            }
         }

         dtUtil::Clamp(ratio, 0.0f, 1.0f);
         
         return ratio;
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::UpdateSpray(float simTimeDelta)
      {
         // Determine the displacement of the object.
         dtCore::Transform xform;
         GetTransform(xform);
         osg::Vec3 pos;
         xform.GetTranslation(pos);

         // --- Get the displacement from last tick.
         pos.z() = 0.0f; // The spray effect is not affected by up/down motion.
         osg::Vec3 dif(mLastPos);
         mLastPos = pos;
         dif = pos - dif;

         // Get the velocity and its amount of spray effect.
         float velocity = dif.length2() > 0.0f ? dif.length() : 0.0f;
         float ratio = GetVelocityRatio(velocity * (simTimeDelta!=0.0f ? 1.0f/simTimeDelta : 0.0f));
         float interpTime = 0.01f;

         mSprayUpdateTimer += simTimeDelta;

         bool allowInterpolation = mSprayUpdateTimer > simTimeDelta || dtUtil::Abs(mLastSprayRatio - ratio) > 0.1f;

         if(allowInterpolation)
         {
            // Reset control variables that determine the next update should occur.
            mLastSprayRatio = ratio;
            mSprayUpdateTimer = 0.0f;

            // DEBUG:
            //std::cout << "\n\tUpdating particles ("<< GetLastKnownVelocity().length2() <<" / "<< GetSprayVelocityMax() <<" = "<< ratio <<")\n\n";
         }

         // Update the particle systems.
         if(mWaterSprayFront.valid())
         {
            mWaterSprayFront->Update(simTimeDelta);

            if(allowInterpolation)
            {
               mWaterSprayFront->InterpolateAllLayers(interpTime, ratio);
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
      void SurfaceVesselActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         BaseClass::OnTickLocal(tickMessage);

         float simTimeDelta = tickMessage.GetDeltaSimTime();
         UpdateSpray(simTimeDelta);
      }

      //////////////////////////////////////////////////////////
      void SurfaceVesselActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         BaseClass::OnTickRemote(tickMessage);

         float simTimeDelta = tickMessage.GetDeltaSimTime();
         UpdateSpray(simTimeDelta);
      }

   }
}

