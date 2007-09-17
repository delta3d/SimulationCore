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

#include <prefix/SimCorePrefix-src.h>
#include <dtCore/shadermanager.h>
#include <osg/Geode>
#include <osg/Geometry>
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
         osg::Vec3 end( 0.0f, lineLength, 0.f );

         // VERTICES
         mVerts = new osg::Vec3Array( 4 );
         (*mVerts)[0].set( start );
         (*mVerts)[1].set( end );
         (*mVerts)[2].set( end );
         (*mVerts)[3].set( start );

         // NORMALS
         osg::Vec3Array* norms = new osg::Vec3Array( 4 );
         (*norms)[0].set( 0.0f, 1.0f, 0.0f ); // point to end
         (*norms)[1].set( 0.0f, -1.0f, 0.0f ); // point to start
         (*norms)[2].set( 0.0f, -1.0f, 0.0f ); // point to start
         (*norms)[3].set( 0.0f, 1.0f, 0.0f ); // point to end

         // STATES
         osg::StateSet* states = new osg::StateSet();
         states->setMode(GL_BLEND,osg::StateAttribute::ON);
         states->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
         states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

         // GEOMETRY
         osg::Geometry* geom = new osg::Geometry;
         geom->setColorArray( mData.get() ); // use colors for uvs and other parameters
         geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
         geom->setNormalArray( norms );
         geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
         geom->setVertexArray( mVerts.get() );

         // Make sure the geometry knows that it is a quad
         geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, mVerts->size()));

         osg::Geode* geode = new osg::Geode;
         geode->setStateSet( states );
         geode->addDrawable( geom );

         // Attach the shader
         dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype( *shader, *geode );

         GetMatrixNode()->addChild( geode );
      }

      //////////////////////////////////////////////////////////////////////////
      void VolumetricLine::SetLength( float lineLength )
      {
         // Update the vertex data
         (*mData)[0][3] = lineLength;
         (*mData)[1][3] = lineLength;
         (*mData)[2][3] = lineLength;
         (*mData)[3][3] = lineLength;

         // Adjust end points
         lineLength *= 0.5f;
         (*mVerts)[0][1] = -lineLength; // start
         (*mVerts)[1][1] =  lineLength; // end
         (*mVerts)[2][1] =  lineLength; // end
         (*mVerts)[3][1] = -lineLength; // start
      }

      //////////////////////////////////////////////////////////////////////////
      float VolumetricLine::GetLength() const
      {
         return (*mData)[0][3];
      }

      //////////////////////////////////////////////////////////////////////////
      void VolumetricLine::SetThickness( float lineThickness )
      {
         (*mData)[0][2] = -lineThickness;
         (*mData)[1][2] =  lineThickness;
         (*mData)[2][2] = -lineThickness;
         (*mData)[3][2] =  lineThickness;
      }

      //////////////////////////////////////////////////////////////////////////
      float VolumetricLine::GetThickness() const
      {
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
