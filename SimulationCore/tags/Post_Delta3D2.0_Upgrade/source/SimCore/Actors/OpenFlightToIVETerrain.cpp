/*
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology
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
* @author Allen Danklefsen
*/
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/OpenFlightToIVETerrain.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/actorproxyicon.h>

#include <dtCore/shadergroup.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>

#include <osg/MatrixTransform>
#include <osg/Node>

#include <osg/PagedLOD>
#include <osgDB/DatabasePager>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
 
namespace SimCore
{
   namespace Actors
   {
      ///////////////////////////////////////////////////////////////////////////////
      OpenFlightToIVETerrainActor::OpenFlightToIVETerrainActor(dtGame::GameActorProxy &proxy) : BaseTerrainActor(proxy)
         , mPaging_Min_X(0.0f)      
         , mPaging_Min_Y(0.0f)      
         , mPaging_Max_X(85.0f)     
         , mPaging_Max_Y(33.0f)     
         , mPaging_Delta(1000.0f)   
         , mPaging_Radius(4000.0f) // Changing these two values does NOT show more terrain
         , mPaging_Range(4000.0f) // Changing these two values does NOT show more terrain
         , mPaging_BaseName("flight%i_%i.ive")
         , mPaging_ExpiringDelay(30.0f)
         , mPaging_Frame_Rate_Targeted(30.0f)
         , mPaging_Precompile(false)
         , mMaximumObjectsToCompile(5) // used to be 1
         , mZOffsetForTerrain(100.0f)
         , mTerrainPath("Terrains/BaghdadFallujah/")
      {
      }

      ///////////////////////////////////////////////////////////////////////////////
      OpenFlightToIVETerrainActor::~OpenFlightToIVETerrainActor()
      {

      }

      ///////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActor::AddedToScene(dtCore::Scene* scene)
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
      void OpenFlightToIVETerrainActor::LoadFile(const std::string &fileName)
      {
         //Don't actually load the file unless
         if (GetSceneParent() != NULL)
         {
            //We should always clear the geometry.  If LoadFile fails, we should have no geometry.
            if (GetMatrixNode()->getNumChildren() != 0)
            {
               GetMatrixNode()->removeChild(0, GetMatrixNode()->getNumChildren());
            }

            if(!mTerrainPath.empty())
            {
               /////////////////////////////////////////////////////////////////////
               // do loading here
               const std::string& currentTerrainPath = mTerrainPath;
               
               osgDB::DatabasePager* pPager =
                  osgDB::Registry::instance()->getOrCreateDatabasePager();
               pPager->setDoPreCompile(mPaging_Precompile);

               osgDB::ReaderWriter::Options* options = new osgDB::ReaderWriter::Options;
               options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
               osgDB::Registry::instance()->setOptions(options);

               //pPager->setTargetFrameRate(mPaging_Frame_Rate_Targeted);
               //pPager->setExpiryDelay(mPaging_ExpiringDelay);
               pPager->setMaximumNumOfObjectsToCompilePerFrame(mMaximumObjectsToCompile);
               
               // TODO-UPGRADE
               //pPager->setThreadPriorityDuringFrame(OpenThreads::Thread::THREAD_PRIORITY_NOMINAL);//HIGH);
               //pPager->setThreadPriorityOutwithFrame(OpenThreads::Thread::THREAD_PRIORITY_NOMINAL);//HIGH);

               int tile_x =0 ,tile_y =0;

               int pagingMaxXInt = int(mPaging_Max_X);
               int pagingMaxYInt = int(mPaging_Max_Y);
               mGroupNodeForTerrain = new osg::Group;
               
               for(tile_x = int(mPaging_Min_X);tile_x <= pagingMaxXInt;tile_x++)
               {
                  for(tile_y = int(mPaging_Min_Y);tile_y <= pagingMaxYInt;tile_y++)
                  {
                     char name[512];
                     sprintf(name,mPaging_BaseName.c_str(),tile_x,tile_y);

                     osg::PagedLOD* pPage= new osg::PagedLOD();
                     osg::Vec3 pagingOffset; // non declared variable from lm code, (wasnt in the ter file)

                     pPage->setCenter(osg::Vec3((tile_x * mPaging_Delta + mPaging_Delta/2.0),
                                                (tile_y * mPaging_Delta + mPaging_Delta/2.0),
                                                mZOffsetForTerrain)); 

                     pPage->setFileName(0,currentTerrainPath + name);
                     pPage->setRange(0,0.0,mPaging_Range);
                     pPage->setRadius(mPaging_Radius);

                     mGroupNodeForTerrain->addChild(pPage);
                  }
               }

               //// Feed nodes into pager system
               pPager->registerPagedLODs(mGroupNodeForTerrain.get());
               pPager->setDoPreCompile(mPaging_Precompile);
               /////////////////////////////////////////////////////////////////////
            }

            if (!GetShaderGroup().empty())
               SetShaderGroup(GetShaderGroup());

            GetMatrixNode()->addChild(mGroupNodeForTerrain.get());
         }
         else
         {
            mNeedToLoad = true;
         }
         mLoadedFile = fileName;
      }

      ///////////////////////////////////////////////////////////////////////////////
      OpenFlightToIVETerrainActorProxy::OpenFlightToIVETerrainActorProxy()
      {

      }

      ///////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActorProxy::BuildPropertyMap()
      {
         const std::string TERRAIN_GROUP_NAME = "LM_Terrain_Properties";
         BaseTerrainActorProxy::BuildPropertyMap();
         OpenFlightToIVETerrainActor &ta = static_cast<OpenFlightToIVETerrainActor&>(GetGameActor());

         AddProperty(new dtDAL::FloatActorProperty("mPaging_Min_X", "mPaging_Min_X",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingMinX),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingMinX),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::FloatActorProperty("mPaging_Min_Y", "mPaging_Min_Y",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingMinY),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingMinY),
            "", TERRAIN_GROUP_NAME));
         AddProperty(new dtDAL::FloatActorProperty("mPaging_Max_X", "mPaging_Max_X",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingMaxX),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingMaxX),
            "", TERRAIN_GROUP_NAME));
         
         AddProperty(new dtDAL::FloatActorProperty("mPaging_Max_Y", "mPaging_Max_Y",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingMaxY),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingMaxY),
            "", TERRAIN_GROUP_NAME));
        
         AddProperty(new dtDAL::FloatActorProperty("mPaging_Delta", "mPaging_Delta",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingDelta),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingDelta),
            "", TERRAIN_GROUP_NAME));
         
         AddProperty(new dtDAL::FloatActorProperty("mPaging_Radius", "mPaging_Radius",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingRadius),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingRadius),
            "", TERRAIN_GROUP_NAME));
         
         AddProperty(new dtDAL::FloatActorProperty("mPaging_Range", "mPaging_Range",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingRange),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingRange),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::StringActorProperty("mPaging_BaseName", "mPaging_BaseName",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingBaseName),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingBaseName),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::FloatActorProperty("mPaging_ExpiringDelay", "mPaging_ExpiringDelay",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingExpiringDelay),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingExpiringDelay),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::FloatActorProperty("mPaging_Frame_Rate_Targeted", "mPaging_Frame_Rate_Targeted",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingFPSTarget),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingFPSTarget),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::BooleanActorProperty("mPaging_Precompile", "mPaging_Precompile",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetPagingPrecompile),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetPagingPrecompile),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::IntActorProperty("mMaximumObjectsToCompile", "mMaximumObjectsToCompile",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetMaxObjectsToCompile),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetMaxObjectsToCompile),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::FloatActorProperty("mZOffsetForTerrain", "mZOffsetForTerrain",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetZOffsetforTerrain),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetZOffsetforTerrain),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtDAL::StringActorProperty("mTerrainPath", "mTerrainPath",
            dtDAL::MakeFunctor(ta, &OpenFlightToIVETerrainActor::SetTerrainPath),
            dtDAL::MakeFunctorRet(ta, &OpenFlightToIVETerrainActor::GetTerrainPath),
            "", TERRAIN_GROUP_NAME));
      }
   }
}
