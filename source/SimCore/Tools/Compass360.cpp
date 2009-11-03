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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <dtDAL/project.h>
#include <SimCore/Tools/Compass360.h>



namespace SimCore
{
   namespace Tools
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      Compass360::Compass360()
         : BaseClass(NULL)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      Compass360::~Compass360()
      {
         // Cleanup.
         DetachFromScene();
      }

      //////////////////////////////////////////////////////////////////////////
      void Compass360::Init(osg::Group& sceneNode)
      {
         CreateDrawable();

         sceneNode.addChild(mRoot.get());
      }

      //////////////////////////////////////////////////////////////////////////
      void Compass360::Enable(bool enable)
      {
         BaseClass::Enable(enable);

         if(mRoot.valid())
         {
            mRoot->setNodeMask(enable?0xFFFFFFFF:0x0);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void Compass360::CreateDrawable()
      {
         mRoot = new osg::Projection;
         mRoot->setMatrix(osg::Matrix::ortho2D(0.0f ,1.0f, 0.0f, 0.75f));//WIN_HEIGHT_RATIO));

         mDrawable = new osg::MatrixTransform;
         mRoot->addChild(mDrawable.get());

         osg::Geode* geode = new osg::Geode;
         osg::Geometry* geometry = new osg::Geometry;

         // Setup the node tree
         mDrawable->addChild(geode);
         geode->addDrawable(geometry);

         // VERTICES
         float zDepth = -10.0f;
         dtCore::RefPtr<osg::Vec3Array> verts = new osg::Vec3Array;
         verts->push_back(osg::Vec3(0.0, 0.0, zDepth)); // LEFT-BOTTOM
         verts->push_back(osg::Vec3(1.0, 0.0, zDepth)); // RIGHT-BOTTOM
         verts->push_back(osg::Vec3(1.0, 1.0, zDepth)); // RIGHT-TOP
         verts->push_back(osg::Vec3(0.0, 1.0, zDepth)); // LEFT-TOP
         //*/

         /*verts->push_back(osg::Vec3(0.0, zDepth, 0.0)); // LEFT-BOTTOM
         verts->push_back(osg::Vec3(1.0, zDepth, 0.0)); // RIGHT-BOTTOM
         verts->push_back(osg::Vec3(1.0, zDepth, 1.0)); // RIGHT-TOP
         verts->push_back(osg::Vec3(0.0, zDepth, 1.0)); // LEFT-TOP
         //*/

         // COLOR
         dtCore::RefPtr<osg::Vec4Array> color = new osg::Vec4Array;
         color->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

         // UVS
         dtCore::RefPtr<osg::Vec2Array> uvs = new osg::Vec2Array(4);
         osg::Vec4 rect(0.0,0.0,1.0f,1.0f);
         (*uvs)[0].set( rect.x(),          rect.y());          // LEFT-BOTTOM
         (*uvs)[1].set( rect.x()+rect.z(), rect.y());          // RIGHT-BOTTOM
         (*uvs)[2].set( rect.x()+rect.z(), rect.y()+rect.w()); // RIGHT-TOP
         (*uvs)[3].set( rect.x(),          rect.y()+rect.w()); // LEFT-TOP

         // NORMALS
         dtCore::RefPtr<osg::Vec3Array> norms = new osg::Vec3Array;
         norms->push_back(osg::Vec3(0.0f,1.0f,0.0f));

         // STATES
         osg::StateSet* states = geometry->getOrCreateStateSet();
         states->setMode(GL_BLEND,osg::StateAttribute::ON);
         states->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
         states->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
         states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
//         states->setRenderBinDetails(BIN_NUMBER, BIN_NAME);

         // Get the current texture directory
         /*if( ! imageFileName.empty())
         {
            // Set the texture
            dtCore::RefPtr<osg::Texture2D> texture = new osg::Texture2D;
            texture->setDataVariance(osg::Object::DYNAMIC);
            osg::Image* image;
            std::string filePath("Textures:");
            filePath += imageFileName;
            try
            {
               filePath = dtDAL::Project::GetInstance().GetResourcePath(dtDAL::ResourceDescriptor(filePath));
               image = osgDB::readImageFile(filePath);
               texture->setImage(image);
               states->setTextureAttributeAndModes(0,texture.get(),osg::StateAttribute::ON);
            }
            catch(dtUtil::Exception& e)
            {
               std::stringstream ss;
               ss << "HUDQuadElement could not load texture \"" << filePath << "\" because:\n"
                  << e.ToString() << std::endl;
               LOG_ERROR(ss.str());
            }
         }*/

         // Setup the geometry
         geometry->setNormalArray(norms.get());
         geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
         geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
         geometry->setTexCoordArray(0,uvs.get());
         geometry->setVertexArray(verts.get());
         geometry->setColorArray(color.get());
         geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
      }

      //////////////////////////////////////////////////////////////////////////
      void Compass360::DetachFromScene()
      {
         if(mRoot.valid())
         {
            int numParents = mRoot->getNumParents();

            osg::Group* curParent = NULL;
            for(int parentIndex = 0; parentIndex != numParents; ++parentIndex)
            {
               curParent = dynamic_cast<osg::Group*>(mRoot->getParent(parentIndex));
               if(curParent != NULL)
               {
                  curParent->removeChild(mRoot.get());
               }
            }
         }
      }

   }
}
