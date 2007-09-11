
////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/WeaponTypeEnum.h>



namespace SimCore
{
   ////////////////////////////////////////////////////////////////////////////////
   // WEAPON TYPE ENUMERATION CODE
   ////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(WeaponTypeEnum);
   WeaponTypeEnum WeaponTypeEnum::WEAPON_50CAL("Weaon_50Cal", 0, "StaticMeshes/m2 .50cal/m2_50cal.ive"); 
   WeaponTypeEnum WeaponTypeEnum::WEAPON_MK19("Weaon_MK19", 1, "StaticMeshes/mk19/mk19.ive"); 
   WeaponTypeEnum WeaponTypeEnum::WEAPON_M240("Weaon_M240", 2, "StaticMeshes/m240/m240.ive");

   ////////////////////////////////////////////////////////////////////////////////
   WeaponTypeEnum::WeaponTypeEnum(const std::string &name, unsigned enumValue, const std::string& modelFileUrl)
   : dtUtil::Enumeration(name),
   mEnumValue(enumValue),
   mModelFileUrl(modelFileUrl)
   {
      AddInstance(this);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void WeaponTypeEnum::GetModelFileUrlList( std::vector<std::string>& outFileUrlList )
   {
      const std::vector<dtUtil::Enumeration*>& weaponTypes = WeaponTypeEnum::Enumerate();

      const WeaponTypeEnum* curType = NULL;
      unsigned limit = weaponTypes.size();
      for( unsigned i = 0; i < limit; ++i )
      {
         curType = dynamic_cast<const WeaponTypeEnum*>(weaponTypes[i]);
         if( curType != NULL )
         {
            outFileUrlList.push_back( curType->GetModelFileUrl() );
         }
      }
   }
}
