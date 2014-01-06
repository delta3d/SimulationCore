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
#ifndef NETDEMO_TOWERACTOR
#define NETDEMO_TOWERACTOR

#include <DemoExport.h>

#include <SimCore/PhysicsTypes.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

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

   class NETDEMO_EXPORT TowerActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

         /// Constructor
         TowerActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();

         virtual void OnTickLocal( const dtGame::TickMessage& tickMessage );
         virtual void OnTickRemote( const dtGame::TickMessage& tickMessage );


         virtual void SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum& damageState);

         /// Overridden from BaseEntity - can reduce, increase, or ignore incoming damage.
         virtual float ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message,
            const SimCore::Actors::MunitionTypeActor& munition);


         void Sleep(float dt);

      protected:
      virtual ~TowerActor();

         virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);
         virtual void UpdateSoundEffects(float deltaTime);

         void FindTarget(float);
         void Shoot(float);
         float GetDistance(const dtCore::Transformable& t) const;
         void SetTarget(const BaseEnemyActor* t);

         void InitWeapon();

      private:

         float mSleepTime, mMaxSleepTime;
         std::shared_ptr<TowerAIHelper> mAIHelper;
         std::shared_ptr<dtAudio::Sound> mSndCollisionHit;
   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT TowerActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActorProxy BaseClass;

         TowerActorProxy();
         virtual void BuildPropertyMap();
         /// Override this to add your own components or to init values on the ones that are already added.
         virtual void BuildActorComponents();

      protected:
         virtual ~TowerActorProxy();
         void CreateDrawable();
         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();
   };

}

#endif
//#endif //NETDEMO_TOWERACTOR
