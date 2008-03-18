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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Tools/HelpWindow.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>
#include <dtCore/deltawin.h>
#include <dtGame/exceptionenum.h>

//solves a problem with the prefix headers
#ifdef None
#undef None
#endif

#include <CEGUI/CEGUI.h>

namespace SimCore
{
   namespace Tools
   {
      HelpWindow::HelpWindow(CEGUI::Window *mainWindow) : 
         mIsEnabled(false),
         mOverlay(NULL), 
         mHeaderText(NULL), 
         mBinocsText(NULL), 
         mLRFText(NULL), 
         mCompassText(NULL), 
         mGPSText(NULL),
         mToggleFullScreen(NULL),
         mMagnifyModels(NULL)
         ,mCurrentParticleCount(NULL)
         ,mLifeTimeParticleCount(NULL)
      {
         mMainWindow = NULL;
         InitGui(mainWindow);
      }

      HelpWindow::~HelpWindow()
      {
         mOverlay->removeChildWindow(mHeaderText);
         mOverlay->removeChildWindow(mBinocsText);
         mOverlay->removeChildWindow(mLRFText);
         mOverlay->removeChildWindow(mCompassText);
         mOverlay->removeChildWindow(mGPSText);
         mOverlay->removeChildWindow(mToggleFullScreen);
         mOverlay->removeChildWindow(mMagnifyModels);
         mOverlay->removeChildWindow(mCurrentParticleCount);
         mOverlay->removeChildWindow(mLifeTimeParticleCount);

         if(mMainWindow != NULL)
            mMainWindow->removeChildWindow(mOverlay);

         mHeaderText->destroy();
         mBinocsText->destroy();
         mLRFText->destroy();
         mCompassText->destroy();
         mGPSText->destroy();
         mToggleFullScreen->destroy();
         mMagnifyModels->destroy();
         mCurrentParticleCount->destroy();
         mLifeTimeParticleCount->destroy();
         mOverlay->destroy();
      }

      void HelpWindow::Enable(bool enable)
      {
         mIsEnabled = enable;

         mIsEnabled ? mOverlay->show() : mOverlay->hide();
      }

      void HelpWindow::InitGui(CEGUI::Window *mainWindow)
      {
         try
         {
            mMainWindow = mainWindow;
            CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
            mOverlay = static_cast<CEGUI::FrameWindow*>(wm->createWindow("WindowsLook/FrameWindow", "help_window"));
            if(mMainWindow != NULL)
               mMainWindow->addChildWindow(mOverlay);

            mHeaderText       = wm->createWindow("WindowsLook/StaticText", "header_helptext");
            mBinocsText       = wm->createWindow("WindowsLook/StaticText", "binocs_helptext");
            mLRFText          = wm->createWindow("WindowsLook/StaticText", "lrf_helptext");
            mCompassText      = wm->createWindow("WindowsLook/StaticText", "compass_helptext");
            mGPSText          = wm->createWindow("WindowsLook/StaticText", "gps_helptext");
            mToggleFullScreen = wm->createWindow("WindowsLook/StaticText", "fullscreen_helptext");
            mMagnifyModels    = wm->createWindow("WindowsLook/StaticText", "magnifiy_helptext");
            
            mCurrentParticleCount = wm->createWindow("WindowsLook/StaticText", "particle_count");
            mLifeTimeParticleCount= wm->createWindow("WindowsLook/StaticText", "lifetime_particle_count");

            mOverlay->addChildWindow(mHeaderText);
            mOverlay->addChildWindow(mBinocsText);
            mOverlay->addChildWindow(mLRFText);
            mOverlay->addChildWindow(mCompassText);
            mOverlay->addChildWindow(mGPSText);
            mOverlay->addChildWindow(mToggleFullScreen);
            mOverlay->addChildWindow(mMagnifyModels);
            mOverlay->addChildWindow(mCurrentParticleCount);
            mOverlay->addChildWindow(mLifeTimeParticleCount);
            mOverlay->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)));
            mOverlay->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(1.0f)));
            //mOverlay->setFrameEnabled(false);

            mHeaderText->setFont("DejaVuSans-10");
            mHeaderText->setText("Controls");
            mHeaderText->setPosition(CEGUI::UVector2(cegui_reldim(0.33f), cegui_reldim(0.0f)));
            mHeaderText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mHeaderText->setProperty("FrameEnabled", "false");
            mHeaderText->setProperty("BackgroundEnabled", "false");
            mHeaderText->setHorizontalAlignment(CEGUI::HA_CENTRE);

            mBinocsText->setFont("DejaVuSans-10");
            mBinocsText->setText("Binoculars: 1");
            mBinocsText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.1f)));
            mBinocsText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mBinocsText->setProperty("FrameEnabled", "false");
            mBinocsText->setProperty("BackgroundEnabled", "false");
            mBinocsText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mLRFText->setFont("DejaVuSans-10");
            mLRFText->setText("Laser Range Finder: 2, Spacebar to capture.");
            mLRFText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.2f)));
            mLRFText->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(0.25f)));
            mLRFText->setProperty("FrameEnabled", "false");
            mLRFText->setProperty("BackgroundEnabled", "false");
            mLRFText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mCompassText->setFont("DejaVuSans-10");
            mCompassText->setText("Compass: 3");
            mCompassText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.3f)));
            mCompassText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mCompassText->setProperty("FrameEnabled", "false");
            mCompassText->setProperty("BackgroundEnabled", "false");
            mCompassText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mGPSText->setFont("DejaVuSans-10");
            mGPSText->setText("GPS: 4");
            mGPSText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.4f)));
            mGPSText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mGPSText->setProperty("FrameEnabled", "false");
            mGPSText->setProperty("BackgroundEnabled", "false");
            mGPSText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mCurrentParticleCount->setFont("DejaVuSans-10");
            mCurrentParticleCount->setText("mCurrentParticleCount: 4");
            mCurrentParticleCount->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.85f)));
            mCurrentParticleCount->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mCurrentParticleCount->setProperty("FrameEnabled", "false");
            mCurrentParticleCount->setProperty("BackgroundEnabled", "false");
            mCurrentParticleCount->setHorizontalAlignment(CEGUI::HA_LEFT);

            mLifeTimeParticleCount->setFont("DejaVuSans-10");
            mLifeTimeParticleCount->setText("mLifeTimeParticleCount: 4");
            mLifeTimeParticleCount->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.9f)));
            mLifeTimeParticleCount->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mLifeTimeParticleCount->setProperty("FrameEnabled", "false");
            mLifeTimeParticleCount->setProperty("BackgroundEnabled", "false");
            mLifeTimeParticleCount->setHorizontalAlignment(CEGUI::HA_LEFT);

            mToggleFullScreen->setFont("DejaVuSans-10");
            mToggleFullScreen->setText("Toggle Full Screen: Esc");
            mToggleFullScreen->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.5f)));
            mToggleFullScreen->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mToggleFullScreen->setProperty("FrameEnabled", "false");
            mToggleFullScreen->setProperty("BackgroundEnabled", "false");
            mToggleFullScreen->setHorizontalAlignment(CEGUI::HA_LEFT);

            mMagnifyModels->setFont("DejaVuSans-10");
            mMagnifyModels->setText("+/- Model Size: Page Up/Page Dn");
            mMagnifyModels->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.6f)));
            mMagnifyModels->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mMagnifyModels->setProperty("FrameEnabled", "false");
            mMagnifyModels->setProperty("BackgroundEnabled", "false");
            mMagnifyModels->setHorizontalAlignment(CEGUI::HA_LEFT);

            mCloseButton = static_cast<CEGUI::PushButton*>(wm->createWindow("WindowsLook/Button", "Close Button"));
            mCloseButton->setText("Close");
            mCloseButton->setSize(CEGUI::UVector2(cegui_reldim(0.1f), cegui_reldim(0.1f)));
            mCloseButton->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.8f)));
            mCloseButton->setMouseCursor(NULL);
            mCloseButton->setHorizontalAlignment(CEGUI::HA_CENTRE);

            mOverlay->addChildWindow(mCloseButton);
            mOverlay->setDragMovingEnabled(false);
            mOverlay->setSizingBorderThickness(0);
            mOverlay->setSizingEnabled(false);

            Enable(false);
         }
         catch(const CEGUI::Exception &e) 
         {
            std::ostringstream oss;
            oss << "CEGUI exception caught: " << e.getMessage().c_str();
            throw dtUtil::Exception(dtGame::ExceptionEnum::GAME_APPLICATION_CONFIG_ERROR,
               oss.str(), __FILE__, __LINE__);
         }
      }
   }
}
