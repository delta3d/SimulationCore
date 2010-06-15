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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <SimCore/GUI/CeguiUtils.h>



namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      osg::Vec4 CeguiUtils::GetNormalizedScreenBounds(const CEGUI::Window& window)
      {
         osg::Vec4 bounds(0.0f, 0.0f, 1.0f, 1.0f);

         const CEGUI::Window* curWin = &window;
         while(curWin != NULL)
         {
            bounds.z() *= curWin->getWidth().d_scale;
            bounds.w() *= curWin->getHeight().d_scale;

            bounds.x() *= curWin->getWidth().d_scale; // scale child position X
            switch(curWin->getHorizontalAlignment())
            {
            case CEGUI::HA_RIGHT:
               bounds.x() += 1.0f - bounds.z() + curWin->getXPosition().d_scale;
               break;
            case CEGUI::HA_CENTRE:
               bounds.x() += (1.0f - bounds.z()) * 0.5f + curWin->getXPosition().d_scale;
               break;
            default:
               bounds.x() += curWin->getXPosition().d_scale;
               break;
            }

            bounds.y() *= curWin->getHeight().d_scale; // scale child position Y
            switch(curWin->getVerticalAlignment())
            {
            case CEGUI::VA_BOTTOM:
               bounds.y() += 1.0f - bounds.w() + curWin->getYPosition().d_scale;
               break;
            case CEGUI::VA_CENTRE:
               bounds.y() += (1.0f - bounds.w()) * 0.5f + curWin->getYPosition().d_scale;
               break;
            default:
               bounds.y() += curWin->getYPosition().d_scale;
               break;
            }

            curWin = curWin->getParent();
         }

         // Change left-handed screen coordinates (+y down from screen top)
         // to right-handed OpenGL screen coordinates (+y up from screen bottom).
         bounds.y() = 1.0f - bounds.y() - bounds.w(); // W is the height component.

         return bounds;
      }

      //////////////////////////////////////////////////////////////////////////
      bool CeguiUtils::IsParentAndChild(const CEGUI::Window& parent, const CEGUI::Window& child)
      {
         bool success = false;

         const CEGUI::Window* curWin = child.getParent();
         while(curWin != NULL)
         {
            if(curWin == &parent)
            {
               success = true;
               break;
            }

            curWin = curWin->getParent();
         }

         return success;
      }

   } // END - GUI namespace
} // END - SimCore namespace
