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

#include "VehicleShield.h"

#ifdef AGEIA_PHYSICS
#include <HoverTargetActor.h>
#include <HoverVehiclePhysicsHelper.h>
#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaRaycastReport.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/batchisector.h>
#include <dtCore/keyboard.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/basemessages.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Components/MunitionsComponent.h>

#include <dtUtil/nodeprintout.h>

namespace DriverDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   HoverTargetActor ::HoverTargetActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mGoalLocation(10.0, 10.0, 10.0)
      , mTimeSinceKilled(0.0f)
      , mTimeSinceBorn(0.0f)
   {

      SetDefaultScale(osg::Vec3(2.0f, 2.0f, 2.0f));

      SetTimeForSendingDeadReckoningInfoOut(0.0f);
      SetTimesASecondYouCanSendOutAnUpdate(3.0f);

      SetPublishLinearVelocity(true);
      SetPublishAngularVelocity(true);

      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      //dtAgeiaPhysX::NxAgeiaPhysicsHelper * helper = new dtAgeiaPhysX::NxAgeiaPhysicsHelper(proxy);
      HoverTargetPhysicsHelper *helper = new HoverTargetPhysicsHelper(proxy);
      helper->SetBaseInterfaceClass(this);
      SetPhysicsHelper(helper);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverTargetActor::~HoverTargetActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActor::OnEnteredWorld()
   {

      // Create our vehicle with a starting position
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      osg::Vec3 startVec;
      if (!IsRemote()) // Local - we vary it's starting position.
      {
         startVec = GetPhysicsHelper()->GetVehicleStartingPosition();
         startVec[0] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[1] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[2] += dtUtil::RandFloat(0.0, 4.0);

         // Since we changed our starting position, update our visual actor, or it 'blips' for
         // one frame in the wrong place. Very ugly.
         ourTransform.SetTranslation(startVec[0], startVec[1], startVec[2]);
         SetTransform(ourTransform);

         SetEntityType("HoverTarget");

         // Make a semi-unique name.
         static int targetCounter = 0;
         targetCounter ++;
         SetName("Target " + dtUtil::ToString(targetCounter));
      }
      else // Remote -just grab position
      {
         startVec = ourTransform.GetTranslation();
      }

      // Create our physics object
      GetTargetPhysicsHelper()->CreateTarget(startVec, IsRemote());

      SimCore::Actors::BasePhysicsVehicleActor::OnEnteredWorld();

      // REMOTE - Finish initial startup conditions
      if(IsRemote())
      {
         // THIS LINE MUST BE AFTER Super::OnEnteredWorld()! Undo the kinematic flag on remote entities. Lets us
         // apply velocities to remote hover vehicles so that they will impact us and make us bounce back
         GetTargetPhysicsHelper()->GetPhysXObject()->clearBodyFlag(NX_BF_KINEMATIC);
      }
      // LOCAL - Finish initial startup conditions
      else
      {
         // Give it a boost upwards on creation.
         osg::Vec3 dir(0.0, 0.0, 2000.0);
         ApplyForce(dir);

         // Offset the Target Dir so that they spread out around the map.
         mGoalLocation[0] += dtUtil::RandFloat(-40.0, 40.0);
         mGoalLocation[1] += dtUtil::RandFloat(-50.0, 50.0);
         mGoalLocation[2] += dtUtil::RandFloat(2.0, 4.0);


         // Register a munitions component to the target so it can take damage
         SimCore::Components::MunitionsComponent* munitionsComp;
         GetGameActorProxy().GetGameManager()->GetComponentByName
            (SimCore::Components::MunitionsComponent::DEFAULT_NAME, munitionsComp);
         if( munitionsComp != NULL )
         {
            munitionsComp->Register(*this);
         }
      }

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActor::UpdateSoundEffects(float deltaTime)
   {
      SimCore::Actors::BasePhysicsVehicleActor::UpdateSoundEffects(deltaTime);
   }


  ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      if( ! IsMobilityDisabled())
      {
         GetTargetPhysicsHelper()->ApplyTargetHoverForces(deltaTime, mGoalLocation);
      }
      else
      {
         mTimeSinceKilled += deltaTime;
      }

      mTimeSinceBorn += deltaTime;

      // Delete the target after dead a while or just too 'old'
      if (mTimeSinceBorn > 120.0f || mTimeSinceKilled > 20.0f)
      {
         GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
      }
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   HoverTargetActorProxy::HoverTargetActorProxy()
   {
      SetClassName("HoverTargetActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActorProxy::BuildPropertyMap()
   {
      const std::string& VEHICLEGROUP   = "Vehicle Property Values";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      HoverTargetActor  &actor = static_cast<HoverTargetActor &>(GetGameActor());

   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverTargetActorProxy::~HoverTargetActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActorProxy::CreateActor()
   {
      SetActor(*new HoverTargetActor(*this));

      SimCore::Actors::BaseEntity* entityActor = dynamic_cast<SimCore::Actors::BaseEntity*> (GetActor());
      if( entityActor != NULL )
      {
         entityActor->InitDeadReckoningHelper();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActorProxy::OnEnteredWorld()
   {
      //RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

      SimCore::Actors::BasePhysicsVehicleActorProxy::OnEnteredWorld();
   }

} // namespace
#endif
