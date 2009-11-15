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


namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   TowerActor::TowerActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   {
      SetMaxUpdateSendRate(2.0f);

      SetPublishLinearVelocity(false);
      SetPublishAngularVelocity(false);

      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      dtPhysics::PhysicsHelper *helper = new dtPhysics::PhysicsHelper(proxy);
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
      physObj->CreateFromProperties(GetNonDamagedFileNode());
      physObj->SetTransform(ourTransform);
      physObj->SetActive(true);

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

   //////////////////////////////////////////////////////////////////////
   void TowerActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

      dtUtil::NodeCollector *nodes = GetNodeCollector();
      osgSim::DOFTransform *dof = nodes->GetDOFTransform("dof_turret_01");
      if (dof != NULL)
      {
         // Spin the turret in a circle every few seconds
         osg::Vec3 hpr = dof->getCurrentHPR() * 57.29578;
         osg::Vec3 hprChange;
         hprChange[0] = 60.0f * tickMessage.GetDeltaSimTime();
         hpr[0] += hprChange[0];
         dof->setCurrentHPR(hpr * 0.0174533); // convert degrees to radians
         // Let the artics decide if the actor is dirty or not
         if(GetArticulationHelper() != NULL)
         {
            GetArticulationHelper()->HandleUpdatedDOFOrientation(*dof, hprChange, hpr);
         }
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