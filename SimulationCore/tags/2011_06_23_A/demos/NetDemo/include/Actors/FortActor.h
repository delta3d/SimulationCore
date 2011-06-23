/* -*-c++-*-
* Driver Demo - HoverVehicleActor (.cpp & .h) - Using 'The MIT License'
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
* @author Bradley Anderegg
*/

//#ifdef AGEIA_PHYSICS
#ifndef _FORT_ACTOR_
#define _FORT_ACTOR_

#include <DemoExport.h>

#include <SimCore/PhysicsTypes.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/VolumeRenderingComponent.h>

namespace dtAudio
{
   class Sound;
}

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   class DetonationMessage;

   namespace Actors
   {
      class MunitionTypeActor;
   }
}

namespace NetDemo
{

   ////////////////////////////////////////////////////////////////////////////////
   /* This class is for the team's home fort. If it is destroyed, the team looses. 
    * It has some firepower and may have some interactions with the player.
    */
   class NETDEMO_EXPORT FortActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

         /// Constructor
         FortActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

      protected:
         /// Destructor
         virtual ~FortActor();

      // INHERITED PUBLIC
      public:

         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         //virtual void PostPhysicsUpdate();

         virtual void OnTickLocal( const dtGame::TickMessage& tickMessage );
         virtual void OnTickRemote( const dtGame::TickMessage& tickMessage );

         /// Overridden from BaseEntity - can reduce, increase, or ignore incoming damage.
         virtual float ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message,
            const SimCore::Actors::MunitionTypeActor& munition);

         /// Overridden from BaseEntity - force a partial update every hit, not just when we take damage
         virtual void RespondToHit(const SimCore::DetonationMessage& message,
            const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force,
            const osg::Vec3& location);

      // PUBLIC METHODS
      public:

         //HoverVehiclePhysicsActComp* GetHoverPhysicsActComp() {
         //   return static_cast<HoverVehiclePhysicsActComp*> (GetPhysicsActComp());}


      protected:
         /// Called update the dofs for your vehicle. Wheels or whatever. Of the updates, this is called second
         /// By default, this does nothing.
         virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);

         /// called from tick. Do your sounds. Of the updates, this is called third.
         /// Does nothing by default.
         virtual void UpdateSoundEffects(float deltaTime);

      // Private vars
      private:

		 float mTimeSinceLightsWereUpdated;

         bool mLightIsOn;
         dtCore::RefPtr<SimCore::Components::RenderingSupportComponent::DynamicLight> mMainLight;
         dtCore::RefPtr<SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord> mShapeVolume;
         
         ///////////////////////////////////////////////////
         // Sound effects
         dtCore::RefPtr<dtAudio::Sound> mSndCollisionHit;
         ///////////////////////////////////////////////////
   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT FortActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActorProxy BaseClass;

         FortActorProxy();
         virtual void BuildPropertyMap();
         /// Override this to add your own components or to init values on the ones that are already added.
         virtual void BuildActorComponents();

      protected:
         virtual ~FortActorProxy();
         void CreateActor();
         virtual void OnEnteredWorld();
   };

}

#endif
//#endif
