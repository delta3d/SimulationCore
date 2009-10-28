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

#include <EnemyMineAIHelper.h>
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
   {
      mAIHelper = new EnemyMineAIHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMineActor::~EnemyMineActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActor::OnEnteredWorld()
   {

      BaseClass::OnEnteredWorld();

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
   //void EnemyMineActor::PostPhysicsUpdate()
   //{
   //   // Mostly copied from BasePhysicsVehicleActor - we do NOT want want our vehicle to 'roll', so we
   //   // take the position and throw away the rotation.

   //   // This is ONLY called if we are LOCAL (we put the check here just in case... )
   //   if (!IsRemote() && GetPhysicsHelper() != NULL)
   //   {
   //      // The base behavior is that we want to pull the translation and rotation off the object
   //      // in our physics scene and apply it to our 3D object in the visual scene.
   //      dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();

   //      //TODO: Ask if the object is activated.  If not, the transform should not be pushed.
   //      if (!GetPushTransformToPhysics())
   //      {
   //         if(physicsObject != NULL)
   //         {
   //            // Take rotation from physics and apply to current xform - IE NO ROTATION!
   //            dtCore::Transform currentXForm;
   //            GetTransform(currentXForm);
   //            dtCore::Transform physicsXForm;
   //            physicsObject->GetTransform(physicsXForm);
   //            currentXForm.SetTranslation(physicsXForm.GetTranslation());

   //            //apply our own rotation
   //            mAIHelper->PostSync(currentXForm);

   //            SetTransform(currentXForm);
   //            //SetPushTransformToPhysics(false);
   //         }
   //      }
   //   }
   //}


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

      if(GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
      {       
         //this lets the AI respond to being hit
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TOOK_DAMAGE);
      }
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

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
