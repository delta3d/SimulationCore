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
#include <dtDAL/enginepropertytypes.h>

namespace SimCore
{
   namespace Actors
   {
      /////////////////////////////////////////////////////////////////////////
      VehicleAttachingConfigActor::VehicleAttachingConfigActor(dtGame::GameActorProxy &proxy) : dtGame::GameActor(proxy)
         , mUseInsideModel(false)
      {
                   
      }

      /////////////////////////////////////////////////////////////////////////
      VehicleAttachingConfigActor::~VehicleAttachingConfigActor()
      {
         
      }

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
         const std::string GROUP = "VehicleConfig";
         dtGame::GameActorProxy::BuildPropertyMap();
         VehicleAttachingConfigActor &actor = static_cast<VehicleAttachingConfigActor&>(GetGameActor());

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "VEHICLE_INSIDE_MODEL_GOOD", "VEHICLE_INSIDE_MODEL_PATH_GOOD", dtDAL::MakeFunctor(actor, &VehicleAttachingConfigActor::SetInsideModelResourceGood),
            "What is the filepath / string of the inside model good", GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "VEHICLE_INSIDE_MODEL_DESTROYED", "VEHICLE_INSIDE_MODEL_PATH_DESTROYED", dtDAL::MakeFunctor(actor, &VehicleAttachingConfigActor::SetInsideModelResourceDestroyed),
            "What is the filepath / string of the inside model dmged", GROUP));
         
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "VEHICLE_INSIDE_MODEL_DAMAGED", "VEHICLE_INSIDE_MODEL_PATH_DAMAGED", dtDAL::MakeFunctor(actor, &VehicleAttachingConfigActor::SetInsideModelResourceDamaged),
            "What is the filepath / string of the inside model destroyed", GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("SeatOffSetPosition", "SeatOffSetPosition",
            dtDAL::MakeFunctor(actor, &VehicleAttachingConfigActor::SetSeatPosition),
            dtDAL::MakeFunctorRet(actor, &VehicleAttachingConfigActor::GetSeatPosition),
            "For positioning, wheres the offset?", GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("SeatOffSetRotation", "SeatOffSetRotation",
            dtDAL::MakeFunctor(actor, &VehicleAttachingConfigActor::SetRotationOffSet),
            dtDAL::MakeFunctorRet(actor, &VehicleAttachingConfigActor::GetRotationOffSet),
            "For rotation, wheres the offset?", GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("UseInsideModel", "UseInsideModel",
            dtDAL::MakeFunctor(actor, &VehicleAttachingConfigActor::SetUsesInsideModel),
            dtDAL::MakeFunctorRet(actor, &VehicleAttachingConfigActor::GetUsesInsideModel),
            "Does it use the inside model or not?", GROUP));
      }

      /////////////////////////////////////////////////////////////////////////
      VehicleAttachingConfigActorProxy::~VehicleAttachingConfigActorProxy()
      {
      }

      /////////////////////////////////////////////////////////////////////////
      void VehicleAttachingConfigActorProxy::CreateActor()
      {
         SetActor(*new VehicleAttachingConfigActor(*this));
      }

      /////////////////////////////////////////////////////////////////////////
      void VehicleAttachingConfigActorProxy::OnEnteredWorld()
      {
         dtGame::GameActorProxy::OnEnteredWorld();
      }
   }
}
