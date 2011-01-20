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

#include <prefix/SimCorePrefix.h>

#include <SimCore/Components/BaseHUD.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/Components/RenderingSupportComponent.h>

#include <dtUtil/datapathutils.h>
#include <dtCore/scene.h>
#include <dtCore/deltawin.h>

#include <dtGame/gamemanager.h>

#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
#include <dtGUI/ceuidrawable.h>
#include <dtGUI/basescriptmodule.h>
#else
#include <dtGUI/gui.h>
#include <dtGUI/scriptmodule.h>
#endif

#include <dtUtil/fileutils.h>

#include <dtABC/application.h>
#include <dtDAL/project.h>
#include <dtGame/messagetype.h>

#include <osg/MatrixTransform>
#include <osg/Matrix>

#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
#include <dtABC/applicationconfigschema.h>
//these are to realize a window if we do not have a valid context to initialize
#include <osg/GraphicsContext>
#include <osgViewer/GraphicsWindow>
#endif

namespace SimCore
{
   namespace Components
   {

      BaseHUDInitException::BaseHUDInitException(const std::string& message,
               const std::string& filename, unsigned int linenum)
      : dtUtil::Exception(message, filename, linenum)
      {
      }

      BaseHUDRuntimeException::BaseHUDRuntimeException(const std::string& message,
               const std::string& filename, unsigned int linenum)
      : dtUtil::Exception(message, filename, linenum)
      {
      }


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
         : dtGame::GMComponent(name)
         , mWin(win)
         , mScriptModule(new dtGUI::ScriptModule())
         , mHUDState(&HUDState::MINIMAL)
         , mSchemeFile(ceguiScheme)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      BaseHUD::~BaseHUD()
      {
         mMainWindow = NULL;
         mGUI = NULL;
         delete mScriptModule; // a script module does not extend dtCore::Base
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::Initialize( unsigned int designedResWidth, unsigned int designedResHeight )
      {
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
         bool realized = GetGameManager()->GetApplication().GetWindow()->GetOsgViewerGraphicsWindow()->isRealized();
         //this code will create an opengl context to initialize CEGUI with
         //we only need to do this if OSG does not realize on creation
         dtCore::RefPtr<osg::GraphicsContext> gc;
         if(!realized)
         {
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->x = 0;
            traits->y = 0;
            traits->width = 1;
            traits->height = 1;
            traits->windowDecoration = false;
            traits->doubleBuffer = false;
            traits->sharedContext = 0;
            traits->pbuffer = false;

            gc = osg::GraphicsContext::createGraphicsContext(traits.get());

            if (gc.valid())
            {
               gc->realize();
               gc->makeCurrent();
               if (!dynamic_cast<osgViewer::GraphicsWindow*>(gc.get()))
               {
                  LOG_ERROR("Unable to create graphics context to initialize CEGUI.");
               }
            }
         }
#endif

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
            throw BaseHUDInitException(oss.str(), __FILE__, __LINE__);
         }
      }

      void BaseHUD::InitializeCEGUI()
      {
         dtABC::Application &app = GetGameManager()->GetApplication();
         // Initialize CEGUI
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
         mGUI = new dtGUI::CEUIDrawable(app.GetWindow(), app.GetKeyboard(), app.GetMouse(), mScriptModule);
         osg::Node* guiOSGNode = mGUI->GetOSGNode();
         std::string path = dtUtil::FindFileInPathList(mSchemeFile);
         if(path.empty())
         {
            throw dtUtil::Exception(BaseHUDException::INIT_ERROR,
               "Failed to find the scheme file : " + mSchemeFile, __FILE__, __LINE__);
         }

         std::string dir = path.substr(0, path.length() - (mSchemeFile.length() - std::string("CEGUI").length()) );
         dtUtil::FileUtils::GetInstance().PushDirectory(dir);
         try
         {
            CEGUI::SchemeManager::getSingleton().loadScheme(path);
#else
         mGUI = new dtGUI::GUI(app.GetCamera(), app.GetKeyboard(), app.GetMouse());
         osg::Group* guiOSGNode = &mGUI->GetRootNode();
         mGUI->SetScriptModule(mScriptModule);         

         try
         {
            mGUI->LoadScheme(mSchemeFile);
#endif
         }
         catch(CEGUI::Exception &e)
         {
            std::ostringstream oss;
            oss << "CEGUI while setting up BaseHUD: " << e.getMessage().c_str();
            throw BaseHUDInitException(oss.str(), __FILE__, __LINE__);
         }
         dtUtil::FileUtils::GetInstance().PopDirectory();

         guiOSGNode->getOrCreateStateSet()->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD, "RenderBin");
         CEGUI::System::getSingleton().setDefaultFont("DejaVuSans-10");
         mMainWindow = new HUDGroup("root","DefaultGUISheet");
         CEGUI::System::getSingleton().setGUISheet(mMainWindow->GetCEGUIWindow());
         CEGUI::System::getSingleton().getGUISheet()->setMousePassThroughEnabled(true);
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
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR >= 7
      osg::Group& BaseHUD::GetRootNode()
      {
         return mGUI->GetRootNode();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Group& BaseHUD::GetRootNode() const
      {
         return mGUI->GetRootNode();
      }
#endif

      //////////////////////////////////////////////////////////////////////////
      void BaseHUD::ProcessMessage(const dtGame::Message& message)
      {
         if (message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            TickHUD();
         }
         else if (message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
            GetGameManager()->GetScene().AddDrawable(GetGUIDrawable().get());
#endif
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

#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR >= 7
      //////////////////////////////////////////////////////////////////////////
      dtGUI::GUI* BaseHUD::GetGUI()
      {
         return mGUI.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtGUI::GUI* BaseHUD::GetGUI() const
      {
         return mGUI.get();
      }
#endif

   }
}
