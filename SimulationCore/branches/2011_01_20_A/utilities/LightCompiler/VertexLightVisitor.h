#ifndef __VERTEXLIGHTVISITOR_H__
#define __VERTEXLIGHTVISITOR_H__

#include <osg/NodeVisitor>
#include <osg/Transform>
#include <osg/Geometry>
#include <osg/Geode>

   
#include <iostream>

#include "LightCollector.h"

    
class VertexLightVisitor: public osg::NodeVisitor
{
public:

   VertexLightVisitor(osg::Node* rootNode, LightCollector* lc, float lightEpsilon) 
     : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
     , mLightEpsilon(lightEpsilon)
     , mGeodeCounter(0)
     , mDrawableCounter(0)
     , mRootNode(rootNode)  
     , mLightCollector(lc)
   {
      mRootNode->accept(*this);
      
      //this clears the text line from the progress bar, slightly hacky, but oh well
      std::cout << std::endl;
   }

   unsigned GetGeodeCount() const
   {
      return mGeodeCounter;
   }

   unsigned GetDrawableCount() const
   {
      return mDrawableCounter;
   }
 
   void apply(osg::Geode& node)
   {   
      ++mGeodeCounter;

      for(unsigned i = 0; i < node.getNumDrawables(); ++i)
      {
         osg::Drawable* d = node.getDrawable(i);
         osg::Geometry* geom = dynamic_cast<osg::Geometry*>(d); 
         if(geom)
         {
            osg::NodePath nodePath = getNodePath();
            osg::Matrix mat = osg::computeLocalToWorld(nodePath);  

            ProcessGeometry(mat, geom);
         }
         else
         {
            std::cout << "Found an osg::Drawable named '" << d->getName() << "' that was not an osg::Geometry, only osg::Geometry is supported." << std::endl;
         }

         ++mDrawableCounter;
      } 
   }

private:

   void ProcessGeometry(const osg::Matrix&, osg::Geometry*);
   void ProcessVertex(const osg::Vec3& vert, const osg::Vec3& norm, osg::Vec4& index, osg::Vec4& weight);
   float EvaluateLight(const osg::Vec3& vert, const osg::Vec3& norm, const osg::Light* light);

   osg::Vec3 OSGArrayToVec3(osg::Array* ptr, unsigned index);

   float mLightEpsilon;
   unsigned mGeodeCounter, mDrawableCounter; 
   osg::ref_ptr<osg::Node> mRootNode;
   osg::ref_ptr<LightCollector> mLightCollector;

};

#endif //__VERTEXLIGHTVISITOR_H__
