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


#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/LensFlareDrawable.h>

#include <dtUtil/exception.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/matrixutil.h>
#include <dtDAL/project.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/camera.h>
#include <dtCore/shadermanager.h>
#include <dtABC/application.h>

#include <SimCore/CollisionGroupEnum.h>

#include <dtPhysics/raycast.h>
#include <dtPhysics/palphysicsworld.h>

#include <SimCore/Components/RenderingSupportComponent.h>

#include <osg/Group>
#include <osg/State>
#include <osg/StateSet>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Projection>

#include <osgDB/ReadFile>
#include <iostream>

namespace SimCore
{
   namespace Actors
   {

      ////////////////////////////////////////////////////////////////////////////////////////
      LensFlareDrawable::LensFlareUpdateCallback::LensFlareUpdateCallback(LensFlareOSGDrawable* lensFlareReference, const dtCore::Camera& camera)
         : mAttach(false)
         , mLensFlare(lensFlareReference)
      {
         osg::Camera* cam = const_cast<osg::Camera*>(camera.GetOSGCamera());
         if(cam != NULL)
         { 
            if(cam->getRenderTargetImplementation() == osg::Camera::FRAME_BUFFER_OBJECT)
            {
               mAttach = true;
            }

            LensFlareOSGDrawable::FadeParams& params = mLensFlare->mFadeMap[cam];

            int width = cam->getViewport()->width();
            int height = cam->getViewport()->height();

            params.mDepthTexture = new osg::Texture2D;
            params.mDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT);

            params.mDepthTexture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
            params.mDepthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
            params.mDepthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

            if(mAttach)
            {
               params.mDepthTexture->setTextureSize(width, height);
               cam->attach(osg::Camera::DEPTH_BUFFER, params.mDepthTexture);               
            }
            else
            {
               params.mDepthTexture->setTextureSize(16, 16);
            }

            osg::Uniform* texUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "lastDepthTexture");
            texUniform->set(3);
            osg::StateSet* ss = lensFlareReference->getOrCreateStateSet();
            ss->addUniform(texUniform);
            ss->setTextureAttributeAndModes(3, params.mDepthTexture .get(), osg::StateAttribute::ON);

            params.mDepthTexCoordUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "depthTexCoords");
            cam->getOrCreateStateSet()->addUniform(params.mDepthTexCoordUniform);
        
         }
         else
         {
            LOG_ERROR("Invalid camera passed to LensFlareDrawable::LensFlareUpdateCallback constructor");
         }

      }

      ////////////////////////////////////////////////////////////////////////////////////////      
      void LensFlareDrawable::LensFlareUpdateCallback::operator () (const dtCore::Camera& camera, osg::RenderInfo& renderInfo) const
      {
         osg::Camera* cam = const_cast<osg::Camera*>(camera.GetOSGCamera());
         if(cam != NULL)
         {                        
            LensFlareOSGDrawable::FadeParams& params = mLensFlare->mFadeMap[cam];

            //compute position of sun
            osg::Vec4d screenXYZ(mLensFlare->mLightPos.x(), mLensFlare->mLightPos.y(), mLensFlare->mLightPos.z(), 1.0);
            screenXYZ = cam->getViewMatrix().preMult(screenXYZ);
            screenXYZ = cam->getProjectionMatrix().preMult(screenXYZ);
            screenXYZ /= screenXYZ.w();
            screenXYZ += osg::Vec4d(1.0, 1.0, 1.0, 1.0);
            screenXYZ *= 0.5;

            //float depthValue = 100.0f;
            //bool occluded = true;

            //set depth texture
            int width = cam->getViewport()->width();
            int height = cam->getViewport()->height();

            int startX = int(screenXYZ.x() * width) - 8;
            int startY = int(screenXYZ.y() * height) - 8;

            bool inFrontOfCamera = (screenXYZ.w() > 0.0f) && (screenXYZ.z() > 0.0f) && (startX >= 0 && startX <= width) && (startY >= 0 && startY <= height);

            params.mVisible = inFrontOfCamera;

            if(inFrontOfCamera)
            {
               if(!mAttach)
               {
                  params.mDepthTexture->copyTexImage2D(*renderInfo.getState(), startX, startY, 16, 16);
                  params.mDepthTexCoords.set(0.5f, 0.5f);
                  params.mDepthTexCoordUniform->set(params.mDepthTexCoords);

               }
               else
               {
                  params.mDepthTexCoords.set(screenXYZ.x(), screenXYZ.y());
                  params.mDepthTexCoordUniform->set(params.mDepthTexCoords);
               }

            }

            //mLensFlare->CalculateEffectScale(cam, inFrontOfCamera && !occluded);

         }
         
      }


      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString LensFlareDrawable::TEXTURE_LENSFLARE_SOFT_GLOW("Textures:sun_glare_soft_glow.bmp");
      const dtUtil::RefString LensFlareDrawable::TEXTURE_LENSFLARE_HARD_GLOW("Textures:sun_glare_hard_glow.bmp");
      const dtUtil::RefString LensFlareDrawable::TEXTURE_LENSFLARE_STREAKS("Textures:sun_glare_streaks_01.PNG");


      //////////////////////////////////////////////////////////////////////////
      LensFlareDrawable::LensFlareDrawable()
         : dtCore::DeltaDrawable("LensFlareDrawable")
         , mTraversalMask(0xFFFFFFFF)
         , mLensFlareDrawable(new LensFlareOSGDrawable())
      {
         AddSender(&dtCore::System::GetInstance());
      }

      //////////////////////////////////////////////////////////////////////////
      LensFlareDrawable::~LensFlareDrawable()
      {
         RemoveSender(&dtCore::System::GetInstance());
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Node* SimCore::Actors::LensFlareDrawable::GetOSGNode()
      {
         return mNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Node* SimCore::Actors::LensFlareDrawable::GetOSGNode() const
      { 
         return mNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::Init(dtGame::GameManager& gm)
      {
         osg::Geode* geode = new osg::Geode();

         geode->addDrawable(mLensFlareDrawable.get());
         geode->setCullingActive(false);

         osg::StateSet* ss = geode->getOrCreateStateSet();
         ss->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD - 1, "RenderBin" );
         ss->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
         ss->setMode( GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );

         osg::BlendFunc* bf = new osg::BlendFunc(GL_ONE, GL_ONE);
         ss->setMode(GL_BLEND, osg::StateAttribute::ON);
         ss->setAttributeAndModes(bf, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
         ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

         mNode = new osg::MatrixTransform();
         mNode->addChild(geode);

		 mNode->setNodeMask(SimCore::Components::RenderingSupportComponent::DISABLE_SHADOW_NODE_MASK);

         //add a camera callback for updates
         dtCore::Camera::AddCameraSyncCallback(*this,
            dtCore::Camera::CameraSyncCallback(this, &LensFlareDrawable::UpdateView));

         //create batch isector
         mIsector = new dtCore::BatchIsector(&gm.GetScene());

         //SetShader
         dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();         
         dtCore::ShaderProgram* sp = sm.FindShaderPrototype("LensFlareShader", "LensFlareGroup");
         if(sp != NULL)
         {
            sm.AssignShaderFromPrototype(*sp, *mNode);
         }
         else
         {
            LOG_ERROR("Unable to find shader for LensFlareDrawable, with shader group 'LensFlareGroup', shader name 'LensFlareShader'." );
         }
      }
      
      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::OnMessage(dtCore::Base::MessageData* data)
      {
         /*if (data->message == dtCore::System::MESSAGE_POST_EVENT_TRAVERSAL)
         {
            if(mLensFlareDrawable->mUseRayCast)
            {
               if(mLensFlareDrawable->mUsePhysicsRaycast)
               {

                  RayCastArray::iterator iter = mRayCastArray.begin();
                  RayCastArray::iterator iterEnd = mRayCastArray.end();

                  for(;iter != iterEnd; ++iter)
                  {

                     RayCastCameraPair& rccp = *iter;

                     if(rccp.second.valid())
                     {
                        LensFlareOSGDrawable::FadeParams& params = mLensFlareDrawable->mFadeMap[rccp.second.get()];

                        dtPhysics::RayCast::Report report;
                        dtPhysics::PhysicsWorld::GetInstance().TraceRay(rccp.first, report);
                        params.mRayCastVisible = !report.mHasHitObject;
                     }
                  }

                  mRayCastArray.clear();
               }
               else
               {
                  if(mTerrainNode.valid())
                  {
                     mIsector->SetQueryRoot(mTerrainNode.get());
                  }
                  else
                  {
                     mIsector->ClearQueryRoot();
                  }

                  int rayCount = 0;
                  RayCastArray::iterator iter = mRayCastArray.begin();
                  RayCastArray::iterator iterEnd = mRayCastArray.end();

                  for(;iter != iterEnd; ++iter, ++rayCount)
                  {
                     RayCastCameraPair& rccp = *iter;

                     if(rccp.second.valid())
                     {
                        dtCore::BatchIsector::SingleISector& singleISector = mIsector->EnableAndGetISector(rayCount);
                        singleISector.SetSectorAsRay(osg::Vec3(rccp.first.GetOrigin()), osg::Vec3(rccp.first.GetDirection()), 1000000.0f);
                     }
                  }
   
                  mIsector->Update();

                  rayCount = 0;
                  iter = mRayCastArray.begin();
                  iterEnd = mRayCastArray.end();

                  for(;iter != iterEnd; ++iter, ++rayCount)
                  {
                     RayCastCameraPair& rccp = *iter;

                     LensFlareOSGDrawable::FadeParams& params = mLensFlareDrawable->mFadeMap[rccp.second.get()];
                     
                     unsigned numHits = mIsector->GetSingleISector(rayCount).GetNumberOfHits();
                     bool rayVisible = numHits == 0;                    
                     params.mRayCastVisible = rayVisible;
                  }
               
                  mRayCastArray.clear();
                  mIsector->StopUsingAllISectors();
               }
            }
         }*/
      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::UpdateView(dtCore::Camera& pCamera)
      {
         if(!HasCamera(&pCamera))
         {     
            dtCore::RefPtr<LensFlareUpdateCallback> lensCallback = new LensFlareUpdateCallback(mLensFlareDrawable.get(), pCamera);
            pCamera.AddPostDrawCallback(*lensCallback);

            mCurrentCameras.push_back(std::make_pair(lensCallback, &pCamera));
         }

         /*if(mLensFlareDrawable->mUseRayCast)
         {
            osg::Camera* cam = pCamera.GetOSGCamera();
            if(cam != NULL)
            {
               dtCore::Transform t;
               osg::Vec3d pos, dir;

               pCamera.GetTransform(t);
               t.GetTranslation(pos);

               dir = mLensFlareDrawable->mLightPos - pos;
               float length = dir.normalize();
               if(length > 0.99)
               {
                  dir = dir * 100000.0;

                  dtPhysics::RayCast ray;

                  ray.SetOrigin(pos);
                  ray.SetDirection(dir);

                  //static const dtPhysics::CollisionGroupFilter GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);
                  //ray.SetCollisionGroupFilter(GROUPS_FLAGS);

                  //we have to store these for later since we cannot process them in the camera synch callback
                  //if physics is running in a background thread
                  mRayCastArray.push_back(RayCastCameraPair(ray, cam));                  
               }
            }
         }*/

         
      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::Update(const osg::Vec3& lightPos)
      {
         mLensFlareDrawable->SetLightPos(lightPos);
      }

      ////////////////////////////////////////////////////////////////////////////
      //void LensFlareDrawable::SetUseRayCast(bool useRayCast, bool usePhysicsRaycast)
      //{
      //   mLensFlareDrawable->mUseRayCast = useRayCast;
      //   mLensFlareDrawable->mUsePhysicsRaycast = usePhysicsRaycast;

      //   //prevents from getting in a bad state
      //   if(usePhysicsRaycast)
      //   {
      //      mLensFlareDrawable->mUseRayCast = true;
      //   }
      //}

      ////////////////////////////////////////////////////////////////////////////
      //void LensFlareDrawable::GetUseRayCast(bool& useRayCast, bool& usePhysicsRaycast) const
      //{
      //   useRayCast = mLensFlareDrawable->mUseRayCast;
      //   usePhysicsRaycast = mLensFlareDrawable->mUsePhysicsRaycast;
      //}

      //////////////////////////////////////////////////////////////////////////
      //void LensFlareDrawable::SetRayCastTraversalMask( int mask )
      //{
      //   mTraversalMask = mask;
      //}

      ////////////////////////////////////////////////////////////////////////////
      //int LensFlareDrawable::GetRayCastTraversalMask() const
      //{
      //   return mTraversalMask;
      //}

      ////////////////////////////////////////////////////////////////////////////
      //void LensFlareDrawable::SetTerrainNode(dtCore::DeltaDrawable* terrainRoot)
      //{
      //   mTerrainNode = terrainRoot;
      //}

      ////////////////////////////////////////////////////////////////////////////
      //dtCore::DeltaDrawable* LensFlareDrawable::GetTerrainNode()
      //{
      //   return mTerrainNode.get();
      //}

      //////////////////////////////////////////////////////////////////////////
      bool LensFlareDrawable::HasCamera(dtCore::Camera* cam) const
      {
         for(size_t i = 0; i < mCurrentCameras.size(); ++i)
         {
            if(mCurrentCameras[i].second.get() == cam)
            {
               return true;
            }
         }

         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      LensFlareDrawable::LensFlareOSGDrawable::LensFlareOSGDrawable()
         : mUseRayCast(false)
         , mUsePhysicsRaycast(false)
      {
         setUseDisplayList(false);
         Init();
      }

      //////////////////////////////////////////////////////////////////////////
      LensFlareDrawable::LensFlareOSGDrawable::LensFlareOSGDrawable( const LensFlareOSGDrawable& bd, const osg::CopyOp& copyop/*=osg::CopyOp::SHALLOW_COPY*/ )
         : mUseRayCast(false)
         , mUsePhysicsRaycast(false)
      {
         setUseDisplayList(false);
         Init();
      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::LensFlareOSGDrawable::Init()
      {
         unsigned numParticles = 1;
         float w = 1.0f;
         float h = 1.0f;

         // set up the coords
         osg::Vec4Array& v = *(new osg::Vec4Array(4 * numParticles));
         osg::Vec2Array& t = *(new osg::Vec2Array(4 * numParticles));


         for(unsigned i = 0; i < numParticles; ++i)
         {
            v[(i * 4) + 0].set(-w*1.0f,-h*1.0f, 0.0f, i);
            v[(i * 4) + 1].set( w*1.0f,-h*1.0f, 0.0f, i);
            v[(i * 4) + 2].set( w*1.0f,h*1.0f, 0.0f, i);
            v[(i * 4) + 3].set(-w*1.0f,h*1.0f, 0.0f, i);
 
            t[(i * 4) + 0].set(0.0f,0.0f);
            t[(i * 4) + 1].set(1.0f,0.0f);
            t[(i * 4) + 2].set(1.0f,1.0f);
            t[(i * 4) + 3].set(0.0f,1.0f);
         }


         setVertexArray(&v);
         setTexCoordArray(0, &t);

         addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4 * numParticles));

         LoadTextures();         

         osg::StateSet* ss = getOrCreateStateSet();         
         mLightPosUniform = ss->getOrCreateUniform("sunPosition", osg::Uniform::DOUBLE_VEC3);
         //mEffectRadiusUniform = ss->getOrCreateUniform("effectRadius", osg::Uniform::FLOAT);

      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::LensFlareOSGDrawable::LoadTextures()
      {
         dtDAL::Project& project = dtDAL::Project::GetInstance();

         try
         {
            std::string softGlowFile = project.GetResourcePath(dtDAL::ResourceDescriptor(TEXTURE_LENSFLARE_SOFT_GLOW));
            mSoftGlow = new osg::Texture2D();
            InitTexture(softGlowFile, mSoftGlow.get(), "softGlow", 0);

            std::string hardGlowFile = project.GetResourcePath(dtDAL::ResourceDescriptor(TEXTURE_LENSFLARE_HARD_GLOW));
            mHardGlow = new osg::Texture2D();
            InitTexture(hardGlowFile, mHardGlow.get(), "hardGlow", 1);

            std::string streaksFile = project.GetResourcePath(dtDAL::ResourceDescriptor(TEXTURE_LENSFLARE_STREAKS));
            mStreaks = new osg::Texture2D();
            InitTexture(streaksFile, mStreaks.get(), "streaks", 2);


         }
         catch (dtUtil::Exception& e)
         {
            e.LogException(dtUtil::Log::LOG_ERROR);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::LensFlareOSGDrawable::InitTexture( const std::string& filename, osg::Texture2D* ptr, const std::string& uniformName, unsigned index)
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

         osg::Uniform* texUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, uniformName);
         texUniform->set(int(index));
         osg::StateSet* ss = getOrCreateStateSet();
         ss->addUniform(texUniform);
         ss->setTextureAttributeAndModes(int(index), ptr, osg::StateAttribute::ON);
      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::LensFlareOSGDrawable::EnableTextureState(osg::RenderInfo& renderInfo) const
      {
         renderInfo.getState()->setClientActiveTextureUnit(0);
         glDisable(GL_TEXTURE_2D);

         renderInfo.getState()->setActiveTextureUnit(0);
         glEnable(GL_TEXTURE_2D);
      }

      //////////////////////////////////////////////////////////////////////////
      //this function calculates a scalar to apply to the flare effect which fades it in and
      //out over the fade rate
      float LensFlareDrawable::LensFlareOSGDrawable::CalculateEffectScale(osg::Camera* cam, bool visible) const
      {
         const double currentTime = dtCore::System::GetInstance().GetSimTimeSinceStartup();

         //this will insert one if it doesnt exist
         FadeParams& params = mFadeMap[cam];

         if(params.mVisible != visible)
         {
            //if visible is false, fade direction will be -1, implying we are fading out
            //else fade direction will be 1, implying we are fading in
            params.mFadeDirection = (2 * int(visible)) - 1;
            params.mVisible = visible;
         }
         else if(params.mFadeDirection != 0)
         {
            float fadeDelta = (currentTime - params.mLastTickTime) / params.mFadeRate;
            params.mFadeCurrent += float(params.mFadeDirection) * fadeDelta;

            //if we are fading down and we hit zero or up and we hit one then we are no longer fading
            if((params.mFadeCurrent <= 0.0f && params.mFadeDirection == -1) || (params.mFadeCurrent >= 1.0 && params.mFadeDirection == 1))
            {
               params.mFadeDirection = 0;
            }

            dtUtil::Clamp(params.mFadeCurrent, 0.0f, 1.0f);
         }

         params.mLastTickTime = currentTime;
         return params.mFadeCurrent;
      }

      //////////////////////////////////////////////////////////////////////////
      void LensFlareDrawable::LensFlareOSGDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
      {
         if(renderInfo.getState() != NULL)
         {
            //calculate the screen position of the light
            osg::Camera* cam = renderInfo.getCurrentCamera();

            FadeParams& params = mFadeMap[cam];
            if(params.mVisible)
            {

               if(params.mDepthTexture.valid())
               {
                  params.mDepthTexture->apply(*renderInfo.getState());
               }

               //queue base class drawing behavior  
               osg::Geometry::drawImplementation(renderInfo);
            }
         }
      }

      void LensFlareDrawable::LensFlareOSGDrawable::SetLightPos(const osg::Vec3d& pos)
      {
         mLightPos = pos;

         mLightPosUniform->set(mLightPos);
         mLightPosUniform->dirty();

      }

   }
}
