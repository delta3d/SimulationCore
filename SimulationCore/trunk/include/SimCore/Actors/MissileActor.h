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
#ifndef _MISSILE_ACTOR_H_
#define _MISSILE_ACTOR_H_

#include <SimCore/Actors/Platform.h>

namespace dtActors
{
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
      class SIMCORE_EXPORT MissileActor : public Platform
      {
      public:

         /// Constructor
         MissileActor(dtGame::GameActorProxy& parent);

         void SetFlameEnabled( bool enable );

         bool GetFlameEnabled();

         void LoadFlameFile(const std::string& fileName);

         void SetFlame( dtCore::ParticleSystem* particles );

         dtCore::ParticleSystem* GetFlame();

         void SetSmokeTrailEnabled( bool enable );

         bool GetSmokeTrailEnabled();

         void LoadSmokeTrailFile(const std::string& fileName);

         void SetSmokeTrail( dtActors::ParticleSystemActor* particleActor );

         dtActors::ParticleSystemActor* GetSmokeTrail();

         void ScheduleSmokeTrailDelete();

      protected:

         /// Destructor
         virtual ~MissileActor();

         void OnEnteredWorld();

      private:

         dtCore::RefPtr<dtCore::ParticleSystem> mFlame;
         dtCore::RefPtr<dtActors::ParticleSystemActor> mSmokeTrail;

      };

      class SIMCORE_EXPORT MissileActorProxy : public PlatformActorProxy
      {
      public:

         /// Constructor
         MissileActorProxy();

         /// Adds the properties associated with this actor
         void BuildPropertyMap();

         void BuildInvokables();

         /// Creates the actor
         void CreateDrawable();

         void LoadSmokeTrailFile(const std::string& fileName);

         void LoadFlameFile(const std::string& fileName);

         virtual void OnRemovedFromWorld();

      protected:

         /// Destructor
         virtual ~MissileActorProxy();

      private:

      };
   }
}

#endif
