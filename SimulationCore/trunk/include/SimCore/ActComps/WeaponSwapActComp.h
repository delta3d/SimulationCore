/* -*-c++-*-
 * SimulationCore
 * Copyright 2010, Alion Science and Technology
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

#ifndef WEAPONSWAPACTCOMP_H_
#define WEAPONSWAPACTCOMP_H_

#include <dtGame/actorcomponent.h>
#include <dtUtil/getsetmacros.h>
#include <dtUtil/nodecollector.h>
#include <dtDAL/resourcedescriptor.h>

#include <dtCore/observerptr.h>

#include <SimCore/Export.h>

namespace osgSim
{
   class DOFTransform;
}

namespace SimCore
{

   namespace ActComps
   {

      class SIMCORE_EXPORT WeaponSwapActComp : public dtGame::ActorComponent
      {
      public:
         typedef dtGame::ActorComponent BaseClass;

         static const ActorComponent::ACType TYPE;

         class SIMCORE_EXPORT WeaponDescription : public osg::Referenced
         {
            public:
               typedef osg::Referenced BaseClass;

               std::string mWeaponName;
               std::string mHotSpotName;
               dtCore::RefPtr<osg::Node> mRootNode;
               dtCore::RefPtr<osgSim::DOFTransform> mWeaponSwapNode;
               
         };

         WeaponSwapActComp();
         virtual ~WeaponSwapActComp();

         DECLARE_PROPERTY(std::string, WeaponName);
         DECLARE_PROPERTY(std::string, WeaponSwapRootNode);
         DECLARE_PROPERTY(std::string, WeaponHotSpotDOF);
         DECLARE_PROPERTY(dtDAL::ResourceDescriptor, WeaponSwapMesh);

         dtUtil::NodeCollector* GetNodeCollector();
         void SetNodeCollector(dtUtil::NodeCollector* nodeCollector);

         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();
         virtual void BuildPropertyMap();

         void NextWeapon();
         void PreviousWeapon();
         unsigned GetNumWeapons() const;

         void SelectWeapon(const std::string& weaponName);
         
         bool HasWeapon(const std::string& weaponName) const;
         const std::string& GetCurrentWeapon() const;

         bool AddWeapon(const std::string weaponName, const std::string& weaponHotspotName, const dtDAL::ResourceDescriptor& meshToLoad);
         void RemoveWeapon(const std::string& weaponName);

         void Update();

      private:

         void SwapWeapon();

         bool AttachWeapon(WeaponDescription* wp);
         void UnAttachWeapon();

         WeaponDescription* FindWeapon(const std::string& weaponName);
         const WeaponDescription* FindWeapon(const std::string& weaponName) const;
         int FindWeaponIndex(const std::string& weaponName) const;
         int FindWeaponIndex(const WeaponDescription* wp) const;
         void SetNextWeapon(WeaponDescription* wp);


         bool mHasWeapon;
         bool mSwitchWeapons;
         
         dtCore::RefPtr<WeaponDescription> mCurrentWeapon;
         dtCore::ObserverPtr<WeaponDescription> mWeaponToSwitchTo;

         typedef std::vector<dtCore::RefPtr<WeaponDescription> > WeaponArray;
         WeaponArray mWeapons;

         dtCore::RefPtr<dtUtil::NodeCollector> mNodeCollector;
      };

   }

}

#endif /* WEAPONSWAPACTCOMP_H_ */
