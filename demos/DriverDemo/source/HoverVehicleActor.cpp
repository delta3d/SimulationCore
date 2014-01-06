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
#include <HoverVehicleActor.h>
#include <HoverVehiclePhysicsHelper.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsmaterials.h>
//#include <NxAgeiaWorldComponent.h>
//#include <NxAgeiaRaycastReport.h>
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
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/CollisionGroupEnum.h>

#include <dtUtil/nodeprintout.h>

namespace DriverDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActor ::HoverVehicleActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   //, SOUND_GEAR_CHANGE_HIGH(0.0f)
   , mTimeTillJumpReady(0.0f)
   , mVehicleIsTurret(true)
   {

      SetEntityType("HoverVehicle"); // Used for HLA mapping mostly
      SetMunitionDamageTableName("VehicleDamageType"); // Used for Munitions Damage.
   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActor::~HoverVehicleActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      //std::shared_ptr<dtUtil::NodePrintOut> nodePrinter = new dtUtil::NodePrintOut();
      //std::string nodes = nodePrinter->CollectNodeData(*GetNonDamagedFileNode());
      //std::cout << " --------- NODE PRINT OUT FOR HOVER VEHICLE --------- " << std::endl;
      //std::cout << nodes.c_str() << std::endl;

      GetHoverPhysicsActComp()->CreateVehicle(ourTransform,
         GetNodeCollector()->GetDOFTransform("dof_chassis"));
      //dtPhysics::PhysicsObject* physActor = GetPhysicsActComp()->GetMainPhysicsObject();

      SimCore::Actors::BasePhysicsVehicleActor::OnEnteredWorld();


      /*
      if(IsRemote() && physActor != nullptr)
      {
         GetHoverPhysicsActComp()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_GET_COLLISION_REPORT |
            dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE);
         // THIS LINE MUST BE AFTER Super::OnEnteredWorld()! Undo the kinematic flag on remote entities. Lets us
         // apply velocities to remote hover vehicles so that they will impact us and make us bounce back
         physActor->clearBodyFlag(NX_BF_KINEMATIC);
      }
      */

      // Add the swirly shield to remote vehicles.
      mShield = new VehicleShield();
      mShield->SetTranslation(osg::Vec3(0.0f, 0.0f, 0.5f));
      AddChild(mShield.get());

      SimCore::Components::RenderingSupportComponent* rsComp = nullptr;
      GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsComp);
      if( rsComp != nullptr)
      {
         rsComp->SetMaxSpotLights(1);
         //Add a spot light
         SimCore::Components::RenderingSupportComponent::SpotLight* sl = new SimCore::Components::RenderingSupportComponent::SpotLight();
         sl->mIntensity = 1.0f;
         sl->mColor.set(1.0f, 1.0f, 1.0f);
         sl->mAttenuation.set(0.00002, 0.00002, 0.00005);
         sl->mDirection.set(0.0f, 1.0f, 0.0f);
         sl->mSpotExponent = 20.0f;
         sl->mSpotCosCutoff = 0.75f;
         sl->mTarget = this;
         sl->mAutoDeleteLightOnTargetNull = true;
         sl->mUseAbsoluteDirection = false;

         rsComp->AddDynamicLight(sl);
      }

      /*
      if(!IsRemote())
      {
         // Setup our articulation helper for the vehicle
         std::shared_ptr<SimCore::Components::DefaultFlexibleArticulationHelper> articHelper =
            new SimCore::Components::DefaultFlexibleArticulationHelper();
         articHelper->SetEntity(this);
         SetArticulationHelper(articHelper.get());
         articHelper->AddArticulation("dof_turret_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_HEADING);
         articHelper->AddArticulation("dof_gun_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_ELEVATION, "dof_turret_01");
      }
      */

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::UpdateSoundEffects(float deltaTime)
   {
      SimCore::Actors::BasePhysicsVehicleActor::UpdateSoundEffects(deltaTime);
   }


  ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::ResetVehicle()
   {
      SimCore::Actors::BasePhysicsVehicleActor::ResetVehicle();
   }


   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      dtCore::Keyboard *keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
      if(keyboard == nullptr)
         return;

      bool accelForward = false, accelReverse = false, accelLeft = false, accelRight = false;
      //float currentMPH = GetMPH(); // speed, not a velocity with direction

      if( ! IsMobilityDisabled() && GetHasDriver() )
      {
         // FORWARD OR BACKWARD
         if (keyboard->GetKeyState('w') || (keyboard->GetKeyState('W')) ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
            accelForward = true;
         else if (keyboard->GetKeyState('s') || keyboard->GetKeyState('S') ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
            accelReverse = true;

         // LEFT OR RIGHT
         if (keyboard->GetKeyState('a') || keyboard->GetKeyState('A') ||
              keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
            accelLeft = true;
         else if(keyboard->GetKeyState('d') || keyboard->GetKeyState('D') ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
            accelRight = true;

      }

      GetHoverPhysicsActComp()->UpdateVehicle(deltaTime,
         accelForward, accelReverse, accelLeft, accelRight);

      // Jump button
      if( ! IsMobilityDisabled() && GetHasDriver() )
      {
         mTimeTillJumpReady -= deltaTime;
         if (keyboard->GetKeyState(' ') && mTimeTillJumpReady < 0.0f)
         {
            GetHoverPhysicsActComp()->DoJump(deltaTime);
            mTimeTillJumpReady = 3.0f;
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::PostPhysicsUpdate()
   {
      // Mostly copied from BasePhysicsVehicleActor - we do NOT want want our vehicle to 'roll', so we
      // take the position and throw away the rotation.

      // This is ONLY called if we are LOCAL (we put the check here just in case... )
      if (!IsRemote() && GetPhysicsActComp() != nullptr)
      {
         // The base behavior is that we want to pull the translation and rotation off the object
         // in our physics scene and apply it to our 3D object in the visual scene.
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsActComp()->GetMainPhysicsObject();

         //TODO: Ask if the object is activated.  If not, the transform should not be pushed.
         if (!GetPushTransformToPhysics())
         {
            if(physicsObject != nullptr)
            {
               // Take rotation from physics and apply to current xform - IE NO ROTATION!
               dtCore::Transform currentXForm;
               GetTransform(currentXForm);
               dtCore::Transform physicsXForm;
               //physicsObject->GetTransform(physicsXForm);
               physicsObject->GetTransformAsVisual(physicsXForm);
               currentXForm.SetTranslation(physicsXForm.GetTranslation());
               SetTransform(currentXForm);
               SetPushTransformToPhysics(false);
            }
         }
      }
   }

   /*
   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::AgeiaPrePhysicsUpdate()
   {
      dtPhysics::PhysicsObject* physObject = GetPhysicsActComp()->GetMainPhysicsObject();

      // The PRE physics update is only trapped if we are remote. It updates the physics
      // engine and moves the vehicle to where we think it is now (based on Dead Reckoning)
      // We do this because we don't own remote vehicles and naturally can't just go
      // physically simulating them however we like. But, the physics scene needs them to interact with.
      if (IsRemote() && physObject != nullptr)
      {
         osg::Matrix rot = GetMatrixNode()->getMatrix();

         // In order to make our local vehicle bounce on impact, the physics engine needs the velocity of
         // the remote entities. Essentially remote entities are kinematic (physics isn't really simulating),
         // but we want to act like their not.
         osg::Vec3 velocity = GetDeadReckoningHelper().GetLastKnownVelocity();;
         NxVec3 physVelocity(velocity[0], velocity[1], velocity[2]);
         physObject->setLinearVelocity(physVelocity );

         // Move the remote physics object to its dead reckoned position/rotation.
         physObject->setGlobalPosition(NxVec3(rot.operator ()(3,0), rot.operator ()(3,1), rot.operator ()(3,2)));
         physObject->setGlobalOrientation(
            NxMat33( NxVec3(rot.operator ()(0,0), rot.operator ()(0,1), rot.operator ()(0,2)),
            NxVec3(rot.operator ()(1,0), rot.operator ()(1,1), rot.operator ()(1,2)),
            NxVec3(rot.operator ()(2,0), rot.operator ()(2,1), rot.operator ()(2,2))));
      }
   }
   */

   //////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

      if( mShield.valid() )
      {
         mShield->Update();
      }
   }

   //////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );

      if( mShield.valid() )
      {
         mShield->Update();
      }
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   HoverVehicleActorProxy::HoverVehicleActorProxy()
   {
      SetClassName("HoverVehicleActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::BuildPropertyMap()
   {
      static const dtUtil::RefString VEH_GROUP   = "Vehicle Property Values";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      HoverVehicleActor* actor = nullptr;
      GetActor(actor);

      AddProperty(new dtDAL::BooleanActorProperty("VehicleIsTheTurret", "Vehicle Is The Turret",
               dtDAL::BooleanActorProperty::SetFuncType(actor, &HoverVehicleActor::SetVehicleIsTurret),
               dtDAL::BooleanActorProperty::GetFuncType(actor, &HoverVehicleActor::GetVehicleIsTurret),
               "True means the turret and the vehicle rotate together (unlike a HMMWV with a distinct turret).", VEH_GROUP));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActorProxy::~HoverVehicleActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::CreateDrawable()
   {
      SetDrawable(*new HoverVehicleActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::OnEnteredWorld()
   {
      SimCore::Actors::BasePhysicsVehicleActorProxy::OnEnteredWorld();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::BuildActorComponents()
   {
      dtGame::GameActor* owner = nullptr;
      GetActor(owner);

      if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
      {
         AddComponent(*new HoverVehiclePhysicsActComp());
      }

      BaseClass::BuildActorComponents();

      dtGame::DRPublishingActComp* drPublishingActComp = nullptr;
      GetComponent(drPublishingActComp);
      if (drPublishingActComp == nullptr)
      {
         LOG_ERROR("CRITICAL ERROR - No DR Publishing Actor Component.");
         return;
      }
      drPublishingActComp->SetMaxUpdateSendRate(5.0f);
      //drPublishingActComp->SetPublishLinearVelocity(true);
      //drPublishingActComp->SetPublishAngularVelocity(false);
      drPublishingActComp->SetMaxTranslationError(0.001f);
      drPublishingActComp->SetMaxRotationError(0.5f);
      drPublishingActComp->SetPublishAngularVelocity(false);
   }

} // namespace
//#endif
