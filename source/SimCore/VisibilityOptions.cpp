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

#include <prefix/SimCorePrefix.h>
#include <SimCore/VisibilityOptions.h>
#include <SimCore/Actors/BaseEntity.h>
#include <cstring>

namespace SimCore
{
   BasicVisibilityOptions::BasicVisibilityOptions()
   {
      const std::vector<SimCore::Actors::BaseEntityActorProxy::ForceEnum*>& forces =
         SimCore::Actors::BaseEntityActorProxy::ForceEnum::EnumerateType();
      for (size_t i = 0; i < forces.size(); ++i)
      {
         SetEnumVisible(*forces[i], true);
      }

      const std::vector<SimCore::Actors::BaseEntityActorProxy::DomainEnum*>& domains =
         SimCore::Actors::BaseEntityActorProxy::DomainEnum::EnumerateType();
      for (size_t i = 0; i < domains.size(); ++i)
      {
         SetEnumVisible(*domains[i], true);
      }
   }

   void BasicVisibilityOptions::SetAllTrue()
   {
      mDismountedInfantry = true;
      mPlatforms = true;
      mSensorBlips = true;
      mTracks = true;
      mBattlefieldGraphics = true;

      std::map<dtUtil::Enumeration*, bool>::iterator i, iend;
      i = mVisibleMap.begin();
      iend = mVisibleMap.end();
      for (; i != iend; ++i)
      {
         i->second = true;
      }
   }

   void BasicVisibilityOptions::SetAllFalse()
   {
      mDismountedInfantry = false;
      mPlatforms = false;
      mSensorBlips = false;
      mTracks = false;
      mBattlefieldGraphics = false;

      std::map<dtUtil::Enumeration*, bool>::iterator i, iend;
      i = mVisibleMap.begin();
      iend = mVisibleMap.end();
      for (; i != iend; ++i)
      {
         i->second = false;
      }
   }

   void BasicVisibilityOptions::SetEnumVisible(dtUtil::Enumeration& enumVal, bool visible)
   {
      mVisibleMap[&enumVal] = visible;
   }

   bool BasicVisibilityOptions::IsEnumVisible(dtUtil::Enumeration& enumVal) const
   {
      std::map<dtUtil::Enumeration*, bool>::const_iterator found = mVisibleMap.find(&enumVal);
      if (found != mVisibleMap.end())
      {
         return found->second;
      }
      //default to visible.
      return true;
   }

   VisibilityOptions::VisibilityOptions()
   {
      mBasicOptions.SetAllTrue();
      // all flags but blips and tracks should be on by default.
      mBasicOptions.mSensorBlips = false;
      mBasicOptions.mTracks = false;
   }

   VisibilityOptions::~VisibilityOptions()
   {

   }

   const BasicVisibilityOptions& VisibilityOptions::GetBasicOptions() const
   {
      return mBasicOptions;
   }

   void VisibilityOptions::SetBasicOptions(const BasicVisibilityOptions& options)
   {
      mBasicOptions = options;
   }

}
