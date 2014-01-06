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
* @author Bradley Anderegg
*/
//#ifdef AGEIA_PHYSICS
#ifndef NETDEMO_LightTower
#define NETDEMO_LightTower

#include <DemoExport.h>

#include <SimCore/PhysicsTypes.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

#include <SimCore/Components/RenderingSupportComponent.h>

#include <TowerAIHelper.h>
#include <Actors/BaseEnemyActor.h>


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
   namespace Actors
   {
      class WeaponActor;
      class WeaponActorProxy;
   }
}

namespace NetDemo
{

   class NETDEMO_EXPORT LightTower : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

         /// Constructor
         LightTower (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();

         virtual void OnTickLocal( const dtGame::TickMessage& tickMessage );
         virtual void OnTickRemote( const dtGame::TickMessage& tickMessage );


         virtual void SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum& damageState);

         void EnableSpotLight(bool b);
         void Sleep(float dt);

      protected:
      virtual ~LightTower();

         virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);
         virtual void UpdateSoundEffects(float deltaTime);

         void FindTarget(float);
         float GetDistance(const dtCore::Transformable& t) const;
         void SetTarget(const BaseEnemyActor* t);

         void CreateLights();

      private:

         float mSleepTime, mMaxSleepTime;
         std::shared_ptr<SimCore::Components::RenderingSupportComponent::SpotLight> mTargetLight;
         std::shared_ptr<SimCore::Components::RenderingSupportComponent::DynamicLight> mMainLight;

         std::shared_ptr<TowerAIHelper> mAIHelper;
         std::shared_ptr<dtAudio::Sound> mSndCollisionHit;
   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT LightTowerProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActorProxy BaseClass;

         LightTowerProxy();
         virtual void BuildPropertyMap();

         /// Override this to add your own components or to init values on the ones that are already added.
         virtual void BuildActorComponents();

      protected:
         virtual ~LightTowerProxy();
         void CreateDrawable();
         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();
   };

}

#endif
//#endif //NETDEMO_LightTower
