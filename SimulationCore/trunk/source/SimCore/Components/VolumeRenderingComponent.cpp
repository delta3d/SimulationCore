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
   if (tex != NULL)
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
}

static osg::Texture2D* CreateTexture(int width, int height)
{
   osg::Texture2D* tex = new osg::Texture2D();
   tex->setTextureSize(width, height);
   tex->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT);
   tex->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT);
   tex->setInternalFormat(GL_RGBA);
   tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
   tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
   return tex;
}


namespace SimCore
{
   namespace Components
   {

   const std::string VolumeRenderingComponent::DEFAULT_NAME = "VolumeRenderingComponent";

   /////////////////////////////////////////////////////////////
   //useful functors
   /////////////////////////////////////////////////////////////
   struct findVolumeById
   {
      findVolumeById(VolumeRenderingComponent::ShapeRecordID id): mId(id){}

      template<class T>
      bool operator()(T ptr)
      {
         return ptr->mId == mId;
      }
   private:

      VolumeRenderingComponent::ShapeRecordID mId;
   };

   struct findVolumeByUniqueId
   {
      findVolumeByUniqueId(const dtCore::UniqueId& id): mId(id){}

      template<class T>
      bool operator()(T ptr)
      {
         bool result = false; 

         if(ptr->mTarget.valid())
         {
            result = (ptr->mTarget->GetUniqueId() == mId);
         }
         
         return result;
      }
   private:

      dtCore::UniqueId mId;
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
   OpenThreads::Atomic VolumeRenderingComponent::ShapeVolumeRecord::mCounter = 0;
   
   //////////////////////////////////////////////////////////////////////////
   VolumeRenderingComponent::ShapeVolumeRecord::ShapeVolumeRecord()
      : mId(mCounter)
      , mShapeType(VolumeRenderingComponent::SPHERE)
      , mRenderMode(SIMPLE_SHAPE_GEOMETRY)
      , mDeleteMe(false)
      , mAutoDeleteAfterMaxTime(false)
      , mMaxTime(0.0f)
      , mFadeOut(false)
      , mFadeOutTime(0.0f)
      , mIntensity(1.0f)
      , mNumParticles(150)
      , mParticleRadius(5.0f)
      , mColor(1.0f, 1.0f, 1.0f, 0.5f)
      , mPosition(0.0f, 0.0f, 0.0f)
      , mRadius(0.0f, 0.0f, 0.0f)
      , mVelocity(0.0f, 0.0f, 0.0f)
      , mTransform()
      , mTarget(NULL)
   {
      ++mCounter;
      mTransform.makeIdentity();

   }

   /////////////////////////////////////////////////////////////
   VolumeRenderingComponent::VolumeRenderingComponent(const std::string& name)
   : BaseClass(name) 
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
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();

      CreateDepthPrePass("sceneDepth", 512, 512);
      GetGameManager()->GetScene().GetSceneNode()->addChild(mRootNode.get());

      mRootNode->setNodeMask(RenderingSupportComponent::MAIN_CAMERA_ONLY_FEATURE_NODE_MASK);

      CreateNoiseTexture();
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::OnRemovedFromGM()
   {
      CleanUp();
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::Init()
   {
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RegisterActor(Actors::SimpleMovingShapeActorProxy& newActor)
   {
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::UnregisterActor(const dtCore::UniqueId& actorID)
   {
      
   }
   
   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RemoveShapeVolume(ShapeRecordID id)
   {
      mVolumes.erase(std::remove_if(mVolumes.begin(), mVolumes.end(), findVolumeById(id)), mVolumes.end());
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeRecordID VolumeRenderingComponent::CreateShapeVolume(dtCore::RefPtr<ShapeVolumeRecord> svr)
   {
      if (svr != NULL)
      {
         mVolumes.push_back(svr);
         CreateDrawable(*svr);
         return svr->mId;
      }
      else
      {
         LOG_ERROR("Attempting to add a ShapeVolumeRecord that is NULL to the Volume Rendering Component");
         return 0;
      }
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeRecordID VolumeRenderingComponent::CreateStaticShapeVolume(Shape s, const osg::Vec4& color, const osg::Vec3& center, const osg::Vec3& radius)
   {
      dtCore::RefPtr<ShapeVolumeRecord> svr = new ShapeVolumeRecord();
      svr->mShapeType = s;
      svr->mColor = color;
      svr->mPosition = center;
      svr->mRadius = radius;
      return CreateShapeVolume(svr);
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindShapeVolumeForActor(const dtCore::UniqueId& actorID)
   {
      ShapeVolumeArray::iterator iter = std::find_if(mVolumes.begin(), mVolumes.end(), findVolumeByUniqueId(actorID));
      if(iter != mVolumes.end())
      {
         return (*iter).get();
      }

      return NULL;
   }

   ////////////////////////////////////////////////////////////////////////// 
   /*void VolumeRenderingComponent::FindAllShapeVolumesForActor( const dtCore::UniqueId& actorID, std::vector<ShapeVolumeRecord*> pContainerToFill )
   {

   }*/

   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::ProcessMessage(const dtGame::Message& message)
   {
      
      if(message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
      {
         float dt = float(static_cast<const dtGame::TickMessage&>(message).GetDeltaSimTime());
         Tick(dt);
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == message.GetMessageType())
      {
         const dtGame::ActorUpdateMessage& updateMessage = static_cast<const dtGame::ActorUpdateMessage&> (message);
         
         dtDAL::ActorProxy* actor = GetGameManager()->FindActorById(updateMessage.GetAboutActorId());
         if (actor == NULL)
         {
            return;
         }

         const dtDAL::ActorType& type = actor->GetActorType();

         if (type == *SimCore::Actors::EntityActorRegistry::ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE)
         {
            Actors::SimpleMovingShapeActorProxy* proxy =
               static_cast<Actors::SimpleMovingShapeActorProxy*>(actor);
            RegisterActor(*proxy);
         }

      }
      else if (message.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED)
      {
         // TODO Write unit tests for the delete message.
         UnregisterActor(message.GetAboutActorId());
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
      {
         CleanUp();
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
                  svr->mDeleteMe = true;
                  //std::cout << "Auto delete on NULL Ptr" << std::ensvr;
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
                  svr->mDeleteMe = true;
                  //std::cout << "Auto delete on Max Time" << std::ensvr;
                  continue;
               }
            }
         }
         else if(svr->mFadeOut)
         {
            svr->mIntensity -= (dt / svr->mFadeOutTime);
            if(svr->mIntensity <= 0.0f)
            {
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
      if(svr != NULL && svr->mTarget.valid())
      {
         dtCore::Transform xform;
         svr->mTarget->GetTransform(xform);
         xform.Move(svr->mPosition);

         osg::Matrix mat;
         xform.Get(mat);
         svr->mParentNode->setMatrix(mat);
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RemoveVolume(ShapeVolumeArray::iterator iter)
   {
      mVolumes.erase(iter);
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindVolume(ShapeRecordID id)
   {
      ShapeVolumeArray::iterator iter = std::find_if(mVolumes.begin(), mVolumes.end(), findVolumeById(id));
      if(iter != mVolumes.end())
      {
         return (*iter).get();
      }

      return NULL;
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindShapeVolumeById(ShapeRecordID id)
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
      dtCore::RefPtr<osg::Geode> g = new osg::Geode();
      dtCore::RefPtr<osg::MatrixTransform> matTrans = new osg::MatrixTransform();
      newShape.mParentNode = matTrans.get();
      newShape.mParentNode->addChild(g.get());

      osg::StateSet* ss = g->getOrCreateStateSet();
      ss->setMode(GL_BLEND, osg::StateAttribute::ON);

      osg::BlendFunc* blendFunc = new osg::BlendFunc();
      blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
      ss->setAttributeAndModes(blendFunc);
      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


      CreateShape(newShape);

      dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(newShape.mShape.get());
      shapeDrawable->setColor(newShape.mColor);
      g->addDrawable(shapeDrawable);

      mRootNode->addChild(newShape.mParentNode.get());
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateParticleVolume(ShapeVolumeRecord& newShape)
   {

      dtCore::RefPtr<osg::Geode> g = new osg::Geode();
      //dtCore::RefPtr<osg::Billboard> g = new osg::Billboard();
      //g->setMode(osg::Billboard::POINT_ROT_WORLD);

      dtCore::RefPtr<osg::MatrixTransform> matTrans = new osg::MatrixTransform();
      newShape.mParentNode = matTrans.get();
      newShape.mParentNode->addChild(g.get());

      osg::StateSet* ss = g->getOrCreateStateSet();
      ss->setMode(GL_BLEND, osg::StateAttribute::ON);

      osg::Depth* depthSS = new osg::Depth();
      depthSS->setWriteMask(false);
      ss->setAttribute(depthSS);

      osg::BlendFunc* blendFunc = new osg::BlendFunc();
      blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
      ss->setAttributeAndModes(blendFunc);
      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

      CreateShape(newShape);

      newShape.mParticleDrawable = new ParticleVolumeDrawable();
      newShape.mParticleDrawable->Init(newShape.mNumParticles, &newShape);
      g->addDrawable(newShape.mParticleDrawable.get());

      AssignParticleVolumeShader(*newShape.mParticleDrawable, *g);
      AssignParticleVolumeUniforms(newShape);

      mRootNode->addChild(newShape.mParentNode.get());
   
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateShape(ShapeVolumeRecord& newShape)
   {
      switch(newShape.mShapeType)
      {
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
      case CAPSULE:
         {
            newShape.mShape = new osg::Capsule(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1]);
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
   void VolumeRenderingComponent::AssignParticleVolumeShader(ParticleVolumeDrawable& pvd, osg::Geode& g)
   {
      dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
      dtCore::ShaderProgram* sp = sm.FindShaderPrototype("ParticleVolumeShader", "VolumeRenderingGroup");
      if(sp != NULL)
      {
         sm.AssignShaderFromPrototype(*sp, g);

         osg::Uniform* tex = new osg::Uniform(osg::Uniform::SAMPLER_2D, "depthTexture");
         tex->set(0);
         mRootNode->getOrCreateStateSet()->addUniform(tex);
         mRootNode->getOrCreateStateSet()->setTextureAttributeAndModes(0, mDepthTexture.get(), osg::StateAttribute::ON);

         mRootNode->getOrCreateStateSet()->addUniform(mNoiseTextureUniform.get());
         mRootNode->getOrCreateStateSet()->setTextureAttributeAndModes(1, mNoiseTexture.get(), osg::StateAttribute::ON);
      }
      else
      {
         LOG_ERROR("Unable to find shader for Particle Volume.");
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::AssignParticleVolumeUniforms(ShapeVolumeRecord& newShape)
   {
      osg::StateSet* ss = newShape.mParticleDrawable->getOrCreateStateSet();

#ifdef __APPLE__
      static const std::string VOLUME_PARTICLE_POS_UNIFORM = "volumeParticlePos[0]";
#else
      static const std::string VOLUME_PARTICLE_POS_UNIFORM = "volumeParticlePos";
#endif
      static const std::string VOLUME_PARTICLE_COLOR_UNIFORM = "volumeParticleColor";

      osg::Uniform* particleColor = ss->getOrCreateUniform(VOLUME_PARTICLE_COLOR_UNIFORM, osg::Uniform::FLOAT_VEC4);
      particleColor->set(newShape.mColor);

      osg::Uniform* particleArray = ss->getOrCreateUniform(VOLUME_PARTICLE_POS_UNIFORM, osg::Uniform::FLOAT_VEC4, newShape.mParticleDrawable->GetNumParticles());
      
      for(unsigned i = 0; i < newShape.mParticleDrawable->GetNumParticles(); ++i)
      {
         osg::Vec3 pos = newShape.mParticleDrawable->GetPointLocation(i);
         //last variable is the radius, todo- make part of the shape description
         particleArray->setElement(i, osg::Vec4(pos[0], pos[1], pos[2], newShape.mParticleRadius));
      }

   }

   void VolumeRenderingComponent::CreateNoiseTexture()
   {
      LOG_INFO("Creating noise texture for VolumeRenderingComponent.");
      dtUtil::NoiseTexture noise3d(6, 2, 0.7, 0.5, 16, 16, 16);
      dtCore::RefPtr<osg::Image> img = noise3d.MakeNoiseTexture(GL_ALPHA);
      LOG_INFO("Finished creating noise texture for VolumeRenderingComponent.");

      dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();

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
      : mNumParticles(150)
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
         v[(i * 4) + 0].set(-w*0.5f,-h*0.5f, 0.0f, i);
         v[(i * 4) + 1].set( w*0.5f,-h*0.5f, 0.0f, i);
         v[(i * 4) + 2].set( w*0.5f,h*0.5f, 0.0f, i);
         v[(i * 4) + 3].set(-w*0.5f,h*0.5f, 0.0f, i);

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
      float radius1 = 0.0f;// Outer radius
      float radius2 = 0.0f;// Inner radius
      float radius1Sqr = 0.0f;// Used for fast Within test of spheres,
      float radius2Sqr = 0.0f;// and for mag. of u and v vectors for plane.


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

               mPoints.push_back(pos);
            }
         }
         break;
         /*
         case VolumeRenderingComponent::GAUSSIAN_SPHERE:
         {

         }
         break;
         */
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
               
               mPoints.push_back(pos);
            }
         }
         break;
      case VolumeRenderingComponent::CAPSULE:
         {
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
               radius2 = radius[0];
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
            if(fabsf(basis * n) > 0.999)
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

               mPoints.push_back(pos);
            }            
           }
         break;
      default:

         break;
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::CreateDepthPrePass(const std::string& textureName, unsigned width, unsigned height)
   {

      osg::Camera* sceneCam = GetGameManager()->GetApplication().GetCamera()->GetOSGCamera();

      mDepthCamera= new dtCore::Camera();
      mDepthCamera->SetWindow(GetGameManager()->GetApplication().GetCamera()->GetWindow());

      GetGameManager()->GetApplication().GetCamera()->AddChild(mDepthCamera.get());

      mDepthView = new dtCore::View();
      mDepthView->SetCamera(mDepthCamera.get());
      mDepthView->SetScene(&GetGameManager()->GetScene());
      GetGameManager()->GetApplication().AddView(*mDepthView);

      //the rear view texture is used as the render target for the rear view mirror
      mDepthTexture = CreateTexture(width, height);
      mDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT);

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

      //the mDebugCamera just renders the result of the mRearViewTexture onto the screen
      /*mDebugCamera = new osg::Camera();
      mDebugCamera->setRenderOrder(osg::Camera::POST_RENDER, 1);
      mDebugCamera->setClearMask(GL_NONE);
      mDebugCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
      mDebugCamera->setProjectionMatrixAsOrtho2D(-10.0, 10.0, -10.0, 10.0);
      mDebugCamera->setViewport(128, 10, 256, 256);*/
      //mDebugCamera->setGraphicsContext(new osgViewer::GraphicsWindowEmbedded());

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
      return osg::BoundingBox();
   }

   ////////////////////////////////////////////////////////////////////////// 
   const osg::Vec3& VolumeRenderingComponent::ParticleVolumeDrawable::GetPointLocation(unsigned i) const
   {
      if(i < mNumParticles)
      {
         return mPoints[i];
      }
      else
      {
         static const osg::Vec3 defaultVec;

         return defaultVec;
      }
   }


   }//namespace Components
}//namespace SimCore

