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
#include <Actors/HoverVehicleActor.h>
#include <Actors/HoverVehiclePhysicsHelper.h>
#include <dtPhysics/physicshelper.h>
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
//#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/DefaultFlexibleArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/CollisionGroupEnum.h>

#include <dtUtil/nodeprintout.h>
#include <iostream>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActor::HoverVehicleActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   , mVehicleIsTurret(true)
   {
      SetMaxUpdateSendRate(5.0f);

      SetPublishLinearVelocity(true);
      SetPublishAngularVelocity(false);
      SetMaxTranslationError(0.02f);
      SetMaxRotationError(1.0f);

      // Create physics helper - almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      HoverVehiclePhysicsHelper* helper = new HoverVehiclePhysicsHelper(proxy);
      //helper->SetBaseInterfaceClass(this);
      SetPhysicsHelper(helper);

      SetEntityType("HoverVehicle");
      SetMunitionDamageTableName("StandardDamageType");
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

      dtCore::RefPtr<dtUtil::NodePrintOut> nodePrinter = new dtUtil::NodePrintOut();
      std::string nodes = nodePrinter->CollectNodeData(*GetNonDamagedFileNode());
      std::cout << " --------- NODE PRINT OUT FOR HOVER VEHICLE --------- " << std::endl;
      std::cout << nodes.c_str() << std::endl;

      GetHoverPhysicsHelper()->CreateVehicle(ourTransform,
         GetNodeCollector()->GetDOFTransform("dof_chassis"));
      //dtPhysics::PhysicsObject *physObj = GetHoverPhysicsHelper()->GetMainPhysicsObject();

      if(!IsRemote())
      {
         //SetEntityType("HoverTank");
      }

      SimCore::Actors::BasePhysicsVehicleActor::OnEnteredWorld();

      SimCore::Components::RenderingSupportComponent* renderComp;
      GetGameActorProxy().GetGameManager()->GetComponentByName(
         SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);
      if(renderComp != NULL)
      {
         //Add a spot light
         SimCore::Components::RenderingSupportComponent::SpotLight* sl =
            new SimCore::Components::RenderingSupportComponent::SpotLight();
         sl->mIntensity = 1.0f;
         sl->mColor.set(1.0f, 1.0f, 1.0f);
         //sl->mAttenuation.set(0.02, 0.004, 0.00008);
         sl->mAttenuation.set(0.001, 0.004, 0.0002);
         sl->mDirection.set(0.0f, 1.0f, 0.0f);
         sl->mSpotExponent = 6.0f;
         sl->mSpotCosCutoff = 0.6f;
         sl->mTarget = this;
         sl->mAutoDeleteLightOnTargetNull = true;
         sl->mUseAbsoluteDirection = false;

         renderComp->AddDynamicLight(sl);
      }

      if(!IsRemote())
      {

         // Setup our articulation helper for the vehicle
         dtCore::RefPtr<SimCore::Components::DefaultFlexibleArticulationHelper> articHelper =
            new SimCore::Components::DefaultFlexibleArticulationHelper();
         articHelper->SetEntity(this);
         SetArticulationHelper(articHelper.get());
         articHelper->AddArticulation("dof_turret_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_HEADING);
         articHelper->AddArticulation("dof_gun_01",
            SimCore::Components::DefaultFlexibleArticulationHelper::ARTIC_TYPE_ELEVATION, "dof_turret_01");
      }

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
      if(keyboard == NULL)
         return;

      bool accelForward = false, accelReverse = false, accelLeft = false, accelRight = false;
      //float currentMPH = GetMPH(); // speed, not a velocity with direction

      if (!IsMobilityDisabled() && GetHasDriver())
      {
         // FORWARD OR BACKWARD
         if (keyboard->GetKeyState('w') || (keyboard->GetKeyState('W')) ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
         {
            accelForward = true;
         }
         else if (keyboard->GetKeyState('s') || keyboard->GetKeyState('S') ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
         {
            accelReverse = true;
         }

         // LEFT OR RIGHT
         if (keyboard->GetKeyState('a') || keyboard->GetKeyState('A') ||
              keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left))
         {
            accelLeft = true;
         }
         else if(keyboard->GetKeyState('d') || keyboard->GetKeyState('D') ||
               keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right))
         {
            accelRight = true;
         }

      }

      GetHoverPhysicsHelper()->UpdateVehicle(deltaTime,
         accelForward, accelReverse, accelLeft, accelRight);

      // Jump button
      if (!IsMobilityDisabled() && GetHasDriver())
      {
         if (keyboard->GetKeyState(' '))
         {
            //GetHoverPhysicsHelper()->Boost(deltaTime);
         }
      }
   }

   // NOTE - Ifdef Ageia and put the pre and post update back.

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::PostPhysicsUpdate()
   {
      // Mostly copied from BasePhysicsVehicleActor - we do NOT want want our vehicle to 'roll', so we
      // take the position and throw away the rotation.

      // This is ONLY called if we are LOCAL (we put the check here just in case... )
      if (!IsRemote() && GetPhysicsHelper() != NULL)
      {
         // The base behavior is that we want to pull the translation and rotation off the object
         // in our physics scene and apply it to our 3D object in the visual scene.
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();

         //TODO: Ask if the object is activated.  If not, the transform should not be pushed.
         if (!GetPushTransformToPhysics())
         {
            if(physicsObject != NULL)
            {
               // Take rotation from physics and apply to current xform - IE NO ROTATION!
               dtCore::Transform currentXForm;
               GetTransform(currentXForm);
               dtCore::Transform physicsXForm;
               physicsObject->GetTransform(physicsXForm);
               currentXForm.SetTranslation(physicsXForm.GetTranslation());
               SetTransform(currentXForm);
               SetPushTransformToPhysics(false);
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );

   }

   //////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );

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
      const std::string VEH_GROUP   = "Vehicle Property Values";
      //const std::string SOUND_GROUP = "Sound Property Values";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      HoverVehicleActor& actor = *static_cast<HoverVehicleActor*>(GetActor());

      AddProperty(new dtDAL::BooleanActorProperty("VehicleIsTheTurret", "Vehicle Is The Turret",
         dtDAL::MakeFunctor(actor, &HoverVehicleActor::SetVehicleIsTurret),
         dtDAL::MakeFunctorRet(actor, &HoverVehicleActor::GetVehicleIsTurret),
         "True means the turret and the vehicle rotate together (unlike a HMMWV with a distinct turret).", VEH_GROUP));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActorProxy::~HoverVehicleActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::CreateActor()
   {
      SetActor(*new HoverVehicleActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::OnEnteredWorld()
   {
      SimCore::Actors::BasePhysicsVehicleActorProxy::OnEnteredWorld();
   }

} // namespace
//#endif
