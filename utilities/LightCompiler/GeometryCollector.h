#ifndef __GEOMETRY_COLLECTOR_H__
#define __GEOMETRY_COLLECTOR_H__



#include <osg/NodeVisitor>
#include <osg/LightSource>
#include <osg/Light>
#include <osg/Transform>

#include <vector>



class GeometryCollector: public osg::NodeVisitor
{

public:
   typedef std::vector< osg::ref_ptr<osg::Geometry> > GeometryArray;

public:
   GeometryCollector(osg::Node* rootNode)
      : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
      , mRootNode(rootNode)
   {
      mRootNode->accept(*this);
   }


   GeometryArray& GetGeometryArray()
   {
      return mGeometry;
   }

   virtual void apply(osg::Geode& geode)
   { 
      osg::NodePath nodePath = getNodePath();
 
      for(unsigned i = 0; i < geode.getNumDrawables(); ++i)
      {
         osg::Drawable* d = geode.getDrawable(i);
         osg::Geometry* geom = dynamic_cast<osg::Geometry*>(d);
         if(geom != nullptr)
         {
            mGeometry.push_back(geom);
         }
      }

      traverse(geode);
   }


private:

   osg::ref_ptr<osg::Node> mRootNode;
   GeometryArray mGeometry;

};


#endif //__GEOMETRY_COLLECTOR_H__
