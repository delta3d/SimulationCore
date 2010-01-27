/* -*-c++-*-
* Simulation Core
* Copyright 2009-2010, Alion Science and Technology
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
*
* Curtiss Murphy
*/
#ifndef _DR_GHOST_ACTOR_
#define _DR_GHOST_ACTOR_

#include <SimCore/Export.h>

#include <dtActors/gamemeshactor.h>
#include <dtCore/observerptr.h>
#include <dtCore/particlesystem.h>
#include <dtCore/transformable.h>

#include <osg/Geometry>
#include <osg/Geode>

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   namespace Actors
   {
      class BaseEntity;
      class DRGhostActorProxy;

      ////////////////////////////////////////////////////////////////////////////////
      /** This class provides a weird ghost actor that shows how Dead Reckoning
       * works. It follows a real vehicle and shows where the remote version is.
       */
      class SIMCORE_EXPORT DRGhostActor : public dtActors::GameMeshActor
      {
         public:
            typedef dtActors::GameMeshActor BaseClass;

            DRGhostActor(DRGhostActorProxy &proxy);

         protected:
            virtual ~DRGhostActor();

         public:

            /**
             * Called when the actor has been added to the game manager.
             * You can respond to OnEnteredWorld on either the proxy or actor or both.
             */
            virtual void OnEnteredWorld();

            //virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

            void SetSlavedEntity(SimCore::Actors::BaseEntity *newEntity);
            SimCore::Actors::BaseEntity *GetSlavedEntity() { return mSlavedEntity.get(); }

            /// The velocity scalar is used to magnify the velocity that is rendered to make it more or less visible. Default is 1.0f
            void SetVelocityArrowDrawScalar(float newValue) { mVelocityArrowDrawScalar = newValue; }
            float GetVelocityArrowDrawScalar() { return mVelocityArrowDrawScalar; }

            /// The number of velocity trails that we are showing. You MUST set this before adding it to the GM.
            void SetVelocityArrowMaxNumVelTrails(unsigned int newValue);
            unsigned int GetVelocityArrowMaxNumVelTrails() { return mVelocityArrowMaxNumVelTrails; }

            void UpdateOurPosition();

            /// Default invokable. Used for messages about our slave.
            void ProcessMessage(const dtGame::Message& message);

            void CleanUp();

         protected:
            void SetupVelocityLine();

         private:
            dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mSlavedEntity;
            bool mSlaveUpdatedParticleIsActive;
            int mPosUpdatedParticleCountdown;

            // Velocity Arrow
            float mVelocityArrowDrawScalar;
            dtCore::RefPtr<osg::Geode> mVelocityArrowGeode;
            dtCore::RefPtr<osg::Geometry> mVelocityArrowGeom;
            dtCore::RefPtr<osg::Vec3Array> mVelocityArrowVerts;
            osg::Vec3 mVelocityArrowColor;
            dtCore::RefPtr<dtCore::Transformable> mVelocityParentNode;
            unsigned int mVelocityArrowMaxNumVelTrails;
            int mVelocityArrowCurrentVelIndex;
            bool mVelocityArrowDrawOnNextFrame;


            dtCore::RefPtr<dtCore::ParticleSystem> mTrailParticles;
            dtCore::RefPtr<dtCore::ParticleSystem> mUpdateTrailParticles;

      };

      ////////////////////////////////////////////////////////////////////////////////
      /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
      class SIMCORE_EXPORT DRGhostActorProxy : public dtActors::GameMeshActorProxy
      {
         public:
            typedef dtActors::GameMeshActorProxy BaseClass;

            DRGhostActorProxy();
            virtual void BuildPropertyMap();

            /// Returns a useful reference to our actor. If no actor is created yet, this will likely crash.
            DRGhostActor &GetActorAsDRGhostActor()
            {
               return *(static_cast<DRGhostActor*>(GetActor()));
            }

         protected:
            virtual ~DRGhostActorProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
            virtual void OnRemovedFromWorld();
      };
   }
}

#endif
//#endif
