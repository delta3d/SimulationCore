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

#include <StealthViewer/GMApp/StealthHUD.h>
#include <SimCore/Components/StealthHUDElements.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/RenderingSupportComponent.h>

#include <dtUtil/macros.h>
#include <dtUtil/exception.h>

#include <dtCore/object.h>
#include <dtCore/globals.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>

#include <dtGame/binarylogstream.h>
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
   StealthHUD::StealthHUD(dtCore::DeltaWin *win, 
                          dtGame::LogController* logController, 
                          const std::string &name, 
                          bool hasUI)
   :  SimCore::Components::BaseHUD(win, name),
      mLastHUDStateBeforeHelp(&SimCore::Components::HUDState::MINIMAL),
      mLogController(logController),
      mRightTextXOffset(225.0f),
      mTextYTopOffset(10.0f),
      mTextYSeparation(2.0f),
      mTextHeight(40.0f),
      mLastLogState(NULL), 
      mCoordSystem(&CoordSystem::MGRS), 
      mHasUI(hasUI)
   {
   }
   
   //////////////////////////////////////////////////////////////////////////
   StealthHUD::~StealthHUD()
   {
   }
   
   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::ProcessMessage(const dtGame::Message& message)
   {
      if (message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
      {
         TickHUD();
      }
      else if(message.GetMessageType() == SimCore::MessageType::BINOCULARS)
      {
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&> (message);
         mToolbar->SetButtonActive("Binoculars",toolMsg.IsEnabled());
         UpdateHelpButton();
      }
      else if(message.GetMessageType() == SimCore::MessageType::NIGHT_VISION)
      {
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
         mToolbar->SetButtonActive("NightVision",toolMsg.IsEnabled());
         UpdateHelpButton();
      }
      else if(message.GetMessageType() == SimCore::MessageType::LASER_RANGE_FINDER)
      {
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
         mToolbar->SetButtonActive("LRF",toolMsg.IsEnabled());
         UpdateHelpButton();
      }
      else if(message.GetMessageType() == SimCore::MessageType::GPS)
      {
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
         mToolbar->SetButtonActive("GPS",toolMsg.IsEnabled());
         UpdateHelpButton();
      }
      else if(message.GetMessageType() == SimCore::MessageType::MAP)
      {
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
         mToolbar->SetButtonActive("Map",toolMsg.IsEnabled());
         UpdateHelpButton();
      }
      else if(message.GetMessageType() == SimCore::MessageType::COMPASS)
      {
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
         mToolbar->SetButtonActive("Compass",toolMsg.IsEnabled());
         UpdateHelpButton();
      }
      else if (message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
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


         std::vector<dtDAL::ActorProxy*> proxies;
         GetGameManager()->FindActorsByType(*dtActors::EngineActorRegistry::COORDINATE_CONFIG_ACTOR_TYPE, proxies);

         if(proxies.empty())
         {
            LOG_ERROR("Failed to find a coordinate config actor in the map. Using default values.");
            return;
         }

         dtActors::CoordinateConfigActor &ccActor = 
            static_cast<dtActors::CoordinateConfigActor&>(*proxies[0]->GetActor());
           
         SetCoordinateConverter(ccActor.GetCoordinateConverter());   
      }
   }
   
   //////////////////////////////////////////////////////////////////////////
   void StealthHUD::SetupGUI( SimCore::Components::HUDGroup& mainOverlay, unsigned int designedResWidth, unsigned int designedResHeight )
   {
      mHUDOverlay = new SimCore::Components::HUDGroup("HUD Overlay");
      mainOverlay.Add( mHUDOverlay.get() );
   
      mToolbarOverlay = new SimCore::Components::HUDGroup("HUD Toolbar Overlay");
      mainOverlay.Add( mToolbarOverlay.get() );
   
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
      mHUDOverlay->Add(mCompass.get());
      mCompass->SetAlignment( SimCore::Components::HUDAlignment::LEFT_BOTTOM );
   
      // GPS Meter
      mGPS = new SimCore::Components::StealthGPSMeter("StealthGPSMeter");
      mHUDOverlay->Add(mGPS.get());
      mGPS->SetPosition( 512.0f/1920.0f, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );

      // MGRS Meter
      mMGRSMeter = new SimCore::Components::StealthMGRSMeter("StealthMGRSMeter");
      mHUDOverlay->Add(mMGRSMeter.get());
      mMGRSMeter->SetPosition( 512.0f/1920.0f, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mMGRSMeter->SetVisible( false );

      // Cartesian Meter
      mCartesianMeter = new SimCore::Components::StealthCartesianMeter("StealthCartesianMeter");
      mHUDOverlay->Add(mCartesianMeter.get());
      mCartesianMeter->SetPosition( 512.0f/1920.0f, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mCartesianMeter->SetVisible( false );

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
   void StealthHUD::TickHUD()
   {
      int x(0), y(0), w(0), h(0);
      GetMainDeltaWindow()->GetPosition(&x, &y, &w, &h);
   //   float curYPos;
   
      // update the compass
      if(mMotionModel.valid() && mMotionModel->GetTarget() != NULL)
      {
         dtCore::Transform xform;
         mMotionModel->GetTarget()->GetTransform(xform);
         const osg::Vec3& pos = xform.GetTranslation();//mMotionModel->GetPosition();
         osg::Vec3 hpr;
         xform.GetRotation(hpr);

         // Update the compass
         mCompass->SetValue(hpr[0], 180.0f, -180.0f);

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

            mCartesianMeter->SetX(pos[0]);
            mCartesianMeter->SetY(pos[1]);
            mCartesianMeter->SetZ(pos[2]);
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
            mGPS->SetLatLong(globePos[0], globePos[1]);
         }
      }
   
      if(GetHUDState() != SimCore::Components::HUDState::HELP)
      {
         char clin[HUDCONTROLMAXTEXTSIZE]; // general buffer to print
   
         if( mLogController.valid() 
            && mLastLogState != &mLogController->GetLastKnownStatus().GetStateEnum() )
         {
            mLastLogState = &mLogController->GetLastKnownStatus().GetStateEnum();
   
            // Playback State
            if (dtGame::LogStateEnumeration::LOGGER_STATE_IDLE == *mLastLogState )
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
         } 

         // Set the time control to the basic sim time 
         snprintf(clin, HUDCONTROLMAXTEXTSIZE, "%.2f", GetGameManager()->GetSimulationTime());
         mSimTimeAndState->SetText2( std::string(clin) );
   
   
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
}
