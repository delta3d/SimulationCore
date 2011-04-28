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
#include <SimCore/UnitEnums.h>
#include <osg/Math>

namespace SimCore
{
   //////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(UnitOfLength);
   UnitOfLength UnitOfLength::METER("METER", 1.00, "m", true);
   UnitOfLength UnitOfLength::YARD("YARD", 1.0936133, "yd", true);
   UnitOfLength UnitOfLength::FOOT("FOOT", 3.2808399, "ft", true);
   UnitOfLength UnitOfLength::NAUTICAL_MILE("NAUTICAL_MILE", 0.000539956803, "nmi", false);
   UnitOfLength UnitOfLength::FURLONG("FURLONG", 1.0/201.168, "fur", false);

   //////////////////////////////////////////////////////////////////////
   UnitOfLength::UnitOfLength(const std::string& name, double conversionFromMeters,
            const std::string& abbrev, bool useWholeUnits)
   : dtUtil::Enumeration(name)
   , mConversionFromMeters(conversionFromMeters)
   , mAbbreviation(abbrev)
   , mWholeUnitsOnly(useWholeUnits)
   {
      AddInstance(this);
   }

   //////////////////////////////////////////////////////////////////////
   UnitOfLength::~UnitOfLength() {}

   //////////////////////////////////////////////////////////////////////
   double UnitOfLength::Convert(UnitOfLength& unitFrom, UnitOfLength& unitTo, double value)
   {
      double fromConv = 1.0 / unitFrom.mConversionFromMeters;
      double toConv = unitTo.mConversionFromMeters;
      return value * fromConv * toConv;
   }

   //////////////////////////////////////////////////////////////////////
   const std::string& UnitOfLength::GetAbbreviation() const
   {
      return mAbbreviation;
   }

   //////////////////////////////////////////////////////////////////////
   bool UnitOfLength::GetUseWholeUnits() const
   {
      return mWholeUnitsOnly;
   }

   //////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(UnitOfAngle);
   UnitOfAngle UnitOfAngle::DEGREE("DEGREE", 1.0, 360.0, "deg", false);
   UnitOfAngle UnitOfAngle::MIL("MIL", 6400.0 / 360.0, 6400.0, "mil", true);
   UnitOfAngle UnitOfAngle::RADIAN("RADIAN", osg::PI / 180.0, osg::PI * 2.0, "rad", false);

   //////////////////////////////////////////////////////////////////////
   UnitOfAngle::UnitOfAngle(const std::string& name, double conversionFromDegrees, double max,
            const std::string& abbrev, bool useWholeUnits)
   : dtUtil::Enumeration(name)
   , mConversionFromDegrees(conversionFromDegrees)
   , mMax(max)
   , mAbbreviation(abbrev)
   , mWholeUnitsOnly(useWholeUnits)
   {
      AddInstance(this);
   }

   //////////////////////////////////////////////////////////////////////
   UnitOfAngle::~UnitOfAngle() {}

   //////////////////////////////////////////////////////////////////////
   double UnitOfAngle::Convert(UnitOfAngle& unitFrom, UnitOfAngle& unitTo, double value)
   {
      double fromConv = 1.0 / unitFrom.mConversionFromDegrees;
      double toConv = unitTo.mConversionFromDegrees;
      return value * fromConv * toConv;
   }

   //////////////////////////////////////////////////////////////////////
   const std::string& UnitOfAngle::GetAbbreviation() const
   {
      return mAbbreviation;
   }

   //////////////////////////////////////////////////////////////////////
   double UnitOfAngle::GetMax() const
   {
      return mMax;
   }

   //////////////////////////////////////////////////////////////////////
   bool UnitOfAngle::GetUseWholeUnits() const
   {
      return mWholeUnitsOnly;
   }
}
