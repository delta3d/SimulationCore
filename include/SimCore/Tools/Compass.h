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
#ifndef _COMPASS_H_
#define _COMPASS_H_

#include <SimCore/Tools/Tool.h>
#include <dtUtil/refcountedbase.h>
#include <dtUtil/refcountedbase.h>

namespace osg
{
   class Group;
   class MatrixTransform;
   class Uniform;
}
namespace dtCore
{
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
                     dtCore::Camera& camera,
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

            void InitLens(osg::Group& hudLayer);

         protected:

            /// Destructor
            virtual ~Compass();

         private:

            bool mUseMagneticNorth;
            float mNeedleRotation;
            float mNeedlePosition;
            float mNeedleVelocity;
            float mNeedleAcceleration;
            float mNeedleTorque;
            float mNeedleDragCoef;

            // Lens related references
            osg::ref_ptr<osg::MatrixTransform> mCompassOverlay;
            osg::ref_ptr<osg::MatrixTransform> mDisk;
            osg::ref_ptr<osg::Uniform> mLensFocus;
            std::weak_ptr<dtCore::Camera> mCamera;
      };
   }
}
#endif
