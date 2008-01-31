/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Components/BaseHUD.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/Components/RenderingSupportComponent.h>

#include <dtCore/globals.h>
#include <dtCore/scene.h>
#include <dtCore/deltawin.h>

#include <dtGame/gamemanager.h>

#include <dtGUI/ceuidrawable.h>
#include <dtGUI/basescriptmodule.h>

#include <dtUtil/fileutils.h>

#include <dtABC/application.h>

#include <osg/MatrixTransform>
#include <osg/Matrix>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Base HUD Exceptions Code
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseHUDException);
      BaseHUDException BaseHUDException::INIT_ERROR("INIT_ERROR");
      BaseHUDException BaseHUDException::RUNTIME_ERROR("RUNTIME_ERROR");



      //////////////////////////////////////////////////////////////////////////
      // Base HUD States Code
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(HUDState);
      HUDState HUDState::MINIMAL("MINIMAL");
      HUDState HUDState::MAXIMUM("MAXIMUM");
      HUDState HUDState::NONE("NONE");
      HUDState HUDState::HELP("HELP");



      //////////////////////////////////////////////////////////////////////////
      // Base HUD Code
      //////////////////////////////////////////////////////////////////////////
      BaseHUD::BaseHUD(dtCore::DeltaWin *win, const std::string& name,
         const std::string& ceguiScheme )
         : dtGame::GMComponent(name),
         mWin(win),
         mScriptModule(new dtGUI::ScriptModule()),
         mHUDState(&HUDState::MINIMAL), 
         mSchemeFile(ceguiScheme)
      {

      }

      //////////////////////////////////////////////////////////////////////////
      BaseHUD::~BaseHUD()
      {
         delete mScriptModule; // a script module does not extend dtCore::Base
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::Initialize( unsigned int designedResWidth, unsigned int designedResHeight )
      {
         InitializeCEGUI();

         dtCore::RefPtr<HUDGroup> hudOverlay = new HUDGroup( "HUDOverlay" );
         mMainWindow->Add( hudOverlay.get() );
         hudOverlay->SetPosition( 0.0f, 0.0f );
         hudOverlay->SetSize( 1.0f, 1.0f );

         // Do the GUI specific setup
         try
         {
            SetupGUI( *hudOverlay, designedResWidth, designedResHeight );

            // finally, update the state - disable/hide to make it match current state
            UpdateState();

            // Show the main HUD overlay. Only sub elements
            // should have their visibility toggled.
            hudOverlay->Show();
         }
         catch(CEGUI::Exception &e)
         {
            std::ostringstream oss;
            oss << "CEGUI while setting up GUI: " << e.getMessage().c_str();
            throw dtUtil::Exception(BaseHUDException::INIT_ERROR,oss.str(), __FILE__, __LINE__);
         }
      }

      void BaseHUD::InitializeCEGUI()
      {
         dtABC::Application &app = GetGameManager()->GetApplication();
         // Initialize CEGUI
         mGUI = new dtGUI::CEUIDrawable(app.GetWindow(), app.GetKeyboard(), app.GetMouse(), mScriptModule);

         osg::StateSet* ss = mGUI->GetOSGNode()->getOrCreateStateSet();
         ss->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD, "DepthSortedBin");

         std::string path = dtCore::FindFileInPathList(mSchemeFile);
         if(path.empty())
         {
            throw dtUtil::Exception(BaseHUDException::INIT_ERROR,
               "Failed to find the scheme file.", __FILE__, __LINE__);
         }

         std::string dir = path.substr(0, path.length() - (mSchemeFile.length() - 5));
         dtUtil::FileUtils::GetInstance().PushDirectory(dir);
         try
         {
            CEGUI::SchemeManager::getSingleton().loadScheme(path);
         }
         catch(CEGUI::Exception &e)
         {
            std::ostringstream oss;
            oss << "CEGUI while setting up BaseHUD: " << e.getMessage().c_str();
            throw dtUtil::Exception(BaseHUDException::INIT_ERROR,oss.str(), __FILE__, __LINE__);
         }
         dtUtil::FileUtils::GetInstance().PopDirectory();

         CEGUI::System::getSingleton().setDefaultFont("DejaVuSans-10");
         mMainWindow = new HUDGroup("root","DefaultGUISheet");
         CEGUI::System::getSingleton().setGUISheet(mMainWindow->GetCEGUIWindow());
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::RegisterCallbacks()
      {
         // EXAMPLE:
         // 1) Create the functor to act as a callback.
         // dtGUI::ScriptModule::HandlerFunctor newHandler( dtUtil::MakeFunctor( &BaseHUD::MyCallback, this ) );
         //
         // 2) Map the functor to an identifier string.
         // mScriptModule->AddCallback("MyCallback", newHandler );
         //
         // 3) In later code (window setup), 
         //    map the callback functor identifier to a window event:
         // ceguiWindow->subscribeScriptedEvent(Checkbox::EventCheckStateChanged, "MyCallback" );
      }

      void BaseHUD::OnAddedToGM()
      {
         //InitializeCEGUI();
      }

      //////////////////////////////////////////////////////////////////////////
      CEGUI::Window* BaseHUD::GetMainCEGUIWindow()
      {
         return mMainWindow->GetCEGUIWindow();
      }

      //////////////////////////////////////////////////////////////////////////
      const CEGUI::Window* BaseHUD::GetMainCEGUIWindow() const
      {
         return mMainWindow->GetCEGUIWindow();
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::ProcessMessage(const dtGame::Message& message)
      {
         if (message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            TickHUD();
         }
         else if (message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            GetGameManager()->GetScene().AddDrawable(GetGUIDrawable().get());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::SetupGUI( HUDGroup& hudOverlay, unsigned int designedResWidth, unsigned int designedResHeight )
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::TickHUD()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::UpdateState()
      {

      }
   }
}
