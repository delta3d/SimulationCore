/* Net Demo - EnemyMothership (.cpp & .h) - Using 'The MIT License'
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
#ifndef NETDEMO_ENEMY_MOTHERSHIP_ACTOR
#define NETDEMO_ENEMY_MOTHERSHIP_ACTOR

#include <DemoExport.h>
#include <EnemyMothershipAIHelper.h>
#include <Actors/BaseEnemyActor.h>


namespace NetDemo
{
   class EnemyAIHelper;

   ////////////////////////////////////////////////////////////////////////////////
   /* This class extends BasePhysicsVehicle. It is intended to be a simple target
   * that you can shoot at. It will publish itself and can be shot by remote players.
   */
   class NETDEMO_EXPORT EnemyMothershipActor : public BaseEnemyActor
   {
   public:
      typedef BaseEnemyActor BaseClass;

      /// Constructor
      EnemyMothershipActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

      /*virtual*/ float ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message, 
         const SimCore::Actors::MunitionTypeActor& munition);

      /*virtual*/ void OnEnteredWorld();

      /*virtual*/ void OnTickLocal(const dtGame::TickMessage& tickMessage);

      void PostPhysicsUpdate();

      osg::Vec3 GetSpawnPoint() const;

   protected:
      /// Destructor
      virtual ~EnemyMothershipActor();

      /// Angles/ steering moving etc done here. Of the updates, this is called first.
      /*virtual*/ void UpdateVehicleTorquesAndAngles(float deltaTime);

      void FindTarget(float dt);
      void AddDynamicLight();

	  
      void SelectFortToAttack();

      // Private vars
   private:
      float mTimeToCheckForTarget;
	  
	  // the first mothership created is the main mothership and decides which fort to attack
	  bool mMainMothership;
	  static bool mHasMainMothership;
   };

   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT EnemyMothershipActorProxy : public BaseEnemyActorProxy
   {
   public:
      typedef BaseEnemyActorProxy BaseClass;

      EnemyMothershipActorProxy();
      virtual void BuildPropertyMap();

   protected:
      virtual ~EnemyMothershipActorProxy();
      void CreateActor();
      virtual void OnEnteredWorld();
      virtual void OnRemovedFromWorld();
   };

}

#endif //NETDEMO_ENEMY_MOTHERSHIP_ACTOR
