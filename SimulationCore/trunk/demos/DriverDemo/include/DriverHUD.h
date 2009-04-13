/* -*-c++-*-
* Driver Demo - DriverHUD (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* @author Curtiss Murphy
*/
#ifndef DRIVER_HUD_H_
#define DRIVER_HUD_H_


#include "DriverExport.h"
#include <dtABC/application.h>
#include <dtGame/logstatus.h>
#include <dtUtil/enumeration.h>
#include <osg/Referenced>

//This is for the CEGUI headers.
/*#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>
#include <dtGUI/dtgui.h>//*/

#define HUDCONTROLMAXTEXTSIZE 100

#include <SimCore/Components/BaseHUD.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/DamageHelper.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <dtCore/observerptr.h>
#include <dtUtil/coordinates.h>

#include <GameAppComponent.h>

#include <cmath>
////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtGame
{
   class GameManager;
   class GameActorProxy;
   class TickMessage;
}

namespace SimCore
{
   namespace Actors
   {
      //class BasePhysicsVehicleActor; //class NxAgeiaFourWheelVehicleActor;
      class WeaponActor;
   }
   namespace Components
   {
      class StealthAmmoMeter;
      class StealthButton;
      class StealthCompassMeter;
      class StealthCallSign;
      class StealthCartesianMeter;
      class StealthGPSMeter;
      class StealthHealthMeter;
      class StealthMGRSMeter;
      class StealthSpeedometer;
      class StealthToolbar;
   }
}

namespace DriverDemo
{
   class GameAppComponent;
   /**
    * This class draws the HUD for the StealthApp with using CEGUI.  It draws
    * status information like AAR state (record, playback, idle), sim time,
    * speed factor, num messages, and other help info etc...
    */
   class DRIVER_DEMO_EXPORT DriverHUD : public SimCore::Components::BaseHUD
   {
      public:
         typedef SimCore::Components::BaseHUD BaseClass;
         
         static const float REDRAW_TIME; // the amount of time to wait before recomputing HUD controls
   
         class CoordSystem : public dtUtil::Enumeration
         {
            DECLARE_ENUM(CoordSystem);
   
            public:
   
               static const CoordSystem MGRS;
               static const CoordSystem RAW_XYZ;
               static const CoordSystem LAT_LON;
   
            private:
   
               CoordSystem( const std::string &name );
         };
   
         /**
          * Constructs the class.
          */
         DriverHUD( dtCore::DeltaWin *win,
            const std::string& ceguiScheme = "CEGUI/schemes/DriverDemo.scheme",
            bool usePhysicsDemoMode = false );
   
         static const std::string DEFAULT_NAME;
   
         /**
          * Destroys the class.
          */
         virtual ~DriverHUD();
   
         /**
          * Get messages from the GM component
          */
         virtual void ProcessMessage( const dtGame::Message& message );
   
         /**
          * Sets up the basic GUI.
          */
         virtual void SetupGUI( SimCore::Components::HUDGroup& hudOverlay, 
                  unsigned int designedResWidth, unsigned int designedResHeight );
   
         /**
          * Cycles HUD state to the next most data.  From minimal, to moderate, to max,
          * and then back to minimal.
          */
         SimCore::Components::HUDState &CycleToNextHUDState();
   
         virtual void TickHUD(const dtGame::TickMessage &tick);
   
         void SetCallSign( const std::string& callSign );
         std::string GetCallSign() const;

         /**
          * Change the function key label of a tool button.
          * @param buttonName The name of the button as it appears in Toolbar.imageset
          * @param keyLabel The name of the key label as it appears in KeyLabels.imageset
          * @return success of operation
          */
         bool SetToolButtonKeyLabel( const std::string& buttonName, const std::string& keyLabel );
   
         /**
          * This function allows the game entry point to enable certain buttons.
          * @param buttonName The name of the button as it appears in Toolbar.imageset
          * @param keyLabel The name of the key label as it appears in KeyLabels.imageset
          * @param enabled Default state at which the button should be created.
          * @return success of operation
          */
         bool AddToolButton( const std::string& buttonName, const std::string& keyLabel, bool enabled = true );
         
         /**
          * This function exists as a compliment to AddToolButton.
          * @param buttonName The name of the button as it appears in Toolbar.imageset
          * @return success of operation
          */
         bool RemoveToolButton( const std::string& buttonName );
   
         /**
          * Get a button that was added to the HUD toolbar.
          * @param buttonName The name of the button as it appears in Toolbar.imageset
          * @return button that has the name buttonName; NULL if not found.
          */
         const SimCore::Components::StealthButton* GetToolButton( const std::string& buttonName ) const;
   
         CEGUI::Window* GetToolsWindow();
         const CEGUI::Window* GetToolsWindow() const;
   
         void SetVehicle( SimCore::Actors::BasePhysicsVehicleActor* vehicle );
         SimCore::Actors::BasePhysicsVehicleActor* GetVehicle();
   
         void SetWeapon( SimCore::Actors::WeaponActor* weapon );
   
         void SetDamageHelper( SimCore::Components::DamageHelper* damageHelper )
         {
            mDamageHelper = damageHelper;
         }
   
         void SetCoordinateSystem( const DriverHUD::CoordSystem &system )
         {
            mCoordSystem = &system;
         }
         
         const DriverHUD::CoordSystem& GetCoordinateSystem() const
         {
            return *mCoordSystem;
         }
   
         void SetCoordinateConverter( const dtUtil::Coordinates& converter );

         dtUtil::Coordinates& GetCoordinateConverter()
         {
            return mCoordinateConverter;
         }
         
         const dtUtil::Coordinates& GetCoordinateConverter() const
         {
            return mCoordinateConverter;
         }
   
         void CycleCoordinateSystem();
   
         void SetHelpEnabled( bool enabled );
         bool IsHelpEnabled() const;
      
      protected:
   
         void InitHelpOverlay( SimCore::Components::HUDGroup& hudOverlay );
   
         dtCore::RefPtr<SimCore::Components::HUDText> CreateText(const std::string &name, const std::string &text, 
            float x, float y, float width, float height );
   
         /**
          * Makes sure to enable/disable controls as appropriate for each state.
          */
         //void UpdateState();
   
      private:
         SimCore::Components::HUDState* mLastHUDStateBeforeHelp;
     
         bool mUsePhysicsDemoMode;
         bool mEmbarkMode;
         bool mEmbarkFlash;
         float mFlashToggleTime;
   
         // Coordinate related variables
         const DriverHUD::CoordSystem* mCoordSystem;
         dtUtil::Coordinates mCoordinateConverter;
   
         // NEW HUD ELEMENTS
         dtCore::RefPtr<SimCore::Components::StealthToolbar> mToolbar;
         dtCore::RefPtr<SimCore::Components::StealthHealthMeter> mHealthMeter;
         dtCore::RefPtr<SimCore::Components::StealthAmmoMeter> mAmmoMeter;
         dtCore::RefPtr<SimCore::Components::StealthCompassMeter> mCompassMeter;
         dtCore::RefPtr<SimCore::Components::StealthCartesianMeter> mGPSMeter;
         dtCore::RefPtr<SimCore::Components::StealthMGRSMeter> mMGRSMeter;
         dtCore::RefPtr<SimCore::Components::StealthCartesianMeter> mCartesianMeter;
         //dtCore::RefPtr<SimCore::Components::StealthCallSign> mCallSign;
         //dtCore::RefPtr<SimCore::Components::StealthSpeedometer> mSpeedometer;
         dtCore::RefPtr<SimCore::Components::StealthGPSMeter> mSpeedometer;
         dtCore::RefPtr<SimCore::Components::HUDGroup> mToolsLayer;
         dtCore::RefPtr<SimCore::Components::StealthGPSMeter> mSimTimeMeter;
   
         dtCore::RefPtr<SimCore::Components::HUDText> mWeaponName; // temporary to show weapon name
         dtCore::RefPtr<SimCore::Components::HUDText> mCoordinatesLabel; // temporary to show weapon name
   
         // Help Screen HUD Elements
         dtCore::RefPtr<SimCore::Components::HUDGroup> mHelpOverlay;
         dtCore::RefPtr<SimCore::Components::StealthButton> mHelpButton;
         dtCore::RefPtr<SimCore::Components::HUDText> mHelpText_Commander;
         dtCore::RefPtr<SimCore::Components::HUDText> mHelpText_Driver;
         dtCore::RefPtr<SimCore::Components::HUDText> mHelpText_Gunner;
         dtCore::RefPtr<SimCore::Components::HUDText> mHelpText_Soldier;
   
         // References to objects that pass data to the HUD for display
         // --- for speedometer and health
         dtCore::ObserverPtr<SimCore::Actors::BasePhysicsVehicleActor> mVehicle;
   
         // --- for ammo meter
         dtCore::ObserverPtr<SimCore::Actors::WeaponActor> mWeapon;
   
         // --- for the health meter, for the current player entity (vehicle or human)
         dtCore::ObserverPtr<SimCore::Components::DamageHelper> mDamageHelper;
      
         float mTimeTillNextHUDUpdate;
   };
}
#endif
