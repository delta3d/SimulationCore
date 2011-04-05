/* -*-c++-*-
 * SimulationCore
 * Copyright 2009, Alion Science and Technology
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
 *
 * David Guthrie
 */

#ifndef SIMCORE_VISIBILITY_OPTIONS
#define SIMCORE_VISIBILITY_OPTIONS

#include <SimCore/Export.h>
#include <osg/Referenced>
#include <dtUtil/enumeration.h>
#include <map>

namespace SimCore
{
   class SIMCORE_EXPORT BasicVisibilityOptions
   {
   public:
      BasicVisibilityOptions();

      bool mDismountedInfantry : 1;
      bool mPlatforms : 1;

      bool mSensorBlips: 1;
      bool mTracks: 1;

      bool mBattlefieldGraphics: 1;

      void SetAllTrue();
      void SetAllFalse();

      void SetEnumVisible(dtUtil::Enumeration&, bool);
      bool IsEnumVisible(dtUtil::Enumeration&) const;

   private:
      std::map<dtUtil::Enumeration*, bool> mVisibleMap;
   };

   /**
    * Class that holds onto a set of visibility options.  It is meant to be extensible, hence
    * the virtual destructor.
    */
   class SIMCORE_EXPORT VisibilityOptions: public osg::Referenced
   {
   public:
      VisibilityOptions();

      virtual ~VisibilityOptions();

      const BasicVisibilityOptions& GetBasicOptions() const;
      void SetBasicOptions(const BasicVisibilityOptions& options);

   private:
      BasicVisibilityOptions mBasicOptions;
   };
}

#endif /* SIMCORE_VISIBILITY_OPTIONS */
