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
#ifndef _LASER_RANGE_FINDER_H_
#define _LASER_RANGE_FINDER_H_

#include <SimCore/Tools/Binoculars.h>

namespace dtCore
{
   class Isector;
}

namespace CEGUI
{
   /*class StaticImage;
   class StaticText;*/
   class Window;
}

namespace dtCore
{
   class Transform;
}

namespace SimCore
{
   namespace Tools 
   {	
      class SIMCORE_EXPORT LaserRangeFinder : public Binoculars
      {
         public:

            /// Constructor
            LaserRangeFinder(dtCore::Camera &camera, CEGUI::Window *mainWindow);

            /// Enables/Disables the LaserRangeFinder
            void Enable(bool enable);

            /**
             * Calculates the intersection point
             * @param terrain The drawable object to be used for intersection
             * @param xform The current transform of the eyepoint
             */
            void FindIntersectionPoint(dtCore::DeltaDrawable &terrain, const dtCore::Transform &xform);

         protected:

            /// Destructor
            virtual ~LaserRangeFinder();

         private:

            CEGUI::Window *mOverlay;
            //, *mIntersectionText;
            dtCore::RefPtr<dtCore::Isector> mIsector;
            //bool mShowReticle;
            //bool mShowDistance;
       };
   }
}
#endif 
