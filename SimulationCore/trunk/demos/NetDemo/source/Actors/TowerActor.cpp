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
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/DRPublishingActComp.h>
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
#include <Components/WeaponComponent.h>

//for debug printouts
#include <iostream>


namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   TowerActor::TowerActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   {
      SetTerrainPresentDropHeight(0.0);

      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      dtPhysics::PhysicsHelper* helper = new dtPhysics::PhysicsHelper(proxy);
      //helper->SetBaseInterfaceClass(this);
      SetPhysicsHelper(helper);

      // Add our initial body.
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("VehicleBody");
      helper->AddPhysicsObject(*physicsObject);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
      physicsObject->SetMass(30000.0f);
      //physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::STATIC);

      SetEntityType("Fort");
      SetMunitionDamageTableName("StandardDamageType");

      mAIHelper = new TowerAIHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   TowerActor::~TowerActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::BuildActorComponents()
   {
      BaseClass::BuildActorComponents();

      SimCore::Actors::DRPublishingActComp* drPublishingActComp = GetDRPublishingActComp();
      if (drPublishingActComp == NULL)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(1.5f);
      //drPublishingActComp->SetPublishLinearVelocity(false);
      //drPublishingActComp->SetPublishAngularVelocity(false);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      dtPhysics::PhysicsObject* physObj = GetPhysicsHelper()->GetMainPhysicsObject();
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
         mAIHelper->GetPhysicsModel()->SetPhysicsHelper(GetPhysicsHelper());

         //redirecting the find target function
         dtAI::NPCState* state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIND_TARGET);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &TowerActor::FindTarget));

         //redirecting the shoot 
         state = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_FIRE_LASER);
         state->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &TowerActor::Shoot));

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
      if(mWeaponProxy.valid())
      {
         GetGameActorProxy().GetGameManager()->DeleteActor(*mWeaponProxy);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::InitWeapon()
   {
      // Get the weapon component that is used to create weapons.
      WeaponComponent* weaponComp = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(WeaponComponent::DEFAULT_NAME, weaponComp);

      if(weaponComp != NULL)
      {
         SimCore::Actors::WeaponActor* weapon = NULL;

         // Create the primary weapon.
         // --- This method will automatically add the created weapon to the world.
         weaponComp->CreateWeapon("Weapon_MachineGun",
            "Particle_System_Weapon_GunWithTracer",
            "weapon_gun_flash.osg", weapon);

         // Customize the new weapon.
         if(weapon != NULL)
         {
            // Attach the weapon to this object.
            weapon->SetOwner(&GetGameActorProxy());
            AddChild(weapon, "dof_gun_01");

            // Maintain references to the weapon and its proxy.
            mWeapon = weapon;
            mWeaponProxy = static_cast<SimCore::Actors::WeaponActorProxy*>(&weapon->GetGameActorProxy());

            mWeapon->SetFireRate(0.333f);
         }
      }
      else
      {
         LOG_ERROR("Could not find Weapon Component to create weapon.");
      }
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
         if(IsMobilityDisabled())
         {
            mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DIE);

            if (mWeapon.valid())
            {
               mWeapon->SetTriggerHeld(false);
            }
         }
      }
   }


   ///////////////////////////////////////////////////////////////////////////////////
   void TowerActor::FindTarget(float)
   {
      float minDist = 250.0;
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
         mAIHelper->SetCurrentTarget(*enemy);
      }

      //mAIHelper->SetCurrentTarget(*GetGameActorProxy().GetGameManager()->GetApplication().GetCamera());

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
   void TowerActor::Shoot(float)
   {
      if(mWeapon.valid())
      {
         mWeapon->SetTriggerHeld(true);
         mWeapon->Fire();
      }

      //randomly search for better targets
      //temporary hack
      if(dtUtil::RandPercent() < 0.1)
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


      if (mWeapon.valid())
      {
         mWeapon->SetTriggerHeld(false);
      }
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
   void TowerActorProxy::CreateActor()
   {
      SetActor(*new TowerActor(*this));
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

      TowerActor* actor = NULL;
      GetActor(actor);      
      actor->OnRemovedFromWorld();
   }

} // namespace
//#endif

