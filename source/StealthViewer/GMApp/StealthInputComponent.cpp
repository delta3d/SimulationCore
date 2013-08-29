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
 * @author William E. Johnson II, Allen Danklefsen, Chris Rodgers, Curtiss Murphy
 */
#include <prefix/SimCorePrefix.h>
#include <dtUtil/mswin.h>
#include <SimCore/StealthMotionModel.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/BaseGameEntryPoint.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <StealthViewer/GMApp/StealthHUD.h>

#include <dtABC/application.h>
#include <dtDAL/actorproxy.h>
#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/transformableactorproxy.h>
#include <dtCore/shadermanager.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shaderparamfloat.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtCore/system.h>
#include <dtCore/mouse.h>
//#include <dtTerrain/terrain.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/AttachedMotionModel.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Tools/Tool.h>
#include <SimCore/Tools/Binoculars.h>
#include <SimCore/Tools/Compass.h>
#include <SimCore/Tools/LaserRangeFinder.h>
#include <SimCore/Tools/GPS.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Utilities.h>

#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <dtGame/gameactor.h>
#include <dtGame/binarylogstream.h>
#include <dtGame/logtag.h>
#include <dtGame/logkeyframe.h>
#include <dtGame/logstatus.h>
#include <dtGame/loggermessages.h>
#include <dtGame/logcontroller.h>
#include <dtGame/serverloggercomponent.h>
#include <dtGame/gameapplication.h>
#include <dtGame/basemessages.h>

#include <dtUtil/fileutils.h>

#include <SimCore/Actors/TerrainActorProxy.h>

#include <dtHLAGM/hlacomponent.h>

#include <osgDB/FileNameUtils>

#include <dtDAL/project.h>

#include <dtCore/view.h>

#include <iostream>

using dtCore::RefPtr;

namespace StealthGM
{
   const std::string StealthInputComponent::DEFAULT_NAME = "Input Component";

   /////////////////////////////////////////////////////////////////////////////////
   StealthInputComponent::StealthInputComponent(bool enableLogging,
                                                bool enablePlayback,
                                                const std::string& name,
                                                bool hasUI)
      : SimCore::Components::BaseInputComponent(name)
      , mCycleIndex(0)
      , mEnableLogging(enableLogging)
      , mEnablePlayback(enablePlayback)
      , mWasConnected(false)
      , mReconnectOnIdle(true)
      , mLoopContinuouslyInPlayback(false)
      , mTicksToLogStateChange(0)
      , mTicksToRestartPlayback(0)
      , mTargetLogState(&dtGame::LogStateEnumeration::LOGGER_STATE_IDLE)
      , mFirstPersonAttachMode(true)
      , mHasUI(hasUI)
      , mCollideWithGround(true)
      , mCountDownToPeriodicProcessing(1.0)
   {
      mMachineInfo = new dtGame::MachineInfo;
   }

   /////////////////////////////////////////////////////////////////////////////////
   StealthInputComponent::~StealthInputComponent()
   {
      mHLA = NULL;
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::OnAddedToGM()
   {
      // Attempt connection to the network
      if (!mHLA.valid() )
      {
         // NOTE: If the HLA component cannot be found after a fresh
         // compile, go to the BaseEntryPoint.cpp to verify the HLA
         // component's true name.
         mHLA = static_cast<dtHLAGM::HLAComponent*>
            (GetGameManager()->GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME));
         if (!mHLA.valid())
            GetLogger().LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Critical Error!  Stealth Input Component could not find an HLA Component!!!");
      }

      //SetListeners();
      SimCore::Components::BaseInputComponent::OnAddedToGM();
   }

   // TEST --- START
   dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mCurEntity;
   // TEST --- END

   ///////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& msgType = message.GetMessageType();

      // TICK LOCAL
      if (msgType == dtGame::MessageType::TICK_LOCAL)
      {
         // TEST --- START
         if (mCurEntity.valid() )
         {
            osg::Vec3 entityPos;
            dtCore::Transform xform;
            mCurEntity->GetTransform( xform );
            xform.GetTranslation( entityPos );
            //mHUDComponent->SetEntityLabelPosition( entityPos );
         }
         // TEST --- END

         const dtGame::TickMessage& tick = static_cast<const dtGame::TickMessage&>(message);
         UpdateTools( tick.GetDeltaSimTime() );
         HandlePeriodicProcessing(tick.GetDeltaSimTime());

         if (mTicksToLogStateChange > 0 )
         {
            mTicksToLogStateChange--;
            if (mTicksToLogStateChange == 0 )
            {
               ChangeAARState( *mTargetLogState );
            }
         }

         // A playback ended a few ticks ago, and we are in looping mode, so we are going to start over.  
         if (mTicksToRestartPlayback > 0)
         {
            mTicksToRestartPlayback--;

            // If still in playback, then jump to first keyframe and start over.
            if (mTicksToRestartPlayback == 0 && mLoopContinuouslyInPlayback && 
               mLogController->GetLastKnownStatus().GetStateEnum() == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
            {
               GotoFirstKeyframe();
            }
         }
      }
      // A Local Player entered world, so create our motion models
//      else if (msgType == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD &&
//         message.GetSource() == GetGameManager()->GetMachineInfo())
      else if (msgType == dtGame::MessageType::INFO_TIMER_ELAPSED && mTerrainActor.valid() &&
         message.GetAboutActorId() == mTerrainActor->GetId())
      {
         RefPtr<dtGame::GameActorProxy> stealthProxy = GetGameManager()->FindGameActorById(message.GetAboutActorId());
         if (!stealthProxy.valid())
         {
            GetLogger().LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "A player entered world message was received, but the about actor id does not refer to a Game Actor in the Game Manager.");
            return;
         }

         RefPtr<SimCore::Actors::StealthActor> stealthActor
            = static_cast<SimCore::Actors::StealthActor*>(&stealthProxy->GetGameActor());
         if (!stealthActor.valid())
         {
            GetLogger().LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "A player entered world message was received, but the actor is not the right type.");
         }
         else if (!stealthProxy->IsRemote())
         {
            if (GetStealthActor() == NULL)
            {
               SetStealthActor(stealthActor.get());
            }
            osg::Vec3 attachOffset = GetStealthActor()->GetAttachOffset();
            attachOffset.y() = -4.0f;
            GetStealthActor()->SetAttachOffset( attachOffset );
            GetStealthActor()->SetAttachAsThirdPerson(!mFirstPersonAttachMode);

            // Ensure that the compass tool has reference to the stealth actor
            SimCore::Tools::Compass* compass
               = dynamic_cast<SimCore::Tools::Compass*>(GetTool(SimCore::MessageType::COMPASS));
            if (compass != NULL)
               compass->SetPlayerActor( GetStealthActor() );

            std::vector<dtDAL::ActorProxy*> actors;

            const dtDAL::ActorType *type = GetGameManager()->FindActorType("dtcore", "Player Start");
            GetGameManager()->FindActorsByType(*type, actors);
            if (actors.empty())
            {
               if (mTerrainActor.valid() && message.GetAboutActorId() == mTerrainActor->GetId())
               {
                  SimCore::Actors::TerrainActor* tdraw;
                  mTerrainActor->GetDrawable(tdraw);
                  if (tdraw == NULL || tdraw->CheckForTerrainLoaded())
                  {
                     osg::Node* node = tdraw->GetOSGNode();
                     dtCore::Transform stealthStart;
                     node->computeBound();
                     osg::Vec3 startPos = node->getBound().center();
                     stealthStart.SetTranslation(startPos);
                     SimCore::Utils::KeepTransformOnGround(stealthStart, *tdraw, 100.0f, 5000.0f, 5000.0f);
                     GetStealthActor()->SetTransform(stealthStart);
                  }
               }
               else
               {
                  LOG_WARNING("Failed to find a player start proxy in the map. Defaulting to [0, 0, 0]");
               }
            }
            else
            {
               RefPtr<dtDAL::TransformableActorProxy> proxy = dynamic_cast<dtDAL::TransformableActorProxy*>(actors[0]);
               osg::Vec3 startPos = proxy->GetTranslation();
               dtCore::Transform stealthStart;
               stealthStart.SetTranslation(startPos);
               GetStealthActor()->SetTransform(stealthStart);
            }

            if (mLogController.valid())
            {
               mLogController->RequestAddIgnoredActor(GetStealthActor()->GetUniqueId());
            }

            dtABC::Application& app = GetGameManager()->GetApplication();
            dtCore::Camera& cam = *app.GetCamera();
            if (cam.GetParent() != NULL)
            {
               cam.Emancipate();
            }

            GetStealthActor()->AddChild(app.GetCamera());

            // create the fly motion model
            if (!mStealthMM.valid())
            {
               mStealthMM = new SimCore::StealthMotionModel(app.GetKeyboard(), app.GetMouse(), dtCore::FlyMotionModel::OPTION_DEFAULT);
               mStealthMM->SetCollideWithGround(mCollideWithGround);
               mStealthMM->SetUseSimTimeForSpeed(false);
            }
            mStealthMM->SetTarget(GetStealthActor());
            mStealthMM->SetScene(GetGameManager()->GetScene());

            std::vector<dtDAL::ActorProxy*> toFill;
            GetGameManager()->FindActorsByName(SimCore::Actors::TerrainActor::DEFAULT_NAME, toFill);
            if (!toFill.empty())
            {
               // Update the stealth actor with a reference to the terrain
               // with which it will collide.
               mStealthMM->SetCollidableGeometry(toFill[0]->GetDrawable());
            }
            mStealthMM->SetEnabled(GetStealthActor()->GetAttachAsThirdPerson());

            // create the attached motion model
            if (!mAttachedMM.valid())
            {
               mAttachedMM = new SimCore::AttachedMotionModel(app.GetKeyboard(), app.GetMouse());
               mAttachedMM->SetCenterMouse(false);
            }
            mAttachedMM->SetTarget(GetStealthActor());
            mAttachedMM->SetEnabled(!GetStealthActor()->GetAttachAsThirdPerson());

            // The HUD will need to access data from the motion model
            // so it can be displayed.
            StealthHUD *hud = GetHUDComponent();
            if (hud != NULL)
            {
               // Data should not be sent by messages because they
               // may be recorded during record mode.
               if (GetStealthActor()->GetAttachAsThirdPerson())
               {
                  hud->SetMotionModel(mStealthMM.get());
               }
               else
               {
                  hud->SetMotionModel(mAttachedMM.get());
               }
            }
         }
      }
      else if (dynamic_cast<const SimCore::ToolMessage*>(&message) != NULL)
      {
         // Act only on local tool messages because the messages could be sent from playback.
         if (mMachineInfo.valid() && *mMachineInfo == message.GetSource() )
         {
            if (GetStealthActor() != NULL
               && GetStealthActor()->GetUniqueId() == message.GetAboutActorId())
            {
               const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
               SetToolEnabled(
                  static_cast<SimCore::MessageType&>(const_cast<dtGame::MessageType&>(toolMsg.GetMessageType())),
                  toolMsg.GetEnabled());
            }
         }
      }
      else if (msgType == dtGame::MessageType::INFO_ACTOR_DELETED)
      {
         const dtCore::UniqueId& id = message.GetAboutActorId();
         if (mTerrainActor.valid() && mTerrainActor->GetId() == id)
         {
            mTerrainActor = NULL;
         }
      }

      // RESTARTED
      else if (msgType == dtGame::MessageType::INFO_RESTARTED)
      {
         // Setup the logger component
         if (mEnableLogging && !mLogController.valid())
         {
            mLogController = dynamic_cast<dtGame::LogController *> (GetGameManager()->
               GetComponentByName("LogController")); // "ServerLoggerComponent"
         }
      }
      // Logging server rejected a message request - print it.
      else if (mEnableLogging && msgType == dtGame::MessageType::SERVER_REQUEST_REJECTED)
      {
         std::string messageError;
         message.ToString(messageError);
         LOG_ALWAYS("   Reject Message[" + messageError + "].");
      }
      else if (msgType == dtGame::MessageType::INFO_MAP_UNLOADED)
      {
         mTerrainActor = NULL;
      }
      else if (msgType == dtGame::MessageType::INFO_MAP_LOADED)
      {
         dtGame::GameManager& gameManager = *GetGameManager();
         const dtGame::MapMessage& mlm = static_cast<const dtGame::MapMessage&>(message);
         dtGame::GameManager::NameVector mapNames;
         mlm.GetMapNames(mapNames);

         std::vector<dtDAL::ActorProxy*> actors;
         // TODO make a constant for this.
         gameManager.FindActorsByName("Terrain", actors);
         if (actors.empty())
         {
            LOG_ERROR("No terrain actor was found in the map: " + mapNames[0]);
         }
         else
         {
            mTerrainActor = actors[0];
            //inputComp->SetTerrainActor(*terrainActor);
            //std::string name = terrainActor->GetName();
            //inputComp->SetTerrainActorName(name);
            dtGame::GameActorProxy *gap = dynamic_cast<dtGame::GameActorProxy*>(mTerrainActor.get());
            if (gap == NULL)
            {
               LOG_ERROR("The terrain actor is not a game actor. Ignoring.");
            }
            else
            {
               // Ignore the terrain and the environment from recording
               if (mEnableLogging)
               {
                  if (mLogController.valid() )
                  {
                     if (mTerrainActor.valid())
                        mLogController->RequestAddIgnoredActor(mTerrainActor->GetId());

                     if (gameManager.GetEnvironmentActor() != NULL)
                        mLogController->RequestAddIgnoredActor(gameManager.GetEnvironmentActor()->GetId());
                  }
                  else
                  {
                     LOG_ERROR("LogController NOT found. Terrain actor CANNOT be ignored from logging.");
                  }
               }
            }
         }

         // Avoid deleting the Coordinate Config Actor
         actors.clear();
         const dtDAL::ActorType *type = gameManager.FindActorType("dtutil", "Coordinate Config");
         gameManager.FindActorsByType(*type, actors);
         if (!actors.empty() && mLogController.valid())
         {
            RefPtr<dtDAL::ActorProxy> proxy = actors[0];
            mLogController->RequestAddIgnoredActor(proxy->GetId());
         }

         if (GetStealthActor() == NULL || !mStealthActorProxy.valid())
         {
            dtCore::RefPtr<dtDAL::BaseActorObject> proxy;
            std::vector<dtDAL::ActorProxy*> proxies;

            gameManager.FindPrototypesByActorType(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, proxies);
            if (proxies.empty())
            {
               //create one by default
               proxy = gameManager.CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE);
            }
            else
            {
               proxy = gameManager.CreateActorFromPrototype(proxies[0]->GetId());
            }

            mStealthActorProxy = static_cast<SimCore::Actors::StealthActorProxy*> (proxy.get());

            SetStealthActor(static_cast<SimCore::Actors::StealthActor*>(mStealthActorProxy->GetDrawable()));
         
         }

         // Re-add the stealth actor to the game manager since a map unload
         // will have already removed it from the game manager. This assumes
         // load map has been called after an unload map procedure.
         if (mStealthActorProxy.valid() )
         {
            gameManager.AddActor( *mStealthActorProxy, false, false);
         }

         // After map is loaded, we could set the base elevation - default is 0, which is fine.
         //SimCore::Components::WeatherComponent* weatherComp = static_cast<SimCore::Components::WeatherComponent*>
         //   (GetGameManager()->GetComponentByName(SimCore::WeatherComponent::DEFAULT_NAME));
         //if (weatherComp != NULL)
         //   weatherComp->SetBaseElevation(600.0f);
      }

      else if (msgType == dtGame::MessageType::LOG_INFO_PLAYBACK_END_OF_MESSAGES)
      {
         HandleEndOfPlayback();
      }
      else if(msgType == SimCore::MessageType::REQUEST_WARP_TO_POSITION || 
               msgType == SimCore::MessageType::ATTACH_TO_ACTOR)
      {
         mStealthMM->ResetOffset();
      }
   }

   ////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandlePeriodicProcessing(float deltaTime)
   {
      mCountDownToPeriodicProcessing -= deltaTime;

      if (mCountDownToPeriodicProcessing < 0.0f)
      {
         // Slow down the camera based on how much view area we have.
         // Helps with binoculars and other FoV scale effects in Stealth View
         dtCore::Camera *camera = GetGameManager()->GetApplication().GetCamera();
         float avgFoV = 0.5f * (camera->GetHorizontalFov() + camera->GetVerticalFov());
         float cameraFoVScalar = (75.0f / avgFoV);
         float maxMouseTurnSpeed = 70.0f / cameraFoVScalar;

         if (mAttachedMM != NULL)
         {
            mAttachedMM->SetMaximumMouseTurnSpeed(maxMouseTurnSpeed); // old used to be 1440 before time was removed
            mAttachedMM->SetKeyboardTurnSpeed(maxMouseTurnSpeed);
         }
         if (mStealthMM != NULL)
         {
            mStealthMM->SetMaximumTurnSpeed(maxMouseTurnSpeed);
         }

         // Reset count down
         mCountDownToPeriodicProcessing = 1.0f;
      }

   }

   ////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::SetupLogging()
   {
      if (mEnableLogging)
      {
         mLogController = dynamic_cast<dtGame::LogController *> (GetGameManager()->
            GetComponentByName("LogController")); // "ServerLoggerComponent"
         if (mLogController.valid() && mEnablePlayback)
         {
            LOG_ALWAYS("Logging is enabled - beginning *PLAYBACK*");
            mLogController->RequestChangeStateToPlayback();
            mLogController->RequestServerGetKeyframes();
         }
         else
         {
            LOG_ALWAYS("Logging is enabled - beginning RECORD.");
            mLogController->RequestChangeStateToRecord();
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::HandleButtonPressed( const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button )
   {
      bool handled = false;

      osg::Vec3 point;
      if (button == dtCore::Mouse::LeftButton)
      {
         dtCore::View* view = GetGameManager()->GetApplication().GetView();
         dtCore::DeltaDrawable* dd = view->GetMousePickedObject();

         SimCore::Actors::BaseEntity* entity = dynamic_cast<SimCore::Actors::BaseEntity*>(dd);
         if (entity != NULL )
         {
            //std::cout << "\n\tEntity found!!!\n" << std::endl;
            mCurEntity = entity;

            /*osg::Vec3 entityPos;
            dtCore::Transform xform;
            entity->GetTransform( xform );
            xform.GetTranslation( entityPos );
            mHUDComponent->SetEntityLabelPosition( entityPos );*/
         }
      }

      if (!handled)
         return SimCore::Components::BaseInputComponent::HandleButtonPressed( mouse, button );

      return handled;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::HandleButtonReleased( const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button )
   {
      bool handled = false;

      if (! handled)
         return SimCore::Components::BaseInputComponent::HandleButtonReleased( mouse, button );

      return handled;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)
   {

      bool handled = true;
      switch(key)
      {
         case '=':
         {
            ChangeFlyMotionModelSpeed(true);
         }
         break;

         case '-':
         {
            ChangeFlyMotionModelSpeed(false);
         }
         break;

         case '\\':
         case osgGA::GUIEventAdapter::KEY_Insert:
         {
            std::string developerMode;
            developerMode = GetGameManager()->GetConfiguration().GetConfigPropertyValue
               (SimCore::BaseGameEntryPoint::CONFIG_PROP_DEVELOPERMODE, "false");
            if (developerMode == "true" || developerMode == "1")
            {
               GetGameManager()->GetApplication().SetNextStatisticsType();
            }
         }
         break;

         case '9':
         {
            dtCore::Transform xform;
            GetStealthActor()->GetTransform(xform);
            osg::Vec3 pos;
            xform.GetTranslation(pos);
            std::ostringstream oss;
            oss << "The player's position is: " << pos << '\n';
            LOG_ALWAYS(oss.str());
         }
         break;

         case ',':
         {
            Cycle(false, true);
         }
         break;

         case '.':
         {
            Cycle(true, true);
         }
         break;

         case 'p':
         {
            ReloadShadersIfDevMode();
         }
         break;

         case 'P':
         {
            SetNextPhysicsDebugDrawIfDevMode();
         }
         break;

         case 'l':
         {
            RefPtr<dtGame::Message> msg = GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
            SimCore::AttachToActorMessage &atamsg = static_cast<SimCore::AttachToActorMessage&>(*msg);
            atamsg.SetAboutActorId(GetStealthActor()->GetUniqueId());
            atamsg.SetSource(*mMachineInfo);

            if (GetStealthActor()->GetParent() != NULL)
            {
               atamsg.SetAttachToActor(dtCore::UniqueId(""));
            }
            else
            {
               atamsg.SetAttachToActor(mCurrentActorId);
            }

            GetGameManager()->SendMessage(atamsg);
         }
         break;

         case 'o':
         {
            IncrementTime(5);
         }
         break;

         case 'i':
         {
            IncrementTime(-5);
         }
         break;

         case 'm':
         {
            if (!mHasUI)
            {
               //swap motion model.
               bool fly = mStealthMM->IsEnabled();
               mStealthMM->SetEnabled(!fly);
               mAttachedMM->SetEnabled(fly);
               GetGameManager()->GetApplication().GetWindow()->SetShowCursor(!fly);
            }
         }
         break;

         case osgGA::GUIEventAdapter::KEY_F1:
            HandleHelpPressed();
            break;

         case osgGA::GUIEventAdapter::KEY_F2:
            if (GetHUDComponent() != NULL )
            {
               mHUDComponent->CycleToNextHUDState(); // already in help, so toggle it off
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F7:
            {
               ToggleTool(SimCore::MessageType::NIGHT_VISION);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F8:
            {
               ToggleTool(SimCore::MessageType::COMPASS);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F9:
            {
               SimCore::Tools::Tool* tool = GetTool(SimCore::MessageType::BINOCULARS);
               if (tool != NULL && tool->GetPlayerActor() == NULL)
               {
                  GetTool(SimCore::MessageType::BINOCULARS)->SetPlayerActor(GetStealthActor());
               }

               ToggleTool(SimCore::MessageType::BINOCULARS);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F10:
            {
               ToggleTool(SimCore::MessageType::LASER_RANGE_FINDER);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F11:
            {
               ToggleTool(SimCore::MessageType::GPS);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F12:
            {
               //ToggleTool(SimCore::MessageType::MAP);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_Shift_L:
         case osgGA::GUIEventAdapter::KEY_Shift_R:
            {
               if (GetStealthActor() != NULL
                        && !GetStealthActor()->IsRemote())
               {
                  SimCore::Tools::LaserRangeFinder *lrf = static_cast<SimCore::Tools::LaserRangeFinder*>(GetTool(SimCore::MessageType::LASER_RANGE_FINDER));
                  if (lrf == NULL || !lrf->IsEnabled())
                     break;

                  dtCore::Transform xform;
                  GetGameManager()->GetApplication().GetCamera()->GetTransform(xform);
                  lrf->FindIntersectionPoint(*mTerrainActor->GetDrawable(), xform);
               }
            }
            break;

         /////////////////////////////////
         // AAR PLAYBACK AND RECORD KEYS
         /////////////////////////////////

         // STOP A RECORD OR PLAYBACK
         case '1':
         {
            if (!mHasUI)
            {
               EnableIdle();
            }
         }
         break;

         // RECORD
         case '2':
         {
            if (!mHasUI)
            {
               EnableRecord();
            }
         }
         break;

         // PLAYBACK
         case '3':
         {
            if (!mHasUI)
            {
               EnablePlayback();
            }
         }
         break;

         // PAUSE THE PLAYBACK
         case '6':
         {
            if (!mHasUI)
            {
               HandlePause();
            }
         }
         break;

         // SPEED CHANGES FOR AAR PLAYBACK - SLOWER
         case '7':
         {
            if (!mHasUI)
            {
               HandleSpeedChange(false);
            }
         }
         break;

         // SPEED CHANGES FOR AAR PLAYBACK - FASTER
         case '8':
         {
            if (!mHasUI)
            {
               HandleSpeedChange(true);
            }
         }
         break;

         case '[':
         {
            if (!mHasUI)
            {
               HandleGotoKeyFrame(false);
            }
         }
         break;

         case ']':
         {
            if (!mHasUI)
            {
               HandleGotoKeyFrame(true);
            }
         }
         break;

         // Escaping or quitting when launched from Qt is BAD. Set flag so these
         // won't be processed by the base class
         case osgGA::GUIEventAdapter::KEY_Escape:
         {
            if (mHasUI)
            {
               handled = true;
            }
         }
         break;

         case 'x':
         {
            dtABC::Application &app = GetGameManager()->GetApplication();

            if (app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_L) ||
               app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_R))
            {
               if (mHasUI)
               {
                  handled = true;
               }
            }
         }
         break;

         default:
            // Implemented to get rid of warnings in Linux
            handled = false;
         break;
      }

      if (!handled)
         return SimCore::Components::BaseInputComponent::HandleKeyPressed(keyboard, key);

      return handled;
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ChangeAARState( const dtGame::LogStateEnumeration& targetState )
   {
      if (targetState == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK )
      {
         // DEBUG: std::cout << "StealthInputComponent::HandlePlayback" << std::endl;
         // Disconnect from the network if currently in IDLE mode
         if (mLogController->GetLastKnownStatus().GetStateEnum()
            == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE)
         {
            // Start playback
            mLogController->RequestChangeStateToPlayback();
            mLogController->RequestServerGetKeyframes();
         }
         // If playback is being set again, restart the playback from the beginning.
         else if (mLogController->GetLastKnownStatus().GetStateEnum()
            == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
         {
            // DEBUG: std::cout << "\tStealthInputComponent::GotoFirstKeyframe" << std::endl;
            GotoFirstKeyframe();
         }
         else // this must be RECORD state
         {
            mLogController->RequestChangeStateToIdle();
         }
      }
      else if (targetState == dtGame::LogStateEnumeration::LOGGER_STATE_RECORD )
      {
         // DEBUG: std::cout << "StealthInputComponent::HandleRecord" << std::endl;
         if (mLogController->GetLastKnownStatus().GetStateEnum()
            == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE)
         {
            mLogController->RequestChangeStateToRecord();
         }
         else if (mLogController->GetLastKnownStatus().GetStateEnum()
            == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
         {
            mLogController->RequestChangeStateToIdle();
            if (mReconnectOnIdle && mWasConnected ) { JoinFederation(); }
         }
      }
      else if (targetState == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE )
      {
         // DEBUG: std::cout << "StealthInputComponent::HandleIdle" << std::endl;
         mLogController->RequestChangeStateToIdle();

         if (mLogController->GetLastKnownStatus().GetStateEnum()
            == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
         {
            if (mReconnectOnIdle && mWasConnected ) { JoinFederation(); }
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::EnableIdle()
   {
      if (mEnableLogging && mLogController.valid())
      {
         mTargetLogState = &dtGame::LogStateEnumeration::LOGGER_STATE_IDLE;
         mTicksToLogStateChange = 2;
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::EnableRecord()
   {
      if (mEnableLogging && mLogController.valid())
      {
         mTargetLogState = &dtGame::LogStateEnumeration::LOGGER_STATE_RECORD;
         mTicksToLogStateChange = 2;
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ChangeFlyMotionModelSpeed(bool higher)
   {
      float val = mStealthMM->GetMaximumFlySpeed();
      mStealthMM->SetMaximumFlySpeed(higher ? val * 1.5 : val / 1.5);
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::EnablePlayback()
   {
      if (mEnableLogging && mLogController.valid())
      {
         // If this is currently IDLE mode, attempt disconnect.
         if (mLogController->GetLastKnownStatus().GetStateEnum()
            == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE)
         {
            // Do not do network activity during playback
            LeaveFederation();
         }

         mTargetLogState = &dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK;
         mTicksToLogStateChange = 2;
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandlePause()
   {
      if (mEnableLogging && mEnablePlayback)
      {
         GetGameManager()->SetPaused(!GetGameManager()->IsPaused());
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleSpeedChange(bool higherSpeed)
   {
      if (mEnableLogging && mEnablePlayback)
      {
         if (higherSpeed)
         {
            float speedFactor = GetGameManager()->GetTimeScale() * 1.20f;
            if (speedFactor <= 10.0f)
               GetGameManager()->ChangeTimeSettings(GetGameManager()->GetSimulationTime(),
               speedFactor, GetGameManager()->GetSimulationClockTime());
         }
         else
         {
            float speedFactor = GetGameManager()->GetTimeScale() * 0.8f;
            if (speedFactor >= 0.10f)
               GetGameManager()->ChangeTimeSettings(GetGameManager()->GetSimulationTime(),
               speedFactor, GetGameManager()->GetSimulationClockTime());
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::SetLoopContinuouslyInPlayback(bool newValue)
   {
      mLoopContinuouslyInPlayback = newValue;
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::GetLoopContinuouslyInPlayback()
   {
      return mLoopContinuouslyInPlayback;
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleSpeedChange(float newSpeed)
   {
      if (mEnableLogging && mEnablePlayback)
      {
         GetGameManager()->ChangeTimeSettings(GetGameManager()->GetSimulationTime(),
         newSpeed, GetGameManager()->GetSimulationClockTime());
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleGotoKeyFrame(bool nextKeyFrame)
   {
      // DEBUG: std::cout << "StealthInputComponent::HandleGotoKeyFrame" << std::endl;
      if (mEnableLogging && mEnablePlayback)
      {
         nextKeyFrame ? GotoNextKeyframe() : GotoPreviousKeyframe();
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleGotoKeyFrame(const std::string &name)
   {
      // DEBUG: std::cout << "StealthInputComponent::HandleGotoKeyFrame(" << name.c_str() << ")" << std::endl;
      if (mEnableLogging && mEnablePlayback)
      {
         const std::vector<dtGame::LogKeyframe> &KFs = mLogController->GetLastKnownKeyframeList();
         for(size_t i = 0; i < KFs.size(); i++)
         {
            if (KFs[i].GetName() == name)
            {
               mLogController->RequestJumpToKeyframe(KFs[i]);
               break;
            }
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleAddKeyFrame(const dtGame::LogKeyframe &kf)
   {
      if (mEnableLogging && mEnablePlayback)
      {
         mLogController->RequestCaptureKeyframe(kf);
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleGetKeyFrames()
   {
      if (mEnableLogging && mEnablePlayback)
      {
         mLogController->RequestServerGetKeyframes();
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleSetAutoKeyFrameInterval(double interval)
   {
      if (mEnableLogging && mEnablePlayback)
      {
         mLogController->RequestSetAutoKeyframeInterval(interval);
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleEndOfPlayback()
   {
      // Set a loop restart count down.
      if (mEnableLogging && mEnablePlayback && mLoopContinuouslyInPlayback)
      {
         mTicksToRestartPlayback = 5; // There's really no rush.
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   StealthHUD* StealthInputComponent::GetHUDComponent()
   {
      if (mHUDComponent == NULL)
      {
         /*StealthHUD *hudGUI = NULL;
         std::vector<GMComponent*> allComponents;

         GetGameManager()->GetAllComponents(allComponents);

         for (unsigned int i =0; i < allComponents.size(); i++)
         {
            hudGUI = dynamic_cast<StealthHUD *>(allComponents[i]);
            if (hudGUI != NULL)
               break;
         }

         mHUDComponent = hudGUI;*/
         dtGame::GMComponent *component = GetGameManager()->GetComponentByName(StealthHUD::DEFAULT_NAME);
         mHUDComponent = static_cast<StealthHUD*>(component);
      }

      return mHUDComponent.get();
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::HandleHelpPressed()
   {
      if (GetHUDComponent() != NULL )
      {
         if (SimCore::Components::HUDState::HELP == GetHUDComponent()->GetHUDState())
            mHUDComponent->CycleToNextHUDState(); // already in help, so toggle it off
         else
            mHUDComponent->SetHUDState(SimCore::Components::HUDState::HELP);
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ToggleEntityShaders()
   {
      dtCore::ShaderManager::GetInstance().ReloadAndReassignShaderDefinitions("Shaders/ShaderDefs.xml");

   }

   void StealthInputComponent::Cycle(bool forward, bool attach)
   {
      std::vector<dtDAL::ActorProxy*> actors;
      //GetGameManager()->GetAllActors(actors);
      GetGameManager()->GetAllActors(actors);
      //unsigned int size = actors.size();
      GetGameManager()->FindActorsByClassName("SimCore::Actors::BaseEntity", actors);
      if (actors.empty())
         return;

      if (mCycleIndex >= actors.size())
         mCycleIndex = mCycleIndex % actors.size();

      RefPtr<dtDAL::ActorProxy> ap = actors[mCycleIndex];
      if (ap->GetDrawable() != GetStealthActor() && ap != mTerrainActor &&
         ap.get() != GetGameManager()->GetEnvironmentActor())
      {
         RefPtr<dtGame::Message> nMsg = GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
         nMsg->SetAboutActorId(GetStealthActor()->GetUniqueId());

         SimCore::AttachToActorMessage* atamsg = dynamic_cast<SimCore::AttachToActorMessage*>(nMsg.get());

         if (attach)
         {
            mCurrentActorId = ap->GetId();
            atamsg->SetAttachToActor(mCurrentActorId);
         }
         else
         {
            //clear the object id so that the player will detach.
            atamsg->SetAttachToActor(dtCore::UniqueId(""));

            dtCore::Transform objectxform;
            dtCore::Transformable *t = dynamic_cast<dtCore::Transformable*>(ap->GetDrawable());
            if (t != NULL)
            {
               t->GetTransform(objectxform);
               float x, y, z;
               objectxform.GetTranslation(x, y, z);
               y -= 15;
               z += 3;
               GetStealthActor()->SetTransform(dtCore::Transform(x, y, z));
            }
            else
            {
               LOG_ERROR("The dynamic cast failed");
            }
         }
         //select the right motion model.
         //mFlyMM->SetEnabled(!attach);
         //mAttachedMM->SetEnabled(attach);
         //GetGameManager()->GetApplication().GetWindow()->ShowCursor(!attach);

         //send the attachment message.
         GetGameManager()->SendMessage(*atamsg);
      }
      if (forward)
         mCycleIndex = (mCycleIndex + 1) % actors.size();
      else
      {
         if (mCycleIndex > 0)
            mCycleIndex -= 1;
         else
            mCycleIndex = actors.size() - 1;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::GotoFirstKeyframe()
   {
      if (mEnableLogging && mEnablePlayback && mLogController.valid() )
      {
         const std::vector<dtGame::LogKeyframe> &frames = mLogController->GetLastKnownKeyframeList();

         if (frames.size() > 0)
         {
            mLogController->RequestJumpToKeyframe(frames[0]);
         }
         GetGameManager()->SetPaused(false);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::GotoPreviousKeyframe()
   {
      if (mEnableLogging && mEnablePlayback && mLogController.valid() )
      {
         const std::vector<dtGame::LogKeyframe> &frames = mLogController->GetLastKnownKeyframeList();
         const dtGame::LogKeyframe *prevFrame = NULL;

         for (int i = 0; i < (int) frames.size(); i ++)
         {
            // Get the oldest frame less than the current sim time.  Allow a 3 second grace period
            // so that we don't have the same problem you do on CD players
            if ((frames[i].GetSimTimeStamp() + 2.25) < GetGameManager()->GetSimulationTime() &&
               (prevFrame == NULL || frames[i].GetSimTimeStamp() > prevFrame->GetSimTimeStamp()))
            {
               prevFrame = &(frames[i]);
            }
         }

         // found one, so jump to keyframe
         if (prevFrame != NULL)
         {
            // DEBUG:
            /*std::ostringstream ss;
            ss << "## Attempting to Jump to Previous Keyframe: [" << prevFrame->GetName() << "] ##";
            std::cout << ss.str() << std::endl;*/

            mLogController->RequestJumpToKeyframe(*prevFrame);
         }
         GetGameManager()->SetPaused(false);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::GotoNextKeyframe()
   {
      if (mEnableLogging && mEnablePlayback && mLogController.valid() )
      {
         const std::vector<dtGame::LogKeyframe> &frames = mLogController->GetLastKnownKeyframeList();
         const dtGame::LogKeyframe *nextFrame = NULL;

         for (int i = 0; i < (int) frames.size(); i ++)
         {
            // Get the first frame older than the current sim time.
            if (frames[i].GetSimTimeStamp() > GetGameManager()->GetSimulationTime() &&
               (nextFrame == NULL || frames[i].GetSimTimeStamp() < nextFrame->GetSimTimeStamp()))
            {
               nextFrame = &(frames[i]);
            }
         }

         // found one, so jump to keyframe
         if (nextFrame != NULL)
         {
            // DEBUG:
            /*std::ostringstream ss;
            ss << "## Attempting to Jump to Next Keyframe: [" << nextFrame->GetName() << "] ##";
            std::cout << ss.str() << std::endl;*/

            mLogController->RequestJumpToKeyframe(*nextFrame);
         }
         GetGameManager()->SetPaused(false);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::SetConnectionParameters(const std::string &executionName,
                                                       const std::string &fedFile,
                                                       const std::string &federateName,
                                                       const std::string &ridFile,
                                                       const std::string& rtiImplementation)
   {
      mExecutionName = executionName;
      mFedFile = fedFile;
      mFederateName = federateName;
      mRidFile = ridFile;
      mRtiImplementation = rtiImplementation;
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::JoinFederation( bool updateSystem )
   {
      // If not connected, attempt a connection.
      // This check avoids multiple calls to connection
      // such as in the case of input flooding.
      if (! IsConnected() )
      {
         if (updateSystem )
         {
            // Let the ServerLoggerComponent and LogController components
            // synchronize before attempting connection.
            //This is bad.  we can do this another way.
            //dtCore::System::GetInstance().Step();
         }

         // Clear any munitions or particle effects that may have been around BEFORE we joined.
         // This is necessary to stop problems with lingering effects during and after playback
         SimCore::Components::TimedDeleterComponent *deleterComp =
            dynamic_cast<SimCore::Components::TimedDeleterComponent*> (GetGameManager()->
            GetComponentByName(SimCore::Components::TimedDeleterComponent::DEFAULT_NAME));
         if (deleterComp != NULL)
            deleterComp->Clear();

         if (mHLA.valid() )
         {
            mHLA->JoinFederationExecution(mExecutionName, mFedFile, mFederateName, mRidFile, mRtiImplementation);
         }
         else
         {
            LOG_ERROR("HLA Component cannot be accessed. Attempted to join federation.");
         }

         if (updateSystem )
         {
            GetGameManager()->SetPaused(false);
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::LeaveFederation()
   {
      mWasConnected = IsConnected();

      // If connected, disconnect from the network. This check
      // prevents input flooding from calling disconnect
      // too many times; this needs to be called only once.
      if (mWasConnected )
      {
         // Attempt the disconnect from the network
         if (mHLA.valid() )
         {
            // Clear any munitions or particle effects that may have been around BEFORE we joined.
            // This is necessary to stop problems with lingering effects during and after playback
            SimCore::Components::TimedDeleterComponent *deleterComp =
               dynamic_cast<SimCore::Components::TimedDeleterComponent*> (GetGameManager()->
               GetComponentByName(SimCore::Components::TimedDeleterComponent::DEFAULT_NAME));
            if (deleterComp != NULL)
               deleterComp->Clear();

            mHLA->LeaveFederationExecution();
         }
         else
         {
            LOG_ERROR("HLA Component cannot be accessed. Attempted to leave federation.\
                      Application may still be connected.");
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ToggleTool(SimCore::MessageType& msgType)
   {
      if (GetStealthActor() == NULL
         || GetStealthActor()->IsRemote() )
      {
         return;
      }

      // Create the tool message to be circulated.
      dtCore::RefPtr<dtGame::Message> msg = GetGameManager()->GetMessageFactory().CreateMessage(msgType);
      SimCore::Tools::Tool *tool = GetTool(msgType);
      if (tool == NULL)
      {
         LOG_ERROR("Received a tool message from a tool the player does not have");
         return;
      }

      // Fill out the message with important data
      bool enable = !tool->IsEnabled();

      if (enable )
      {
         DisableAllTools();
      }

      static_cast<SimCore::ToolMessage*>(msg.get())->SetEnabled(enable);
      msg->SetAboutActorId(GetStealthActor()->GetUniqueId());
      msg->SetSource(*mMachineInfo);

      GetGameManager()->SendMessage(*msg);

   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::DisableAllTools()
   {
      SimCore::Tools::Tool* nvg = GetTool(SimCore::MessageType::NIGHT_VISION);

      // Turn off all other tools (this could be NO_TOOL)
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i;
      for(i = mToolList.begin(); i != mToolList.end(); ++i)
      {
         if (i->second->IsEnabled())
         {
            if (nvg != NULL && i->second == nvg )
            {
               dtCore::RefPtr<dtGame::Message> nvgMsg
                  = GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::NIGHT_VISION);
               static_cast<SimCore::ToolMessage*>(nvgMsg.get())->SetEnabled(false);
               nvgMsg->SetAboutActorId(GetStealthActor()->GetUniqueId());
               nvgMsg->SetSource(*mMachineInfo);
               GetGameManager()->SendMessage(*nvgMsg);
            }

            i->second->Enable(false);
         }
      }

      // Note - The motion model turn speeds was moved to HandlePeriodicProcessing()
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::AddTool(SimCore::Tools::Tool &tool, SimCore::MessageType &type)
   {
      if (!SimCore::MessageType::IsValidToolType(type))
      {
         LOG_ERROR("Tried to add a tool with an invalid type");
         return;
      }
      if (mToolList.find(&type) == mToolList.end())
         mToolList.insert(std::make_pair(&type, RefPtr<SimCore::Tools::Tool>(&tool)));
      else
         LOG_ERROR("AddTool tried to add a tool more than once.");
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::RemoveTool(SimCore::MessageType &type)
   {
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i =  mToolList.find(&type);
      if (i == mToolList.end())
      {
         LOG_ERROR("RemoveTool tried to remove a tool that doesn't exist");
         return;
      }
      mToolList.erase(i);
   }

   /////////////////////////////////////////////////////////////////////////////////
   SimCore::Tools::Tool* StealthInputComponent::GetTool(SimCore::MessageType &type)
   {
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i =
         mToolList.find(&type);

      return i == mToolList.end() ? NULL : i->second.get();
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::IsToolEnabled(SimCore::MessageType &type) const
   {
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::const_iterator i =
         mToolList.find(&type);

      return i == mToolList.end() ? false : i->second->IsEnabled();
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::UpdateTools( float timeDelta )
   {
      SimCore::Tools::Compass *compass = static_cast<SimCore::Tools::Compass*>(GetTool(SimCore::MessageType::COMPASS));
      if (compass == NULL)
      {
         LOG_DEBUG("The player actor currently does not have a compass");

      }
      else
      {
   	   compass->Update( timeDelta );
      }

      SimCore::Tools::GPS *gps = static_cast<SimCore::Tools::GPS*>(GetTool(SimCore::MessageType::GPS));
      if (gps == NULL)
      {
         LOG_DEBUG("The player actor currently does not have a GPS");
      }
      else
         gps->Update();
   }

   /////////////////////////////////////////////////////////////////////////////////
   SimCore::MessageType& StealthInputComponent::GetEnabledTool() const
   {
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::const_iterator i;
      for(i = mToolList.begin(); i != mToolList.end(); ++i)
      {
         if (i->second->IsEnabled())
            return *i->first;
      }
      return SimCore::MessageType::NO_TOOL;
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::SetToolEnabled(SimCore::MessageType& toolType, bool enable)
   {
      SimCore::Tools::Tool* tool = GetTool(toolType);

      if (tool != NULL )
      {
         if (enable != tool->IsEnabled() )
         {
            tool->Enable(enable);
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::IsInPlayback() const
   {
      return mEnablePlayback && mLogController.valid()
         && mLogController->GetLastKnownStatus().GetStateEnum()
         == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK;
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::IsInRecord() const
   {
      return mEnableLogging && mLogController.valid()
         && mLogController->GetLastKnownStatus().GetStateEnum()
         == dtGame::LogStateEnumeration::LOGGER_STATE_RECORD;
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::IsIdle() const
   {
      return (!mEnableLogging && !mEnablePlayback)
         || ( mLogController.valid()
         && mLogController->GetLastKnownStatus().GetStateEnum()
         == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE );
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::SetLogFileName(const std::string &fileName)
   {
      if (mLogController.valid())
      {
         std::string file = osgDB::getStrippedName(fileName);
         dtUtil::FileInfo info = dtUtil::FileUtils::GetInstance().GetFileInfo(fileName);

         mLogController->RequestSetLogFile(file);

         dtGame::ServerLoggerComponent *serverComp = NULL;
         GetGameManager()->GetComponentByName(dtGame::ServerLoggerComponent::DEFAULT_NAME, serverComp);
         if (serverComp == NULL)
         {
            LOG_ERROR("ERROR: The ServerLoggerComponent was not found.");
            return;
         }
         if (!info.path.empty())
         {
            LOG_DEBUG("Setting the record log directory to:" + info.path);
            serverComp->SetLogDirectory(info.path);
         }
         else
         {
            LOG_DEBUG("Settings the record log directory to the current working directory. \
                      This may or may not cause unpredictable behavior.");
            serverComp->SetLogDirectory(".");
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ChangeMotionModels(bool firstPerson)
   {
      mFirstPersonAttachMode = firstPerson;

      StealthHUD *hud = GetHUDComponent();

       if (firstPerson)
       {
         if (!mAttachedMM.valid())
            return;

         // Already enabled correctly. Peace out.
         if (mAttachedMM->IsEnabled())
            return;

         mStealthMM->SetTarget(NULL);
         mStealthMM->SetEnabled(false);

         mAttachedMM->SetTarget(GetStealthActor());
         mAttachedMM->SetEnabled(true);

         GetStealthActor()->SetAttachAsThirdPerson(false);

         if (hud != NULL)
         {
            // Data should not be sent by messages because they
            // may be recorded during record mode.
            hud->SetMotionModel(mAttachedMM.get());
         }
      }
      else
      {
         if (!mStealthMM.valid())
            return;

         // Already enabled correctly. Peace out.
         if (mStealthMM->IsEnabled())
            return;

         mAttachedMM->SetTarget(NULL);
         mAttachedMM->SetEnabled(false);

         mStealthMM->SetTarget(GetStealthActor());
         mStealthMM->SetEnabled(true);

         GetStealthActor()->SetAttachAsThirdPerson(true);

         if (hud != NULL)
         {
            // Data should not be sent by messages because they
            // may be recorded during record mode.
            hud->SetMotionModel(mStealthMM.get());
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::EnableCameraCollision(bool enable)
   {
      mCollideWithGround = enable;

      if (mStealthMM.valid())
         mStealthMM->SetCollideWithGround(mCollideWithGround);
   }

   //////////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::IsConnected() const
   {
      if (!mHLA.valid())
      {
         LOG_WARNING("HLA component is not accessible.");
      }
      return mHLA.valid() && mHLA->IsConnectedToFederation();
   }
}
