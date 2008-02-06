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

#include <dtCore/shadergroup.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>

#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/StateSet>


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
         TerrainActor &ta = static_cast<TerrainActor&>(GetGameActor());

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TERRAIN, 
            "TerrainMesh", "TerrainMesh", 
            dtDAL::MakeFunctor(ta, &TerrainActor::LoadFile), 
            "Loads in a terrain mesh for this object", "Terrain"));
      }

      dtDAL::ActorProxyIcon* TerrainActorProxy::GetBillBoardIcon()
      {
         if(!mBillBoardIcon.valid())
            mBillBoardIcon = new dtDAL::ActorProxyIcon(dtDAL::ActorProxyIcon::IconType::STATICMESH);

         return mBillBoardIcon.get();
      }

      ///////////////////////////////////////////////////////////////
      TerrainActor::TerrainActor(dtGame::GameActorProxy &proxy) : IGActor(proxy), mNeedToLoad(false)
      {
      }

      TerrainActor::~TerrainActor()
      {

      }
      
      void TerrainActor::AddedToScene(dtCore::Scene* scene)
      {
         IGActor::AddedToScene(scene);
         //Actually load the file, even if it's empty string so that if someone were to 
         //load a mesh, remove it from the scene, then try to clear the mesh, this actor will still
         //work.
         if (mNeedToLoad)
         {
            LoadFile(mLoadedFile);
            mNeedToLoad = false;
         }
      }

      void TerrainActor::LoadFile(const std::string &fileName)
      {
         //Don't actually load the file unless
         if (GetSceneParent() != NULL)
         {
            //We should always clear the geometry.  If LoadFile fails, we should have no geometry.
            if (GetMatrixNode()->getNumChildren() != 0)
            {
               GetMatrixNode()->removeChild(0, GetMatrixNode()->getNumChildren());
            }

            if (!fileName.empty())
            {
               dtCore::RefPtr<osg::Node> unused;
               if( !IGActor::LoadFile(fileName, mTerrainNode, unused, false, true) )
                  LOG_ERROR("Failed to load the terrain file: " + fileName);

               //mTerrainNode = IGActor::LoadFile(fileName, false);
               //if (mTerrainNode == NULL)
               //   LOG_ERROR("Failed to load the terrain file: " + fileName);

               if(mTerrainNode.valid())
               {
                  osg::StateSet* ss = mTerrainNode->getOrCreateStateSet();
                  ss->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_TERRAIN, "RenderBin");
               }
            }

            if (!GetShaderGroup().empty())
               SetShaderGroup(GetShaderGroup());

            GetMatrixNode()->addChild(mTerrainNode.get());
         }
         else
         {
            mNeedToLoad = true;
         }
         mLoadedFile = fileName;
      }
   }
}
