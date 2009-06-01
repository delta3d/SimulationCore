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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <GUICOmponent.h>
#include <dtABC/application.h>
#include <dtCore/globals.h>
#include <dtCore/scene.h>
#include <dtGUI/ceuidrawable.h>
#include <dtGUI/scriptmodule.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/MessageType.h>
#include <SimCore/gui/SimpleScreen.h>
#include <States.h>

using namespace SimCore::Components;



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // CONSTANTS
   /////////////////////////////////////////////////////////////////////////////
   static const dtUtil::RefString BUTTON_TYPE_1("WindowsLook/Button");
   static const dtUtil::RefString BUTTON_PROPERTY_ACTION("Action");



   /////////////////////////////////////////////////////////////////////////////
   // CLASS CODE
   /////////////////////////////////////////////////////////////////////////////
   const dtUtil::RefString GUIComponent::DEFAULT_NAME("GUIComponent");

   /////////////////////////////////////////////////////////////////////////////
   GUIComponent::GUIComponent( const std::string& name )
      : BaseClass(name)
      , mScriptModule(new dtGUI::ScriptModule)
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   GUIComponent::~GUIComponent()
   {
      delete mScriptModule;
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::Initialize()
   {
      InitializeCEGUI("CEGUI/schemes/NetDemo.scheme");

      // MAIN MENU
      mScreenMainMenu = new SimCore::GUI::SimpleScreen("Main Menu", "CEGUI/layouts/NetDemo/MainMenu.layout");
      mScreenMainMenu->Setup( mMainWindow.get() );

      // OPTIONS
      mScreenOptions = new SimCore::GUI::SimpleScreen("Main Menu", "CEGUI/layouts/NetDemo/Options.layout");
      mScreenOptions->Setup( mMainWindow.get() );

      // QUIT PROMPT
      mScreenQuitPrompt = new SimCore::GUI::SimpleScreen("Main Menu", "CEGUI/layouts/NetDemo/QuitPrompt.layout");
      mScreenQuitPrompt->Setup( mMainWindow.get() );

      // Bind all buttons added to the menu system.
      BindButtons( *mMainWindow->GetCEGUIWindow() );
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ShowMouseCursor( bool enable )
   {
      GetGameManager()->GetApplication().GetWindow()->ShowCursor( enable );
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessMessage( const dtGame::Message& message )
   {
      const dtGame::MessageType& messageType = message.GetMessageType();

      if( messageType == SimCore::MessageType::GAME_STATE_CHANGED )
      {
         ProcessStateChangeMessage( static_cast<const SimCore::Components::GameStateChangedMessage&>(message) );
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessStateChangeMessage( const GameStateChangedMessage& stateChange )
   {
      const StateType& state = stateChange.GetNewState();

      bool isRunningState = state == NetDemoState::STATE_GAME;

      ShowMouseCursor( ! isRunningState );

      mScreenMainMenu->SetVisible( state == NetDemoState::STATE_MENU );
      mScreenMainMenu->SetEnabled( state == NetDemoState::STATE_MENU );

      mScreenOptions->SetVisible( state == NetDemoState::STATE_GAME_OPTIONS );
      mScreenOptions->SetEnabled( state == NetDemoState::STATE_GAME_OPTIONS );

      mScreenQuitPrompt->SetVisible( state == NetDemoState::STATE_GAME_QUIT );
      mScreenQuitPrompt->SetEnabled( state == NetDemoState::STATE_GAME_QUIT );
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::InitializeCEGUI( const std::string& schemeFile )
   {
      dtABC::Application &app = GetGameManager()->GetApplication();

      // Initialize CEGUI
      mGUI = new dtGUI::CEUIDrawable(app.GetWindow(), app.GetKeyboard(), app.GetMouse(), mScriptModule);
      mGUI->SetRenderBinDetails(RenderingSupportComponent::RENDER_BIN_HUD, "RenderBin");

      // Add the GUI drawable directly to the OSG scene node rather than
      // going through the over-managed dtCore Scene. If this is not done,
      // MapChangeStateData will remove the GUI drawable from the scene
      // when it tells the dtCore Scene to remove all drawable upon changing maps.
      // This line works around the over-managed code in scenes and map loading,
      // preventing the GUI from being unintentionally removed.
      GetGameManager()->GetScene().GetSceneNode()->addChild(mGUI->GetOSGNode());

      std::string path = dtCore::FindFileInPathList(schemeFile);
      if(path.empty())
      {
         throw dtUtil::Exception(
            "Failed to find the CEGUI scheme file : " + schemeFile, __FILE__, __LINE__);
      }

      try
      {
         CEGUI::SchemeManager::getSingleton().loadScheme(path);
      }
      catch(CEGUI::Exception &e)
      {
         std::ostringstream oss;
         oss << "CEGUI while setting up GUI Component: " << e.getMessage().c_str();
         throw dtUtil::Exception(oss.str(), __FILE__, __LINE__);
      }

      CEGUI::System::getSingleton().setDefaultFont("DejaVuSans-10");
      mMainWindow = new HUDGroup("Root","DefaultGUISheet");
      CEGUI::System::getSingleton().setGUISheet(mMainWindow->GetCEGUIWindow());

      // Prepare the main window.
      mMainWindow->SetVisible( true );
      mMainWindow->SetSize( 1.0f, 1.0f );
   }

   /////////////////////////////////////////////////////////////////////////////
   GameAppComponent* GUIComponent::GetAppComponent()
   {
      if( ! mAppComp.valid() )
      {
         GameAppComponent* comp = NULL;
         GetGameManager()->GetComponentByName( GameAppComponent::DEFAULT_NAME, comp );
         mAppComp = comp;
      }

      if( ! mAppComp.valid() )
      {
         LOG_ERROR( "GUI Component cannot access the Game App Component." );
      }

      return mAppComp.get();
   }

   /////////////////////////////////////////////////////////////////////////////
   const CEGUI::Window* GUIComponent::GetWidgetFromEventArgs( const CEGUI::EventArgs& args ) const
   {
      // Cast to WindowEventArgs in order to access the associated CEGUI window.
      const CEGUI::WindowEventArgs& winArgs
         = static_cast<const CEGUI::WindowEventArgs&>(args);

      return winArgs.window;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::OnButtonClicked( const CEGUI::EventArgs& args )
   {
      const CEGUI::Window* button = GetWidgetFromEventArgs( args );

      if( button != NULL )
      {
         // Prepare to capture the button action name.
         std::string action;

         // Attempt to access the button's action property.
         try
         {
            CEGUI::String& value = button->getProperty(BUTTON_PROPERTY_ACTION.Get());
            action = std::string( value.c_str() );
         }
         catch( CEGUI::Exception& ceguiEx )
         {
            std::ostringstream oss;
            oss << "Button \"" << button->getName().c_str()
               << "\" does not have the \"Action\" property.\n"
               << ceguiEx.getMessage().c_str() << std::endl;
            LOG_ERROR( oss.str() );
         }

         GetAppComponent()->HandleTransition( action );
      }

      // Let CEGUI know the button has been handled.
      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::BindButtons( CEGUI::Window& rootWindow )
   {
      CEGUI::Window* curChild = NULL;
      CEGUI::PushButton* button = NULL;
      typedef std::vector<CEGUI::Window*> CEGUIChildList;
      CEGUIChildList childList;

      // Go through all child windows and bind all buttons.
      childList.push_back( &rootWindow );
      while( ! childList.empty() )
      {
         curChild = childList.back();
         childList.pop_back();

         // If this is a button...
         if( IsButtonType( curChild->getType().c_str() ) )
         {
            button = static_cast<CEGUI::PushButton*>(curChild);

            // ...bind it to this component's callback for handling buttons.
            BindButton( *button );
         }
         // ...else if this is a normal widget window...
         else if( curChild->getChildCount() > 0 )
         {
            size_t numChildren = curChild->getChildCount(); 
            for( size_t i = 0; i < numChildren; ++i )
            {
               childList.push_back( curChild->getChildAtIdx( i ) );
            }
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::BindButton( CEGUI::PushButton& button )
   {
      button.subscribeEvent(CEGUI::PushButton::EventClicked,
         CEGUI::Event::Subscriber(&GUIComponent::OnButtonClicked, this));
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::IsButtonType( const std::string& buttonType ) const
   {
      // Add any other button types here.
      return buttonType == BUTTON_TYPE_1;
   }

}
