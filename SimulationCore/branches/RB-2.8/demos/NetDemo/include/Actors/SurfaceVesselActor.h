/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2014, Caper Holdings LLC
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
* Bradley Anderegg
*/

#ifndef SurfaceVesselActor_H_
#define SurfaceVesselActor_H_

#include <DemoExport.h>
#include <dtCore/propertymacros.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <dtGame/basemessages.h>

namespace dtGame
{
   class TickMessage;
}

namespace NetDemo
{

   /////////////////////////////////////////////////////////
   class NETDEMO_EXPORT SurfaceVessel : public SimCore::Actors::BasePhysicsVehicleActor
   {
   public:
      typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

      SurfaceVessel(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy);

      virtual void OnEnteredWorld();
      virtual void UpdateVehicleTorquesAndAngles(float deltaTime);

   protected:
      virtual ~SurfaceVessel();

   private:
   };


   /////////////////////////////////////////////////////////
   class NETDEMO_EXPORT SurfaceVesselActor : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
   public:
      typedef SimCore::Actors::BasePhysicsVehicleActorProxy BaseClass;

      SurfaceVesselActor();

      virtual void CreateDrawable();

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
      virtual ~SurfaceVesselActor();
   };

}

#endif /* SurfaceVesselActor_H_ */
