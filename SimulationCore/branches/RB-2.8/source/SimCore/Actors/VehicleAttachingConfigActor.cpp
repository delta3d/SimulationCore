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
 * @author Allen Danklefsen
 */
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/VehicleAttachingConfigActor.h>
#include <dtCore/enginepropertytypes.h>

namespace SimCore
{
   namespace Actors
   {
      /////////////////////////////////////////////////////////////////////////
      VehicleAttachingConfigActor::VehicleAttachingConfigActor(dtGame::GameActorProxy& owner)
      : dtGame::GameActor(owner)
      , mUseInsideModel(false)
      {
                   
      }

      /////////////////////////////////////////////////////////////////////////
      VehicleAttachingConfigActor::~VehicleAttachingConfigActor()
      {
         
      }

      DT_IMPLEMENT_ACCESSOR(VehicleAttachingConfigActor, dtCore::ResourceDescriptor, InsideModelResourceGood);
      DT_IMPLEMENT_ACCESSOR(VehicleAttachingConfigActor, dtCore::ResourceDescriptor, InsideModelResourceDamaged);
      DT_IMPLEMENT_ACCESSOR(VehicleAttachingConfigActor, dtCore::ResourceDescriptor, InsideModelResourceDestroyed);

      ////////////////////////////////////////////////////////////////////
      // Actor Proxy Below here
      ////////////////////////////////////////////////////////////////////
      VehicleAttachingConfigActorProxy::VehicleAttachingConfigActorProxy()
      {
         SetClassName("VehicleAttachingConfigActor");
      }

      /////////////////////////////////////////////////////////////////////////
      void VehicleAttachingConfigActorProxy::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUP = "VehicleConfig";
         dtGame::GameActorProxy::BuildPropertyMap();
         VehicleAttachingConfigActor* actor = NULL;
         GetActor(actor);

         AddProperty(new dtCore::ResourceActorProperty(dtCore::DataType::STATIC_MESH,
            "VEHICLE_INSIDE_MODEL_GOOD", "VEHICLE_INSIDE_MODEL_PATH_GOOD",
            dtCore::ResourceActorProperty::SetDescFuncType(actor, &VehicleAttachingConfigActor::SetInsideModelResourceGood),
            dtCore::ResourceActorProperty::GetDescFuncType(actor, &VehicleAttachingConfigActor::GetInsideModelResourceGood),
            "What is the filepath / string of the inside model good", GROUP));

         AddProperty(new dtCore::ResourceActorProperty(dtCore::DataType::STATIC_MESH,
            "VEHICLE_INSIDE_MODEL_DESTROYED", "VEHICLE_INSIDE_MODEL_PATH_DESTROYED",
            dtCore::ResourceActorProperty::SetDescFuncType(actor, &VehicleAttachingConfigActor::SetInsideModelResourceDestroyed),
            dtCore::ResourceActorProperty::GetDescFuncType(actor, &VehicleAttachingConfigActor::GetInsideModelResourceDestroyed),
            "What is the filepath / string of the inside model dmged", GROUP));
         
         AddProperty(new dtCore::ResourceActorProperty(dtCore::DataType::STATIC_MESH,
            "VEHICLE_INSIDE_MODEL_DAMAGED", "VEHICLE_INSIDE_MODEL_PATH_DAMAGED",
            dtCore::ResourceActorProperty::SetDescFuncType(actor, &VehicleAttachingConfigActor::SetInsideModelResourceDamaged),
            dtCore::ResourceActorProperty::GetDescFuncType(actor, &VehicleAttachingConfigActor::GetInsideModelResourceDamaged),
            "What is the filepath / string of the inside model destroyed", GROUP));

         AddProperty(new dtCore::Vec3ActorProperty("SeatOffSetPosition", "SeatOffSetPosition",
            dtCore::Vec3ActorProperty::SetFuncType(actor, &VehicleAttachingConfigActor::SetSeatPosition),
            dtCore::Vec3ActorProperty::GetFuncType(actor, &VehicleAttachingConfigActor::GetSeatPosition),
            "For positioning, wheres the offset?", GROUP));

         AddProperty(new dtCore::Vec3ActorProperty("SeatOffSetRotation", "SeatOffSetRotation",
            dtCore::Vec3ActorProperty::SetFuncType(actor, &VehicleAttachingConfigActor::SetRotationOffSet),
            dtCore::Vec3ActorProperty::GetFuncType(actor, &VehicleAttachingConfigActor::GetRotationOffSet),
            "For rotation, wheres the offset?", GROUP));

         AddProperty(new dtCore::BooleanActorProperty("UseInsideModel", "UseInsideModel",
            dtCore::BooleanActorProperty::SetFuncType(actor, &VehicleAttachingConfigActor::SetUsesInsideModel),
            dtCore::BooleanActorProperty::GetFuncType(actor, &VehicleAttachingConfigActor::GetUsesInsideModel),
            "Does it use the inside model or not?", GROUP));
      }

      /////////////////////////////////////////////////////////////////////////
      VehicleAttachingConfigActorProxy::~VehicleAttachingConfigActorProxy()
      {
      }

      /////////////////////////////////////////////////////////////////////////
      void VehicleAttachingConfigActorProxy::CreateDrawable()
      {
         SetDrawable(*new VehicleAttachingConfigActor(*this));
      }

      /////////////////////////////////////////////////////////////////////////
      void VehicleAttachingConfigActorProxy::OnEnteredWorld()
      {
         dtGame::GameActorProxy::OnEnteredWorld();
      }
   }
}
