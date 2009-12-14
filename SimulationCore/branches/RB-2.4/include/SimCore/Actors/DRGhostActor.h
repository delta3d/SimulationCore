/* -*-c++-*-
* Driver Demo - HoverVehicleActor (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* Curtiss Murphy
*/
#ifndef _DR_GHOST_ACTOR_
#define _DR_GHOST_ACTOR_

#include <SimCore/Export.h>

#include <dtActors/gamemeshactor.h>
#include <dtCore/observerptr.h>
#include <dtCore/particlesystem.h>

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
            dtCore::RefPtr<osg::Group> mVelocityParentNode;
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
