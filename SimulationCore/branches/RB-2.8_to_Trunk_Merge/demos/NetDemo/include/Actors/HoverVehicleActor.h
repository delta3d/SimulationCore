/* -*-c++-*-
* NetDemo - HoverVehicleActor (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
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
* @author Curtiss Murphy
*/
//#ifdef AGEIA_PHYSICS
#ifndef _HOVER_VEHICLE_ACTOR_
#define _HOVER_VEHICLE_ACTOR_

#include <DemoExport.h>

#include <Actors/HoverVehiclePhysicsHelper.h>
#include <SimCore/PhysicsTypes.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

namespace dtAudio
{
   class Sound;
}

namespace dtGame
{
   class Message;
}

namespace NetDemo
{

   ////////////////////////////////////////////////////////////////////////////////
   /* This class extends BasePhysicsVehicle and has hover tank behavior.
    */
   class NETDEMO_EXPORT HoverVehicleActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

         /// Constructor
         HoverVehicleActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

      protected:
         /// Destructor
         virtual ~HoverVehicleActor();

      // INHERITED PUBLIC
      public:

         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         virtual void PostPhysicsUpdate();

         /// Overridden so we can undo our rotation only thing for the physics. 
         virtual void SetTransform(const dtCore::Transform& xform, dtCore::Transformable::CoordSysEnum cs = dtCore::Transformable::ABS_CS);

         virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);
         virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);


      // PUBLIC METHODS
      public:

         /// Reset to starting position In additional to base behavior, it turns off sounds.
         virtual void ResetVehicle();

         HoverVehiclePhysicsActComp* GetHoverPhysicsActComp() {
            return static_cast<HoverVehiclePhysicsActComp*> (GetPhysicsActComp()); }

         /// These methods are kind of odd. Some vehicles have a distinct turret (no up/down, just rotate) that is
         /// separate from the vehicle. On others, the turret is hard attached to the vehicle.
         /// In this case, turning the motion model will rotate the vehicle.
         void SetVehicleIsTurret( bool vehicleIsTurret ) { mVehicleIsTurret = vehicleIsTurret; }
         bool GetVehicleIsTurret() const { return mVehicleIsTurret; }

      protected:
         /// Angles/ steering moving etc done here. Of the updates, this is called first.
         /// This does nothing by default.
         virtual void UpdateVehicleTorquesAndAngles(float deltaTime);

         /// Called update the dofs for your vehicle. Wheels or whatever. Of the updates, this is called second
         /// By default, this does nothing.
         virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);

         /// called from tick. Do your sounds. Of the updates, this is called third.
         /// Does nothing by default.
         virtual void UpdateSoundEffects(float deltaTime);


      // Private vars
      private:

         ///////////////////////////////////////////////////
         // Sound effects
         dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
         ///////////////////////////////////////////////////

         bool mVehicleIsTurret;

   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT HoverVehicleActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         HoverVehicleActorProxy();
         virtual void BuildPropertyMap();
         /// Override this to add your own components or to init values on the ones that are already added.
         virtual void BuildActorComponents();

      protected:
         virtual ~HoverVehicleActorProxy();
         void CreateDrawable();
         virtual void OnEnteredWorld();
   };

}

#endif
//#endif
