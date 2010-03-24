/* -*-c++-*-
 * Simulation Core
 * Copyright 2009, Alion Science and Technology
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
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/StateSet>
#include <osg/Uniform>
#include <dtCore/shadermanager.h>
#include <dtCore/shaderparamfloat.h>
#include <dtCore/transform.h>
#include <SimCore/TrailEffect.h>



namespace SimCore
{
   /////////////////////////////////////////////////////////////////////////////
   // CODE
   /////////////////////////////////////////////////////////////////////////////
   const float TrailEffect::DEFAULT_WIDTH = 1.0f;

   /////////////////////////////////////////////////////////////////////////////
   TrailEffect::TrailEffect(int segmentCount)
      : mCurrentIndex(0)
      , mSegmentCount(0)
      , mWidth(DEFAULT_WIDTH)
   {
      // This will force build the drawable, if the entered value is valid.
      SetSegmentCount(segmentCount);
   }

   /////////////////////////////////////////////////////////////////////////////
   TrailEffect::~TrailEffect()
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::SetSegmentCount(int segmentCount)
   {
      if(segmentCount >= 1)
      {
         mSegmentCount = segmentCount;
         BuildDrawable();
      }
   }
   
   /////////////////////////////////////////////////////////////////////////////
   int TrailEffect::GetSegmentCount() const
   {
      return mSegmentCount;
   }

   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::SetWidth(float width)
   {
      mWidth = width;
      SetFloatParameter("trailWidth", width);
   }

   /////////////////////////////////////////////////////////////////////////////
   float TrailEffect::GetWidth() const
   {
      return mWidth;
   }

   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::SetNextPoint(const osg::Vec3& worldPoint)
   {
      if( ! mGeom.valid())
      {
         return;
      }

      // Get the index of the last point (vertex pair).
      int numPoints = mSegmentCount + 1;
      int v0 = mCurrentIndex * 2;
      int v1 = v0 + 1;
      osg::Vec4& lastData0 = (*mData)[v0];

      // The last vertex data element will maintain the vector pointing to
      // the new point.
      osg::Vec3 vecToNewPoint(worldPoint - (*mVerts)[v0]);
      lastData0.set(vecToNewPoint.x(), vecToNewPoint.y(), vecToNewPoint.z(), 1.0f);
      (*mData)[v1] = lastData0;
      (*mData)[v1].w() = -1.0f;

      // Set the next index.
      // Wrap the index if it exceeds the vertex array.
      mCurrentIndex = (mCurrentIndex + 1) % numPoints;

      // Get the indexes of the new vertex pair acting as the last point.
      v0 = mCurrentIndex * 2;
      v1 = v0 + 1;

      // The new point vertex data will have the same vector as the previous element,
      // so that vertex pair for the new point can be displaced appropriately.
      (*mData)[v0] = lastData0;
      (*mData)[v1] = lastData0;
      (*mData)[v1].w() = -1.0f;

      // Set the point position on the last vertices.
      (*mVerts)[v0].set(worldPoint);
      (*mVerts)[v1].set(worldPoint);

      // Update the vertices.
      mGeom->setVertexArray(mVerts.get());
      mGeom->setColorArray(mData.get());

      // Change the indices to shift the rendering order of the vertices.
      // The first pair now becomes the last, and the second pair now
      // becomes the first.
      int newIndex = v0;
      int numVerts = numPoints * 2;
      for(int i0 = 0, i1 = 1; i1 < numVerts; i0 += 2, i1 += 2)
      {
         (*mIndices)[i0] = newIndex % numVerts;
         (*mIndices)[i1] = (newIndex + 1) % numVerts;

         //
         newIndex = (newIndex - 2 + numVerts) % numVerts;
      }

      // Update the index array with the new index values.
      mGeom->setPrimitiveSet(0, mIndices.get());
   }

   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::SetPoints(const osg::Vec3Array& worldPointPath)
   {
      int numPoints = int(worldPointPath.size());
      for(int i = 0; i < numPoints; ++i)
      {
         SetNextPoint(worldPointPath[i]);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::ResetPoints()
   {
      if(mGeom.valid())
      {
         // Get this object's current position.
         dtCore::Transform xform;
         GetTransform(xform);
         osg::Vec3 pos;
         xform.GetTranslation(pos);

         // Set the vertices to the current position.
         int numVerts = int(mVerts->size());
         for(int i = 0; i < numVerts; i += 2)
         {
            (*mVerts)[i] = pos;
            (*mVerts)[i + 1] = pos;
         }

         // Update the vertex array.
         mGeom->setVertexArray(mVerts.get());
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::BuildDrawable()
   {
      // Clear out the old drawable if it exists.
      ClearDrawable();

      // Get this object's current position.
      dtCore::Transform xform;
      GetTransform(xform);
      osg::Vec3 pos;
      xform.GetTranslation(pos);

      // Create the arrays.
      int vertexCount = (mSegmentCount + 1) * 2;
      mData = new osg::Vec4Array(vertexCount);
      mVerts = new osg::Vec3Array(vertexCount);
      mIndices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP, vertexCount);
      dtCore::RefPtr<osg::Vec3Array> norms = new osg::Vec3Array(vertexCount);
      dtCore::RefPtr<osg::Vec2Array> uvs = new osg::Vec2Array(vertexCount);

      // Initialize the array elements.
      float uCoord = 0.0f;
      float uStep = 1.0f / float(mSegmentCount);
      for(int v0 = 0, v1 = 1; v1 < vertexCount; v0 += 2, v1 += 2)
      {
         // VERTICES
         (*mVerts)[v0].set(pos);
         (*mVerts)[v1].set(pos);

         // NORMALS
         (*norms)[v0].set(0.0f, 0.0f, 1.0f);
         (*norms)[v1].set(0.0f, 0.0f, 1.0f);

         // INDICES
         (*mIndices)[v0] = v0;
         (*mIndices)[v1] = v1;

         // UVS
         (*uvs)[v0].set(uCoord, 1.0f);
         (*uvs)[v1].set(uCoord, 0.0f);
         uCoord += uStep;

         // COLOR - used for other data: UVs , line thickness and length
         (*mData)[v0].set(0.0f, 0.0f, 0.0f, 1.0f);
         (*mData)[v1] = (*mData)[v0];
         (*mData)[v1].w() = -1.0f;
      }

      // STATES
      dtCore::RefPtr<osg::StateSet> states = GetOSGNode()->getOrCreateStateSet();
      states->setMode(GL_BLEND,osg::StateAttribute::ON);
      states->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
      states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

      // GEOMETRY
      mGeom = new osg::Geometry;
      mGeom->addPrimitiveSet(mIndices.get());
      mGeom->setColorArray(mData.get()); // use colors for uvs and other parameters
      mGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
      mGeom->setNormalArray(norms.get());
      mGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
      mGeom->setVertexArray(mVerts.get());
      mGeom->setTexCoordArray(0, uvs.get());

      // GEODE (GEOMETRY NODE)
      dtCore::RefPtr<osg::Geode> geode = new osg::Geode;
      geode->addDrawable(mGeom.get());
      GetMatrixNode()->addChild(geode.get());

      // Attach the shader
      dtCore::RefPtr<dtCore::ShaderProgram> protoShader
         = dtCore::ShaderManager::GetInstance().FindShaderPrototype("TrailEffectShader","TrailEffectShaderGroup");
      if(protoShader.valid())
      {
         mShader = dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*protoShader, *GetOSGNode());

         if( ! mShader.valid())
         {
            LOG_ERROR("Could not create and attach trail effect shader.");
         }
      }
   }
   
   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::ClearDrawable()
   {
      mCurrentIndex = 0;
      if(mGeom.valid())
      {
         mGeom->setTexCoordArray(0, NULL);
         mGeom->setColorArray(NULL);
         mGeom->setNormalArray(NULL);
         mGeom->setVertexArray(NULL);
      }
      
      mGeom = NULL;
      mVerts = NULL;
      mData = NULL;
      mIndices = NULL;
      mShader = NULL;
   }

   /////////////////////////////////////////////////////////////////////////////
   void TrailEffect::SetFloatParameter(const std::string& paramName, float value)
   {
      if(mShader.valid())
      {
         dtCore::ShaderParamFloat* param = dynamic_cast<dtCore::ShaderParamFloat*>
            (mShader->FindParameter(paramName));
         if(param != NULL)
         {
            param->SetValue(mWidth);
         }
      }
   }
}
