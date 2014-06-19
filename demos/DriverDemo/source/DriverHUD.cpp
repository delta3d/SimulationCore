/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author Curtiss Murphy
*/

#include <DriverHUD.h>

#include <dtCore/object.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>

#include <dtUtil/datetime.h>
#include <dtUtil/exception.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/macros.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/datapathutils.h>

#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/binarylogstream.h>
#include <dtGame/defaultmessageprocessor.h>
#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>
#include <dtGame/serverloggercomponent.h>

#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/map.h>
#include <dtCore/actorproxy.h>
#include <dtCore/transformableactorproxy.h>

#include <dtActors/taskactor.h>
#include <dtActors/coordinateconfigactor.h>
#include <dtActors/engineactorregistry.h>

//#include <DVTE/Actors/DVTEActorRegistry.h>

#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <SimCore/Components/StealthHUDElements.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/WeaponActor.h>

#include <CEGUI/CEGUIVersion.h>

#include <ctime>
#include <cmath>


namespace DriverDemo
{
   ////////////////////////////////////////////////////////////////////////////////
   const std::string DriverHUD::DEFAULT_NAME("DriverHUD");
   const float DriverHUD::REDRAW_TIME = 0.1f; // recomputes the HUD a few times per second

   ////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(DriverHUD::CoordSystem);
   const DriverHUD::CoordSystem DriverHUD::CoordSystem::MGRS("MGRS");
   const DriverHUD::CoordSystem DriverHUD::CoordSystem::RAW_XYZ("RAW_XYZ");
   const DriverHUD::CoordSystem DriverHUD::CoordSystem::LAT_LON("LAT_LON");

   ////////////////////////////////////////////////////////////////////////////////
   DriverHUD::DriverHUD(dtCore::DeltaWin *win,
                          const std::string& ceguiScheme, bool usePhysicsDemoMode)
   : BaseClass(win, DEFAULT_NAME, ceguiScheme),
      mLastHUDStateBeforeHelp(&SimCore::Components::HUDState::MINIMAL),
      mUsePhysicsDemoMode(usePhysicsDemoMode),
      mFlashToggleTime(0.0f),
      mCoordSystem(&CoordSystem::LAT_LON),
      mTimeTillNextHUDUpdate(REDRAW_TIME)
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   DriverHUD::~DriverHUD()
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   DriverHUD::CoordSystem::CoordSystem( const std::string &name ) : dtUtil::Enumeration(name)
   {
      AddInstance(this);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::SetCoordinateConverter( const dtUtil::Coordinates& converter )
   {
      mCoordinateConverter = converter;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::SetWeapon( SimCore::Actors::WeaponActor* weapon )
   {
      mWeapon = weapon;
      mWeaponName->SetText( mWeapon.valid() ? mWeapon->GetName() : "" );
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& type = message.GetMessageType();

      if( type == dtGame::MessageType::TICK_LOCAL)
      {
         TickHUD(static_cast<const dtGame::TickMessage&>(message));
      }

      else if (type == dtGame::MessageType::INFO_ACTOR_CREATED
            || type == dtGame::MessageType::INFO_ACTOR_UPDATED )
      {
         // Do nothing for now
      }

      else if( type == dtGame::MessageType::INFO_ACTOR_DELETED )
      {
         // Do nothing for now
      }

      /*
      // Tool code is left here as examples of how to manipulate the toolbar
      else if( type == SimCore::MessageType::NIGHT_VISION)
      {
         SetMapToolEnabled(false);
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = dynamic_cast<const SimCore::ToolMessage&> (message);
         mToolbar->SetButtonActive("NightVision",toolMsg.IsEnabled());
      }
      else if( type == SimCore::MessageType::COMPASS)
      {
         SetMapToolEnabled(false);
         mToolbar->SetButtonsActive(false);
         const SimCore::ToolMessage& toolMsg = dynamic_cast<const SimCore::ToolMessage&> (message);
         mToolbar->SetButtonActive("Compass",toolMsg.IsEnabled());
      }
      */
      else if ( type == dtGame::MessageType::INFO_MAP_LOADED)
      {
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
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
            GetGameManager()->GetScene().AddDrawable(GetGUIDrawable().get());
         }
#endif

         std::vector<dtCore::ActorProxy*> proxies;
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


      // Update the help button back to enabled if help is still visible.
      // When a tool button is activated, all others are disabled.
      if( mHelpButton.valid() && ! mHelpButton->IsActive() && IsHelpEnabled() )
      {
         mHelpButton->SetActive( true );
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::SetupGUI( SimCore::Components::HUDGroup& hudOverlay, unsigned int designedResWidth, unsigned int designedResHeight )
   {
      hudOverlay.GetCEGUIWindow()->setMouseInputPropagationEnabled(true);
      // Create the layer on which tools will render.
      // This is added first since the main HUD will need
      // to be drawn over the tools' GUIs.
      mToolsLayer = new SimCore::Components::HUDGroup("ToolsLayer");
      mToolsLayer->GetCEGUIWindow()->disable(); // avoids mouse clicks that change z-order
      hudOverlay.Add( mToolsLayer.get() );

      // Add tool bar
      mToolbar = new SimCore::Components::StealthToolbar("DriverToolbar");
      hudOverlay.Add(mToolbar.get());
      mToolbar->SetAlignment( SimCore::Components::HUDAlignment::CENTER_TOP );
      mToolbar->AddButton("Help","F1",true);

      // --- Maintain a reference to the toolbar's help button
      //     so that it can be updated properly.
      mHelpButton = mToolbar->GetButton("Help");
      mHelpButton->SetActive( false );


      const float METER_WIDTH = 256.0f;
      const float SCREEN_WIDTH = 1920.0f;
      const float SCREEN_HEIGHT = 1200.0f;
      float leftOffset = 0.0f;

      // Call Sign Display
      //mCallSign = new SimCore::Components::StealthCallSign("DriverCallSign");
      //mCallSign->SetAlignment( SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      //hudOverlay.Add( mCallSign.get() );

      // Sim Time Meter
      //leftOffset += METER_WIDTH;
      mSimTimeMeter = new SimCore::Components::StealthGPSMeter("SimTimeMeter");
      mSimTimeMeter->SetAlignment( SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      //mSimTimeMeter->SetPosition( leftOffset/SCREEN_WIDTH, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mSimTimeMeter->SetText1("Clock");
      mSimTimeMeter->GetText1().SetColor(0.0f, 0.0f, 0.5f);
      mSimTimeMeter->GetText2().SetColor(0.2f, 0.2f, 0.2f);
      hudOverlay.Add( mSimTimeMeter.get() );

      // Compass Meter
      leftOffset += METER_WIDTH;
      mCompassMeter = new SimCore::Components::StealthCompassMeter("DriverCompassMeter");
      mCompassMeter->Initialize();
      mCompassMeter->SetPosition( leftOffset/SCREEN_WIDTH, 0.0f );
      hudOverlay.Add(mCompassMeter.get());
      mCompassMeter->SetAlignment( SimCore::Components::HUDAlignment::LEFT_BOTTOM );

      // Ammo Meter
      leftOffset += METER_WIDTH;
      mAmmoMeter = new SimCore::Components::StealthAmmoMeter("DriverAmmoMeter");
      mAmmoMeter->Initialize();
      mAmmoMeter->SetPosition( leftOffset/SCREEN_WIDTH, 0.0f, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      hudOverlay.Add( mAmmoMeter.get() );

      // Weapon Name (TEMPORARY)
      mWeaponName = new SimCore::Components::HUDText("WeaponName");
      mWeaponName->SetFontAndText("Arial-Bold-12","",leftOffset/SCREEN_WIDTH, -64.0f/SCREEN_HEIGHT);
      mWeaponName->SetSize(METER_WIDTH/SCREEN_WIDTH,32.0f/SCREEN_HEIGHT);
      mWeaponName->SetAlignment(SimCore::Components::HUDAlignment::LEFT_BOTTOM);
      mWeaponName->SetColor( 1.0f, 1.0f, 0.0f );
      hudOverlay.Add( mWeaponName.get() );

      float rightOffset = 0.0f;

      // Add the speedometer only if this is the driving simulation.
      mSpeedometer = new SimCore::Components::StealthGPSMeter("DriverSpeedometer");
      mSpeedometer->SetAlignment( SimCore::Components::HUDAlignment::RIGHT_BOTTOM );
      mSpeedometer->SetText1("Speed");
      mSpeedometer->GetText1().SetColor(0.0f, 0.0f, 0.5f);
      mSpeedometer->GetText2().SetColor(0.2f, 0.2f, 0.2f);
      //mSpeedometer->RegisterNeedleWithGUI( GetGUIDrawable().get() );
      hudOverlay.Add( mSpeedometer.get() );
      leftOffset += METER_WIDTH;

      rightOffset -= METER_WIDTH;

      // The health and ammo meters currently have no relevance to
      // the current applications. They are temporarily disabled.
      mHealthMeter = new SimCore::Components::StealthHealthMeter("DriverHealthMeter");
      mHealthMeter->Initialize();
      mHealthMeter->SetPosition( rightOffset/SCREEN_WIDTH, 0.0f, SimCore::Components::HUDAlignment::RIGHT_BOTTOM );
      hudOverlay.Add(mHealthMeter.get());
      rightOffset -= METER_WIDTH;

      // Coordinates Label
      mCoordinatesLabel = new SimCore::Components::HUDText("CoordinatesLabel");
      mCoordinatesLabel->SetFontAndText("Arial-Bold-12","",rightOffset/SCREEN_WIDTH, -64.0f/1200.0f);
      mCoordinatesLabel->SetSize(METER_WIDTH/SCREEN_WIDTH,32.0f/SCREEN_HEIGHT);
      mCoordinatesLabel->SetAlignment(SimCore::Components::HUDAlignment::RIGHT_BOTTOM);
      mCoordinatesLabel->SetColor( 1.0f, 1.0f, 0.0f );
      mCoordinatesLabel->SetText("GPS");
      hudOverlay.Add( mCoordinatesLabel.get() );

      // MGRS Meter
      mMGRSMeter = new SimCore::Components::StealthMGRSMeter("DriverMGRSMeter");
      mMGRSMeter->SetPosition( rightOffset/SCREEN_WIDTH, 0.0f, SimCore::Components::HUDAlignment::RIGHT_BOTTOM );
      hudOverlay.Add(mMGRSMeter.get());
      mMGRSMeter->SetVisible( false );

      // GPS Meter - Lat Lon
      // NOTE: 22 is added to shift the meter right since the image is not a multiple
      // of 256. Currently the image is 362.
      rightOffset += 22.0f;
      mGPSMeter = new SimCore::Components::StealthCartesianMeter("DriverLatLonMeter");
      mGPSMeter->SetPosition( rightOffset/SCREEN_WIDTH, 0.0f, SimCore::Components::HUDAlignment::RIGHT_BOTTOM );
      hudOverlay.Add( mGPSMeter.get() );

      // Cartesian Meter
      mCartesianMeter = new SimCore::Components::StealthCartesianMeter("DriverCartesianMeter");
      mCartesianMeter->SetPosition( rightOffset/SCREEN_WIDTH, 0.0f, SimCore::Components::HUDAlignment::RIGHT_BOTTOM );
      hudOverlay.Add(mCartesianMeter.get());
      mCartesianMeter->SetVisible( false );


      // Help Overlay
      InitHelpOverlay( hudOverlay );
      mHelpText_Gunner->SetVisible(true);
      SetHelpEnabled( false );
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::InitHelpOverlay( SimCore::Components::HUDGroup& hudOverlay )
   {
      const float SCREEN_WIDTH = 1920.0f;
      const float SCREEN_HEIGHT = 1200.0f;

      mHelpOverlay = new SimCore::Components::HUDGroup("HelpScreen");
      mHelpOverlay->SetPosition( 0, 0, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      hudOverlay.Add( mHelpOverlay.get() );

      float offsetX = 0.0f;
      float offsetY = 150.0f/SCREEN_HEIGHT;
      SimCore::Components::HUDAlignment* align = &SimCore::Components::HUDAlignment::LEFT_TOP;

      // Create the help text background to give contrast with the scene
      dtCore::RefPtr<SimCore::Components::HUDImage> background = new SimCore::Components::HUDImage("HelpTextClipboard");
      background->SetImage("Clipboard","Clipboard");
      background->SetSize( 350.0f/SCREEN_WIDTH, 500.0f/SCREEN_HEIGHT );
      background->SetPosition( 0.0, -256.0f/SCREEN_HEIGHT, SimCore::Components::HUDAlignment::LEFT_BOTTOM );
      mHelpOverlay->Add( background.get() );


      // Gunner Mode Help Text
      std::string text("\
            * Vehicle Controls * \n\
            \n\
            Mouse - Aim weapon\n\
            Mouse Left Btn - Fire\n\
            Mouse + Right Btn - Turn ring mount\n\
            H - Change Weapon\n\
            Space - Jump\n\
            R - Reset Vehicle\n\
            T - Create Target\n\
            . - Toggle View Mode\n\
            G - Show DR Ghost\n\
            Del - Kill Target (+ Shft=All)\n\
            I & O - Change Time\n\
            F1 - Help\n\
            \n\
            Alt-X - Exit the app\n\
            Esc - Full screen");
            //Mouse + Left Crtl - Look around

      mHelpText_Gunner = CreateText( "HelpText_Gunner", text, offsetX, offsetY, 1.0, 1.0 );
      mHelpText_Gunner->SetAlignment(*align);
      mHelpOverlay->Add( mHelpText_Gunner.get() );
   }

   //////////////////////////////////////////////////////////////////////////
   dtCore::RefPtr<SimCore::Components::HUDText> DriverHUD::CreateText(const std::string &name, const std::string &text,
                                             float x, float y, float width, float height)
   {
      dtCore::RefPtr<SimCore::Components::HUDText> newText = new SimCore::Components::HUDText(name);
      newText->SetFontAndText("Arial-Bold-12",text,x,y);
      newText->SetSize(width,height);
      return newText;
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DriverHUD::SetToolButtonKeyLabel( const std::string& buttonName, const std::string& keyLabel )
   {
      if( ! mToolbar.valid() )
      {
         SimCore::Components::StealthButton* button = mToolbar->GetButton(buttonName);
         if( button != NULL )
         {
            return button->SetKeyLabel( keyLabel );
         }
      }
      return false;
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DriverHUD::AddToolButton( const std::string& buttonName, const std::string& keyLabel, bool enabled )
   {
      if(mToolbar.valid())
      {
         SimCore::Components::StealthButton* button = mToolbar->GetButton(buttonName);
         if( button != NULL )
         {
            std::stringstream ss;
            ss << "Button \"" << buttonName << "\" has already been added to the HUD toolbar." << std::endl;
            LOG_WARNING( ss.str() );
            return false;
         }

         bool success = mToolbar->AddButton( buttonName, keyLabel );
         if( mToolbar->GetButtonCount() > 0 )
         {
            mToolbar->Show();
         }

         button = mToolbar->GetButton(buttonName);
         if( button != NULL ) { button->SetDisabled( ! enabled ); }

         return success;
      }
      return false;
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DriverHUD::RemoveToolButton( const std::string& buttonName )
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

   ////////////////////////////////////////////////////////////////////////////////
   const SimCore::Components::StealthButton* DriverHUD::GetToolButton( const std::string& buttonName ) const
   {
      return mToolbar.valid() ? mToolbar->GetButton(buttonName) : NULL;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::TickHUD(const dtGame::TickMessage &tick)
   {
      // Set the time control to the basic sim time
      mSimTimeMeter->SetText2(
         dtUtil::DateTime::ToString(GetGameManager()->GetSimulationClockTime() / 1000000,
         dtUtil::DateTime::TimeFormat::CLOCK_TIME_24_HOUR_FORMAT) );


      // work out whether we should draw or not
      mTimeTillNextHUDUpdate -= tick.GetDeltaRealTime();
      if (mTimeTillNextHUDUpdate > 0.0)
      {
         return;
      }
      mTimeTillNextHUDUpdate = REDRAW_TIME; // reset timer

      // Update the health meter.
      if( ! mDamageHelper.valid() && mHealthMeter.valid() && mHealthMeter->IsVisible() )
      {
         mHealthMeter->SetVisible( false );
      }
      else if( mDamageHelper.valid() && mHealthMeter.valid() )
      {
         if( ! mHealthMeter->IsVisible() )
         {
            mHealthMeter->SetVisible( true );
         }
         float health = mDamageHelper->GetDamageState() == SimCore::Components::DamageType::DAMAGE_KILL
            ? 0.0f : 1.0f - mDamageHelper->GetCurrentDamageRatio();
         mHealthMeter->SetValue( 100.0f*(health < 0.0f ? 0.0f : health), 100.0f, 0.0f );
      }

      // Update Driver related HUD elements
      SimCore::Actors::BasePhysicsVehicleActor* vehicle = GetVehicle();
      if( vehicle != NULL )
      {
         dtCore::Transform trans;
         vehicle->GetTransform(trans);

         // Get the location of the vehicle
         osg::Vec3 pos;
         trans.GetTranslation(pos);

         if( ! mCoordinatesLabel->IsVisible() ) { mCoordinatesLabel->SetVisible( true ); }

         // Update the appropriate meter with the current coordinates
         if( *mCoordSystem == CoordSystem::MGRS )
         {
            if( ! mMGRSMeter->IsVisible() )
            {
               mCoordinatesLabel->SetText("MGRS");
               mCartesianMeter->SetVisible( false );
               mGPSMeter->SetVisible( false );
               mMGRSMeter->SetVisible( true );
            }

            std::string milgrid = mCoordinateConverter.XYZToMGRS( pos );
            mMGRSMeter->SetText( milgrid );
         }
         else if( *mCoordSystem == CoordSystem::RAW_XYZ )
         {
            if( ! mCartesianMeter->IsVisible() )
            {
               mCoordinatesLabel->SetText("Cartesian");
               mCartesianMeter->SetVisible( true );
               mGPSMeter->SetVisible( false );
               mMGRSMeter->SetVisible( false );
            }

            mCartesianMeter->SetX( pos[0], "M");
            mCartesianMeter->SetY( pos[1], "M" );
            mCartesianMeter->SetZ( pos[2], "M" );
         }
         else
         {
            if( ! mGPSMeter->IsVisible() )
            {
               mCoordinatesLabel->SetText("GPS");
               mCartesianMeter->SetVisible( false );
               mGPSMeter->SetVisible( true );
               mMGRSMeter->SetVisible( false );
            }

            mCoordinateConverter.SetIncomingCoordinateType( dtUtil::IncomingCoordinateType::GEODETIC );
            const osg::Vec3d& globePos = mCoordinateConverter.ConvertToRemoteTranslation( pos );
            mGPSMeter->SetX( globePos[0], "deg" );
            mGPSMeter->SetY( globePos[1], "deg" );
            mGPSMeter->SetZ( globePos[2], "M" );
         }

         // Update the compass HUD element
         osg::Vec3 hpr;
         trans.GetRotation( hpr );
         mCompassMeter->SetValue( hpr[0], 180.0f, -180.0f );

         // Check if speedometer exists. It will be NULL
         // if this is not the driving sim.
         if( mSpeedometer.valid() )
         {
            if(vehicle != NULL)
            {
               mSpeedometer->SetText2(dtUtil::ToString((int) vehicle->GetMPH()));
               //mSpeedometer->SetValue( vehicle->GetMPH(), 60.0f );
            }
            else
               mSpeedometer->SetText2("0");
               //mSpeedometer->SetValue( 0, 60.0f );
         }

      }
      else // no vehicle
      {
         // Turn off coordinate meters
         if( mCoordinatesLabel->IsVisible() ) { mCoordinatesLabel->SetVisible( false ); }
         if( mCartesianMeter->IsVisible() ) { mCartesianMeter->SetVisible( false ); }
         if( mGPSMeter->IsVisible() ) { mGPSMeter->SetVisible( false ); }
         if( mMGRSMeter->IsVisible() ) { mMGRSMeter->SetVisible( false ); }

         // Turn off vehicle compass
         if( ! mCompassMeter->IsVisible() ) { mCompassMeter->SetVisible( false ); }

         // Turn off vehicle compass
         if( ! mSpeedometer->IsVisible() ) { mSpeedometer->SetVisible( false ); }

      }

      // Updated Gunner related HUD elements
      if( mWeapon.valid() )
      {
         if( ! mAmmoMeter->IsVisible() )
         {
            mAmmoMeter->SetVisible( true );
            mWeaponName->SetVisible( true );
         }

         mAmmoMeter->SetValue( mWeapon->GetAmmoCount(), mWeapon->GetAmmoMax(), 0.0f );
      }
      else
      {
         if( mAmmoMeter->IsVisible() )
         {
            mAmmoMeter->SetVisible( false );
            mWeaponName->SetVisible( false );
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::SetVehicle( SimCore::Actors::BasePhysicsVehicleActor* vehicle )
   {
      mVehicle = vehicle;
   }

   ////////////////////////////////////////////////////////////////////////////////
   SimCore::Actors::BasePhysicsVehicleActor* DriverHUD::GetVehicle()
   {
      return mVehicle.get();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::SetHelpEnabled( bool enabled )
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
   bool DriverHUD::IsHelpEnabled() const
   {
      return mHelpOverlay.valid() && mHelpOverlay->IsVisible();
   }

   ////////////////////////////////////////////////////////////////////////////////
   SimCore::Components::HUDState & DriverHUD::CycleToNextHUDState()
   {
      // CURT - I think we can delete this entirely. This is WAAAYYY left over from like 2 apps ago

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


      return GetHUDState();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::SetCallSign( const std::string& callSign )
   {
      //mCallSign->SetCallSign( callSign );
   }

   ////////////////////////////////////////////////////////////////////////////////
   std::string DriverHUD::GetCallSign() const
   {
      return "Error - No Callsign in HUD"; //mCallSign->GetTextElement()->GetText();
   }

   ////////////////////////////////////////////////////////////////////////////////
   CEGUI::Window* DriverHUD::GetToolsWindow()
   {
      return mToolsLayer->GetCEGUIWindow();
   }

   ////////////////////////////////////////////////////////////////////////////////
   const CEGUI::Window* DriverHUD::GetToolsWindow() const
   {
      return mToolsLayer->GetCEGUIWindow();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverHUD::CycleCoordinateSystem()
   {
      if( *mCoordSystem == CoordSystem::LAT_LON )
      {
         mCoordSystem = &CoordSystem::MGRS;
      }
      else if( *mCoordSystem == CoordSystem::MGRS )
      {
         mCoordSystem = &CoordSystem::RAW_XYZ;
      }
      else
      {
         mCoordSystem = &CoordSystem::LAT_LON;
      }
   }

}
