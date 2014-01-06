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
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <SimCore/WeaponTypeEnum.h>

namespace SimCore
{
   ////////////////////////////////////////////////////////////////////////////////
   // WEAPON TYPE ENUMERATION CODE
   ////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(WeaponTypeEnum);
   WeaponTypeEnum WeaponTypeEnum::WEAPON_50CAL("Weaon_50Cal", 0, dtDAL::ResourceDescriptor("StaticMeshes:m2 .50cal:m2_50cal.ive"));
   WeaponTypeEnum WeaponTypeEnum::WEAPON_MK19("Weaon_MK19", 1, dtDAL::ResourceDescriptor("StaticMeshes:mk19:mk19.ive"));
   WeaponTypeEnum WeaponTypeEnum::WEAPON_M240("Weaon_M240", 2, dtDAL::ResourceDescriptor("StaticMeshes:m240:m240.ive"));
   WeaponTypeEnum WeaponTypeEnum::WEAPON_M249("Weaon_M249", 3, dtDAL::ResourceDescriptor("StaticMeshes:m249:m249.ive"));

   ////////////////////////////////////////////////////////////////////////////////
   WeaponTypeEnum::WeaponTypeEnum(const std::string& name, unsigned enumValue, const dtDAL::ResourceDescriptor& modelResource)
   : dtUtil::Enumeration(name),
   mEnumValue(enumValue),
   mModelResource(modelResource)
   {
      AddInstance(this);
   }

   ////////////////////////////////////////////////////////////////////////////////
   unsigned WeaponTypeEnum::GetEnumValue() const
   {
      return mEnumValue;
   }

   ////////////////////////////////////////////////////////////////////////////////
   const dtDAL::ResourceDescriptor& WeaponTypeEnum::GetModelResource() const
   {
      return mModelResource;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void WeaponTypeEnum::GetModelResourceList( std::vector<dtDAL::ResourceDescriptor>& outFileList )
   {
      const std::vector<dtUtil::Enumeration*>& weaponTypes = WeaponTypeEnum::Enumerate();

      const WeaponTypeEnum* curType = nullptr;
      unsigned limit = weaponTypes.size();
      for( unsigned i = 0; i < limit; ++i )
      {
         curType = dynamic_cast<const WeaponTypeEnum*>(weaponTypes[i]);
         if( curType != nullptr )
         {
            outFileList.push_back( curType->mModelResource );
         }
      }
   }
}
