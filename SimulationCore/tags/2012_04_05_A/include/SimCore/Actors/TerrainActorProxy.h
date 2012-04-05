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
 * @author Matthew W. Campbell
 */
#ifndef DELTA_TERRAINACTORPROXY
#define DELTA_TERRAINACTORPROXY

#include <SimCore/Export.h>
#include <dtGame/gameactor.h>
#include <SimCore/Actors/IGActor.h>

#include <SimCore/PhysicsTypes.h>

#include <SimCore/TerrainPhysicsMode.h>

#include <dtUtil/threadpool.h>

// Sadly, this includes a lot, but it's needed for the options on the LoadNodeTask
#include <osgDB/ReaderWriter>

#if AGEIA_PHYSICS
#include <NxAgeiaPrimitivePhysicsHelper.h>
#endif

namespace dtGame
{
   class TimerElapsedMessage;
}

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT LoadNodeTask : public dtUtil::ThreadPoolTask
      {
      public:
         LoadNodeTask();

         virtual void operator()();

         osg::Node* GetLoadedNode();
         const osg::Node* GetLoadedNode() const;

         /// Check to see if the loading is complete.  If it returns true, call WaitUntilComplete() to make sure.
         bool IsComplete() const;

         virtual void ResetData();

         DT_DECLARE_ACCESSOR(bool, UseFileCaching);
         DT_DECLARE_ACCESSOR(std::string, FileToLoad);
         DT_DECLARE_ACCESSOR(dtCore::RefPtr<osgDB::ReaderWriter::Options>, LoadOptions);

      protected:
         virtual ~LoadNodeTask();
      private:
         dtCore::RefPtr<osg::Node> mLoadedNode;
         volatile bool mComplete;
      };

      class SIMCORE_EXPORT TerrainActor : public IGActor
      {
         public:

            static const std::string DEFAULT_NAME;
            /// Constructor
            TerrainActor(dtGame::GameActorProxy& proxy);

            /**
             * Loads a mesh file which contains terrain.
             * @param fileName The file of the terrain mesh to load.
             * @note Although terrain meshes are the same "type" of file as static meshes
             *  and other geometry, mesh terrains have a special resource of type
             *  DataType::TERRAIN.
             */
            void LoadFile(const std::string& fileName);

            /// Turn on or off texture, and geometry caching when loading the terrain.
            void SetLoadTerrainMeshWithCaching(bool enable);
            /// @return true if texture, and geometry caching when loading the terrain is on.
            bool GetLoadTerrainMeshWithCaching();

            virtual void AddedToScene(dtCore::Scene* scene);
            virtual void RemovedFromScene(dtCore::Scene* scene);

            /*virtual*/ void OnEnteredWorld();

            void SetPhysicsModelFile( const std::string& filename );
            const std::string& GetPhysicsModelFile() const;

            void SetPhysicsDirectory( const std::string& filename );
            std::string GetPhysicsDirectory() const;

            /// @return the physics mode as in if the physics should be loaded and when.
            TerrainPhysicsMode& GetTerrainPhysicsMode() const;

            /// @return the physics mode as in if the physics should be loaded and when.
            void SetTerrainPhysicsMode(TerrainPhysicsMode& physicsMode);

            dtPhysics::PhysicsActComp& GetHelper() { return *mHelper; }
            const dtPhysics::PhysicsActComp& GetHelper() const { return *mHelper; }

            /**
             * Override this to do something after the terrain loads.
             * To override, Call the base class and check the base return
             * value to see if you should do any post load behavior.
             * The caller uses the return value, so be sure to propagate it.
             * Keep in mind that the loading process could be complete, but
             * the terrain node will be null because the loading failed.
             * @return true if the terrain loading completed.
             */
            virtual bool CheckForTerrainLoaded();

            /// This is called if CheckForTerrainLoaded returns true.
            virtual void SetupTerrainPhysics();

         protected:

            /// Destructor
            virtual ~TerrainActor();

         private:

            void LoadMeshFromFile(const std::string& filename, const std::string& materialType);

            dtCore::RefPtr<dtPhysics::PhysicsActComp> mHelper;

            TerrainPhysicsMode* mTerrainPhysicsMode;
            dtCore::RefPtr<osg::Node> mTerrainNode;

            dtCore::RefPtr<LoadNodeTask> mLoadNodeTask;

            std::string mLoadedFile, mCollisionResourceString, mPhysicsDirectory;
            //This doesn't load the file unless it's in a scene, so this flag tells it to load
            bool mNeedToLoad;

            bool mLoadTerrainMeshWithCaching;
      };

      class SIMCORE_EXPORT TerrainActorProxy : public dtGame::GameActorProxy
      {
         public:
            typedef dtGame::GameActorProxy BaseClass;

            /**
             * Constructor
             */
            TerrainActorProxy();

            virtual void OnRemovedFromWorld();

            /**
             * Adds the properties to the actor.
             */
            virtual void BuildPropertyMap();

            /// overridden to initialize the physics.
            virtual void BuildActorComponents();

            /// overridden to setup the timer invokable.
            virtual void BuildInvokables();

            /// Creates the actor we are encapsulating
            virtual void CreateActor() { SetActor(*new TerrainActor(*this)); }

            /**
             * Gets the billboard used to represent static meshes if this proxy's
             * render mode is RenderMode::DRAW_BILLBOARD_ICON.
             * @return a pointer to the icon
             */
            virtual dtDAL::ActorProxyIcon* GetBillBoardIcon();

            /**
             * Gets the current render mode for positional lights.
             * @return dtDAL::BaseActorObject::RenderMode::DRAW_ACTOR_AND_BILLBOARD_ICON.
             */
            virtual const dtDAL::BaseActorObject::RenderMode& GetRenderMode()
            {
               return dtDAL::BaseActorObject::RenderMode::DRAW_ACTOR_AND_BILLBOARD_ICON;
            }

            void HandleNodeLoaded(const dtGame::TimerElapsedMessage& timerElapsed);

         protected:

            /**
             * Destructor
             */
            virtual ~TerrainActorProxy() { }

         private:

      };
   }
}

#endif
