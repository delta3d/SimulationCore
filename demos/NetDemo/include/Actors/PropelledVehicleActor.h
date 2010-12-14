/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
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
#include <dtGame/basemessages.h>

namespace dtGame
{
   class TickMessage;
}

namespace NetDemo
{

   /////////////////////////////////////////////////////////
   class PropelledVehicleActor : public SimCore::Actors::FourWheelVehicleActor
   {
   public:
      typedef SimCore::Actors::FourWheelVehicleActor BaseClass;

      PropelledVehicleActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy);

      virtual void OnEnteredWorld();
      virtual void UpdateVehicleTorquesAndAngles(float deltaTime);

      virtual void OnTickRemoteTest(const dtGame::TickMessage& message);

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

      /// Override this to add your own components or to init values on the ones that are already added.
      virtual void BuildActorComponents();

   protected:
      virtual ~PropelledVehicleActorProxy();
   };

}

#endif /* PROPELLEDVEHICLEACTOR_H_ */
