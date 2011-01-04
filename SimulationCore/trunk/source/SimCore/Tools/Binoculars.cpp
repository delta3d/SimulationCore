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

#include <SimCore/Tools/Binoculars.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/UnitEnums.h>

#include <dtGame/exceptionenum.h>

#include <dtUtil/log.h>

#include <dtCore/camera.h>
#include <dtCore/isector.h>
#include <dtCore/transform.h>

#include <dtUtil/stringutils.h>

#include <CEGUI/CEGUI.h>

#ifdef None
#undef None
#endif


#include <iostream>

namespace SimCore
{
   namespace Tools
   {
      const float Binoculars::FAR_CLIPPING_PLANE  = 11000.0f;
      const float Binoculars::NEAR_CLIPPING_PLANE = 0.25f;

      Binoculars::Binoculars(dtCore::Camera& camera, CEGUI::Window* mainWindow, bool isLRF)
      : Tool(mainWindow)
      , mIntersectionText(NULL)
      , mElevationText(NULL)
      , mUnitOfLength(&SimCore::UnitOfLength::METER)
      , mUnitOfAngle(&SimCore::UnitOfAngle::MIL)
      , mCamera(&camera)
      , mOverlay(NULL)
      , mOriginalVFOV(camera.GetVerticalFov())
      , mOriginalAspect(camera.GetAspectRatio())
      , mOriginalNear(NEAR_CLIPPING_PLANE)
      , mOriginalFar(FAR_CLIPPING_PLANE)
      , mOriginalLODScale(camera.GetOSGCamera()->getLODScale())
      , mIsDynamicZooming(false)
      , mZoomFactor(7.0f)
 	   {
         CEGUI::String propFrameEnabled(Components::HUDElement::PROPERTY_FRAME_ENABLED.c_str());
         CEGUI::String propBackEnabled(Components::HUDElement::PROPERTY_BACKGROUND_ENABLED.c_str());
         CEGUI::String propValue("false");

         try
         {
            CEGUI::WindowManager*wm = CEGUI::WindowManager::getSingletonPtr();
            mOverlay = wm->createWindow("WindowsLook/StaticImage", !isLRF ? "binoculars_overlay" : "lrf_binocs_overlay");

            if(mainWindow != NULL)
               mainWindow->addChildWindow(mOverlay);
            mOverlay->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)));
            mOverlay->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(1.0f)));
            mOverlay->setProperty(propBackEnabled, propValue);
            mOverlay->setProperty(propFrameEnabled, propValue);

            mReticle = wm->createWindow("WindowsLook/StaticImage", !isLRF ? "binoculars_recticle" : "lrf_recticle");
            mReticle->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)));
            mReticle->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(1.0f)));
            mReticle->setProperty(propBackEnabled, propValue);
            mReticle->setProperty(propFrameEnabled, propValue);

            if(mCamera->GetAspectRatio() < 1.47)
            {
               SetOverlayImage("Binoculars4.3","Binoculars4.3");
            }
            else
            {
               SetOverlayImage("Binoculars8.5","Binoculars8.5");
            }

            mOverlay->addChildWindow(mReticle);

            if (!isLRF)
            {
               mIntersectionText = wm->createWindow("WindowsLook/StaticText", "binos_intersection_text");

               mOverlay->addChildWindow(mIntersectionText);
               mIntersectionText->setFont("DejaVuSans-10");
               mIntersectionText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 1, 1)));
               mIntersectionText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
               mIntersectionText->setProperty(propFrameEnabled, propValue);
               mIntersectionText->setProperty(propBackEnabled, propValue);
               mIntersectionText->setHorizontalAlignment(CEGUI::HA_LEFT);
               SetDistanceReadoutScreenPosition(0.910f,0.85f);

               mElevationText = wm->createWindow("WindowsLook/StaticText", "binos_elevation_text");

               mOverlay->addChildWindow(mElevationText);
               mElevationText->setFont("DejaVuSans-10");
               mElevationText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 1, 1)));
               mElevationText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
               mElevationText->setProperty(propFrameEnabled, propValue);
               mElevationText->setProperty(propBackEnabled, propValue);
               mElevationText->setHorizontalAlignment(CEGUI::HA_LEFT);
               SetElevationReadoutScreenPosition(0.910f,0.8f);
            }

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

      ////////////////////////////////////////////////
      Binoculars::~Binoculars()
      {
         if(mMainWindow != NULL)
            mMainWindow->removeChildWindow(mOverlay);

         CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
         wm->destroyWindow(mOverlay);
      }

      ////////////////////////////////////////////////
      void Binoculars::SetOriginalNearFar(float nearValue, float farValue)
      {
         mOriginalNear = nearValue;
         mOriginalFar = farValue;
      }

      ////////////////////////////////////////////////
      void Binoculars::SetOriginalNear(float nearValue)
      {
         mOriginalNear = nearValue;
      }

      ////////////////////////////////////////////////
      void Binoculars::SetOriginalFar(float farValue)
      {
         mOriginalFar = farValue;
      }

      ////////////////////////////////////////////////
      void Binoculars::Enable(bool enable)
      {
         bool wasEnabled = IsEnabled();
         Tool::Enable(enable);

         if (wasEnabled != IsEnabled())
         {
            if(IsEnabled())
            {
               mOverlay->show();
               ZoomIn();
            }
            else
            {
               mOverlay->hide();
               mCamera->GetOSGCamera()->setLODScale(mOriginalLODScale);
               double vfov, aspect, nearPlane, farPlane;
               mCamera->GetPerspectiveParams(vfov, aspect, nearPlane, farPlane);
               std::cout << mOriginalVFOV << " " << mOriginalAspect << " " << nearPlane << " " << farPlane << std::endl;
               mCamera->SetPerspectiveParams(mOriginalVFOV, mOriginalAspect, nearPlane, farPlane);
            }
         }
      }

      ////////////////////////////////////////////////
      void Binoculars::SetElevationReadoutScreenPosition(float x, float y)
      {
         mElevationText->setPosition(CEGUI::UVector2(cegui_reldim(x), cegui_reldim(y)));
      }

      ////////////////////////////////////////////////
      void Binoculars::SetDistanceReadoutScreenPosition(float x, float y)
      {
         mIntersectionText->setPosition(CEGUI::UVector2(cegui_reldim(x), cegui_reldim(y)));
      }

      ////////////////////////////////////////////////
      void Binoculars::ZoomIn()
      {
         if(!IsEnabled())
            return;

         if (mZoomFactor <= 0)
         {
            LOG_INFO("The zoom factor is less than or equal to 0. Setting zoom factor to 1");
            mZoomFactor = 1.0f;
         }

         double vfov, aspect, nearPlane, farPlane;

         mCamera->GetPerspectiveParams(vfov, aspect, nearPlane, farPlane);

         mOriginalVFOV = vfov;
         mOriginalAspect = aspect;

         // TODO fix this algorithm later
         if (mIsDynamicZooming)
         {
            // Check for maximum zoom
            // If this is the very first zoom, the fov will be equal to the original
            // Therefore, we need to check to make sure the zoomFactor isn't greater than
            // the max zoom factor
            if (vfov == mZoomFactor)
            {
               if (mZoomFactor >= MAX_ZOOM_DISTANCE)
               {
                  LOG_ERROR("The zoom factor specified is greater than the maximum zoom distance. Not zooming in.");
                  return;
               }
            }
            else
            {
               if ((vfov / mZoomFactor) > MAX_ZOOM_DISTANCE)
               {
                  vfov /= mZoomFactor;
               }
               else
               {
                  LOG_INFO("The maximum zoom distance has been attained. Not zooming in further.");
                  return;
               }
            }
         }

         if (mOriginalAspect < 1.47)
         {
            vfov /= (mZoomFactor * 1.135f);
         }
         else
         {
            vfov /= (mZoomFactor * 1.103f);
         }

         mOriginalLODScale = mCamera->GetOSGCamera()->getLODScale();
         float newLODScale = (1.0f / mZoomFactor) * mOriginalLODScale;

         mCamera->GetOSGCamera()->setLODScale(newLODScale);

         std::cout << mZoomFactor << " " << vfov << " " << mOriginalAspect << " " << nearPlane << " " << farPlane << std::endl;
         mCamera->SetPerspectiveParams(vfov, mOriginalAspect, nearPlane, farPlane);
      }

      ////////////////////////////////////////////////
      void Binoculars::ZoomOut()
      {
         if(!IsEnabled())
            return;

         if(mZoomFactor <= 0)
         {
            LOG_INFO("The zoom factor was equal or less than zero. Setting to 1.");
            mZoomFactor = 1.0f;
         }

         double vfov, aspect, nearPlane, farPlane;
         mCamera->GetPerspectiveParams(vfov, aspect, nearPlane, farPlane);

         // TODO fix this algorithm later, this is currently broken.
         if (mIsDynamicZooming)
         {
            if (vfov * mZoomFactor < mOriginalVFOV)
            {
               vfov *= mZoomFactor;
            }
            else
            {
               LOG_INFO("The zoom distance is equal to the starting distance. Not zooming out further.");
               return;
            }
         }

         mCamera->GetOSGCamera()->setLODScale(mOriginalLODScale);
         mCamera->SetPerspectiveParams(mOriginalVFOV, mOriginalAspect, nearPlane, farPlane);
      }

      ////////////////////////////////////////////////
      void Binoculars::Update(dtCore::DeltaDrawable& terrain)
      {
         if(!IsEnabled() || GetPlayerActor() == NULL)
            return;

         if(!mIsector.valid())
         {
            mIsector = new dtCore::Isector;
            mIsector->SetScene(terrain.GetSceneParent());
            mIsector->SetGeometry(&terrain);
         }

         dtCore::Transform xform;
         GetPlayerActor()->GetTransform(xform);

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

         if (mIsector->Update())
         {
            osg::Vec3 point;
            mIsector->GetHitPoint(point);

            float distance = int(((point - playerPos).length()));
            float resultingDistance = SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, *mUnitOfLength, distance);
            if (mUnitOfLength->GetUseWholeUnits())
            {
               mIntersectionText->setText(PadNumber(int(resultingDistance)) + " " + mUnitOfLength->GetAbbreviation());
            }
            else
            {
               mIntersectionText->setText(dtUtil::ToString(resultingDistance, 3) + " " + mUnitOfLength->GetAbbreviation());
            }

            float elevation = point.z() - playerPos.z();

            float degrees = CalculateDegrees(distance, elevation);
            // Instead of converting to degrees to clamp between -45, and 45,
            // clamp the mils between -800, 800
            dtUtil::Clamp(degrees, -45.0f, 45.0f);

            float resultingAngle = SimCore::UnitOfAngle::Convert(SimCore::UnitOfAngle::DEGREE, *mUnitOfAngle, degrees);

            if (mUnitOfAngle->GetUseWholeUnits())
            {
               mElevationText->setText(PadNumber(int(resultingAngle)) + " " + mUnitOfAngle->GetAbbreviation());
            }
            else
            {
               mElevationText->setText(dtUtil::ToString(resultingAngle, 3) + " " + mUnitOfAngle->GetAbbreviation());
            }
         }
         else
         {
            mIntersectionText->setText(PadNumber(0) + " " + mUnitOfLength->GetAbbreviation());
            mElevationText->setText(PadNumber(0) + " " + mUnitOfAngle->GetAbbreviation());
         }
      }

      ////////////////////////////////////////////////
      void Binoculars::SetShowReticle(bool enable)
      {
         if (enable)
         {
            mReticle->show();
         }
         else
         {
            mReticle->hide();
         }
      }

      ////////////////////////////////////////////////
      void Binoculars::SetShowDistance(bool enable)
      {
         if (mIntersectionText != NULL)
         {
            if (enable)
            {
               mIntersectionText->show();
               mIntersectionText->setAlwaysOnTop(true);
            }
            else
            {
               mIntersectionText->hide();
            }
         }
      }

      ////////////////////////////////////////////////
      void Binoculars::SetShowElevation(bool enable)
      {
         if (mElevationText != NULL)
         {
            if (enable)
            {
               mElevationText->show();
               mElevationText->setAlwaysOnTop(true);
            }
            else
            {
               mElevationText->hide();
            }
         }
      }

      ////////////////////////////////////////////////
      bool Binoculars::GetShowDistance() const
      {
         return mIntersectionText->isVisible(true);
      }

      ////////////////////////////////////////////////
      bool Binoculars::GetShowElevation() const
      {
         return mElevationText->isVisible(true);
      }


      ////////////////////////////////////////////////
      bool Binoculars::GetShowReticle() const
      {
         return mReticle->isVisible(true);
      }

      ////////////////////////////////////////////////
      void Binoculars::SetOverlayImage(const std::string& imageset, const std::string& imageName)
      {
         std::string imageSetAndName("set:"+imageset+" image:"+imageName);
         mReticle->setProperty("Image", imageSetAndName);
      }

      /////////////////////////////////////////////////////////////////////////
      void Binoculars::SetUnitOfLength(SimCore::UnitOfLength& unit)
      {
         mUnitOfLength = &unit;
      }

      /////////////////////////////////////////////////////////////////////////
      SimCore::UnitOfLength& Binoculars::GetUnitOfLength() const
      {
         return *mUnitOfLength;
      }

      /////////////////////////////////////////////////////////////////////////
      void Binoculars::SetUnitOfAngle(SimCore::UnitOfAngle& unit)
      {
         mUnitOfAngle = &unit;
      }

      /////////////////////////////////////////////////////////////////////////
      SimCore::UnitOfAngle& Binoculars::GetUnitOfAngle() const
      {
         return *mUnitOfAngle;
      }
   }
}
