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
* @author Curtiss Murphy
*/

#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/StealthHUD.h>
#include <SimCore/Components/LabelManager.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/StealthHUDElements.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/UnitEnums.h>
#include <SimCore/Tools/Compass360.h>

#include <dtUtil/macros.h>
#include <dtUtil/exception.h>
#include <dtUtil/datapathutils.h>
#include <dtUtil/datetime.h>

#include <dtCore/object.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>

#include <dtGame/logtag.h>
#include <dtGame/logkeyframe.h>
#include <dtGame/logstatus.h>
#include <dtGame/defaultmessageprocessor.h>
#include <dtGame/loggermessages.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>
#include <dtGame/logcontroller.h>
#include <dtGame/logstatus.h>

#include <dtActors/taskactor.h>
#include <dtActors/coordinateconfigactor.h>
#include <dtActors/engineactorregistry.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/project.h>
#include <dtDAL/map.h>
#include <dtDAL/actorproxy.h>
#include <dtDAL/transformableactorproxy.h>

#include <CEGUI/CEGUIVersion.h>

#include <ctime>

#ifdef AGEIA_PHYSICS
   #include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
#endif

namespace StealthGM
{
   IMPLEMENT_ENUM(CoordSystem);

   const CoordSystem CoordSystem::MGRS("MGRS");
   const CoordSystem CoordSystem::RAW_XYZ("RAW_XYZ");
   const CoordSystem CoordSystem::LAT_LON("LAT_LON");

   const std::string StealthHUD::DEFAULT_NAME("StealthHUD");

   //////////////////////////////////////////////////////////////////////////
   StealthHUD::StealthHUD(dtCore::DeltaWin* win,
                          dtGame::LogController* logController,
                          const std::string &name,
                          bool hasUI)
      : SimCore::Components::BaseHUD(win, name)
      , mLastHUDStateBeforeHelp(&SimCore::Components::HUDState::MINIMAL)
      , mLogController(logController)
      , mRightTextXOffset(225.0f)
      , mTextYTopOffset(10.0f)
      , mTextYSeparation(2.0f)
      , mTextHeight(40.0f)
      , mZoomToolEnabled(false)
      , mCompass360WasEnabled(false)
      , mLastLogState(NULL)
      , mCoordSystem(&CoordSystem::MGRS)
      , mHasUI(hasUI)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   StealthHUD::~StealthHUD()
   {
      perror("Stealth HUD Destructor\n");
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& type = message.GetMessageType();

      if( type == dtGame::MessageType::TICK_LOCAL )
      {
         TickHUD();
      }
      else if( type == dtGame::MessageType::TICK_REMOTE )
      {
         // DO NOTHING but escape early.
      }
      else if( type == dtGame::MessageType::TICK_END_OF_FRAME )
      {
         //const dtGame::TickMessage& tick
            //= static_cast<const dtGame::TickMessage&>(message);

         // Update Label Manager since all entities are at their
         // final positions in the world.
//         mLabelManager->Update( tick.GetDeltaRealTime() );
      }
      else if( type == dtGame::MessageType::INFO_MAP_LOADED )
      {
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
         //Is this a problem now?
         SimCore::Components::RenderingSupportComponent* renderComp = NULL;
         dtGame::GMComponent* comp = GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME);

         if(comp != NULL)
         {
            renderComp = dynamic_cast<SimCore::Components::RenderingSupportComponent*>(comp);
         }

         if(comp == NULL || renderComp == NULL)
         {
            GetGameManager()->GetScene().AddDrawable( GetGUIDrawable().get() );
            LOG_WARNING("Unable to add GUI to the RenderSupportComponent, adding GUI to the Scene instead.");
         }
         else
         {
            renderComp->SetGUI(GetGUIDrawable().get());
         }
#endif

         std::vector<dtDAL::ActorProxy*> proxies;
         GetGameManager()->FindActorsByType(*dtActors::EngineActorRegistry::COORDINATE_CONFIG_ACTOR_TYPE, proxies);

         if(proxies.empty())
         {
            LOG_ERROR("Failed to find a coordinate config actor in the map. Using default values.");
            return;
         }

         dtActors::CoordinateConfigActor &ccActor =
            static_cast<dtActors::CoordinateConfigActor&>(*proxies[0]->GetDrawable());

         SetCoordinateConverter(ccActor.GetCoordinateConverter());
      }
      else // Is this a tool message?
      {
         const SimCore::ToolMessage* toolMessage = dynamic_cast<const SimCore::ToolMessage*>(&message);
         if(toolMessage != NULL)
         {
            ProcessToolMessage(*toolMessage);
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::ProcessToolMessage(const SimCore::ToolMessage& toolMessage)
   {
      const dtGame::MessageType& type = toolMessage.GetMessageType();
      bool toolEnabled = toolMessage.GetEnabled();

      if(type == SimCore::MessageType::BINOCULARS)
      {
         mToolbar->SetButtonsActive(false);
         mToolbar->SetButtonActive("Binoculars",toolEnabled);
         UpdateHelpButton();

         mZoomToolEnabled = toolEnabled;
      }
      else if(type == SimCore::MessageType::NIGHT_VISION)
      {
         mToolbar->SetButtonsActive(false);
         mToolbar->SetButtonActive("NightVision",toolEnabled);
         UpdateHelpButton();

         mZoomToolEnabled = toolEnabled;
      }
      else if(type == SimCore::MessageType::LASER_RANGE_FINDER)
      {
         mToolbar->SetButtonsActive(false);
         mToolbar->SetButtonActive("LRF",toolEnabled);
         UpdateHelpButton();

         mZoomToolEnabled = toolEnabled;
      }
      else if(type == SimCore::MessageType::GPS)
      {
         mToolbar->SetButtonsActive(false);
         mToolbar->SetButtonActive("GPS",toolEnabled);
         UpdateHelpButton();
      }
      else if(type == SimCore::MessageType::MAP)
      {
         mToolbar->SetButtonsActive(false);
         mToolbar->SetButtonActive("Map",toolEnabled);
         UpdateHelpButton();
      }
      else if(type == SimCore::MessageType::COMPASS)
      {
         mToolbar->SetButtonsActive(false);
         mToolbar->SetButtonActive("Compass",toolEnabled);
         UpdateHelpButton();
      }
      else if(type == SimCore::MessageType::COMPASS_360)
      {
         mCompass360WasEnabled = toolEnabled;
         SetCompass360Enabled(toolEnabled);
      }
      else if(type == SimCore::MessageType::NO_TOOL)
      {
         mZoomToolEnabled = false;
         mCompass360WasEnabled = false;
      }

      // Handle the 360 compass special case; it should not
      // be on when using one of the other tools that affect
      // the camera zoom.
      //
      // Should it be forced off?
      if(mZoomToolEnabled)
      {
         // Maintain the old state of mCompass360WasEnabled.
         bool wasEnabled = mCompass360WasEnabled;

         // This method affects the value of mCompass360WasEnabled.
         SetCompass360Enabled(false);

         // Set the state back.
         mCompass360WasEnabled = wasEnabled;
      }
      else
      {
          // ...otherwise return the tool back to its normal state.
         SetCompass360Enabled(mCompass360WasEnabled);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetupGUI( SimCore::Components::HUDGroup& mainOverlay, unsigned int designedResWidth, unsigned int designedResHeight )
   {
      mainOverlay.SetDeleteWindowOnDestruct(true);

      mHUDOverlay = new SimCore::Components::HUDGroup("HUD Overlay");
      mainOverlay.Add( mHUDOverlay.get() );

      mToolbarOverlay = new SimCore::Components::HUDGroup("HUD Toolbar Overlay");
      mainOverlay.Add( mToolbarOverlay.get() );

      // Label Layer
      mLabelLayer = new SimCore::Components::HUDElement( "LabelLayer",
         SimCore::Components::HUDElement::DEFAULT_BLANK_TYPE );
      mLabelLayer->SetSize( 1.0f, 1.0f );
      mLabelLayer->SetVisible( true );
      mainOverlay.Add( mLabelLayer.get() );

      // --- Setup the associated label manager.
      mLabelManager = new SimCore::Components::LabelManager;
      mLabelManager->SetGameManager( GetGameManager() );
      mLabelManager->SetGUILayer( mLabelLayer.get() );

      // Toolbar
      mToolbar = new SimCore::Components::StealthToolbar("StealthToolbar");
      mToolbarOverlay->Add(mToolbar.get());
      mToolbar->SetAlignment( SimCore::Components::HUDAlignment::CENTER_TOP );
      mToolbar->AddButton("Help","F1",true);

      // --- Maintain a reference to the toolbar's help button
      //     so that it can be updated properly.
      mHelpButton = mToolbar->GetButton("Help");
      mHelpButton->SetActive( false );

      // Tools GUI Layer
      mToolsLayer = new SimCore::Components::HUDGroup("ToolsLayer");
      mToolsLayer->GetCEGUIWindow()->disable(); // avoids mouse clicks that change z-order
      mHUDOverlay->Add(mToolsLayer.get());

      // Sim time & record/playback state
      mSimTimeAndState = new SimCore::Components::StealthGPSMeter("SimTimeAndStateMeter");
      mSimTimeAndState->SetPosition( 256.0f/1920.0f, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mSimTimeAndState->SetText1("Sim Time");
      mSimTimeAndState->GetText1().SetColor(0.1, 0.1, 1.0);
      mSimTimeAndState->GetText2().SetColor(0.2, 0.2, 0.2);
      mHUDOverlay->Add( mSimTimeAndState.get() );

      // Compass Meter
      mCompass = new SimCore::Components::StealthCompassMeter("StealthCompass");
      mCompass->Initialize();
      mCompass->SetAlignment( SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mHUDOverlay->Add(mCompass.get());

      // GPS Meter
      mGPS = new SimCore::Components::StealthCartesianMeter("StealthGPSMeter");
      mGPS->SetPosition( 512.0f/1920.0f, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mHUDOverlay->Add(mGPS.get());

      // MGRS Meter
      mMGRSMeter = new SimCore::Components::StealthMGRSMeter("StealthMGRSMeter");
      mMGRSMeter->SetPosition( 512.0f/1920.0f, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mMGRSMeter->SetVisible( false );
      mHUDOverlay->Add(mMGRSMeter.get());

      // Cartesian Meter
      mCartesianMeter = new SimCore::Components::StealthCartesianMeter("StealthCartesianMeter");
      mCartesianMeter->SetPosition( 512.0f/1920.0f, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mCartesianMeter->SetVisible( false );
      mHUDOverlay->Add(mCartesianMeter.get());

      // Help Overlay
      InitHelpOverlay( mainOverlay );
      SetHelpEnabled( false );
   }

   void StealthHUD::InitHelpOverlay( SimCore::Components::HUDGroup& hudOverlay )
   {
      mHelpOverlay = new SimCore::Components::HUDGroup("HelpScreen");
      mHelpOverlay->SetPosition( 0, 0, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      hudOverlay.Add( mHelpOverlay.get() );

      float offsetX = 0.0f;
      float offsetY = 140.0f/1200.0f;
      SimCore::Components::HUDAlignment* align = &SimCore::Components::HUDAlignment::LEFT_TOP;

      // Create the help text background to give contrast with the scene
      dtCore::RefPtr<SimCore::Components::HUDImage> background = new SimCore::Components::HUDImage("HelpTextClipboard");
      background->SetImage("Clipboard","Clipboard");
      background->SetSize( 350.0f/1920.0f, 532.0f/1200.0f );
      background->SetPosition( 0.0, -256.0f/1200.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mHelpOverlay->Add( background.get() );

      // Soldier Mode Help Text
      std::string text;
      if( mHasUI )
      {
         text = "\
         Mouse Right Btn : Move camera\n\
         W/A/S/D : Move camera\n\
         Mouse Left Btn : Turn camera\n\
         Arrow Keys : Turn camera\n\
         +/- : Change camera speed\n\
         L : Attach/Detach from entity\n\
         F2 : Toggle HUD display\n\
         Insert : Toggle statistics";
      }
      else
      {
         text = "\
         Mouse Right Btn : Move camera\n\
         W/A/S/D : Move camera\n\
         Mouse Left Btn : Turn camera\n\
         Arrow Keys : Turn camera\n\
         +/- : Change camera speed\n\
         ,/. : Cycle between entities\n\
         L : Toggle attachment to entity\n\
         I/O : Decrease/Increase time of day\n\
         1 : Set to IDLE state\n\
         2 : Set RECORD state from IDLE\n\
         3 : Set PLAYBACK state from IDLE\n\
         9 : Report world coordinates\n\
         F2 : Toggle HUD display\n\
         Insert : Toggle statistics";
      }
      mHelpText = CreateText( "HelpText", text, offsetX, offsetY, 1.0, 1.0 );
      mHelpText->SetAlignment(*align);
      mHelpOverlay->Add( mHelpText.get() );
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetUnitOfLength(SimCore::UnitOfLength& unit)
   {
      mUnitOfLength = &unit;
   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::UnitOfLength& StealthHUD::GetUnitOfLength() const
   {
      return *mUnitOfLength;
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetUnitOfAngle(SimCore::UnitOfAngle& unit)
   {
      mUnitOfAngle = &unit;
   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::UnitOfAngle& StealthHUD::GetUnitOfAngle() const
   {
      return *mUnitOfAngle;
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::TickHUD()
   {
      int x(0), y(0), w(0), h(0);
      GetMainDeltaWindow()->GetPosition(x, y, w, h);
   //   float curYPos;

      // update the compass
      if(mMotionModel.valid() && mMotionModel->GetTarget() != NULL)
      {
         dtCore::Transform xform;
         mMotionModel->GetTarget()->GetTransform(xform);
         osg::Vec3 pos;
         xform.GetTranslation(pos);
         osg::Vec3 hpr;
         xform.GetRotation(hpr);

         // negative hpr because we want turning right to be positive.
         double angle = SimCore::UnitOfAngle::Convert(SimCore::UnitOfAngle::DEGREE, *mUnitOfAngle, -hpr[0]);

         mCompass->SetShowWholeNumbersOnly(mUnitOfAngle->GetUseWholeUnits());
         // Update the compass
         mCompass->SetValue(angle, mUnitOfAngle->GetMax(), 0.0f);

         // Update the appropriate meter with the current coordinates
         if(*mCoordSystem == CoordSystem::MGRS)
         {
            if(!mMGRSMeter->IsVisible())
            {
               mCartesianMeter->SetVisible(false);
               mGPS->SetVisible(false);
               mMGRSMeter->SetVisible(true);
            }

            /*mCoordinateConverter.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
            osg::Vec3d latLonElev = mCoordinateConverter.ConvertToRemoteTranslation(pos);

            unsigned ewZone;
            char nsZone;

            dtUtil::Coordinates::CalculateUTMZone(latLonElev[0], latLonElev[1], ewZone, nsZone);

            mCoordinateConverter.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::UTM);
            osg::Vec3d eastingNorthingElev = GetCoordinateConverter().ConvertToRemoteTranslation(pos);*/

            std::string milgrid = mCoordinateConverter.XYZToMGRS(pos);
            mMGRSMeter->SetText(milgrid);
         }
         else if(*mCoordSystem == CoordSystem::RAW_XYZ)
         {
            if(!mCartesianMeter->IsVisible())
            {
               mCartesianMeter->SetVisible(true);
               mGPS->SetVisible(false);
               mMGRSMeter->SetVisible(false);
            }

            mCartesianMeter->SetX(SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, *mUnitOfLength, pos[0]),
                     mUnitOfLength->GetAbbreviation());
            mCartesianMeter->SetY(SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, *mUnitOfLength, pos[1]),
                     mUnitOfLength->GetAbbreviation());
            mCartesianMeter->SetZ(SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, *mUnitOfLength, pos[2]),
                     mUnitOfLength->GetAbbreviation());
         }
         else
         {
            if(!mGPS->IsVisible())
            {
               mCartesianMeter->SetVisible(false);
               mGPS->SetVisible(true);
               mMGRSMeter->SetVisible(false);
            }

            mCoordinateConverter.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
            const osg::Vec3d& globePos = mCoordinateConverter.ConvertToRemoteTranslation(pos);
            mGPS->SetX(globePos[0], SimCore::UnitOfAngle::DEGREE.GetAbbreviation());
            mGPS->SetY(globePos[1], SimCore::UnitOfAngle::DEGREE.GetAbbreviation());
            mGPS->SetZ(SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, *mUnitOfLength, globePos[2]),
                     mUnitOfLength->GetAbbreviation());
         }
      }

      if(GetHUDState() != SimCore::Components::HUDState::HELP)
      {
         // Log - CMM - I removed this check in order to account for 'paused' which can happen at any time
         //if( mLogController.valid() && mLastLogState != &mLogController->GetLastKnownStatus().GetStateEnum() ) {
         mLastLogState = &mLogController->GetLastKnownStatus().GetStateEnum();

         // Playback State
         if (GetGameManager()->IsPaused())
         {
            mSimTimeAndState->SetText1("PAUSED");
            mSimTimeAndState->GetText1().SetColor(0.2, 0.2, 0.7);
         }
         else if (dtGame::LogStateEnumeration::LOGGER_STATE_IDLE == *mLastLogState )
         {
            mSimTimeAndState->SetText1("LIVE");
            mSimTimeAndState->GetText1().SetColor(0.1, 0.1, 0.5);
         }
         else if (dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK == *mLastLogState )
         {
            mSimTimeAndState->SetText1("REPLAY");
            mSimTimeAndState->GetText1().SetColor(0.1, 0.5, 0.1);
         }
         else // if (dtGame::LogStateEnumeration::LOGGER_STATE_RECORD == *mLastLogState )
         {
            mSimTimeAndState->SetText1("RECORD");
            mSimTimeAndState->GetText1().SetColor(0.5, 0.1, 0.1);
         }

         // Set the time control to the basic sim time
         mSimTimeAndState->SetText2(
            dtUtil::DateTime::ToString(GetGameManager()->GetSimulationClockTime() / 1000000,
            dtUtil::DateTime::TimeFormat::CLOCK_TIME_24_HOUR_FORMAT) );

         UpdateHighDetailData();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetMotionModel( dtCore::MotionModel* motionModel )
   {
      mMotionModel = motionModel;
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::UpdateHighDetailData()
   {
      if (GetHUDState() == SimCore::Components::HUDState::MAXIMUM)
      {
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::UpdateStaticText(SimCore::Components::HUDText* textControl, char *newText,
                                     float red, float blue, float green, float x, float y)
   {
      if (textControl != NULL)
      {
         // text and color
         if (newText != NULL && textControl->GetText() != std::string(newText))
         {
            textControl->SetText(newText);
            if (red >= 0.0f && blue >= 0.0f && green >= 0.0f)
            {
               textControl->SetColor(red,green,blue);
            }
         }
         // position
         if (x > 0.0 && y > 0.0)
         {
            textControl->SetPosition(x,y,true);
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::Components::HUDState & StealthHUD::CycleToNextHUDState()
   {
      if (GetHUDState() == SimCore::Components::HUDState::MINIMAL) // MINIMAL - go to MAXIMUM
      {
         SetHUDState( SimCore::Components::HUDState::MAXIMUM );
         mLastHUDStateBeforeHelp = &GetHUDState();
      }
      else if (GetHUDState() == SimCore::Components::HUDState::MAXIMUM) // MAXIMUM, go to NONE
      {
         SetHUDState( SimCore::Components::HUDState::NONE );
         mLastHUDStateBeforeHelp = &GetHUDState();
      }
      else if (GetHUDState() == SimCore::Components::HUDState::NONE) // NONE - go to MINIMUM
      {
         SetHUDState( SimCore::Components::HUDState::MINIMAL );
         mLastHUDStateBeforeHelp = &GetHUDState();
      }
      else // HELP - go to last state before Help
      {
         SetHUDState( *mLastHUDStateBeforeHelp );
      }

      // we've changed our state, so reset our hide/show status
      UpdateState();

      return GetHUDState();
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::UpdateState()
   {
      if (GetHUDState() == SimCore::Components::HUDState::HELP)
      {
         if( ! mHasUI )
         {
            mHUDOverlay->Hide();
            mToolsLayer->Hide();
         }
         mHelpOverlay->Show();
      }
      else {
         mHelpOverlay->Hide();

         if (GetHUDState() == SimCore::Components::HUDState::MINIMAL
            || GetHUDState() == SimCore::Components::HUDState::MAXIMUM)
         {
            mHUDOverlay->Show();
            mToolsLayer->Show();
            mToolbar->Show();
         }
         else // if (GetHUDState() == SimCore::Components::HUDState::NONE)
         {
            mHUDOverlay->Hide();
            mToolsLayer->Hide();
            mToolbar->Hide();
         }
      }
      UpdateHelpButton();
   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::Components::HUDText* StealthHUD::CreateText(const std::string &name, const std::string &text,
      float x, float y, float width, float height)
   {
      SimCore::Components::HUDText* newText = new SimCore::Components::HUDText(name);
      newText->SetFontAndText("Arial-Bold-12",text,x,y);
      newText->SetSize(width,height);
      return newText;
   }

   //////////////////////////////////////////////////////////////////////////
   bool StealthHUD::AddToolButton( const std::string& buttonName, const std::string& keyLabel, bool enable )
   {
      if(mToolbar.valid())
      {
         bool success = mToolbar->AddButton( buttonName, keyLabel );
         if( mToolbar->GetButtonCount() > 0 )
         {
            mToolbar->Show();
         }

         SimCore::Components::StealthButton* button = mToolbar->GetButton( buttonName );
         if( button != NULL ) { button->SetDisabled( !enable ); }

         return success;
      }
      return false;
   }

   //////////////////////////////////////////////////////////////////////////
   bool StealthHUD::RemoveToolButton( const std::string& buttonName )
   {
      if(mToolbar.valid())
      {
         bool success = mToolbar->RemoveButton( buttonName );
         if( mToolbar->GetButtonCount() == 0 )
         {
            mToolbar->Hide();
         }
         return success;
      }
      return false;
   }

   //////////////////////////////////////////////////////////////////////////
   CEGUI::Window* StealthHUD::GetToolsWindow()
   {
      return mToolsLayer->GetCEGUIWindow();
   }

   //////////////////////////////////////////////////////////////////////////
   const CEGUI::Window* StealthHUD::GetToolsWindow() const
   {
      return mToolsLayer->GetCEGUIWindow();
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetToolbarVisible( bool visible )
   {
      mToolsLayer->SetVisible( visible );
   }

   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::UpdateHelpButton()
   {
      mToolbar->SetButtonActive( "Help", GetHUDState() == SimCore::Components::HUDState::HELP );
   }

   ////////////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetHelpEnabled( bool enabled )
   {
      if( mHelpOverlay.valid() )
      {
         mHelpOverlay->SetVisible( enabled );
      }
      if( mHelpButton.valid() )
      {
         mHelpButton->SetActive( enabled );
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool StealthHUD::IsHelpEnabled() const
   {
      return mHelpOverlay.valid() && mHelpOverlay->IsVisible();
   }

   ////////////////////////////////////////////////////////////////////////////////
   SimCore::Components::LabelManager& StealthHUD::GetLabelManager()
   {
      return *mLabelManager;
   }

   ////////////////////////////////////////////////////////////////////////////////
   const SimCore::Components::LabelManager& StealthHUD::GetLabelManager() const
   {
      return *mLabelManager;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetupCompass360()
   {
      if( ! mCompass360.valid())
      {
         // 360 Compass
         osg::Group* sceneNode = GetGameManager()->GetApplication().GetScene()->GetSceneNode();
         mCompass360 = new SimCore::Tools::Compass360();
         mCompass360->Init(*sceneNode, "Textures/Tools/Compass360.tga");
         mCompass360->Enable(false);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool StealthHUD::HasCompass360() const
   {
      return mCompass360.valid();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetCompass360Enabled(bool enable)
   {
      // The HUD may or may not have a compass 360.
      if(mCompass360.valid())
      {
         mCompass360WasEnabled = enable;

         mCompass360->Enable(enable && ( ! mZoomToolEnabled));
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool StealthHUD::IsCompass360Enabled() const
   {
      return mCompass360.valid() && mCompass360WasEnabled;
   }

}
