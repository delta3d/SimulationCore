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

#include <SimCore/Actors/SimpleMovingShapeActor.h>
#include <dtGame/gameactor.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtDAL/propertymacros.h>

namespace SimCore
{

   namespace Actors
   {

      ////////////////////////////////////////////////////////////////////////////
      SimpleMovingShapeActorProxy::SimpleMovingShapeActorProxy()
      : mOwner("")
      , mIndex(0)
      {
         SetClassName("SimCore::Actors::SimpleMovingShapeActorProxy");
      }

      ////////////////////////////////////////////////////////////////////////////
      SimpleMovingShapeActorProxy::~SimpleMovingShapeActorProxy()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::BuildActorComponents()
      {
         dtGame::GameActor* ga = NULL;
         GetActor(ga);

         // DEAD RECKONING - ACT COMPONENT
         if (!ga->HasComponent(dtGame::DeadReckoningHelper::TYPE)) // not added by a subclass
         {
            // TODO, use a subclassed dr actor comp that makes the shape dr too.
            dtCore::RefPtr<dtGame::DeadReckoningHelper> drHelper = new dtGame::DeadReckoningHelper();
            drHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);
            drHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::NONE);
            ga->AddComponent(*drHelper);
         }

         BaseClass::BuildActorComponents();
      }

      ////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(SimpleMovingShapeActorProxy, dtCore::UniqueId, Owner);
      DT_IMPLEMENT_ACCESSOR(SimpleMovingShapeActorProxy, int, Index);

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::OnEnteredWorld()
      {

      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::CreateActor()
      {
         SetActor(*new dtGame::GameActor(*this));
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUPNAME("SimpleMovingShape");

         typedef dtDAL::PropertyRegHelper<SimpleMovingShapeActorProxy&, SimpleMovingShapeActorProxy> PropRegHelperType;
         PropRegHelperType propRegHelper(*this, this, GROUPNAME);

         DT_REGISTER_PROPERTY(Index,
                  "The index key of this moving shape, so if an actor is keeping track of many shapes, it can identify them by their index.",
                  PropRegHelperType, propRegHelper);

         DT_REGISTER_ACTOR_ID_PROPERTY("", Owner, "Owning actor",
                  "The actor that owns this actor.",
                  PropRegHelperType, propRegHelper);

      }
   }

}
