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
#include <dtUtil/datapathutils.h>
#include <dtCore/scene.h>
#include <dtCore/shaderprogram.h>
#include <dtDAL/project.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/gmsettings.h>
#include <dtGame/gamemanager.h>
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
#include <dtGUI/ceuidrawable.h>
#endif
#include <dtGUI/scriptmodule.h>
#include <SimCore/ApplyShaderVisitor.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/MessageType.h>
#include <SimCore/GUI/SimpleScreen.h>
#include <SimCore/GUI/CeguiUtils.h>

// Local include directives
#include "ActorRegistry.h"
#include "Actors/FortActor.h"
#include "Actors/PlayerStatusActor.h"
#include "GUI/ButtonHighlight.h"
#include "GUI/CustomCeguiWidgets.h"
#include "GUI/HUDScreen.h"
#include "GUI/MainMenuScreen.h"
#include "GUI/ReadyRoomScreen.h"
#include "GUI/ScoreLabelManager.h"
#include "NetDemoMessages.h"
#include "NetDemoMessageTypes.h"
#include "NetDemoUtils.h"
#include "States.h"

// DEBUG:
#include <iostream>
#include <SimCore/Messages.h>



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
      , mListVehicleType(NULL)
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

      // Initialize the special effects layers.
      InitializeEffectsOverlays();

      // Get the Game Manager since some screens may need it.
      dtGame::GameManager& gm = *GetGameManager();

      // MAIN MENU
      mScreenMainMenu = new NetDemo::GUI::MainMenuScreen();
      mScreenMainMenu->Setup(*mMainWindow, *mEffectsOverlay);
      RegisterScreenWithState(*mScreenMainMenu, NetDemoState::STATE_MENU);
      mScreenMainMenu->OnEnter();//->SetVisible(true);

      // LOBBY
      mScreenLobby = new SimCore::GUI::SimpleScreen("Lobby", "CEGUI/layouts/NetDemo/Lobby.layout");
      mScreenLobby->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenLobby, NetDemoState::STATE_LOBBY);
      mInputServerPort = static_cast<CEGUI::Editbox*>(wm.getWindow("Lobby_Input_ServerPort"));
      mInputServerIP = static_cast<CEGUI::Editbox*>(wm.getWindow("Lobby_Input_ServerIP"));
      mListVehicleType = static_cast<CEGUI::ItemListbox*>(wm.getWindow("Lobby_List_VehicleType"));
      mListVehicleType->subscribeEvent(CEGUI::Window::EventMouseLeaves,
         CEGUI::Event::Subscriber(&GUIComponent::OnVehicleTypeSelected, this));
      mServerMode = static_cast<CEGUI::ItemListbox*>(wm.getWindow("Lobby_List_ServerMode"));
      mServerMode->subscribeEvent(CEGUI::Window::EventMouseLeaves,
         CEGUI::Event::Subscriber(&GUIComponent::OnServerModeSelected, this));
      mDifficulty = static_cast<CEGUI::ItemListbox*>(wm.getWindow("Lobby_List_Difficulty"));
      mDifficulty->subscribeEvent(CEGUI::Window::EventMouseLeaves,
         CEGUI::Event::Subscriber(&GUIComponent::OnDifficultySelected, this));


      // Default the server IP to what's in the config file.
      dtUtil::ConfigProperties& configParams = GetGameManager()->GetConfiguration();
      const std::string serverIP = configParams.GetConfigPropertyValue("dtNetGM.ServerHost", "127.0.0.1");
      mInputServerIP->setText(serverIP);

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

      // GARAGE
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
      // Do not register with a state for now since the HUD spans several sub-states.

      // QUIT PROMPT
      mScreenQuitPrompt = new SimCore::GUI::SimpleScreen("Main Menu", "CEGUI/layouts/NetDemo/QuitPrompt.layout");
      mScreenQuitPrompt->Setup( mMainWindow.get() );
      RegisterScreenWithState(*mScreenQuitPrompt, NetDemoState::STATE_GAME_QUIT);

      // Bind all buttons added to the menu system.
      BindButtons( *mMainWindow->GetCEGUIWindow() );

      // Setup the Score Label Manager.
      mScoreLabelManager = new NetDemo::GUI::ScoreLabelManager;
      mScoreLabelManager->SetGuiLayer(*mEffectsOverlay);
      mScoreLabelManager->SetCamera(*gm.GetApplication().GetCamera());

      // Setup the main background for most menus.
      mBackground = new osg::MatrixTransform;
      mBackground->addChild(LoadNodeFile("StaticMeshes:NetDemo:UI:MainBackground.ive"));
      mBackground->addChild(LoadNodeFile("Particles:stars.osg"));
      mEffectsOverlay->addChild(mBackground.get());

      // Attach a special shader.
      dtCore::RefPtr<SimCore::ApplyShaderVisitor> visitor = new SimCore::ApplyShaderVisitor();
      visitor->AddNodeName("UFOBeam");
      visitor->SetShaderName("ColorPulseShader");
      visitor->SetShaderGroup("CustomizableVehicleShaderGroup");
      mBackground->accept(*visitor);
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ShowMouseCursor( bool enable )
   {
      GetGameManager()->GetApplication().GetWindow()->SetShowCursor( enable );
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& messageType = message.GetMessageType();

      if(messageType == dtGame::MessageType::TICK_LOCAL)
      {
         Update(static_cast<const dtGame::TickMessage&>(message).GetDeltaRealTime());
      }
      else if(messageType == SimCore::MessageType::GAME_STATE_CHANGED)
      {
         ProcessStateChangeMessage(static_cast<const SimCore::Components::GameStateChangedMessage&>(message));
      }
      else if(messageType == NetDemo::MessageType::UI_OPTION_NEXT)
      {
         OnOptionNext(false);
      }
      else if(messageType == NetDemo::MessageType::UI_OPTION_PREV)
      {
         OnOptionNext(true);
      }
      else if(messageType == NetDemo::MessageType::UI_OPTION_SELECT)
      {
         OnOptionSelect();
      }
      else if(messageType == NetDemo::MessageType::UI_HELP)
      {
         mScreenHUD->ToggleHelp();
      }
      else if (messageType == NetDemo::MessageType::UI_DEBUGINFO_UPDATED)
      {
         mScreenHUD->HandleDebugInfoUpdated();
      }
      else if(messageType == dtGame::MessageType::INFO_ACTOR_CREATED
         || messageType == dtGame::MessageType::INFO_ACTOR_UPDATED)
      {
         ProcessActorUpdate(static_cast<const dtGame::ActorUpdateMessage&>(message));
      }
      else if(messageType == NetDemo::MessageType::ENTITY_ACTION)
      {
         ProcessEntityActionMessage(message);
      }
      else if(messageType == dtGame::MessageType::INFO_MAP_LOADED)
      {
         // Get the reference to the player.
         dtGame::GameActorProxy* proxy = NULL;
         GetGameManager()->FindActorByType(*NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE, proxy);
         if(proxy != NULL)
         {
            PlayerStatusActor* player = NULL;
            proxy->GetActor(player);
            mPlayer = player;
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::Update(float timeDelta)
   {
      if(mPreviousScreen.get() != mCurrentScreen.get() && mPreviousScreen.valid())
      {
         mPreviousScreen->Update(timeDelta);
      }

      if(mCurrentScreen.valid())
      {
         mCurrentScreen->Update(timeDelta);
      }

      if(mScreenHUD->IsEnabled())
      {
         mScreenHUD->Update(timeDelta);
      }

      if(mScoreLabelManager->IsEnabled())
      {
         mScoreLabelManager->Update(timeDelta);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::InitializeCEGUI(const std::string& schemeFile)
   {
      dtABC::Application& app = GetGameManager()->GetApplication();

      // Initialize CEGUI
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
      mGUI = new dtGUI::CEUIDrawable(app.GetWindow(), app.GetKeyboard(), app.GetMouse(), mScriptModule);
      osg::Node* guiOSGNode = mGUI->GetOSGNode();

      // Add the GUI drawable directly to the OSG scene node rather than
      // going through the over-managed dtCore Scene. If this is not done,
      // MapChangeStateData will remove the GUI drawable from the scene
      // when it tells the dtCore Scene to remove all drawable upon changing maps.
      // This line works around the over-managed code in scenes and map loading,
      // preventing the GUI from being unintentionally removed.
      GetGameManager()->GetScene().GetSceneNode()->addChild(guiOSGNode);
#else
      mGUI = new dtGUI::GUI(app.GetCamera(), app.GetKeyboard(), app.GetMouse());
      mGUI->SetResourceGroupDirectory("imagesets", dtDAL::Project::GetInstance().GetContext());
      mGUI->SetResourceGroupDirectory("looknfeels", dtDAL::Project::GetInstance().GetContext());
      mGUI->SetResourceGroupDirectory("layouts", dtDAL::Project::GetInstance().GetContext());
      mGUI->SetResourceGroupDirectory("schemes", dtDAL::Project::GetInstance().GetContext());
      mGUI->SetResourceGroupDirectory("fonts", dtDAL::Project::GetInstance().GetContext());
      osg::Node* guiOSGNode = &mGUI->GetRootNode();
#endif

      guiOSGNode->getOrCreateStateSet()->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD, "RenderBin");

#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
      std::string path = dtUtil::FindFileInPathList(schemeFile);
      if(path.empty())
      {
         throw dtUtil::Exception(
            "Failed to find the CEGUI scheme file : " + schemeFile, __FILE__, __LINE__);
      }
#endif

      try
      {
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
         CEGUI::SchemeManager::getSingleton().loadScheme(path);
#else
         mGUI->LoadScheme(schemeFile);
#endif

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
      CEGUI::System::getSingleton().getGUISheet()->setMousePassThroughEnabled(true);

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
   void GUIComponent::ProcessEntityActionMessage(const dtGame::Message& message)
   {
      const NetDemo::EntityActionMessage& actionMessage
         = static_cast<const NetDemo::EntityActionMessage&>(message);

      const EntityAction& action = actionMessage.GetAction();

      // Show the score label.
      if(action == EntityAction::SCORE)
      {
         mScoreLabelManager->AddScoreLabel(actionMessage.GetLocation(),
            actionMessage.GetPoints(), 2.0f);

         if(mPlayer.valid())
         {
            mScreenHUD->UpdatePlayerInfo(*mPlayer);
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessActorUpdate(const dtGame::ActorUpdateMessage& updateMessage)
   {
      const dtDAL::ActorType& actorType = *updateMessage.GetActorType();
      const dtCore::UniqueId& actorId = updateMessage.GetAboutActorId();

      if(actorType == *NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE)
      {
         // Get the actor to which the message refers.
         NetDemo::PlayerStatusActor* playerStats = NULL;
         if(mAppComp->FindActor(actorId, playerStats))
         {
            // Process the actor.
            ProcessPlayerStatusUpdate(*playerStats);
         }
      }
      else if(actorType == *NetDemoActorRegistry::FORT_ACTOR_TYPE)
      {
         // If one of the forts sent out an update, go through all the forts 
         // and display the most damaged, but not completed destroyed tower.
         std::vector<dtDAL::ActorProxy*> fortActors;
         GetGameManager()->FindActorsByType(*NetDemoActorRegistry::FORT_ACTOR_TYPE, fortActors);

         float curRatio = 0.0f;
         bool found = false;
         for (unsigned i = 0; i < fortActors.size(); ++i)
         {
            FortActor* fort = static_cast<FortActor*>(fortActors[i]->GetActor());
            if(fort != NULL && fort->GetCurDamageRatio() >  curRatio && fort->GetCurDamageRatio() < 1.0f)
            {
               curRatio = fort->GetCurDamageRatio();
               found = true;
            }
         }
         if (!found)
         {
            curRatio = 1.0f; // no towers found or all dead.  So, show no health
         }
         mScreenHUD->SetFortDamageRatio(1.0 - curRatio);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void GUIComponent::ProcessStateChangeMessage(const SimCore::Components::GameStateChangedMessage& stateChange)
   {
      const GameStateType& state = stateChange.GetNewState();

      ShowMouseCursor(state != NetDemoState::STATE_GAME_RUNNING);

      bool runningState = mAppComp->IsRunningState(state);
      mScreenHUD->SetVisible(runningState && state != NetDemoState::STATE_GAME_READYROOM);
      mScreenHUD->SetEnabled(runningState && state != NetDemoState::STATE_GAME_READYROOM);

      // Reference the previous and current screens so that they both can be updated.
      mPreviousScreen = GetScreenForState(stateChange.GetOldState());
      mCurrentScreen = GetScreenForState(state); // New State

      // Set the state for the Score Label Manager.
      if( ! runningState)
      {
         if(mScoreLabelManager->IsEnabled())
         {
            mScoreLabelManager->SetEnabled(false);
            mScoreLabelManager->Clear();
         }
      }
      else
      {
         mScoreLabelManager->SetEnabled(true);
      }

      // Enable or disble the background based on the current state.
      mBackground->setNodeMask(runningState ? 0x0 : 0xFFFFFFFF);

      // DEBUG:
      //std::cout << "\n\tNew: " << stateChange.GetNewState().GetName()
      //   << "\n\tOld: " << stateChange.GetOldState().GetName() << "\n\n";

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
      //const CEGUI::Window* button = GetWidgetFromEventArgs(args);

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
   bool GUIComponent::OnVehicleTypeSelected(const CEGUI::EventArgs& args)
   {
      // Do nothing - it generate the event when you click, instead it generates it 
      // when you move the mouse around or something.  So, I moved it to when the press
      // connect

      // Let CEGUI know the button has been handled.
      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::OnServerModeSelected(const CEGUI::EventArgs& args)
   {
      // Handled when you press connect.      
      return true; // Let CEGUI know the button has been handled.
   }

   /////////////////////////////////////////////////////////////////////////////
   bool GUIComponent::OnDifficultySelected(const CEGUI::EventArgs& args)
   {
      // Handled when you press connect.      
      return true; // Let CEGUI know the button has been handled.
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
         // Pick the vehicle type
         if(mListVehicleType->getSelectedCount() > 0)
         {
            CEGUI::ItemEntry* listItem = mListVehicleType->getFirstSelectedItem();
            if(listItem != NULL)
            {
               std::string selectedValue(listItem->getText().c_str());
               PlayerStatusActor::VehicleTypeEnum* vehicleType = NULL;
               if(selectedValue == "Truck")
               {
                  vehicleType = &PlayerStatusActor::VehicleTypeEnum::FOUR_WHEEL;
               }
               else if(selectedValue == "Hover")
               {
                  vehicleType = &PlayerStatusActor::VehicleTypeEnum::HOVER;
               }

               if(vehicleType != NULL)
               {
                  mAppComp->SetVehicleType(*vehicleType);
               }
            }
         }

         // Pick the server mode
         if(mServerMode->getSelectedCount() > 0)
         {
            CEGUI::ItemEntry* listItem = mServerMode->getFirstSelectedItem();
            if(listItem != NULL)
            {
               std::string selectedValue(listItem->getText().c_str());
               //PlayerStatusActor::VehicleTypeEnum* vehicleType = NULL;
               if(selectedValue == "Client")
               {
                  GetGameManager()->GetGMSettings().SetServerRole(false);
                  GetGameManager()->GetGMSettings().SetClientRole(true);
               }
               else // if(selectedValue == "Server") or anything else.
               {
                  // as server, we want both GM to be both Server AND Client
                  GetGameManager()->GetGMSettings().SetServerRole(true);
                  GetGameManager()->GetGMSettings().SetClientRole(true);
               }
            }
         }

         // Pick the server mode
         if(mDifficulty->getSelectedCount() > 0)
         {
            CEGUI::ItemEntry* listItem = mDifficulty->getFirstSelectedItem();
            if(listItem != NULL)
            {
               std::string selectedValue(listItem->getText().c_str());
               if(selectedValue == "Easiest")
               {
                  mAppComp->SetGameDifficulty(0);
               }
               else if(selectedValue == "Normal")
               {
                  mAppComp->SetGameDifficulty(1);
               }
               else // if(selectedValue == "Hard") or anything else.
               {
                  mAppComp->SetGameDifficulty(2);
               }
            }
         }


         //dtUtil::ConfigProperties& configParams = GetGameManager()->GetConfiguration();
         //const std::string role = configParams.GetConfigPropertyValue("dtNetGM.Role", "server");
         //const std::string gameName = configParams.GetConfigPropertyValue("dtNetGM.GameName", "NetDemo");
         const std::string hostIP(mInputServerIP->getText().c_str());
         int serverPort = CEGUI::PropertyHelper::stringToInt(mInputServerPort->getText());

         // NOTE - Server Role is now taken from the UI. The config settings for this are ignored.
         if( ! mAppComp->JoinNetwork(""/*role*/, serverPort, hostIP))
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
