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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/AgeiaTerrainCullVisitor.h>
#include <osg/Billboard>
#include <osg/ProxyNode>

namespace SimCore
{
   ///////////////////////////////////////////////////////////////////////////
   AgeiaTerrainCullVisitor::AgeiaTerrainCullVisitor() : CullVisitor()
      , mCurrentlyInTerrain(false)
      , mTerrainStep(0)
      , mRunFinalizeTerrain(false)
      , mHitProxyNode(false)
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   void AgeiaTerrainCullVisitor::apply(osg::Transform& node)
   {
      if(mTerrainNode.valid() 
         && ++mTerrainStep > mCheckTerrainAmount
         && mLandActor.valid())
      {
         if(&node == mTerrainNode.get())
         {
            mTerrainStep = 0;
            mLandActor->ResetTerrainIterator();
            mCurrentlyInTerrain = true;
            mRunFinalizeTerrain = true;
         }
      }

      osgUtil::CullVisitor::apply(node);
      
      if(&node == mTerrainNode.get())
      {
         mCurrentlyInTerrain = false;
      }

      if(mCurrentlyInTerrain == false && mRunFinalizeTerrain && mLandActor.valid())
      {
         mRunFinalizeTerrain = mLandActor->FinalizeTerrain(mCheckTerrainAmount - 1);
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   void AgeiaTerrainCullVisitor::apply(osg::Geode& node)
   {
      // Terrex terrain for example
      if(mLandActor.valid() && mCurrentlyInTerrain && node.getBoundingBox().valid())
      {
         osg::Vec3 position = node.getBoundingBox().center() - GetCameraTransform();

         osg::Matrix absMatrix;
         if(mHitProxyNode)
         {
            osg::NodePath& nodePath = getNodePath();
            absMatrix.set( osg::computeLocalToWorld(nodePath) );
            position = (node.getBoundingBox().center() + absMatrix.getTrans()) - GetCameraTransform();
         }

         if( position.length() <= mRadius)
         {
            mLandActor->CheckGeode(node, mHitProxyNode, absMatrix);
         }
      }
      // ive terrain for example
      else if(mLandActor.valid() && mCurrentlyInTerrain && node.getBound().valid())
      {
         osg::Vec3 position = node.getBound().center() - GetCameraTransform();
         
         osg::Matrix absMatrix;
         if(mHitProxyNode)
         {
            osg::NodePath& nodePath = getNodePath();
            absMatrix.set( osg::computeLocalToWorld(nodePath) );
            position = (node.getBound().center() + absMatrix.getTrans()) - GetCameraTransform();
         }

         if( position.length() <= mRadius)
         {
            mLandActor->CheckGeode(node, mHitProxyNode, absMatrix);
         }
      }

      osgUtil::CullVisitor::apply(node);
   }

   /////////////////////////////////////////////////////////////////////////
   void AgeiaTerrainCullVisitor::apply(osg::Group& node)
   {
      bool hitframeDontCallOtherCull  = false;
      if(mCurrentlyInTerrain)
      {
         osg::ProxyNode* proxyNode = dynamic_cast<osg::ProxyNode*>(&node);
         if(proxyNode != NULL)
         {
            mHitProxyNode = true;
            osgUtil::CullVisitor::apply(node);
            hitframeDontCallOtherCull = true;
            mHitProxyNode = false;
         }
      }

      if(hitframeDontCallOtherCull == false)
         osgUtil::CullVisitor::apply(node);
   }

   /////////////////////////////////////////////////////////////////////////
   void AgeiaTerrainCullVisitor::apply(osg::LOD& node)
   {
      if(mLandActor.valid() && mCurrentlyInTerrain)
      {
         osg::Vec3 position = node.getCenter() - GetCameraTransform();

         if( position.length() > mPagingDistance)
         {
            osgUtil::CullVisitor::apply(node);
            return;
         }

         // push the culling mode.
         pushCurrentMask();

         // push the node's state.
         osg::StateSet* node_state = node.getStateSet();
         if (node_state) pushStateSet(node_state);

         handle_cull_callbacks_and_traverse(node);

         // pop the node's state off the render graph stack.    
         if (node_state) popStateSet();

         // pop the culling mode.
         popCurrentMask();
      }
      else
      {
         osgUtil::CullVisitor::apply(node);
      }
   }

}