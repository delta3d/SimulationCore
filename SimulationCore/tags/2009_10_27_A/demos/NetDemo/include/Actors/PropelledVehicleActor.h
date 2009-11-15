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
 *
 * Bradley Anderegg
 * David Guthrie
 */

#ifndef PROPELLEDVEHICLEACTOR_H_
#define PROPELLEDVEHICLEACTOR_H_

#include <DemoExport.h>
#include <dtDAL/propertymacros.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/FourWheelVehiclePhysicsHelper.h>

namespace NetDemo
{

   class PropelledVehicleActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
   public:
      typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

      PropelledVehicleActor(SimCore::Actors::PlatformActorProxy& proxy);

      virtual void OnEnteredWorld();
      virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);
      virtual void UpdateVehicleTorquesAndAngles(float deltaTime);
      virtual void PostPhysicsUpdate();

      void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);

   protected:
      virtual ~PropelledVehicleActor();
   private:
      
      DECLARE_PROPERTY_INLINE(bool, StartBoost);
      DECLARE_PROPERTY_INLINE(float, StartBoostForce);
      DECLARE_PROPERTY_INLINE(float, MaximumBoostPerSecond);
      DECLARE_PROPERTY_INLINE(float, CurrentBoostTime);
      DECLARE_PROPERTY_INLINE(float, TimeToResetBoost);
      DECLARE_PROPERTY_INLINE(float, BoostResetTimer);

      dtCore::RefPtr<SimCore::FourWheelVehiclePhysicsHelper> mHelper;
   };

   class PropelledVehicleActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
   public:
      typedef SimCore::Actors::BasePhysicsVehicleActorProxy BaseClass;

      PropelledVehicleActorProxy();

      virtual void CreateActor();

      /**
       * Build the properties common to all platform objects
       */
      virtual void BuildPropertyMap();

      /// Adds additional invokables for this class.
      virtual void BuildInvokables();

   protected:
      virtual ~PropelledVehicleActorProxy();
   };

}

#endif /* PROPELLEDVEHICLEACTOR_H_ */