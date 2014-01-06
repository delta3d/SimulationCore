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

#ifndef _SIMCORE_TRAIL_EFFECT_H_
#define _SIMCORE_TRAIL_EFFECT_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/Array>
#include <dtCore/transformable.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace osg
{
   class Geometry;
   class DrawElementsUInt;
}

namespace dtCore
{
   class ShaderProgram;
}



////////////////////////////////////////////////////////////////////////////////
// CODE
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   class SIMCORE_EXPORT TrailEffect : public dtCore::Transformable
   {
      public:
         typedef dtCore::Transformable BaseClass;

         static const int DEFAULT_SEGMENT_COUNT = 10;
         static const float DEFAULT_WIDTH;

         TrailEffect(int segmentCount = DEFAULT_SEGMENT_COUNT);

         void SetSegmentCount(int segmentCount);
         int GetSegmentCount() const;

         void SetWidth(float width);
         float GetWidth() const;

         void SetNextPoint(const osg::Vec3& worldPoint);

         void SetPoints(const osg::Vec3Array& worldPointPath);

         void ResetPoints();

         void BuildDrawable();

      protected:
         virtual ~TrailEffect();

         void ClearDrawable();

         void SetFloatParameter(const std::string& paramName, float value);

      private:
         int mCurrentIndex;
         int mSegmentCount;
         float mWidth;
         osg::ref_ptr<osg::Geometry>  mGeom;
         osg::ref_ptr<osg::Vec3Array> mVerts;
         osg::ref_ptr<osg::Vec4Array> mData;
         osg::ref_ptr<osg::DrawElementsUInt> mIndices;
         std::shared_ptr<dtCore::ShaderProgram> mShader;
   };
}

#endif
