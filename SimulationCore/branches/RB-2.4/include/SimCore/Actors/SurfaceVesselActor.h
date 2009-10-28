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

#include <dtDAL/propertymacros.h>
#include <SimCore/Actors/Platform.h>

namespace dtActors
{
   class ParticleSystemActorProxy;
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
      class SIMCORE_EXPORT SurfaceVesselActor : public Platform
      {
      public:

         /// Constructor
         SurfaceVesselActor(dtGame::GameActorProxy &proxy);

         void SetWaterSprayEnabled( bool enable );

         bool GetWaterSprayEnabled();

         void LoadWaterSprayFrontFile(const std::string &fileName);
         void LoadWaterSpraySideFile(const std::string &fileName);
         void LoadWaterSprayBackFile(const std::string &fileName);
       
         void SetWaterSprayFrontOffsetStarboard(const osg::Vec3& vec);
         osg::Vec3 GetWaterSprayFrontOffsetStarboard() const;

         void SetWaterSprayFrontOffsetPort(const osg::Vec3& vec);
         osg::Vec3 GetWaterSprayFrontOffsetPort() const;

         void SetWaterSpraySideOffsetStarboard(const osg::Vec3& vec);
         osg::Vec3 GetWaterSpraySideOffsetStarboard() const;

         void SetWaterSpraySideOffsetPort(const osg::Vec3& vec);
         osg::Vec3 GetWaterSpraySideOffsetPort() const;

         void SetWaterSprayBackOffset(const osg::Vec3& vec);
         osg::Vec3 GetWaterSprayBackOffset() const;

         void SetWaterSprayStartSpeed(float speed);
         float GetWaterSprayStartSpeed() const;

      protected:

         /// Destructor
         virtual ~SurfaceVesselActor();

         void OnEnteredWorld();
         void BindShaderToParticleSystem(dtCore::ParticleSystem& particles, const std::string& shaderName);
         void BindShaderToNode(const std::string& shaderName, osg::Node& node);

      private:

         float mWaterSprayStartSpeed;

         osg::Vec3 mWaterSprayFrontOffsetStarboard;
         osg::Vec3 mWaterSprayFrontOffsetPort;

         osg::Vec3 mWaterSpraySideOffsetStarboard;
         osg::Vec3 mWaterSpraySideOffsetPort;

         osg::Vec3 mWaterSprayBackOffset;


         dtCore::RefPtr<dtCore::ParticleSystem> mWaterSprayFrontStarboard;
         dtCore::RefPtr<dtCore::ParticleSystem> mWaterSprayFrontPort;

         dtCore::RefPtr<dtCore::ParticleSystem> mWaterSpraySideStarboard;
         dtCore::RefPtr<dtCore::ParticleSystem> mWaterSpraySidePort;

         dtCore::RefPtr<dtCore::ParticleSystem> mWaterSprayBack;

      };

      class SIMCORE_EXPORT SurfaceVesselActorProxy : public PlatformActorProxy
      {
      public:

         /// Constructor
         SurfaceVesselActorProxy();

         /// Adds the properties associated with this actor
         void BuildPropertyMap();

         void BuildInvokables();

         /// Creates the actor
         void CreateActor();

         virtual void OnRemovedFromWorld();

      protected:

         /// Destructor
         virtual ~SurfaceVesselActorProxy();

      private:

      };
   }
}

#endif
