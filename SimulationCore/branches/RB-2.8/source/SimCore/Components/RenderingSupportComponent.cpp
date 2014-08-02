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
* @author Allen Danklefsen, Bradley Anderegg, Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/DynamicLightPrototypeActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>

#include <SimCore/Actors/FlareActor.h>

#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>
#include <dtGame/actorupdatemessage.h>

#include <dtABC/application.h>

#include <dtCore/scene.h>
#include <dtCore/camera.h>
#include <dtCore/deltadrawable.h>
#include <dtUtil/datapathutils.h>
#include <dtCore/system.h>
#include <dtCore/transform.h>

#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>

#include <dtUtil/noiseutility.h>
#include <dtUtil/log.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/nodemask.h>

#include <osg/Camera>
#include <osg/Geode>
#include <osg/Texture>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Version>

#include <osg/Notify>//to squelch warnings

#include <osg/GraphicsContext>
#include <osgViewer/Renderer>

namespace SimCore
{
   namespace Components
   {

#if defined (__APPLE__) && OSG_VERSION_LESS_THAN(3,2,0)
      static const std::string DYN_LIGHT_UNIFORM = "dynamicLights[0]";
      static const std::string SPOT_LIGHT_UNIFORM = "spotLights[0]";
#else
      static const std::string DYN_LIGHT_UNIFORM = "dynamicLights";
      static const std::string SPOT_LIGHT_UNIFORM = "spotLights";
#endif


     //useful functors
      struct findLightById
      {
         findLightById(RenderingSupportComponent::LightID id): mId(id){}

         template<class T>
         bool operator()(T lightPtr)
         {
            return lightPtr->GetId() == mId;
         }
      private:

         RenderingSupportComponent::LightID mId;
      };

      struct removeLightsFunc
      {
         template<class T>
         bool operator()(T lightPtr)
         {
            return lightPtr->mDeleteMe;
         }
      };


      struct funcCompareLights
      {
         funcCompareLights(const osg::Vec3& viewPos): mViewPos(viewPos){}

         //todo- cache these operations for efficiency
         template<class T>
         bool operator()(T& pElement1, T& pElement2)
         {
            osg::Vec3 vectElement1 = pElement1->mPosition - mViewPos;
            osg::Vec3 vectElement2 = pElement2->mPosition - mViewPos;

            float dist1 = dtUtil::Max(0.0f, vectElement1.length() - pElement1->mRadius);
            float dist2 = dtUtil::Max(0.0f, vectElement2.length() - pElement2->mRadius);

            return  dist1 < dist2;
         }

      private:
         osg::Vec3 mViewPos;

      };

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(RenderingSupportComponent::LightType)
      RenderingSupportComponent::LightType RenderingSupportComponent::LightType::OMNI_DIRECTIONAL("OMNI_DIRECTIONAL");
      RenderingSupportComponent::LightType RenderingSupportComponent::LightType::SPOT_LIGHT("SPOT_LIGHT");

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const std::string RenderingSupportComponent::DEFAULT_NAME = "RenderingSupportComponent";

      const std::string RenderingSupportComponent::DEFAULT_LIGHT_NAME = "DefaultLight";

      OpenThreads::Atomic RenderingSupportComponent::DynamicLight::mLightCounter(1U);

      //dyamic light constructor
      RenderingSupportComponent::DynamicLight::DynamicLight()
         : mId(++mLightCounter)
         , mLightType(&RenderingSupportComponent::LightType::OMNI_DIRECTIONAL) //omni by default
         , mDeleteMe(false)
         , mIntensity(1.0f)
         , mSaturationIntensity(1.0f)
         , mColor(1.0f, 1.0f, 1.0f)
         , mPosition()
         , mAttenuation(1.0f, 0.01f, 0.001f)
         , mFlicker(false)
         , mFlickerScale(0.1f)
         , mAutoDeleteAfterMaxTime(false)
         , mMaxTime(0.0f)
         , mFadeOut(false)
         , mFadeOutTime(0.0f)
         , mRadius(0.0f)
         , mAutoDeleteLightOnTargetNull(false)
         , mTarget(NULL)
      {
      }

      RenderingSupportComponent::DynamicLight::DynamicLight(const LightType* lightType)
         : mId(++mLightCounter)
         , mLightType(lightType)
         , mDeleteMe(false)
         , mIntensity(1.0f)
         , mSaturationIntensity(1.0f)
         , mColor(1.0f, 1.0f, 1.0f)
         , mPosition()
         , mAttenuation(1.0f, 0.01f, 0.001f)
         , mFlicker(false)
         , mFlickerScale(0.1f)
         , mAutoDeleteAfterMaxTime(false)
         , mMaxTime(0.0f)
         , mFadeOut(false)
         , mFadeOutTime(0.0f)
         , mRadius(0.0f)
         , mAutoDeleteLightOnTargetNull(false)
         , mTarget(NULL)
      {
      }

      //spot light constructor
      RenderingSupportComponent::SpotLight::SpotLight()
         : DynamicLight(&LightType::SPOT_LIGHT)
         , mUseAbsoluteDirection(false)
         , mSpotExponent(0.5)
         , mSpotCosCutoff(0.75)
         , mDirection(0.0, 1.0, 0.0)
         , mCurrentDirection(0.0, 1.0, 0.0)
      {

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::RenderingSupportComponent(const std::string& name)
         : dtGame::GMComponent(name)
         , mEnableDynamicLights(true)
         , mEnableCullVisitor(false)
         , mEnableStaticTerrainPhysics(false)
         , mEnableNVGS(false)
         , mMaxDynamicLights(15)
         , mMaxSpotLights(10)
         //, mSceneRoot(new osg::Group())
         , mGUIRoot(new osg::Camera())
         , mNVGSRoot(new osg::Camera())
         , mNVGS(0)
         , mCullVisitor(new SimCore::SimCoreCullVisitor())
      {
         // HACK!  Should net be committed!  If it is, delete it.
         mCullVisitor->SetEnablePhysics(false);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::~RenderingSupportComponent()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::OnAddedToGM()
      {
         InitializeCSM();
         InitializeFrameBuffer();

         dtCore::Camera* cam = GetGameManager()->GetApplication().GetCamera();
         osg::Camera* osgCam = cam->GetOSGCamera();
         osgCam->setCullMask(dtUtil::NodeMask::MAIN_CAMERA_CULL_MASK);

         ///Added a callback to the camera this can set uniforms on each camera.
         dtCore::Camera::AddCameraSyncCallback(*this,
                  dtCore::Camera::CameraSyncCallback(this, &RenderingSupportComponent::UpdateViewMatrix));

         osgUtil::RenderBin* terrainBin = new osgUtil::RenderBin(osgUtil::RenderBin::SORT_FRONT_TO_BACK);
         osgUtil::RenderBin::addRenderBinPrototype("TerrainBin", terrainBin);

         if (mEnableCullVisitor)
         {
            // Force a reset so it won't break if you call SetEnableCullVisitor before adding it to the GM.
            InitializeCullVisitor(*GetGameManager()->GetApplication().GetCamera());
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::OnRemovedFromGM()
      {
         dtCore::Camera::RemoveCameraSyncCallback(*this);
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      SimCoreCullVisitor* RenderingSupportComponent::GetCullVisitor()
      {
         return mCullVisitor.get();
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetGUI(dtCore::DeltaDrawable* gui)
      {
         if (gui == NULL)
         {
            mGUIRoot->removeChildren(0, mGUIRoot->getNumChildren());
         }
         else
         {
            osg::Node* node = gui->GetOSGNode();
            mGUIRoot->addChild(node);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::InitializeFrameBuffer()
      {
         mSceneRoot = GetGameManager()->GetApplication().GetScene()->GetSceneNode();

         /*dtCore::View *view = GetGameManager()->GetApplication().GetView();
         if(view != NULL)
         {
            view->GetOsgViewerView()->setSceneData(mSceneRoot.get());
            view->GetOsgViewerView()->assignSceneDataToCameras();
         }
         else
         {
            LOG_ERROR("The dtCore.View on the application is NULL. Cannot set the scene data.");
         }*/
         ///////////////////////////////////////////////////////////////////////////////////////////////

         mSceneRoot->addChild(mNVGSRoot.get());
         mSceneRoot->addChild(mGUIRoot.get());


         mNVGSRoot->setRenderOrder(osg::Camera::NESTED_RENDER);
         mGUIRoot->setRenderOrder(osg::Camera::POST_RENDER);
         mGUIRoot->setClearMask( GL_NONE );
         mGUIRoot->setNodeMask(dtUtil::NodeMask::FOREGROUND);
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::AddCamera(osg::Camera* cam)
      {
         mSceneRoot->addChild(cam);
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::InitializeCullVisitor(dtCore::Camera& pCamera)
      {
         osg::Camera* camera = pCamera.GetOSGCamera();
         osgViewer::Renderer* renderer = static_cast<osgViewer::Renderer*>(camera->getRenderer());
         if (renderer == NULL)
         {
            return;
         }

         osgUtil::SceneView* sceneView = renderer->getSceneView(0);

         mCullVisitor->setRenderStage(sceneView->getRenderStage());
         mCullVisitor->setStateGraph(sceneView->getStateGraph());

         int flags = osgUtil::CullVisitor::ENABLE_ALL_CULLING;

         // probably only need to set the scene veiw, and it
         // auto sets the cull visitor but havent checked yet.

         mCullVisitor->setCullingMode(flags);
         sceneView->setCullingMode(flags);
         sceneView->setCullVisitor(mCullVisitor.get());
      }


      ///////////////////////////////////////////////////////////////////////////////////////////////////
      bool RenderingSupportComponent::GetEnableNVGS()
      {
         return mEnableNVGS;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetEnableNVGS(bool pEnable)
      {
         if(mNVGS.valid())
         {
            mEnableNVGS = pEnable;
            if(mEnableNVGS)
            {
               //This is really bad.  It FORCES the notify level.  This is bad for debugging.

               //tell OSG to keep it quiet
               osg::setNotifyLevel(osg::FATAL);
            }
            else
            {
               osg::setNotifyLevel(osg::WARN);
            }

            mNVGS->SetEnable(mEnableNVGS);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::LoadPrototypes()
      {
         // Find all the dynamic light prototypes
         std::vector<dtCore::ActorProxy*> prototypes;
         GetGameManager()->FindPrototypesByActorType(*SimCore::Actors::EntityActorRegistry::DYNAMIC_LIGHT_PROTOTYPE_ACTOR_TYPE, prototypes);

         // Add all the prototypes to our map of light proxies. This allows others to quickly add a light by name
         unsigned int numPrototypes = prototypes.size();
         for (unsigned int i = 0; i < numPrototypes; i ++)
         {
            SimCore::Actors::DynamicLightPrototypeProxy* prototype = dynamic_cast<SimCore::Actors::DynamicLightPrototypeProxy*>(prototypes[i]);
            LOG_DEBUG("Found dynamic light proxy [" + prototype->GetName() + "].");
            mDynamicLightPrototypes.insert(std::make_pair(prototype->GetName(), prototype));
         }

         // Find all the spot light prototypes
         prototypes.clear();
         GetGameManager()->FindPrototypesByActorType(*SimCore::Actors::EntityActorRegistry::SPOT_LIGHT_PROTOTYPE_ACTOR_TYPE, prototypes);

         // Add all the prototypes to our map of light proxies. This allows others to quickly add a light by name
         numPrototypes = prototypes.size();
         for (unsigned int i = 0; i < numPrototypes; i ++)
         {
            SimCore::Actors::SpotLightPrototypeProxy* prototype = dynamic_cast<SimCore::Actors::SpotLightPrototypeProxy*>(prototypes[i]);
            LOG_DEBUG("Found spot light proxy [" + prototype->GetName() + "].");
            mSpotLightPrototypes.insert(std::make_pair(prototype->GetName(), prototype));
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::LightID RenderingSupportComponent::AddDynamicLight(DynamicLight* dl)
      {
         if (dl != NULL)
         {
            mLights.push_back(dl);
            return dl->GetId();
         }
         else
         {
            LOG_ERROR("Attempting to add a light that is NULL to the Rendering Support Component");
            return 0;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetDynamicLightProperties( SimCore::Actors::DynamicLightPrototypeActor* dlActor, DynamicLight* result )
      {
         result->mAttenuation = dlActor->GetAttenuation();
         result->mIntensity = dlActor->GetIntensity();
         result->mColor = dlActor->GetLightColor();
         // flick - is on if scale > 0
         result->mFlicker = (dlActor->GetFlickerScale() > 0.0);
         result->mFlickerScale = dlActor->GetFlickerScale();
         // delete after time if maxtime > 0
         result->mAutoDeleteAfterMaxTime = (dlActor->GetMaxTime() > 0.0);
         result->mMaxTime = dlActor->GetMaxTime();
         // fade out on if fade out time > 0
         result->mFadeOut = (dlActor->GetFadeOutTime() > 0.0);
         result->mFadeOutTime = dlActor->GetFadeOutTime();
         result->mRadius = dlActor->GetRadius();
         result->mAutoDeleteLightOnTargetNull = dlActor->IsDeleteOnTargetIsNull();

      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::DynamicLight* RenderingSupportComponent::AddDynamicLightByPrototypeName(const std::string& prototypeName)
      {
         DynamicLight* result = NULL;

         std::map<const std::string, dtCore::RefPtr<SimCore::Actors::DynamicLightPrototypeProxy> >::const_iterator iter = mDynamicLightPrototypes.find(prototypeName);
         if(iter == mDynamicLightPrototypes.end() || iter->second.get() == NULL)
         {
            LOG_ERROR("Failed to find dynamic light prototype [" + prototypeName + "]. Making bogus default light instead. Make sure the light exists in the map.");
            result = new DynamicLight();
            result->mMaxTime = 1.0f;
            result->mAutoDeleteLightOnTargetNull = true;
            mLights.push_back(result);
         }
         else
         {
            SimCore::Actors::DynamicLightPrototypeActor* dlActor = dynamic_cast<SimCore::Actors::DynamicLightPrototypeActor*>(iter->second.get()->GetDrawable());
            result = new DynamicLight();

            SetDynamicLightProperties(dlActor, result);

            // Set the light type from the enum!
            mLights.push_back(result);
         }

         return result;
      }

      RenderingSupportComponent::SpotLight* RenderingSupportComponent::AddSpotLightByPrototypeName(const std::string& prototypeName )
      {
         SpotLight* result = NULL;

         std::map<const std::string, dtCore::RefPtr<SimCore::Actors::SpotLightPrototypeProxy> >::const_iterator iter = mSpotLightPrototypes.find(prototypeName);
         if(iter == mSpotLightPrototypes.end() || iter->second.get() == NULL)
         {
            LOG_ERROR("Failed to find spot light prototype [" + prototypeName + "]. Making bogus default light instead. Make sure the light exists in the map.");
            result = new SpotLight();
            result->mMaxTime = 1.0f;
            result->mAutoDeleteLightOnTargetNull = true;
            mLights.push_back(result);
         }
         else
         {
            SimCore::Actors::SpotLightPrototypeActor* dlActor = dynamic_cast<SimCore::Actors::SpotLightPrototypeActor*>(iter->second.get()->GetDrawable());
            result = new SpotLight();

            SetDynamicLightProperties(dlActor, result);
            result->mSpotExponent = dlActor->GetSpotExponent();
            result->mSpotCosCutoff = dlActor->GetSpotCosCutoff();
            result->mDirection = dlActor->GetSpotDirection();
            result->mUseAbsoluteDirection = dlActor->GetUseAbsoluteDirection();

            // Set the light type from the enum!
            mLights.push_back(result);
         }

         return result;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::RemoveDynamicLight(RenderingSupportComponent::LightID id)
      {
         mLights.erase(std::remove_if(mLights.begin(), mLights.end(), findLightById(id)), mLights.end());
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::RemoveLight(LightArray::iterator iter)
      {
         mLights.erase(iter);
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      bool RenderingSupportComponent::HasLight(RenderingSupportComponent::LightID id) const
      {
         LightArray::const_iterator iter = std::find_if(mLights.begin(), mLights.end(), findLightById(id));
         return iter != mLights.end();
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::DynamicLight* RenderingSupportComponent::GetDynamicLight(RenderingSupportComponent::LightID id)
      {
         return FindLight(id);
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::DynamicLight* RenderingSupportComponent::FindLight(RenderingSupportComponent::LightID id)
      {
         LightArray::iterator iter = std::find_if(mLights.begin(), mLights.end(), findLightById(id));
         if(iter != mLights.end())
         {
            return (*iter).get();
         }

         return NULL;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetNVGS(RenderFeature* rf)
      {
         if(rf != NULL)
         {
            mNVGS = rf;
            mNVGS->Init(mNVGSRoot.get(), GetGameManager()->GetApplication().GetCamera());
         }
         else
         {
            mNVGS = NULL;
            mEnableNVGS = false;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      const RenderingSupportComponent::RenderFeature* RenderingSupportComponent::GetNVGS() const
      {
         return mNVGS.get();
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      bool RenderingSupportComponent::GetEnableCullVisitor()
      {
         return mEnableCullVisitor;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetEnableCullVisitor(bool pEnable)
      {
         bool valueChangingToTrue = pEnable && !mEnableCullVisitor;
         mEnableCullVisitor = pEnable;
         if (valueChangingToTrue)
         {
            if (GetGameManager() != NULL)
            {
               InitializeCullVisitor(*GetGameManager()->GetApplication().GetCamera());
            }
            else
            {
               LOG_WARNING("Code enabled the paged terrain cull visitor, but the RenderingSupportComponent has not been added to the GM.");
            }

            if (mEnableStaticTerrainPhysics)
            {
               LOG_WARNING("Turning on the terrain Cull Visitor, but Static Terrain Physics was already enabled. Disabling Static Terrain Physics, .");
               SetEnableStaticTerrainPhysics(false);
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool RenderingSupportComponent::GetEnableStaticTerrainPhysics() const
      {
         return mEnableStaticTerrainPhysics;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetEnableStaticTerrainPhysics(bool pEnable)
      {
         mEnableStaticTerrainPhysics = pEnable;

         if (pEnable && mEnableCullVisitor)
         {
            LOG_WARNING("Turning on Static Terrain Physics, but the Cull Visitor was already enabled. Disabling the Cull Visitor.");
            SetEnableCullVisitor(false);
         }
      }


      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetEnableDynamicLights(bool b)
      {
         mEnableDynamicLights = b;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      bool RenderingSupportComponent::GetEnableDynamicLights() const
      {
         return mEnableDynamicLights;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::UpdateViewMatrix(dtCore::Camera& pCamera)
      {
         osg::StateSet* ss = pCamera.GetOSGCamera()->getOrCreateStateSet();
         osg::Uniform* viewInverseUniform = ss->getOrCreateUniform("inverseViewMatrix", osg::Uniform::FLOAT_MAT4);
         viewInverseUniform->setDataVariance(osg::Object::DYNAMIC);
         
         osg::Uniform* mvpiUniform = ss->getOrCreateUniform("modelViewProjectionInverse", osg::Uniform::FLOAT_MAT4);
         mvpiUniform->setDataVariance(osg::Object::DYNAMIC);

         osg::Uniform* hprUniform = ss->getOrCreateUniform("cameraHPR", osg::Uniform::FLOAT_VEC3);
         hprUniform->setDataVariance(osg::Object::DYNAMIC);

         osg::Uniform* nearPlaneUniform = ss->getOrCreateUniform("nearPlane", osg::Uniform::FLOAT);
         nearPlaneUniform->setDataVariance(osg::Object::DYNAMIC);

         osg::Uniform* farPlaneUniform = ss->getOrCreateUniform("farPlane", osg::Uniform::FLOAT);
         farPlaneUniform->setDataVariance(osg::Object::DYNAMIC);

         osg::Uniform* screenDims = ss->getOrCreateUniform("ScreenDimensions", osg::Uniform::FLOAT_VEC2);
         screenDims->setDataVariance(osg::Object::DYNAMIC);

         osg::Vec2 dims(pCamera.GetOSGCamera()->getViewport()->width(), pCamera.GetOSGCamera()->getViewport()->height());
         screenDims->set(dims);

         osg::Matrix matView, matViewInverse, matProj, matProjInverse, matViewProj, matViewProjInverse;

         matView.set(pCamera.GetOSGCamera()->getViewMatrix());
         matViewInverse.invert(matView);

         matProj.set(pCamera.GetOSGCamera()->getProjectionMatrix());
         matProjInverse.invert(matProj);

         matViewProj = matView * matProj;
         matViewProjInverse.invert(matViewProj);

         mvpiUniform->set(matViewProjInverse);
         viewInverseUniform->set(matViewInverse);

         double vfov, aspect, nearp, farp;
         pCamera.GetPerspectiveParams(vfov, aspect, nearp, farp);
         nearPlaneUniform->set(float(nearp));
         farPlaneUniform->set(float(farp));

         dtCore::Transform trans;
         pCamera.GetTransform(trans);

         osg::Vec3 hpr;
         trans.GetRotation(hpr);
         hprUniform->set(hpr);
      }


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::ProcessMessage(const dtGame::Message& msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }
         else if (msg.GetMessageType() == dtGame::MessageType::SYSTEM_FRAME_SYNCH)
         {
            // Compute the dynamic lights position AFTER entities and the camera final positions are done
            if (mEnableDynamicLights)
            {
               UpdateDynamicLights(float(static_cast<const dtGame::TickMessage&>(msg).GetDeltaSimTime()));
            }
         }
         else if (msg.GetMessageType() == SimCore::MessageType::NIGHT_VISION)
         {
            if(mNVGS.valid())
            {
               SetEnableNVGS(static_cast<const SimCore::ToolMessage*>(&msg)->GetEnabled());
            }
         }

         // When the map change begins, clear out our lists.
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_CHANGE_BEGIN)
         {
            mLights.clear();
            mDynamicLightPrototypes.clear();
            mSpotLightPrototypes.clear();
         }

         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            LoadPrototypes();
            LoadStaticTerrain();
         }

         //else if( msg.GetMessageType() == SimCore::MessageType::INFO_ACTOR_CREATED
         //   )//|| msg.GetMessageType() == dvte::IG::MessageType::INFO_ACTOR_UPDATED )
         /*else if(msg.GetMessageType() == dtGame::MessageType::TICK_REMOTE){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_PUBLISHED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED){}*/
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::ProcessTick(const dtGame::TickMessage& msg)
      {
         //we update the view matrix for all the shaders
         //UpdateViewMatrix(); //-- now called during the framesynch message.

         if(mEnableNVGS && mNVGS.valid())
         {
            mNVGS->Update();
         }

         if(mEnableCullVisitor)
         {
            // sync up actors if needbe...
            UpdateCullVisitor();
         }


         // Should mStaticTerrainPhysicsEnabled & mEnableCullVisitor be mutually exclusive?
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::LoadStaticTerrain()
      {
         // Loads the terrain statically, as one big mesh. This completely bypasses the cull visitor

         if (mEnableStaticTerrainPhysics)
         {

            // Get the physics land actor
            SimCore::Actors::PagedTerrainPhysicsActorProxy* landActorProxy = NULL;
            GetGameManager()->FindActorByName(SimCore::Actors::PagedTerrainPhysicsActor::DEFAULT_NAME, landActorProxy);

            if (landActorProxy == NULL)
            {
               dtCore::RefPtr<SimCore::Actors::PagedTerrainPhysicsActorProxy> terrainPhysicsActorProxy = NULL;
               GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::PAGED_TERRAIN_PHYSICS_ACTOR_TYPE, terrainPhysicsActorProxy);
               GetGameManager()->AddActor(*terrainPhysicsActorProxy, false, false);
               landActorProxy = terrainPhysicsActorProxy;
            }

            if (landActorProxy != NULL)
            {
               SimCore::Actors::PagedTerrainPhysicsActor* landActor = NULL;
               landActorProxy->GetDrawable(landActor);

               // Get the terrain - which has our mesh node
               dtCore::ActorProxy* terrainActorProxy;
               GetGameManager()->FindActorByName(SimCore::Actors::TerrainActor::DEFAULT_NAME, terrainActorProxy);
               if (terrainActorProxy != NULL)
               {
                  static int nameCounter = 1;
                  landActor->ClearAllTerrainPhysics();
                  std::string bogusIndexthing;
                  dtUtil::MakeIndexString(nameCounter, bogusIndexthing);
                  nameCounter ++;

                  bool usePerGeodeLoading = true;

                  // Build the terrain as a static mesh, but with each geode loaded separately
                  landActor->BuildTerrainAsStaticMesh(terrainActorProxy->GetDrawable()->GetOSGNode(),
                     "Base Terrain " + bogusIndexthing, usePerGeodeLoading);
               }
               else
               {
                  landActor->ClearAllTerrainPhysics();
                  // Clear if we had something before and now we don't
                  landActor->BuildTerrainAsStaticMesh(NULL, "", false);
               }

            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::TimeoutAndDeleteLights(float dt)
      {
         LightArray::iterator iter = mLights.begin();
         LightArray::iterator endIter = mLights.end();

         for(;iter != endIter; ++iter)
         {
            DynamicLight* dl = (*iter).get();

            //this if check looks a little iffy but the control flow is used to allow the auto delete, max time, and fade out to all work together
            //while the first component of the if is straight forward, the second component "(!dl->mAutoDeleteAfterMaxTime && dl->mAutoDeleteLightOnTargetNull)"
            //ensures that we do not fade out if are target is still valid but we DONT have a max time
            if((dl->mAutoDeleteLightOnTargetNull && !dl->mTarget.valid()) || (!dl->mAutoDeleteAfterMaxTime && dl->mAutoDeleteLightOnTargetNull))
            {
               if(!dl->mTarget.valid())
               {
                  if(dl->mFadeOut)
                  {
                     //by setting this to false we will continue into a fade out
                     dl->mAutoDeleteLightOnTargetNull = false;
                     //by setting this one false we ensure we will begin fading out next frame
                     dl->mAutoDeleteAfterMaxTime = false;
                  }
                  else
                  {
                     dl->mDeleteMe = true;
                     //std::cout << "Auto delete on NULL Ptr" << std::endl;
                     continue;
                  }
               }
            }
            else if(dl->mAutoDeleteAfterMaxTime)
            {
               dl->mMaxTime -= dt;

               if(dl->mMaxTime <= 0.0f)
               {
                  if(dl->mFadeOut)
                  {
                     //by setting this to false we will continue into a fade out
                     dl->mAutoDeleteAfterMaxTime = false;
                     //by setting this one false we ensure we will begin fading out next frame
                     dl->mAutoDeleteLightOnTargetNull = false;
                  }
                  else
                  {
                     dl->mDeleteMe = true;
                     //std::cout << "Auto delete on Max Time" << std::endl;
                     continue;
                  }
               }
            }
            else if(dl->mFadeOut)
            {
               dl->mIntensity -= (dt / dl->mFadeOutTime);
               if(dl->mIntensity <= 0.0f)
               {
                  dl->mDeleteMe = true;
                  //std::cout << "Auto delete on fade out" << std::endl;
                  continue;
               }
            }

            //apply different update effects
            if(dl->mFlicker)
            {
               //lets flicker the lights a little
               static dtUtil::Noise1f perlinNoise;
               float noiseValue = dl->mFlickerScale * perlinNoise.GetNoise(dt + dtUtil::RandFloat(0.0f, 10.0f));

               //keep the intensity within range of the noise flicker
               //TODO- don't assume an intensity of 1.0
               if(dtUtil::Abs(1.0f - (dl->mIntensity + noiseValue)) > dl->mFlickerScale) noiseValue *= -1.0f;
               dl->mIntensity += noiseValue;

               //std::cout << "Intensity " << dl->mIntensity << std::endl;
            }

         }

         //now remove all flagged lights, note this is actually faster because we only have a single deallocation for N lights
         mLights.erase(std::remove_if(mLights.begin(), mLights.end(), removeLightsFunc()), mLights.end());

      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::TransformAndSortLights()
      {
         LightArray::iterator iter = mLights.begin();
         LightArray::iterator endIter = mLights.end();

         for(;iter != endIter; ++iter)
         {
            DynamicLight* dl = (*iter).get();
            if(dl->mTarget.valid())
            {
               //update the light's position
               SetPosition(dl);
            }

            if (dl->GetLightType() == LightType::SPOT_LIGHT)
            {
               SpotLight* sLight = dynamic_cast<SpotLight*>(dl);
               if(sLight != NULL)
               {
                  SetDirection(sLight);
               }
            }
         }

         //update uniforms by finding the closest lights to the camera
         dtCore::Transform trans;
         GetGameManager()->GetApplication().GetCamera()->GetTransform(trans);
         osg::Vec3 pos;
         trans.GetTranslation(pos);
         //sort the lights, though a heap may be more efficient here, we will sort so that we can combine lights later
         std::sort(mLights.begin(), mLights.end(), funcCompareLights(pos));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::FindBestLights(dtCore::Transformable& actor)
      {
         LightArray tempLightArray(mLights);

         dtCore::Transform trans;
         actor.GetTransform(trans);
         osg::Vec3 pos;
         trans.GetTranslation(pos);
         //sort the lights, though a heap may be more efficient here, we will sort so that we can combine lights later
         std::sort(tempLightArray.begin(), tempLightArray.end(), funcCompareLights(pos));

         //now setup the lighting uniforms necessary for rendering the dynamic lights
         osg::StateSet* ss = actor.GetOSGNode()->getOrCreateStateSet();
         //temporary hack
         osg::Uniform* lightArrayUniform = ss->getOrCreateUniform(DYN_LIGHT_UNIFORM, osg::Uniform::FLOAT_VEC4, mMaxDynamicLights * 3);
         lightArrayUniform->setDataVariance(osg::Object::DYNAMIC);

         osg::Uniform* spotLightArrayUniform = ss->getOrCreateUniform(SPOT_LIGHT_UNIFORM, osg::Uniform::FLOAT_VEC4, mMaxSpotLights * 4);
         spotLightArrayUniform->setDataVariance(osg::Object::DYNAMIC);

         UpdateDynamicLightUniforms(tempLightArray, lightArrayUniform, spotLightArrayUniform);
      }

	  ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::UpdateDynamicLightUniforms(osg::Uniform* lightArray, osg::Uniform* spotLightArray)
      {
		 UpdateDynamicLightUniforms(mLights, lightArray, spotLightArray);
	  }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::UpdateDynamicLightUniforms(const LightArray& lights, osg::Uniform* lightArray, osg::Uniform* spotLightArray)
      {
         unsigned numDynamicLights = 0;
         unsigned numSpotLights = 0;

         unsigned numDynamicLightAttributes = 3;
         unsigned numSpotLightAttributes = 4;

         unsigned maxDynamicLightUniforms = numDynamicLightAttributes * mMaxDynamicLights;
         unsigned maxSpotLightUniforms = numSpotLightAttributes * mMaxSpotLights;

         unsigned numLights = 0;
         unsigned totalLightSlots = mMaxSpotLights + mMaxDynamicLights;

         LightArray::const_iterator iter = lights.begin();
         LightArray::const_iterator endIter = lights.end();

         // Go over our lights and add them if they have actual intensity.
         for(; iter != endIter && numLights < totalLightSlots; ++iter)
         {
            DynamicLight* dl = (*iter).get();
            SpotLight* sl = NULL;

            bool useSpotLight = false;
            float spotExp = 0.0f;
            osg::Vec4 spotParams;

            //if we have an open slot for spot lights and we have a spot light
            //else we are out of dynamic light spots and we have a dynamic light make a spot light and bind it here
            if( (numSpotLights < maxSpotLightUniforms) && (dl->GetLightType() == LightType::SPOT_LIGHT))
            {
               sl = static_cast<SpotLight*>(dl); // if it's not a SpotLight, crash anyway to let the dev know they messed up
               spotExp = sl->mSpotExponent;
               spotParams.set(sl->mCurrentDirection[0], sl->mCurrentDirection[1], sl->mCurrentDirection[2], sl->mSpotCosCutoff);
               useSpotLight = true;
            }
            else if( !(numDynamicLights < maxDynamicLightUniforms) )
            {
               spotExp = 0.0f;
               spotParams.set(0.0f, 1.0f, 0.0f, -1.0f);
               useSpotLight = true;
            }

            //don't bind lights of zero intensity
            if(dl->mIntensity > 0.0001f)
            {
               if(useSpotLight)
               {
                  spotLightArray->setElement(numSpotLights, osg::Vec4(dl->mPosition, dl->mIntensity));
                  spotLightArray->setElement(numSpotLights + 1, osg::Vec4(dl->mColor, 1.0f));
                  spotLightArray->setElement(numSpotLights + 2, osg::Vec4(dl->mAttenuation, spotExp));
                  spotLightArray->setElement(numSpotLights + 3, spotParams);
                  numSpotLights += numSpotLightAttributes;
               }
               else
               {
                  lightArray->setElement(numDynamicLights, osg::Vec4(dl->mPosition, dl->mIntensity));
                  lightArray->setElement(numDynamicLights + 1, osg::Vec4(dl->mColor, 1.0f));
                  lightArray->setElement(numDynamicLights + 2, osg::Vec4(dl->mAttenuation, 1.0f));
                  numDynamicLights += numDynamicLightAttributes;
               }

               ++numLights;
            }
         }

         // clear out any remaining dynamic lights from previous by setting intensity to 0
         for(; numDynamicLights < maxDynamicLightUniforms; numDynamicLights += numDynamicLightAttributes)
         {
            lightArray->setElement(numDynamicLights, osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
            lightArray->setElement(numDynamicLights + 1, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
            lightArray->setElement(numDynamicLights + 2, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
         }

         // Clear out any remaining spot lights from previous by setting intensity to 0
         for(; numSpotLights < maxSpotLightUniforms; numSpotLights += numSpotLightAttributes)
         {
            spotLightArray->setElement(numSpotLights, osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
            spotLightArray->setElement(numSpotLights + 1, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
            spotLightArray->setElement(numSpotLights + 2, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
            spotLightArray->setElement(numSpotLights + 3, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::UpdateDynamicLights(float dt)
      {
         TimeoutAndDeleteLights(dt);
         TransformAndSortLights();

         //now setup the lighting uniforms necessary for rendering the dynamic lights
         osg::StateSet* ss = GetGameManager()->GetScene().GetSceneNode()->getOrCreateStateSet();
         osg::Uniform* lightArray = ss->getOrCreateUniform(DYN_LIGHT_UNIFORM, osg::Uniform::FLOAT_VEC4, mMaxDynamicLights * 3);
         lightArray->setDataVariance(osg::Object::DYNAMIC);

         osg::Uniform* spotLightArray = ss->getOrCreateUniform(SPOT_LIGHT_UNIFORM, osg::Uniform::FLOAT_VEC4, mMaxSpotLights * 4);
         spotLightArray->setDataVariance(osg::Object::DYNAMIC);

         UpdateDynamicLightUniforms(mLights, lightArray, spotLightArray);
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetPosition(DynamicLight* dl)
      {
         if(dl != NULL && dl->mTarget.valid())
         {
            dtCore::Transform xform;
            dl->mTarget->GetTransform(xform);
            xform.GetTranslation(dl->mPosition);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetDirection(SpotLight* light)
      {
         if(light->mTarget.valid() && !light->mUseAbsoluteDirection)
         {
            //this transforms our direction relative to our target
            //to create a world space direction
            dtCore::Transform xform;
            light->mTarget->GetTransform(xform);
            osg::Matrix rot;
            xform.GetRotation(rot);

            light->mDirection.normalize();
            light->mCurrentDirection = rot.preMult(light->mDirection);

            light->mCurrentDirection.normalize();

         }
         else
         {
            //if our direction is absolute or we do not have a target
            //then our world space direction is our current local direction
            light->mCurrentDirection = light->mDirection;
         }
      }


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      bool RenderingSupportComponent::UpdateCullVisitor()
      {
         if(!mCullVisitor.valid())
         {
            LOG_ALWAYS("mCullVisitor Is not valid in Rendering Component, which means \
               it wasnt initialized in OnAddedToGM. ");
            return false;
         }

         std::vector<dtCore::ActorProxy*> toFill;

         if (mCullVisitor->GetLandActor() == NULL)
         {
            SimCore::Actors::PagedTerrainPhysicsActorProxy* landActorProxy = NULL;
            GetGameManager()->FindActorByName(SimCore::Actors::PagedTerrainPhysicsActor::DEFAULT_NAME, landActorProxy);

            if (landActorProxy != NULL)
            {
               SimCore::Actors::PagedTerrainPhysicsActor* landActor = NULL;
               landActorProxy->GetDrawable(landActor);
               mCullVisitor->SetLandActor(landActor);
            }
            else
            {
               dtCore::RefPtr<SimCore::Actors::PagedTerrainPhysicsActorProxy> terrainPhysicsActorProxy = NULL;
               GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::PAGED_TERRAIN_PHYSICS_ACTOR_TYPE, terrainPhysicsActorProxy);
               GetGameManager()->AddActor(*terrainPhysicsActorProxy, false, false);
               SimCore::Actors::PagedTerrainPhysicsActor* landActor = NULL;
               terrainPhysicsActorProxy->GetDrawable(landActor);
               mCullVisitor->SetLandActor(landActor);
            }
         }

         if (mCullVisitor->GetTerrainNode() == NULL)
         {
            GetGameManager()->FindActorsByName("Terrain", toFill);
            if(!toFill.empty())
            {
               mCullVisitor->SetTerrainNode(toFill[0]->GetDrawable()->GetOSGNode()->asTransform());
            }
         }

         // Finalize if there is anything to do.
         mCullVisitor->FinalizeTerrain();

         dtCore::Transform cameraTransform;
         GetGameManager()->GetApplication().GetCamera()->GetTransform(cameraTransform);
         osg::Vec3 pos;
         cameraTransform.GetTranslation(pos);
         mCullVisitor->SetCameraTransform(pos);
         return true;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::InitializeCSM()
      {
         char* csmData = getenv("CSM_DATA");
         if(csmData == NULL)
         {
            std::string csmPath = dtCore::Project::GetInstance().GetContext();
            for(size_t i = 0; i < csmPath.size(); i++)
            {
               if(csmPath[i] == '\\')
               {
                  csmPath[i] = '/';
               }
            }

            csmPath += "/CSM/data";

            dtUtil::SetEnvironment("CSM_DATA", csmPath);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetMaxDynamicLights( unsigned lights )
      {
         mMaxDynamicLights = lights;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      unsigned RenderingSupportComponent::GetMaxDynamicLights() const
      {
         return mMaxDynamicLights;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetMaxSpotLights( unsigned lights )
      {
         mMaxSpotLights = lights;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      unsigned RenderingSupportComponent::GetMaxSpotLights() const
      {
         return mMaxSpotLights;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      osg::Group* RenderingSupportComponent::GetSceneRoot()
      {
         return mSceneRoot.get();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      osg::Group* RenderingSupportComponent::GetNVGSRoot()
      {
         return mNVGSRoot.get();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      osg::Group* RenderingSupportComponent::GetGUIRoot()
      {
         return mGUIRoot.get();
      }
   }
}
