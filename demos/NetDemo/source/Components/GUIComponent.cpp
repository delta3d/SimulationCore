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
#include <Components/GUIComponent.h>
#include <dtABC/application.h>
#include <dtCore/globals.h>
#include <dtCore/scene.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGUI/ceuidrawable.h>
#include <dtGUI/scriptmodule.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/MessageType.h>
#include <SimCore/gui/SimpleScreen.h>
#include <SimCore/gui/CeguiUtils.h>

// Local include directives
#include "ActorRegistry.h"
#include "Actors/FortActor.h"
#include "Actors/PlayerStatusActor.h"
#include "GUI/CustomCeguiWidgets.h"
#include "GUI/HUDScreen.h"
#include "GUI/ReadyRoomScreen.h"
#include "GUI/ButtonHighlight.h"
#include "MessageType.h"
#include "States.h"

// DEBUG:
#include <iostream>



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // CONSTANTS
   /////////////////////////////////////////////////////////////////////////////
   static const dtUtil::RefString BUTTON_TYPE_1("WindowsLook/Button");
   static const dtUtil::RefString BUTTON_PROPERTY_ACTION("Action");
   static const dtUtil::RefString BUTTON_PROPERTY_TYPE("ButtonType");

   static const dtUtil::RefString BUTTON_TYPE_CONNECT("CONNECT");



   /////////////////////////////////////////////////////////////////////////////
   // UTILITY CLASS CODE
   /////////////////////////////////////////////////////////////////////////////
   class ActionButtonFilter : public SimCore::GUI::CeguiUtils::CeguiWindowFilter<CEGUI::PushButton>
   {
      public:
         static const CEGUI::String PROPERTY_ACTION;

         virtual bool Accept(const CEGUI::PushButton& button)
         {
            return button.isPropertyPresent(PROPERTY_ACTION);
         }
   };

   /////////////////////////////////////////////////////////////////////////////
   const CEGUI::String ActionButtonFilter::PROPERTY_ACTION(NetDemo::BUTTON_PROPERTY_ACTION.Get());



   /////////////////////////////////////////////////////////////////////////////
   // CLASS CODE
   /////////////////////////////////////////////////////////////////////////////
   const dtUtil::RefString GUIComponent::DEFAULT_NAME("GUIComponent");

   /////////////////////////////////////////////////////////////////////////////
   GUIComponent::GUIComponent( const std::string& name )
      : BaseClass(name)
      , mScriptModule(new dtGUI::ScriptModule)
      , mCurrentHoveredWidget(NULL)
      , mInputServerPort(NULL)
      , mInputServerIP(NULL)
      , mCurrentButtonIndex(0)
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
      CEGUI::WindowManager& wm = CEGUI::WindowManager::getSingleton();

      // Get the Game Manager since some screens may need it.
      dtGame::GameManager& gm = *GetGameManager();

      // MAIN MENU
      mScreenMainMenu = new SimCore::GUI::SimpleScreen("Main Menu", "CEGUI/layouts/NetDemo/MainMenu.layout");
      mScreenMainMenu->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenMainMenu, NetDemoState::STATE_MENU);
      mScreenMainMenu->SetVisible(true);

      // LOBBY
      mScreenLobby = new SimCore::GUI::SimpleScreen("Lobby", "CEGUI/layouts/NetDemo/Lobby.layout");
      mScreenLobby->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenLobby, NetDemoState::STATE_LOBBY);
      mInputServerPort = static_cast<CEGUI::Editbox*>(wm.getWindow("Lobby_Input_ServerPort"));
      mInputServerIP = static_cast<CEGUI::Editbox*>(wm.getWindow("Lobby_Input_ServerIP"));

      // CONNECTION FAIL PROMPT
      mScreenConnectFailPrompt = new SimCore::GUI::SimpleScreen("Connection Fail Prompt", "CEGUI/layouts/NetDemo/ConnectionFailPrompt.layout");
      mScreenConnectFailPrompt->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenConnectFailPrompt, NetDemoState::STATE_LOBBY_CONNECT_FAIL);

      // LOADING
      mScreenLoading = new SimCore::GUI::SimpleScreen("Loading", "CEGUI/layouts/NetDemo/Loading.layout");
      mScreenLoading->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenLoading, NetDemoState::STATE_LOADING);

      // READY ROOM
      mScreenReadyRoom = new NetDemo::GUI::ReadyRoomScreen();
      mScreenReadyRoom->Setup(gm, mMainWindow.get());
      RegisterScreenWithState(*mScreenReadyRoom, NetDemoState::STATE_GAME_READYROOM);

      // GARGE
      mScreenGarage = new SimCore::GUI::SimpleScreen("Garage", "CEGUI/layouts/NetDemo/Garage.layout");
      mScreenGarage->Setup(mMainWindow.get());
      RegisterScreenWithState(*mScreenGarage, NetDemoState::STATE_GAME_GARAGE);

      // OPTIONS
      mScreenOptions = new SimCore::GUI::SimpleScreen("Main Menu", "CEGUI/layouts/NetDemo/Options.layout");
      mScreenOptions->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenOptions, NetDemoState::STATE_GAME_OPTIONS);

      // HUD
      mScreenHUD = new NetDemo::GUI::HUDScreen();
      mScreenHUD->Setup(*mAppComp, mMainWindow.get());

      // QUIT PROMPT
      mScreenQuitPrompt = new SimCore::GUI::SimpleScreen("Main Menu", "CEGUI/layouts/NetDemo/QuitPrompt.layout");
      mScreenQuitPrompt->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenQuitPrompt, NetDemoState::STATE_GAME_QUIT);

      // Bind all buttons added to the menu system.
      BindButtons( *mMainWindow->GetCEGUIWindow() );

      // Initialize the special effects layers.
      InitializeEffectsOverlays();
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ShowMouseCursor( bool enable )
   {
      GetGameManager()->GetApplication().GetWindow()->ShowCursor( enable );
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& messageType = message.GetMessageType();

      if(messageType == SimCore::MessageType::GAME_STATE_CHANGED)
      {
         ProcessStateChangeMessage(static_cast<const SimCore::Components::GameStateChangedMessage&>(message));
      }
      else if(messageType == NetDemo::MessageType::OPTION_NEXT)
      {
         OnOptionNext(false);
      }
      else if(messageType == NetDemo::MessageType::OPTION_PREV)
      {
         OnOptionNext(true);
      }
      else if(messageType == NetDemo::MessageType::OPTION_SELECT)
      {
         OnOptionSelect();
      }
      else if(messageType == dtGame::MessageType::INFO_ACTOR_CREATED
         || messageType == dtGame::MessageType::INFO_ACTOR_UPDATED)
      {
         ProcessActorUpdate(static_cast<const dtGame::ActorUpdateMessage&>(message));
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::InitializeCEGUI(const std::string& schemeFile)
   {
      dtABC::Application& app = GetGameManager()->GetApplication();

      // Initialize CEGUI
      mGUI = new dtGUI::CEUIDrawable(app.GetWindow(), app.GetKeyboard(), app.GetMouse(), mScriptModule);
      mGUI->SetRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD, "RenderBin");

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

         // Initialize custom widget factories.
         CEGUI::CustomWidgets::bindCEGUIWindowFactories();
      }
      catch(CEGUI::Exception& e)
      {
         std::ostringstream oss;
         oss << "CEGUI while setting up GUI Component: " << e.getMessage().c_str();
         throw dtUtil::Exception(oss.str(), __FILE__, __LINE__);
      }

      CEGUI::System::getSingleton().setDefaultFont("DejaVuSans-10");
      mMainWindow = new SimCore::Components::HUDGroup("Root","DefaultGUISheet");
      CEGUI::System::getSingleton().setGUISheet(mMainWindow->GetCEGUIWindow());

      // Prepare the main window.
      mMainWindow->SetVisible( true );
      mMainWindow->SetSize( 1.0f, 1.0f );
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::InitializeEffectsOverlays()
   {
      mEffectsOverlay = new osg::MatrixTransform;
      mEffectsOverlay->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

      dtCore::RefPtr<osg::Projection> projection = new osg::Projection;
      projection->setMatrix(osg::Matrix::ortho2D(0.0f ,1.0f, 0.0f, 1.0f));
      projection->addChild(mEffectsOverlay.get());

      osg::Group* sceneRoot = GetGameManager()->GetScene().GetSceneNode();
      sceneRoot->addChild(projection);

      mButtonHighlight = new ButtonHighlight();
      mButtonHighlight->Init(*mEffectsOverlay);
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessActorUpdate(const dtGame::ActorUpdateMessage& updateMessage)
   {
      const dtDAL::ActorType& actorType = *updateMessage.GetActorType();
      const dtCore::UniqueId& actorId = updateMessage.GetAboutActorId();

      if(actorType == *NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE)
      {
         // Get the actor to which the message refers.
         dtDAL::ActorProxy* proxy = NULL;
         NetDemo::PlayerStatusActor* playerStats = NULL;
         if(mAppComp->FindActor(actorId, playerStats))
         {
            // Process the actor.
            ProcessPlayerStatusUpdate(*playerStats);
         }
      }
      else if(actorType == *NetDemoActorRegistry::FORT_ACTOR_TYPE)
      {
         FortActor* fort = NULL;
         if(mAppComp->FindActor(actorId, fort))
         {
            mScreenHUD->SetFortDamageRatio(1.0 - fort->GetCurDamageRatio());
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessStateChangeMessage(const SimCore::Components::GameStateChangedMessage& stateChange)
   {
      const GameStateType& state = stateChange.GetNewState();

      ShowMouseCursor(state != NetDemoState::STATE_GAME_RUNNING);

      bool runningState = mAppComp->IsRunningState(state);
      mScreenHUD->SetVisible(runningState);
      mScreenHUD->SetEnabled(runningState);

      /*mScreenMainMenu->SetVisible( state == NetDemoState::STATE_MENU );
      mScreenMainMenu->SetEnabled( state == NetDemoState::STATE_MENU );

      mScreenLobby->SetVisible( state == NetDemoState::STATE_LOBBY );
      mScreenLobby->SetEnabled( state == NetDemoState::STATE_LOBBY );

      mScreenConnectFailPrompt->SetVisible( state == NetDemoState::STATE_LOBBY_CONNECT_FAIL );
      mScreenConnectFailPrompt->SetEnabled( state == NetDemoState::STATE_LOBBY_CONNECT_FAIL );

      mScreenLoading->SetVisible( state == NetDemoState::STATE_LOADING );
      mScreenLoading->SetEnabled( state == NetDemoState::STATE_LOADING );

      mScreenReadyRoom->SetVisible( state == NetDemoState::STATE_GAME_READYROOM );
      mScreenReadyRoom->SetEnabled( state == NetDemoState::STATE_GAME_READYROOM );

      mScreenOptions->SetVisible( state == NetDemoState::STATE_GAME_OPTIONS );
      mScreenOptions->SetEnabled( state == NetDemoState::STATE_GAME_OPTIONS );

      mScreenQuitPrompt->SetVisible( state == NetDemoState::STATE_GAME_QUIT );
      mScreenQuitPrompt->SetEnabled( state == NetDemoState::STATE_GAME_QUIT );*/

      // Update the list of buttons for the current screen.
      UpdateButtonArray();
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessPlayerStatusUpdate(const PlayerStatusActor& playerStats)
   {
      if(mAppComp->IsInState(NetDemoState::STATE_GAME_READYROOM))
      {
         mScreenReadyRoom->UpdatePlayerList();
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   GameLogicComponent* GUIComponent::GetAppComponent()
   {
      if( ! mAppComp.valid() )
      {
         GameLogicComponent* comp = NULL;
         GetGameManager()->GetComponentByName( GameLogicComponent::DEFAULT_NAME, comp );
         mAppComp = comp;
      }

      if( ! mAppComp.valid() )
      {
         LOG_ERROR( "GUI Component cannot access the Game App Component." );
      }

      return mAppComp.get();
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::OnOptionNext(bool reverse)
   {
      int numButtons = int(mCurrentScreenButtons.size());
      if(numButtons != 0)
      {
         if(reverse)
         {
            mCurrentButtonIndex = (--mCurrentButtonIndex + numButtons) % numButtons;
         }
         else
         {
            mCurrentButtonIndex = (++mCurrentButtonIndex) % numButtons;
         }

         // DEBUG:
         // std::cout << "\n\nItem: " << mCurrentButtonIndex << " / " << numButtons << "\n\n";

         SetButtonFocused(mCurrentScreenButtons[mCurrentButtonIndex]);
      }
   }
   
   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::OnOptionSelect()
   {
      if(mCurrentHoveredWidget != NULL)
      {
         // Simulate the button being pressed by the mouse cursor.
         HandleButton(*mCurrentHoveredWidget);
      }
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
      const CEGUI::Window* button = GetWidgetFromEventArgs(args);

      if(button != NULL)
      {
         HandleButton(*button);
      }

      // Let CEGUI know the button has been handled.
      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::OnButtonFocusGain(const CEGUI::EventArgs& args)
   {
      const CEGUI::Window* button = GetWidgetFromEventArgs(args);

      // Enable special hover effects.
      SetButtonFocused(button);

      // Let CEGUI know the button has been handled.
      return true;
   }
   
   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::OnButtonFocusLost(const CEGUI::EventArgs& args)
   {
      const CEGUI::Window* button = GetWidgetFromEventArgs(args);

      // Disable special hover effects.
      /*if(button != NULL && mCurrentHoveredWidget != NULL)
      {
         SetHoverEffectEnabled(false);
         mCurrentHoveredWidget = NULL;
      }*/

      // Let CEGUI know the button has been handled.
      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::SetButtonFocused(const CEGUI::Window* button)
   {
      if(button != NULL)
      {
         mCurrentHoveredWidget = button;
         SetHoverEffectOnElement(*button);
         SetHoverEffectEnabled(true);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::HandleButton(const CEGUI::Window& button)
   {
      // Prepare to capture the button action name.
      std::string action;
      std::string buttonType;

      // Attempt to access the button's action property.
      try
      {
         CEGUI::String actionValue = button.getProperty(BUTTON_PROPERTY_ACTION.Get());
         CEGUI::String buttonTypeValue = button.getProperty(BUTTON_PROPERTY_TYPE.Get());
         action = std::string(actionValue.c_str());
         buttonType = std::string(buttonTypeValue.c_str());
      }
      catch(CEGUI::Exception& ceguiEx)
      {
         std::ostringstream oss;
         oss << "Button \"" << button.getName().c_str()
            << "\" does not have the \"Action\" property.\n"
            << ceguiEx.getMessage().c_str() << std::endl;
         LOG_ERROR(oss.str());
      }

      // Determine if this is a special button.
      HandleSpecialButton(buttonType, action);

      // Execute the transition specified by the button.
      GetAppComponent()->DoStateTransition(action);
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::HandleSpecialButton(const std::string& buttonType, std::string& inOutAction)
   {
      // Determine if this is a special button.
      if(buttonType == BUTTON_TYPE_CONNECT.Get())
      {
         dtUtil::ConfigProperties& configParams = GetGameManager()->GetConfiguration();
         const std::string role = configParams.GetConfigPropertyValue("dtNetGM.Role", "server");
         //const std::string gameName = configParams.GetConfigPropertyValue("dtNetGM.GameName", "NetDemo");
         const std::string hostIP(mInputServerIP->getText().c_str());
         int serverPort = CEGUI::PropertyHelper::stringToInt(mInputServerPort->getText());

         if( ! mAppComp->JoinNetwork(role, serverPort, hostIP))
         {
            // Show connection failure prompt.
            inOutAction = Transition::TRANSITION_CONNECTION_FAIL.GetName();
         }
      }
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
   void GUIComponent::BindButton(CEGUI::PushButton& button)
   {
      button.subscribeEvent(CEGUI::PushButton::EventClicked,
         CEGUI::Event::Subscriber(&GUIComponent::OnButtonClicked, this));

      // Special Effects Bindings
      button.subscribeEvent(CEGUI::Window::EventMouseEnters,
         CEGUI::Event::Subscriber(&GUIComponent::OnButtonFocusGain, this));
      button.subscribeEvent(CEGUI::Window::EventActivated,
         CEGUI::Event::Subscriber(&GUIComponent::OnButtonFocusGain, this));

      button.subscribeEvent(CEGUI::Window::EventMouseLeaves,
         CEGUI::Event::Subscriber(&GUIComponent::OnButtonFocusLost, this));
      button.subscribeEvent(CEGUI::Window::EventDeactivated,
         CEGUI::Event::Subscriber(&GUIComponent::OnButtonFocusLost, this));
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::IsButtonType( const std::string& buttonType ) const
   {
      // Add any other button types here.
      return buttonType == BUTTON_TYPE_1;
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::UpdateButtonArray()
   {
      // Clear old button references.
      mCurrentScreenButtons.clear();

      // Disable the button highlight effect.
      SetHoverEffectEnabled(false);

      const GameStateType* currentState = mAppComp->GetCurrentState();
      if(currentState != NULL)
      {
         // Obtain the current screen.
         SimpleScreen* currentScreen = dynamic_cast<SimpleScreen*>(GetScreenForState(*currentState));
         if(currentScreen != NULL)
         {
            // Get all the buttons for the current screen.
            ActionButtonFilter filter;
            CEGUI::Window* screenRoot = currentScreen->GetRoot()->GetCEGUIWindow();
            SimCore::GUI::CeguiUtils::GetChildWindowsByType(mCurrentScreenButtons, screenRoot, &filter);

            // Set focus on the first button.
            mCurrentButtonIndex = 0;
            if( ! mCurrentScreenButtons.empty())
            {
               SetButtonFocused(mCurrentScreenButtons[0]);
            }
         }
      }
      else
      {
         LOG_ERROR("Could not access the current game state.");
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::RegisterScreenWithState(Screen& screen, GameStateType& state)
   {
      bool success = mStateScreenMap.insert(std::make_pair(&state, &screen)).second;

      if(success)
      {
         using namespace SimCore::Components;
         GameState* gameState = GetAppComponent()->GetState(&state);
         if(gameState != NULL)
         {
            // Bind the screen update functor.
            GameState::UpdateFunctor updateFunc(&screen, &Screen::OnUpdate);
            gameState->SetUpdate(updateFunc);

            typedef dtUtil::Functor<void,TYPELIST_0()> VoidFunc;

            // Bind Entry method.
            VoidFunc enterFunc(&screen, &Screen::OnEnter);
            dtCore::RefPtr<dtUtil::Command0<void> > comEnter = new dtUtil::Command0<void>(enterFunc);
            gameState->AddEntryCommand(comEnter.get());

            // Bind Exit method.
            VoidFunc exitFunc(&screen, &Screen::OnExit);
            dtCore::RefPtr<dtUtil::Command0<void> > comExit = new dtUtil::Command0<void>(exitFunc);
            gameState->AddExitCommand(comExit);
         }
         else
         {
            LOG_WARNING("Could not find the game state for state type \""
               +state.GetName()+"\" to assign to screen \""+screen.GetName()+"\"");
         }

         screen.SetVisible(false);
      }

      return success;
   }

   /////////////////////////////////////////////////////////////////////////////
   Screen* GUIComponent::GetScreenForState(const GameStateType& state)
   {
      StateScreenMap::const_iterator foundIter = mStateScreenMap.find(&state);
      return foundIter != mStateScreenMap.end() ? foundIter->second : NULL;
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::SetHoverEffectOnElement(const CEGUI::Window& window)
   {
      osg::Vec4 screenBounds(SimCore::GUI::CeguiUtils::GetNormalizedScreenBounds(window));
      mButtonHighlight->SetScreenBounds(screenBounds);
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::SetHoverEffectEnabled(bool enabled)
   {
      mButtonHighlight->SetVisible(enabled);
      mButtonHighlight->SetEnabled(enabled);
   }

}
