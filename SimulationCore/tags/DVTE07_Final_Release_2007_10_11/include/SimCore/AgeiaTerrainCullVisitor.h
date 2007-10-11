/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
#ifndef AGEIA_TERRAIN_CULL_VISITOR
#define AGEIA_TERRAIN_CULL_VISITOR

#include <SimCore/Export.h>
#include <dtCore/observerptr.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <osgUtil/CullVisitor>
#include <osg/Transform>
#include <osg/Geode>
#include <osg/Vec3>
#include <osg/LOD>

namespace SimCore
{
   ///////////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT AgeiaTerrainCullVisitor : public osgUtil::CullVisitor
   {
      public:
         /// Constructor
         AgeiaTerrainCullVisitor();

         /// In Here we load objects within radius to physics
           virtual void apply(osg::Geode& node);

         /// In here we check the top lvl node and finalize the physics terrain
         virtual void apply(osg::Transform& node);

         /// In here we let tiles in our range pass through so we keep them in range
         virtual void apply(osg::LOD& node);

         /// Used for proxy node tests.
         virtual void apply(osg::Group& node);

         /// Set and Get For the land Actor
         void SetLandActor(NxAgeiaTerraPageLandActor* _land) {mLandActor = _land;}
         NxAgeiaTerraPageLandActor* GetLandActor() {return mLandActor.get();} 

         /// Set and get for the terrain node
         void SetTerrainNode(osg::Transform* terrainNodeCheckAgainst){mTerrainNode = terrainNodeCheckAgainst;}
         osg::Node* GetTerrainNode() {return mTerrainNode.get();}

              /// Set and get for the camera transform
         void SetCameraTransform(const osg::Vec3& camTransform) {mCameraPosition = camTransform;}
         osg::Vec3 GetCameraTransform() const {return mCameraPosition;}

      protected:
         /// Destructor
         virtual ~AgeiaTerrainCullVisitor()
         {
            mLandActor = NULL;
            mTerrainNode = NULL;
         }

      private:
         /// we feed the terrain data through here, it loads the physics
         dtCore::ObserverPtr<NxAgeiaTerraPageLandActor>  mLandActor;

         /// this is the top level transform node of the terrain, for knowing when
         /// we are in the terrain
         dtCore::ObserverPtr<osg::Transform>             mTerrainNode;
         
         /// are we in the terrain currently? mNodeWeCheckAgainst passed, so work 
         /// is being done this frame
         bool                                            mCurrentlyInTerrain;

         /// The camera position we test for distance for lod calculations
         osg::Vec3                                       mCameraPosition;

         /// radius check for loading tiles to physics
         static const int                                mRadius  = 1250;

         /// radius check for paging in tiles
         static const int                                mPagingDistance = 7500;

         /// How often we do terrain stuff....
         static const int                                mCheckTerrainAmount = 30;

         /// iter used with mCheckTerrainAmount
         int                                             mTerrainStep;

         /// Set to true when mTerrainStep > mCheckTerrainAmount , and both
         /// observer nodes are valid, then set to true or false, if the loading
         /// of the mLandActor->FinalizeTerrain is complete.
         bool                                            mRunFinalizeTerrain;

         /// This is for proxy nodes that arent loaded into the basic geode structure 
         /// of the terrain.
         bool                                            mHitProxyNode;
   };
} // namespace

#endif
