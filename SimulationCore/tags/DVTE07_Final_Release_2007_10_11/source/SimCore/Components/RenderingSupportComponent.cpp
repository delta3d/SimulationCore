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
* @author Allen Danklefsen, Bradley Anderegg
*/
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
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

#include <osg/CameraNode>
#include <osg/Geode>
#include <osg/Texture>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/StateSet>


namespace SimCore
{
   namespace Components
   {


      class UpdateViewCallback: public osg::NodeCallback
      {
      public:
         UpdateViewCallback(RenderingSupportComponent* mp):mRenderComp(mp){}

         void operator()(osg::Node*, osg::NodeVisitor* nv)
         {
            if(osg::CameraNode* cn = dynamic_cast<osg::CameraNode*>(nv->getNodePath()[0]))
            {
               mRenderComp->UpdateViewMatrix(cn->getViewMatrix());
            }
         }
      private:
         RenderingSupportComponent* mRenderComp;
      };



      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const std::string &RenderingSupportComponent::DEFAULT_NAME = "RenderingSupportComponent";      

      RenderingSupportComponent::LightID RenderingSupportComponent::DynamicLight::mLightCounter = 0;

      const unsigned RenderingSupportComponent::MAX_LIGHTS = 20;

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::RenderingSupportComponent(const std::string &name) 
         : dtGame::GMComponent(name)
         , mEnableDynamicLights(true)
         , mEnableCullVisitor(false)
         , mEnableNVGS(false)
         , mGUIRoot(new osg::CameraNode())
         , mNVGSRoot(new osg::CameraNode())
         , mNVGS(0)
         , mViewCallback(0)
      {
         ////if this is in the initializer list VS complains
         ////using this in initializer list is the warning
         mViewCallback = new UpdateViewCallback(this);
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

      void RenderingSupportComponent::SetGUI(dtCore::DeltaDrawable* gui)
      {
         mGUIRoot->addChild(gui->GetOSGNode());
      }

      void RenderingSupportComponent::InitializeFrameBuffer()
      {
         dtCore::RefPtr<osg::Group> grp = new osg::Group();
         GetGameManager()->GetApplication().GetScene()->SetSceneNode(mNVGSRoot.get());
         GetGameManager()->GetApplication().GetCamera()->GetSceneHandler()->GetSceneView()->setSceneData(grp.get());

         grp->addChild(mNVGSRoot.get());
         grp->addChild(mGUIRoot.get());
                  
         mNVGSRoot->setRenderOrder(osg::CameraNode::NESTED_RENDER);      
         mGUIRoot->setRenderOrder(osg::CameraNode::POST_RENDER);
         mGUIRoot->setClearMask( GL_NONE );
      }

      void RenderingSupportComponent::InitializeCullVisitor()
      {        
         osgUtil::SceneView* sceneView = GetGameManager()->GetApplication().GetCamera()->GetSceneHandler()->GetSceneView();
         mCullVisitor = new SimCore::AgeiaTerrainCullVisitor();
         mCullVisitor->setRenderStage(sceneView->getRenderStage());
         mCullVisitor->setStateGraph(sceneView->getStateGraph());

         int flags = osgUtil::CullVisitor::ENABLE_ALL_CULLING;

         // probably only need to set the scene veiw, and it 
         // auto sets the cull visitor but havent checked yet.
         mCullVisitor->setCullingMode(flags);
         sceneView->setCullingMode(flags);

         sceneView->setCullVisitor(mCullVisitor.get());
      }


      bool RenderingSupportComponent::GetEnableNVGS()
      {
         return mEnableNVGS;
      }

      void RenderingSupportComponent::SetEnableNVGS(bool pEnable)
      {
         if(mNVGS.valid())
         {
            mEnableNVGS = pEnable;
            mNVGS->SetEnable(mEnableNVGS);
         }                  
      }

      RenderingSupportComponent::LightID RenderingSupportComponent::AddDynamicLight(DynamicLight* dl)
      {         
         mLights.insert(std::make_pair(dl->mID, dl));
         return dl->mID;
      }

      void RenderingSupportComponent::RemoveDynamicLight(RenderingSupportComponent::LightID id)
      {
         ID_To_Light_Map::iterator iter = mLights.find(id);
         if(iter != mLights.end())
         {
            mLights.erase(iter);
         }
      }

      RenderingSupportComponent::DynamicLight* RenderingSupportComponent::GetDynamicLight(RenderingSupportComponent::LightID id)
      {
         ID_To_Light_Map::iterator iter = mLights.find(id);
         if(iter != mLights.end())
         {
            return (*iter).second.get();
         }
         return 0;
      }

      void RenderingSupportComponent::SetNVGS(RenderFeature* rf)
      {
         if(rf != NULL)
         {
            mNVGS = rf;
            mNVGS->Init(mNVGSRoot.get(), GetGameManager()->GetApplication().GetCamera());         
         }
      }

      const RenderingSupportComponent::RenderFeature* RenderingSupportComponent::GetNVGS() const
      {
         return mNVGS.get();
      }

      bool RenderingSupportComponent::GetEnableCullVisitor()
      {
         return mEnableCullVisitor;
      }

      void RenderingSupportComponent::SetEnableCullVisitor(bool pEnable)
      {
         mEnableCullVisitor = pEnable;
      }

      void RenderingSupportComponent::SetEnableDynamicLights(bool b)
      {
         mEnableDynamicLights = b;
      }
      bool RenderingSupportComponent::GetEnableDynamicLights() const
      {
         return mEnableDynamicLights;
      }

      void RenderingSupportComponent::UpdateViewMatrix(const osg::Matrix& mat)
      {
         osg::StateSet* ss = GetGameManager()->GetScene().GetSceneNode()->getOrCreateStateSet();
         osg::Uniform* viewUniform = ss->getOrCreateUniform("inverseViewMatrix", osg::Uniform::FLOAT_MAT4);

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

         else if( msg.GetMessageType() == SimCore::MessageType::INFO_ACTOR_CREATED
            )//|| msg.GetMessageType() == dvte::IG::MessageType::INFO_ACTOR_UPDATED )
         {
            const dtGame::ActorUpdateMessage& updateMsg = static_cast<const dtGame::ActorUpdateMessage&>(msg);
            //this should probably be done within the munitions comp or something 
            if(updateMsg.GetActorType() == SimCore::Actors::EntityActorRegistry::FLARE_ACTOR_TYPE)
            {
               dtDAL::ActorProxy* proxy = dynamic_cast<dtDAL::ActorProxy*>(GetGameManager()->FindActorById( updateMsg.GetAboutActorId()));
               if(proxy != NULL)
               {
                  //to make an illumination round dynamic light we must note that
                  //these are dropped at 600meters and will light the ground directly below within 
                  //a radius of 1km
                  SimCore::Actors::FlareActor* flare = dynamic_cast<SimCore::Actors::FlareActor*>(proxy->GetActor());
                  if(flare != NULL)
                  {
                     DynamicLight* dl = new DynamicLight();
                     dl->mSaturationIntensity = 1.0f;
                     dl->mIntensity = 1.0f;//flare->GetSourceIntensity();
                     dl->mColor.set(osg::Vec3(1.0f, 1.0f, 1.0f));//flare->
                     dl->mAttenuation.set(0.1, 0.005, 0.00002);
                     dl->mTarget = flare;

                     AddDynamicLight(dl);
                  }
               }
            }
         }
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            GetGameManager()->GetApplication().GetCamera()->GetOSGNode()->setUpdateCallback(mViewCallback.get());
         }

         /*else if(msg.GetMessageType() == dtGame::MessageType::TICK_REMOTE){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_PUBLISHED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED){}*/
      }

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
            //now setup the lighting uniforms necessary for rendering the dynamic lights
            osg::StateSet* ss = GetGameManager()->GetScene().GetSceneNode()->getOrCreateStateSet();
            osg::Uniform* lightArray = ss->getOrCreateUniform("dynamicLights", osg::Uniform::FLOAT_VEC4, MAX_LIGHTS * 3);
            
            ID_To_Light_Map::iterator iter = mLights.begin();
            ID_To_Light_Map::iterator endIter = mLights.end();

            //update uniforms
            int count = 0;
            for(;count < MAX_LIGHTS * 3; count += 3)
            { 
               if(iter != endIter)
               {
                  DynamicLight* dl = (*iter).second.get();

                  if(dl->mTarget.valid())
                  {
                     SetPosition(dl);
                  }

                  lightArray->setElement(count, osg::Vec4(dl->mPosition[0], dl->mPosition[1], dl->mPosition[2], dl->mIntensity));
                  lightArray->setElement(count + 1, osg::Vec4(dl->mColor[0], dl->mColor[1], dl->mColor[2], 1.0f));
                  lightArray->setElement(count + 2, osg::Vec4(dl->mAttenuation[0], dl->mAttenuation[1], dl->mAttenuation[2], dl->mSaturationIntensity));
               
                  ++iter;
               }
               else
               {
                  //else we turn the light off by setting the intensity to 0
                  lightArray->setElement(count, osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
                  lightArray->setElement(count + 1, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
                  lightArray->setElement(count + 2, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
               }
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
            GetGameManager()->FindActorsByName(NxAgeiaTerraPageLandActor::DEFAULT_NAME, toFill);
            NxAgeiaTerraPageLandActor* landActor = NULL;
            if(!toFill.empty())
            {
               landActor = dynamic_cast<NxAgeiaTerraPageLandActor*>(toFill[0]->GetActor());
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