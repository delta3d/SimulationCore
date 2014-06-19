/*
 * Copyright, 2007, Alion Science and Technology Corporation, all rights reserved.
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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/actorproxyicon.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/Actors/DynamicParticleSystem.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      DynamicParticleSystemActor::DynamicParticleSystemActor( DynamicParticleSystemActorProxy& proxy )
         : BaseClass(proxy)
         , mFileName("")
         , mParticles(new dtCore::ParticleSystem)
      {
         // Attach the new particle system to the root Game Actor node.
         GetMatrixNode()->addChild( mParticles->GetOSGNode() );
      }

      //////////////////////////////////////////////////////////////////////////
      DynamicParticleSystemActor::~DynamicParticleSystemActor()
      {
         ClearParticleLayers();
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::SetParticleSystemFile( const std::string& fileName )
      {
         SetParticleSystem( fileName );
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& DynamicParticleSystemActor::GetParticleSystemFile() const
      {
         return mFileName.Get();
      }

      //////////////////////////////////////////////////////////////////////////
      bool DynamicParticleSystemActor::SetParticleSystem( const std::string& fileName )
      {
         mFileName = fileName;
         bool success = NULL != mParticles->LoadFile( fileName );

         if( success )
         {
            SetParticleSystem( mParticles.get() );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::SetParticleSystem( dtCore::ParticleSystem* particleSystem )
      {
         // If this is not the current particle system, the reset the file name.
         if( particleSystem != mParticles.get() )
         {
            mFileName = std::string("");
         }

         // Get the old states of the current particle system so that they can be
         // set on the new particle system.
         bool wasEnabled = IsEnabled();

         // Detach the new particle system to the root Game Actor node.
         GetMatrixNode()->removeChild( mParticles->GetOSGNode() );

         if( particleSystem != NULL )
         {
            if( particleSystem != mParticles.get() )
            {
               mParticles = particleSystem;
            }
         }
         else
         {
            mParticles = new dtCore::ParticleSystem;
         }

         // Attach the new particle system to the root Game Actor node.
         GetMatrixNode()->addChild( mParticles->GetOSGNode() );

         // Obtain a map to all the particle system's layers.
         SetParticleLayers( *mParticles );

         // Set the state of the particle system.
         SetEnabled( wasEnabled );
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::ParticleSystem& DynamicParticleSystemActor::GetParticleSystem()
      {
         return *mParticles;
      }

      //////////////////////////////////////////////////////////////////////////
      const dtCore::ParticleSystem& DynamicParticleSystemActor::GetParticleSystem() const
      {
         return *mParticles;
      }

      //////////////////////////////////////////////////////////////////////////
      ParticleLayerInterpolator* DynamicParticleSystemActor::GetInterpolator( const std::string& layerName )
      {
         ParticleLayerInterpMap::iterator foundIter = mLayerInterps.find( layerName );
         return foundIter == mLayerInterps.end() ? NULL : foundIter->second.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const ParticleLayerInterpolator* DynamicParticleSystemActor::GetInterpolator( const std::string& layerName ) const
      {
         ParticleLayerInterpMap::const_iterator foundIter = mLayerInterps.find( layerName );
         return foundIter == mLayerInterps.end() ? NULL : foundIter->second.get();
      }

      //////////////////////////////////////////////////////////////////////////
      ParticleLayerInterpolator* DynamicParticleSystemActor::GetInterpolator( unsigned index )
      {
         ParticleLayerInterpolator* interpolator = NULL;

         PSLayerList& layers = mParticles->GetAllLayers();
         if( ! layers.empty() && unsigned(layers.size()) > index )
         {
            dtCore::ParticleLayer* layer = NULL;
            PSLayerList::iterator curLayer = layers.begin();
            PSLayerList::iterator endLayerList = layers.end();
            for( unsigned curIndex = 0; curLayer != endLayerList; ++curLayer, ++curIndex )
            {
               if( index == curIndex )
               {
                  layer = &(*curLayer);
                  break;
               }
            }

            interpolator = GetInterpolator( layer->GetLayerName() );
         }

         return interpolator;
      }

      //////////////////////////////////////////////////////////////////////////
      const ParticleLayerInterpolator* DynamicParticleSystemActor::GetInterpolator( unsigned index ) const
      {
         const ParticleLayerInterpolator* interpolator = NULL;

         const PSLayerList& layers = mParticles->GetAllLayers();
         if( ! layers.empty() && unsigned(layers.size()) > index )
         {
            const dtCore::ParticleLayer* layer = NULL;
            PSLayerList::const_iterator curLayer = layers.begin();
            PSLayerList::const_iterator endLayerList = layers.end();
            for( unsigned curIndex = 0; curLayer != endLayerList; ++curLayer, ++curIndex )
            {
               if( index == curIndex )
               {
                  layer = &(*curLayer);
                  break;
               }
            }

            interpolator = GetInterpolator( layer->GetLayerName() );
         }

         return interpolator;
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::GetAllInterpolators(InterpolatorArray& outArray)
      {
         ParticleLayerInterpMap::iterator curInterp = mLayerInterps.begin();
         ParticleLayerInterpMap::iterator endInterpArray = mLayerInterps.end();
         for( ; curInterp != endInterpArray; ++curInterp)
         {
            outArray.push_back(curInterp->second.get());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::GetAllInterpolators(InterpolatorArray_Const& outArray) const
      {
         ParticleLayerInterpMap::const_iterator curInterp = mLayerInterps.begin();
         ParticleLayerInterpMap::const_iterator endInterpArray = mLayerInterps.end();
         for( ; curInterp != endInterpArray; ++curInterp)
         {
            outArray.push_back(curInterp->second.get());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::SetEnabled( bool enabled )
      {
         mParticles->SetEnabled( enabled );
      }

      //////////////////////////////////////////////////////////////////////////
      bool DynamicParticleSystemActor::IsEnabled() const
      {
         return mParticles->IsEnabled();
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::ParticleLayer* DynamicParticleSystemActor::GetParticleLayer( const std::string& layerName )
      {
         ParticleLayerInterpolator* interp = GetInterpolator( layerName );
         return interp == NULL ? NULL : &interp->GetLayer();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtCore::ParticleLayer* DynamicParticleSystemActor::GetParticleLayer( const std::string& layerName ) const
      {
         const ParticleLayerInterpolator* interp = GetInterpolator( layerName );
         return interp == NULL ? NULL : &interp->GetLayer();
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::SetInterpolation( float interpolationRatio )
      {
         // Queue the time for interpolating all properties.
         InterpolateAllLayers( 0.01f, interpolationRatio );

         // Force the properties to the specified interpolation
         // by artificially ticking the interpolation effect.
         Update( 0.01f );
      }

      //////////////////////////////////////////////////////////////////////////
      float DynamicParticleSystemActor::GetInterpolation() const
      {
         // Get the primary interpolator since it may have the best chance
         // of representing the whole particle system's interpolation.
         const ParticleLayerInterpolator* interpolator = GetInterpolator();

         // Access the current interpolation registered with the
         // ALL PROPERTIES particle property.
         return interpolator != NULL
            ? interpolator->GetParticlePropertyInterpolation( PS_ALL_PROPERTIES ) : 0.0f;
      }

      //////////////////////////////////////////////////////////////////////////
      /*void DynamicParticleSystemActor::SetSettingsForInterpolator( ParticleLayerInterpolator& layer,
         ParticleSystemSettings& settings, bool isStartSettings, ParticlePropertySet* ignoreProperties )
      {
         ParticlePropertySet::iterator curProp = ignoreProperties.begin();
         ParticlePropertySet::iterator endPropSet = ignoreProperties.end();
         for( ; curProp != endPropSet; ++curProp )
         {

         }
      }

      //////////////////////////////////////////////////////////////////////////
      int DynamicParticleSystemActor::SetSettingsForAllInterpolators( ParticleSystemSettings& settings,
         bool isStartSettings, ParticlePropertySet* ignoreProperties )
      {
         int layers = 0;

         ParticleLayerInterpMap::iterator curInterp = mLayerInterps.begin();
         ParticleLayerInterpMap::iterator endInterpMap = mLayerInterps.end();
         for( ; curInterp != endInterpMap; ++curInterp )
         {
            SetSettingsForLayer( *(curInterp->second), settings,
               isStartSettings, ignoreProperties );

            ++layers;
         }

         return layers;
      }*/

      //////////////////////////////////////////////////////////////////////////
      int DynamicParticleSystemActor::InterpolateAllLayers( ParticlePropertyEnum prop,
         float time, float targetInterpolationRatio )
      {
         int layers = 0;

         ParticleLayerInterpMap::iterator curInterp = mLayerInterps.begin();
         ParticleLayerInterpMap::iterator endInterpMap = mLayerInterps.end();
         for( ; curInterp != endInterpMap; ++curInterp )
         {
            curInterp->second->InterpolateOverTime( prop, time, targetInterpolationRatio );
            ++layers;
         }

         return layers;
      }

      //////////////////////////////////////////////////////////////////////////
      int DynamicParticleSystemActor::InterpolateAllLayers( float time, float interpolation )
      {
         return InterpolateAllLayers( PS_ALL_PROPERTIES, time, interpolation );
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::Reset()
      {
         ParticleLayerInterpMap::iterator curInterp = mLayerInterps.begin();
         ParticleLayerInterpMap::iterator endInterpMap = mLayerInterps.end();
         for( ; curInterp != endInterpMap; ++curInterp )
         {
            curInterp->second->Reset();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      int DynamicParticleSystemActor::SetParticleLayers( dtCore::ParticleSystem& particleSystem )
      {
         // Clear out the mappings to any current layers.
         ClearParticleLayers();

         int successes = 0;

         PSLayerList& layers = particleSystem.GetAllLayers();

         // Maintain a map to all particle layers by their associated particle system's name.
         PSLayerList::iterator curLayer = layers.begin();
         PSLayerList::iterator endLayerList = layers.end();
         for( ; curLayer != endLayerList; ++curLayer )
         {
            // Map the layers to the name of the associated OSG particle system.
            // This will allow for quick access to a particular layer by name.
            if( mLayerInterps.insert( std::make_pair( curLayer->GetLayerName(),
               new ParticleLayerInterpolator(*curLayer) ) ).second )
            {
               ++successes;
            }
         }

         return successes;
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::ClearParticleLayers()
      {
         mLayerInterps.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::Update( float simTimeDelta )
      {
         ParticleLayerInterpMap::iterator curInterp = mLayerInterps.begin();
         ParticleLayerInterpMap::iterator endInterpMap = mLayerInterps.end();
         for( ; curInterp != endInterpMap; ++curInterp )
         {
            curInterp->second->Update( simTimeDelta );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActor::TickLocal( const dtGame::Message& tickMessage )
      {
         Update( static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime() );
      }



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString DynamicParticleSystemActorProxy::CLASS_NAME("dcsim::DynamicParticleSystemActor");
      const dtUtil::RefString DynamicParticleSystemActorProxy::PROPERTY_ENABLED("Enabled");
      const dtUtil::RefString DynamicParticleSystemActorProxy::PROPERTY_PARTICLE_FILE("Particle File");
      const dtUtil::RefString DynamicParticleSystemActorProxy::PROPERTY_START_INTERPOLATION("Start Interpolation");

      //////////////////////////////////////////////////////////////////////////
      DynamicParticleSystemActorProxy::DynamicParticleSystemActorProxy()
         : BaseClass()
      {
         SetClassName( DynamicParticleSystemActorProxy::CLASS_NAME );
      }

      //////////////////////////////////////////////////////////////////////////
      DynamicParticleSystemActorProxy::~DynamicParticleSystemActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActorProxy::CreateDrawable()
      {
         SetDrawable( *new DynamicParticleSystemActor(*this) );
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         const std::string &GROUPNAME = "Particle System";

         DynamicParticleSystemActor* actor = NULL;
         GetDrawable( actor );

         // BOOLEAN PROPERTIES
         AddProperty(new dtCore::BooleanActorProperty(
            DynamicParticleSystemActorProxy::PROPERTY_ENABLED.Get(),
            DynamicParticleSystemActorProxy::PROPERTY_ENABLED.Get(),
            dtCore::BooleanActorProperty::SetFuncType(actor, &DynamicParticleSystemActor::SetEnabled),
            dtCore::BooleanActorProperty::GetFuncType(actor, &DynamicParticleSystemActor::IsEnabled),
            "Sets whether the particle system should be enabled or disabled when it enters the scene.",
            GROUPNAME));

         // FLOAT PROPERTIES
         AddProperty(new dtCore::FloatActorProperty(
            DynamicParticleSystemActorProxy::PROPERTY_START_INTERPOLATION.Get(),
            DynamicParticleSystemActorProxy::PROPERTY_START_INTERPOLATION.Get(),
            dtCore::FloatActorProperty::SetFuncType(actor, &DynamicParticleSystemActor::SetInterpolation),
            dtCore::FloatActorProperty::GetFuncType(actor, &DynamicParticleSystemActor::GetInterpolation),
            "Sets the initial interpolation for the particle system between it start and end interpolation settings.",
            GROUPNAME));

         // RESOURCE PROPERTIES
         AddProperty(new dtCore::ResourceActorProperty(*this,
            dtCore::DataType::PARTICLE_SYSTEM,
            DynamicParticleSystemActorProxy::PROPERTY_PARTICLE_FILE.Get(),
            DynamicParticleSystemActorProxy::PROPERTY_PARTICLE_FILE.Get(),
            dtCore::ResourceActorProperty::SetFuncType(actor, &DynamicParticleSystemActor::SetParticleSystemFile),
            "Sets the particle system file to be loaded",
            GROUPNAME));
      }

      //////////////////////////////////////////////////////////////////////////
      void DynamicParticleSystemActorProxy::OnEnteredWorld()
      {
         if( ! IsRemote())
         {
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);

         }
      }

      //////////////////////////////////////////////////////////////////////////
      DynamicParticleSystemActor& DynamicParticleSystemActorProxy::GetDynamicParticleSystemActor()
      {
         DynamicParticleSystemActor* actor = NULL;
         GetDrawable( actor );
         return *actor;
      }

      //////////////////////////////////////////////////////////////////////////
      const DynamicParticleSystemActor& DynamicParticleSystemActorProxy::GetDynamicParticleSystemActor() const
      {
         const DynamicParticleSystemActor* actor = NULL;
         GetDrawable( actor );
         return *actor;
      }


      //////////////////////////////////////////////////////////////////////////
      dtCore::ActorProxyIcon* DynamicParticleSystemActorProxy::GetBillBoardIcon()
      {
         if(!mBillBoardIcon.valid())
         {
            dtCore::ActorProxyIcon::ActorProxyIconConfig config;
            config.mForwardVector = true;
            config.mUpVector = true;
            config.mScale = 0.1;

            mBillBoardIcon = new dtCore::ActorProxyIcon(dtCore::ActorProxyIcon::IMAGE_BILLBOARD_STATICMESH, config);
         }

         return mBillBoardIcon.get();
      }

   }
}
