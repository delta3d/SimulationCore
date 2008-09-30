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
#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Actors/IGActor.h>

#include <osg/Node>

#include <dtCore/particlesystem.h>

#include <dtGame/gamemanager.h>

#include <SimCore/Components/ParticleManagerComponent.h>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////////////////////////
      IGActor::IGActor(dtGame::GameActorProxy &proxy) :
         GameActor(proxy)
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      IGActor::~IGActor()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      bool IGActor::LoadFile(const std::string& fileName, dtCore::RefPtr<osg::Node>& originalFile,
         dtCore::RefPtr<osg::Node>& copiedFile, bool useCache , bool loadTerrainMaterialsOn )
      {
         return IGActor::LoadFileStatic(fileName, originalFile,
            copiedFile, useCache, loadTerrainMaterialsOn);
      }

      ///////////////////////////////////////////////////////////////////////////
      bool IGActor::LoadFileStatic(const std::string& fileName, dtCore::RefPtr<osg::Node>& originalFile,
         dtCore::RefPtr<osg::Node>& copiedFile, bool useCache , bool loadTerrainMaterialsOn )
      {
         //Log::GetInstance().LogMessage(Log::LOG_DEBUG, __FUNCTION__,
         //   "Loading '%s'", filename.c_str());

         dtCore::RefPtr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;

         if (useCache)
         {
            options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
         }
         else
         {
            options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_NONE);
         }

         if(loadTerrainMaterialsOn)
         {
            options->setOptionString("loadMaterialsToStateSet");
         }

         originalFile = osgDB::readNodeFile(fileName, options.get());
         if ( originalFile.valid() )
         {
            if(useCache)
            {
               copiedFile = static_cast<osg::Node*>(originalFile->clone(osg::CopyOp(COPY_OPS_SHARED_GEOMETRY) ));
            }
            return true;
         }
         else
         {
//            Log::GetInstance().LogMessage(Log::LOG_WARNING, __FUNCTION__,
//               "Can't load '%s'", mFilename.c_str() );
            return false;
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void IGActor::RegisterParticleSystem( dtCore::ParticleSystem& particles, const SimCore::Components::ParticleInfo::AttributeFlags* attributes )
      {
         // Register the particle systems with the particle manager component
         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
         if( gm == NULL ) { return; }

         SimCore::Components::ParticleManagerComponent* comp;
         gm->GetComponentByName(SimCore::Components::ParticleManagerComponent::DEFAULT_NAME, comp);

         if( comp != NULL )
         {
            comp->Register(particles, attributes);
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void IGActor::UnregisterParticleSystem( dtCore::ParticleSystem& particles )
      {
         // Register the particle systems with the particle manager component
         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
         if( gm == NULL ) { return; }

         SimCore::Components::ParticleManagerComponent* comp;
         gm->GetComponentByName(SimCore::Components::ParticleManagerComponent::DEFAULT_NAME, comp);

         if( comp != NULL )
         {
            comp->Unregister(particles);
         }
      }

    }
}
