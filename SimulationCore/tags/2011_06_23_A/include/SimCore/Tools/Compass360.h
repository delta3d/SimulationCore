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
 * @author Chris Rodgers
 */

#ifndef _COMPASS_360_H_
#define _COMPASS_360_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Tools/Tool.h>



namespace osg
{
   class MatrixTransform;
   class Projection;
}

namespace dtCore
{
   class Camera;
}

namespace SimCore
{
   namespace Tools 
   {	
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT Compass360 : public Tool
      {
         public:
            typedef Tool BaseClass;

            Compass360();

            void Init(osg::Group& sceneNode, const std::string& imageFileName);

            void Enable(bool enable);

            void UpdateFOV(dtCore::Camera& camera);

         protected:
            virtual ~Compass360();

            void CreateDrawable(const std::string& imageFileName);

            void DetachFromScene();

         private:
            dtCore::RefPtr<osg::Projection> mRoot;
            dtCore::RefPtr<osg::MatrixTransform> mDrawable;
       };

   }
}

#endif 
