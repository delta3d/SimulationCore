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
#include <prefix/SimCorePrefix-src.h>

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
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/keyboard.h>
#include <dtGame/basemessages.h>
#include <SimCore/Components/RenderingSupportComponent.h>
//#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/DefaultFlexibleArticulationHelper.h>
#include <SimCore/CollisionGroupEnum.h>

//#include <dtUtil/nodeprintout.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>

#include <ActorRegistry.h>
#include <AIState.h>
#include <Actors/FortActor.h>
#include <Actors/EnemyHelix.h>
#include <Actors/EnemyMine.h>




namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   TowerActor::TowerActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   {
      SetMaxUpdateSendRate(2.0f);

      SetPublishLinearVelocity(false);
      SetPublishAngularVelocity(false);

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
   void TowerActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      dtPhysics::PhysicsObject *physObj = GetPhysicsHelper()->GetMainPhysicsObject();
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
      }

      BaseClass::OnEnteredWorld();

      // Add a dynamic light to our fort
      SimCore::Components::RenderingSupportComponent* renderComp;
      GetGameActorProxy().GetGameManager()->GetComponentByName(
         SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);
      if(renderComp != NULL)
      {
         //Add a spot light
         SimCore::Components::RenderingSupportComponent::DynamicLight* sl =
            new SimCore::Components::RenderingSupportComponent::DynamicLight();
         sl->mRadius = 30.0f;
         sl->mIntensity = 1.0f;
         sl->mColor.set(1.0f, 1.0f, 1.0f);
         sl->mAttenuation.set(0.001, 0.004, 0.0002);
         sl->mTarget = this;
         sl->mAutoDeleteLightOnTargetNull = true;
         renderComp->AddDynamicLight(sl);
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
   void TowerActor::FindTarget(float)
   {
      float minDist = 200.0;
      BaseEnemyActor* enemy = NULL;

      EnemyMineActorProxy* mineProxy = NULL;
      GetGameActorProxy().GetGameManager()->FindActorByType(*NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE, mineProxy);
      if (mineProxy != NULL)
      {
         EnemyMineActor& mine = *static_cast<EnemyMineActor*>(mineProxy->GetActor());

         float dist = GetDistance(mine);

         if(dist < minDist)
         {
            enemy = &mine;
         }
      }

      if(enemy == NULL)
      {
         EnemyHelixActorProxy* helixProxy = NULL;
         GetGameActorProxy().GetGameManager()->FindActorByType(*NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE, helixProxy);
         if (helixProxy != NULL)
         {
            EnemyHelixActor& helix = *static_cast<EnemyHelixActor*>(helixProxy->GetActor());

            float dist = GetDistance(helix);

            if(dist < minDist)
            {
               enemy = &helix;
            }
         }
      }

      if(enemy != NULL)
      {
         mAIHelper->SetCurrentTarget(*enemy);
      }

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

      //dtCore::Transform xform;
      //osg::Vec3 enemyPos;

      //mTarget.GetTransform(xform);
      //xform.GetTranslation(enemyPos);

      //dtUtil::NodeCollector *nodes = GetNodeCollector();
      //osgSim::DOFTransform *dof = nodes->GetDOFTransform("dof_turret_01");
      //if (dof != NULL)
      //{
      //   // Spin the turret in a circle every few seconds
      //   osg::Vec3 hpr = dof->getCurrentHPR() * 57.29578;
      //   osg::Vec3 hprChange;
      //   hprChange[0] = 60.0f * tickMessage.GetDeltaSimTime();
      //   hpr[0] += hprChange[0];
      //   dof->setCurrentHPR(hpr * 0.0174533); // convert degrees to radians
      //   // Let the artics decide if the actor is dirty or not
      //   if(GetArticulationHelper() != NULL)
      //   {
      //      GetArticulationHelper()->HandleUpdatedDOFOrientation(*dof, hprChange, hpr);
      //   }
      //}

   }

   //////////////////////////////////////////////////////////////////////
   void TowerActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

      //Tick the AI
      //update the AI's position and orientation
      dtCore::Transform trans;
      GetTransform(trans);
      mAIHelper->PreSync(trans);

      ////////let the AI do its thing
      mAIHelper->Update(tickMessage.GetDeltaSimTime());

      //dtUtil::NodeCollector *nodes = GetNodeCollector();
      //osgSim::DOFTransform *dof = nodes->GetDOFTransform("dof_turret_01");
      //if (dof != NULL)
      //{
      //   // Spin the turret in a circle every few seconds
      //   osg::Vec3 hpr = dof->getCurrentHPR() * 57.29578;
      //   osg::Vec3 hprChange;
      //   hprChange[0] = 60.0f * tickMessage.GetDeltaSimTime();
      //   hpr[0] += hprChange[0];
      //   dof->setCurrentHPR(hpr * 0.0174533); // convert degrees to radians
      //   // Let the artics decide if the actor is dirty or not
      //   if(GetArticulationHelper() != NULL)
      //   {
      //      GetArticulationHelper()->HandleUpdatedDOFOrientation(*dof, hprChange, hpr);
      //   }
      //}
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
      TowerActor& actor = *static_cast<TowerActor*>(GetActor());
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

} // namespace
//#endif
