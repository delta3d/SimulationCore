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
         virtual void ApplyChanges(dtGame::GameManager&) = 0;

         /**
          * Called when the map changes, or whenever the config component feels it's necessary.
          */
         virtual void Reset(dtGame::GameManager&) {};

      protected:

         /// Destructor
         virtual ~ConfigurationObjectInterface() { }

      private:

         bool mIsUpdated;
   };
}

#endif

