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
* @author Allen Danklefsen
*/
#ifndef _MATRIX_MANIPULATIONS_
#define _MATRIX_MANIPULATIONS_

#include <SimCore/Export.h>
#include <dtUtil/refcountedbase.h>
#include <osg/Matrix>
#include <dtCore/isector.h>
#include <dtCore/camera.h>
#include <dtCore/mouse.h>

namespace SimCore
{
   namespace Components
   {
      class SIMCORE_EXPORT Matrix_Manipulations : public std::enable_shared_from_this
      {
         public:
            enum MATRIX_MANIPULATIONS_MODES 
            {
                  FLY_MODE = 0, 
                  ATTACH_TO_UNLOCKED,
                  HARDATTACH_MODE,
                  SOFTATTACH_MODE,
                  VELOCITY_FOLLOW_MODE,
                  REARVIEW, 
                  SIDE_VIEW_LEFT,
                  SIDE_VIEW_RIGHT,
               MAX_MANIPULATIONS_MODES
            };

            struct Matrix_Manipulations_Matrix
            {
               float mMatrix[16];

               float& operator[](unsigned int iter)
               {
                  return mMatrix[iter];
               }

               void operator=(Matrix_Manipulations_Matrix from)
               {
                  for(int i = 0; i < 16; ++i)
                     mMatrix[i] = from[i];
               }

               float* operator&()
               {
                  return mMatrix;
               }
            };

         public:    
            Matrix_Manipulations(MATRIX_MANIPULATIONS_MODES camTypeToUse = ATTACH_TO_UNLOCKED, unsigned int softAttachFrames = 30)
            {
               mAmountOfSoftAttach = softAttachFrames;
               mIsector = new dtCore::Isector();
               mCurrentMode = camTypeToUse;
            }
            virtual ~Matrix_Manipulations(){}
            
            void ToggleCamera(MATRIX_MANIPULATIONS_MODES camTypeToUse);
            void UpdateCamera(float deltaTime, 
                              Matrix_Manipulations_Matrix& inMatrix, 
                              Matrix_Manipulations_Matrix& outMatrix, 
                              osg::Vec3& offsetVector,
                              dtCore::Mouse* ourMouse = nullptr, dtCore::DeltaDrawable* drawableForISectorTests = nullptr);
            MATRIX_MANIPULATIONS_MODES GetCurrentManipulationMode() {return mCurrentMode;}
            void HPRClamp( osg::Vec3& clampValues, 
                           osg::Vec3& originalOrientateClamp, 
                           osg::Vec3& modifiedOrientateClamp, 
                           osg::Vec3& outGoingOrientateClamp);

         private: 
            MATRIX_MANIPULATIONS_MODES       mCurrentMode;        /// Current camera mode for update mode
            std::list<osg::Matrix>           mSoftAttachList;     /// for soft attach matrix transforms for the camera
            std::shared_ptr<dtCore::Isector>  mIsector;            /// so we know how to collide with terrain if needed. 
            unsigned int                     mAmountOfSoftAttach; /// how many matrix pushes u doing before u pop, todo change for frame time
            osg::Vec2                        mHPROffset;
      };
   }
}

#endif
