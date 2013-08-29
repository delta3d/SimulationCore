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
#include <prefix/SimCorePrefix.h>

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
#include <dtPhysics/physicsactcomp.h>
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
         mAIHelper->GetPhysicsModel()->SetPhysicsActComp(GetPhysicsActComp());
         
         //redirecting the find target function
         dtAI::NPCState* state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyMineActor::FindTarget));

         //redirecting the detonate function
         state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_DETONATE);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &EnemyMineActor::DoExplosion));


         //all this code is to make the mine detonate when it takes damage
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_IDLE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_DIE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FIND_TARGET, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_GO_TO_WAYPOINT, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_ATTACK, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_EVADE, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FOLLOW, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_FLOCK, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_WANDER, &AIStateType::AI_STATE_DETONATE);
         mAIHelper->AddTransition(&AIEvent::AI_EVENT_TOOK_DAMAGE, &AIStateType::AI_STATE_DIE, &AIStateType::AI_STATE_DIE);

         
         //calling spawn will start the AI
         mAIHelper->Spawn();
      }
   }

   //////////////////////////////////////////////////////////////////////
   void EnemyMineActor::FindTarget(float)
   {
      FortActor* fort = GetCurrentFortUnderAttack();
      if(fort != NULL)
      {
         mAIHelper->SetCurrentTarget(*fort);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      BaseClass::UpdateVehicleTorquesAndAngles(deltaTime);
   }

   //////////////////////////////////////////////////////////////////////
   void EnemyMineActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
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

      BaseClass::OnTickLocal(tickMessage);
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
      BaseClass::BuildPropertyMap();
      
      //const std::string GROUP = "Enemy Props";

      //EnemyMineActor& actor = static_cast<EnemyMineActor &>(GetGameActor());

   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyMineActorProxy::~EnemyMineActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActorProxy::CreateDrawable()
   {
      SetDrawable(*new EnemyMineActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyMineActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
