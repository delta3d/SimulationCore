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
#include <SimCore/Actors/OpenFlightToIVETerrain.h>

#include <dtUtil/fileutils.h>

#include <dtCore/floatactorproperty.h>
#include <dtCore/stringactorproperty.h>
#include <dtCore/intactorproperty.h>
#include <dtCore/booleanactorproperty.h>
#include <dtCore/actorproxyicon.h>
#include <dtCore/datatype.h>

#include <dtCore/shadergroup.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

// Curt test
#include <dtCore/view.h>
#include <dtABC/application.h>

#include <osg/MatrixTransform>
#include <osg/Node>

#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
 
namespace SimCore
{
   namespace Actors
   {
      ///////////////////////////////////////////////////////////////////////////////
      OpenFlightToIVETerrainActor::OpenFlightToIVETerrainActor(dtGame::GameActorProxy& owner)
      : IGActor(owner)
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
      , mPaging_Precompile(true)
      , mMaximumObjectsToCompile(25) // used to be 1
      , mZOffsetForTerrain(100.0f)
      , mTerrainPath("")
      , mNeedToLoad(true)
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

         if (scene != NULL)
         {
            //Actually load the file, even if it's empty string so that if someone were to
            //load a mesh, remove it from the scene, then try to clear the mesh, this actor will still
            //work.
            if (mNeedToLoad)
            {
               LoadTerrain();
               mNeedToLoad = false;
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActor::LoadTerrain()
      {
         //Don't actually load the file unless
         if (GetSceneParent() != NULL)
         {
            //We should always clear the geometry.  If LoadFile fails, we should have no geometry.
            if (GetMatrixNode()->getNumChildren() != 0)
            {
               GetMatrixNode()->removeChild(0, GetMatrixNode()->getNumChildren());
            }

            if (!mTerrainPath.empty() && !mPaging_BaseName.empty())
            {
               /////////////////////////////////////////////////////////////////////
               // do loading here
               std::string currentTerrainPath = mTerrainPath;

               // Make sure it has correct system slashes.
               for (unsigned i = 0; i < currentTerrainPath.size(); ++i)
               {
                  if (currentTerrainPath[i] == '/' || currentTerrainPath[i] == '\\')
                  {
                     currentTerrainPath[i] = dtUtil::FileUtils::PATH_SEPARATOR;
                  }
               }

               // Be sure there is a trailing slash.
               if (*currentTerrainPath.rbegin() != '/' && *currentTerrainPath.rbegin() != '\\')
               {
                  currentTerrainPath += dtUtil::FileUtils::PATH_SEPARATOR;
               }

               osgDB::ReaderWriter::Options* options = new osgDB::ReaderWriter::Options;
               options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
               osgDB::Registry::instance()->setOptions(options);

               int tile_x =0 ,tile_y =0;

               int pagingMaxXInt = int(mPaging_Max_X);
               int pagingMaxYInt = int(mPaging_Max_Y);
               mGroupNodeForTerrain = new osg::Group;

               for (tile_x = int(mPaging_Min_X);tile_x <= pagingMaxXInt;tile_x++)
               {
                  for (tile_y = int(mPaging_Min_Y);tile_y <= pagingMaxYInt;tile_y++)
                  {
                     char name[512];
                     sprintf(name,mPaging_BaseName.c_str(),tile_x,tile_y);

                     osg::PagedLOD* pPage= new osg::PagedLOD();
                     osg::Vec3 pagingOffset; // non declared variable from lm code, (wasnt in the ter file)

                     pPage->setCenter(osg::Vec3((tile_x * mPaging_Delta + mPaging_Delta/2.0),
                                                (tile_y * mPaging_Delta + mPaging_Delta/2.0),
                                                mZOffsetForTerrain)); 

                     pPage->setFileName(0, currentTerrainPath + name);
                     pPage->setRange(0, 0.0, mPaging_Range);
                     pPage->setRadius(mPaging_Radius);

                     mGroupNodeForTerrain->addChild(pPage);
                  }
               }

            }

            if (!GetShaderGroup().empty())
            {
               SetShaderGroup(GetShaderGroup());
            }

            GetMatrixNode()->addChild(mGroupNodeForTerrain.get());
         }
         else
         {
            mNeedToLoad = true;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActor::SetTerrainPath(const std::string& value)
      {
         if (value == mTerrainPath)
         {
            return;
         }

         mTerrainPath = value;

         LoadTerrain();
      }

      ///////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActor::SetPagingBaseName(const std::string& value)
      {
         if (value == mPaging_BaseName)
         {
            return;
         }

         mPaging_BaseName = value;

         LoadTerrain();
      }

      ///////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////
      OpenFlightToIVETerrainActorProxy::OpenFlightToIVETerrainActorProxy()
      {
         SetClassName("SimCore::Actors::OpenFlightToIVETerrainActor");
      }

      ///////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActorProxy::BuildPropertyMap()
      {
         const std::string TERRAIN_GROUP_NAME = "LM_Terrain_Properties";
         BaseClass::BuildPropertyMap();
         OpenFlightToIVETerrainActor* ta = NULL;
         GetDrawable(ta);

         AddProperty(new dtCore::FloatActorProperty("mPaging_Min_X", "mPaging_Min_X",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingMinX),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingMinX),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::FloatActorProperty("mPaging_Min_Y", "mPaging_Min_Y",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingMinY),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingMinY),
            "", TERRAIN_GROUP_NAME));
         AddProperty(new dtCore::FloatActorProperty("mPaging_Max_X", "mPaging_Max_X",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingMaxX),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingMaxX),
            "", TERRAIN_GROUP_NAME));
         
         AddProperty(new dtCore::FloatActorProperty("mPaging_Max_Y", "mPaging_Max_Y",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingMaxY),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingMaxY),
            "", TERRAIN_GROUP_NAME));
        
         AddProperty(new dtCore::FloatActorProperty("mPaging_Delta", "mPaging_Delta",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingDelta),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingDelta),
            "", TERRAIN_GROUP_NAME));
         
         AddProperty(new dtCore::FloatActorProperty("mPaging_Radius", "mPaging_Radius",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingRadius),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingRadius),
            "", TERRAIN_GROUP_NAME));
         
         AddProperty(new dtCore::FloatActorProperty("mPaging_Range", "mPaging_Range",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingRange),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingRange),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::StringActorProperty("mPaging_BaseName", "mPaging_BaseName",
            dtCore::StringActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingBaseName),
            dtCore::StringActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingBaseName),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::FloatActorProperty("mPaging_ExpiringDelay", "mPaging_ExpiringDelay",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingExpiringDelay),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingExpiringDelay),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::FloatActorProperty("mPaging_Frame_Rate_Targeted", "mPaging_Frame_Rate_Targeted",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingFPSTarget),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingFPSTarget),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::BooleanActorProperty("mPaging_Precompile", "mPaging_Precompile",
            dtCore::BooleanActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetPagingPrecompile),
            dtCore::BooleanActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetPagingPrecompile),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::IntActorProperty("mMaximumObjectsToCompile", "mMaximumObjectsToCompile",
            dtCore::IntActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetMaxObjectsToCompile),
            dtCore::IntActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetMaxObjectsToCompile),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::FloatActorProperty("mZOffsetForTerrain", "mZOffsetForTerrain",
            dtCore::FloatActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetZOffsetforTerrain),
            dtCore::FloatActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetZOffsetforTerrain),
            "", TERRAIN_GROUP_NAME));

         AddProperty(new dtCore::StringActorProperty("mTerrainPath", "mTerrainPath",
            dtCore::StringActorProperty::SetFuncType(ta, &OpenFlightToIVETerrainActor::SetTerrainPath),
            dtCore::StringActorProperty::GetFuncType(ta, &OpenFlightToIVETerrainActor::GetTerrainPath),
            "", TERRAIN_GROUP_NAME));
      }

      /////////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActorProxy::CreateDrawable()
      {
         SetDrawable(*new OpenFlightToIVETerrainActor(*this));
      }

      /////////////////////////////////////////////////////////////////////////////////
      void OpenFlightToIVETerrainActorProxy::RemovedFromScene( dtCore::Scene* scene )
      {
         scene->ResetDatabasePager();
      }
   }
}
