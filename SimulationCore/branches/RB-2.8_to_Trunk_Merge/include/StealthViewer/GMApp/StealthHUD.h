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
#ifndef STEALTH_HUD_H_
#define STEALTH_HUD_H_


#include <dtABC/application.h>
#include <dtGame/logstatus.h>
#include <dtGame/message.h>
#include <dtGame/gmcomponent.h>
#include <dtGame/logcontroller.h>
#include <dtUtil/coordinates.h>
#include <dtUtil/enumeration.h>
#include <osg/Referenced>
#include <SimCore/Components/BaseHUD.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/Components/StealthHUDElements.h>
#include <SimCore/Components/LabelManager.h>
#include <SimCore/StealthMotionModel.h>
#include <StealthViewer/GMApp/Export.h>
//This is for the CEGUI headers.
#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>

#define HUDCONTROLMAXTEXTSIZE 100

namespace SimCore
{
   class ToolMessage;
   class UnitOfAngle;
   class UnitOfLength;

   namespace Tools
   {
      class Compass360;
   }
}

namespace StealthGM
{

   class STEALTH_GAME_EXPORT CoordSystem : public dtUtil::Enumeration
   {
      DECLARE_ENUM(CoordSystem);

      public:

         static const CoordSystem MGRS;
         static const CoordSystem RAW_XYZ;
         static const CoordSystem LAT_LON;

      private:

         CoordSystem(const std::string &name) : dtUtil::Enumeration(name)
         {

         }
   };

   /**
    * This class draws the HUD for the StealthApp with using CEGUI.  It draws
    * status information like AAR state (record, playback, idle), sim time,
    * speed factor, num messages, and other help info etc...
    */
   class STEALTH_GAME_EXPORT StealthHUD : public SimCore::Components::BaseHUD
   {
   public:
      typedef dtGame::GMComponent BaseClass;
      static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;
      //default component name.
      static const std::string DEFAULT_NAME;

      /**
       * Constructs the class.
       */
      StealthHUD(dtGame::LogController* logController = NULL,
                 dtCore::SystemComponentType& type = *TYPE,
                 bool hasUI = false);

      /**
       * Destroys the class.
       */
      virtual ~StealthHUD();

      /**
       * Get messages from the GM component
       */
      virtual void ProcessMessage(const dtGame::Message& message);

      /**
       * Sets up the basic GUI.
       */
      //void SetupGUI(dtCore::DeltaWin *win);
      void SetupGUI( SimCore::Components::HUDGroup& hudOverlay, unsigned int designedResWidth = 1920, unsigned int designedResHeight = 1200 );

      /**
       * Cycles HUD state to the next most data.  From minimal, to moderate, to max,
       * and then back to minimal.
       */
      SimCore::Components::HUDState &CycleToNextHUDState();

      void TickHUD();

      void SetMotionModel( dtCore::MotionModel* motionModel );
      void SetCoordinateConverter(const dtUtil::Coordinates& converter) { mCoordinateConverter = converter; }

      void UpdateCompassHeading( float heading );

      // Used to change the key bindings when in physics mode.
      //void SetUsePhysicsDemoMode(bool usePhysicsDemoMode) { mUsePhysicsDemoMode = usePhysicsDemoMode; }

      // This function allows the game entry point to enable certain buttons.
      // @param buttonName The name of the button as it appears in Toolbar.imageset
      // @param keyLabel The name of the key label as it appears in KeyLabels.imageset
      // @param enable Set the new button in enabled or disabled state; enabled by default
      // @return success of operation
      bool AddToolButton( const std::string& buttonName, const std::string& keyLabel, bool enable = true );

      // This function exists as a compliment to AddToolButton.
      // @param buttonName The name of the button as it appears in Toolbar.imageset
      // @return success of operation
      bool RemoveToolButton( const std::string& buttonName );

      // Access the HUDGroup CEGUI window devoted to the display of tools
      CEGUI::Window* GetToolsWindow();
      const CEGUI::Window* GetToolsWindow() const;

      void SetToolbarVisible( bool visible );

      /**
       * Returns the coordinate converter
       * @return mCoordinateConverter
       */
      dtUtil::Coordinates& GetCoordinateConverter() { return mCoordinateConverter; }

      void SetCoordinateSystem(const CoordSystem &system) { mCoordSystem = &system; }
      const CoordSystem& GetCoordinateSystem() const { return *mCoordSystem; }

      void SetHelpEnabled( bool enabled );
      bool IsHelpEnabled() const;

      SimCore::Components::LabelManager& GetLabelManager();
      const SimCore::Components::LabelManager& GetLabelManager() const;

      /// Set the unit to use for the length display.
      void SetUnitOfLength(SimCore::UnitOfLength& unit);
      /// Get the unit to use for the length display.
      SimCore::UnitOfLength& GetUnitOfLength() const;

      /// Set the unit to use for the angle display.
      void SetUnitOfAngle(SimCore::UnitOfAngle& unit);

      /// Get the unit to use for the angle display.
      SimCore::UnitOfAngle& GetUnitOfAngle() const;

      void SetupCompass360();
      bool HasCompass360() const;

      void SetCompass360Enabled(bool enable);
      bool IsCompass360Enabled() const;

   protected:

      void InitHelpOverlay( SimCore::Components::HUDGroup& hudOverlay );

      /**
       * Makes sure to enable/disable controls as appropriate for each state.
       */
      void UpdateState();

      /**
       * Utility method to set the text, position, and color of a text control
       * Check to see if the data changed.  The default values for color and position
       * won't do anything since they use a color and position < 0.
       */
      void UpdateStaticText(SimCore::Components::HUDText *textControl, char *newText,
         float red = -1.0, float blue = -1.0, float green = -1.0,
         float x = -1, float y = -1);

      void ProcessToolMessage(const SimCore::ToolMessage& toolMessage);

   private:

      /**
       * During the tickHUD() - update our medium data
       */
      void UpdateHighDetailData();

      /**
       * Ensures that the help button is toggled on or off based
       * on the current state of the HUD.
       */
      void UpdateHelpButton();

      /**
       * Utility method to create text
       */
      SimCore::Components::HUDText* CreateText(const std::string &name, const std::string &text,
         float x, float y, float width, float height);

      SimCore::Components::HUDState *mLastHUDStateBeforeHelp;

      dtCore::RefPtr<dtGame::LogController> mLogController;

      // Overlays
      dtCore::RefPtr<SimCore::Components::HUDGroup> mHUDOverlay;
      dtCore::RefPtr<SimCore::Components::HUDGroup> mToolbarOverlay;
      dtCore::RefPtr<SimCore::Components::StealthCompassMeter> mCompass;
      dtCore::RefPtr<SimCore::Components::StealthCartesianMeter> mGPS;
      dtCore::RefPtr<SimCore::Components::StealthMGRSMeter> mMGRSMeter;
      dtCore::RefPtr<SimCore::Components::StealthCartesianMeter> mCartesianMeter;
      dtCore::RefPtr<SimCore::Components::StealthToolbar> mToolbar;
      dtCore::RefPtr<SimCore::Components::HUDGroup> mToolsLayer;

      // Main info
      dtCore::RefPtr<SimCore::Components::HUDText> mStateText;
      dtCore::RefPtr<SimCore::Components::HUDText> mSimTimeText;
      dtCore::RefPtr<SimCore::Components::StealthGPSMeter> mSimTimeAndState;

      // Help Screen HUD Elements
      dtCore::RefPtr<SimCore::Components::HUDGroup> mHelpOverlay;
      dtCore::RefPtr<SimCore::Components::StealthButton> mHelpButton;
      dtCore::RefPtr<SimCore::Components::HUDText> mHelpText;

      // Label Manager
      dtCore::RefPtr<SimCore::Components::LabelManager> mLabelManager;
      dtCore::RefPtr<SimCore::Components::HUDElement> mLabelLayer;

      // Tools
      dtCore::RefPtr<SimCore::Tools::Compass360> mCompass360;

      // The HUD will need a reference to the motion model
      // in order to get all data that needs to be displayed.
      dtCore::ObserverPtr<dtCore::MotionModel> mMotionModel;
      dtUtil::Coordinates mCoordinateConverter;

      SimCore::UnitOfLength* mUnitOfLength;
      SimCore::UnitOfAngle* mUnitOfAngle;

      float mRightTextXOffset;
      float mTextYTopOffset;
      float mTextYSeparation;
      float mTextHeight;

      bool mZoomToolEnabled;
      bool mCompass360WasEnabled;

      bool mUsePhysicsDemoMode;

      const dtGame::LogStateEnumeration* mLastLogState;

      const CoordSystem *mCoordSystem;

      bool mHasUI;
   };
}
#endif
