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
#include <dtCore/object.h>

#include <osg/Geometry>
#include <osg/Geode>

namespace dtGame
{
   class ActorUpdateMessage;
   class TickMessage;
}

namespace SimCore
{
   namespace Actors
   {
      class BaseEntity;
      class DRGhostDrawable;

      class SIMCORE_EXPORT DRGhostActor : public dtActors::GameMeshActor
      {
         public:
            typedef dtActors::GameMeshActor BaseClass;

            DRGhostActor();
            void BuildPropertyMap() override;

         protected:
            virtual ~DRGhostActor();
            void CreateDrawable();
            virtual void OnEnteredWorld();
            virtual void OnRemovedFromWorld();
      };

      ////////////////////////////////////////////////////////////////////////////////
      /** This class provides a weird ghost actor that shows how Dead Reckoning
       * works. It follows a real vehicle and shows where the remote version is.
       *
       * @TODO most of the code in this class needs to be moved to the actor.
       * It's majorly over-engineered also, so it really should be cut down.
       */
      class SIMCORE_EXPORT DRGhostDrawable : public dtCore::Object
      {
         public:
            typedef dtCore::Object BaseClass;

            DRGhostDrawable(DRGhostActor& actor);

         protected:
            virtual ~DRGhostDrawable();

            DRGhostActor* GetOwner() { return mOwner.get(); }

         public:

            // Not virtual, just on this custom drawable
            void OnEnteredWorld();

            // Not virtual, just on this custom drawable
            //virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);
            // Not virtual, just on this custom drawable
            void OnTickLocal(const dtGame::TickMessage& tickMessage);

            void SetSlavedEntity(SimCore::Actors::BaseEntity* newEntity);
            SimCore::Actors::BaseEntity* GetSlavedEntity() { return mSlavedEntity.get(); }

            /// The velocity scalar is used to magnify the velocity that is rendered to make it more or less visible. Default is 1.0f
            void SetArrowDrawScalar(float newValue) { mArrowDrawScalar = newValue; }
            float GetArrowDrawScalar() { return mArrowDrawScalar; }

            /// The number of velocity trails that we are showing. You MUST set this before adding it to the GM.
            void SetArrowMaxNumTrails(unsigned int newValue);
            unsigned int GetArrowMaxNumTrails() { return mArrowMaxNumTrails; }

            void UpdateOurPosition();

            void OnSlavedUpdate(const dtGame::ActorUpdateMessage& message);

            void CleanUp();

            void ClearLinesAndParticles();

         protected:
            void SetupVelocityArrows();
            void SetupAccelerationArrows();

            void SetupLineData(osg::Geode& arrowGeode, osg::Geometry& arrowGeom, 
               osg::Vec3Array& arrowVerts, const osg::Vec3& arrowColor);

            void SetCurrentLine(osg::Geometry& arrowGeom, osg::Vec3& startPos, osg::Vec3& endPosDelta);

         private:
            dtCore::ObserverPtr<DRGhostActor> mOwner;
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
   }
}

#endif
//#endif
