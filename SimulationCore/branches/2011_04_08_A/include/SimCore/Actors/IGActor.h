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
 * @author Eddie Johnson
 */
#ifndef _IGACTOR_H_
#define _IGACTOR_H_

#include <SimCore/Export.h>

#include <dtGame/gameactor.h>
#include <dtUtil/nodecollector.h>

#include <osg/CopyOp>

namespace dtCore
{
   class ParticleSystem;
}

namespace SimCore
{
   class VisibilityOptions;

   namespace Components
   {
      struct ParticleInfoAttributeFlags;
   }

   namespace Actors
   {
      class IGActor;

      class SIMCORE_EXPORT IGActor : public dtGame::GameActor
      {
      public:
         typedef dtGame::GameActor BaseClass;
         /// This will share images, textures, and vertices between models.
         /// Good use case is for articulated vehicles or characters.
         static const unsigned int COPY_OPS_SHARED_GEOMETRY = (osg::CopyOp::DEEP_COPY_OBJECTS
            | osg::CopyOp::DEEP_COPY_NODES
            | osg::CopyOp::DEEP_COPY_STATESETS
            | osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
            | osg::CopyOp::DEEP_COPY_UNIFORMS );

            /// Constructor
            IGActor(dtGame::GameActorProxy& proxy);

            /**
             * Loads in a model file.  Is now a wrapper that calls LoadFileStatic().
             * @param fileName The name of the file
             * @return A pointer to the node the file was stored in
             * or NULL if error
             */
            //osg::Node* LoadFile(const std::string &fileName, bool useCache = true);
            bool LoadFile(const std::string& fileName, dtCore::RefPtr<osg::Node>& originalFile,
               dtCore::RefPtr<osg::Node>& copiedFile, bool useCache = true, bool loadTerrainMaterialsOn = false);

            /**
             * A static version of LoadFile.  The real LoadFile is now a wrapper that calls this.
             */
            static bool LoadFileStatic(const std::string& fileName, dtCore::RefPtr<osg::Node>& originalFile,
               dtCore::RefPtr<osg::Node>& copiedFile, bool useCache = true, bool loadTerrainMaterialsOn = false);

            /* Registers the particle system to the ParticleManagerComponent
             * contained in the GameManager. This function exists for convenience
             * for all subclasses that need to register their particles with the
             * ParticleManagerComponent. The component is used to control all particle
             * systems and to gather information, such as the global particle count.
             * @param particles The reference to the particle system that must be registered.
             * @param attrFlags The attribute flags that specify what forces can be applied to particles
             */
            void RegisterParticleSystem( dtCore::ParticleSystem& particles,
               const SimCore::Components::ParticleInfoAttributeFlags* attrFlags = NULL );

            /* Removes the particle system from the ParticleManagerComponent
             * contained in the GameManager. This function exists for convenience
             * for all subclasses that need to unregister their particles from the
             * ParticleManagerComponent. The component is used to control all particle
             * systems and to gather information, such as the global particle count.
             * @param particles The reference to the particle system that must be unregistered.
             */
            void UnregisterParticleSystem( dtCore::ParticleSystem& particles );

            /**
             * Adds an optional parameter for the node name, and a bool return if that
             * node was found.
             */
            virtual bool AddChild(dtCore::DeltaDrawable* child, const std::string& nodeName = "");

            /// Overrides RemoveChild to support the new AddChild mechanic.
            virtual void RemoveChild(dtCore::DeltaDrawable* child);

            ///@return true if this actor should be visible based on the visibility options given.
            virtual bool ShouldBeVisible(const SimCore::VisibilityOptions&);

            /**
             * @return true if this drawable should be visible.
             */
            virtual bool IsVisible() const;

            /**
             * Sets this entity to visible, that is, drawing.
             */
            virtual void SetVisible(bool);

            static void SetNodeVisible(bool, osg::Node& nodeToUse);

            /**
            * /brief    Purpose :  to load the dofs for the dofcontainer class
            *           Outs     : filled in mNodeCollector memb vars
            */
            virtual void LoadNodeCollector();

            /// Get the node utility class for hotspots and dofs
            dtUtil::NodeCollector*  GetNodeCollector();
            /// Get the node utility class for hotspots and dofs
            const dtUtil::NodeCollector*  GetNodeCollector() const;

         protected:

            /// Destructor
            virtual ~IGActor();

            void SetNodeCollector(dtUtil::NodeCollector* newNC);

         private:
            dtCore::RefPtr<dtUtil::NodeCollector> mNodeCollector;

            bool mIsVisible;
       };
   }
}
#endif
