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
#ifndef _COMPASS_H_
#define _COMPASS_H_

#include <SimCore/Tools/Tool.h>
#include <dtCore/refptr.h>

namespace osg
{
   class MatrixTransform;
   class Uniform;
}
namespace dtCore
{
   class DeltaDrawable;
   class Camera;
}

namespace CEGUI
{
   class Window;
}

namespace SimCore
{
   namespace Tools
   {
      class SIMCORE_EXPORT Compass : public Tool
      {
         public:

            /// Constructor
            Compass(CEGUI::Window *mainWindow, 
                    bool useMagNorth = false, 
                    float aspectRatio = 1.6f);

            /**
             * Enables/Disables the compass
             * @param enable True to enable, false to disable
             */
            void Enable(bool enable);

            /// Updates the azimuth display on the compass
            void Update( float timeDelta = 0.0f );
            
            float UpdateNeedle( float deltaTime, float heading );

            void InitLens( dtCore::DeltaDrawable& hudLayer );

         protected:

            /// Destructor
            virtual ~Compass();

         private:

            CEGUI::Window *mOverlay;
            CEGUI::Window  *mAzimuthText;
            CEGUI::Window  *mAzimuthDegreesText;
            bool mUseMagneticNorth;
            float mNeedleRotation;
            float mNeedlePosition;
            float mNeedleVelocity;
            float mNeedleAcceleration;
            float mNeedleTorque;
            float mNeedleDragCoef;

            // Lens related references
            dtCore::RefPtr<osg::MatrixTransform> mLensOverlay;
            dtCore::RefPtr<osg::MatrixTransform> mDisk;
            dtCore::RefPtr<osg::Uniform> mLensFocus;
            dtCore::ObserverPtr<dtCore::Camera> mCamera;
      };
   }
}
#endif
