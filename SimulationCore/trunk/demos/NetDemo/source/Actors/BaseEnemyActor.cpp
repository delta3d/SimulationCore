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
*/
#include <prefix/SimCorePrefix-src.h>

//#ifdef AGEIA_PHYSICS
#include <Actors/BaseEnemyActor.h>
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
#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/CollisionGroupEnum.h>

//#include <dtUtil/nodeprintout.h>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   BaseEnemyActor::BaseEnemyActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mTimeSinceBorn(0.0f)
      , mTimeToExistAfterDead(20.0f)
      , mTimeSinceKilled(0.0f)
   {
      /////////////////////////////////////////////////////////////////
      // Set a bunch of initial conditions - Note, some of these may be 
      // changed if the actor is loaded from a map or when received as a remote actor
      /////////////////////////////////////////////////////////////////

      SetTimeForSendingDeadReckoningInfoOut(0.0f);
      SetTimesASecondYouCanSendOutAnUpdate(2.0f);

      SetPublishLinearVelocity(true);
      SetPublishAngularVelocity(true);

      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      dtPhysics::PhysicsHelper *helper = new dtPhysics::PhysicsHelper(proxy);
      //helper->SetBaseInterfaceClass(this);
      SetPhysicsHelper(helper);

      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("VehicleBody");
      helper->AddPhysicsObject(*physicsObject);
      helper->SetMass(500.0f);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
      physicsObject->SetMass(100.0f);
      //physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::STATIC);

      // Preset some Entity values
      SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OPPOSING); // Shows up 'red' in Stealth Viewer
      SetEntityType("Enemy");
      SetMunitionDamageTableName("StandardDamageType");

      // Make a semi-unique name - mostly only useful for Stealth Viewer
      static int uniqueCounter = 0;
      uniqueCounter ++;
      SetName("Enemy " + dtUtil::ToString(uniqueCounter));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   BaseEnemyActor::~BaseEnemyActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      //GetHoverPhysicsHelper()->CreateVehicle(ourTransform,
      //   GetNodeCollector()->GetDOFTransform("dof_chassis"));
      //dtPhysics::PhysicsObject *physObj = GetHoverPhysicsHelper()->GetMainPhysicsObject();
      ////////dtPhysics::PhysicsObject* physActor = GetPhysicsHelper()->GetPhysXObject();

      // TODO - Maybe use sphere instead???

      dtPhysics::PhysicsObject *physObj = GetPhysicsHelper()->GetMainPhysicsObject();
      physObj->CreateFromProperties(GetNonDamagedFileNode());
      physObj->SetTransform(ourTransform);
      physObj->SetActive(true);

      if(!IsRemote())
      {
         // Register a munitions component so the fort can take damage
         SimCore::Components::MunitionsComponent* munitionsComp;
         GetGameActorProxy().GetGameManager()->GetComponentByName
            (SimCore::Components::MunitionsComponent::DEFAULT_NAME, munitionsComp);
         if( munitionsComp != NULL )
         {
            munitionsComp->Register(*this, true, GetMaxDamageAmount());
         }

         // Setup our articulation helper for the vehicle
         //dtCore::RefPtr<SimCore::Components::DefaultFlexibleArticulationHelper> articHelper = 
         //   new SimCore::Components::DefaultFlexibleArticulationHelper();
         //articHelper->SetEntity(this);
         //articHelper->AddArticulation("dof_turret_01", 
         //   SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_HEADING);
         //articHelper->AddArticulation("dof_gun_01", 
         //   SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_ELEVATION, "dof_turret_01");
         //SetArticulationHelper(articHelper.get());
      }

      BaseClass::OnEnteredWorld();

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      mTimeSinceBorn += deltaTime;

      if( ! IsMobilityDisabled())
      {
         // Do physics/pathing/AI of vehicle
      }
      else
      {
         // Self-terminate after we're been dead for a bit - allows time for animations, fire, falling from physics, etc...
         mTimeSinceKilled += deltaTime;
         if (mTimeSinceKilled > mTimeToExistAfterDead)
         {
            GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
         }
      }

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::UpdateSoundEffects(float deltaTime)
   {
      BaseClass::UpdateSoundEffects(deltaTime);
   }

  ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   //////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );
   }

   //////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   BaseEnemyActorProxy::BaseEnemyActorProxy()
   {
      SetClassName("BaseEnemyActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActorProxy::BuildPropertyMap()
   {
      const std::string GROUP = "Enemy Props";
      BaseClass::BuildPropertyMap();

      BaseEnemyActor& actor = *static_cast<BaseEnemyActor*>(GetActor());

      // Add properties
   }

   ///////////////////////////////////////////////////////////////////////////////////
   BaseEnemyActorProxy::~BaseEnemyActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActorProxy::CreateActor()
   {
      SetActor(*new BaseEnemyActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
//#endif
