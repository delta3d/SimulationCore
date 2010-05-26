/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
 * Chris Rodgers
 */

#ifndef NETDEMO_BUTTON_HIGHLIGHT_H
#define NETDEMO_BUTTON_HIGHLIGHT_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <osg/Referenced>
#include "DemoExport.h"



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace osg
{
   class Geometry;
   class Group;
   class MatrixTransform;
   class Projection;
}

namespace NetDemo
{
   namespace GUI
   {
      namespace Effects
      {
         ///////////////////////////////////////////////////////////////////////
         // CODE
         ///////////////////////////////////////////////////////////////////////
         class NETDEMO_EXPORT ButtonHighlight : public osg::Referenced
         {
            public:
               typedef osg::Referenced BaseClass;

               ButtonHighlight();

               void Init(osg::Group& sceneRoot);

               void Clear();

               /**
                * Set the screen rectangle that this effect should encompass.
                * @param screenBounds X, Y, width and height of the bounds in
                *        normalized screen coordinates.
                * @param flipY Flag to signal this method to treat the Y coordinate
                *        as if it originates from the top of the screen, like in
                *        a windowing system (where +Y is down from the top).
                * NOTE: If "flipY" is FALSE as is the default, X & Y start from the
                * lower left of the screen, to match the openGL coordinate system
                * (where +Y is up from the bottom)
                */
               void SetScreenBounds(const osg::Vec4& screenBounds, bool flipY = false);

               void SetVisible(bool visible);
               bool IsVisible() const;

               void SetEnabled(bool enabled);
               bool IsEnabled() const;

            protected:
               virtual ~ButtonHighlight();

               void CreateDrawable(const std::string& imageFileName);

            private:
               dtCore::RefPtr<osg::Projection> mRoot;
               dtCore::RefPtr<osg::MatrixTransform> mDrawable;
               dtCore::RefPtr<osg::Vec3Array> mVerts;
               dtCore::RefPtr<osg::Geometry> mGeom;
         };

      } // END - Effects namespace
   } // END - GUI namespace
} // END - NetDemo namespace

#endif
