#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>

#include <osg/Group>
#include <osg/Geode>

#include <osg/TriangleIndexFunctor>

#include <vector>

namespace LevelCompiler
{
   typedef unsigned MaterialID;
   typedef unsigned VertexID;

   struct Triangle
   {
      MaterialID mMaterial;
      VertexID mVertices[3];
   };

   typedef std::vector<Triangle> TriangleArray;


   typedef enum 
   {
      VERTEX_C,
      VERTEX_CN,
      VERTEX_CNT,
      VERTEX_NT,
      VERTEX_CT
   } VertexType;


   struct VertexArrayInfo 
   {
      osg::Vec3 v;
      osg::Vec4 c;
      osg::Vec3 n;
      osg::Vec2 t;

      VertexType mType;
   };


} //namespace LevelCompiler

#endif //__PRIMITIVES_H__
