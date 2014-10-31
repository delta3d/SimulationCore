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
#include <Actors/TowerActor.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
//#include <NxAgeiaWorldComponent.h>
//#include <NxAgeiaRaycastReport.h>
#include <dtCore/enginepropertytypes.h>
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
#include <SimCore/Actors/BaseEntity.h>

//#include <dtUtil/nodeprintout.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>

#include <ActorRegistry.h>
#include <AIState.h>
#include <AIEvent.h>
#include <Actors/FortActor.h>
#include <Actors/EnemyHelix.h>
#include <Actors/EnemyMine.h>
#include <SimCore/ActComps/WeaponInventoryActComp.h>
#include <Components/GameLogicComponent.h>

//for debug printouts
#include <iostream>


namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   TowerActor::TowerActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mSleepTime(0.0f)
      , mMaxSleepTime(2.0f)
   {
      mSleepTime = mMaxSleepTime * dtUtil::RandPercent();
      SetTerrainPresentDropHeight(0.0);

      SetEntityType("Fort");
      SetMunitionDamageTableName("StandardDamageType");

      mAIHelper = new TowerAIHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   TowerActor::~TowerActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   float TowerActor::ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message, 
      const SimCore::Actors::MunitionTypeActor& munition)
   {
      //dtGame::GameActorProxy* gap = GetGameActorProxy().GetGameManager()->FindGameActorById(message.GetSendingActorId());
      //return incomingDamage * float(!IsEnemyActor(gap));
      float result = incomingDamage;

      GameLogicComponent* comp = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName( GameLogicComponent::DEFAULT_NAME, comp );
      if (comp != NULL)
      {
         int difficulty = comp->GetGameDifficulty(); // 0 = minimal, 1 = normal, 2 = hard
         if(difficulty == 0)
         {
            result *= 0.25f;
         }
         else if(difficulty == 1)
         {
            result *= difficulty * 0.35f;
         }
         else
         {
            result *= 0.5;
         }
      }

      return result;
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::OnEnteredWorld()
   {
      EnsureResourcesAreLoaded();

      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();
      physObj->SetTransform(ourTransform);
      physObj->Create(&GetScaleMatrixTransform());

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
         stateFindTarget->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &TowerActor::FindTarget));

         //redirecting the shoot 
         dtAI::NPCState* stateFireLaser = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIRE_LASER);
         stateFireLaser->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &TowerActor::Shoot));

         //redirecting the idle to our sleep function
         dtAI::NPCState* stateIdle = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_IDLE);
         stateIdle->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &TowerActor::Sleep));


         //calling spawn will start the AI
         mAIHelper->Spawn();

         // Setup the tower's weapon.
         InitWeapon();
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
   void TowerActor::OnRemovedFromWorld()
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::InitWeapon()
   {
      dtCore::RefPtr<SimCore::ActComps::WeaponInventoryActComp> weaponInv;
      GetComponent(weaponInv);

      if (!weaponInv.valid())
      {
         weaponInv = new SimCore::ActComps::WeaponInventoryActComp;
         AddComponent(*weaponInv);
      }

      dtCore::RefPtr<SimCore::ActComps::WeaponInventoryActComp::WeaponDescription> wd = new SimCore::ActComps::WeaponInventoryActComp::WeaponDescription;

      wd->SetWeaponPrototypeName("Weapon_MachineGun");
      wd->SetShooterPrototypeName("Particle_System_Weapon_GunWithTracer");
      wd->SetFiringParticleSystem(dtCore::ResourceDescriptor("Particles:weapon_gun_flash.osg"));
      wd->SetWeaponSwapRootNode("dof_gun_01");

      dtCore::RefPtr<SimCore::Actors::WeaponActor> weapon;

      weaponInv->CreateAndAddWeapon(*wd, true)->mWeapon->GetDrawable(weapon);

      //slow down the rate of fire
      weapon->SetFireRate(0.65f);

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::UpdateSoundEffects(float deltaTime)
   {
      BaseClass::UpdateSoundEffects(deltaTime);
   }

  ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum &damageState)
   {
      if (damageState != GetDamageState())
      {
         BaseClass::SetDamageState(damageState);

         // Mark the AI as 'dead' so we stop 'steering'
         if(damageState == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
         {
            mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DIE);

            GetComponent<SimCore::ActComps::WeaponInventoryActComp>()->StopFiring();
         }
      }
   }


   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::FindTarget(float)
   {
      float minDist = 250.0;
      dtCore::Transformable* enemy = NULL;

      std::vector<dtCore::ActorProxy*> actorArray;
      GetGameActorProxy().GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE, actorArray);
      //GetGameActorProxy().GetGameManager()->FindActorsByType(*NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE, actorArray);

      while(!actorArray.empty())
      {
         dtCore::Transformable* curr = static_cast<dtCore::Transformable*>(actorArray.back()->GetDrawable());
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
         dtCore::Transformable* curr = static_cast<dtCore::Transformable*>(actorArray.back()->GetDrawable());
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
         mAIHelper->SetCurrentTarget(*enemy);
      }
      else
      {
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_NO_TARGET_FOUND);
      }

   }

   //////////////////////////////////////////////////////////////////////
   void TowerActor::SetTarget(const BaseEnemyActor* enemy)
   {
      mAIHelper->SetCurrentTarget(*enemy);
   }
   //////////////////////////////////////////////////////////////////////
   float TowerActor::GetDistance( const dtCore::Transformable& t ) const
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
   void TowerActor::Sleep(float dt)
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
   void TowerActor::Shoot(float)
   {
      GetComponent<SimCore::ActComps::WeaponInventoryActComp>()->StartFiring();

      //randomly search for better targets
      //temporary hack
      if(dtUtil::RandPercent() < 0.05)
      {
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TARGET_KILLED);
      }
   }

   //////////////////////////////////////////////////////////////////////
   void TowerActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
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


      GetComponent<SimCore::ActComps::WeaponInventoryActComp>()->StopFiring();
   }

   //////////////////////////////////////////////////////////////////////
   void TowerActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   TowerActorProxy::TowerActorProxy()
   {
      SetClassName("TowerActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActorProxy::BuildPropertyMap()
   {
      BaseClass::BuildPropertyMap();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   TowerActorProxy::~TowerActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActorProxy::CreateDrawable()
   {
      SetDrawable(*new TowerActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActorProxy::OnRemovedFromWorld()
   {
      BaseClass::OnRemovedFromWorld();

      TowerActor* drawable = NULL;
      GetDrawable(drawable);
      drawable->OnRemovedFromWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActorProxy::BuildActorComponents()
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

