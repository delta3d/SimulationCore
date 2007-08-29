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
 * @author William E. Johnson II, Allen Danklefsen, Chris Rodgers, Curtiss Murphy
 */
#include <SimCore/StealthMotionModel.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
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
#include <dtCore/system.h>
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

#include <SimCore/Actors/TerrainActorProxy.h>

#include <dtHLAGM/hlacomponent.h>

#ifdef AGEIA_PHYSICS
   #include <NxAgeiaWorldComponent.h>
   #include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
   #include <NxAgeiaMaterialActor.h>
   #include <SimCore/Actors/NxAgeiaParticleSystemActor.h>
   #include <dtDAL/project.h>
#endif

using dtCore::RefPtr;

namespace StealthGM
{
   const std::string &StealthInputComponent::DEFAULT_NAME = "Input Component";

   StealthInputComponent::StealthInputComponent(bool enableLogging,
                                                bool enablePlayback, 
                                                const std::string& name, 
                                                bool hasUI) :
      SimCore::Components::BaseInputComponent(name),
      mCycleIndex(0),
      mEnableLogging(enableLogging),
      mEnablePlayback(enablePlayback), 
      mWasConnected(false),
      mReconnectOnIdle(true),
      mTargetLogState(&dtGame::LogStateEnumeration::LOGGER_STATE_IDLE),
      mFirstPersonAttachMode(true), 
      mHasUI(hasUI), 
      mCollideWithGround(true),
      mEnvUpdateTime(0.0f),
      mEnvUpdateAttempts(2)
   {
      mLogger = &dtUtil::Log::GetInstance("StealthInputComponent.cpp");
      mMachineInfo = new dtGame::MachineInfo;
   }
   
   StealthInputComponent::~StealthInputComponent()
   {
      mHLA = NULL;
   }

   void StealthInputComponent::OnAddedToGM()
   {
      // Attempt connection to the network
      if( !mHLA.valid() )
      {
         // NOTE: If the HLA component cannot be found after a fresh
         // compile, go to the BaseEntryPoint.cpp to verify the HLA
         // component's true name.
         mHLA = static_cast<dtHLAGM::HLAComponent*>
            (GetGameManager()->GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME));
      }

      //SetListeners();
      SimCore::Components::BaseInputComponent::OnAddedToGM();
   }
   
   ///////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& msgType = message.GetMessageType();
   
      // TICK LOCAL
      if (msgType == dtGame::MessageType::TICK_LOCAL)
      {
         const dtGame::TickMessage& tick = static_cast<const dtGame::TickMessage&>(message);
         UpdateTools();

         if( mTicksToLogStateChange > 0 )
         {
            mTicksToLogStateChange--;
            if( mTicksToLogStateChange == 0 )
            {
               ChangeAARState( *mTargetLogState );
            }
         }

         // Update the view height distance - helps the fog decay nicely based on height.
         // Note - used to do this only a few times per second, but it causes jumpiness and
         // it is noticeable. Should be a fairly small hit anyway
         SimCore::Components::WeatherComponent* weatherComp = static_cast<SimCore::Components::WeatherComponent*>
            (GetGameManager()->GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME));
         if(weatherComp != NULL && mStealthMM.valid())
         {
            weatherComp->SetViewElevation(mStealthMM->GetElevation());

            // HACK: Force the time actor values to be applied to the ephemeris rendering.
            // The ephemeris updating is suspected to be asynchronous--the probable cause
            // for the time-of-day issue (sky does not match the time).
            if( mEnvUpdateAttempts > 0 )
            {
               if( mEnvUpdateTime >= 2.0f ) // time for update
               {
                  if( (mEnvUpdateAttempts%2) == 1 ) // time back
                  {
                     IncrementTime(-5);
                  }
                  else // time forward
                  {
                     IncrementTime(5);
                  }

                  mEnvUpdateTime = 0.0f; // reset clock for another attempt
                  --mEnvUpdateAttempts; // one attempt was burned
               }
               else // advance time toward next update
               {
                  mEnvUpdateTime += tick.GetDeltaRealTime();
               }
            }
         }
      }

      // A Local Player entered world, so create our motion models
      else if (msgType == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD && 
         message.GetSource() == GetGameManager()->GetMachineInfo())
      {
         RefPtr<dtGame::GameActorProxy> stealthProxy = GetGameManager()->FindGameActorById(message.GetAboutActorId());
         if (!stealthProxy.valid())
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "A player entered world message was received, but the about actor id does not refer to a Game Actor in the Game Manager.");
            return;
         }
   
         RefPtr<SimCore::Actors::StealthActor> stealthActor
            = static_cast<SimCore::Actors::StealthActor*>(&stealthProxy->GetGameActor());
         if (!stealthActor.valid())
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "A player entered world message was received, but the actor is not the right type.");
         }
         else if(!stealthProxy->IsRemote())
         {
            mStealthActor = stealthActor.get();
            mStealthActor->SetAttachAsThirdPerson(!mFirstPersonAttachMode);

            // Ensure thatthe compass tool has reference to the stealth actor
            SimCore::Tools::Compass* compass
               = dynamic_cast<SimCore::Tools::Compass*>(GetTool(SimCore::MessageType::COMPASS));
            if(compass != NULL) 
               compass->SetPlayerActor( mStealthActor.get() );

            std::vector<dtDAL::ActorProxy*> actors;

            const dtDAL::ActorType *type = GetGameManager()->FindActorType("dtcore", "Player Start");
            GetGameManager()->FindActorsByType(*type, actors);
            if(actors.empty())
            {
               LOG_WARNING("Failed to find a player start proxy in the map. Defaulting to [0, 0, 0]");
            }
            else
            {
               RefPtr<dtDAL::TransformableActorProxy> proxy = dynamic_cast<dtDAL::TransformableActorProxy*>(actors[0]);
               osg::Vec3 mStartPos = proxy->GetTranslation();
               dtCore::Transform stealthStart;
               stealthStart.SetTranslation(mStartPos);
               mStealthActor->SetTransform(stealthStart);
            }

            if(mLogController.valid())
            {
               mLogController->RequestAddIgnoredActor(mStealthActor->GetUniqueId());
            }

            dtABC::Application& app = GetGameManager()->GetApplication();
            dtCore::Camera& cam = *app.GetCamera();
            if (cam.GetParent() != NULL)
            {
               cam.Emancipate();
            }
            else if (cam.GetSceneParent() != NULL)
            {
               cam.GetSceneParent()->RemoveDrawable(&cam);
            }
               
            mStealthActor->AddChild(app.GetCamera());
   
            // create the fly motion model 
            if(!mStealthMM.valid())
            {
               mStealthMM = new SimCore::StealthMotionModel(app.GetKeyboard(), app.GetMouse(), false);
               mStealthMM->SetCollideWithGround(mCollideWithGround);
            }            
            mStealthMM->SetTarget(mStealthActor.get());
            mStealthMM->SetScene(GetGameManager()->GetScene());
            
            std::vector<dtDAL::ActorProxy*> toFill;
            GetGameManager()->FindActorsByName(SimCore::Actors::TerrainActor::DEFAULT_NAME, toFill);
            if(!toFill.empty())
            {
               // Update the stealth actor with a reference to the terrain
               // with which it will collide.
               mStealthMM->SetCollidableGeometry(toFill[0]->GetActor());
            }
            mStealthMM->SetEnabled(mStealthActor->GetAttachAsThirdPerson());
   
            // create the attached motion model
            if(!mAttachedMM.valid())
            {
               mAttachedMM = new SimCore::AttachedMotionModel(app.GetKeyboard(), app.GetMouse());
               mAttachedMM->SetCenterMouse(false);
            }
            mAttachedMM->SetTarget(mStealthActor.get());
            mAttachedMM->SetEnabled(!mStealthActor->GetAttachAsThirdPerson());

            // The HUD will need to access data from the motion model
            // so it can be displayed. 
            StealthHUD *hud = GetHUDComponent();
            if(hud != NULL)
            {
               // Data should not be sent by messages because they
               // may be recorded during record mode.
               if(mStealthActor->GetAttachAsThirdPerson())
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
         if (mStealthActor.valid() && mStealthActor->GetUniqueId() == message.GetAboutActorId())
         {
            const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
            SetEnabledTool(toolMsg.IsEnabled() ? 
               static_cast<SimCore::MessageType&>(const_cast<dtGame::MessageType&>(toolMsg.GetMessageType())) 
               : SimCore::MessageType::NO_TOOL);
         }
      }
      else if (msgType == dtGame::MessageType::INFO_ACTOR_DELETED)
      {
         const dtCore::UniqueId& id = message.GetAboutActorId();
         if (mStealthActor.valid() && mStealthActor->GetUniqueId() == id)
         {
            mStealthActor = NULL;
         }
         else if (mTerrainActor.valid() && mTerrainActor->GetId() == id)
         {
            mTerrainActor = NULL;
         }
      }
   
      // RESTARTED
      else if (msgType == dtGame::MessageType::INFO_RESTARTED)
      {
         // Setup the logger component
         if(mEnableLogging && !mLogController.valid())
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
      else if(msgType == dtGame::MessageType::INFO_MAP_LOADED)
      {
         dtGame::GameManager &gameManager = *GetGameManager();
         const dtGame::MapMessage &mlm = static_cast<const dtGame::MapMessage&>(message);
         dtGame::GameManager::NameVector mapNames;
         mlm.GetMapNames(mapNames);
   
         std::vector<dtDAL::ActorProxy*> actors;
         gameManager.FindActorsByType(*SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, actors);
         if(actors.empty())
         {
            LOG_ERROR("No terrain actor was found in the map: " + mapNames[0]);
         }
         else
         {
            RefPtr<dtDAL::ActorProxy> terrainActor = actors[0];
            //inputComp->SetTerrainActor(*terrainActor);
            //std::string name = terrainActor->GetName();
            //inputComp->SetTerrainActorName(name);
            dtGame::GameActorProxy *gap = dynamic_cast<dtGame::GameActorProxy*>(terrainActor.get());
            if(gap == NULL)
            {
               LOG_ERROR("The terrain actor is not a game actor. Ignoring.");
            }
            else
            {
               // Ignore the terrain and the environment from recording
               if(mEnableLogging)
               {
                  if( mLogController.valid() )
                  {
                     mLogController->RequestAddIgnoredActor(terrainActor->GetId());
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
         if(!actors.empty() && mLogController.valid())
         {
            RefPtr<dtDAL::ActorProxy> proxy = actors[0];
            mLogController->RequestAddIgnoredActor(proxy->GetId());
         }

         std::vector<dtDAL::ActorProxy*> proxies;
         gameManager.FindPrototypesByActorType(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, proxies);
         if(proxies.empty())
         {
            throw dtUtil::Exception(dtDAL::ExceptionEnum::InvalidActorException, 
               "Failed to find the stealth actor prototype in the map", 
               __FILE__, __LINE__);
         }

         dtCore::RefPtr<dtDAL::ActorProxy> p = 
            gameManager.CreateActorFromPrototype(proxies[0]->GetId());
         
         mStealthActor = 
            static_cast<SimCore::Actors::StealthActor*>(p->GetActor());

         gameManager.AddActor(mStealthActor->GetGameActorProxy(), false, false);

         // After map is loaded, we could set the base elevation - default is 0, which is fine. 
         //SimCore::Components::WeatherComponent* weatherComp = static_cast<SimCore::Components::WeatherComponent*>
         //   (GetGameManager()->GetComponentByName(SimCore::WeatherComponent::DEFAULT_NAME));
         //if (weatherComp != NULL)
         //   weatherComp->SetBaseElevation(600.0f);
      }
   }
   
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
            LOG_ALWAYS("Logging is enabled - beginning RECORD using automatic keyframes.");
            mLogController->RequestSetAutoKeyframeInterval(20.0f);
            mLogController->RequestChangeStateToRecord();
         }
      }
   }
   
   bool StealthInputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard,
                                          Producer::KeyboardKey key,
                                          Producer::KeyCharacter character)
   {
   
      bool handled = true;
      switch(key)
      {
         case Producer::Key_equal:
         {
            ChangeFlyMotionModelSpeed(true);
         }
         break;
   
         case Producer::Key_minus:
         {
            ChangeFlyMotionModelSpeed(false);
         }
         break;

         case Producer::Key_backslash:
         case Producer::Key_Insert:
         {
            GetGameManager()->GetApplication().GetCamera()->SetNextStatisticsType();
         }
         break;
   
         case Producer::Key_9:
         {
            dtCore::Transform xform;
            mStealthActor->GetTransform(xform);
            osg::Vec3 pos = xform.GetTranslation();
            std::ostringstream oss;
            oss << "The player's position is: " << pos << '\n';
            LOG_ALWAYS(oss.str());
         }
         break;
   
         case Producer::Key_comma:
         {
            Cycle(false, true);
         }
         break;
   
         case Producer::Key_period:
         {
            Cycle(true, true);
         }
         break;
   
         case Producer::Key_P:
         {
            ToggleEntityShaders();
         }
         break;
   
         case Producer::Key_L:
         {
            RefPtr<dtGame::Message> msg = GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
            SimCore::AttachToActorMessage &atamsg = static_cast<SimCore::AttachToActorMessage&>(*msg);
            atamsg.SetAboutActorId(mStealthActor->GetUniqueId());
            atamsg.SetSource(*mMachineInfo);

            if(mStealthActor->GetParent() != NULL)
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
   
         case Producer::Key_O:
         {
            IncrementTime(5);
         }
         break;
   
         case Producer::Key_I:
         {
            IncrementTime(-5);
         }
         break;
   
         case Producer::Key_M:
         {
            if(!mHasUI)
            {
               //swap motion model.
               bool fly = mStealthMM->IsEnabled();
               mStealthMM->SetEnabled(!fly);
               mAttachedMM->SetEnabled(fly);
               GetGameManager()->GetApplication().GetWindow()->ShowCursor(!fly);
            }
         }
         break;
   
         case Producer::Key_F1:
            HandleHelpPressed();
            break;
   
         case Producer::Key_F2:
            if (GetHUDComponent() != NULL )
            {
               mHUDComponent->CycleToNextHUDState(); // already in help, so toggle it off
            }
            break;

         case Producer::Key_F7:
            {
               EnableTool(SimCore::MessageType::NIGHT_VISION);
            }
            break;
   
         case Producer::Key_F8:
            {
               EnableTool(SimCore::MessageType::COMPASS);
            }
            break;
   
         case Producer::Key_F9:
            {
               if(GetTool(SimCore::MessageType::BINOCULARS)->GetPlayerActor() == NULL)
                  GetTool(SimCore::MessageType::BINOCULARS)->SetPlayerActor(mStealthActor.get());

               EnableTool(SimCore::MessageType::BINOCULARS);
            }
            break;
   
         case Producer::Key_F10:
            {
               EnableTool(SimCore::MessageType::LASER_RANGE_FINDER);
            }
            break;
   
         case Producer::Key_F11:
            {
               EnableTool(SimCore::MessageType::GPS);
            }
            break;
   
         case Producer::Key_F12:
            {
               //EnableTool(SimCore::MessageType::MAP);
            }
            break;
   
         case Producer::Key_Shift_L:
         case Producer::Key_Shift_R:
            {
               if ( mStealthActor.valid() && !mStealthActor->IsRemote())
               {
                  SimCore::Tools::LaserRangeFinder *lrf = static_cast<SimCore::Tools::LaserRangeFinder*>(GetTool(SimCore::MessageType::LASER_RANGE_FINDER));
                  if(lrf == NULL || !lrf->IsEnabled())
                     break;
   
                  dtCore::Transform xform;
                  GetGameManager()->GetApplication().GetCamera()->GetTransform(xform);
                  lrf->FindIntersectionPoint(*mTerrainActor->GetActor(), xform);
               }
            }
            break;
       
         /////////////////////////////////
         // AAR PLAYBACK AND RECORD KEYS
         /////////////////////////////////
   
         // STOP A RECORD OR PLAYBACK
         case Producer::Key_1:
         {
            if(!mHasUI)
            {
               EnableIdle();
            }
         }
         break;
   
         // RECORD
         case Producer::Key_2:
         {
            if(!mHasUI)
            {
               EnableRecord();
            }
         }
         break;
   
         // PLAYBACK
         case Producer::Key_3:
         {
            if(!mHasUI)
            {
               EnablePlayback();
            }
         }
         break;
   
         // PAUSE THE PLAYBACK
         case Producer::Key_6:
         {
            if(!mHasUI)
            {
               HandlePause();
            }
         }
         break;
   
         // SPEED CHANGES FOR AAR PLAYBACK - SLOWER
         case Producer::Key_7:
         {
            if(!mHasUI)
            {
               HandleSpeedChange(false);
            }
         }
         break;
   
         // SPEED CHANGES FOR AAR PLAYBACK - FASTER
         case Producer::Key_8:
         {
            if(!mHasUI)
            {
               HandleSpeedChange(true);
            }
         }
         break;
   
         case Producer::Key_bracketleft:
         {
            if(!mHasUI)
            {
               HandleGotoKeyFrame(false);
            }
         }
         break;
   
         case Producer::Key_bracketright:
         {
            if(!mHasUI)
            {
               HandleGotoKeyFrame(true);
            }
         }
         break;

         // Escaping or quitting when launched from Qt is BAD. Set flag so these
         // won't be processed by the base class
         case Producer::Key_Escape:
         {
            if(mHasUI)
            {
               handled = true;
            }
         }
         break;

         case Producer::Key_X:
         {
            dtABC::Application &app = GetGameManager()->GetApplication();

            if(app.GetKeyboard()->GetKeyState(Producer::Key_Alt_L) ||
               app.GetKeyboard()->GetKeyState(Producer::Key_Alt_R))
            {
               if(mHasUI)
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
   
      if(!handled)
         return SimCore::Components::BaseInputComponent::HandleKeyPressed(keyboard, key, character);
      
      return handled;
   }

   void StealthInputComponent::ChangeAARState( const dtGame::LogStateEnumeration& targetState )
   {
      if( targetState == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK )
      {
         std::cout << "StealthInputComponent::HandlePlayback" << std::endl;
         // Disconnect from the network if currently in IDLE mode
         if( mLogController->GetLastKnownStatus().GetStateEnum() 
            == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE)
         {
            // Start playback
            mLogController->RequestChangeStateToPlayback();
            mLogController->RequestServerGetKeyframes();
         }
         // If playback is being set again, restart the playback from the beginning.
         else if(mLogController->GetLastKnownStatus().GetStateEnum() 
            == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
         {
            std::cout << "\tStealthInputComponent::GotoFirstKeyframe" << std::endl;
            GotoFirstKeyframe();
         }
         else // this must be RECORD state
         {
            mLogController->RequestChangeStateToIdle();
         }
      }
      else if( targetState == dtGame::LogStateEnumeration::LOGGER_STATE_RECORD )
      {
         std::cout << "StealthInputComponent::HandleRecord" << std::endl;
         if(mLogController->GetLastKnownStatus().GetStateEnum() 
            == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE)
         {
            mLogController->RequestSetAutoKeyframeInterval(20.0f);
            mLogController->RequestChangeStateToRecord();
         }
         else if(mLogController->GetLastKnownStatus().GetStateEnum() 
            == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
         {
            mLogController->RequestChangeStateToIdle();
            if( mReconnectOnIdle && mWasConnected ) { JoinFederation(); }
         }
      }
      else if( targetState == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE )
      {
         std::cout << "StealthInputComponent::HandleIdle" << std::endl;
         mLogController->RequestChangeStateToIdle();

         if(mLogController->GetLastKnownStatus().GetStateEnum() 
            == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
         {
            if( mReconnectOnIdle && mWasConnected ) { JoinFederation(); }
         }
      }
   }

   void StealthInputComponent::EnableIdle()
   {
      if (mEnableLogging && mLogController.valid())
      {
         mTargetLogState = &dtGame::LogStateEnumeration::LOGGER_STATE_IDLE;
         mTicksToLogStateChange = 2;
      }
   }

   void StealthInputComponent::EnableRecord()
   {
      if (mEnableLogging && mLogController.valid())
      {
         mTargetLogState = &dtGame::LogStateEnumeration::LOGGER_STATE_RECORD;
         mTicksToLogStateChange = 2;
      }
   }

   void StealthInputComponent::ChangeFlyMotionModelSpeed(bool higher)
   {
      float val = mStealthMM->GetMaximumFlySpeed();
      mStealthMM->SetMaximumFlySpeed(higher ? val * 2 : val / 2);
   }
   
   void StealthInputComponent::EnablePlayback()
   {
      if (mEnableLogging && mLogController.valid())
      {
         // If this is currently IDLE mode, attempt disconnect.
         if( mLogController->GetLastKnownStatus().GetStateEnum() 
            == dtGame::LogStateEnumeration::LOGGER_STATE_IDLE)
         {
            // Do not do network activity during playback
            LeaveFederation();
         }

         mTargetLogState = &dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK;
         mTicksToLogStateChange = 2;
      }
   }
   
   void StealthInputComponent::HandlePause()
   {
      if (mEnableLogging && mEnablePlayback)
      {
         GetGameManager()->SetPaused(!GetGameManager()->IsPaused());
      }
   }

   void StealthInputComponent::HandleSpeedChange(bool higherSpeed)
   {
      if(mEnableLogging && mEnablePlayback)
      {
         if(higherSpeed)
         {
            float speedFactor = GetGameManager()->GetTimeScale() * 1.20f;
            if(speedFactor <= 10.0f)
               GetGameManager()->ChangeTimeSettings(GetGameManager()->GetSimulationTime(),
               speedFactor, GetGameManager()->GetSimulationClockTime());
         }
         else
         {
            float speedFactor = GetGameManager()->GetTimeScale() * 0.8f;
            if(speedFactor >= 0.10f)
               GetGameManager()->ChangeTimeSettings(GetGameManager()->GetSimulationTime(),
               speedFactor, GetGameManager()->GetSimulationClockTime());
         }
      }
   }

   void StealthInputComponent::HandleSpeedChange(float newSpeed)
   {
      if(mEnableLogging && mEnablePlayback)
      {
         GetGameManager()->ChangeTimeSettings(GetGameManager()->GetSimulationTime(),
         newSpeed, GetGameManager()->GetSimulationClockTime());
      }
   }

   void StealthInputComponent::HandleGotoKeyFrame(bool nextKeyFrame)
   {     
      std::cout << "StealthInputComponent::HandleGotoKeyFrame" << std::endl;
      if(mEnableLogging && mEnablePlayback)
      {
         nextKeyFrame ? GotoNextKeyframe() : GotoPreviousKeyframe();
      }
   }

   void StealthInputComponent::HandleGotoKeyFrame(const std::string &name)
   {
      std::cout << "StealthInputComponent::HandleGotoKeyFrame(" << name.c_str() << ")" << std::endl;
      if(mEnableLogging && mEnablePlayback)
      {
         const std::vector<dtGame::LogKeyframe> &KFs = mLogController->GetLastKnownKeyframeList();
         for(size_t i = 0; i < KFs.size(); i++)
         {
            if(KFs[i].GetName() == name)
            {
               mLogController->RequestJumpToKeyframe(KFs[i]);
               break;
            }
         }
      }
   }

   void StealthInputComponent::HandleAddKeyFrame(const dtGame::LogKeyframe &kf)
   {
      if(mEnableLogging && mEnablePlayback)
      {
         mLogController->RequestCaptureKeyframe(kf);
      }
   }

   void StealthInputComponent::HandleGetKeyFrames()
   {
      if(mEnableLogging && mEnablePlayback)
      {
         mLogController->RequestServerGetKeyframes();
      }
   }

   void StealthInputComponent::HandleSetAutoKeyFrameInterval(double interval)
   {
      if(mEnableLogging && mEnablePlayback)
      {
         mLogController->RequestSetAutoKeyframeInterval(interval);
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
      dtCore::ShaderManager::GetInstance().ReloadAndReassignShaderDefinitions("Shaders/DVTEShaderDefs.xml");

   }
   
   void StealthInputComponent::Cycle(bool forward, bool attach)
   {
      std::vector<dtDAL::ActorProxy*> actors;
      //GetGameManager()->GetAllActors(actors);
      GetGameManager()->GetAllActors(actors);
      //unsigned int size = actors.size();
      GetGameManager()->FindActorsByClassName("SimCore::Actors::BaseEntity", actors);
      if(actors.empty())
         return;
   
      if (mCycleIndex >= actors.size())
         mCycleIndex = mCycleIndex % actors.size();
   
      RefPtr<dtDAL::ActorProxy> ap = actors[mCycleIndex];
      if (ap->GetActor() != mStealthActor.get() && ap != mTerrainActor &&
         ap.get() != GetGameManager()->GetEnvironmentActor())
      {
         RefPtr<dtGame::Message> nMsg = GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
         nMsg->SetAboutActorId(mStealthActor->GetUniqueId());
   
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
            dtCore::Transformable *t = dynamic_cast<dtCore::Transformable*>(ap->GetActor());
            if (t != NULL)
            {
               t->GetTransform(objectxform);
               float x, y, z;
               objectxform.GetTranslation(x, y, z);
               y -= 15;
               z += 3;
               mStealthActor->SetTransform(dtCore::Transform(x, y, z));
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
            std::ostringstream ss;
            ss << "## Attempting to Jump to Previous Keyframe: [" << prevFrame->GetName() << "] ##";
            std::cout << ss.str() << std::endl;
   
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
            std::ostringstream ss;
            ss << "## Attempting to Jump to Next Keyframe: [" << nextFrame->GetName() << "] ##";
            std::cout << ss.str() << std::endl;
   
            mLogController->RequestJumpToKeyframe(*nextFrame);
         }
         GetGameManager()->SetPaused(false);
      }
   }
   
   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::SetConnectionParameters(const std::string &executionName,
                                                       const std::string &fedFile,
                                                       const std::string &federateName, 
                                                       const std::string &ridFile)
   {
      mExecutionName = executionName;
      mFedFile = fedFile;
      mFederateName = federateName;
      mRidFile = ridFile;
   }
   
   //////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::JoinFederation( bool updateSystem )
   {
      // If not connected, attempt a connection.
      // This check avoids multiple calls to connection
      // such as in the case of input flooding.
      if( ! IsConnected() )
      {
         if( updateSystem )
         {
            // Let the ServerLoggerComponent and LogController components
            // synchronize before attempting connection.
            //This is bad.  we can do this another way.
            //dtCore::System::GetInstance().Step();
         }
   
         if( mHLA.valid() )
         {
            mHLA->JoinFederationExecution(mExecutionName, mFedFile, mFederateName, mRidFile);
         }
         else
         {
            LOG_ERROR("HLA Component cannot be accessed. Attempted to join federation.");
         }
         
         if( updateSystem )
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
      if( mWasConnected )
      {
         // Attempt the disconnect from the network
         if( mHLA.valid() )
         {
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
   void StealthInputComponent::EnableTool(SimCore::MessageType &msgType)
   {
      if( ! mStealthActor.valid() 
         || mStealthActor->IsRemote() )
      {
         return;
      }
   
      // Create the tool message to be circulated.
      dtCore::RefPtr<dtGame::Message> msg = GetGameManager()->GetMessageFactory().CreateMessage(msgType);
      SimCore::Tools::Tool *tool = GetTool(msgType);
      if(tool == NULL)
      {
         LOG_ERROR("Received a tool message from a tool the player does not have");
         return;
      }
   
      // Fill out the message with important data
      bool enable = !tool->IsEnabled();
      static_cast<SimCore::ToolMessage*>(msg.get())->SetEnabled(enable);
      msg->SetAboutActorId(mStealthActor->GetUniqueId());
      msg->SetSource(*mMachineInfo);
   
      // Avoid circulating tool messages if in RECORD mode.
      // They should not be recorded by the server logger component.
      // ...otherwise...
      // If PLAYBACK mode has ended, the game manager will be paused.
      // If so, the message will have to be sent directly to the HUD
      // as well as this component so that they are updated properly.
      if( IsInRecord() || GetGameManager()->IsPaused() )
      {
         // Update this Input Component's current tool
         SetEnabledTool( enable ? msgType : SimCore::MessageType::NO_TOOL );
         UpdateTools();
         // Update the HUD
         GetHUDComponent()->ProcessMessage(*msg);
      }
      else // messages can be circulated
      {
         // Let messages update the relevant components.
         GetGameManager()->SendMessage(*msg);
      }
   
      // Update the camera rotation speed if the tool has
      // differing field of view.
      if(mAttachedMM.valid())
      {
         if(msgType == SimCore::MessageType::BINOCULARS || 
            msgType == SimCore::MessageType::LASER_RANGE_FINDER)
         {
            if(enable)
            {
               mAttachedMM->SetMaximumMouseTurnSpeed(200.0);
               mAttachedMM->SetKeyboardTurnSpeed(10.0);
            }
            else
            {
               mAttachedMM->SetMaximumMouseTurnSpeed(1440.0f);
               mAttachedMM->SetKeyboardTurnSpeed(70.0f);
            }
         }
         else
         {
            mAttachedMM->SetMaximumMouseTurnSpeed(1440.0f);
            mAttachedMM->SetKeyboardTurnSpeed(70.0f);
         }
      }
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::AddTool(SimCore::Tools::Tool &tool, SimCore::MessageType &type)
   {
      if(!SimCore::MessageType::IsValidToolType(type))
      {
         LOG_ERROR("Tried to add a tool with an invalid type");
         return;
      }
      if(mToolList.find(&type) == mToolList.end())
         mToolList.insert(std::make_pair(&type, RefPtr<SimCore::Tools::Tool>(&tool)));
      else
         LOG_ERROR("AddTool tried to add a tool more than once.");
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::RemoveTool(SimCore::MessageType &type)
   {
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i =  mToolList.find(&type);
      if(i == mToolList.end())
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
   void StealthInputComponent::UpdateTools()
   {
      SimCore::Tools::Compass *compass = static_cast<SimCore::Tools::Compass*>(GetTool(SimCore::MessageType::COMPASS));
      if(compass == NULL)
      {
         LOG_DEBUG("The player actor currently does not have a compass");
   
      }
      else
      {
   	   compass->Update();
      }
   
      SimCore::Tools::GPS *gps = static_cast<SimCore::Tools::GPS*>(GetTool(SimCore::MessageType::GPS));
      if(gps == NULL)
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
   void StealthInputComponent::SetEnabledTool(SimCore::MessageType &tool)
   {
      if(!SimCore::MessageType::IsValidToolType(tool))
      {
         LOG_ERROR("Tried to add a tool with an invalid type");
         return;
      }
   
      if(tool == SimCore::MessageType::NO_TOOL)
      {
         std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i;
         for(i = mToolList.begin(); i != mToolList.end(); ++i)
         {
            if(i->second->IsEnabled())
               i->second->Enable(false);
         }
         return;
      }
   
      SimCore::Tools::Tool *t = GetTool(tool);
      if(t == NULL)
      {
         LOG_WARNING("Tried to enable a tool the player does not have");
         return;
      }
      if(!t->IsEnabled())
      {
         // Turn off all other tools
         std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i;
         for(i = mToolList.begin(); i != mToolList.end(); ++i)
         {
            if(i->second->IsEnabled())
               i->second->Enable(false);
         }
         t->Enable(true);
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
      if(mLogController.valid())
      {
         mLogController->RequestSetLogFile(fileName);
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void StealthInputComponent::ChangeMotionModels(bool firstPerson)
   {
      mFirstPersonAttachMode = firstPerson;

      StealthHUD *hud = GetHUDComponent();

       if(firstPerson)
      {
         if(!mAttachedMM.valid())
            return;

         // Already enabled correctly. Peace out. 
         if(mAttachedMM->IsEnabled())
            return;

         mStealthMM->SetTarget(NULL);
         mStealthMM->SetEnabled(false);

         mAttachedMM->SetTarget(mStealthActor.get());
         mAttachedMM->SetEnabled(true);

         mStealthActor->SetAttachAsThirdPerson(false);

         if(hud != NULL)
         {
            // Data should not be sent by messages because they
            // may be recorded during record mode.
            hud->SetMotionModel(mAttachedMM.get());
         }
      }
      else
      {
         if(!mStealthMM.valid())
            return;

         // Already enabled correctly. Peace out. 
         if(mStealthMM->IsEnabled())
            return;

         mAttachedMM->SetTarget(NULL);
         mAttachedMM->SetEnabled(false);

         mStealthMM->SetTarget(mStealthActor.get());
         mStealthMM->SetEnabled(true);

         mStealthActor->SetAttachAsThirdPerson(true);

         if(hud != NULL)
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

      if(mStealthMM.valid())
         mStealthMM->SetCollideWithGround(mCollideWithGround);
   }

   //////////////////////////////////////////////////////////////////////////////////
   bool StealthInputComponent::IsConnected() const
   {
      return mHLA->IsConnectedToFederation();
   }
}
