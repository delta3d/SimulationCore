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
#include <SimCore/CustomCullVisitor.h>
#include <osg/Billboard>
#include <osg/ProxyNode>
#include <iostream>

namespace SimCore
{

   dtCore::ObserverPtr<osg::Transform> CustomCullVisitor::mTerrainNode;
   std::vector<int> CustomCullVisitor::mFIDCodes;

   ///////////////////////////////////////////////////////////////////////////
   CustomCullVisitor::CustomCullVisitor() : CullVisitor()
      , mCurrentlyInTerrain(false)
   {
   }

   CustomCullVisitor::~CustomCullVisitor()
   {

   }


   ///////////////////////////////////////////////////////////////////////////
   void CustomCullVisitor::apply(osg::Transform& node)
   {
      if(mTerrainNode.valid())
      {
         //std::cout << "Terrain Node: " << mTerrainNode->getName() << std::endl;
         if(&node == mTerrainNode.get())
         {
            mCurrentlyInTerrain = true;
         }
      }

      osgUtil::CullVisitor::apply(node);
      
      if(&node == mTerrainNode.get())
      {
         mCurrentlyInTerrain = false;
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   void CustomCullVisitor::apply(osg::Geode& node)
   {

      for(unsigned int i=0;i<node.getNumDrawables();i++)
      {
         osg::Drawable* d = node.getDrawable(i);
         osg::StateSet* tempStateSet = d->getStateSet();
         
         if(tempStateSet != NULL)
         {
            osg::IntArray* intArray = dynamic_cast<osg::IntArray*>(tempStateSet->getUserData());
            
            if(intArray != NULL)
            {
               std::vector<int>::iterator iter = std::find(mFIDCodes.begin(), mFIDCodes.end(), intArray->at(0));

               if(iter == mFIDCodes.end())
               {
                  osgUtil::CullVisitor::apply(node);
               }
            }
            else
            {
               osgUtil::CullVisitor::apply(node);
            }

         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   void CustomCullVisitor::apply( osg::LOD& node )
   {
      osgUtil::CullVisitor::apply(node);
   }

   ///////////////////////////////////////////////////////////////////////////
   void CustomCullVisitor::apply( osg::Group& node )
   {
      osgUtil::CullVisitor::apply(node);
   }

   ///////////////////////////////////////////////////////////////////////////
   const std::vector<int>& CustomCullVisitor::GetDisabledFIDCodes()
   {
      return mFIDCodes;
   }

   ///////////////////////////////////////////////////////////////////////////
   void CustomCullVisitor::SetDisabledFIDCodes( const std::vector<int>& fidCodeArray )
   {
      mFIDCodes = fidCodeArray;
   }
}

