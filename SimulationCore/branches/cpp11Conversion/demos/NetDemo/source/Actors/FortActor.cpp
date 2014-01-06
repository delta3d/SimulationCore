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
* @author Curtiss Murphy
* @author Bradley Anderegg
*/

#include <prefix/SimCorePrefix.h>

//#ifdef AGEIA_PHYSICS
#include <Actors/FortActor.h>
#include <dtPhysics/physicsactcomp.h>
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
#include <SimCore/Components/VolumeRenderingComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/DefaultFlexibleArticulationHelper.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Actors/BaseEntity.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/Actors/MunitionTypeActor.h>


//#include <dtUtil/nodeprintout.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>

#include <Components/GameLogicComponent.h>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   FortActor::FortActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mTimeSinceLightsWereUpdated(0.0f)
	  , mLightIsOn(true)
   {
      SetTerrainPresentDropHeight(0.0);

      SetEntityType("Fort");
      SetMunitionDamageTableName("StandardDamageType");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   FortActor::~FortActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActor::OnEnteredWorld()
   {
      EnsureResourcesAreLoaded();

      dtCore::Transform ourTransform;
      GetTransform(ourTransform);


      dtPhysics::PhysicsObject *physObj = GetPhysicsActComp()->GetMainPhysicsObject();
      physObj->SetTransform(ourTransform);
      physObj->CreateFromProperties(&GetScaleMatrixTransform());

      if(!IsRemote())
      {

         // Setup our articulation helper for the vehicle
         std::shared_ptr<SimCore::Components::DefaultFlexibleArticulationHelper> articHelper =
            new SimCore::Components::DefaultFlexibleArticulationHelper();
         articHelper->SetEntity(this);
         articHelper->AddArticulation("dof_turret_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_HEADING);
         articHelper->AddArticulation("dof_gun_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_ELEVATION, "dof_turret_01");
         SetArticulationHelper(articHelper.get());
      }

      BaseClass::OnEnteredWorld();

      // Add a dynamic light to our fort
      SimCore::Components::RenderingSupportComponent* renderComp;
      GetGameActorProxy().GetGameManager()->GetComponentByName(
         SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);
      if(renderComp != nullptr)
      {
         //Add a spot light
         mMainLight = new SimCore::Components::RenderingSupportComponent::DynamicLight();
         
         mMainLight->mRadius = 150.0f;
         mMainLight->mIntensity = 1.0f;
         mMainLight->mColor.set(1.0f, 1.0f, 1.0f);
         mMainLight->mAttenuation.set(0.001, 0.00035, 0.0008);
         mMainLight->mTarget = this;
         mMainLight->mAutoDeleteLightOnTargetNull = true;
         renderComp->AddDynamicLight(mMainLight.get());
      }

      //add a shape volume for the beam
      SimCore::Components::VolumeRenderingComponent* vrc = nullptr;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 
      
      if(vrc != nullptr)
      {
         SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord* svr = new SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord();
         svr->mPosition.set(1.0f, 30.0f, 1.0f);
         svr->mColor.set(1.0f, 1.0f, 1.0f, 1.0f);
         svr->mShapeType = SimCore::Components::VolumeRenderingComponent::ELLIPSOID;
         svr->mRadius.set(30.0f, 30.0f, 2.5f);
         svr->mNumParticles = 150;
         svr->mParticleRadius = 8.0f;
         svr->mVelocity = 0.07;
         svr->mDensity = 0.15f;
         svr->mTarget = GetOSGNode();
         svr->mAutoDeleteOnTargetNull = true;
         svr->mRenderMode = SimCore::Components::VolumeRenderingComponent::PARTICLE_VOLUME;

         vrc->CreateShapeVolume(svr);
      } 
   }

   ///////////////////////////////////////////////////////////////////////////////////
   float FortActor::ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message, 
      const SimCore::Actors::MunitionTypeActor& munition)
   {
      //dtGame::GameActorProxy* gap = GetGameActorProxy().GetGameManager()->FindGameActorById(message.GetSendingActorId());
      //return incomingDamage * float(!IsEnemyActor(gap));
      float result = incomingDamage;

      GameLogicComponent* comp = nullptr;
      GetGameActorProxy().GetGameManager()->GetComponentByName( GameLogicComponent::DEFAULT_NAME, comp );
      if (comp != nullptr)
      {
         int difficulty = comp->GetGameDifficulty(); // 0 = minimal, 1 = normal, 2 = hard
         if(difficulty == 0)
         {
            result *= 0.5f;
         }
         else if(difficulty == 1)
         {
            result *= difficulty * 1.5f;
         }
         else
         {
            result *= 2.0;
         }
      }

      return result;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void FortActor::RespondToHit(const SimCore::DetonationMessage& message,
      const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force,
      const osg::Vec3& location)
   {
      // An opportunity to respond after damage was applied to local entities
      BaseClass::RespondToHit(message, munition, force, location);

      // Baseentity sends an update if our damage state changes, but not if we 
      // just take a minor amount of damage. So, we do that here.
      std::vector<dtUtil::RefString> propNames;
      propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_DAMAGE_STATE);
      propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_CUR_DAMAGE_RATIO);
      GetGameActorProxy().NotifyPartialActorUpdate(propNames);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActor::UpdateSoundEffects(float deltaTime)
   {
      BaseClass::UpdateSoundEffects(deltaTime);
   }

  ///////////////////////////////////////////////////////////////////////////////////
   void FortActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   //////////////////////////////////////////////////////////////////////
   void FortActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

      dtUtil::NodeCollector *nodes = GetNodeCollector();
      osgSim::DOFTransform *dof = nodes->GetDOFTransform("dof_turret_01");
      if (dof != nullptr)
      {
         // Spin the turret in a circle every few seconds
         osg::Vec3 hpr = dof->getCurrentHPR() * 57.29578;
         osg::Vec3 hprChange;
         hprChange[0] = 60.0f * tickMessage.GetDeltaSimTime();
         hpr[0] += hprChange[0];
         dof->setCurrentHPR(hpr * 0.0174533); // convert degrees to radians
         // Let the artics decide if the actor is dirty or not
         if(GetArticulationHelper() != nullptr)
         {
            GetArticulationHelper()->HandleUpdatedDOFOrientation(*dof, hprChange, hpr);
         }
      }

      if(mLightIsOn && mMainLight.valid() && GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
      {
         mLightIsOn = false;

         SimCore::Components::RenderingSupportComponent* rsc = nullptr;
         GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsc);

         if(rsc != nullptr)
         {
            rsc->RemoveDynamicLight(mMainLight->GetId());
         }
      }

	  //update which dynamic lights we are using
      mTimeSinceLightsWereUpdated += tickMessage.GetDeltaSimTime();
      if(mTimeSinceLightsWereUpdated > 1.0f)
      {
		 SimCore::Components::RenderingSupportComponent* rsComp = nullptr;

         GetGameActorProxy().GetGameManager()->GetComponentByName( SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsComp);

         if(rsComp != nullptr)
         {
            rsComp->FindBestLights(*this);
         }

         mTimeSinceLightsWereUpdated = 0.0f;
	  }
   }

   //////////////////////////////////////////////////////////////////////
   void FortActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   FortActorProxy::FortActorProxy()
   {
      SetClassName("FortActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActorProxy::BuildPropertyMap()
   {
      const std::string VEH_GROUP   = "Fort Values";
      BaseClass::BuildPropertyMap();

//      FortActor* actor = nullptr;
//      GetActor(actor);

      // Add properties
   }

   ///////////////////////////////////////////////////////////////////////////////////
   FortActorProxy::~FortActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActorProxy::CreateDrawable()
   {
      SetDrawable(*new FortActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActorProxy::BuildActorComponents()
   {
      BaseClass::BuildActorComponents();


      dtPhysics::PhysicsActComp* physAC = nullptr;
      GetComponent(physAC);
      // Add our initial body.
      std::shared_ptr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("VehicleBody");
      physAC->AddPhysicsObject(*physicsObject);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
      physicsObject->SetMass(30000.0f);
      //physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::STATIC);


      dtGame::DRPublishingActComp* drPublishingActComp = nullptr;
      GetComponent(drPublishingActComp);
      if (drPublishingActComp == nullptr)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(2.0f);
      //drPublishingActComp->SetPublishLinearVelocity(false);
      //drPublishingActComp->SetPublishAngularVelocity(false);
   }

} // namespace
//#endif
