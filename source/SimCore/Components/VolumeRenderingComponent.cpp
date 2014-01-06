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


#include <SimCore/Components/VolumeRenderingComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtCore/transform.h>
#include <dtCore/scene.h>
#include <dtCore/shadermanager.h>

#include <dtUtil/mathdefines.h>
#include <dtUtil/noisetexture.h>

#include <dtABC/application.h>
//#include <dtCore/deltawin.h>

#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>

#include <SimCore/Actors/SimpleMovingShapeActor.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Components/RenderingSupportComponent.h>

#include <osg/Group>
#include <osg/Geode>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/RenderInfo>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/Uniform>
#include <osg/Billboard>
#include <osg/Depth>
#include <math.h>
#include <iostream>
#define SQRT2PI 2.506628274631000502415765284811045253006
#define ONEOVERSQRT2PI (1.0 / SQRT2PI)


// Return a random number with a normal distribution.
static inline float NRand(float sigma = 1.0f)
{
#define ONE_OVER_SIGMA_EXP (1.0f / 0.7975f)

   if(sigma == 0) return 0;

   float y;
   do
   {
      y = -logf(dtUtil::RandPercent());
   }
   while(dtUtil::RandPercent() > expf(-((y - 1.0f)*(y - 1.0f))*0.5f));

   if(rand() & 0x1)
      return y * sigma * ONE_OVER_SIGMA_EXP;
   else
      return -y * sigma * ONE_OVER_SIGMA_EXP;
}

/*
static osg::Node* CreateQuad( osg::Texture2D* tex, int renderBin )
{
   osg::Geometry* geo = new osg::Geometry;
   geo->setUseDisplayList( false );
   osg::Vec4Array* colors = new osg::Vec4Array;
   colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
   geo->setColorArray(colors);
   geo->setColorBinding(osg::Geometry::BIND_OVERALL);
   osg::Vec3Array *vx = new osg::Vec3Array;
   vx->push_back(osg::Vec3(-10, -10, 0));
   vx->push_back(osg::Vec3(10, -10, 0));
   vx->push_back(osg::Vec3(10, 10, 0 ));
   vx->push_back(osg::Vec3(-10, 10, 0));
   geo->setVertexArray(vx);
   osg::Vec3Array *nx = new osg::Vec3Array;
   nx->push_back(osg::Vec3(0, 0, 1));
   geo->setNormalArray(nx);
   if (tex != nullptr)
   {
      osg::Vec2Array *tx = new osg::Vec2Array;
      tx->push_back(osg::Vec2(0, 0));
      tx->push_back(osg::Vec2(1, 0));
      tx->push_back(osg::Vec2(1, 1));
      tx->push_back(osg::Vec2(0, 1));
      geo->setTexCoordArray(0, tx);
      geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
   }

   geo->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
   osg::Geode *geode = new osg::Geode;
   geode->addDrawable(geo);
   geode->setCullingActive(false);
   osg::StateSet* ss = geode->getOrCreateStateSet();
   ss->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
   ss->setMode( GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
   ss->setRenderBinDetails( renderBin, "RenderBin" );
   return geode;
}*/

static osg::Texture2D* CreateDepthTexture(int width, int height)
{
   osg::Texture2D* tex = new osg::Texture2D();
   tex->setTextureSize(width, height);
   tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
   tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
   tex->setSourceFormat(GL_DEPTH_COMPONENT);
   tex->setSourceType(GL_FLOAT);
   tex->setInternalFormat(GL_DEPTH_COMPONENT32);
   tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
   tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
   return tex;
}


namespace SimCore
{
   namespace Components
   {

#ifdef __APPLE__
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_POS_UNIFORM = "volumeParticlePos[0]";
#else
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_POS_UNIFORM = "volumeParticlePos";
#endif
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_COLOR_UNIFORM = "volumeParticleColor";
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_INTENSITY_UNIFORM = "volumeParticleIntensity";
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_DENSITY_UNIFORM = "volumeParticleDensity";
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_VELOCITY_UNIFORM = "volumeParticleVelocity";
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_RADIUS_UNIFORM = "volumeParticleRadius";
   const std::string VolumeRenderingComponent::VOLUME_PARTICLE_NOISE_SCALE_UNIFORM = "volumeParticleNoiseScale";
   //const std::string VolumeRenderingComponent::CAMERA_LINEAR_DEPTH_UNIFORM = "writeLinearDepth";

   const std::string VolumeRenderingComponent::DEFAULT_NAME = "VolumeRenderingComponent";


   //////////////////////////////////////////////////////////////////////////
   //Draw Callbacks
   //////////////////////////////////////////////////////////////////////////
   class VRC_DrawCallback : public osg::Camera::DrawCallback
   {
   public:

      enum Phase{ PRE_DRAW, POST_DRAW};

      VRC_DrawCallback(osg::Camera* sceneCam, osg::Camera* depthCam, osg::Node& n, Phase p)
         : mSceneCamera(sceneCam)
         , mDepthCamera(depthCam)
         , mNode(&n)
         , mPhase(p)
      {

      }

      ~VRC_DrawCallback(){}


      virtual void operator () (const osg::Camera& /*camera*/) const
      {
         if(mNode.valid())
         {
            osg::StateSet* sceneStateSet = mNode->getOrCreateStateSet();
            osg::Uniform* sceneDepthUniform = sceneStateSet->getOrCreateUniform("writeLinearDepth", osg::Uniform::BOOL);

            if(mPhase == PRE_DRAW)
            {
               if(mSceneCamera.valid() && mDepthCamera.valid())
               {
                  //the view matrix is inherited but not the projection, so we set it here every frame
                  mDepthCamera->setProjectionMatrix(mSceneCamera->getProjectionMatrix());
               }

               sceneDepthUniform->set(true);
            }
            else
            {
               sceneDepthUniform->set(false);
            }
         }
      }

   private:

      osg::observer_ptr<osg::Camera> mSceneCamera;
      osg::observer_ptr<osg::Camera> mDepthCamera;
      osg::observer_ptr<osg::Node> mNode;
      Phase mPhase;

   };

   /////////////////////////////////////////////////////////////
   //useful functors
   /////////////////////////////////////////////////////////////
   struct findVolumeById
   {
      findVolumeById(VolumeRenderingComponent::ShapeRecordId id): mId(id){}

      template<class T>
      bool operator()(T ptr)
      {
         return ptr->GetId() == mId;
      }
   private:

      VolumeRenderingComponent::ShapeRecordId mId;
   };

   struct removeVolumesFunc
   {
      template<class T>
      bool operator()(T ptr)
      {
         return ptr->mDeleteMe;
      }
   };

   /////////////////////////////////////////////////////////////
   //ShapeVolumeRecord
   /////////////////////////////////////////////////////////////
   OpenThreads::Atomic VolumeRenderingComponent::ShapeVolumeRecord::mCounter;
   
   //////////////////////////////////////////////////////////////////////////
   VolumeRenderingComponent::ShapeVolumeRecord::ShapeVolumeRecord()
      : mShapeType(VolumeRenderingComponent::SPHERE)
      , mRenderMode(PARTICLE_VOLUME)
      , mDirtyParams(false)
      , mDeleteMe(false)
      , mAutoDeleteOnTargetNull(false)
      , mAutoDeleteAfterMaxTime(false)
      , mMaxTime(0.0f)
      , mFadeOut(false)
      , mFadeOutTime(0.0f)
      , mIntensity(1.0f)
      , mDensity(0.15f)
      , mVelocity(0.15f)
      , mNoiseScale(0.85f)
      , mNumParticles(50)
      , mParticleRadius(5.0f)
      , mColor(1.0f, 1.0f, 1.0f, 1.0f)
      , mPosition(0.0f, 0.0f, 0.0f)
      , mRadius(1.0f, 0.0f, 0.0f)
      , mTransform()
      , mShaderGroup("VolumeRenderingGroup")
      , mShaderName("ParticleVolumeShader")
      , mParentNode(nullptr)
      , mTarget(nullptr)
      , mShape(nullptr)
      , mParticleDrawable(nullptr)
      , mId(mCounter)
   {
      ++mCounter;
      mTransform.makeIdentity();

   }

   /////////////////////////////////////////////////////////////
   VolumeRenderingComponent::ShapeRecordId VolumeRenderingComponent::ShapeVolumeRecord::GetId() const
   {
      return mId;
   }

   /////////////////////////////////////////////////////////////
   VolumeRenderingComponent::VolumeRenderingComponent(const std::string& name)
   : BaseClass(name) 
   , mInitialized(false)
   , mMaxParticlesPerDrawable(150)
   , mRootNode(new osg::Group())
   , mVolumes()
   {

   }

   /////////////////////////////////////////////////////////////
   VolumeRenderingComponent::~VolumeRenderingComponent()
   {

   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::CleanUp()
   {
      mVolumes.clear();
      
      unsigned numChildren = mRootNode->getNumChildren();
      if(numChildren > 0)
      {
         mRootNode->removeChildren(0, numChildren);
      }
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();

   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::OnRemovedFromGM()
   {
      CleanUp();
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::Init()
   {
      double vfov, aspect, nearp, farp;
      GetGameManager()->GetApplication().GetCamera()->GetPerspectiveParams(vfov, aspect, nearp, farp);

      CreateDepthPrePass(1024 * aspect, 1024);

      //CreateDepthPrePass("sceneDepth", 16, 16);
      GetGameManager()->GetScene().GetSceneNode()->addChild(mRootNode.get());

      mRootNode->setNodeMask(RenderingSupportComponent::MAIN_CAMERA_ONLY_FEATURE_NODE_MASK);

      CreateNoiseTexture();

      mInitialized = true;
   }

   
   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RemoveShapeVolume(ShapeVolumeRecord* svr)
   {
      if(svr != nullptr)
      {
         RemoveDrawable(*svr);
         mVolumes.erase(std::remove_if(mVolumes.begin(), mVolumes.end(), findVolumeById(svr->GetId())), mVolumes.end());
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeRecordId VolumeRenderingComponent::CreateShapeVolume(std::shared_ptr<ShapeVolumeRecord> svr)
   {
      if (svr != nullptr)
      {
         //do lazy initialization   
         if(!mInitialized)
         {
            Init();
         }

         mVolumes.push_back(svr);
         CreateDrawable(*svr);
         return svr->GetId();
      }
      else
      {
         LOG_ERROR("Attempting to add a ShapeVolumeRecord that is nullptr to the Volume Rendering Component");
         return 0;
      }
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeRecordId VolumeRenderingComponent::CreateStaticShapeVolume(Shape s, const osg::Vec4& color, const osg::Vec3& center, const osg::Vec3& radius)
   {
      //do lazy initialization   
      if(!mInitialized)
      {
         Init();
      }

      std::shared_ptr<ShapeVolumeRecord> svr = new ShapeVolumeRecord();
      svr->mShapeType = s;
      svr->mColor = color;
      svr->mPosition = center;
      svr->mRadius = radius;
      return CreateShapeVolume(svr);
   }

   ////////////////////////////////////////////////////////////////////////// 
   /*void VolumeRenderingComponent::FindAllShapeVolumesForActor( const dtCore::UniqueId& actorID, std::vector<ShapeVolumeRecord*> pContainerToFill )
   {

   }*/

   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::ProcessMessage(const dtGame::Message& message)
   {
      if(mInitialized)
      {  
         if(message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            float dt = float(static_cast<const dtGame::TickMessage&>(message).GetDeltaSimTime());
            Tick(dt);
         }
         else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
         {
            CleanUp();
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::Tick(float dt)
   {
      UpdateVolumes(dt);
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::UpdateVolumes(float dt)
   {
      TimeoutAndDeleteVolumes(dt);
      TransformAndSortVolumes();
      UpdateUniforms();
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::TimeoutAndDeleteVolumes(float dt)
   {
      ShapeVolumeArray::iterator iter = mVolumes.begin();
      ShapeVolumeArray::iterator endIter = mVolumes.end();

      for(;iter != endIter; ++iter)
      {
         ShapeVolumeRecord* svr = (*iter).get();

         if((svr->mAutoDeleteOnTargetNull && !svr->mTarget.valid()) || (!svr->mAutoDeleteAfterMaxTime && svr->mAutoDeleteOnTargetNull))
         {
            if(!svr->mTarget.valid())
            {
               if(svr->mFadeOut)
               {
                  //by setting this to false we will continue into a fade out
                  svr->mAutoDeleteOnTargetNull = false;
                  //by setting this one false we ensure we will begin fading out next frame
                  svr->mAutoDeleteAfterMaxTime = false;
               }
               else
               {
                  RemoveDrawable(*svr);
                  svr->mDeleteMe = true;
                  //std::cout << "Auto delete on nullptr Ptr" << std::endl;
                  continue;
               }
            }
         }
         else if(svr->mAutoDeleteAfterMaxTime)
         {
            svr->mMaxTime -= dt;

            if(svr->mMaxTime <= 0.0f)
            {
               if(svr->mFadeOut)
               {
                  //by setting this to false we will continue into a fade out
                  svr->mAutoDeleteAfterMaxTime = false;
                  //by setting this one false we ensure we will begin fading out next frame
                  svr->mAutoDeleteOnTargetNull = false;
               }
               else
               {
                  RemoveDrawable(*svr);
                  svr->mDeleteMe = true;
                  //std::cout << "Auto delete on Max Time" << std::ensvr;
                  continue;
               }
            }
         }
         else if(svr->mFadeOut)
         {
            svr->mIntensity -= (dt / svr->mFadeOutTime);
            svr->mDirtyParams = true;
            if(svr->mIntensity <= 0.0f)
            {
               RemoveDrawable(*svr);
               svr->mDeleteMe = true;
               //std::cout << "Auto delete on fade out" << std::ensvr;
               continue;
            }
         }
      }

      //now remove all flagged Volumes, note this is actually faster because we only have a single deallocation for N Volumes
      mVolumes.erase(std::remove_if(mVolumes.begin(), mVolumes.end(), removeVolumesFunc()), mVolumes.end());

   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::TransformAndSortVolumes()
   {
      ShapeVolumeArray::iterator iter = mVolumes.begin();
      ShapeVolumeArray::iterator endIter = mVolumes.end();

      for(;iter != endIter; ++iter)
      {
         ShapeVolumeRecord* svr = (*iter).get();
         if(svr->mTarget.valid())
         {
            //update the Volume's position
            SetPosition(svr);
         }
      }

      ////update uniforms by finding the closest Volumes to the camera
      //dtCore::Transform trans;
      //GetGameManager()->GetApplication().GetCamera()->GetTransform(trans);
      //osg::Vec3 pos;
      //trans.GetTranslation(pos);
      ////sort the Volumes, though a heap may be more efficient here, we will sort so that we can combine Volumes later
      //std::sort(mVolumes.begin(), mVolumes.end(), funcCompareVolumes(pos));
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::SetPosition(ShapeVolumeRecord* svr)
   {
      if(svr != nullptr && svr->mTarget.valid())
      {
         /*dtCore::Transform xform;
         svr->mTarget->GetTransform(xform);
         xform.Move(svr->mPosition);*/

         osg::NodePathList nodePathList = svr->mTarget->getParentalNodePaths();

         if(!nodePathList.empty())
         {
            osg::Matrix mat = osg::computeLocalToWorld(nodePathList[0]);
            mat.translate(svr->mPosition);
            svr->mParentNode->setMatrix(mat);
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::MoveVolume(ShapeVolumeRecord& svr, const osg::Vec3& moveBy)
   {
      dtCore::Transform xform;
      osg::Matrix mat = svr.mParentNode->getMatrix();
      xform.Get(mat);
      xform.Move(moveBy);

      svr.mParentNode->setMatrix(mat);
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ExpandVolume(ShapeVolumeRecord& svr, const osg::Vec3& expandBy)
   {
      if(svr.mParticleDrawable.valid())
      {
         osg::StateSet* ss = svr.mParticleDrawable->getOrCreateStateSet();
         osg::Uniform* particleArray = ss->getOrCreateUniform(VOLUME_PARTICLE_POS_UNIFORM, osg::Uniform::FLOAT_VEC4, svr.mParticleDrawable->GetNumParticles());

         switch(svr.mShapeType)
         {
         case BOX:
         case ELLIPSOID:
            {
               osg::Vec3 oldRadius = svr.mRadius;
               osg::Vec3 newRadius = oldRadius + expandBy;

               for(unsigned i = 0; i < svr.mParticleDrawable->GetNumParticles(); ++i)
               {
                  //move the offset
                  osg::Vec4 pos4 = svr.mParticleDrawable->GetPointLocation(i);
                  osg::Vec3 pos(pos4[0], pos4[1], pos4[2]);
                  osg::Vec3 vecFromCenter = (pos - svr.mPosition);
                  osg::Vec3 percentFromRadius(vecFromCenter[0] / oldRadius[0], vecFromCenter[1] / oldRadius[1], vecFromCenter[2] / oldRadius[2]);
                  osg::Vec3 newOffset(percentFromRadius[0] * newRadius[0], percentFromRadius[1] * newRadius[1], percentFromRadius[2] * newRadius[2]);
                  pos = svr.mPosition + newOffset;

                  osg::Vec4 newPos(pos[0], pos[1], pos[2], pos4[3]);
                  svr.mParticleDrawable->SetPointLocation(i, newPos);
                  particleArray->setElement(i, newPos);
               }
               svr.mRadius = newRadius;
            }
            break;
               
         case SPHERE:
            {
               float oldRadius = svr.mRadius[0];
               float newRadius = svr.mRadius[0] + expandBy[0];
               for(unsigned i = 0; i < svr.mParticleDrawable->GetNumParticles(); ++i)
               {
                  //move the offset
                  osg::Vec4 pos4 = svr.mParticleDrawable->GetPointLocation(i);
                  osg::Vec3 pos(pos4[0], pos4[1], pos4[2]);
                  osg::Vec3 vecFromCenter = (pos - svr.mPosition);
                  float percentFromRadius = vecFromCenter.normalize() / oldRadius;
                  osg::Vec3 newOffset = vecFromCenter * (newRadius * percentFromRadius);
                  pos = svr.mPosition + newOffset;

                  osg::Vec4 newPos(pos[0], pos[1], pos[2], pos4[3]);
                  svr.mParticleDrawable->SetPointLocation(i, newPos);
                  particleArray->setElement(i, newPos);
               }
               svr.mRadius[0] = newRadius;
            }
            break;
         case CYLINDER:
            {
               LOG_WARNING("Functionality Unimplemented");
            }
            break;
         case CONE:
            {
               LOG_WARNING("Functionality Unimplemented");
            }
            break;
         default:
            break;
         }
         
         particleArray->dirty(); 
      }
      svr.mParticleDrawable->dirtyBound();
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ComputeParticleRadius(ShapeVolumeRecord& svr)
   {
      if(svr.mParticleDrawable.valid())
      {
         float particleVolume = 0.0f; 
         float multiplier = 1.0f;

         switch(svr.mShapeType)
         {
         case BOX:
         case ELLIPSOID:
            {
               float volume = svr.mRadius[0] * svr.mRadius[1] * svr.mRadius[2];
               particleVolume = volume / svr.mNumParticles;
               multiplier = 3.5f;
            }
            break;

         case SPHERE:
            {
               float volume = (3.0f / 4.0f) * osg::PI * svr.mRadius[0] * svr.mRadius[0] * svr.mRadius[0];
               particleVolume = volume / svr.mNumParticles;
               multiplier = 3.5f;
            }
            break;
         case CYLINDER:
            {
               float volume = osg::PI * svr.mRadius[0] * svr.mRadius[0] * svr.mRadius[1];
               particleVolume = volume / svr.mNumParticles;
               multiplier = 4.5f;
            }
            break;
         case CONE:
            {
               float volume = (1.0f / 3.0f) * osg::PI * svr.mRadius[0] * svr.mRadius[0] * svr.mRadius[1];
               particleVolume = volume / svr.mNumParticles;
               multiplier = 4.5f;
            }
            break;
         default:
            break;
         }

         osg::StateSet* ss = svr.mParticleDrawable->getOrCreateStateSet();
         osg::Uniform* particleRadiusUniform = ss->getOrCreateUniform(VOLUME_PARTICLE_RADIUS_UNIFORM, osg::Uniform::FLOAT);

         float particleRadius = multiplier * std::pow( (3.0f * particleVolume) / (4.0f * float(osg::PI)), 1.0f / 3.0f);
         svr.mParticleRadius = particleRadius;
         particleRadiusUniform->set(particleRadius);

         particleRadiusUniform->dirty(); 
      }
      svr.mParticleDrawable->dirtyBound();
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RemoveDrawable(ShapeVolumeRecord& svr)
   {
      mRootNode->removeChild(svr.mParentNode.get());
      svr.mParticleDrawable = nullptr;
      svr.mShape = nullptr;
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindVolume(ShapeRecordId id)
   {
      ShapeVolumeArray::iterator iter = std::find_if(mVolumes.begin(), mVolumes.end(), findVolumeById(id));
      if(iter != mVolumes.end())
      {
         return (*iter).get();
      }

      return nullptr;
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindShapeVolumeById(ShapeRecordId id)
   {
      return FindVolume(id);
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateDrawable(ShapeVolumeRecord& newShape)
   {
      switch(newShape.mRenderMode)
      {
         case SIMPLE_SHAPE_GEOMETRY:
            {
               CreateSimpleShape(newShape);
            }
            break;

         case PARTICLE_VOLUME:
            {
               CreateParticleVolume(newShape);
            }

         default:
            break;
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateSimpleShape(ShapeVolumeRecord& newShape)
   {
      osg::ref_ptr<osg::Geode> g = new osg::Geode();
      osg::ref_ptr<osg::MatrixTransform> matTrans = new osg::MatrixTransform();
      newShape.mParentNode = matTrans.get();
      newShape.mParentNode->addChild(g.get());

      osg::StateSet* ss = g->getOrCreateStateSet();
      ss->setMode(GL_BLEND, osg::StateAttribute::ON);

      osg::BlendFunc* blendFunc = new osg::BlendFunc();
      blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
      ss->setAttributeAndModes(blendFunc);
      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


      CreateShape(newShape);

      osg::ref_ptr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(newShape.mShape.get());
      shapeDrawable->setColor(newShape.mColor);
      g->addDrawable(shapeDrawable);

      mRootNode->addChild(newShape.mParentNode.get());
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateParticleVolume(ShapeVolumeRecord& newShape)
   {
       osg::ref_ptr<osg::Geode> g = new osg::Geode();

      osg::ref_ptr<osg::MatrixTransform> matTrans = new osg::MatrixTransform();
      newShape.mParentNode = matTrans.get();
      newShape.mParentNode->addChild(g.get());

      osg::StateSet* ss = g->getOrCreateStateSet();
      ss->setMode(GL_BLEND, osg::StateAttribute::ON);

      osg::Depth* depthSS = new osg::Depth();
      depthSS->setWriteMask(false);
      ss->setAttribute(depthSS);
      //ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

      osg::BlendFunc* blendFunc = new osg::BlendFunc();
      blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
      ss->setAttributeAndModes(blendFunc);
      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

      CreateShape(newShape);

      newShape.mParticleDrawable = new ParticleVolumeDrawable();
      newShape.mParticleDrawable->Init(newShape.mNumParticles, &newShape);
      g->addDrawable(newShape.mParticleDrawable.get());

      AssignParticleVolumeShader(newShape, *g);
      AssignParticleVolumeUniforms(newShape);

      mRootNode->addChild(newShape.mParentNode.get());
   
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateShape(ShapeVolumeRecord& newShape)
   {
      switch(newShape.mShapeType)
      {
      case ELLIPSOID:
      case SPHERE:
         {
            newShape.mShape = new osg::Sphere(newShape.mPosition, newShape.mRadius[0]);
         }
         break;
      case BOX:
         {
            newShape.mShape = new osg::Box(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1], newShape.mRadius[2]);
         }
         break;
      case CYLINDER:
         {
            newShape.mShape = new osg::Cylinder(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1]);
         }
         break;
      case CONE:
         {
            newShape.mShape = new osg::Cone(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1]);
         }
         break;
      default:
         break;
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::AssignParticleVolumeShader(ShapeVolumeRecord& svr, osg::Geode& g)
   {
      dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
      dtCore::ShaderProgram* sp = sm.FindShaderPrototype(svr.mShaderName, svr.mShaderGroup);
      if(sp != nullptr)
      {
         sm.AssignShaderFromPrototype(*sp, g);
      }
      else
      {
         LOG_ERROR("Unable to find shader for Particle Volume.");
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::AssignParticleVolumeUniforms(ShapeVolumeRecord& newShape)
   {
      if(newShape.mParticleDrawable.valid())
      {
         osg::StateSet* ss = newShape.mParticleDrawable->getOrCreateStateSet();

         SetUniformData(newShape);

         mRootNode->getOrCreateStateSet()->addUniform(mDepthTextureUniform.get());
         mRootNode->getOrCreateStateSet()->setTextureAttributeAndModes(0, mDepthTexture.get(), osg::StateAttribute::ON);

         mRootNode->getOrCreateStateSet()->addUniform(mNoiseTextureUniform.get());
         mRootNode->getOrCreateStateSet()->setTextureAttributeAndModes(1, mNoiseTexture.get(), osg::StateAttribute::ON);


         osg::Uniform* particleArray = ss->getOrCreateUniform(VOLUME_PARTICLE_POS_UNIFORM, osg::Uniform::FLOAT_VEC4, newShape.mParticleDrawable->GetNumParticles());
         
         for(unsigned i = 0; i < newShape.mParticleDrawable->GetNumParticles(); ++i)
         {
            osg::Vec4 pos = newShape.mParticleDrawable->GetPointLocation(i);
            particleArray->setElement(i, pos);
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::UpdateUniforms()
   {
      ShapeVolumeArray::iterator iter = mVolumes.begin();
      ShapeVolumeArray::iterator endIter = mVolumes.end();

      for(;iter != endIter; ++iter)
      {
         std::shared_ptr<ShapeVolumeRecord> svr = (*iter).get();
         if(svr.valid() && svr->mRenderMode == PARTICLE_VOLUME && svr->mDirtyParams)
         {
            SetUniformData(*svr);

            svr->mDirtyParams = false;
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::SetUniformData( ShapeVolumeRecord& s )
   {
      if(s.mParticleDrawable.valid())
      {
         osg::StateSet* ss = s.mParticleDrawable->getOrCreateStateSet();

         osg::Uniform* particleColor = ss->getOrCreateUniform(VOLUME_PARTICLE_COLOR_UNIFORM, osg::Uniform::FLOAT_VEC4);
         particleColor->set(s.mColor);

         osg::Uniform* particleIntensity = ss->getOrCreateUniform(VOLUME_PARTICLE_INTENSITY_UNIFORM, osg::Uniform::FLOAT);
         particleIntensity->set(s.mIntensity);

         osg::Uniform* particleDensity = ss->getOrCreateUniform(VOLUME_PARTICLE_DENSITY_UNIFORM, osg::Uniform::FLOAT);
         particleDensity->set(s.mDensity);

         osg::Uniform* particleVel = ss->getOrCreateUniform(VOLUME_PARTICLE_VELOCITY_UNIFORM, osg::Uniform::FLOAT);
         particleVel->set(s.mVelocity);

         osg::Uniform* particleNoiseScale = ss->getOrCreateUniform(VOLUME_PARTICLE_NOISE_SCALE_UNIFORM, osg::Uniform::FLOAT);
         particleNoiseScale->set(s.mNoiseScale);

         s.mParticleDrawable->SetParticleRadius(s.mParticleRadius);
         osg::Uniform* particleRadius = ss->getOrCreateUniform(VOLUME_PARTICLE_RADIUS_UNIFORM, osg::Uniform::FLOAT);
         particleRadius->set(s.mParticleRadius);
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateNoiseTexture()
   {
      LOG_INFO("Creating noise texture for VolumeRenderingComponent.");
      dtUtil::NoiseTexture noise3d(6, 2, 0.7, 0.5, 64, 64, 128);
      osg::ref_ptr<osg::Image> img = noise3d.MakeNoiseTexture(GL_ALPHA);
      LOG_INFO("Finished creating noise texture for VolumeRenderingComponent.");

      //dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();

      mNoiseTexture = new osg::Texture3D();
      mNoiseTexture->setImage(img.get());

      mNoiseTexture->setFilter( osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR );
      mNoiseTexture->setFilter( osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR );
      mNoiseTexture->setWrap( osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT );
      mNoiseTexture->setWrap( osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT );
      mNoiseTexture->setWrap( osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT );

      mNoiseTextureUniform = new osg::Uniform( osg::Uniform::SAMPLER_3D, "noiseTexture" );
      mNoiseTextureUniform->set(1);
    }
   

   ////////////////////////////////////////////////////////////////////////// 
   //ParticleVolumeDrawable
   VolumeRenderingComponent::ParticleVolumeDrawable::ParticleVolumeDrawable()
      : mParticleRadius(1.0f)
      , mNumParticles(0)
   {
      setUseDisplayList(false);
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ParticleVolumeDrawable::ParticleVolumeDrawable(const ParticleVolumeDrawable& bd, const osg::CopyOp& copyop)
   {
      setUseDisplayList(false);
      mNumParticles = bd.mNumParticles;
   }

   ////////////////////////////////////////////////////////////////////////// 
   unsigned VolumeRenderingComponent::ParticleVolumeDrawable::GetNumParticles() const
   {
      return mNumParticles;
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ParticleVolumeDrawable::Init(unsigned numPoints, ShapeVolumeRecord* svr)
   {
      mParticleRadius = svr->mParticleRadius;
      mNumParticles = numPoints;

      CreateBillboards(mNumParticles, 1.0f, 1.0f);

      CreateRandomPointsInVolume(svr->mShapeType, mNumParticles, svr->mPosition, svr->mRadius, mPoints);
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ParticleVolumeDrawable::CreateBillboards(unsigned numParticles, float w, float h)
   {
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
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ParticleVolumeDrawable::CreateRandomPointsInVolume(VolumeRenderingComponent::Shape s, float numPoints, const osg::Vec3& center, const osg::Vec3& radius, PointList& pointArrayToFill)
   {
      pointArrayToFill.reserve(numPoints);
 
      osg::Vec3 p1, p2;// Box vertices, Sphere center, Cylinder/Cone e`nds
      float radius1 = 0.0f;
      float radius2 = 0.0f;
      //float radius3 = 0.0f;
      float radius1Sqr = 0.0f;
      float radius2Sqr = 0.0f;
      //float radius3Sqr = 0.0f;


      switch(s)
      {
      case VolumeRenderingComponent::SPHERE:
         {
            p1.set(center);
            radius1 = radius[0];
            radius2 = radius[1];
            radius1Sqr = radius1 * radius1;
            radius2Sqr = radius2 * radius2;

            for(unsigned i = 0; i < numPoints; ++i)
            {
               // Place on [-1..1] sphere
               osg::Vec3 randVec(dtUtil::RandPercent(), dtUtil::RandPercent(), dtUtil::RandPercent());
               osg::Vec3 vHalf(0.5f, 0.5f, 0.5f);
               osg::Vec3 pos = randVec - vHalf;
               pos.normalize();

               // Scale unit sphere pos by [0..r] and translate
               // (should distribute as r^2 law)
               if(radius1 == radius2)
               {
                  //pos = p1 + pos * radius1;
                  pos = p1 + pos * radius1;
               }
               else
               {
                  //pos = p1 + pos * (radius2 + dtUtil::RandPercent() * (radius1 - radius2));
                  pos = p1 + pos * (radius2 + dtUtil::RandPercent() * (radius1 - radius2));
               }

               osg::Vec4 pos4(pos[0], pos[1], pos[2], NRand());
               mPoints.push_back(pos4);
            }
         }
         break;
      case VolumeRenderingComponent::ELLIPSOID:
         {
            //todo- need better equation here
            p1.set(center);

            for(unsigned i = 0; i < numPoints; ++i)
            {
               osg::Vec3 pos(2.0f * radius[0] * (0.5f - dtUtil::RandPercent()), 2.0f * radius[1] * (0.5f - dtUtil::RandPercent()), 2.0f * radius[2] * (0.5f - dtUtil::RandPercent()));

               pos += p1;
               osg::Vec4 pos4(pos[0], pos[1], pos[2], NRand());
               mPoints.push_back(pos4);
            }
         }
         break;
      case VolumeRenderingComponent::BOX:
         { 
            p1.set(center); 
            p2.set(center); 

            osg::Vec3 halfWidth = radius * 0.5f;

            p1 -= halfWidth;
            p2 += halfWidth;

            for(unsigned i = 0; i < numPoints; ++i)
            {
               osg::Vec3 pos;

               pos[0] = p1[0] + (p2[0]-p1[0]) * dtUtil::RandPercent();
               pos[1] = p1[1] + (p2[1]-p1[1]) * dtUtil::RandPercent();
               pos[2] = p1[2] + (p2[2]-p1[2]) * dtUtil::RandPercent();

               osg::Vec4 pos4(pos[0], pos[1], pos[2], NRand());
               mPoints.push_back(pos4);
            }
         }
         break;
      case VolumeRenderingComponent::CYLINDER:
      case VolumeRenderingComponent::CONE:
         {
            // p2 is a vector from p1 to the other end of cylinder.
            // p1 is apex of cone.

            p1 = center;
            p1[2] += radius[1];
            
            p2 = center;
            p2[2] -= radius[1];
            p2 = p2 - p1;

            if(s == VolumeRenderingComponent::CONE)
            {
               radius1 = 0.0f;
               radius2 = radius[0];
            }
            else
            {
               radius1 = radius[0];
               radius2 = radius[1];
            }

            radius1Sqr = radius1*radius1;

            // Given an arbitrary nonzero vector n, make two orthonormal
            // vectors u and v forming a frame [u,v,n.normalize()].
            osg::Vec3 n = p2;
            float p2l2 = n.normalize();

            // radius2Sqr stores 1 / (p2.p2)
            p2l2 *= p2l2;
            radius2Sqr = p2l2 ? 1.0f / p2l2 : 0.0f;

            // Find a vector orthogonal to n.
            osg::Vec3 basis(1.f, 0.f, 0.f);
            if(std::abs(basis * n) > 0.999)
            {
               basis.set(0.0f, 1.0f, 0.0f);
            }

            // Project away N component, normalize and cross to get
            // second orthonormal vector.
            //u = basis - n * (basis * n);
            osg::Vec3 u, v;// Orthonormal basis vectors for Cylinder/Cone
            float tmpFloat = basis * n;
            osg::Vec3 tmpv = n * tmpFloat;
            u = basis - tmpv;
            u.normalize();
            v = n ^ u;

            for(unsigned i = 0; i < numPoints; ++i)
            {
               osg::Vec3 pos;

               // For a cone, p2 is the apex of the cone.
               float dist = dtUtil::RandPercent(); // Distance between base and tip
               float theta = dtUtil::RandPercent() * 2.0f * float(osg::PI); // Angle around axis
               // Distance from axis
               float r = radius2 + dtUtil::RandPercent() * (radius1 - radius2);

               float x = r * cosf(theta); // Weighting of each frame vector
               float y = r * sinf(theta);

               // Scale radius along axis for cones
               if(s == VolumeRenderingComponent::CONE)
               {
                  x *= dist;
                  y *= dist;
               }

               //pos = p1 + p2 * dist + u * x + v * y;
               pos = p1 + p2 * dist;
               pos += u * x;
               pos += v * y;

               osg::Vec4 pos4(pos[0], pos[1], pos[2], NRand());
               mPoints.push_back(pos4);
            }            
           }
         break;
      default:

         break;
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::CreateDepthPrePass(unsigned width, unsigned height)
   {
      osg::Camera* sceneCam = GetGameManager()->GetApplication().GetCamera()->GetOSGCamera();

      mDepthCamera = new dtCore::Camera();

      VRC_DrawCallback* vrcPre = new VRC_DrawCallback(sceneCam, mDepthCamera->GetOSGCamera(), *GetGameManager()->GetScene().GetSceneNode(), VRC_DrawCallback::PRE_DRAW);
      VRC_DrawCallback* vrcPost = new VRC_DrawCallback(sceneCam, mDepthCamera->GetOSGCamera(), *GetGameManager()->GetScene().GetSceneNode(), VRC_DrawCallback::POST_DRAW);
      
      mDepthCamera->GetOSGCamera()->setPreDrawCallback(vrcPre);
      mDepthCamera->GetOSGCamera()->setPostDrawCallback(vrcPost);
      mDepthCamera->SetWindow(GetGameManager()->GetApplication().GetCamera()->GetWindow());

      GetGameManager()->GetApplication().GetCamera()->AddChild(mDepthCamera.get());

      mDepthView = new dtCore::View();
      mDepthView->SetCamera(mDepthCamera.get());
      mDepthView->SetScene(&GetGameManager()->GetScene());
      GetGameManager()->GetApplication().AddView(*mDepthView);

      //the rear view texture is used as the render target for the rear view mirror
      mDepthTexture = CreateDepthTexture(width, height);

      mDepthCamera->GetOSGCamera()->setReferenceFrame(osg::Transform::RELATIVE_RF);

      double vfov, aspectRatio, nearClip, farClip;
      sceneCam->getProjectionMatrixAsPerspective(vfov, aspectRatio, nearClip, farClip);
      mDepthCamera->SetPerspectiveParams(vfov, aspectRatio, nearClip, farClip);

      mDepthCamera->GetOSGCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

      mDepthCamera->GetOSGCamera()->setViewport(0, 0, width, height);
      mDepthCamera->GetOSGCamera()->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
      mDepthCamera->GetOSGCamera()->detach(osg::Camera::COLOR_BUFFER);
      mDepthCamera->GetOSGCamera()->attach(osg::Camera::DEPTH_BUFFER, mDepthTexture.get());

      mDepthCamera->GetOSGCamera()->setRenderOrder(osg::Camera::PRE_RENDER);
      mDepthCamera->GetOSGCamera()->setClearMask(GL_DEPTH_BUFFER_BIT);
      mDepthCamera->GetOSGCamera()->setCullMask(SimCore::Components::RenderingSupportComponent::ADDITIONAL_CAMERA_CULL_MASK);
      mDepthCamera->GetOSGCamera()->setClearColor(osg::Vec4(0.0, 0.0, 1.0, 1.0));

      mDepthTextureUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "depthTexture");
      mDepthTextureUniform->set(0);

      //the mDebugCamera just renders the result of the mRearViewTexture onto the screen
      //mDebugCamera = new osg::Camera();
      //mDebugCamera->setRenderOrder(osg::Camera::POST_RENDER, 1);
      //mDebugCamera->setClearMask(GL_NONE);
      //mDebugCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
      //mDebugCamera->setProjectionMatrixAsOrtho2D(-10.0, 10.0, -10.0, 10.0);
      //mDebugCamera->setViewport(128, 10, 256, 256);
      ////mDebugCamera->setGraphicsContext(new osgViewer::GraphicsWindowEmbedded());

      //osg::Node* quad = CreateQuad(mDepthTexture.get(), 50);
      //mDebugCamera->addChild(quad);

      //sceneCam->addChild(mDebugCamera.get());      

   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ParticleVolumeDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
   {
      BaseClass::drawImplementation(renderInfo);
   }

   ////////////////////////////////////////////////////////////////////////// 
   osg::BoundingBox VolumeRenderingComponent::ParticleVolumeDrawable::computeBound() const
   {
      osg::BoundingBox bb;
      for(unsigned i = 0; i < mNumParticles; ++i)
      {
         osg::BoundingSphere bs;
         osg::Vec4 pos = mPoints[i];
         bs.set(osg::Vec3(pos[0], pos[1], pos[2]), mParticleRadius);
         bb.expandBy(bs);
      }
      return bb;
   }

   ////////////////////////////////////////////////////////////////////////// 
   const osg::Vec4& VolumeRenderingComponent::ParticleVolumeDrawable::GetPointLocation(unsigned i) const
   {
      if(i < mNumParticles)
      {
         return mPoints[i];
      }
      else
      {
         static const osg::Vec4 defaultVec;

         return defaultVec;
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ParticleVolumeDrawable::SetPointLocation(unsigned i, const osg::Vec4& newOffset)
   {
      if(i < mNumParticles)
      {
         mPoints[i].set(newOffset[0], newOffset[1], newOffset[2], newOffset[3]);
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   float VolumeRenderingComponent::ParticleVolumeDrawable::GetParticleRadius() const
   {
      return mParticleRadius;
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::ParticleVolumeDrawable::SetParticleRadius(float f)
   {
      mParticleRadius = f;
   }


   }//namespace Components

}//namespace SimCore

