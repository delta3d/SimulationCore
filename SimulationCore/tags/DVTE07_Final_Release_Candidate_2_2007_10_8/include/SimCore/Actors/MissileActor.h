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
#ifndef _MISSILE_ACTOR_H_
#define _MISSILE_ACTOR_H_

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
      class SIMCORE_EXPORT MissileActor : public Platform
      {
      public:

         /// Constructor
         MissileActor(dtGame::GameActorProxy &proxy);

         void SetFlameEnabled( bool enable );

         bool IsFlameEnabled();

         void LoadFlameFile(const std::string &fileName);

         void SetFlame( dtCore::ParticleSystem* particles );

         dtCore::ParticleSystem* GetFlame();

         void SetSmokeTrailEnabled( bool enable );

         bool IsSmokeTrailEnabled();

         void LoadSmokeTrailFile(const std::string &fileName);

         void SetSmokeTrail( dtActors::ParticleSystemActorProxy* particleProxy );

         dtActors::ParticleSystemActorProxy* GetSmokeTrail();

         void SetLastKnownTranslation(const osg::Vec3& vec);

         void SetLastKnownRotation(const osg::Vec3& vec);

         void ScheduleSmokeTrailDelete( const dtGame::Message& message );

      protected:

         /// Destructor
         virtual ~MissileActor();

         void OnEnteredWorld();

      private:

         dtCore::RefPtr<dtCore::ParticleSystem> mFlame;
         dtCore::RefPtr<dtActors::ParticleSystemActorProxy> mSmokeTrail;

         bool mLastTranslationSet;
         bool mLastRotationSet;

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
         void CreateActor();

         void LoadSmokeTrailFile(const std::string &fileName);

         void LoadFlameFile(const std::string &fileName);

      protected:

         /// Destructor
         virtual ~MissileActorProxy();

      private:

      };
   }
}

#endif
