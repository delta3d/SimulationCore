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

#if AGEIA_PHYSICS
#include <NxAgeiaPrimitivePhysicsHelper.h>
#endif

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT TerrainActor : public IGActor
#if AGEIA_PHYSICS
         , public dtAgeiaPhysX::NxAgeiaPhysicsInterface
#endif
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

            /*virtual*/ void OnEnteredWorld();

            void SetPhysicsModelFile( const std::string& filename );
            const std::string& GetPhysicsModelFile() const;

            void SetPhysicsDirectory( const std::string& filename );
            std::string GetPhysicsDirectory() const;


#if AGEIA_PHYSICS
            virtual void AgeiaPrePhysicsUpdate() { }

            virtual void AgeiaPostPhysicsUpdate() { }

            virtual void AgeiaRaycastReport(const NxRaycastHit& hit,
               const NxActor& actor,
               const NxActor& collidedActor)
            {}

            virtual void AgeiaRaycastReport(const NxRaycastHit& hit,
               NxActor& ourSelf,
               NxActor& whatWeHit)
            {}

            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport,
               NxActor& ourSelf,
               NxActor& whatWeHit)
            {}

#endif
            dtPhysics::PhysicsHelper& GetHelper() { return *mHelper; }
            const dtPhysics::PhysicsHelper& GetHelper() const { return *mHelper; }

         protected:

            /// Destructor
            virtual ~TerrainActor();

         private:

            void LoadMeshFromFile(const std::string& filename, const std::string& materialType);

#if AGEIA_PHYSICS
            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper> mHelper;
#else
            dtCore::RefPtr<dtPhysics::PhysicsHelper> mHelper;
#endif

            dtCore::RefPtr<osg::Node> mTerrainNode;
            std::string mLoadedFile, mCollisionResourceString, mPhysicsDirectory;
            //This doesn't load the file unless it's in a scene, so this flag tells it to load
            bool mNeedToLoad;

            bool mLoadTerrainMeshWithCaching;
      };

      class SIMCORE_EXPORT TerrainActorProxy : public dtGame::GameActorProxy
      {
         public:

            /**
             * Constructor
             */
            TerrainActorProxy();

            /**
             * Adds the properties to the actor.
             */
            virtual void BuildPropertyMap();

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
             * @return ActorProxy::RenderMode::DRAW_ACTOR_AND_BILLBOARD_ICON.
             */
            virtual const ActorProxy::RenderMode& GetRenderMode()
            {
               return ActorProxy::RenderMode::DRAW_ACTOR_AND_BILLBOARD_ICON;
            }

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
