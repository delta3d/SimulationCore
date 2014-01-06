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
* @author Allen Danklefsen
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/BaseTerrainActor.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/actorproxyicon.h>

#include <dtCore/shadergroup.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>

#include <osg/MatrixTransform>
#include <osg/Node>

namespace SimCore
{
   namespace Actors
   {
      ///////////////////////////////////////////////////////////////////////////////
      BaseTerrainActor::BaseTerrainActor(dtGame::GameActorProxy &proxy) : IGActor(proxy), mNeedToLoad(false)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////
      BaseTerrainActor::~BaseTerrainActor()
      {

      }

      ///////////////////////////////////////////////////////////////////////////////
      void BaseTerrainActor::AddedToScene(dtCore::Scene* scene)
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

      ///////////////////////////////////////////////////////////////////////////////
      void BaseTerrainActor::LoadFile(const std::string &fileName)
      {
         //Don't actually load the file unless
         if (GetSceneParent() != nullptr)
         {
            //We should always clear the geometry.  If LoadFile fails, we should have no geometry.
            if (GetMatrixNode()->getNumChildren() != 0)
            {
               GetMatrixNode()->removeChild(0, GetMatrixNode()->getNumChildren());
            }

            if (!fileName.empty())
            {
               osg::ref_ptr<osg::Node> unused;
               if( !IGActor::LoadFile(fileName, mTerrainNode, unused, false, true) )
                  LOG_ERROR("Failed to load the terrain file: " + fileName);
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

      ///////////////////////////////////////////////////////////////////////////////
      BaseTerrainActorProxy::BaseTerrainActorProxy()
      {

      }

      ///////////////////////////////////////////////////////////////////////////////
      void BaseTerrainActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();
         BaseTerrainActor &ta = static_cast<BaseTerrainActor&>(GetGameActor());

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TERRAIN, 
            "TerrainMesh", "TerrainMesh", 
            dtDAL::ResourceActorProperty::SetFuncType(&ta, &BaseTerrainActor::LoadFile),
            "Loads in a terrain mesh for this object", "Terrain"));
      }
   }
}
