/*
 * Copyright, 2007, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * Chris Rodgers
 */

#ifndef SIMCORE_CEGUI_UTILS_H
#define SIMCORE_CEGUI_UTILS_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <osg/Vec4>
#include <SimCore/Export.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace CEGUI
{
   class Window;
}

namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT CeguiUtils
      {
         public:
            /**
            * Get the absolute normalized OpenGL screen coordinates and size for
            * a nested CEGUI window.
            * NOTE: The CEGUI window must be at its final level of nesting in order
            *       for this function to return a proper result.
            * NOTE: CEGUI Y-Axis is +y down from top of screen while the returned
            *       OpenGL coordinates are returned as +y up from the bottom of the screen.
            */
            static osg::Vec4 GetNormalizedScreenBounds(const CEGUI::Window& window);

         private:
            // This should not be instantiated.
            CeguiUtils();
            ~CeguiUtils();
      };

   } // END - GUI namespace
} // END - SimCore namespace

#endif
