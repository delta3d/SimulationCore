/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
 * Chris Rodgers
 */

#ifndef NETDEMO_GUI_COMPONENT_H
#define NETDEMO_GUI_COMPONENT_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include "DemoExport.h"
#include "Actors/PlayerStatusActor.h"
#include "Components/GameLogicComponent.h"

#include <CEGUI/CEGUIVersion.h>

#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
#include <dtGUI/ceuidrawable.h>
#else
#include <dtGUI/gui.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace CEGUI
{
   class Editbox;
   class EventArgs;
   class ItemListbox;
   class PushButton;
   class Window;
}

namespace dtGame
{
   class ActorUpdateMessage;
   class Message;
   class TickMessage;
}

namespace dtGUI
{
   class ScriptModule;
}

namespace SimCore
{
   namespace Components
   {
      class GameStateChangedMessage;
      class HUDGroup;
   }

   namespace GUI
   {
      class Screen;
      class SimpleScreen;
   }
}

namespace NetDemo
{
   namespace GUI
   {
      class HUDScreen;
      class MainMenuScreen;
      class ReadyRoomScreen;
      class ScoreLabelManager;

      namespace Effects
      {
         class ButtonHighlight;
      }
   }
}



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // TYPE DEFINITIONS (Use to switch an external class to this namespace)
   /////////////////////////////////////////////////////////////////////////////
   typedef SimCore::GUI::SimpleScreen SimpleScreen;
   typedef SimCore::GUI::Screen Screen;
   typedef SimCore::Components::StateType GameStateType;
   typedef NetDemo::GUI::Effects::ButtonHighlight ButtonHighlight;



   /////////////////////////////////////////////////////////////////////////////
   // CLASS CODE
   /////////////////////////////////////////////////////////////////////////////
   class NETDEMO_EXPORT GUIComponent : public dtGame::GMComponent
   {
      public:
         typedef dtGame::GMComponent BaseClass;

         static const dtUtil::RefString DEFAULT_NAME;

         GUIComponent( const std::string& name = DEFAULT_NAME.Get() );

         void Initialize();

         void ShowMouseCursor( bool enable );

         virtual void ProcessMessage( const dtGame::Message& message );

         void Update(float timeDelta);

      protected:
         virtual ~GUIComponent();

         void InitializeCEGUI(const std::string& schemeFile);
         void InitializeEffectsOverlays();

         void ProcessEntityActionMessage(const dtGame::Message& message);

         void ProcessActorUpdate(const dtGame::ActorUpdateMessage& updateMessage);

         void ProcessStateChangeMessage(const SimCore::Components::GameStateChangedMessage& stateChange);

         void ProcessPlayerStatusUpdate(const PlayerStatusActor& playerStats);

         GameLogicComponent* GetAppComponent();

         void OnOptionNext(bool reverse = false);
         void OnOptionSelect();

         /**
          * Helper method for obtaining a CEGUI window from its associated Event Args.
          * @param args Event Args object that should have a reference to the window that triggered the event.
          * @return CEGUI window/widget associated with the event.
          */
         const CEGUI::Window* GetWidgetFromEventArgs( const CEGUI::EventArgs& args ) const;

         bool OnButtonClicked(const CEGUI::EventArgs& args);
         bool OnButtonFocusGain(const CEGUI::EventArgs& args);
         bool OnButtonFocusLost(const CEGUI::EventArgs& args);
         bool OnVehicleTypeSelected(const CEGUI::EventArgs& args);
         bool OnServerModeSelected(const CEGUI::EventArgs& args);
         bool OnDifficultySelected(const CEGUI::EventArgs& args);

         void SetButtonFocused(const CEGUI::Window* button);

         void HandleButton(const CEGUI::Window& button);
         void HandleSpecialButton(const std::string& buttonType, std::string& inOutAction);

         void BindButtons( CEGUI::Window& rootWindow );
         void BindButton( CEGUI::PushButton& button );

         bool IsButtonType( const std::string& buttonType ) const;

         void UpdateButtonArray();

         bool RegisterScreenWithState(Screen& screen, GameStateType& state);
         Screen* GetScreenForState(const GameStateType& state);

         void SetHoverEffectOnElement(const CEGUI::Window& window);
         void SetHoverEffectEnabled(bool enabled);

      private:
         // High Order Objects
         std::weak_ptr<PlayerStatusActor> mPlayer;
         std::weak_ptr<GameLogicComponent> mAppComp;
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
         std::shared_ptr<dtGUI::CEUIDrawable> mGUI;
#else
         std::shared_ptr<dtGUI::GUI> mGUI;
#endif
         std::shared_ptr<SimCore::Components::HUDGroup> mMainWindow;
         dtGUI::ScriptModule* mScriptModule;
         std::shared_ptr<NetDemo::GUI::ScoreLabelManager> mScoreLabelManager;

         //
         typedef std::map<const GameStateType*, Screen*> StateScreenMap;
         StateScreenMap mStateScreenMap;

         // Screens
         osg::ref_ptr<osg::MatrixTransform> mBackground;
         std::shared_ptr<NetDemo::GUI::MainMenuScreen> mScreenMainMenu;
         std::shared_ptr<SimpleScreen> mScreenLobby;
         std::shared_ptr<SimpleScreen> mScreenConnectFailPrompt;
         std::shared_ptr<SimpleScreen> mScreenLoading;
         std::shared_ptr<NetDemo::GUI::ReadyRoomScreen> mScreenReadyRoom;
         std::shared_ptr<SimpleScreen> mScreenGarage;
         std::shared_ptr<SimpleScreen> mScreenOptions;
         std::shared_ptr<SimpleScreen> mScreenQuitPrompt;
         std::shared_ptr<NetDemo::GUI::HUDScreen> mScreenHUD;

         std::shared_ptr<Screen> mCurrentScreen;
         std::shared_ptr<Screen> mPreviousScreen;

         // Special Effects Overlays
         osg::ref_ptr<osg::MatrixTransform> mEffectsOverlay;

         // Special Effects Elements
         std::shared_ptr<ButtonHighlight> mButtonHighlight;

         // Special Widgets
         const CEGUI::Window* mCurrentHoveredWidget;
         CEGUI::Editbox* mInputServerPort;
         CEGUI::Editbox* mInputServerIP;
         CEGUI::ItemListbox* mListVehicleType;
         CEGUI::ItemListbox* mServerMode;
         CEGUI::ItemListbox* mDifficulty;

         // Button array - for current screen.
         int mCurrentButtonIndex;
         typedef std::vector<CEGUI::PushButton*> ButtonArray;
         ButtonArray mCurrentScreenButtons;
   };

}

#endif
