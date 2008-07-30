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
 * @author Chris Rodgers
 */

#ifndef _BASE_HUD_H_
#define _BASE_HUD_H_

#include <SimCore/Export.h>
#include <dtCore/base.h>
#include <dtCore/deltawin.h>
#include <dtGame/message.h>
#include <dtGame/gmcomponent.h>
#include <dtUtil/enumeration.h>
#include <SimCore/Components/BaseHUDElements.h>

namespace dtGUI
{
   class BaseScriptModule;
}

namespace SimCore
{
   namespace Components
   {

      /**
      * Exception that may be thrown by the HUD.
      */
      class BaseHUDException : public dtUtil::Enumeration
      {
         DECLARE_ENUM(BaseHUDException);
      public:
         static BaseHUDException INIT_ERROR;
         static BaseHUDException RUNTIME_ERROR;
      private:
         BaseHUDException(const std::string &name) : dtUtil::Enumeration(name)
         {
            AddInstance(this);
         }
      };

      /**
      * HUD State enumeration - what info is the HUD showing.
      */
      class SIMCORE_EXPORT HUDState : public dtUtil::Enumeration
      {
         DECLARE_ENUM(HUDState);
      public:
         static HUDState MINIMAL;
         static HUDState MAXIMUM;
         static HUDState NONE;
         static HUDState HELP;
      private:
         HUDState(const std::string &name) : dtUtil::Enumeration(name)
         {
            AddInstance(this);
         }
      };


      class SIMCORE_EXPORT BaseHUD : public dtGame::GMComponent
      {
      public:
         BaseHUD( dtCore::DeltaWin* win, const std::string& name = "BaseHUD",
            const std::string& ceguiScheme = "CEGUI/schemes/WindowsLook.scheme" );

         virtual ~BaseHUD();

         // Does additional setup prior to calling the virtual function SetupGUI.
         // This MUST be called before the HUD is used.
         // The parameters passed in represent the resolution at which the
         // HUD elements and layout were designed, and aid in setup of
         // elements in relative position and relative size modes.
         void Initialize( unsigned int designedResWidth = 1920, unsigned int designedResHeight = 1200);

         CEGUI::Window* GetMainCEGUIWindow();
         const CEGUI::Window* GetMainCEGUIWindow() const;

         dtCore::RefPtr<dtGUI::CEUIDrawable> GetGUIDrawable() { return mGUI; }
         const dtCore::RefPtr<dtGUI::CEUIDrawable> GetGUIDrawable() const { return mGUI; }

         virtual void SetHUDState(HUDState& newState) { mHUDState = &newState;  UpdateState(); }
         HUDState& GetHUDState() { return *mHUDState; }
         const HUDState& GetHUDState() const { return *mHUDState; }

         // Get messages from the GM component
         virtual void ProcessMessage(const dtGame::Message& message);

         // This function should be overridden for classes extending this class.
         // Sub classes SHOULD also call this version prior to
         virtual void SetupGUI( HUDGroup& hudOverlay, unsigned int designedResWidth, unsigned int designedResHeight );

         virtual void TickHUD();

         dtCore::DeltaWin* GetMainDeltaWindow() { return mWin; }
         const dtCore::DeltaWin* GetMainDeltaWindow() const { return mWin; }

      protected:

         /**
          * Makes sure to enable/disable controls as appropriate for each state.
          */
         virtual void UpdateState();

         virtual void RegisterCallbacks();

         void InitializeCEGUI();

         virtual void OnAddedToGM();

      private:
         dtCore::DeltaWin* mWin;
         dtCore::RefPtr<HUDGroup> mMainWindow;
         dtCore::RefPtr<dtGUI::CEUIDrawable> mGUI;
         dtGUI::BaseScriptModule* mScriptModule;

         HUDState* mHUDState;

         std::string mSchemeFile;
      };

   }
}

#endif
