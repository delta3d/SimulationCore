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

#include <SimCore/Tools/LaserRangeFinder.h>
#include <SimCore/Tools/Binoculars.h>
#include <SimCore/Components/BaseHUDElements.h>

#include <SimCore/Actors/StealthActor.h>
#include <SimCore/UnitEnums.h>

#include <dtUtil/log.h>
#include <dtUtil/coordinates.h>

#include <dtGame/gamemanager.h>
#include <dtGame/exceptionenum.h>

#include <dtCore/isector.h>
#include <dtCore/camera.h>
#include <dtCore/transform.h>

#ifdef None
#undef None
#endif

#include <CEGUI/CEGUI.h> 

namespace SimCore
{
   namespace Tools
   {
      LaserRangeFinder::LaserRangeFinder(dtCore::Camera &camera, CEGUI::Window *mainWindow)
         : Binoculars(camera, mainWindow, true),
           mOverlay(NULL)//,
           //mElevationText(NULL)//,
           //mIntersectionText(NULL)
      {
         using namespace Components;

         try
         {
            CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
            mOverlay          = wm->createWindow("WindowsLook/StaticImage", "lrf_overlay");
            //mIntersectionText = wm->createWindow("WindowsLook/StaticText" , "lrf_intersection_text");
            mElevationText    = wm->createWindow("WindowsLook/StaticText",  "lrf_elevation_text");

            if(mainWindow != NULL)
               mainWindow->addChildWindow(mOverlay);
            mOverlay->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)));
            mOverlay->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(1.0f)));
            mOverlay->setProperty(HUDElement::PROPERTY_BACKGROUND_ENABLED.c_str(), "false");
            mOverlay->setProperty(HUDElement::PROPERTY_FRAME_ENABLED.c_str(), "false");
            mOverlay->setProperty(HUDElement::PROPERTY_IMAGE.c_str(), "set:LRF image:LRF");

            /*mOverlay->addChildWindow(mIntersectionText);
            mIntersectionText->setFont("DejaVuSans-10");
            mIntersectionText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 1, 1)));
            mIntersectionText->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.60f)));
            mIntersectionText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mIntersectionText->setProperty("FrameEnabled", "false");
            mIntersectionText->setProperty("BackgroundEnabled", "false");
            mIntersectionText->setHorizontalAlignment(CEGUI::HA_LEFT);*/

            /*mOverlay->addChildWindow(mElevationText);
            mElevationText->setFont("DejaVuSans-10");
            mElevationText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 1, 1)));
            mElevationText->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.65f)));
            mElevationText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mElevationText->setProperty("FrameEnabled", "false");
            mElevationText->setProperty("BackgroundEnabled", "false");
            mElevationText->setHorizontalAlignment(CEGUI::HA_LEFT);*/

            mElevationText->setText("");
            mOverlay->hide();
         }
         catch(CEGUI::Exception &e)
         {
            std::ostringstream oss;
            oss << "CEGUI exception caught: " << e.getMessage().c_str();
            throw dtGame::GameApplicationConfigException(
               oss.str(), __FILE__, __LINE__);
         }
      }

      LaserRangeFinder::~LaserRangeFinder()
      {
         if(mIntersectionText != NULL)
         {
            mOverlay->removeChildWindow(mIntersectionText);
            mIntersectionText->destroy();
         }
         if(mElevationText != NULL)
         {
            mOverlay->removeChildWindow(mElevationText);
            mElevationText->destroy();
         }
         if(mMainWindow != NULL)
         {
            mMainWindow->removeChildWindow(mOverlay);
            mOverlay->destroy();
         }
      }

      void LaserRangeFinder::Enable(bool enable)
      {
         Tool::Enable(enable);

         if(IsEnabled())
         {
            mOverlay->show();
            ZoomIn();
         }
         else
         {
            mOverlay->hide();
            GetCamera()->GetOSGCamera()->setLODScale(GetOriginalLODScale());
            GetCamera()->SetPerspectiveParams(GetOriginalVFOV(), GetCamera()->GetAspectRatio(), GetOriginalNear(), GetOriginalFar());
            //mIntersectionText->setText("");
            mElevationText->setText("");
         }
      }

      void LaserRangeFinder::FindIntersectionPoint(dtCore::DeltaDrawable &terrain, const dtCore::Transform &xform)
      {
         if(!IsEnabled() )//|| GetPlayerActor() == NULL)
            return;

         if(!mIsector.valid())
         {
            mIsector = new dtCore::Isector;
            mIsector->SetScene( terrain.GetSceneParent() );
            mIsector->SetGeometry( &terrain );
         }

         osg::Vec3 playerPos, unitVec(0, 1, 0);
         osg::Matrix matrix;
         xform.GetTranslation(playerPos);
         xform.GetRotation(matrix);
         unitVec = unitVec * matrix;
         mIsector->SetStartPosition(playerPos);
         unitVec *= 10000;
         mIsector->Reset();
         mIsector->SetEndPosition(playerPos + unitVec);
         mIsector->SetGeometry(&terrain);

         if(mIsector->Update())
         {
            osg::Vec3 point;
            mIsector->GetHitPoint(point);

            int distance = int(((point - playerPos).length()));
            mIntersectionText->setText(PadNumber(distance));

            float elevation = point.z() - playerPos.z();

            float degrees = CalculateDegrees(distance, elevation);
            // Instead of converting to degrees to clamp between -45, and 45,
            // clamp the mils between -800, 800
            dtUtil::Clamp(degrees, -45.0f, 45.0f);

            float resultingAngle = SimCore::UnitOfAngle::Convert(SimCore::UnitOfAngle::DEGREE, SimCore::UnitOfAngle::MIL, degrees);

            mElevationText->setText(PadNumber(int(resultingAngle)) + " " + SimCore::UnitOfAngle::MIL.GetAbbreviation());
         }
         else
         {
            mElevationText->setText("0");
            mIntersectionText->setText("0");
         }
      }
   }
}
