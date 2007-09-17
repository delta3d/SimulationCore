/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Tools/Binoculars.h>

#include <SimCore/Actors/StealthActor.h>

#include <dtGame/exceptionenum.h>

#include <dtUtil/log.h>

#include <dtCore/camera.h>
#include <dtCore/isector.h>

#ifdef None
#undef None
#endif

#include <CEGUI/CEGUI.h> 

namespace SimCore
{
   namespace Tools
   {
      const float Binoculars::FAR_CLIPPING_PLANE  = 11000.0f;
      const float Binoculars::NEAR_CLIPPING_PLANE = 0.25f;
     
      Binoculars::Binoculars(dtCore::Camera &camera, CEGUI::Window *mainWindow, bool isLRF) :
         Tool(mainWindow),
         mIntersectionText(NULL), 
         mElevationText(NULL),
         mCamera(&camera),
         mOverlay(NULL),
         mOriginalHFOV(camera.GetHorizontalFov()),
         mOriginalVFOV(camera.GetVerticalFov()),
         mOriginalLODScale(camera.GetSceneHandler()->GetSceneView()->getLODScale()),
         mIsDynamicZooming(false), 
         mZoomFactor(7.0f)
 	   {
         try
         {
            CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
            mOverlay = wm->createWindow("WindowsLook/StaticImage", !isLRF ? "binoculars_overlay" : "lrf_binocs_overlay");

            if(mainWindow != NULL)
               mainWindow->addChildWindow(mOverlay);
            mOverlay->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)));
            mOverlay->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(1.0f)));
            mOverlay->setProperty("BackgroundEnabled", "false");
            mOverlay->setProperty("FrameEnabled", "false");

            mRecticle = wm->createWindow("WindowsLook/StaticImage", !isLRF ? "binoculars_recticle" : "lrf_recticle");
            mRecticle->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)));
            mRecticle->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(1.0f)));
            mRecticle->setProperty("BackgroundEnabled", "false");
            mRecticle->setProperty("FrameEnabled", "false");

            if(mCamera->GetAspectRatio() < 1.47)
               mRecticle->setProperty("Image", "set:Binoculars4.3 image:Binoculars4.3");
            else
               mRecticle->setProperty("Image", "set:Binoculars8.5 image:Binoculars8.5");
            mOverlay->addChildWindow(mRecticle);

            if(!isLRF)
            {
               mIntersectionText = wm->createWindow("WindowsLook/StaticText", "binos_intersection_text");
            
               mOverlay->addChildWindow(mIntersectionText);
               mIntersectionText->setFont("DejaVuSans-10");
               mIntersectionText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 1, 1)));
               mIntersectionText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
               mIntersectionText->setProperty("FrameEnabled", "false");
               mIntersectionText->setProperty("BackgroundEnabled", "false");
               mIntersectionText->setHorizontalAlignment(CEGUI::HA_LEFT); 
               SetDistanceReadoutScreenPosition(0.925f,0.85f);

               mElevationText = wm->createWindow("WindowsLook/StaticText", "binos_elevation_text");

               mOverlay->addChildWindow(mElevationText);
               mElevationText->setFont("DejaVuSans-10");
               mElevationText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 1, 1)));
               mElevationText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
               mElevationText->setProperty("FrameEnabled", "false");
               mElevationText->setProperty("BackgroundEnabled", "false");
               mElevationText->setHorizontalAlignment(CEGUI::HA_LEFT);
               SetElevationReadoutScreenPosition(0.925f,0.8f);
            }
         }
         catch(CEGUI::Exception &e)
         {
            std::ostringstream oss;
            oss << "CEGUI exception caught: " << e.getMessage().c_str();
            throw dtUtil::Exception(dtGame::ExceptionEnum::GAME_APPLICATION_CONFIG_ERROR, 
               oss.str(), __FILE__, __LINE__);
         }
         Enable(false);
 	   }

 	   Binoculars::~Binoculars()
 	   {
         if(mMainWindow != NULL)
            mMainWindow->removeChildWindow(mOverlay);
         
         CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
         wm->destroyWindow(mOverlay);
 	   }

      void Binoculars::Enable(bool enable)
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
            mCamera->GetSceneHandler()->GetSceneView()->setLODScale(mOriginalLODScale);
            mCamera->SetPerspective(mOriginalHFOV, mOriginalVFOV, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
         }
      }

      void Binoculars::SetElevationReadoutScreenPosition( float x, float y )
      {
         mElevationText->setPosition(CEGUI::UVector2(cegui_reldim(x), cegui_reldim(y)));
      }

      void Binoculars::SetDistanceReadoutScreenPosition( float x, float y )
      {
         mIntersectionText->setPosition(CEGUI::UVector2(cegui_reldim(x), cegui_reldim(y)));
      }

      void Binoculars::ZoomIn()
      {
         if(!IsEnabled())
            return;

         // Make sure to check for this whack state
         if(mZoomFactor == 0)
         {
            LOG_ERROR("The zoom factor is 0. Cannot divide by 0.");
            return;
         }

         // Yet another whack state
         if(mZoomFactor < 0)
         {
            LOG_INFO("The zoom factor is negative. Setting zoom factor to 1");
            mZoomFactor = 1.0f;
         }

         float vfov = mCamera->GetVerticalFov();
         float hfov = mCamera->GetHorizontalFov();

         // TODO fix this algorithm later
         if(mIsDynamicZooming)
         {
            // Check for maximum zoom
            // If this is the very first zoom, the fov will be equal to the original
            // Therefor, we need to check to make sure the zoomFactor isn't greater than
            // the max zoom factor
            if(vfov == mZoomFactor)
            {
               if(mZoomFactor >= MAX_ZOOM_DISTANCE)
               {
                  LOG_ERROR("The zoom factor specified is greater than the maximum zoom distance. Not zooming in.");
                  return;
               }
            }
            else
            {
               if((vfov / mZoomFactor) > MAX_ZOOM_DISTANCE)
               {
                  vfov /= mZoomFactor;
                  hfov /= mZoomFactor;
               }
               else
               {
                  LOG_INFO("The maximum zoom distance has been attained. Not zooming in further.");
                  return;
               }
            }
         }

         // Check to see if we are already zoomed in
         if(vfov == mOriginalVFOV && hfov == mOriginalHFOV)
         {
            if(mCamera->GetAspectRatio() < 1.47)
            {
               vfov /= (mZoomFactor *= 1.135f);
               hfov /= (mZoomFactor *= 1.135f);
            }
            else
            {
               vfov /= (mZoomFactor *= 1.103f);
               hfov /= (mZoomFactor *= 1.103f);
            }
         }
          else
            return;

         mOriginalLODScale = mCamera->GetSceneHandler()->GetSceneView()->getLODScale();
         float newZoom = (1.0f / mZoomFactor) * mOriginalLODScale;
         mCamera->GetSceneHandler()->GetSceneView()->setLODScale(newZoom);
         mCamera->SetPerspective(hfov, vfov, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
      }

      void Binoculars::ZoomOut()
      {
         if(!IsEnabled())
            return;

         if(mZoomFactor <= 0)
         {
            LOG_INFO("The zoom factor was equal or less than zero. Setting to 1.");
            mZoomFactor = 1.0f;
         }

         float vfov = mCamera->GetVerticalFov();
         float hfov = mCamera->GetHorizontalFov();

         // TODO fix this algorithm later
         if(mIsDynamicZooming)
         {
            if(vfov * mZoomFactor < mOriginalVFOV)
            {
               vfov *= mZoomFactor;
               hfov *= mZoomFactor;
            }
            else
            {
               LOG_INFO("The zoom distance is equal to the starting distance. Not zooming out further.");
               return;
            }
         }

         // Check to see if we are zoomed out already
         if(vfov != mOriginalVFOV && hfov != mOriginalHFOV)
         {
            vfov *= mZoomFactor;
            hfov *= mZoomFactor;
         }
         else
            return;

         mCamera->GetSceneHandler()->GetSceneView()->setLODScale(mOriginalLODScale);
         mCamera->SetPerspective(hfov, vfov, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
      }

      void Binoculars::Update(dtCore::DeltaDrawable &terrain)
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

         if(mIsector->Update())
         {
            osg::Vec3 point;
            mIsector->GetHitPoint(point);

            int distance = int(((point - playerPos).length()));
            mIntersectionText->setText(PadNumber(distance));

            float elevation = point.z() - playerPos.z();

            // Convert to possibly negative mils, formula in dtUtil::Coordinates
            // forces mils to be positive
            int mils = int(CalculateMils(distance, elevation));
            // Instead of converting to degrees to clamp between -45, and 45, 
            // clamp the mils between -800, 800
            dtUtil::Clamp(mils, -800, 800);
            mElevationText->setText(PadNumber(mils));
         }
         else
         {
            mIntersectionText->setText("0");
            mElevationText->setText("0");
         }
      }

      void Binoculars::SetShowReticle(bool enable)
      { 
         if( enable )
         {
            mRecticle->show();
         }
         else
         {
            mRecticle->hide();
         }
      }

      void Binoculars::SetShowDistance(bool enable)
      {
         if( mIntersectionText != NULL )
         {
            if( enable )
            {
               mIntersectionText->show();
            }
            else
            {
               mIntersectionText->hide();
            }
         }
      }

      void Binoculars::SetShowElevation(bool enable)
      {
         if( mElevationText != NULL )
         {
            if( enable )
            {
               mElevationText->show();
            }
            else
            {
               mElevationText->hide();
            }
         }
      }
   }
}
