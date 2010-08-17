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

#include <SimCore/Export.h>

namespace SimCore
{

   namespace ActComps
   {

      class SIMCORE_EXPORT WeaponSwapActComp : public dtGame::ActorComponent
      {
      public:
         static const ActorComponent::ACType TYPE;

         typedef dtGame::ActorComponent BaseClass;

         WeaponSwapActComp();
         virtual ~WeaponSwapActComp();

         DECLARE_PROPERTY(std::string, WeaponSwapRootNode);
         DECLARE_PROPERTY(std::string, WeaponHotSpotDOF);
         DECLARE_PROPERTY(dtDAL::ResourceDescriptor, WeaponSwapMesh);

         dtUtil::NodeCollector* GetNodeCollector();
         void SetNodeCollector(dtUtil::NodeCollector* nodeCollector);

         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();
         virtual void BuildPropertyMap();

         void Update();
      private:

         void SwapWeapon();

         bool AttachModel(dtCore::RefPtr<osg::Node> model, const std::string& dofName);

         dtCore::RefPtr<dtUtil::NodeCollector> mNodeCollector;
      };

   }

}

#endif /* WEAPONSWAPACTCOMP_H_ */
