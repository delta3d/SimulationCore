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
#ifndef _BINOCULARS_H_
#define _BINOCULARS_H_

#include <SimCore/Export.h>
#include <SimCore/Tools/Tool.h>

namespace dtCore
{
   class Camera;
   class Isector;
}

namespace CEGUI
{
   class Window;
}

namespace dtCore
{
   class DeltaDrawable;
}

namespace SimCore
{
   class UnitOfAngle;
   class UnitOfLength;

   namespace Tools
   {
      class SIMCORE_EXPORT Binoculars : public Tool
      {
         public:

            /// Constructor
            Binoculars(dtCore::Camera& camera, CEGUI::Window* mainWindow, bool isLRF = false);

            /// Enables/Disables the binoculars
            virtual void Enable(bool enable);

            static const float FAR_CLIPPING_PLANE;
            static const float NEAR_CLIPPING_PLANE;

            /**
             * Sets the zoom factor
             * @param factor The new factor
             */
            void SetZoomFactor(float factor) { mZoomFactor = factor; }

            /**
             * Returns the zoom factor
             * @return mZoomFactor
             */
            float GetZoomFactor() const { return mZoomFactor; }

            /**
             * Toggles showing the distance
             * @param enable True to enable
             */
            void SetShowDistance(bool enable);
            bool GetShowDistance() const;

            /**
             * Toggles showing the distance
             * @param enable True to enable
             */
            void SetShowElevation(bool enable);
            bool GetShowElevation() const;

            /**
             * Toggles showing the reticle
             * @param enable True to enable
             */
            void SetShowReticle(bool enable);
            bool GetShowReticle() const;

            /**
             * Set the relative screen position of the elevation readout from
             * the top left corner of the screen.
             * NOTE: Coordinates of the screen range from 0.0 to 1.0
             * @param x Horizontal position from the left edge of the screen.
             * @param y Vertical position from the top edge of the screen.
             */
            void SetElevationReadoutScreenPosition( float x, float y );

            /**
             * Set the relative screen position of the distance readout from
             * the top left corner of the screen.
             * NOTE: Coordinates of the screen range from 0.0 to 1.0
             * @param x Horizontal position from the left edge of the screen.
             * @param y Vertical position from the top edge of the screen.
             */
            void SetDistanceReadoutScreenPosition( float x, float y );

            /**
             * Updates the intersection text
             */
            void Update(dtCore::DeltaDrawable &terrain);

            void SetOriginalNearFar( float nearValue, float farValue );

            void SetOriginalNear( float nearValue );
            float GetOriginalNear() const { return mOriginalNear; }

            void SetOriginalFar( float nearValue );
            float GetOriginalFar() const { return mOriginalFar; }

            void SetOverlayImage( const std::string& imageset, const std::string& imageName );

            void SetUnitOfLength(SimCore::UnitOfLength& unit);
            SimCore::UnitOfLength& GetUnitOfLength() const;

            void SetUnitOfAngle(SimCore::UnitOfAngle& unit);
            SimCore::UnitOfAngle& GetUnitOfAngle() const;

         protected:

            /// Destructor
            virtual ~Binoculars();

            /**
             * Zooms the camera in
             * @param zoomFactor The distance to move the camera
             */
            void ZoomIn();

            /**
             * Zooms the camera out
             * @param zoomFactor The distance to move the camera
             */
            void ZoomOut();

            /**
             * Accessor to the camera
             * @return mCamera
             */
            dtCore::Camera* GetCamera() { return mCamera.get(); }

            /**
             * Accessor to the original hfov
             * @return mOriginalVFOV
             */
            const float GetOriginalVFOV() const { return mOriginalVFOV; }

            /**
             * Accessor to the original lod scale
             * @return mOriginalLODScale
             */
            const float GetOriginalLODScale() const { return mOriginalLODScale; }

            CEGUI::Window *mIntersectionText;
            CEGUI::Window *mElevationText;
         private:

            SimCore::UnitOfLength* mUnitOfLength;
            SimCore::UnitOfAngle* mUnitOfAngle;

            // Pointer to the camera so the perspective can be changed
            std::shared_ptr<dtCore::Camera> mCamera;

            // Pointer to the binocular overlay
            CEGUI::Window *mOverlay;
            CEGUI::Window *mReticle;
            // Farthest distance you can zoom
            static const unsigned int MAX_ZOOM_DISTANCE = 1000;
            // The original settings of the perspectives and LOD
            float mOriginalVFOV;
            float mOriginalAspect;
            float mOriginalNear;
            float mOriginalFar;
            //This is read each time one zooms in.
            float mOriginalLODScale;
            // Static zooming?
            bool mIsDynamicZooming;
            float mZoomFactor;
            std::shared_ptr<dtCore::Isector> mIsector;
      };
   }
}
#endif
