/*
* Copyright, 2009, Alion Science and Technology Corporation, all rights reserved.
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
#include "DriverActorRegistry.h"

//#ifdef AGEIA_PHYSICS
#include <HoverExplodingTargetActor.h>
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
#include <dtCore/shadermanager.h>
#include <dtCore/shaderparameter.h>
#include <dtCore/shaderparamoscillator.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagefactory.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <dtUtil/nodeprintout.h>

#include <iostream>

namespace DriverDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   HoverExplodingTargetActor ::HoverExplodingTargetActor(SimCore::Actors::BasePhysicsVehicleActorProxy& proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mGoalLocation(10.0, 10.0, 10.0)
      , mTimeSinceKilled(0.0f)
      , mTimeSinceBorn(0.0f)
      , mTimeSinceWasHit(0.0f)
      , mChasingModeActive(false)
   {
      SetDefaultScale(osg::Vec3(2.0f, 2.0f, 2.0f));

      SetEntityType("HoverExplodingTarget"); // Used for HLA mostly. 
      SetMunitionDamageTableName("StandardDamageType"); // Used for Munitions Damage.
   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverExplodingTargetActor::~HoverExplodingTargetActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::OnEnteredWorld()
   {

      // Create our vehicle with a starting position
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      osg::Vec3 startVec;
      if (!IsRemote()) // Local - we vary it's starting position.
      {
         //startVec = GetPhysicsActComp()->GetVehicleStartingPosition();
         startVec[0] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[1] += dtUtil::RandFloat(-10.0, 10.0);
         startVec[2] += dtUtil::RandFloat(0.0, 4.0);

         // Since we changed our starting position, update our visual actor, or it 'blips' for
         // one frame in the wrong place. Very ugly.
         ourTransform.SetTranslation(startVec[0], startVec[1], startVec[2]);
         SetTransform(ourTransform);

         // Make a semi-unique name.
         static int targetCounter = 0;
         targetCounter ++;
         SetName("Target Exploding " + dtUtil::ToString(targetCounter));
      }
      else // Remote -just grab position
      {
         startVec = ourTransform.GetTranslation();
      }

      // Create our physics object
      GetTargetPhysicsActComp()->CreateTarget(ourTransform, GetOSGNode());
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
         // Give it a boost upwards on creation.
         osg::Vec3 dir(0.0, 0.0, 2000.0);
         GetComponent<dtPhysics::PhysicsActComp>()->GetMainPhysicsObject()->ApplyImpulse(dir);

         // Offset the Target Dir so that they spread out around the map.
         mGoalLocation[0] += dtUtil::RandFloat(-40.0, 40.0);
         mGoalLocation[1] += dtUtil::RandFloat(-50.0, 50.0);
         mGoalLocation[2] += dtUtil::RandFloat(2.0, 4.0);
      }

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::UpdateSoundEffects(float deltaTime)
   {
      SimCore::Actors::BasePhysicsVehicleActor::UpdateSoundEffects(deltaTime);
   }


  ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      if( ! IsMobilityDisabled())
      {
         // we call this every frame because mPlayerWeAreChasing is an observer. Allows
         // us to stop chasing a player if they were deleted.
         SetChasingModeActive(mPlayerWeAreChasing != NULL);
         // we are chasing a player (for a little while), so update our target loc.
         if (GetChasingModeActive())
         {
            mTimeSinceWasHit += deltaTime;

            // get the player's loc and use it as our goal location.
            dtCore::Transform playerTransform;
            mPlayerWeAreChasing->GetTransform(playerTransform);
            playerTransform.GetTranslation(mGoalLocation);
         }

         // Do all our movement!
         GetTargetPhysicsActComp()->ApplyTargetHoverForces(deltaTime, mGoalLocation);
      }
      else
      {
         mTimeSinceKilled += deltaTime;
      }

      mTimeSinceBorn += deltaTime;

      // Delete the target after dead a while or just too 'old'
      if (mTimeSinceKilled > 1.0 || mTimeSinceWasHit > 8.0f)
      {
         DoExplosion();

         GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
      }
      else if (mTimeSinceBorn > 90.0f)
      {
         GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::DoExplosion()
   {
      //const osg::Vec3& finalVelocity, const osg::Vec3& location, const dtCore::Transformable* target )
      //printf("Sending DETONATION\r\n");

      dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      osg::Vec3 trans = ourTransform.GetTranslation();

      // Prepare a detonation message
      dtCore::RefPtr<SimCore::DetonationMessage> msg;
      gm->GetMessageFactory().CreateMessage( SimCore::MessageType::DETONATION, msg );

      // Required Parameters:
      // --- EventIdentifier
      msg->SetEventIdentifier( 1 );
      // --- DetonationLocation
      msg->SetDetonationLocation(trans);
      // --- DetonationResultCode
      // 1 == Entity Impact
      // 3 == Ground Impact
      // 5 == Detonation
      msg->SetDetonationResultCode( 3 ); // TO BE DYNAMIC
      msg->SetMunitionType("Generic Explosive"); //High Explosive");
      msg->SetFuseType(0);
      msg->SetWarheadType(0);
      msg->SetQuantityFired(1);
      // FiringObjectIdentifier
      msg->SetSendingActorId(GetGameActorProxy().GetId());

      // Optional Parameters:
      // FinalVelocityVector
      //msg->SetFinalVelocityVector( finalVelocity );
      msg->SetRateOfFire(1);

      gm->SendMessage( *msg );
      gm->SendNetworkMessage( *msg );
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::RespondToHit(const SimCore::DetonationMessage& message,
      const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force,
      const osg::Vec3& location)
   {
      SimCore::Actors::BaseEntity::RespondToHit(message, munition, force, location);

      // The target was hit by a munition. We've already taken damage and had forces applied
      // If we aren't about to die, then set our new goal target to be the player
      // that shot us. Then, each frame, we will fly toward that player.
      if (SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED != GetDamageState())
      {
         // Get the one that shot us.
         dtCore::RefPtr<SimCore::Actors::BasePhysicsVehicleActorProxy> shooterProxy
            = dynamic_cast<SimCore::Actors::BasePhysicsVehicleActorProxy*>
               (GetGameActorProxy().GetGameManager()->FindActorById(message.GetSendingActorId()));

         if (shooterProxy != NULL &&
            (shooterProxy->GetActorType() == *DriverActorRegistry::HOVER_VEHICLE_ACTOR_TYPE))
         {
            //std::cout << "Exploding Target was hit! Going to start chasing player [" <<
            //   shooterProxy->GetName() << "]." << std::endl;
            mPlayerWeAreChasing = dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor *>
               (shooterProxy->GetDrawable());

            // Turn on highlight if not already on
            bool wasChasingActive = mChasingModeActive;
            SetChasingModeActive(true);

            // We publish the 'chasing' mode via the 'engine smoke on' property.
            // Since we are local, we set EngineSmokeOn and then publish.
            if (!wasChasingActive)
            {
               InnerSetEngineSmokeOn(true);
               // Note, we could probably do a partial here, but some systems require certain
               // minimum properties (such as velocity and trans) in order to publish, so just do them all
               // to be safe. This is a rare event anyway.
               GetGameActorProxy().NotifyFullActorUpdate();
            }
         }

         // debugging stuff
         //else if (shooterProxy != NULL)
         //   std::cout << "Exploding Target was hit by [" << shooterProxy->GetName() << "]! NO PLAYER TO CHASE." << std::endl;
         //else
         //   std::cout << "Exploding Target was hit - NOT BY A BASE PHYSICS OBJECT. " << std::endl;

      }

   }

   //////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::SetChasingModeActive(bool newMode)
   {

      // NOTE - if you have different models for your damage/destroyed/no-damage states
      // of your entity, you can apply shaders directly that way. In this case, I am
      // bypassing what's in OnShaderGroupChanged() and manually controlling the shaders.

      if (newMode != mChasingModeActive)
      {
         std::string newShaderName;

         if (mChasingModeActive) // We were ON. Turn off chasing and set to NORMAL shader
         {
            newShaderName = "NormalMode";
         }
         else // We were OFF. Turn off normal and set to CHASING shader
         {
            newShaderName = "Chasing";
         }

         // clean up any previous shaders, if any
         dtCore::ShaderManager::GetInstance().UnassignShaderFromNode(*GetOSGNode());

         // Find our template
         dtCore::ShaderProgram* templateShader = dtCore::ShaderManager::GetInstance().
            FindShaderPrototype(newShaderName,GetShaderGroup());
         if (templateShader != NULL)
         {
            //std::cout << " LOADED SHADER [" << newShaderName << "]." << std::endl;
            mCurrentShader = dtCore::ShaderManager::GetInstance().
               AssignShaderFromPrototype(*templateShader, *GetOSGNode());
            //timerParam = dynamic_cast<dtCore::ShaderParamOscillator*> (mCurrentShader->FindParameter("TimeDilation"));
            //if (timerParam != NULL)   timerParam->SetValue(prevTime);
         }
         else
         {
            LOG_ERROR("Could not load shader [" + newShaderName + "], group[" + GetShaderGroup() + "].");
            mCurrentShader = NULL;
         }


         mChasingModeActive = newMode;
      }
   }

   //////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::SetEngineSmokeOn(bool enable)
   {
      // The engine smoke property is what we publish over the network for
      // Chasing Mode since there is no HLA equivalent.
      SetChasingModeActive(enable);
   }

   //////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActor::OnShaderGroupChanged()
   {
      // Overridden from Platform. The platform does some funky stuff with all the damage
      // states and puts shaders on each of the separate damage state nodes. That's great, but
      // makes it difficult to manipulate them manually.

      if (GetShaderGroup().empty())
      {
         return;
      }

      GameActor::OnShaderGroupChanged();
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   HoverExplodingTargetActorProxy::HoverExplodingTargetActorProxy()
   {
      SetClassName("HoverExplodingTargetActor");
  }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActorProxy::BuildPropertyMap()
   {
      //const std::string& VEHICLEGROUP   = "Vehicle Property Values";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverExplodingTargetActorProxy::~HoverExplodingTargetActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActorProxy::CreateDrawable()
   {
      SetDrawable(*new HoverExplodingTargetActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActorProxy::OnEnteredWorld()
   {
      //RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

      SimCore::Actors::BasePhysicsVehicleActorProxy::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverExplodingTargetActorProxy::BuildActorComponents()
   {
      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      AddComponent(*new HoverTargetPhysicsActComp());

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
