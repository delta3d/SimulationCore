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

#include <SimCore/ActComps/WeaponSwapActComp.h>
#include <dtUtil/nodecollector.h>
#include <dtDAL/propertymacros.h>
#include <dtDAL/project.h>

#include <osgSim/DOFTransform>
#include <SimCore/Actors/IGActor.h>

namespace SimCore
{

   namespace ActComps
   {
      const dtGame::ActorComponent::ACType WeaponSwapActComp::TYPE("WeaponSwapActComp");

      //////////////////////////////////////////////////////////////////////////
      WeaponSwapActComp::WeaponSwapActComp()
      : dtGame::ActorComponent(TYPE)
      , mWeaponSwapRootNode("dof_gun_01")
      , mWeaponHotSpotDOF("hotspot_01")
      {
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponSwapActComp::~WeaponSwapActComp()
      {
      }

      IMPLEMENT_PROPERTY_GETTER(WeaponSwapActComp, std::string, WeaponSwapRootNode);
      IMPLEMENT_PROPERTY_GETTER(WeaponSwapActComp, std::string, WeaponHotSpotDOF);
      IMPLEMENT_PROPERTY_GETTER(WeaponSwapActComp, dtDAL::ResourceDescriptor, WeaponSwapMesh);

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SetWeaponSwapRootNode(const std::string& nodeName)
      {
         mWeaponSwapRootNode = nodeName;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SetWeaponSwapMesh(const dtDAL::ResourceDescriptor& rd)
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
      void WeaponSwapActComp::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         if (!mNodeCollector.valid())
         {
            dtGame::GameActor* owner;
            GetOwner(owner);
            SetNodeCollector(new dtUtil::NodeCollector(owner->GetOSGNode(), dtUtil::NodeCollector::AllNodeTypes));
         }

         SwapWeapon();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::OnRemovedFromWorld()
      {
         BaseClass::OnRemovedFromWorld();
         mNodeCollector = NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::BuildPropertyMap()
      {
         typedef dtDAL::PropertyRegHelper<WeaponSwapActComp&, WeaponSwapActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "WeaponSwap");

         REGISTER_PROPERTY(
            WeaponSwapRootNode,
            "The name of the group node to look for to use as the root of the weapon",
            PropRegType, propRegHelper);

         REGISTER_PROPERTY(
            WeaponHotSpotDOF,
            "The name of the DOF node to look for to use as the fire position of the weapon",
            PropRegType, propRegHelper);

         REGISTER_RESOURCE_PROPERTY(dtDAL::DataType::STATIC_MESH,
            WeaponSwapMesh,
            "Weapon Swap Mesh",
            "The weapon mesh resource to use instead of the one in the model.",
            PropRegType, propRegHelper);
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::SwapWeapon()
      {
         // Attempt attachment of a new weapon model.
         bool isModelLoaded = false;
         dtCore::RefPtr<osg::Node> newModel;

         if (mWeaponSwapMesh != dtDAL::ResourceDescriptor::NULL_RESOURCE)
         {
            const std::string& weaponFileName = dtDAL::Project::GetInstance().GetResourcePath(mWeaponSwapMesh);

            isModelLoaded
               = SimCore::Actors::IGActor::LoadFileStatic(weaponFileName, newModel, newModel, false);
         }

         if(isModelLoaded)
         {
            dtGame::GameActor* owner;
            GetOwner(owner);
            if(owner != NULL)
            {
               AttachModel(newModel, mWeaponSwapRootNode);
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      void WeaponSwapActComp::Update()
      {

      }

      ////////////////////////////////////////////////////////////////////////////////
      bool WeaponSwapActComp::AttachModel(dtCore::RefPtr<osg::Node> model, const std::string& dofName )
      {
         if(mNodeCollector.valid())
         {
            osgSim::DOFTransform* dof = mNodeCollector->GetDOFTransform(dofName);
            if(dof != NULL)
            {
               // clear all children of dof before adding the gun.
               dof->removeChildren(0, dof->getNumChildren());
               dof->addChild(model.get());

               mNodeCollector->RemoveDOFTransform(mWeaponHotSpotDOF);

               // Get access to the hot spot on the weapon model
               dtCore::RefPtr<dtUtil::NodeCollector> weaponNodeCollector
                  = new dtUtil::NodeCollector(model.get(), dtUtil::NodeCollector::DOFTransformFlag);
               osgSim::DOFTransform* hotspotDof = weaponNodeCollector->GetDOFTransform(mWeaponHotSpotDOF);

               if( hotspotDof != NULL )
               {
                  //note: this function only works if the dof hot spot has the same name on both weapons
                  mNodeCollector->AddDOFTransform(mWeaponHotSpotDOF, *hotspotDof);
               }
               return true;
            }
         }
         return false;
      }

   }

}
