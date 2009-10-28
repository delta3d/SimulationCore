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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/actorproxyicon.h>
#include <dtDAL/project.h>

#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>

#include <dtCore/shadergroup.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>
#include <dtCore/globals.h>
#include <dtCore/transform.h>

#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/StateSet>

#include <osgDB/ReadFile>
#include <osgDB/Registry>


#include <iostream>

namespace SimCore
{
   namespace Actors
   {
      const std::string TerrainActor::DEFAULT_NAME = "Terrain";

      ///////////////////////////////////////////////////////////////////////////////
      TerrainActorProxy::TerrainActorProxy()
      {

      }

      ///////////////////////////////////////////////////////////////////////////////
      void TerrainActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();
         TerrainActor* ta = NULL;
         GetActor(ta);

         static const dtUtil::RefString GROUP_("Terrain");

         static const dtUtil::RefString PROPERTY_TERRAIN_PHYSICS_MODE("Terrain Physics Mode");
         static const dtUtil::RefString PROPERTY_TERRAIN_PHYSICS_MODE_DESC("The mode for the physics, DISABLED disables all physics, DEFERRED"
                  " uses the cull visitor to load in the background for paged terrains, and IMMEDIATE loads the physics immediately based on the"
                  " properties on this actor.");

         AddProperty(new dtDAL::EnumActorProperty<TerrainPhysicsMode>(
                 PROPERTY_TERRAIN_PHYSICS_MODE, PROPERTY_TERRAIN_PHYSICS_MODE,
            dtDAL::EnumActorProperty<TerrainPhysicsMode>::SetFuncType(ta, &TerrainActor::SetTerrainPhysicsMode),
            dtDAL::EnumActorProperty<TerrainPhysicsMode>::GetFuncType(ta, &TerrainActor::GetTerrainPhysicsMode),
            PROPERTY_TERRAIN_PHYSICS_MODE_DESC, GROUP_));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TERRAIN,
            "TerrainMesh", "TerrainMesh",
            dtDAL::ResourceActorProperty::SetFuncType(ta, &TerrainActor::LoadFile),
            "Loads in a terrain mesh for this object", GROUP_));

         static const dtUtil::RefString PROPERTY_LOAD_TERRAIN_MESH_WITH_CACHING("LoadTerrainMeshWithCaching");
         AddProperty(new dtDAL::BooleanActorProperty(
            PROPERTY_LOAD_TERRAIN_MESH_WITH_CACHING, PROPERTY_LOAD_TERRAIN_MESH_WITH_CACHING,
            dtDAL::BooleanActorProperty::SetFuncType(ta, &TerrainActor::SetLoadTerrainMeshWithCaching),
            dtDAL::BooleanActorProperty::GetFuncType(ta, &TerrainActor::GetLoadTerrainMeshWithCaching),
            "Enables OSG caching when loading the terrain mesh.", GROUP_));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "PhysicsModel", "PhysicsModel",
            dtDAL::ResourceActorProperty::SetFuncType(ta, &TerrainActor::SetPhysicsModelFile),
            "Loads a SINGLE physics model file to use for collision.", GROUP_));

         static const dtUtil::RefString PROPERTY_PHYSICSDIRECTORY("PhysicsDirectory");
         AddProperty(new dtDAL::StringActorProperty(PROPERTY_PHYSICSDIRECTORY, PROPERTY_PHYSICSDIRECTORY,
            dtDAL::StringActorProperty::SetFuncType(ta, &TerrainActor::SetPhysicsDirectory),
            dtDAL::StringActorProperty::GetFuncType(ta, &TerrainActor::GetPhysicsDirectory),
            "The directory name of MULTIPLE physics model files to use for collision within the Terrains folder in your map project.", GROUP_));


      }

      /////////////////////////////////////////////////////////////////////////////
      dtDAL::ActorProxyIcon* TerrainActorProxy::GetBillBoardIcon()
      {
         if (!mBillBoardIcon.valid())
            mBillBoardIcon = new dtDAL::ActorProxyIcon("billboards/staticmesh.png");

         return mBillBoardIcon.get();
      }

      ///////////////////////////////////////////////////////////////
      TerrainActor::TerrainActor(dtGame::GameActorProxy& proxy)
      : IGActor(proxy)
#ifdef AGEIA_PHYSICS
      , mHelper(new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy))
#else
      , mHelper(new dtPhysics::PhysicsHelper(proxy))
#endif
      , mTerrainPhysicsMode(&SimCore::TerrainPhysicsMode::DEFERRED)
      , mNeedToLoad(false)
      , mLoadTerrainMeshWithCaching(false)
      {
#ifdef AGEIA_PHYSICS
         mHelper->SetBaseInterfaceClass(this);
#else
         dtPhysics::PhysicsObject* pobj = new dtPhysics::PhysicsObject("Terrain");
         pobj->SetPrimitiveType(dtPhysics::PrimitiveType::TERRAIN_MESH);
         pobj->SetMechanicsType(dtPhysics::MechanicsType::STATIC);
         mHelper->AddPhysicsObject(*pobj);
#endif
      }

      /////////////////////////////////////////////////////////////////////////////
      TerrainActor::~TerrainActor()
      {
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::OnEnteredWorld()
      {
         dtGame::GameActor::OnEnteredWorld();

         if (mTerrainPhysicsMode == &SimCore::TerrainPhysicsMode::IMMEDIATE)
         {
            dtCore::Transform xform;
            GetTransform(xform);

            dtPhysics::PhysicsComponent* comp;
            GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, comp);

            if(comp != NULL)
            {
               bool loadSuccess = false;

   #ifdef AGEIA_PHYSICS
               osg::Vec3 pos;
               xform.GetTranslation(pos);
               NxVec3 vec(pos.x(), pos.y(), pos.z());

               mCollisionResourceString = dtCore::FindFileInPathList( mCollisionResourceString.c_str() );
               if(!mCollisionResourceString.empty())
               {
                  mHelper->SetCollisionMeshFromFile(mCollisionResourceString, vec);

                  mHelper->SetAgeiaUserData(mHelper.get());

                  mHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);

                  loadSuccess = true;
               }

               //next, if a physics directory is specified, we will load all files in that directory
               if(!mPhysicsDirectory.empty())
               {
                  try
                  {
                     std::string fullDirPath = dtDAL::Project::GetInstance().GetContext() + "/Terrains/" + mPhysicsDirectory;

                     if(dtUtil::FileUtils::GetInstance().DirExists(fullDirPath))
                     {
                        dtUtil::FileExtensionList extensionList;
                        extensionList.push_back(".physx");

                        const dtUtil::DirectoryContents& filesInDir = dtUtil::FileUtils::GetInstance().DirGetFiles(fullDirPath, extensionList);
                        dtUtil::DirectoryContents::const_iterator iter = filesInDir.begin();
                        dtUtil::DirectoryContents::const_iterator iterEnd = filesInDir.end();

                        for( ;iter != iterEnd; ++iter)
                        {
                           const std::string& curFile = *iter;
                           std::string fileWithPath = fullDirPath + "/" + curFile;

                           //double check this isnt the same one we loaded above
                           if(!mCollisionResourceString.empty() && dtUtil::FileUtils::GetInstance().IsSameFile(mCollisionResourceString, fileWithPath))
                           {
                              //don't load file
                           }
                           else
                           {
                              LoadMeshFromFile(fileWithPath, curFile);
                           }
                        }

                        LOG_INFO("Loaded " + dtUtil::ToString(filesInDir.size()) + " physics model files from directory '" + fullDirPath + "'." );
                        loadSuccess = true;
                     }
                     else
                     {
                        LOG_ERROR("Attempting to load physics mesh from file, cannot load directory '" + fullDirPath + "'.");
                     }

                  }
                  catch (dtUtil::Exception& e)
                  {
                     e.LogException(dtUtil::Log::LOG_ERROR);
                  }
               }

               if(!loadSuccess && mTerrainNode.valid())
               {
                  //if we didn't find a pre-baked static mesh but we did have a renderable terrain node
                  //then just bake a static collision mesh with that and spit out a warning
                  mHelper->SetCollisionStaticMesh(mTerrainNode.get(), vec);
                  LOG_WARNING("No pre-baked collision mesh found, creating collision geometry from terrain mesh.");
                  loadSuccess = true;
               }
   #else
               if(mTerrainNode.valid())
               {
                  //if we didn't find a pre-baked static mesh but we did have a renderable terrain node
                  //then just bake a static collision mesh with that and spit out a warning
                  mHelper->GetMainPhysicsObject()->SetTransform(xform);
                  mHelper->GetMainPhysicsObject()->CreateFromProperties(mTerrainNode.get());
                  loadSuccess = true;
               }
   #endif
               if(!loadSuccess)
               {
                  LOG_ERROR("Could not find valid terrain mesh or pre-baked collision mesh to create collision data for terrain.");
               }
            }
            else
            {
               LOG_ERROR("No PhysX World Component exists in the Game Manager.");
            }

            comp->RegisterHelper(*mHelper);
         }
      }


      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::AddedToScene(dtCore::Scene* scene)
      {
         IGActor::AddedToScene(scene);
         //Actually load the file, even if it's empty string so that if someone were to
         //load a mesh, remove it from the scene, then try to clear the mesh, this actor will still
         //work.
         LoadFile(mLoadedFile);
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::SetPhysicsModelFile(const std::string& filename)
      {
         mCollisionResourceString = filename;
      }

      /////////////////////////////////////////////////////////////////////////////
      const std::string& TerrainActor::GetPhysicsModelFile() const
      {
         return mCollisionResourceString;
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::SetPhysicsDirectory( const std::string& filename )
      {
         mPhysicsDirectory = filename;
      }

      /////////////////////////////////////////////////////////////////////////////
      std::string TerrainActor::GetPhysicsDirectory() const
      {
         return mPhysicsDirectory;
      }

      /////////////////////////////////////////////////////////////////////////////
      TerrainPhysicsMode& TerrainActor::GetTerrainPhysicsMode() const
      {
         return *mTerrainPhysicsMode;
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::SetTerrainPhysicsMode(TerrainPhysicsMode& physicsMode)
      {
         mTerrainPhysicsMode = &physicsMode;
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::LoadMeshFromFile(const std::string& fileToLoad, const std::string& materialType)
      {
         if(dtUtil::FileUtils::GetInstance().FileExists(fileToLoad))
         {

#ifdef AGEIA_PHYSICS
            NxVec3 vec(0, 0, 0);
            mHelper->SetCollisionMeshFromFile(fileToLoad, vec,
               dtAgeiaPhysX::NxAgeiaPhysicsHelper::DEFAULT_SCENE_NAME,
               materialType);

            mHelper->SetAgeiaUserData(mHelper.get(), materialType);

            mHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);

#else
            std::string filename = dtCore::FindFileInPathList(filename);
            if(!filename.empty())
            {
               //todo, implement for dtPhysics
            }
#endif
         }
         else
         {
            LOG_ERROR("Unable to find physics mesh file '" + fileToLoad + "'.");
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::LoadFile(const std::string& fileName)
      {
         // Don't do anything if the filenames are the same unless we still need to load
         if (!mNeedToLoad && mLoadedFile == fileName)
         {
            return;
         }

         //Don't actually load the file unless we are in the scene.
         if (GetSceneParent() != NULL)
         {
            //We should always clear the geometry.  If LoadFile fails, we should have no geometry.
            if (GetMatrixNode()->getNumChildren() != 0)
            {
               GetMatrixNode()->removeChild(0, GetMatrixNode()->getNumChildren());
            }

            // Empty string is not an error, just has no geometry.
            if (!fileName.empty())
            {
               dtCore::RefPtr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;

               if (mLoadTerrainMeshWithCaching)
               {
                  options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
               }
               else
               {
                  options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_NONE);
               }
               options->setOptionString("loadMaterialsToStateSet");

               mTerrainNode = osgDB::readNodeFile(fileName, options.get());

               if(mTerrainNode.valid())
               {
                  osg::StateSet* ss = mTerrainNode->getOrCreateStateSet();
                  ss->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_TERRAIN, "RenderBin");
               }
            }

            if (!GetShaderGroup().empty())
               SetShaderGroup(GetShaderGroup());

            GetMatrixNode()->addChild(mTerrainNode.get());

            mNeedToLoad = false;
         }
         else
         {
            mNeedToLoad = true;
         }
         mLoadedFile = fileName;
      }

      ///////////////////////////////////////////////////////////////////
      void TerrainActor::SetLoadTerrainMeshWithCaching(bool enable)
      {
         mLoadTerrainMeshWithCaching = enable;
      }

      ///////////////////////////////////////////////////////////////////
      bool TerrainActor::GetLoadTerrainMeshWithCaching()
      {
         return mLoadTerrainMeshWithCaching;
      }

   }
}
