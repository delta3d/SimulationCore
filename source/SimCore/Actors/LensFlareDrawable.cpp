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
 * Bradley Anderegg
 */


#include <SimCore/Actors/LensFlareDrawable.h>

#include <dtUtil/exception.h>
#include <dtUtil/matrixutil.h>
#include <dtDAL/project.h>
#include <dtCore/scene.h>
#include <dtABC/application.h>

#include <SimCore/Components/RenderingSupportComponent.h>

#include <osg/Group>
#include <osg/State>
#include <osg/StateSet>
#include <osg/Billboard>
#include <osg/BlendFunc>

#include <osgDB/ReadFile>

namespace SimCore
{
   namespace Actors
   {

      const dtUtil::RefString LensFlareDrawable::TEXTURE_LENSFLARE_SOFT_GLOW("Textures:sun_glare_soft_glow.bmp");      
      const dtUtil::RefString LensFlareDrawable::TEXTURE_LENSFLARE_HARD_GLOW("Textures:sun_glare_hard_glow.bmp");
      const dtUtil::RefString LensFlareDrawable::TEXTURE_LENSFLARE_STREAKS("Textures:sun_glare_streaks_01.bmp");


      LensFlareDrawable::LensFlareDrawable()
         : dtCore::DeltaDrawable("LensFlareDrawable")
      {

      }

      LensFlareDrawable::~LensFlareDrawable()
      {
         
      }

      osg::Node* SimCore::Actors::LensFlareDrawable::GetOSGNode()
      {
         return mNode.get();
      }

      const osg::Node* SimCore::Actors::LensFlareDrawable::GetOSGNode() const
      {
         return mNode.get();
      }

      void LensFlareDrawable::Init()
      { 
         mLensFlareDrawable = new LensFlareDrawable::LensFlareOSGDrawable();
         
         osg::Geode* geode = new osg::Geode();

         geode->addDrawable(mLensFlareDrawable.get());
         geode->setCullingActive(false);

         osg::StateSet* ss = geode->getOrCreateStateSet();
         ss->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD - 1, "TransparentBin");               
         ss->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
         ss->setMode( GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );

         osg::BlendFunc* bf = new osg::BlendFunc(GL_ONE, GL_ONE);
         ss->setMode(GL_BLEND, osg::StateAttribute::ON);
         ss->setAttributeAndModes(bf, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
         ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

         osg::Projection* projection = new osg::Projection;
         projection->setMatrix(osg::Matrix::ortho2D(0,1,0,1));
         projection->addChild(geode);

         mNode = new osg::MatrixTransform;
         mNode->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
         mNode->setMatrix(osg::Matrix::identity());
         mNode->addChild(projection);      
      }

      void LensFlareDrawable::Update(const osg::Vec3& lightPos)
      {
         mLensFlareDrawable->SetLightPos(lightPos);    
      }
     
      
      LensFlareDrawable::LensFlareOSGDrawable::LensFlareOSGDrawable()
      {
         setUseDisplayList(false);
         LoadTextures();
      }


      void LensFlareDrawable::LensFlareOSGDrawable::LoadTextures()
      {      
         dtDAL::Project& project = dtDAL::Project::GetInstance();
         
         try
         {      
            std::string softGlowFile = project.GetResourcePath(dtDAL::ResourceDescriptor(TEXTURE_LENSFLARE_SOFT_GLOW));
            mSoftGlow = new osg::Texture2D();
            InitTexture(softGlowFile, mSoftGlow.get());

            std::string hardGlowFile = project.GetResourcePath(dtDAL::ResourceDescriptor(TEXTURE_LENSFLARE_HARD_GLOW));
            mHardGlow = new osg::Texture2D();
            InitTexture(hardGlowFile, mHardGlow.get());

            std::string streaksFile = project.GetResourcePath(dtDAL::ResourceDescriptor(TEXTURE_LENSFLARE_STREAKS));
            mStreaks = new osg::Texture2D();
            InitTexture(streaksFile, mStreaks.get());
         }
         catch (dtUtil::Exception& e)
         {
            e.LogException(dtUtil::Log::LOG_ERROR);
         }
      }

      void LensFlareDrawable::LensFlareOSGDrawable::InitTexture( const std::string& filename, osg::Texture2D* ptr )
      {
         osg::Image* newImage = osgDB::readImageFile(filename);
         if (newImage == 0)
         {
            LOG_ERROR("Failed to load Texture file [" + filename + "].");
         }

         ptr->setImage(newImage);
         ptr->dirtyTextureObject();
         ptr->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
         ptr->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
         ptr->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
         ptr->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
         ptr->setUnRefImageDataAfterApply(true);
      }

      void LensFlareDrawable::LensFlareOSGDrawable::EnableTextureState(osg::RenderInfo& renderInfo) const
      {
         renderInfo.getState()->setClientActiveTextureUnit(0);
         glDisable(GL_TEXTURE_2D);

         renderInfo.getState()->setActiveTextureUnit(0);
         glEnable(GL_TEXTURE_2D);
      }

      void LensFlareDrawable::LensFlareOSGDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
      {
         if(renderInfo.getState() && mHardGlow.valid() && mSoftGlow.valid() && mStreaks.valid())
         {
            float softGlowScale = 1.0;
            float hardGlowScale = 0.25;
            float streaksScale = 0.75;

            //calculate the screen position of the light
            osg::Camera* cam = renderInfo.getCurrentCamera();
            osg::Vec4d screenXYZ(mLightPos.x(), mLightPos.y(), mLightPos.z(), 1.0);
            screenXYZ = cam->getViewMatrix().preMult(screenXYZ);
            screenXYZ = cam->getProjectionMatrix().preMult(screenXYZ);
            bool inFrontOfCamera = (screenXYZ.w() > 0.0f);
            screenXYZ /= screenXYZ.w();
            screenXYZ += osg::Vec4d(1.0, 1.0, 1.0, 1.0);
            screenXYZ *= 0.5;
            
            //if the lens flare is not on the screen dont draw it
            if(inFrontOfCamera)// && screenXYZ.x() >= 0.0 && screenXYZ.x() <= 1.0
               //&& screenXYZ.y() >= 0.0 && screenXYZ.y() <= 1.0)
            {
             
               osg::Vec2 screenPos(screenXYZ.x(), screenXYZ.y());

               osg::Vec4 glowColor(0.60f, 0.60f, 0.8f, 1.0f);
               osg::Vec4 hardGlowColor(1.0f, 1.0f, 1.0f, 1.0f);

               EnableTextureState(renderInfo);

               mHardGlow->apply(*renderInfo.getState());
               RenderQuad(glowColor, screenPos, hardGlowScale);

               mSoftGlow->apply(*renderInfo.getState());
               RenderQuad(glowColor, screenPos, softGlowScale);

               mStreaks->apply(*renderInfo.getState());
               RenderQuad(glowColor, screenPos, streaksScale);
            }
         }
      }

      void LensFlareDrawable::LensFlareOSGDrawable::RenderQuad(const osg::Vec4& rgba, const osg::Vec2& point, float scale) const
      {
         osg::Vec2 q[4];

         q[0].x() = (point[0] - scale);
         q[0].y() = (point[1] - scale);

         q[1].x() = (point[0] - scale);
         q[1].y() = (point[1] + scale);

         q[2].x() = (point[0] + scale);
         q[2].y() = (point[1] - scale);

         q[3].x() = (point[0] + scale);
         q[3].y() = (point[1] + scale);

         glColor4f(rgba[0], rgba[1], rgba[2], rgba[3]);

         glBegin(GL_TRIANGLE_STRIP);
         glTexCoord2f(0.0f, 0.0f);					
         glVertex2f(q[0].x(), q[0].y());
         glTexCoord2f(0.0f, 1.0f);
         glVertex2f(q[1].x(), q[1].y());
         glTexCoord2f(1.0f, 0.0f);
         glVertex2f(q[2].x(), q[2].y());
         glTexCoord2f(1.0f, 1.0f);
         glVertex2f(q[3].x(), q[3].y());
         glEnd();					
      }

   }
}
