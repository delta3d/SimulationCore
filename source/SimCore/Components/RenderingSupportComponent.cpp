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
#include <prefix/dvteprefix-src.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>

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


namespace SimCore
{
   namespace Components
   {
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const std::string &RenderingSupportComponent::DEFAULT_NAME = "RenderingSupportComponent";      

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RenderingSupportComponent::RenderingSupportComponent(const std::string &name) 
         : dtGame::GMComponent(name)
         , mEnableCullVisitor(false)
         , mEnableNVGS(false)
         , mGUIRoot(new osg::CameraNode())
         , mNVGSRoot(new osg::CameraNode())
         , mNVGS(0)
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

         /*else if(msg.GetMessageType() == dtGame::MessageType::TICK_REMOTE){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_PUBLISHED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED){}*/
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
