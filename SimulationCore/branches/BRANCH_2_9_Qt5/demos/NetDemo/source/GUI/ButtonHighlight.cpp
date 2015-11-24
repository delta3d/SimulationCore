/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
 * Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <dtCore/shadermanager.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/project.h>
#include "GUI/ButtonHighlight.h"



namespace NetDemo
{
   namespace GUI
   {
      namespace Effects
      {
         ///////////////////////////////////////////////////////////////////////
         // CODE
         ///////////////////////////////////////////////////////////////////////
         ButtonHighlight::ButtonHighlight()
            : BaseClass()
         {
         }

         ///////////////////////////////////////////////////////////////////////
         ButtonHighlight::~ButtonHighlight()
         {
            Clear();
         }

         ///////////////////////////////////////////////////////////////////////
         void ButtonHighlight::Init(osg::Group& sceneRoot)
         {
            CreateDrawable("");

            // Attach to the scene root.
            sceneRoot.addChild(mRoot.get());
         }

         ///////////////////////////////////////////////////////////////////////
         void ButtonHighlight::Clear()
         {
            if(mRoot.valid())
            {
               int numParents = mRoot->getNumParents();

               osg::Group* curParent = NULL;
               for(int parentIndex = numParents - 1; parentIndex >= 0; --parentIndex)
               {
                  curParent = dynamic_cast<osg::Group*>(mRoot->getParent(parentIndex));
                  if(curParent != NULL)
                  {
                     curParent->removeChild(mRoot.get());
                  }
               }
            }
         }

         ///////////////////////////////////////////////////////////////////////
         void ButtonHighlight::SetScreenBounds(const osg::Vec4& screenBounds, bool flipY)
         {
            // Set the size of the effect.
            float width = screenBounds.z();
            float height = screenBounds.w();
            (*mVerts)[1].x() = width;  // RIGHT-BOTTOM
            (*mVerts)[2].x() = width;  // RIGHT-TOP
            (*mVerts)[2].y() = height; // RIGHT-TOP
            (*mVerts)[3].y() = height; // LEFT-TOP
            mGeom->setVertexArray(mVerts.get());

            // Set the offset of the effect.
            osg::Matrix mtx(mDrawable->getMatrix());
            mtx.setTrans(
               screenBounds.x(),
               (flipY ? 1.0f - height - screenBounds.y() : screenBounds.y()),
               0.0f);
            mDrawable->setMatrix(mtx);
         }

         ///////////////////////////////////////////////////////////////////////
         void ButtonHighlight::SetVisible(bool visible)
         {
            mRoot->setNodeMask(visible ? 0xFFFFFFFF : 0x0);
         }
         
         ///////////////////////////////////////////////////////////////////////
         bool ButtonHighlight::IsVisible() const
         {
            return mRoot->getNodeMask() != 0x0;
         }

         ///////////////////////////////////////////////////////////////////////
         void ButtonHighlight::SetEnabled(bool enabled)
         {
            // TODO:
         }
         
         ///////////////////////////////////////////////////////////////////////
         bool ButtonHighlight::IsEnabled() const
         {
            // TODO:
            return true;
         }

         //////////////////////////////////////////////////////////////////////////
         void ButtonHighlight::CreateDrawable(const std::string& imageFileName)
         {
            mRoot = new osg::Projection;
            mRoot->setMatrix(osg::Matrix::ortho2D(0.0f ,1.0f, 0.0f, 1.0f));

            mDrawable = new osg::MatrixTransform;
            mDrawable->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
            mRoot->addChild(mDrawable.get());

            osg::Geode* geode = new osg::Geode;
            mGeom = new osg::Geometry;

            // Setup the node tree
            mDrawable->addChild(geode);
            geode->addDrawable(mGeom.get());

            // VERTICES
            float zDepth = 0.0f;
            mVerts = new osg::Vec3Array;
            mVerts->push_back(osg::Vec3(0.0, 0.0, zDepth)); // LEFT-BOTTOM
            mVerts->push_back(osg::Vec3(1.0, 0.0, zDepth)); // RIGHT-BOTTOM
            mVerts->push_back(osg::Vec3(1.0, 1.0, zDepth)); // RIGHT-TOP
            mVerts->push_back(osg::Vec3(0.0, 1.0, zDepth)); // LEFT-TOP

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
            norms->push_back(osg::Vec3(0.0f,0.0f,1.0f));

            // STATES
            osg::StateSet* states = mGeom->getOrCreateStateSet();
            states->setMode(GL_BLEND,osg::StateAttribute::ON);
            states->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
            states->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
            states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
            states->setRenderBinDetails(1000, "RenderBin");

            // Get the current texture directory
            if( ! imageFileName.empty())
            {
               // Set the texture
               dtCore::RefPtr<osg::Texture2D> texture = new osg::Texture2D;
               texture->setDataVariance(osg::Object::DYNAMIC);
               osg::Image* image = NULL;
               std::string filePath(imageFileName);
               try
               {
                  filePath = dtCore::Project::GetInstance().GetResourcePath(dtCore::ResourceDescriptor(filePath));
                  image = osgDB::readImageFile(filePath);
                  texture->setImage(image);
                  texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
                  texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
                  states->setTextureAttributeAndModes(0,texture.get(),osg::StateAttribute::ON);
               }
               catch(dtUtil::Exception& e)
               {
                  LOG_ERROR("Compass 360 tool could not load texture \"" + filePath
                     + "\" because:\n" + e.ToString());
               }
            }

            // Setup the geometry
            mGeom->setNormalArray(norms.get());
            mGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
            mGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
            mGeom->setTexCoordArray(0,uvs.get());
            mGeom->setVertexArray(mVerts.get());
            mGeom->setColorArray(color.get());
            mGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

            // Bind the shader to the geometry.
            dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
            dtCore::ShaderProgram* shaderPrototype = sm.FindShaderPrototype("ButtonHighlightShader");
            if(shaderPrototype != NULL)
            {
               sm.AssignShaderFromPrototype(*shaderPrototype, *mRoot);
            }
         }


      }
   }
}
