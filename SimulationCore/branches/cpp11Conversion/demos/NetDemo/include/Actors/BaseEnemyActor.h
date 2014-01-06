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
#ifndef _BASE_ENEMY_ACTOR_
#define _BASE_ENEMY_ACTOR_

#include <DemoExport.h>

#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Messages.h>
#include <SimCore/PhysicsTypes.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

#include <Actors/FortActor.h>
#include <EnemyAIHelper.h>

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
   /* This class is for the team's home fort. If it is destroyed, the team looses. 
    * It has some firepower and may have some interactions with the player.
    */
   class NETDEMO_EXPORT BaseEnemyActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

         /// Constructor
         BaseEnemyActor (SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

      protected:
         /// Destructor
         virtual ~BaseEnemyActor();

      // INHERITED PUBLIC
      public:
         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);
         virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);

         virtual void UpdateVehicleTorquesAndAngles(float deltaTime);


         virtual void InitAI(const EnemyDescriptionActor* desc);

         //overriden so we can not take damage from other enemy vehicles
         virtual float ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message, 
            const SimCore::Actors::MunitionTypeActor& munition);

         EnemyAIHelper* GetAIHelper();
         const EnemyAIHelper* GetAIHelper() const;

         //can take nullptr Ptr, returns false if nullptr true if ActorType = enemy actor type
         bool IsEnemyActor(dtGame::GameActorProxy* proxy) const;

         /// Overrriden to kill our AI when we die.
         virtual void SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum& damageState);

         float GetTimeToExistAfterDead();
         void SetTimeToExistAfterDead(float newTime);

         void SetPointValue(int points);
         int GetPointValue() const;

		 FortActor* GetCurrentFortUnderAttack();

      protected:
         /// Called update the dofs for your vehicle. Wheels or whatever. Of the updates, this is called second
         /// By default, this does nothing.
         virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);

         /// called from tick. Do your sounds. Of the updates, this is called third.
         /// Does nothing by default.
         virtual void UpdateSoundEffects(float deltaTime);

         void DoExplosion(float dt);

         /**
          * Called after getting hit, after damage is already calc'ed and applied. Not much to do here really
          */
         void RespondToHit(const SimCore::DetonationMessage& message,
            const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force, 
            const osg::Vec3& location);

         dtCore::Transformable* GetClosestTower();

         std::shared_ptr<EnemyAIHelper> mAIHelper;

		 //only the main mothership sets this
		 static std::weak_ptr<FortActor> mCurrentFortUnderAttack;

      // Private vars
      private:
         std::shared_ptr<dtAudio::Sound> mSndCollisionHit;

         bool mSendScoreMessage; // Flag to trigger sending a score message when RespondToHit is called after a damage state change to DIE.
         int mPointValue;
         float mTimeSinceBorn; // how long I've been in the world - used for some enemies that self-terminate after X seconds
         float mTimeToExistAfterDead; // Once we are dead, we stay alive for this long, and then self-delete. 
         float mTimeSinceKilled; // used to self-delete dead entities after they explode, flame, fall, etc...
         float mTimeSinceLightsWereUpdated;
   };

   ////////////////////////////////////////////////////////////////////////////////
   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT BaseEnemyActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActorProxy BaseClass;

         static const dtUtil::RefString PROPERTY_POINT_VALUE;

         BaseEnemyActorProxy();
         virtual void BuildPropertyMap();
         /// Override this to add your own components or to init values on the ones that are already added.
         virtual void BuildActorComponents();

      protected:
         virtual ~BaseEnemyActorProxy();
         void CreateDrawable();
         virtual void OnEnteredWorld();
   };

}

#endif
//#endif
