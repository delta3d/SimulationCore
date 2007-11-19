/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Eddie Johnson
 */
#ifndef _CONFIGURATION_OBJECT_INTERFACE_H_
#define _CONFIGURATION_OBJECT_INTERFACE_H_

#include <osg/Referenced>

// Forward declarations
namespace dtGame
{
   class GameManager;
}

namespace StealthGM
{
   class ConfigurationObjectInterface : public osg::Referenced
   {
      public:

         /// Constructor
         ConfigurationObjectInterface() : mIsUpdated(false) { }

         /**
            * Returns true is this object is updated and needs to be updated
            * @return true if updated, false if clean
            */
         bool IsUpdated() const { return mIsUpdated; }

         /**
            * Sets this object as dirty and thusly needing to apply changes
            * @param dirty True if dirty, false if not
            */
         void SetIsUpdated(bool update) { mIsUpdated = update; }

         /**
            * Applies the updates of this object. This method must be overridden in 
            * all subclasses
            */
         virtual void ApplyChanges(dtGame::GameManager &gameManager) = 0;

      protected:
      
         /// Destructor
         virtual ~ConfigurationObjectInterface() { }

      private:

         bool mIsUpdated;
   };
}

#endif

