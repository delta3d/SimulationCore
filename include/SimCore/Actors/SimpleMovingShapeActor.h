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

#ifndef SIMPLEMOVINGSHAPEACTOR_H_
#define SIMPLEMOVINGSHAPEACTOR_H_

#include <SimCore/Export.h>
#include <dtGame/gameactorproxy.h>
#include <dtUtil/getsetmacros.h>

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT SimpleMovingShapeActorProxy : public dtGame::GameActorProxy
      {
      public:
         SimpleMovingShapeActorProxy();

         virtual void BuildActorComponents();

         DT_DECLARE_ACCESSOR(dtCore::UniqueId, Owner);
         DT_DECLARE_ACCESSOR(int, Index);

      protected:
         virtual ~SimpleMovingShapeActorProxy();

         virtual void OnEnteredWorld();

         //virtual void OnRemovedFromWorld() { }

         virtual void CreateActor();
         virtual void BuildPropertyMap();
      };

   }

}

#endif /* SIMPLEMOVINGSHAPEACTOR_H_ */
