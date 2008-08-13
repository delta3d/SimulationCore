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
      , mVehicleBaseWeight(100.0f)
      , mSphereRadius(1.0f)
      , mGroundClearance(4.0f)
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
      //HoverTargetPhysicsHelper *helper = new HoverTargetPhysicsHelper(proxy);
      dtAgeiaPhysX::NxAgeiaPhysicsHelper * helper = new dtAgeiaPhysX::NxAgeiaPhysicsHelper(proxy);
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

      //std::cout << "Target entered world. IsRemote() is [" << IsRemote() << std::endl;
      //dtCore::RefPtr<dtUtil::NodePrintOut> nodePrinter = new dtUtil::NodePrintOut();
      //std::string nodes = nodePrinter->CollectNodeData(*GetNonDamagedFileNode());
      //std::cout << " --------- NODE PRINT OUT FOR HOVER VEHICLE --------- " << std::endl;
      //std::cout << nodes.c_str() << std::endl;

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
      }
      else // Remote -just grab position
      {
         startVec = ourTransform.GetTranslation();
      }


      // Create our Physics Sphere!
      NxVec3 startPos(startVec[0], startVec[1],startVec[2]);
      GetPhysicsHelper()->SetCollisionSphere(startPos, 1.0, 0, 
         mVehicleBaseWeight, 0, "Default", "Default", false);
      NxActor *physActor = GetPhysicsHelper()->GetPhysXObject();
      if (physActor == NULL)
      {
         throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_ACTOR_STATE,
            "Critical Error! Failed to create the Physics Object during creation.", __FILE__, __LINE__);
      }

      // Reorient physics to our Y is forward system.
      NxMat33 orient;
      orient.setRow(0, NxVec3(1,0,0));
      orient.setRow(1, NxVec3(0,0,-1));
      orient.setRow(2, NxVec3(0,1,0));
      GetPhysicsHelper()->SwitchCoordinateSystem(orient);

      // REMOTE - Configure Physics
      if(!IsRemote())
      {
         GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_GET_COLLISION_REPORT | 
            dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
         //GetHoverPhysicsHelper()->TurnObjectsGravityOff("Default");
      }
      // LOCAL - Configure Physics
      //else // -- Flags set in the base class.
      //GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
      //GetHoverPhysicsHelper()->SetAgeiaUserData(mPhysicsHelper.get());

      SimCore::Actors::BasePhysicsVehicleActor::OnEnteredWorld();

      // REMOTE - Finish initial startup conditions
      if(IsRemote())
      {
         // THIS LINE MUST BE AFTER Super::OnEnteredWorld()! Undo the kinematic flag on remote entities. Lets us 
         // apply velocities to remote hover vehicles so that they will impact us and make us bounce back
         physActor->clearBodyFlag(NX_BF_KINEMATIC);

         // Add the swirly shield to remote vehicles.
         //mShield = new VehicleShield();
         //mShield->SetTranslation(osg::Vec3(0.0f, 0.0f, 0.5f));
         //AddChild(mShield.get());
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
         deltaTime = (deltaTime > 0.2) ? 0.2 : deltaTime;  // cap at 0.2 second to avoid rare 'freak' outs.
         float weight = GetVehicleBaseWeight();

         // First thing we do is try to make sure we are hovering...
         NxActor* physicsObject = GetPhysicsHelper()->GetPhysXObject();  // Tick Local protects us from NULL.
         NxVec3 velocity = physicsObject->getLinearVelocity();
         NxVec3 pos = physicsObject->getGlobalPosition();
         NxVec3 posLookAhead = pos + velocity * 0.5; // where would our vehicle be in .5 seconds? 


         // Adjust position so that we are 'hovering' above the ground. The look ahead position
         // massively helps smooth out the bouncyness
         osg::Vec3 location(pos.x, pos.y, pos.z);
         osg::Vec3 locationLookAhead(posLookAhead.x, posLookAhead.y, posLookAhead.z);
         osg::Vec3 direction( 0.0f, 0.0f, -1.0f);
         float distanceToHit = 0.0;
         float futureAdjustment = ComputeEstimatedForceCorrection(locationLookAhead, direction, distanceToHit);
         float currentAdjustment = ComputeEstimatedForceCorrection(location, direction, distanceToHit);

         // Add an 'up' impulse based on the weight of the vehicle, our current time slice, and the adjustment.
         // Use current position and estimated future position to help smooth out the force. 
         float finalAdjustment = currentAdjustment * 0.85 + futureAdjustment * 0.15; 
         NxVec3 dir(0.0, 0.0, 1.0);
         physicsObject->addForce(dir * (weight * finalAdjustment * deltaTime), NX_SMOOTH_IMPULSE);

         // Now, figure out how to move our target toward our goal. We want it to sort of oscillate a bit, 
         // so we play with the forces a tad.
         osg::Vec3 targetVector =  mGoalLocation - location;
         osg::Vec3 targetVectorLookAhead = mGoalLocation - locationLookAhead;
         float distanceAway = targetVector.length() * 0.4 + targetVectorLookAhead.length() * 0.6;
         float distanceAwayPercent = (distanceAway) / mGroundClearance;
         float forceAdjustment = dtUtil::Min(10.0f, distanceAwayPercent); 
         targetVector.normalize();
         targetVector[2] = 0.01; // cancel out the z - that's up above.
         targetVector[1] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
         targetVector[0] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
         NxVec3 hoverDir(targetVector[0], targetVector[1], targetVector[2]);
         physicsObject->addForce(hoverDir * (weight * forceAdjustment * deltaTime), NX_SMOOTH_IMPULSE);



         // Debug code... delete me
         //timeTillPrint += deltaTime;
         //if (timeTillPrint > 0.2)
         //{
         //   timeTillPrint = 0.0f;
         //   std::cout << "Height [" << (distanceToHit) << "], Multiplier[" << finalAdjustment << "]." << std::endl;
         //}
      }
      else 
      {
         mTimeSinceKilled += deltaTime;
      }

      mTimeSinceBorn += deltaTime;

      // Delete the target after dead a while or just too 'old'
      if (mTimeSinceBorn > 120.0f || mTimeSinceKilled > 10.0f)
      {
         GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   float HoverTargetActor::ComputeEstimatedForceCorrection(const osg::Vec3 &location, 
      const osg::Vec3 &direction, float &distanceToHit)
   {
      static const int GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);
      float estimatedForceAdjustment = -dtAgeiaPhysX::DEFAULT_GRAVITY_Z; // gravity 
      osg::Vec3 terrainHitLocation;

      distanceToHit = GetPhysicsHelper()->GetClosestIntersectionUsingDirection(location, 
         direction, terrainHitLocation, GROUPS_FLAGS);

      if (distanceToHit > 0.0f) 
      {
         // Apply gravity force at the exact height we want. More below, less above.
         float distanceToCorrect = GetSphereRadius() + mGroundClearance - distanceToHit;
         if (distanceToCorrect >= 0.0f) // allow a 1 meter buffer to smoothly decay force
         {
            float distanceAwayPercent = distanceToCorrect / mGroundClearance;
            estimatedForceAdjustment *= (1.0f + (distanceAwayPercent * distanceAwayPercent)); 
         }
         else if (distanceToCorrect > -1.0f) // if we are slightly too high, then slow down our force
            estimatedForceAdjustment *= (0.01f + ((1.0f+distanceToCorrect) * 0.99)); // part gravity, part reduced
         else // counteract half of gravity :)
            estimatedForceAdjustment *= 0.01f; 
      }
      else 
         estimatedForceAdjustment = 0.0f;

      return estimatedForceAdjustment;

   }


   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActor::AgeiaPrePhysicsUpdate()
   {
      BasePhysicsVehicleActor::AgeiaPrePhysicsUpdate();

      // move our shield. Otherwise, we're set.
      if(mShield.valid())
      {
         mShield->Update();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActor::AgeiaPostPhysicsUpdate()
   {
      BasePhysicsVehicleActor::AgeiaPostPhysicsUpdate();

      // This is ONLY called if we are LOCAL (we put the check here just in case... )
      //if (!IsRemote())
      //{
         // Pull the position/rotation from the physics scene and put it on our actor in Delta3D. 
         // This allows attached cameras and other visuals to align. It also enables 
         // dead reckoning, which causes our position to be published automatically. 

         // For this hover vehicle, we really only want to push our translation, not our rotation. 
         // We want to bounce in place and move as a sphere. But, we don't want the roll.
         //NxActor* physXActor = GetPhysicsHelper()->GetPhysXObject();
         //if(!physXActor->isSleeping())
         //{
         //   dtCore::Transform ourTransform;
         //   GetTransform(ourTransform);
         //   ourTransform.SetTranslation(physXActor->getGlobalPosition()[0], 
         //      physXActor->getGlobalPosition()[1], physXActor->getGlobalPosition()[2]);
         //   SetTransform(ourTransform);
         //}
      //}

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
      const std::string& SOUND_GROUP = "Sound Property Values";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      HoverTargetActor  &actor = static_cast<HoverTargetActor &>(GetGameActor());

      //AddProperty(new dtDAL::BooleanActorProperty("VehicleIsTheTurret", "Vehicle Is The Turret",
      //   dtDAL::MakeFunctor(actor, &HoverTargetActor::SetVehicleIsTurret),
      //   dtDAL::MakeFunctorRet(actor, &HoverTargetActor::GetVehicleIsTurret),
      //   "True means the turret and the vehicle rotate together (unlike a HMMWV with a distinct turret).", VEH_GROUP));

      AddProperty(new dtDAL::FloatActorProperty("BaseWeight", "BaseWeight", 
         dtDAL::MakeFunctor(actor, &HoverTargetActor::SetVehicleBaseWeight),
         dtDAL::MakeFunctorRet(actor, &HoverTargetActor::GetVehicleBaseWeight),
         "The base weight of this vehicle.", VEHICLEGROUP));

      AddProperty(new dtDAL::FloatActorProperty("Sphere Radius", "Sphere Radius", 
         dtDAL::MakeFunctor(actor, &HoverTargetActor::SetSphereRadius),
         dtDAL::MakeFunctorRet(actor, &HoverTargetActor::GetSphereRadius),
         "The radius of the hover target.", VEHICLEGROUP));

      AddProperty(new dtDAL::FloatActorProperty("Ground Clearance", "Ground Clearance", 
         dtDAL::MakeFunctor(actor, &HoverTargetActor::SetGroundClearance),
         dtDAL::MakeFunctorRet(actor, &HoverTargetActor::GetGroundClearance),
         "The height we should try to leave beneath our target (cause we hover...).", VEHICLEGROUP));

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
