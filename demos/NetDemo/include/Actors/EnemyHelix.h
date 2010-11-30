/* Net Demo - EnemyHelix (.cpp & .h) - Using 'The MIT License'
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
* Bradley Anderegg
*/
#ifndef NETDEMO_ENEMY_HELIX_ACTOR
#define NETDEMO_ENEMY_HELIX_ACTOR

#include <DemoExport.h>
#include <EnemyHelixAIHelper.h>
#include <Actors/BaseEnemyActor.h>

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
   class EnemyAIHelper;

   ////////////////////////////////////////////////////////////////////////////////
   /* This class extends BasePhysicsVehicle. It is intended to be a simple target
    * that you can shoot at. It will publish itself and can be shot by remote players.
    */
   class NETDEMO_EXPORT EnemyHelixActor : public BaseEnemyActor
   {
      public:
         typedef BaseEnemyActor BaseClass;

         /// Constructor
         EnemyHelixActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

         /*virtual*/ void OnEnteredWorld();

         /*virtual*/ void OnRemovedFromWorld();

         /*virtual*/ void OnTickLocal(const dtGame::TickMessage& tickMessage);

         void PostPhysicsUpdate();

      protected:
      /// Destructor
      virtual ~EnemyHelixActor();

         /// Angles/ steering moving etc done here. Of the updates, this is called first.
         /*virtual*/ void UpdateVehicleTorquesAndAngles(float deltaTime);

         void FindTarget(float);
         void Shoot(float);

         void InitWeapon();

      // Private vars
      private:


         float mTimeSinceLastFire;
         dtCore::ObserverPtr<dtCore::Transformable> mTarget;
         dtCore::RefPtr<SimCore::Actors::WeaponActor> mWeapon;
         dtCore::RefPtr<SimCore::Actors::WeaponActorProxy> mWeaponProxy;

   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT EnemyHelixActorProxy : public BaseEnemyActorProxy
   {
      public:
         typedef BaseEnemyActorProxy BaseClass;

         EnemyHelixActorProxy();
         virtual void BuildPropertyMap();

      protected:
         virtual ~EnemyHelixActorProxy();
         void CreateActor();
   };

}

#endif //NETDEMO_ENEMY_HELIX_ACTOR
