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

#ifndef SIMCORE_DYNAMIC_PARTICLE_SYSTEM_ACTOR
#define SIMCORE_DYNAMIC_PARTICLE_SYSTEM_ACTOR

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtGame/gameactor.h>
#include <SimCore/ParticleUtil.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class ParticleLayer;
   class ParticleSystem;
}

namespace SimCore
{
   class ParticleLayerInterpolator;

   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      class DynamicParticleSystemActorProxy;
      class SIMCORE_EXPORT DynamicParticleSystemActor : public dtGame::GameActor
      {
         public:
            typedef dtGame::GameActor BaseClass;

            DynamicParticleSystemActor( DynamicParticleSystemActorProxy& proxy );

            void SetParticleSystemFile( const std::string& fileName );
            const std::string& GetParticleSystemFile() const;

            bool SetParticleSystem( const std::string& fileName );
            void SetParticleSystem( dtCore::ParticleSystem* particleSystem );
            dtCore::ParticleSystem& GetParticleSystem();
            const dtCore::ParticleSystem& GetParticleSystem() const;

            ParticleLayerInterpolator* GetInterpolator( const std::string& layerName ); 
            const ParticleLayerInterpolator* GetInterpolator( const std::string& layerName ) const;

            ParticleLayerInterpolator* GetInterpolator( unsigned index = 0 ); 
            const ParticleLayerInterpolator* GetInterpolator( unsigned index = 0 ) const;

            typedef std::vector<ParticleLayerInterpolator*> InterpolatorArray;
            typedef std::vector<const ParticleLayerInterpolator*> InterpolatorArray_Const;
            void GetAllInterpolators(InterpolatorArray& outArray);
            void GetAllInterpolators(InterpolatorArray_Const& outArray) const;

            void SetEnabled( bool enabled );
            bool IsEnabled() const;

            dtCore::ParticleLayer* GetParticleLayer( const std::string& layerName );
            const dtCore::ParticleLayer* GetParticleLayer( const std::string& layerName ) const;
            
            /**
             * Sets all properties on all contained particle layers to the specified
             * interpolation, IMMEDIATELY.
             * NOTE: This will completely override all existing timed property interpolators
             * on each layer.
             */
            void SetInterpolation( float interpolationRatio );
            /**
             * NOTE: This method only returns interpolations from the first particle layer
             * in the particle system.
             * This method is NOT accurate when modifying the contained particle layers
             * independantly from each other.
             */
            float GetInterpolation() const;

            /**
             * Interpolate a single property type for all layers.
             * @param prop Property type to be interpolated
             * @param time Seconds over which to interpolate
             * @param interpolation Target interpolation to interpolate to over the specified time
             * @return Number of layers updated
             */
            int InterpolateAllLayers( ParticlePropertyEnum prop, float time, float interpolation );

            /**
             * Interpolate all properties of all layers to the specified interpolation.
             * @param time Seconds over which to interpolate
             * @param interpolation Target interpolation to interpolate to over the specified time
             * @return Number of layers updated
             */
            int InterpolateAllLayers( float time, float interpolation );

            void Reset();

            void Update( float simTimeDelta );

            virtual void TickLocal( const dtGame::Message& tickMessage );

         protected:
            virtual ~DynamicParticleSystemActor();

            int SetParticleLayers( dtCore::ParticleSystem& particleSystem );
            void ClearParticleLayers();

         private:
            dtUtil::RefString mFileName;
            dtCore::RefPtr<dtCore::ParticleSystem> mParticles;
            typedef std::map<dtUtil::RefString, dtCore::RefPtr<ParticleLayerInterpolator> > ParticleLayerInterpMap;
            ParticleLayerInterpMap mLayerInterps;
      };



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DynamicParticleSystemActorProxy : public dtGame::GameActorProxy
      {
         public:
            typedef dtGame::GameActorProxy BaseClass;

            static const dtUtil::RefString CLASS_NAME;
            static const dtUtil::RefString PROPERTY_ENABLED;
            static const dtUtil::RefString PROPERTY_PARTICLE_FILE;
            static const dtUtil::RefString PROPERTY_START_INTERPOLATION;
            
            DynamicParticleSystemActorProxy();

            virtual void CreateActor();

            virtual void BuildPropertyMap();

            virtual void OnEnteredWorld();

            /**
             * Convenience method for accessing the actor already cast to its type.
             */
            DynamicParticleSystemActor& GetDynamicParticleSystemActor();
            const DynamicParticleSystemActor& GetDynamicParticleSystemActor() const;

            // Used in STAGE
            virtual dtDAL::ActorProxyIcon* GetBillBoardIcon();

            // STAGE - Draw Billboard or not?
            virtual const dtDAL::ActorProxy::RenderMode& GetRenderMode()
            {
               return dtDAL::ActorProxy::RenderMode::DRAW_ACTOR_AND_BILLBOARD_ICON;
            }

         protected:
            virtual ~DynamicParticleSystemActorProxy();
      };
   }
}

#endif
