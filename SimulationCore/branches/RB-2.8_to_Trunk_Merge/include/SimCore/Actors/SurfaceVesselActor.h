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
#ifndef DELTA_SURFACE_VESSEL_ACTOR_H
#define DELTA_SURFACE_VESSEL_ACTOR_H

#include <dtCore/propertymacros.h>
#include <SimCore/Actors/Platform.h>

namespace dtActors
{
   class DynamicParticleSystem;
   class DynamicParticleSystemActor;
   class ParticleSystemActor;
}

namespace dtCore
{
   class ParticleSystem;
}

namespace dtGame
{
   class Message;
}

namespace osgParticle
{
   class ParticleSystem;
}

namespace SimCore
{
   namespace Actors
   {
      class DynamicParticleSystem;
      class DynamicParticleSystemActor;


      class SIMCORE_EXPORT SurfaceVesselActor : public Platform
      {
      public:
         typedef Platform BaseClass;

         /// Constructor
         SurfaceVesselActor(dtGame::GameActorProxy& parent);

         void SetWaterSprayEnabled( bool enable );

         bool GetWaterSprayEnabled();

         void LoadWaterSprayFrontFile(const std::string &fileName);
         void LoadWaterSpraySideFile(const std::string &fileName);
         void LoadWaterSprayBackFile(const std::string &fileName);
       
         void SetWaterSprayFrontOffset(const osg::Vec3& vec);
         osg::Vec3 GetWaterSprayFrontOffset() const;

         void SetWaterSpraySideOffsetStarboard(const osg::Vec3& vec);
         osg::Vec3 GetWaterSpraySideOffsetStarboard() const;

         void SetWaterSpraySideOffsetPort(const osg::Vec3& vec);
         osg::Vec3 GetWaterSpraySideOffsetPort() const;

         void SetWaterSprayBackOffset(const osg::Vec3& vec);
         osg::Vec3 GetWaterSprayBackOffset() const;

         void SetSprayVelocityMin(float minVelocity);
         float GetSprayVelocityMin() const;

         void SetSprayVelocityMax(float maxVelocity);
         float GetSprayVelocityMax() const;

         float GetVelocityRatio(float velocity) const;

         void UpdateSpray(float simTimeDelta);

         virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);
         virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);

      protected:

         /// Destructor
         virtual ~SurfaceVesselActor();

         void OnEnteredWorld();
         void BindShaderToParticleSystem(dtCore::ParticleSystem& particles, const std::string& shaderName);
         void BindShaderToNode(const std::string& shaderName, osg::Node& node);

         typedef dtActors::DynamicParticleSystem DynamicParticles;
         typedef dtActors::DynamicParticleSystemActor DynamicParticlesActor;
         dtCore::RefPtr<DynamicParticlesActor> CreateDynamicParticleSystemActor(const std::string& filename,
            const std::string& actorName);
         DynamicParticles* GetParticleSystem(DynamicParticlesActor* actor);

      private:
         float mLastSprayRatio;
         float mSprayVelocityMin;
         float mSprayVelocityMax;
         float mSprayUpdateTimer;

         osg::Vec3 mWaterSprayFrontOffset;

         osg::Vec3 mWaterSpraySideOffsetStarboard;
         osg::Vec3 mWaterSpraySideOffsetPort;

         osg::Vec3 mWaterSprayBackOffset;

         osg::Vec3 mLastPos;

         dtCore::RefPtr<DynamicParticlesActor> mWaterSprayFrontActor;
         dtCore::RefPtr<DynamicParticlesActor> mWaterSpraySideStarboardActor;
         dtCore::RefPtr<DynamicParticlesActor> mWaterSpraySidePortActor;
         dtCore::RefPtr<DynamicParticlesActor> mWaterSprayBackActor;

         dtCore::ObserverPtr<DynamicParticles> mWaterSprayFront;
         dtCore::ObserverPtr<DynamicParticles> mWaterSpraySideStarboard;
         dtCore::ObserverPtr<DynamicParticles> mWaterSpraySidePort;
         dtCore::ObserverPtr<DynamicParticles> mWaterSprayBack;
      };

      class SIMCORE_EXPORT SurfaceVesselActorProxy : public PlatformActorProxy
      {
      public:
         typedef PlatformActorProxy BaseClass;

         static const dtUtil::RefString CLASS_NAME;
         static const dtUtil::RefString PROPERTY_SPRAY_VELOCITY_MIN;
         static const dtUtil::RefString PROPERTY_SPRAY_VELOCITY_MAX;

         /// Constructor
         SurfaceVesselActorProxy();

         /// Adds the properties associated with this actor
         void BuildPropertyMap();

         void BuildInvokables();

         /// Creates the actor
         void CreateDrawable();

         virtual void OnEnteredWorld();

      protected:

         /// Destructor
         virtual ~SurfaceVesselActorProxy();

      private:

      };
   }
}

#endif
