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

#ifdef None
#undef None
#endif
#include <CEGUI.h>



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

            /**
             * Get all windows of a specific type that are in the CEGUI window system
             * or that are children of a specified root window.
             * NOTE: this method may be performance heavy, so it is advized to
             * call this method only when absolutely necessary.
             * @param rootWindow Window that the found windows should be children of.
             *        If left NULL, all windows in the system of the specified type will
             *        be returned.
             * @param outChildWindows List in which to capture the found windows.
             * @return Number of windows that were found.
             */
            template<class T_CeguiWin>
            static int GetChildWindowsByType(CEGUI::Window* rootWindow,
               std::vector<T_CeguiWin*>& outChildWindows);

            /**
             * Convenience method for determining if a window is truly a child of another.
             * @param parent Window in question about being the parent to "child".
             * @param child Window in question about being a child of "parent".
             * @return TRUE if "child" is a child window of "parent".
             */
            static bool IsParentAndChild(const CEGUI::Window& parent, const CEGUI::Window& child);

         private:
            // This should not be instantiated.
            CeguiUtils();
            ~CeguiUtils();
      };



      //////////////////////////////////////////////////////////////////////////
      // TEMPLATE METHOD DEFINITIONS
      //////////////////////////////////////////////////////////////////////////
      template<class T_CeguiWin>
      int CeguiUtils::GetChildWindowsByType(CEGUI::Window* rootWindow,
         std::vector<T_CeguiWin*>& outChildWindows)
      {
         int successes = 0;

         // Get ready to loop through all the windows in the CEGUI system.
         T_CeguiWin* curWin = NULL;
         CEGUI::WindowManager::WindowIterator winIter = CEGUI::WindowManager::getSingleton().getIterator();
         winIter.toStart();

         // Search all the windows in the window manager.
         while( ! winIter.isAtEnd())
         {
            curWin = dynamic_cast<T_CeguiWin*>(*winIter);

            // If the window if of the specified type...
            if(curWin != NULL)
            {
               // ...add the window to the list only if it is a child
               // of the specified root window.
               if(rootWindow != NULL)
               {
                  if(CeguiUtils::IsParentAndChild(*rootWindow, *curWin))
                  {
                     printf("\n");
                     printf(curWin->getName().c_str());
                     printf("\n");
                     outChildWindows.push_back(curWin);
                     ++successes;
                  }
               }
               else // No root was specified...
               {
                  // ...simply add the window to the list since no root
                  // was specified.
                  outChildWindows.push_back(curWin);
                  ++successes;
               }
            }

            // Get ready for next loop.
            ++winIter;
         }

         return successes;
      }

   } // END - GUI namespace
} // END - SimCore namespace

#endif
