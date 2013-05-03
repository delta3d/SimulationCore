/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author Bradley Anderegg
*/
#include <prefix/SimCorePrefix.h>

//#ifdef AGEIA_PHYSICS
#include <Actors/FireBallTowerActor.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
//#include <NxAgeiaWorldComponent.h>
//#include <NxAgeiaRaycastReport.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtCore/shaderprogram.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/keyboard.h>
#include <dtGame/basemessages.h>
#include <SimCore/Components/RenderingSupportComponent.h>
//#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/DefaultFlexibleArticulationHelper.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/ApplyShaderVisitor.h>
#include <SimCore/CollisionGroupEnum.h>

//#include <dtUtil/nodeprintout.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>

#include <ActorRegistry.h>
#include <AIState.h>
#include <AIEvent.h>
#include <Actors/FortActor.h>
#include <Actors/EnemyHelix.h>
#include <Actors/EnemyMine.h>
#include <Actors/FireBallActor.h>

//for debug printouts
#include <iostream>


namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   FireBallTowerActor::FireBallTowerActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mSleepTime(0.0f)
      , mMaxSleepTime(2.0f)
      , mTimeSinceLastFire(100.0f)
   {
      mSleepTime = mMaxSleepTime * dtUtil::RandPercent();
      SetTerrainPresentDropHeight(0.0);

      SetEntityType("Fort");
      SetMunitionDamageTableName("StandardDamageType");

      mAIHelper = new TowerAIHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   FireBallTowerActor::~FireBallTowerActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::OnEnteredWorld()
   {
      EnsureResourcesAreLoaded();

      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();
      physObj->SetTransform(ourTransform);
      physObj->CreateFromProperties(GetNonDamagedFileNode());

      if(!IsRemote())
      {

         // Setup our articulation helper for the vehicle
         dtCore::RefPtr<SimCore::Components::DefaultFlexibleArticulationHelper> articHelper =
            new SimCore::Components::DefaultFlexibleArticulationHelper();
         articHelper->SetEntity(this);
         articHelper->AddArticulation("dof_turret_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_HEADING);
         articHelper->AddArticulation("dof_gun_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_ELEVATION, "dof_turret_01");
         SetArticulationHelper(articHelper.get());

         mAIHelper->Init(NULL);

         //this will allow the AI to actually move us
         mAIHelper->GetPhysicsModel()->SetPhysicsActComp(GetPhysicsActComp());

         //redirecting the find target function
         dtAI::NPCState* stateFindTarget = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
         stateFindTarget->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &FireBallTowerActor::FindTarget));

         //redirecting the shoot 
         dtAI::NPCState* stateFireLaser = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIRE_LASER);
         stateFireLaser->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &FireBallTowerActor::Shoot));

         //redirecting the idle to our sleep function
         dtAI::NPCState* stateIdle = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_IDLE);
         stateIdle->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &FireBallTowerActor::Sleep));


         //calling spawn will start the AI
         mAIHelper->Spawn();
      }

      // Attach a special shader.
      dtCore::RefPtr<SimCore::ApplyShaderVisitor> visitor = new SimCore::ApplyShaderVisitor();
      visitor->AddNodeName("Eye360");
      visitor->SetShaderName("ColorPulseShader");
      visitor->SetShaderGroup("CustomizableVehicleShaderGroup");
      GetOSGNode()->accept(*visitor);

      BaseClass::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::OnRemovedFromWorld()
   {

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::UpdateSoundEffects(float deltaTime)
   {
      BaseClass::UpdateSoundEffects(deltaTime);
   }

  ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum &damageState)
   {
      if (damageState != GetDamageState())
      {
         BaseClass::SetDamageState(damageState);

         // Mark the AI as 'dead' so we stop 'steering'
         if(IsMobilityDisabled())
         {
            mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DIE);
         }
      }
   }


   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::FindTarget(float)
   {
      float minDist = 200.0;
      dtCore::Transformable* enemy = NULL;

      std::vector<dtDAL::ActorProxy*> actorArray;
      GetGameActorProxy().GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE, actorArray);
      //GetGameActorProxy().GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE, actorArray);

      while(!actorArray.empty())
      {
         dtCore::Transformable* curr = static_cast<dtCore::Transformable*>(actorArray.back()->GetActor());
         float dist = GetDistance(*curr);

         if(dist < minDist)
         {
            enemy = curr;
            minDist = dist;
         }

         actorArray.pop_back();
      }   

      GetGameActorProxy().GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE, actorArray);
      while(!actorArray.empty())
      {
         dtCore::Transformable* curr = static_cast<dtCore::Transformable*>(actorArray.back()->GetActor());
         float dist = GetDistance(*curr);

         if(dist < minDist)
         {
            enemy = curr;
            minDist = dist;
         }

         actorArray.pop_back();
      }   

      if(enemy != NULL)
      {
         mTarget = enemy;
         mAIHelper->SetCurrentTarget(*enemy);
      }
      else
      {
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_NO_TARGET_FOUND);
      }

   }

   //////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::SetTarget(BaseEnemyActor* enemy)
   {
      mTarget = enemy;
      mAIHelper->SetCurrentTarget(*enemy);
   }
   //////////////////////////////////////////////////////////////////////
   float FireBallTowerActor::GetDistance( const dtCore::Transformable& t ) const
   {

      dtCore::Transform xform;
      osg::Vec3 pos, enemyPos;
      GetTransform(xform);
      xform.GetTranslation(pos);

      t.GetTransform(xform);
      xform.GetTranslation(enemyPos);

      return (enemyPos - pos).length();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::Sleep(float dt)
   {
      mSleepTime -= dt;

      if(mSleepTime <= 0.0f)
      {
         mSleepTime = mMaxSleepTime;
         //using this event to wake us up
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TOOK_DAMAGE);
      }
   }


   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::Shoot(float dt)
   {
      if(mTimeSinceLastFire > 3.0f)
      {
         mTimeSinceLastFire = 0.0f;

         //create a fireball actor
         dtCore::RefPtr<FireBallActorProxy> proxy;
         GetGameActorProxy().GetGameManager()->CreateActor(*NetDemoActorRegistry::FIREBALL_ACTOR_TYPE, proxy);
         if(proxy.valid())
         {
            FireBallActor* fireball = NULL;
            proxy->GetActor(fireball);
            if(fireball != NULL)
            {

               float fireBallSpeed = 2.5f;

               dtCore::Transform xform;
               osg::Vec3 pos, enemyPos; 
               GetTransform(xform);
               xform.GetTranslation(pos);

               dtUtil::NodeCollector* nodes = GetNodeCollector();
               osgSim::DOFTransform* dof = nodes->GetDOFTransform("dof_gun_01");
               if (dof != NULL)
               {
                  osg::NodePathList nodePathList = dof->getParentalNodePaths();
                  if(!nodePathList.empty())
                  {
                     osg::NodePath nodePath = nodePathList[0];
                     osg::Matrix dofMat = osg::computeLocalToWorld(nodePath);

                     osg::Vec3 pos = dtUtil::MatrixUtil::GetRow3(dofMat, 3);
                     osg::Vec3 dir = dtUtil::MatrixUtil::GetRow3(dofMat, 1);
                     
                     fireball->SetVelocity(fireBallSpeed);
                     fireball->SetPosition(osg::Vec3(0.0f, 0.0f, 1.0f) + pos + (dir * 7.5));

                     osg::Vec3 up(0.0, 0.0, 2500.0);
                     fireball->AddForce(up);// + (dir * fireBallSpeed));

                     if(mTarget.valid())
                     {
                        fireball->SetTarget(*mTarget);
                     }
                  }
               }
               
               GetGameActorProxy().GetGameManager()->AddActor(*proxy, false, true);
            }
          }
      }

      //randomly search for better targets
      //temporary hack
      if(dtUtil::RandPercent() < 0.05)
      {
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TARGET_KILLED);
      }
   }

   //////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

      dtUtil::NodeCollector* nodes = GetNodeCollector();
      osgSim::DOFTransform* dof = nodes->GetDOFTransform("dof_turret_01");
      if (dof != NULL)
      {
         osg::Vec3 hpr, hprLast;
         dtCore::Transform trans;

         GetTransform(trans);
         hprLast = dof->getCurrentHPR();

         mAIHelper->PreSync(trans);

         ////////let the AI do its thing
         mAIHelper->Update(tickMessage.GetDeltaSimTime());

         //we are static, no need to obtain an update
         //mAIHelper->PostSync(trans);

         hpr = hprLast;
         osg::Vec2 angle = mAIHelper->GetWeaponAngle();
         hpr[0] = angle[0]  - osg::PI_2;
         hpr[1] = -angle[1] + (0.95 * osg::PI_2);

         if(dtUtil::IsFinite(hpr[0]) && dtUtil::IsFinite(hpr[1]))
         {
            dof->setCurrentHPR(hpr);

            if(GetArticulationHelper() != NULL)
            {
               GetArticulationHelper()->HandleUpdatedDOFOrientation(*dof, hpr - hprLast, hpr);
            }
         }
      }


      mTimeSinceLastFire += tickMessage.GetDeltaSimTime();
      if(mAIHelper->GetTriggerState())
      {
         Shoot(tickMessage.GetDeltaSimTime());
      }
   }

   //////////////////////////////////////////////////////////////////////
   void FireBallTowerActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   FireBallTowerActorProxy::FireBallTowerActorProxy()
   {
      SetClassName("FireBallTowerActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActorProxy::BuildPropertyMap()
   {
      BaseClass::BuildPropertyMap();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   FireBallTowerActorProxy::~FireBallTowerActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActorProxy::CreateActor()
   {
      SetActor(*new FireBallTowerActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActorProxy::OnRemovedFromWorld()
   {
      BaseClass::OnRemovedFromWorld();

      FireBallTowerActor* actor = NULL;
      GetActor(actor);      
      actor->OnRemovedFromWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FireBallTowerActorProxy::BuildActorComponents()
   {
      BaseClass::BuildActorComponents();


      dtPhysics::PhysicsActComp* physAC = NULL;
      GetComponent(physAC);
      // Add our initial body.
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("VehicleBody");
      physAC->AddPhysicsObject(*physicsObject);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
      physicsObject->SetMass(30000.0f);
      //physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::STATIC);


      dtGame::DRPublishingActComp* drPublishingActComp = NULL;
      GetComponent(drPublishingActComp);
      if (drPublishingActComp == NULL)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(1.5f);
      //drPublishingActComp->SetPublishLinearVelocity(false);
      //drPublishingActComp->SetPublishAngularVelocity(false);
   }

} // namespace
//#endif

