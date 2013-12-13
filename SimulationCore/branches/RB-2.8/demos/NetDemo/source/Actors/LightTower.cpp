/* -*-c++-*-
* Using 'The MIT License'
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
* @author Bradley Anderegg
*/

#include <prefix/SimCorePrefix.h>

//#ifdef AGEIA_PHYSICS
#include <Actors/LightTower.h>
#include <dtPhysics/physicsactcomp.h>
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

//for debug printouts
#include <iostream>


namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   LightTower::LightTower(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mSleepTime(0.0f)
      , mMaxSleepTime(2.0f)
      , mTargetLight(new SimCore::Components::RenderingSupportComponent::SpotLight())
      , mMainLight(new SimCore::Components::RenderingSupportComponent::DynamicLight())
   {
      SetTerrainPresentDropHeight(0.0);

      SetEntityType("Fort");
      SetMunitionDamageTableName("StandardDamageType");

      mAIHelper = new TowerAIHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   LightTower::~LightTower(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTower::CreateLights()
   {
      SimCore::Components::RenderingSupportComponent* rsc = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsc);

      if(rsc != NULL)
      {
         mTargetLight->mTarget = this;
         mTargetLight->mIntensity = 0.0f;        
         mTargetLight->mAttenuation.set(0.000125f, 0.000025f, 0.000125f);
         mTargetLight->mColor.set(0.85f, 0.75f, 0.7f);
         mTargetLight->mRadius = 25.0f;
         mTargetLight->mFlicker = false;
         mTargetLight->mUseAbsoluteDirection = true;
         mTargetLight->mDirection.set(0.0f, 0.0f, -1.0f);
         mTargetLight->mSpotExponent = 5.0f;
         mTargetLight->mSpotCosCutoff = 0.975f;
         rsc->AddDynamicLight(mTargetLight);


         mMainLight->mRadius = 15.0f;
         mMainLight->mIntensity = 1.0f;
         mMainLight->mColor.set(1.0f, 1.0f, 1.0f);
         mMainLight->mAttenuation.set(0.0025, 0.0025, 0.0025);
         mMainLight->mTarget = this;
         mMainLight->mAutoDeleteLightOnTargetNull = true;
         rsc->AddDynamicLight(mMainLight);
      }
   }

   //////////////////////////////////////////////////////////////////////
   void LightTower::EnableSpotLight(bool b)
   {
      if(b)
      {
         mTargetLight->mIntensity = 1.0f;
         mMainLight->mIntensity = 0.0f;
      }
      else
      {
         mTargetLight->mIntensity = 0.0f;
         mMainLight->mIntensity = 0.5f;
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTower::OnEnteredWorld()
   {
      EnsureResourcesAreLoaded();

      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();
      physObj->SetTransform(ourTransform);
      physObj->CreateFromProperties(&GetScaleMatrixTransform());

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
         stateFindTarget->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &LightTower::FindTarget));

         //redirecting the idle to our sleep function
         dtAI::NPCState* stateIdle = mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_IDLE);
         stateIdle->SetUpdate(dtAI::NPCState::UpdateFunctor(this, &LightTower::Sleep));


         //here we set up the state machine to automatically turn on and off the spot light
         typedef dtUtil::Command1<void, bool> BooleanCommand;
         typedef dtUtil::Functor<void, TYPELIST_1(bool)> BooleanFunctor;    

         BooleanCommand* cmd_EnableSpotLight = new BooleanCommand(BooleanFunctor(this, &LightTower::EnableSpotLight), true);
         BooleanCommand* cmd_DisableSpotLight = new BooleanCommand(BooleanFunctor(this, &LightTower::EnableSpotLight), false);

         stateIdle->AddEntryCommand(cmd_DisableSpotLight);         
         mAIHelper->GetStateMachine().GetState(&AIStateType::AI_STATE_ATTACK)->AddEntryCommand(cmd_EnableSpotLight);

         //calling spawn will start the AI
         mAIHelper->Spawn();
      }

      // Attach a special shader.
      dtCore::RefPtr<SimCore::ApplyShaderVisitor> visitor = new SimCore::ApplyShaderVisitor();
      visitor->AddNodeName("Eye360");
      visitor->SetShaderName("ColorPulseShader");
      visitor->SetShaderGroup("CustomizableVehicleShaderGroup");
      GetOSGNode()->accept(*visitor);

      CreateLights();


      BaseClass::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTower::OnRemovedFromWorld()
   {

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTower::UpdateSoundEffects(float deltaTime)
   {
      BaseClass::UpdateSoundEffects(deltaTime);
   }

  ///////////////////////////////////////////////////////////////////////////////////
   void LightTower::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTower::SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum &damageState)
   {
      if (damageState != GetDamageState())
      {
         BaseClass::SetDamageState(damageState);

         // Mark the AI as 'dead' so we stop 'steering'
         if(IsMobilityDisabled())
         {
            SimCore::Components::RenderingSupportComponent* rsc = NULL;
            GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsc);

            if(rsc != NULL)
            {
               rsc->RemoveDynamicLight(mMainLight->GetId());
               rsc->RemoveDynamicLight(mTargetLight->GetId());
            }

            mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DIE);
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTower::Sleep(float dt)
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
   void LightTower::FindTarget(float)
   {
      float minDist = 250.0;
      dtCore::Transformable* enemy = NULL;

      std::vector<dtDAL::ActorProxy*> actorArray;
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
   void LightTower::SetTarget(const BaseEnemyActor* enemy)
   {
      mAIHelper->SetCurrentTarget(*enemy);
   }
   //////////////////////////////////////////////////////////////////////
   float LightTower::GetDistance( const dtCore::Transformable& t ) const
   {

      dtCore::Transform xform;
      osg::Vec3 pos, enemyPos;
      GetTransform(xform);
      xform.GetTranslation(pos);

      t.GetTransform(xform);
      xform.GetTranslation(enemyPos);

      return (enemyPos - pos).length();
   }

   //////////////////////////////////////////////////////////////////////
   void LightTower::OnTickLocal( const dtGame::TickMessage& tickMessage )
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
            dtUtil::Clamp(hpr[1], osg::DegreesToRadians(-10.0f), osg::DegreesToRadians(10.0f));
            dof->setCurrentHPR(hpr);

            if(GetArticulationHelper() != NULL)
            {
               GetArticulationHelper()->HandleUpdatedDOFOrientation(*dof, hpr - hprLast, hpr);
            }
         }

         hpr[0] = osg::RadiansToDegrees(hpr[0]);
         hpr[1] = osg::RadiansToDegrees(hpr[1]);
         hpr[2] = osg::RadiansToDegrees(hpr[2]);

         osg::Matrix mat;
         dtUtil::MatrixUtil::HprToMatrix(mat, hpr);
         osg::Vec3 dir = dtUtil::MatrixUtil::GetRow3(mat, 1);

         mTargetLight->mDirection.set(dir);
      }

      if((mAIHelper->GetStateMachine().GetCurrentState()->GetType() == &AIStateType::AI_STATE_FIRE_LASER) && (dtUtil::RandPercent() < 0.05))
      {
         mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TARGET_KILLED);
      }
   }

   //////////////////////////////////////////////////////////////////////
   void LightTower::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );

      //orient the light
      dtUtil::NodeCollector* nodes = GetNodeCollector();
      osgSim::DOFTransform* dof = nodes->GetDOFTransform("dof_turret_01");
      if (dof != NULL && mTargetLight != NULL)
      {
         osg::Vec3 hpr, hprLast;
         hpr = dof->getCurrentHPR();

         hpr[0] = osg::RadiansToDegrees(hpr[0]);
         hpr[1] = osg::RadiansToDegrees(hpr[1]);
         hpr[2] = osg::RadiansToDegrees(hpr[2]);

         osg::Matrix mat;
         dtUtil::MatrixUtil::HprToMatrix(mat, hpr);
         osg::Vec3 dir = dtUtil::MatrixUtil::GetRow3(mat, 1);

         mTargetLight->mDirection.set(dir);
      }

   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   LightTowerProxy::LightTowerProxy()
   {
      SetClassName("LightTower");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTowerProxy::BuildPropertyMap()
   {
      BaseClass::BuildPropertyMap();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   LightTowerProxy::~LightTowerProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTowerProxy::CreateDrawable()
   {
      SetDrawable(*new LightTower(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTowerProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void LightTowerProxy::OnRemovedFromWorld()
   {
      BaseClass::OnRemovedFromWorld();

      LightTower* actor = NULL;
      GetActor(actor);      
      actor->OnRemovedFromWorld();
   }

   void LightTowerProxy::BuildActorComponents()
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

