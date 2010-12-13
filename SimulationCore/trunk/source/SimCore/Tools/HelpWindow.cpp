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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Tools/HelpWindow.h>
#include <SimCore/Components/BaseHUDElements.h>
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
         CEGUI::String typeText("WindowsLook/StaticText");
         CEGUI::String font("DejaVuSans-10");
         CEGUI::String propFrameEnabled(Components::HUDElement::PROPERTY_FRAME_ENABLED.c_str());
         CEGUI::String propBackEnabled(Components::HUDElement::PROPERTY_BACKGROUND_ENABLED.c_str());
         CEGUI::String propValue("false");

         try
         {
            mMainWindow = mainWindow;
            CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
            mOverlay = static_cast<CEGUI::FrameWindow*>(wm->createWindow("WindowsLook/FrameWindow", "help_window"));
            if(mMainWindow != NULL)
               mMainWindow->addChildWindow(mOverlay);

            mHeaderText       = wm->createWindow(typeText, "header_helptext");
            mBinocsText       = wm->createWindow(typeText, "binocs_helptext");
            mLRFText          = wm->createWindow(typeText, "lrf_helptext");
            mCompassText      = wm->createWindow(typeText, "compass_helptext");
            mGPSText          = wm->createWindow(typeText, "gps_helptext");
            mToggleFullScreen = wm->createWindow(typeText, "fullscreen_helptext");
            mMagnifyModels    = wm->createWindow(typeText, "magnifiy_helptext");
            
            mCurrentParticleCount = wm->createWindow(typeText, "particle_count");
            mLifeTimeParticleCount= wm->createWindow(typeText, "lifetime_particle_count");

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

            mHeaderText->setFont(font);
            mHeaderText->setText("Controls");
            mHeaderText->setPosition(CEGUI::UVector2(cegui_reldim(0.33f), cegui_reldim(0.0f)));
            mHeaderText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mHeaderText->setProperty(propFrameEnabled, propValue);
            mHeaderText->setProperty(propBackEnabled, propValue);
            mHeaderText->setHorizontalAlignment(CEGUI::HA_CENTRE);

            mBinocsText->setFont(font);
            mBinocsText->setText("Binoculars: 1");
            mBinocsText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.1f)));
            mBinocsText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mBinocsText->setProperty(propFrameEnabled, propValue);
            mBinocsText->setProperty(propBackEnabled, propValue);
            mBinocsText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mLRFText->setFont(font);
            mLRFText->setText("Laser Range Finder: 2, Spacebar to capture.");
            mLRFText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.2f)));
            mLRFText->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(0.25f)));
            mLRFText->setProperty(propFrameEnabled, propValue);
            mLRFText->setProperty(propBackEnabled, propValue);
            mLRFText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mCompassText->setFont(font);
            mCompassText->setText("Compass: 3");
            mCompassText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.3f)));
            mCompassText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mCompassText->setProperty(propFrameEnabled, propValue);
            mCompassText->setProperty(propBackEnabled, propValue);
            mCompassText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mGPSText->setFont(font);
            mGPSText->setText("GPS: 4");
            mGPSText->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.4f)));
            mGPSText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mGPSText->setProperty(propFrameEnabled, propValue);
            mGPSText->setProperty(propBackEnabled, propValue);
            mGPSText->setHorizontalAlignment(CEGUI::HA_LEFT);

            mCurrentParticleCount->setFont(font);
            mCurrentParticleCount->setText("mCurrentParticleCount: 4");
            mCurrentParticleCount->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.85f)));
            mCurrentParticleCount->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mCurrentParticleCount->setProperty(propFrameEnabled, propValue);
            mCurrentParticleCount->setProperty(propBackEnabled, propValue);
            mCurrentParticleCount->setHorizontalAlignment(CEGUI::HA_LEFT);

            mLifeTimeParticleCount->setFont(font);
            mLifeTimeParticleCount->setText("mLifeTimeParticleCount: 4");
            mLifeTimeParticleCount->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.9f)));
            mLifeTimeParticleCount->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mLifeTimeParticleCount->setProperty(propFrameEnabled, propValue);
            mLifeTimeParticleCount->setProperty(propBackEnabled, propValue);
            mLifeTimeParticleCount->setHorizontalAlignment(CEGUI::HA_LEFT);

            mToggleFullScreen->setFont(font);
            mToggleFullScreen->setText("Toggle Full Screen: Esc");
            mToggleFullScreen->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.5f)));
            mToggleFullScreen->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mToggleFullScreen->setProperty(propFrameEnabled, propValue);
            mToggleFullScreen->setProperty(propBackEnabled, propValue);
            mToggleFullScreen->setHorizontalAlignment(CEGUI::HA_LEFT);

            mMagnifyModels->setFont(font);
            mMagnifyModels->setText("+/- Model Size: Page Up/Page Dn");
            mMagnifyModels->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim(0.6f)));
            mMagnifyModels->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mMagnifyModels->setProperty(propFrameEnabled, propValue);
            mMagnifyModels->setProperty(propBackEnabled, propValue);
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
            throw dtGame::GameApplicationConfigException(
               oss.str(), __FILE__, __LINE__);
         }
      }
   }
}
