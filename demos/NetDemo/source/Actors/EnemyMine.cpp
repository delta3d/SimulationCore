/*
* Copyright, 2009, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix-src.h>

#include <dtUtil/mswin.h>
#include <Actors/EnemyMine.h>

#include <dtDAL/enginepropertytypes.h>
//#include <NxAgeiaWorldComponent.h>
//#include <NxAgeiaRaycastReport.h>
//#include <dtABC/application.h>
//#include <dtAudio/audiomanager.h>
//#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
//#include <dtCore/batchisector.h>
//#include <dtCore/keyboard.h>
//#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
//#include <osg/Switch>
//#include <osgSim/DOFTransform>
//#include <osgViewer/View>
//#include <SimCore/Components/ArticulationHelper.h>
//#include <SimCore/Actors/EntityActorRegistry.h>
//#include <SimCore/Actors/TerrainActorProxy.h>
//#include <SimCore/Actors/MunitionTypeActor.h>
//#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/CollisionGroupEnum.h>
//#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palphysicsworld.h>

#include <EnemyAIHelper.h>
#include <AIEvent.h>
#include <AIState.h>
#include <AIUtility.h>
#include <ActorRegistry.h>
#include <Actors/FortActor.h>
#include <Actors/EnemyDescriptionActor.h>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMineActor::EnemyMineActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : BaseEnemyActor(proxy)
      , mAIHelper(new EnemyAIHelper())
      , mGoalLocation(10.0, 10.0, 10.0)
      , mGroundClearance(3.0)
   {

   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMineActor::~EnemyMineActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyAIHelper* EnemyMineActor::GetAIHelper()
   {
      return mAIHelper.get();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   const EnemyAIHelper* EnemyMineActor::GetAIHelper() const
   {
      return mAIHelper.get();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActor::OnEnteredWorld()
   {

      // Create our vehicle with a starting position
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      osg::Vec3 startVec;
      if (!IsRemote()) // Local - we vary it's starting position.
      {
         startVec[0] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[1] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[2] += dtUtil::RandFloat(0.0, 4.0);

         // Since we changed our starting position, update our visual actor, or it 'blips' for
         // one frame in the wrong place. Very ugly.
         ourTransform.SetTranslation(startVec[0], startVec[1], startVec[2]);
         SetTransform(ourTransform);

      }

      BaseClass::OnEnteredWorld();

      if (!IsRemote()) // Local - we give it a boost on creation
      {
         //osg::Vec3 dir(0.0, 0.0, 2000.0);
         //ApplyForce(dir);

         // Offset the Target Dir so that they spread out around the map.
         mGoalLocation[0] += dtUtil::RandFloat(-40.0, 40.0);
         mGoalLocation[1] += dtUtil::RandFloat(-50.0, 50.0);
         mGoalLocation[2] += dtUtil::RandFloat(2.0, 4.0);
      }


      //AI SETUP CODE
      if (!IsRemote()) // I guess the AI only runs on the server?
      {
         //calling init on the AIHelper will setup the states and transitions
         //note: init is now called by the spawn component
         //mAIHelper->Init();

         //this will allow the AI to actually move us
         mAIHelper->GetPhysicsModel()->SetPhysicsHelper(GetPhysicsHelper());
         
         //redirecting the find target function
         dtAI::NPCState* state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyMineActor::FindTarget));

         //redirecting the detonate function
         state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_DETONATE);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyMineActor::DoExplosion));


         //all this code is to make the mine detonate when it takes damage
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FOLLOW, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FLOCK, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_DETONATE);


         
         //calling spawn will start the AI
         mAIHelper->Spawn();
      }
   }

   void EnemyMineActor::FindTarget(float)
   {
      //temporarily lets just look for a fort to destroy
      FortActorProxy* fortProxy = NULL;
      GetGameActorProxy().GetGameManager()->FindActorByType(*NetDemoActorRegistry::FORT_ACTOR_TYPE, fortProxy);
      if (fortProxy != NULL)
      {
         FortActor& fort = *static_cast<FortActor*>(fortProxy->GetActor());
         mAIHelper->SetCurrentTarget(fort);
      }

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      //if( ! IsMobilityDisabled())
      //{
      //   // Do all our movement!
      //   ApplyTargetHoverForces(deltaTime, mGoalLocation);
      //}

      // DoExplosion();
   }

   //////////////////////////////////////////////////////////////////////
   void EnemyMineActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

      //Tick the AI
      //update the AI's position and orientation
      dtCore::Transform trans;
      GetTransform(trans);
      mAIHelper->PreSync(trans);

      //let the AI do its thing
      mAIHelper->Update(tickMessage.GetDeltaSimTime());

      //update the entities position and orientation
      //note: this is commented out because it is updated by the physics
      //mAIHelper->PostSync(trans);
      //SetTransform(trans);
   }


   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActor::DoExplosion(float)
   {
      //const osg::Vec3& finalVelocity, const osg::Vec3& location, const dtCore::Transformable* target )
      //printf("Sending DETONATION\r\n");

      dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      osg::Vec3 trans = ourTransform.GetTranslation();

      // Prepare a detonation message
      dtCore::RefPtr<SimCore::DetonationMessage> msg;
      gm->GetMessageFactory().CreateMessage( SimCore::MessageType::DETONATION, msg );

      // Required Parameters:
      msg->SetEventIdentifier( 1 );
      msg->SetDetonationLocation(trans);
      // --- DetonationResultCode 1 == Entity Impact, 3 == Ground Impact, 5 == Detonation
      msg->SetDetonationResultCode( 3 ); // TO BE DYNAMIC
      msg->SetMunitionType("Generic Explosive");
      msg->SetFuseType(0);
      msg->SetWarheadType(0);
      msg->SetQuantityFired(1);
      msg->SetSendingActorId(GetGameActorProxy().GetId());
      //msg->SetFinalVelocityVector( finalVelocity );
      msg->SetRateOfFire(1);

      gm->SendMessage( *msg );
      gm->SendNetworkMessage( *msg );

      mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DIE);

      GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActor::RespondToHit(const SimCore::DetonationMessage& message,
      const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force, 
      const osg::Vec3& location)
   {
      // the base class applies an impulse
      BaseClass::RespondToHit(message, munition, force, location);

      //this lets the AI respond to being hit
      mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TOOK_DAMAGE);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActor::ApplyTargetHoverForces(float deltaTime, osg::Vec3 &goalLocation)
   {
      dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();
      deltaTime = (deltaTime > 0.2) ? 0.2 : deltaTime;  // cap at 0.2 second to avoid rare 'freak' outs.
      float weight = physicsObject->GetMass();

      // First thing we do is try to make sure we are hovering...
      osg::Vec3 velocity = physicsObject->GetBodyWrapper()->GetLinearVelocity();
      osg::Vec3 pos = physicsObject->GetTranslation();
      osg::Vec3 posLookAhead = pos + velocity * 0.5; // where would our vehicle be in .5 seconds?

      // Adjust position so that we are 'hovering' above the ground. The look ahead position
      // massively helps smooth out the bouncyness
      osg::Vec3 direction( 0.0f, 0.0f, -1.0f);
      float distanceToHit = 0.0;
      float futureAdjustment = ComputeEstimatedForceCorrection(posLookAhead, direction, distanceToHit);
      float currentAdjustment = ComputeEstimatedForceCorrection(pos, direction, distanceToHit);

      // Add an 'up' impulse based on the weight of the vehicle, our current time slice, and the adjustment.
      // Use current position and estimated future position to help smooth out the force.
      float finalAdjustment = currentAdjustment * 0.95 + futureAdjustment * 0.05;
      float upForce = weight * finalAdjustment;// * deltaTime;
      osg::Vec3 dir(0.0, 0.0, 1.0);
      physicsObject->GetBodyWrapper()->AddForce(dir * upForce);


      // Now, figure out how to move our target toward our goal. We want it to sort of oscillate a bit,
      // so we play with the forces a tad.
      osg::Vec3 targetVector =  goalLocation - pos;
      osg::Vec3 targetVectorLookAhead = goalLocation - posLookAhead;
      float distanceAway = targetVector.length() * 0.4 + targetVectorLookAhead.length() * 0.6;
      float distanceAwayPercent = (distanceAway) / mGroundClearance;
      float forceAdjustment = dtUtil::Min(10.0f, distanceAwayPercent);
      targetVector.normalize();
      targetVector[2] = 0.01; // cancel out the z - that's up above.
      targetVector[1] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
      targetVector[0] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
      physicsObject->GetBodyWrapper()->AddForce(targetVector * upForce);

   }

   ////////////////////////////////////////////////////////////////////////////////
   float EnemyMineActor::ComputeEstimatedForceCorrection(const osg::Vec3 &location,
      const osg::Vec3 &direction, float &distanceToHit)
   {
      static const int GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);
      float estimatedForceAdjustment = -dtPhysics::PhysicsWorld::GetInstance().GetGravity().z(); // gravity
      osg::Vec3 terrainHitLocation;

      distanceToHit = GetPhysicsHelper()->FindClosestIntersectionUsingDirection(location,
         direction, terrainHitLocation, GROUPS_FLAGS);

      if (distanceToHit > 0.0f)
      {
         // Apply gravity force at the exact height we want. More below, less above.
         float distanceToCorrect = /*GetSphereRadius() +*/ mGroundClearance - distanceToHit;
         if (distanceToCorrect >= 0.0f) // allow a 1 meter buffer to smoothly decay force
         {
            float distanceAwayPercent = distanceToCorrect / mGroundClearance;
            estimatedForceAdjustment *= (1.0f + (distanceAwayPercent * distanceAwayPercent));
         }
         else if (distanceToCorrect > -1.0f) // if we are slightly too high, then slow down our force
            estimatedForceAdjustment *= (0.01f + ((1.0f+distanceToCorrect) * 0.99)); // part gravity, part reduced
         else // opportunity to counteract free fall if we want.
            estimatedForceAdjustment = 0.0f;//0.01f;
      }
      else
         estimatedForceAdjustment = 0.0f;

      return estimatedForceAdjustment;
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   EnemyMineActorProxy::EnemyMineActorProxy()
   {
      SetClassName("EnemyMineActor");
  }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActorProxy::BuildPropertyMap()
   {
      const std::string GROUP = "Enemy Props";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      EnemyMineActor& actor = static_cast<EnemyMineActor &>(GetGameActor());

   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMineActorProxy::~EnemyMineActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActorProxy::CreateActor()
   {
      SetActor(*new EnemyMineActor(*this));
   }

   void EnemyMineActorProxy::InitAI(const EnemyDescriptionActor& desc)
   {
      EnemyMineActor& actor = static_cast<EnemyMineActor&>(GetGameActor());
      actor.GetAIHelper()->Init(desc);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
