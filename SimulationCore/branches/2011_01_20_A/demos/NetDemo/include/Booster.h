/* -*-c++-*-
 * SimulationCore
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
 * david
 */

#ifndef BOOSTER_H_
#define BOOSTER_H_

#include <dtDAL/propertycontainer.h>
#include <dtDAL/propertymacros.h>

namespace NetDemo
{

   class Booster: public dtDAL::PropertyContainer
   {
   public:
      Booster();
      virtual ~Booster();

      DT_DECLARE_ACCESSOR(bool, StartBoost);
      DT_DECLARE_ACCESSOR(float, StartBoostAccel);
      DT_DECLARE_ACCESSOR(float, MaxBoostTime);
      DT_DECLARE_ACCESSOR(float, CurrentBoostTime);
      DT_DECLARE_ACCESSOR(float, TimeToResetBoost);
      DT_DECLARE_ACCESSOR(float, BoostResetTimer);

      void Start();

      void Stop();

      void Update(float dt);

      float GetCurrentBoostForce();
   };

}

#endif /* BOOSTER_H_ */
