#ifndef SCENE_SUBDIVISION_H
#define SCENE_SUBDIVISION_H

#include "Primitives.h"

#include <osg/NodeVisitor>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LOD>
#include <osg/TriangleFunctor>

#include <osg/TriangleIndexFunctor>
#include <osgUtil/Optimizer>
#include <vector>

namespace LevelCompiler
{
   
   class SpatializeGroupsVisitor : public osgUtil::BaseOptimizerVisitor
   {
   public:

      SpatializeGroupsVisitor(osgUtil::Optimizer* optimizer=0):
         osgUtil::BaseOptimizerVisitor(optimizer, osgUtil::Optimizer::SPATIALIZE_GROUPS) {}

         //virtual void apply(osg::LOD& node);

         virtual void apply(osg::Group& group);
         virtual void apply(osg::Geode& geode);

         bool divide(unsigned int maxNumTreesPerCell=8);
         bool divide(osg::Group* group, unsigned int maxNumTreesPerCell);
         bool divide(osg::Geode* geode, unsigned int maxNumTreesPerCell);

         void combine(osg::LOD* lod, unsigned int maxNumTreesPerCell);

   private:
         typedef std::set<osg::LOD*> LODsToCombineList;
         LODsToCombineList mLODsToCombine;

         typedef std::set<osg::Group*> GroupsToDivideList;
         GroupsToDivideList mGroupsToDivide;

         typedef std::set<osg::Geode*> GeodesToDivideList;
         GeodesToDivideList mGeodesToDivide;
   };

} //namespace LevelCompiler

#endif //SCENE_SUBDIVISION_H
