#ifndef __LEVEL_COMPILER_UTIL_H__
#define __LEVEL_COMPILER_UTIL_H__

#include "Primitives.h"

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/TriangleFunctor>

#include <osg/TriangleIndexFunctor>

#include <vector>

namespace LevelCompiler
{
   template <class T>
   class CompareStateSet
   {
   public:
      bool operator()(const T& ss1, const T& ss2) const
      {
         return ss1->compare(*ss2,false) < 0;
      }
   };

   class Util
   {
      public:
         static void FlattenGeodeTransformAndState(osg::NodePath& path);
   };


} //namespace LevelCompiler

#endif //__LEVEL_COMPILER_UTIL_H__
