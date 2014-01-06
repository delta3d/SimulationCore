#ifndef __GENERATE_NORMALS_VISITOR_H__
#define __GENERATE_NORMALS_VISITOR_H__


#include <osg/NodeVisitor> 
#include <osg/Transform>
#include <osg/Geometry>
#include <vector>

#include "NormalGenerator.h"

namespace LevelCompiler
{

class GenerateNormalsVisitor: public osg::NodeVisitor
{
public:
   GenerateNormalsVisitor(bool calcNormals, bool regenNormals)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
      , mCalculateNormals(calcNormals)
      , mRegenerateNormals(regenNormals)
   {

   }


   virtual void apply(osg::Geode& geode)
   { 
      for(unsigned i = 0; i < geode.getNumDrawables(); ++i) 
      {
         osg::Geometry* geo = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
         if (geo != nullptr) 
         {
            osg::ref_ptr<NormalGenerator> normGen = new NormalGenerator();
          
            bool validNormals = normGen->ValidateNormals(geo);

            osg::Vec3Array* normalArray = dynamic_cast<osg::Vec3Array*>(geo->getNormalArray());
            if (!validNormals || (normalArray == nullptr && mCalculateNormals) || mRegenerateNormals)
            {
               normGen->Generate(geo);

               geo->setNormalArray(normGen->getNormalArray());
               geo->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);               
            }
         }
      }

      traverse(geode);
   }



   private:

   bool mCalculateNormals, mRegenerateNormals;

};

} //namespace LevelCompiler

#endif //__GENERATE_NORMALS_VISITOR_H__
