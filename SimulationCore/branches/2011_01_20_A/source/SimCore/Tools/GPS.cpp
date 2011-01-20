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
#include <ostream>

#include <SimCore/Tools/GPS.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Components/BaseHUDElements.h>

#include <dtGame/exceptionenum.h>

#include <dtCore/transform.h>

#include <dtUtil/log.h>
#include <dtUtil/coordinates.h>

#ifdef None
#undef None
#endif

#include <CEGUI/CEGUI.h>

namespace SimCore
{
   namespace Tools
   {
      GPS::GPS(CEGUI::Window *mainWindow) :
         Tool(mainWindow),
         mPosText(NULL)
      {
         using namespace Components;

         try
         {
            CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
            mPosText = wm->createWindow("WindowsLook/StaticText", "GPSY_text");

            if(mainWindow != NULL)
            {
               mainWindow->addChildWindow(mPosText);
            }

            mPosText->setFont("DejaVuSans-10");
            mPosText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 1, 1)));
            mPosText->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.80f)));
            mPosText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mPosText->setProperty(HUDElement::PROPERTY_FRAME_ENABLED.c_str(), "false");
            mPosText->setProperty(HUDElement::PROPERTY_BACKGROUND_ENABLED.c_str(), "false");
            mPosText->setHorizontalAlignment(CEGUI::HA_LEFT);
         }
         catch(CEGUI::Exception &e)
         {
            std::ostringstream oss;
            oss << "CEGUI exception caught: " << e.getMessage().c_str();
            throw dtGame::GameApplicationConfigException(
               oss.str(), __FILE__, __LINE__);
         }
         Enable(false);

         GetCoordinateConverter().SetLocalCoordinateType(dtUtil::LocalCoordinateType::CARTESIAN_UTM);
      }

      GPS::~GPS()
      {
         if(mMainWindow != NULL)
            mMainWindow->removeChildWindow(mPosText);
         mPosText->destroy();
      }

      void GPS::Enable(bool enable)
      {
         Tool::Enable(enable);

         IsEnabled() ? mPosText->show() : mPosText->hide();
      }

      void GPS::Update()
      {
         if (IsEnabled() && GetPlayerActor() != NULL)
         {
            dtCore::Transform xform;
            GetPlayerActor()->GetTransform(xform);
            osg::Vec3 loc;
            xform.GetTranslation(loc);

            GetCoordinateConverter().SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
            osg::Vec3d latLonElev = GetCoordinateConverter().ConvertToRemoteTranslation(loc);

            unsigned ewZone;
            char nsZone;

            dtUtil::Coordinates::CalculateUTMZone(latLonElev[0], latLonElev[1], ewZone, nsZone);

            GetCoordinateConverter().SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::UTM);
            osg::Vec3d eastingNorthingElev = GetCoordinateConverter().ConvertToRemoteTranslation(loc);

            std::string milgrid = dtUtil::Coordinates::ConvertUTMToMGRS(eastingNorthingElev.x(), eastingNorthingElev.y(), ewZone, nsZone, 5);

            mPosText->setText(milgrid);
         }
      }
   }
}
