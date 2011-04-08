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
             * Filter object, that acts like a predicate, that determines the conditions
             * under which a window can be accepted during a search operation.
             * This class is intended to be sub-classed to override the Accept method.
             */
            template<typename T_CeguiWin>
            class CeguiWindowFilter
            {
               public:
                  /**
                   * Override method.
                   * Method that determines if the specified window is acceptable.
                   * @param window The window to be checked.
                   * @return TRUE if the window is acceptable.
                   */
                  virtual bool Accept(const T_CeguiWin& window)
                  {
                     return true;
                  }
            };

            /**
             * Get all windows of a specific type that are in the CEGUI window system
             * or that are children of a specified root window.
             * NOTE: this method may be performance heavy, so it is advized to
             * call this method only when absolutely necessary.
             * @param outChildWindows List in which to capture the found windows.
             * @param rootWindow Window that the found windows should be children of.
             *        If left NULL, all windows in the system of the specified type will
             *        be returned.
             * @param filter Pointer to an object that  determines the conditions under
             *        which a window can be added to the outChildWindows list.
             * @return Number of windows that were found.
             */
            template<typename T_CeguiWin>
            static int GetChildWindowsByType(std::vector<T_CeguiWin*>& outChildWindows,
               const CEGUI::Window* rootWindow, CeguiWindowFilter<T_CeguiWin>* filter = NULL);

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
      template<typename T_CeguiWin>
      int CeguiUtils::GetChildWindowsByType(std::vector<T_CeguiWin*>& outChildWindows,
         const CEGUI::Window* rootWindow, CeguiWindowFilter<T_CeguiWin>* filter)
      {
         int successes = 0;

         // Get ready to loop through all the windows in the CEGUI system.
         T_CeguiWin* curWin = NULL;
         CEGUI::WindowManager::WindowIterator winIter = CEGUI::WindowManager::getSingleton().getIterator();
         winIter.toStart();

         // Search all the windows in the window manager.
         bool acceptWindow = true;
         while( ! winIter.isAtEnd())
         {
            curWin = dynamic_cast<T_CeguiWin*>(*winIter);

            // If the window if of the specified type...
            if(curWin != NULL)
            {
               // Filter the current window if a filter was specified.
               if(filter != NULL)
               {
                  acceptWindow = filter->Accept(*curWin);
               }

               if(acceptWindow)
               {
                  // ...add the window to the list only if it is a child
                  // of the specified root window.
                  if(rootWindow != NULL)
                  {
                     if(CeguiUtils::IsParentAndChild(*rootWindow, *curWin))
                     {
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
            }

            // Get ready for next loop.
            ++winIter;
         }

         return successes;
      }

   } // END - GUI namespace
} // END - SimCore namespace

#endif
