/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */
#ifndef _VOLUMETRIC_LINE_H_
#define _VOLUMETRIC_LINE_H_

#include <SimCore/Export.h>

#include <dtCore/transformable.h>

#include <osg/Array>

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

            void SetLength( float lineLength );
            float GetLength() const;

            void SetThickness( float lineThickness );
            float GetThickness() const;

            void SetLengthAndThickness( float lineLength, float lineThickness );

         protected:
            virtual ~VolumetricLine() {}

         private:

            osg::ref_ptr<osg::Vec3Array> mVerts;
            osg::ref_ptr<osg::Vec4Array> mData;
      };

   }
}

#endif
