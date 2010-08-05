/* -*-c++-*-
 * SimulationCore
 * Copyright 2007-2010, Alion Science and Technology
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
 * Curtiss Murphy
 */

#ifndef PROPELLEDVEHICLEACTOR_H_
#define PROPELLEDVEHICLEACTOR_H_

#include <DemoExport.h>
#include <dtDAL/propertymacros.h>
#include <SimCore/Actors/FourWheelVehicleActor.h>
#include <SimCore/FourWheelVehiclePhysicsHelper.h>
#include <dtCore/particlesystem.h>

namespace NetDemo
{

   /////////////////////////////////////////////////////////
   class PropelledVehicleActor : public SimCore::Actors::FourWheelVehicleActor
   {
   public:
      typedef SimCore::Actors::FourWheelVehicleActor BaseClass;

      PropelledVehicleActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy);

      /// Override this to add your own components or to init values on the ones that are already added.
      virtual void BuildActorComponents();

      virtual void OnEnteredWorld();
      virtual void UpdateVehicleTorquesAndAngles(float deltaTime);

      /// Overridden for a few testing messages
      //virtual void ProcessMessage(const dtGame::Message& message);

   protected:
      virtual ~PropelledVehicleActor();

   private:
      // The following are values used to test and render Dead Reckoning Behaviors
      dtCore::RefPtr<dtCore::ParticleSystem> mTrailParticles;
      osg::Vec3 mDRTestingRealLocation;
      float mDRTestingAveragedError;
   };


   /////////////////////////////////////////////////////////
   class PropelledVehicleActorProxy : public SimCore::Actors::FourWheelVehicleActorProxy
   {
   public:
      typedef SimCore::Actors::FourWheelVehicleActorProxy BaseClass;

      PropelledVehicleActorProxy();

      virtual void CreateActor();

      /**
       * Build the properties common to all platform objects
       */
      virtual void BuildPropertyMap();

      /// Adds additional invokables for this class.
      virtual void BuildInvokables();

      virtual void OnEnteredWorld();

   protected:
      virtual ~PropelledVehicleActorProxy();
   };

}

#endif /* PROPELLEDVEHICLEACTOR_H_ */
