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
* @author Bradley Anderegg
*/
#ifndef CUSTOM_CULL_VISITOR
#define CUSTOM_CULL_VISITOR

#include <SimCore/Export.h>
#include <dtCore/observerptr.h>
#include <osgUtil/CullVisitor>
#include <osg/Transform>
#include <osg/Geode>
#include <osg/Vec3>
#include <vector>

namespace SimCore
{
   ///////////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT CustomCullVisitor : public osgUtil::CullVisitor
   {
      public:
         /// Constructor
         CustomCullVisitor();

         /// In Here we load objects within radius to physics
         virtual void apply(osg::Geode& node);

         /// In here we check the top lvl node and finalize the physics terrain
         virtual void apply(osg::Transform& node);

         virtual void apply(osg::LOD& node);
         virtual void apply(osg::Group& node);

         /// Set and get for the terrain node
         /// These are static so we can share terrain nodes between cull visitors
         static void SetTerrainNode(osg::Transform* terrainNodeCheckAgainst){mTerrainNode = terrainNodeCheckAgainst;}
         static osg::Transform* GetTerrainNode() {return mTerrainNode.get();}

         static const std::vector<int>& GetDisabledFIDCodes();
         static void SetDisabledFIDCodes(const std::vector<int>& fidCodeArray);
         

      protected:
         /// Destructor
         virtual ~CustomCullVisitor();

      protected:
 
         /// this is the top level transform node of the terrain, for knowing when
         /// we are in the terrain
         static dtCore::ObserverPtr<osg::Transform>             mTerrainNode;

         static std::vector<int>                                mFIDCodes;
         
         /// are we in the terrain currently? mNodeWeCheckAgainst passed, so work 
         /// is being done this frame
         bool                                            mCurrentlyInTerrain;

   };
} // namespace

#endif

