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
#include <dtGame/gmcomponent.h>
#include <GameLogicComponent.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace CEGUI
{
   class Editbox;
   class EventArgs;
   class PushButton;
   class Window;
}

namespace dtGame
{
   class Message;
}

namespace dtGUI
{
   class CEUIDrawable;
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
      class SimpleScreen;
   }
}



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // CLASS CODE
   /////////////////////////////////////////////////////////////////////////////
   class GUIComponent : public dtGame::GMComponent
   {
      public:
         typedef dtGame::GMComponent BaseClass;

         static const dtUtil::RefString DEFAULT_NAME;

         GUIComponent( const std::string& name = DEFAULT_NAME.Get() );

         void Initialize();

         void ShowMouseCursor( bool enable );

         virtual void ProcessMessage( const dtGame::Message& message );

         void ProcessStateChangeMessage( const SimCore::Components::GameStateChangedMessage& stateChange );

      protected:
         virtual ~GUIComponent();

         void InitializeCEGUI( const std::string& schemeFile );

         GameLogicComponent* GetAppComponent();

         /**
          * Helper method for obtaining a CEGUI window from its associated Event Args.
          * @param args Event Args object that should have a reference to the window that triggered the event.
          * @return CEGUI window/widget associated with the event.
          */
         const CEGUI::Window* GetWidgetFromEventArgs( const CEGUI::EventArgs& args ) const;

         bool OnButtonClicked( const CEGUI::EventArgs& args );

         void BindButtons( CEGUI::Window& rootWindow );
         void BindButton( CEGUI::PushButton& button );

         bool IsButtonType( const std::string& buttonType ) const;

      private:
         dtCore::ObserverPtr<GameLogicComponent> mAppComp;
         dtCore::RefPtr<dtGUI::CEUIDrawable> mGUI;
         dtCore::RefPtr<SimCore::Components::HUDGroup> mMainWindow;
         dtGUI::ScriptModule* mScriptModule;

         // Screens
         dtCore::RefPtr<SimCore::GUI::SimpleScreen> mScreenMainMenu;
         dtCore::RefPtr<SimCore::GUI::SimpleScreen> mScreenLobby;
         dtCore::RefPtr<SimCore::GUI::SimpleScreen> mScreenConnectFailPrompt;
         dtCore::RefPtr<SimCore::GUI::SimpleScreen> mScreenLoading;
         dtCore::RefPtr<SimCore::GUI::SimpleScreen> mScreenReadyRoom;
         dtCore::RefPtr<SimCore::GUI::SimpleScreen> mScreenOptions;
         dtCore::RefPtr<SimCore::GUI::SimpleScreen> mScreenQuitPrompt;

         // Special Widgets
         CEGUI::Editbox* mInputServerPort;
         CEGUI::Editbox* mInputServerIP;
   };

}

#endif
