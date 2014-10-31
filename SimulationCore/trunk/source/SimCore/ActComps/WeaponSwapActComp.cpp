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
#include <prefix/SimCorePrefix.h>
#include <SimCore/ActComps/WeaponSwapActComp.h>
#include <dtUtil/nodecollector.h>
#include <dtCore/propertymacros.h>
#include <dtCore/project.h>

#include <osgSim/DOFTransform>
#include <SimCore/Actors/IGActor.h>

#include <algorithm>

namespace SimCore
{

   namespace ActComps
   {
      const dtGame::ActorComponent::ACType WeaponSwapActComp::TYPE(new dtCore::ActorType("WeaponSwapActComp","ActorComponents",
            "Swaps the weopon geometry on a drawable.", dtGame::ActorComponent::BaseActorComponentType));

      //////////////////////////////////////////////////////////////////////////
      WeaponSwapActComp::WeaponSwapActComp()
      : dtGame::ActorComponent(TYPE)
      , mWeaponName("Default")
      , mWeaponSwapRootNode("dof_gun_01")
      , mWeaponHotSpotDOF("hotspot_01")
      , mHasWeapon(false)
      , mSwitchWeapons(false)
      , mCurrentWeapon()
      , mWeaponToSwitchTo()
      , mWeapons()
      , mNodeCollector()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponSwapActComp::~WeaponSwapActComp()
      {
         mWeapons.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR_GETTER(WeaponSwapActComp, std::string, WeaponName);
      DT_IMPLEMENT_ACCESSOR_GETTER(WeaponSwapActComp, std::string, WeaponSwapRootNode);
      DT_IMPLEMENT_ACCESSOR_GETTER(WeaponSwapActComp, std::string, WeaponHotSpotDOF);
      DT_IMPLEMENT_ACCESSOR_GETTER(WeaponSwapActComp, dtCore::ResourceDescriptor, WeaponSwapMesh);

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SetWeaponName(const std::string& name)
      {
         mWeaponName = name;
      }
      
      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SetWeaponSwapRootNode(const std::string& nodeName)
      {
         mWeaponSwapRootNode = nodeName;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SetWeaponSwapMesh(const dtCore::ResourceDescriptor& rd)
      {
         mWeaponSwapMesh = rd;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SetWeaponHotSpotDOF(const std::string& rd)
      {
         mWeaponHotSpotDOF = rd;
      }


      //////////////////////////////////////////////////////////////////////////
      dtUtil::NodeCollector* WeaponSwapActComp::GetNodeCollector()
      {
         return mNodeCollector.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SetNodeCollector( dtUtil::NodeCollector* nodeCollector )
      {
         mNodeCollector = nodeCollector;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::BuildPropertyMap()
      {
         typedef dtCore::PropertyRegHelper<WeaponSwapActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "WeaponSwap");

         DT_REGISTER_PROPERTY(
            WeaponSwapRootNode,
            "The name of the group node to look for to use as the root of the weapon",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
            WeaponName,
            "The unique name of the weapon or weapon type",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
            WeaponHotSpotDOF,
            "The name of the DOF node to look for to use as the fire position of the weapon",
            PropRegType, propRegHelper);


         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::STATIC_MESH,
            WeaponSwapMesh,
            "Weapon Swap Mesh",
            "The weapon mesh resource to use instead of the one in the model.",
            PropRegType, propRegHelper);
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         if (!mNodeCollector.valid())
         {
            dtGame::GameActorProxy* owner = NULL;
            GetOwner(owner);
            SetNodeCollector(new dtUtil::NodeCollector(owner->GetDrawable()->GetOSGNode(), dtUtil::NodeCollector::AllNodeTypes));
         }

         if (mWeaponSwapMesh != dtCore::ResourceDescriptor::NULL_RESOURCE)
         {
            //we will configure a default weapon to use if the resource   
            bool weaponAdded = AddWeapon(mWeaponName, mWeaponHotSpotDOF, mWeaponSwapMesh);
            if (weaponAdded)
            {
               mWeaponToSwitchTo = FindWeapon(mWeaponName);
               if (mWeaponToSwitchTo != NULL)
               {
                  osgSim::DOFTransform* dof = mNodeCollector->GetDOFTransform(mWeaponSwapRootNode);
                  if (dof != NULL)
                  {
                     dof->removeChildren(0, dof->getNumChildren());
                  }
                  //swap weapon is supposed to do determine if we need to unattch and do this for us but 
                  //in cases where we start with a weapon we have to clear out the dof children 
                  UnAttachWeapon();
                  SwapWeapon();
               }
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::OnRemovedFromWorld()
      {
         BaseClass::OnRemovedFromWorld();
         mNodeCollector = NULL;
         mWeapons.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SwapWeapon()
      {
         dtGame::GameActorProxy* owner = NULL;
         GetOwner(owner);
         if (owner != NULL)
         {
            if (mHasWeapon)
            {
               UnAttachWeapon();
            }

            AttachWeapon(mWeaponToSwitchTo.get());
            
            mSwitchWeapons = false;
            mWeaponToSwitchTo = NULL;
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::Update()
      {
         if (mSwitchWeapons)
         {
            SwapWeapon();
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool WeaponSwapActComp::AttachWeapon(WeaponDescription* wp)
      {
         if (mNodeCollector.valid())
         {
            osgSim::DOFTransform* dof = mNodeCollector->GetDOFTransform(mWeaponSwapRootNode);
            if (dof != NULL)
            {
               dof->addChild(wp->mRootNode.get());

               // Get access to the hot spot on the weapon model
               dtCore::RefPtr<dtUtil::NodeCollector> weaponNodeCollector
                  = new dtUtil::NodeCollector(wp->mRootNode.get(), dtUtil::NodeCollector::DOFTransformFlag);
               osgSim::DOFTransform* hotspotDof = weaponNodeCollector->GetDOFTransform(wp->mHotSpotName);

               if ( hotspotDof != NULL )
               {
                  //note: this function only works if the dof hot spot has the same name on both weapons
                  mNodeCollector->AddDOFTransform(wp->mHotSpotName, *hotspotDof);
               }

               mCurrentWeapon = wp;
               mHasWeapon = true;
               return true;
            }
         }
         return false;
      }

      void WeaponSwapActComp::UnAttachWeapon()
      {
         if (mNodeCollector.valid())
         {
            osgSim::DOFTransform* dof = mNodeCollector->GetDOFTransform(mWeaponSwapRootNode);
            if (dof != NULL)
            {
               if (mCurrentWeapon.valid() && mCurrentWeapon->mRootNode.valid())
               {
                  // delete the last weapon.  Used to delete all nodes, but
                  // other things are added to the
                  dof->removeChild(mCurrentWeapon->mRootNode);
               }

               mNodeCollector->RemoveDOFTransform(mWeaponHotSpotDOF);
               mHasWeapon = false;
            }
         }
      }  

      bool WeaponSwapActComp::AddWeapon(const std::string weaponName, const std::string& weaponHotspotName, const dtCore::ResourceDescriptor& meshToLoad)
      {
         bool isModelLoaded = false;
         dtCore::RefPtr<osg::Node> newModel;

         if (meshToLoad != dtCore::ResourceDescriptor::NULL_RESOURCE)
         {
            const std::string& weaponFileName = dtCore::Project::GetInstance().GetResourcePath(meshToLoad);

            isModelLoaded = SimCore::Actors::IGActor::LoadFileStatic(weaponFileName, newModel, newModel, false);

            if (isModelLoaded)
            {
               // Get access to the hot spot on the weapon model
               dtCore::RefPtr<dtUtil::NodeCollector> weaponNodeCollector = new dtUtil::NodeCollector(newModel.get(), dtUtil::NodeCollector::DOFTransformFlag);
               osgSim::DOFTransform* hotspotDof = weaponNodeCollector->GetDOFTransform(weaponHotspotName);

               if (hotspotDof != NULL)
               {
                  dtCore::RefPtr<WeaponDescription> weaponDesc = new WeaponDescription();
                  weaponDesc->mWeaponName = weaponName;  
                  weaponDesc->mHotSpotName = weaponHotspotName;
                  weaponDesc->mRootNode = newModel.get();
                  weaponDesc->mWeaponSwapNode = hotspotDof;

                  mWeapons.push_back(weaponDesc);
                  return true;
               }
            }
         }

         return false;
      }

      void WeaponSwapActComp::NextWeapon()
      {
         if (mWeapons.size() > 1)
         {
            int index = FindWeaponIndex(mCurrentWeapon);
            if(index >= 0)
            {
               ++index;
               if(unsigned(index) >= mWeapons.size())
               {
                  index = 0;
               }
            }
            SetNextWeapon(mWeapons[index]);
         }
      }

      void WeaponSwapActComp::PreviousWeapon()
      {
         if(mWeapons.size() > 1)
         {
            int index = FindWeaponIndex(mCurrentWeapon);
            if(index >= 0)
            {
               if(index >= 1)
               {
                  --index;
               }
               else 
               {
                  index = mWeapons.size() - 1;
               }
               SetNextWeapon(mWeapons[index]);
            }
         }
      }

      void WeaponSwapActComp::SelectWeapon( const std::string& weaponName )
      {
         WeaponDescription* wp = FindWeapon(weaponName);
         if(wp != NULL)  
         {
            SetNextWeapon(wp);
         }
      }

      bool WeaponSwapActComp::HasWeapon( const std::string& weaponName ) const
      {
         return FindWeapon(weaponName) != NULL;
      }

      const std::string& WeaponSwapActComp::GetCurrentWeapon() const
      {
         if(mCurrentWeapon.valid())
         {
            return mCurrentWeapon->mWeaponName;
         }
         
         static std::string emptyString;
         return emptyString;
      }

      struct CompareWeaponByName
      {
         CompareWeaponByName(const std::string& str): mName(str) {}

         bool operator()(const dtCore::RefPtr<WeaponSwapActComp::WeaponDescription>& wp)
         {
            return mName == wp->mWeaponName;
         }

         const std::string& mName;
      };

      void WeaponSwapActComp::RemoveWeapon( const std::string& weaponName )
      {
         if(mCurrentWeapon.valid() && mCurrentWeapon->mWeaponName == weaponName)
         {
            UnAttachWeapon();
         }

         CompareWeaponByName compareWeaponByName(weaponName);
         mWeapons.erase(std::remove_if(mWeapons.begin(), mWeapons.end(), compareWeaponByName), mWeapons.end());
      }

      WeaponSwapActComp::WeaponDescription* WeaponSwapActComp::FindWeapon( const std::string& weaponName )
      {
         for(unsigned i = 0; i < mWeapons.size(); ++i)
         {
            if(mWeapons[i]->mWeaponName == weaponName)
            {
               return mWeapons[i];
            }
         }
         return NULL;
      }

      const WeaponSwapActComp::WeaponDescription* WeaponSwapActComp::FindWeapon( const std::string& weaponName ) const
      {
         for(unsigned i = 0; i < mWeapons.size(); ++i)
         {
            if(mWeapons[i]->mWeaponName == weaponName)
            {
               return mWeapons[i];
            }
         }
         return NULL;
      }

      int WeaponSwapActComp::FindWeaponIndex(const std::string& weaponName) const
      {
         for(unsigned i = 0; i < mWeapons.size(); ++i)
         {
            if(mWeapons[i]->mWeaponName == weaponName)
            {
               return i;
            }
         }

         return -1;
      }

      int WeaponSwapActComp::FindWeaponIndex( const WeaponDescription* wp ) const
      {
         for(unsigned i = 0; i < mWeapons.size(); ++i)
         {
            if(mWeapons[i]->mWeaponName == wp->mWeaponName)
            {
               return i;
            }
         }

         return -1;
      }

      unsigned WeaponSwapActComp::GetNumWeapons() const
      {
         return mWeapons.size();
      }

      void WeaponSwapActComp::SetNextWeapon(WeaponDescription* wp)
      {
         mWeaponToSwitchTo = wp; 
         mSwitchWeapons = true;
      }

   }

}
