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
 * david
 */

#ifndef UNITS_H_
#define UNITS_H_

#include <SimCore/Export.h>
#include <dtUtil/enumeration.h>

namespace SimCore
{
   class SIMCORE_EXPORT UnitOfLength : public dtUtil::Enumeration
   {
      DECLARE_ENUM(UnitOfLength);
   public:
      static UnitOfLength METER;
      static UnitOfLength YARD;
      static UnitOfLength FOOT;
      static UnitOfLength NAUTICAL_MILE;
      static UnitOfLength FURLONG;

      static double Convert(UnitOfLength& unitFrom, UnitOfLength& unitTo, double value);

      const std::string& GetAbbreviation() const;

      bool GetUseWholeUnits() const;

   protected:
      UnitOfLength(const std::string& name, double conversionFromMeters, const std::string& abbrev, bool useWholeUnits);
      ~UnitOfLength();
   private:
      double mConversionFromMeters;
      std::string mAbbreviation;
      bool mWholeUnitsOnly;
   };

   /**
    * Angular units and conversion fro and to.
    */
   class SIMCORE_EXPORT UnitOfAngle : public dtUtil::Enumeration
   {
      DECLARE_ENUM(UnitOfAngle);
   public:
      static UnitOfAngle DEGREE;
      static UnitOfAngle MIL;
      static UnitOfAngle RADIAN;

      static double Convert(UnitOfAngle& unitFrom, UnitOfAngle& unitTo, double value);

      const std::string& GetAbbreviation() const;

      /// Get's the max size for the angle unit, i.e. the number at which in starts to repeat the rotation.
      double GetMax() const;

      bool GetUseWholeUnits() const;

   protected:
      UnitOfAngle(const std::string& name, double conversionFromDegrees, double max, const std::string& abbrev, bool useWholeUnits);
      ~UnitOfAngle();
   private:
      double mConversionFromDegrees, mMax;
      std::string mAbbreviation;
      bool mWholeUnitsOnly;
   };
}

#endif /* UNITS_H_ */
