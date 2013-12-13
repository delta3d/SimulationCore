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
 * @author Chris Rodgers
 */
#ifndef _VOLUMETRIC_LINE_H_
#define _VOLUMETRIC_LINE_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/Array>
#include <osg/Geometry>
#include <dtCore/refptr.h>
#include <dtCore/transformable.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Volumetric Line Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT VolumetricLine : public dtCore::Transformable
      {
         public:
            VolumetricLine( float lineLength, float lineThickness,
               const std::string& shaderName, const std::string& shaderGroup );

            bool IsValid() const;

            void SetLength( float lineLength );
            float GetLength() const;

            void SetThickness( float lineThickness );
            float GetThickness() const;

            void SetLengthAndThickness( float lineLength, float lineThickness );

         protected:
            virtual ~VolumetricLine() {}

         private:
            dtCore::RefPtr<osg::Geometry>  mGeom;
            dtCore::RefPtr<osg::Vec3Array> mVerts;
            dtCore::RefPtr<osg::Vec4Array> mData;
      };

   }
}

#endif
