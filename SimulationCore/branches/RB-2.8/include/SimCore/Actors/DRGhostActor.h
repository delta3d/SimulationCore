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

            DRGhostActor(DRGhostActorProxy& proxy);

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

            void SetSlavedEntity(SimCore::Actors::BaseEntity* newEntity);
            SimCore::Actors::BaseEntity* GetSlavedEntity() { return mSlavedEntity.get(); }

            /// The velocity scalar is used to magnify the velocity that is rendered to make it more or less visible. Default is 1.0f
            void SetArrowDrawScalar(float newValue) { mArrowDrawScalar = newValue; }
            float GetArrowDrawScalar() { return mArrowDrawScalar; }

            /// The number of velocity trails that we are showing. You MUST set this before adding it to the GM.
            void SetArrowMaxNumTrails(unsigned int newValue);
            unsigned int GetArrowMaxNumTrails() { return mArrowMaxNumTrails; }

            void UpdateOurPosition();

            /// Default invokable. Used for messages about our slave.
            void ProcessMessage(const dtGame::Message& message);

            void CleanUp();

            void ClearLinesAndParticles();

         protected:
            void SetupVelocityArrows();
            void SetupAccelerationArrows();

            void SetupLineData(osg::Geode& arrowGeode, osg::Geometry& arrowGeom, 
               osg::Vec3Array& arrowVerts, const osg::Vec3& arrowColor);

            void SetCurrentLine(osg::Geometry& arrowGeom, osg::Vec3& startPos, osg::Vec3& endPosDelta);

         private:
            dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mSlavedEntity;
            bool mSlaveUpdatedParticleIsActive;
            int mPosUpdatedParticleCountdown;

            // Velocity Arrow
            dtCore::RefPtr<osg::Geode> mVelocityArrowGeode;
            dtCore::RefPtr<osg::Geometry> mVelocityArrowGeom;
            dtCore::RefPtr<osg::Vec3Array> mVelocityArrowVerts;
            osg::Vec3 mVelocityArrowColor;
            // Acceleration Arrow
            dtCore::RefPtr<osg::Geode> mAccelerationArrowGeode;
            dtCore::RefPtr<osg::Geometry> mAccelerationArrowGeom;
            dtCore::RefPtr<osg::Vec3Array> mAccelerationArrowVerts;
            osg::Vec3 mAccelerationArrowColor;
            // Vars for Vel and Acceleration Arrow trails.
            dtCore::RefPtr<dtCore::Transformable> mArrowGlobalParentNode;
            float mArrowDrawScalar; // Scalar to resize both vel and acceleration arrows
            unsigned int mArrowMaxNumTrails; // number of accel and vel trails
            int mArrowCurrentIndex; // current index into our pool of trails
            bool mArrowDrawOnNextFrame; // do we add a new trail arrow next frame (caused by update)?


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
            DRGhostActor& GetActorAsDRGhostActor()
            {
               return *(static_cast<DRGhostActor*>(GetDrawable()));
            }

         protected:
            virtual ~DRGhostActorProxy();
            void CreateDrawable();
            virtual void OnEnteredWorld();
            virtual void OnRemovedFromWorld();
      };
   }
}

#endif
//#endif
