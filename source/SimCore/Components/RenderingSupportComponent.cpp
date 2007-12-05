/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen, Bradley Anderegg, Curtiss Murphy
*/
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/DynamicLightPrototypeActor.h>
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
#include <dtDAL/enginepropertytypes.h>

#include <dtUtil/noiseutility.h> 
#include <dtUtil/log.h>

#include <osg/CameraNode>
#include <osg/Geode>
#include <osg/Texture>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/StateSet>

#include <osg/Notify>//to squelch warnings

#include <osg/GraphicsContext>
#include <osgViewer/Renderer>

namespace SimCore
{
   namespace Components
   {

     //useful functors
      struct findLightById
      { 
         findLightById(RenderingSupportComponent::LightID id): mID(id){}

         template<class T>
         bool operator()(T lightPtr)
         {
            return lightPtr->mID == mID;
         }
      private:

         RenderingSupportComponent::LightID mID;
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


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const std::string &RenderingSupportComponent::DEFAULT_NAME = "RenderingSupportComponent";  

      const std::string &RenderingSupportComponent::DEFAULT_LIGHT_NAME = "DefaultLight";      

      RenderingSupportComponent::LightID RenderingSupportComponent::DynamicLight::mLightCounter = 1;

      const unsigned RenderingSupportComponent::MAX_LIGHTS = 20;

      //dyamic light constructor
      RenderingSupportComponent::DynamicLight::DynamicLight()
         : mDeleteMe(false)
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
         , mID(++mLightCounter)
         , mAutoDeleteLightOnTargetNull(false)
         , mTarget(NULL)
      {

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::RenderingSupportComponent(const std::string &name) 
         : dtGame::GMComponent(name)
         , mEnableDynamicLights(true)
         , mEnableCullVisitor(false)
         , mEnableNVGS(false)
         , mDeltaScene(new osg::Group())
         , mSceneRoot(new osg::Group())
         , mGUIRoot(new osg::CameraNode())
         , mNVGSRoot(new osg::CameraNode())
         , mNVGS(0)
         , mCullVisitor(new SimCore::AgeiaTerrainCullVisitor())
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::~RenderingSupportComponent(void)
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::OnAddedToGM()
      {
         InitializeFrameBuffer();

         //we we are setup to use the cullvisitor then initialize it
         if(mEnableCullVisitor)
         {
            InitializeCullVisitor();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      AgeiaTerrainCullVisitor* RenderingSupportComponent::GetCullVisitor()
      {
         return mCullVisitor.get();
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetGUI(dtCore::DeltaDrawable* gui)
      {
         mGUIRoot->addChild(gui->GetOSGNode());
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::InitializeFrameBuffer()
      {
         GetGameManager()->GetApplication().GetScene()->SetSceneNode(mDeltaScene.get());
         // TODO-UPGRADE ///////////////////////////////////////////////////////////////////////////////
         //GetGameManager()->GetApplication().GetCamera()->GetOSGCamera()->getOs->setSceneData(mSceneRoot.get());
         ///////////////////////////////////////////////////////////////////////////////////////////////

         mSceneRoot->addChild(mNVGSRoot.get());
         mSceneRoot->addChild(mGUIRoot.get());
         mSceneRoot->addChild(mDeltaScene.get());                 
                  
         mNVGSRoot->setRenderOrder(osg::CameraNode::NESTED_RENDER);      
         mGUIRoot->setRenderOrder(osg::CameraNode::POST_RENDER);
         mGUIRoot->setClearMask( GL_NONE );
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::InitializeCullVisitor()
      {        
         osg::Camera* camera = GetGameManager()->GetApplication().GetCamera()->GetOSGCamera();/*->GetSceneView();*/
         osgViewer::Renderer* renderer = static_cast<osgViewer::Renderer*>(camera->getRenderer());
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
               //tell OSG to keep it quite
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
         std::vector<dtDAL::ActorProxy*> prototypes;
         GetGameManager()->FindPrototypesByActorType(*SimCore::Actors::EntityActorRegistry::DYNAMIC_LIGHT_PROTOTYPE_ACTOR_TYPE, prototypes);
         if(prototypes.empty())
         {
            LOG_WARNING("The Rendering component could not find any dynamic light prototypes. Make sure you loaded the correct map.");
         }

         // Add all the prototypes to our map of light proxies. This allows others to quickly add a light by name
         unsigned int numPrototypes = prototypes.size();
         for (unsigned int i = 0; i < numPrototypes; i ++)
         {
            SimCore::Actors::DynamicLightPrototypeProxy *prototype = (SimCore::Actors::DynamicLightPrototypeProxy *)prototypes[i];
            LOG_DEBUG("Found dynamic light proxy [" + prototype->GetName() + "].");
            mDynamicLightPrototypes.insert(std::make_pair(prototype->GetName(), prototype));
         }

      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::LightID RenderingSupportComponent::AddDynamicLight(DynamicLight* dl)
      {         
         if(dl != NULL)
         {
            mLights.push_back(dl);
            return dl->mID;
         }
         else
         {
            LOG_ERROR("Attempting to add a light that is NULL to the Rendering Support Component");
            return 0;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::DynamicLight *RenderingSupportComponent::AddDynamicLightByPrototypeName(const std::string &prototypeName)
      {
         DynamicLight *result = NULL;

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
            SimCore::Actors::DynamicLightPrototypeActor *dlActor = (SimCore::Actors::DynamicLightPrototypeActor *) iter->second.get()->GetActor();
            result = new DynamicLight();
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

            // Set the light type from the enum!
            mLights.push_back(result);
         }

         return result;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::RemoveDynamicLight(RenderingSupportComponent::LightID id)
      {
         mLights.erase(std::remove_if(mLights.begin(), mLights.end(), findLightById(id)));  
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::RemoveLight(LightArray::iterator iter)
      {
         mLights.erase(iter);
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
         
         return 0;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetNVGS(RenderFeature* rf)
      {
         if(rf != NULL)
         {
            mNVGS = rf;
            mNVGS->Init(mNVGSRoot.get(), GetGameManager()->GetApplication().GetCamera());         
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
         mEnableCullVisitor = pEnable;
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
      void RenderingSupportComponent::UpdateViewMatrix()
      {         
         osg::StateSet* ss = GetGameManager()->GetScene().GetSceneNode()->getOrCreateStateSet();
         osg::Uniform* viewUniform = ss->getOrCreateUniform("inverseViewMatrix", osg::Uniform::FLOAT_MAT4);
   
         osg::Matrix mat(GetGameManager()->GetApplication().GetCamera()->GetOSGCamera()->getViewMatrix());

         osg::Matrix viewInverse;
         viewInverse.invert(mat);
         viewUniform->set(viewInverse);         
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::ProcessMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }
         else if(msg.GetMessageType() == SimCore::MessageType::NIGHT_VISION)
         {
            if(mNVGS.valid())
            {
               SetEnableNVGS(static_cast<const SimCore::ToolMessage*>(&msg)->IsEnabled());            
            }
         }

         // When the map change begins, clear out our lists. 
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_CHANGE_BEGIN)
         {
            mLights.clear();
            mDynamicLightPrototypes.clear();
         }

         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            LoadPrototypes();
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

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::SetPosition(DynamicLight* dl)
      {
         if(dl != NULL && dl->mTarget.valid())
         {
            dtCore::Transform xform;
            dl->mTarget->GetTransform(xform);

            dl->mPosition = xform.GetTranslation();
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::ProcessTick(const dtGame::TickMessage &msg)
      {
         //we update the view matrix for all the shaders
         UpdateViewMatrix();

         if(mEnableNVGS && mNVGS.valid())
         {
            mNVGS->Update();
         }

         if(mEnableCullVisitor)
         {
            // sync up actors if needbe...
            UpdateCullVisitor();
         }

         if(mEnableDynamicLights)
         {             
           UpdateDynamicLights(float(static_cast<const dtGame::TickMessage&>(msg).GetDeltaSimTime()));
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      void RenderingSupportComponent::UpdateDynamicLights(float dt)
      {
         //now setup the lighting uniforms necessary for rendering the dynamic lights
         osg::StateSet* ss = GetGameManager()->GetScene().GetSceneNode()->getOrCreateStateSet();
//temporary hack
#ifdef __APPLE__
         static const std::string DYN_LIGHT_UNIFORM = "dynamicLights[0]";
#else
         static const std::string DYN_LIGHT_UNIFORM = "dynamicLights";
#endif
         osg::Uniform* lightArray = ss->getOrCreateUniform(DYN_LIGHT_UNIFORM, osg::Uniform::FLOAT_VEC4, MAX_LIGHTS * 3);

         LightArray::iterator iter = mLights.begin();
         LightArray::iterator endIter = mLights.end();

         //update lights, the extra check for !mLights.empty() keeps from crashing if we erase the last element
         for(;!mLights.empty() && iter != endIter; ++iter)
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

            if(dl->mTarget.valid())
            {
               //update the light's position
               SetPosition(dl);
            }
         }

         //now remove all flagged lights, note this is actually faster because we only have a single deallocation for N lights
         mLights.erase(std::remove_if(mLights.begin(), mLights.end(), removeLightsFunc()), mLights.end());

         //update uniforms by finding the closest lights to the camera
         dtCore::Transform trans;
         GetGameManager()->GetApplication().GetCamera()->GetTransform(trans);

         //sort the lights, though a heap may be more efficient here, we will sort so that we can combine lights later
         std::sort(mLights.begin(), mLights.end(), funcCompareLights(trans.GetTranslation()));

         unsigned count = 0;
         for(iter = mLights.begin(), endIter = mLights.end();count < MAX_LIGHTS * 3;)
         { 
            if(iter != endIter)
            {
               DynamicLight* dl = (*iter).get();

               //don't bind lights of zero intensity
               if(dl->mIntensity > 0.0001f)
               {
                  lightArray->setElement(count, osg::Vec4(dl->mPosition[0], dl->mPosition[1], dl->mPosition[2], dl->mIntensity));
                  lightArray->setElement(count + 1, osg::Vec4(dl->mColor[0], dl->mColor[1], dl->mColor[2], 1.0f));
                  lightArray->setElement(count + 2, osg::Vec4(dl->mAttenuation[0], dl->mAttenuation[1], dl->mAttenuation[2], 1.0f));
                  count += 3;
               }

               ++iter;
            }
            else
            {
               //else we turn the light off by setting the intensity to 0
               lightArray->setElement(count, osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
               lightArray->setElement(count + 1, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
               lightArray->setElement(count + 2, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
               count += 3;
            }
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

         std::vector<dtDAL::ActorProxy*> toFill;
         
         if(mCullVisitor->GetLandActor() == NULL)
         {
            GetGameManager()->FindActorsByName(SimCore::Actors::NxAgeiaTerraPageLandActor::DEFAULT_NAME, toFill);
            SimCore::Actors::NxAgeiaTerraPageLandActor* landActor = NULL;
            if(!toFill.empty())
            {
               landActor = dynamic_cast<SimCore::Actors::NxAgeiaTerraPageLandActor*>(toFill[0]->GetActor());
               mCullVisitor->SetLandActor(landActor);
            }
            toFill.clear();
         }

         if(mCullVisitor->GetTerrainNode() == NULL)
         {
            GetGameManager()->FindActorsByName("Terrain", toFill);
            if(!toFill.empty())
            {
               mCullVisitor->SetTerrainNode(toFill[0]->GetActor()->GetOSGNode()->asTransform());
            }
         }

         dtCore::Transform cameraTransform;
         GetGameManager()->GetApplication().GetCamera()->GetTransform(cameraTransform);
         mCullVisitor->SetCameraTransform(cameraTransform.GetTranslation());
         return true;
      }

   } // end entity namespace.
} // end dvte namespace.
