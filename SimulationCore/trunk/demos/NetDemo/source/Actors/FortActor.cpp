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
#include <FortActor.h>
//#include <HoverVehiclePhysicsHelper.h>
#include <dtPhysics/physicshelper.h>
//#include <NxAgeiaWorldComponent.h>
//#include <NxAgeiaRaycastReport.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
//#include <dtCore/batchisector.h>
#include <dtCore/keyboard.h>
//#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/basemessages.h>
//#include <osg/Switch>
//#include <osgSim/DOFTransform>
//#include <osgViewer/View>
//#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Components/RenderingSupportComponent.h>
//#include <SimCore/Actors/EntityActorRegistry.h>
//#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/CollisionGroupEnum.h>

//#include <dtUtil/nodeprintout.h>

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   FortActor::FortActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   {
      SetTimeForSendingDeadReckoningInfoOut(0.0f);
      SetTimesASecondYouCanSendOutAnUpdate(1.0f);

      SetPublishLinearVelocity(false);
      SetPublishAngularVelocity(false);

      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      dtPhysics::PhysicsHelper *helper = new dtPhysics::PhysicsHelper(proxy);
      //helper->SetBaseInterfaceClass(this);
      SetPhysicsHelper(helper);

   }

   ///////////////////////////////////////////////////////////////////////////////////
   FortActor::~FortActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      //GetHoverPhysicsHelper()->CreateVehicle(ourTransform,
      //   GetNodeCollector()->GetDOFTransform("dof_chassis"));
      //dtPhysics::PhysicsObject *physObj = GetHoverPhysicsHelper()->GetMainPhysicsObject();
      ////////dtPhysics::PhysicsObject* physActor = GetPhysicsHelper()->GetPhysXObject();

      if(!IsRemote())
      {
         SetEntityType("Fort");
      }

      BaseClass::OnEnteredWorld();

      // Add a dynamic light to our fort
      dtGame::GMComponent* comp = GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME);
      if(comp)
      {
         SimCore::Components::RenderingSupportComponent* rsComp = dynamic_cast<SimCore::Components::RenderingSupportComponent*>(comp);
         if(rsComp)
         {
            //Add a spot light
            SimCore::Components::RenderingSupportComponent::DynamicLight* sl = new SimCore::Components::RenderingSupportComponent::DynamicLight();
            sl->mRadius = 30.0f;
            sl->mIntensity = 1.0f;
            sl->mColor.set(1.0f, 1.0f, 1.0f);
            sl->mAttenuation.set(0.001, 0.004, 0.0002);
            sl->mTarget = this;
            sl->mAutoDeleteLightOnTargetNull = true;
            rsComp->AddDynamicLight(sl);
         }
      }
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

      FortActor& actor = *static_cast<FortActor*>(GetActor());

      // Add properties
   }

   ///////////////////////////////////////////////////////////////////////////////////
   FortActorProxy::~FortActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActorProxy::CreateActor()
   {
      SetActor(*new FortActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void FortActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
//#endif
