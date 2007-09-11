/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
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

         unsigned GetEnumValue() const { return mEnumValue; }

         const std::string& GetModelFileUrl() const { return mModelFileUrl; }

         static void GetModelFileUrlList( std::vector<std::string>& outFileUrlList );

      private:
         WeaponTypeEnum(const std::string &name, unsigned enumValue, const std::string& modelFileUrl);

         unsigned mEnumValue;
         std::string mModelFileUrl;
   };
}

#endif
