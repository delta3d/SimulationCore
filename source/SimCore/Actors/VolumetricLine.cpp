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

#include <prefix/SimCorePrefix.h>
#include <dtCore/shadermanager.h>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/MatrixTransform>
#include <SimCore/Actors/VolumetricLine.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Drawable Code
      //////////////////////////////////////////////////////////
      VolumetricLine::VolumetricLine( float lineLength, float thickness,
               const std::string& shaderName, const std::string& shaderGroup )
      : dtCore::Transformable("VolumetricLine")
      {
         dtCore::RefPtr<dtCore::ShaderProgram> shader = 
                  dtCore::ShaderManager::GetInstance().FindShaderPrototype( shaderName, shaderGroup );

         if( ! shader.valid() ) { return; }

         // TODO: make tracer color dynamic
         //      dtCore::ShaderParameter* param = shader->FindParameter("");

         // COLOR - used for other data: UVs , line thickness and length
         mData = new osg::Vec4Array( 4 );
         (*mData)[0].set( 0.0f,  0.0f,  -thickness, lineLength );
         (*mData)[1].set( 0.25f, 0.0f,   thickness, lineLength );
         (*mData)[2].set( 0.25f, 0.25f, -thickness, lineLength );
         (*mData)[3].set( 0.0f,  0.25f,  thickness, lineLength );

         //lineLength *= 0.5f;

         osg::Vec3 start( 0.0f, 0.0f, 0.0f );
         osg::Vec3 end( 0.0f, -lineLength, 0.f );

         // VERTICES
         mVerts = new osg::Vec3Array( 4 );
         (*mVerts)[0].set( start );
         (*mVerts)[1].set( end );
         (*mVerts)[2].set( end );
         (*mVerts)[3].set( start );

         // NORMALS
         dtCore::RefPtr<osg::Vec3Array> norms = new osg::Vec3Array( 4 );
         (*norms)[0].set( 0.0f, -1.0f, 0.0f ); // point to end
         (*norms)[1].set( 0.0f, 1.0f, 0.0f ); // point to start
         (*norms)[2].set( 0.0f, 1.0f, 0.0f ); // point to start
         (*norms)[3].set( 0.0f, -1.0f, 0.0f ); // point to end

         // STATES
         dtCore::RefPtr<osg::StateSet> states = new osg::StateSet();
         states->setMode(GL_BLEND,osg::StateAttribute::ON);
         states->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
         states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

         // GEOMETRY
         mGeom = new osg::Geometry;
         mGeom->setColorArray( mData.get() ); // use colors for uvs and other parameters
         mGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
         mGeom->setNormalArray( norms.get() );
         mGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
         mGeom->setVertexArray( mVerts.get() );

         // Make sure the geometry knows that it is a quad
         mGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, mVerts->size()));

         dtCore::RefPtr<osg::Geode> geode = new osg::Geode;
         geode->setStateSet( states.get() );
         geode->addDrawable( mGeom.get() );

         // Attach the shader
         dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype( *shader, *geode );

         GetMatrixNode()->addChild( geode.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      bool VolumetricLine::IsValid() const
      {
         return mData.valid() 
                  && mVerts.valid()
                  && mData->getNumElements() == 4
                  && mVerts->getNumElements() == 4;
      }

      //////////////////////////////////////////////////////////////////////////
      void VolumetricLine::SetLength( float lineLength )
      {
         if( ! IsValid() )
         {
            return;
         }

         // Update the vertex data
         (*mData)[0][3] = lineLength;
         (*mData)[1][3] = lineLength;
         (*mData)[2][3] = lineLength;
         (*mData)[3][3] = lineLength;

         // Adjust end points
         (*mVerts)[0][1] = 0.0f; // start
         (*mVerts)[1][1] = -lineLength; // end
         (*mVerts)[2][1] = -lineLength; // end
         (*mVerts)[3][1] = 0.0f; // start

         mGeom->setVertexArray( mVerts.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      float VolumetricLine::GetLength() const
      {
         if( ! IsValid() )
         {
            return 0.0f;
         }
         return (*mData)[0][3];
      }

      //////////////////////////////////////////////////////////////////////////
      void VolumetricLine::SetThickness( float lineThickness )
      {
         if( ! IsValid() )
         {
            return;
         }

         (*mData)[0][2] = -lineThickness;
         (*mData)[1][2] =  lineThickness;
         (*mData)[2][2] = -lineThickness;
         (*mData)[3][2] =  lineThickness;
      }

      //////////////////////////////////////////////////////////////////////////
      float VolumetricLine::GetThickness() const
      {
         if( ! IsValid() )
         {
            return 0.0f;
         }

         return (*mData)[1][2]; // the second Vec4 has positive thickness
      }

      //////////////////////////////////////////////////////////////////////////
      void VolumetricLine::SetLengthAndThickness( float lineLength, float lineThickness )
      {
         SetLength( lineLength );
         SetThickness( lineThickness );
      }

   }
}
