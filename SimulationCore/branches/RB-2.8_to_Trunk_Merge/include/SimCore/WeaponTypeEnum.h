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
* @author Chris Rodgers
* 
* NOTE: This is a temporary file allowing the Stealth Viewer to use the same
*       model files for HMMWV weapons as found in the HMMWV sim. This file will
*       be replaced with a more formal way of mapping weapon number values
*       with respective model files. Control states are used to reference
*       weapons between simulations, though they can only refer to weapons by
*       number rather than by name.
*/

#ifndef _WEAPON_TYPE_ENUM_H_
#define _WEAPON_TYPE_ENUM_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtUtil/enumeration.h>
#include <dtCore/resourcedescriptor.h>

namespace SimCore
{
   ////////////////////////////////////////////////////////////////////////////////
   // WEAPON TYPE ENUMERATION CODE
   ////////////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT WeaponTypeEnum : public dtUtil::Enumeration
   {
      DECLARE_ENUM(WeaponTypeEnum);

      public:
         static WeaponTypeEnum WEAPON_50CAL;
         static WeaponTypeEnum WEAPON_MK19;
         static WeaponTypeEnum WEAPON_M240;
         static WeaponTypeEnum WEAPON_M249;

         unsigned GetEnumValue() const;

         const dtCore::ResourceDescriptor& GetModelResource() const;

         static void GetModelResourceList(std::vector<dtCore::ResourceDescriptor>& outFileList);

      private:
         WeaponTypeEnum(const std::string& name, unsigned enumValue, const dtCore::ResourceDescriptor& modelResource);

         unsigned mEnumValue;
         dtCore::ResourceDescriptor mModelResource;
   };
}

#endif
