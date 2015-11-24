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
#include <prefix/SimCorePrefix.h>

#include "VehicleShield.h"

//#ifdef AGEIA_PHYSICS
#include <HoverTargetActor.h>
#include <HoverVehiclePhysicsHelper.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtCore/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/batchisector.h>
#include <dtCore/keyboard.h>
#include <dtCore/system.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/basemessages.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Components/MunitionsComponent.h>

#include <dtUtil/nodeprintout.h>

#include <osg/Stats>

namespace DriverDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   HoverTargetActor::HoverTargetActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mGoalLocation(10.0, 10.0, 10.0)
      , mTimeSinceKilled(0.0f)
      , mTimeSinceBorn(0.0f)
      , mPerfThrottleCountDown(0)
   {
      // Some init values are now in BuildActorComponents()

      SetDefaultScale(osg::Vec3(2.0f, 2.0f, 2.0f));

      SetEntityType("HoverTarget"); // Used for HLA mostly.  
      SetMunitionDamageTableName("StandardDamageType"); // Used for Munitions Damage.
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
      osg::Vec3 startVec(ourTransform.GetTranslation());
      if (!IsRemote()) // Local - we vary it's starting position.
      {
         startVec[0] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[1] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[2] += dtUtil::RandFloat(-3.0, 1.0);

         // Since we changed our starting position, update our visual actor, or it 'blips' for
         // one frame in the wrong place. Very ugly.
         ourTransform.SetTranslation(startVec[0], startVec[1], startVec[2]);
         SetTransform(ourTransform);

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
      GetTargetPhysicsActComp()->CreateTarget(ourTransform, GetOSGNode());
         //GetNodeCollector()->GetDOFTransform("dof_chassis"));
      //GetTargetPhysicsActComp()->CreateTarget(startVec, IsRemote());

      SimCore::Actors::BasePhysicsVehicleActor::OnEnteredWorld();

      // REMOTE - Finish initial startup conditions
      if(IsRemote())
      {
         // THIS LINE MUST BE AFTER Super::OnEnteredWorld()! Undo the kinematic flag on remote entities. Lets us
         // apply velocities to remote hover vehicles so that they will impact us and make us bounce back
         //GetTargetPhysicsActComp()->GetMainPhysicsObject()->clearBodyFlag(NX_BF_KINEMATIC);
      }
      // LOCAL - Finish initial startup conditions
      else
      {
         osg::Vec3 dir(0.0, 0.0, 1000.0);
         GetTargetPhysicsActComp()->GetMainPhysicsObject()->ApplyImpulse(dir);

         // Offset the Target Dir so that they spread out around the map.
         mGoalLocation[0] += dtUtil::RandFloat(-40.0, 40.0);
         mGoalLocation[1] += dtUtil::RandFloat(-50.0, 50.0);
         mGoalLocation[2] += dtUtil::RandFloat(2.0, 4.0);
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
   bool HoverTargetActor::CheckAndUpdatePerformanceThrottle(float deltaTime)
   {
      bool result = false;

      // if already throttling, 
      if (mPerfThrottleCountDown > 0)
      {
         mPerfThrottleCountDown --;
         result = (mPerfThrottleCountDown == 0);
      }
      else // Check to see if we should throttle down for performance reasons
      {
         result = true; // for sure we need to do it this frame, but maybe we skip next time.
         
         double lastPreFrameTime = dtCore::System::GetInstance().
            GetSystemStageTime(dtCore::System::STAGE_PREFRAME);
         if (lastPreFrameTime > 2.0) // past our budget
         {
            // determine how many to skip. Needs to be random or else all 
            // the targets will skip at the same time in a staggered burst
            // get a num from 0.5 to 5.5. Multiply by a rand (0,1). Truncate to int. Skip that many.
            float modifier = ((float(lastPreFrameTime)) - 1.0f)/2.0f;
            modifier = dtUtil::Min(modifier, 5.5f) * dtUtil::RandFloat(0.0f, 1.0f);
            mPerfThrottleCountDown = int(modifier);
         }
      }
      return result;
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      if( ! IsMobilityDisabled())
      {
         if (CheckAndUpdatePerformanceThrottle(deltaTime))
         {
            GetTargetPhysicsActComp()->ApplyTargetHoverForces(deltaTime, mGoalLocation);
         }
         else
         {
            GetTargetPhysicsActComp()->ApplyForceFromLastFrame(deltaTime);
         }
      }
      else
      {
         mTimeSinceKilled += deltaTime;
      }

      mTimeSinceBorn += deltaTime;

      // Delete the target after dead a while or just too 'old'
      if (mTimeSinceBorn > 90.0f || mTimeSinceKilled > 10.0f)
      {
         GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
      }
   }

   //////////////////////////////////////////////////////////////////////
   float HoverTargetActor::ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message,
      const SimCore::Actors::MunitionTypeActor& munition)
   {
      // Take more damage so we die quicker.
      return incomingDamage * 2.0f;
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
      //const std::string VEHICLEGROUP   = "Vehicle Property Values";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverTargetActorProxy::~HoverTargetActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActorProxy::CreateDrawable()
   {
      SetDrawable(*new HoverTargetActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActorProxy::OnEnteredWorld()
   {
      //RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

      SimCore::Actors::BasePhysicsVehicleActorProxy::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverTargetActorProxy::BuildActorComponents()
   {
      if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
      {
         AddComponent(*new HoverTargetPhysicsActComp());
      }

      BaseClass::BuildActorComponents();

      dtGame::DRPublishingActComp* drPublishingActComp = NULL;
      GetComponent(drPublishingActComp);
      if (drPublishingActComp == NULL)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(3.0f);
      drPublishingActComp->SetMaxTranslationError(0.02f);
      drPublishingActComp->SetMaxRotationError(1.0f);
   }

} // namespace
//#endif
