/* -*-c++-*-
 * SimulationCore
 * Copyright 2013, David Guthrie
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
 * David Guthrie
 */

#ifndef KEYBOARDWHEELEDVEHICLEINPUTACTCOMP_H_
#define KEYBOARDWHEELEDVEHICLEINPUTACTCOMP_H_

#include <SimCore/ActComps/AbstractWheeledVehicleInputActComp.h>

namespace dtGame
{
   class TickMessage;
}

namespace SimCore
{
   namespace ActComps
   {

      /*
       *
       */
      class KeyboardWheeledVehicleInputActComp: public AbstractWheeledVehicleInputActComp
      {
      public:
         KeyboardWheeledVehicleInputActComp();

         virtual void Update(const dtGame::TickMessage& msg);

         /// @return the steering angle from -1 to 1 clockwise.
         virtual float GetSteeringAngleNormalized();

         /// @return The brake power from 0 to 1 with 0 as no power.
         virtual float GetBrakesNormalized();

         /// @return The accelerator from -1 to 1 with 0 as no power.
         virtual float GetAcceleratorNormalized();

         /// The number of updates to go from straight to full turn angle
         DT_DECLARE_ACCESSOR(int, NumUpdatesUntilFullSteeringAngle);

         virtual void BuildPropertyMap();
         virtual void OnEnteredWorld();
      protected:
         virtual ~KeyboardWheeledVehicleInputActComp();
         float mLastSteeringAngle, mLastBrakesNormalized, mLastAcceleratorNormalized;
      };
   }
}

#endif /* KEYBOARDWHEELEDVEHICLEINPUTACTCOMP_H_ */
