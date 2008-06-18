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
#ifdef AGEIA_PHYSICS
#include <HoverVehicleActor.h>
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
#include <SimCore/NxCollisionGroupEnum.h>

#include <dtUtil/nodeprintout.h>

namespace DriverDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActor ::HoverVehicleActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy) 
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
   //, SOUND_GEAR_CHANGE_HIGH(0.0f)
   , mTimeTillJumpReady(0.0f)
   {
      SetTimeForSendingDeadReckoningInfoOut(0.0f);
      SetTimesASecondYouCanSendOutAnUpdate(5.0f);

      // create my unique physics helper.  almost all of the physics is on the helper.  
      // The actor just manages properties and key presses mostly.
      HoverVehiclePhysicsHelper *helper = new HoverVehiclePhysicsHelper(proxy);
      helper->SetBaseInterfaceClass(this);
      SetPhysicsHelper(helper);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActor::~HoverVehicleActor(void)
   {
      //if(mSndBrake.valid())
      //{
      //   mSndBrake->Stop();
      //   RemoveChild(mSndBrake.get());
      //   mSndBrake.release();
      //}
      
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::OnEnteredWorld()
   {
      //if(SOUND_EFFECT_IGNITION != "")
      //{
      //   mSndIgnition       = dtAudio::AudioManager::GetInstance().NewSound();
      //   mSndIgnition->LoadFile(SOUND_EFFECT_IGNITION.c_str());
      //   mSndIgnition->ListenerRelative(false);
      //   AddChild(mSndIgnition.get());
      //   dtCore::Transform xform;
      //   mSndIgnition->SetTransform(xform, dtCore::Transformable::REL_CS);
      //}
      
      //osgSim::DOFTransform* Wheel[4];
      //Wheel[BACK_LEFT]  = GetNodeCollector()->GetDOFTransform("dof_wheel_lt_02"); 
      
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      //dtUtil::NodePrintOut nodePrinter;
      dtCore::RefPtr<dtUtil::NodePrintOut> nodePrinter = new dtUtil::NodePrintOut();
      std::string nodes = nodePrinter->CollectNodeData(*GetNonDamagedFileNode());
      std::cout << " --------- NODE PRINT OUT FOR HOVER VEHICLE --------- " << std::endl;
      std::cout << nodes.c_str() << std::endl;

      //GetHoverPhysicsHelper()->SetLocalOffSet(osg::Vec3(0,0,1.1));
      GetHoverPhysicsHelper()->SetLocalOffSet(osg::Vec3(0,0,0));
      GetHoverPhysicsHelper()->CreateVehicle(ourTransform, 
         GetNodeCollector()->GetDOFTransform("dof_chassis"));
      GetHoverPhysicsHelper()->SetLocalOffSet(osg::Vec3(0,0,0));

      if(!IsRemote())
      {
         GetHoverPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_NORMAL | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
      }
      //else // -- Flags set in the base class.
      //GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
      //GetHoverPhysicsHelper()->SetAgeiaUserData(mPhysicsHelper.get());

      if(!IsRemote())
      {
         GetHoverPhysicsHelper()->TurnObjectsGravityOff("Default");
      }

      SimCore::Actors::BasePhysicsVehicleActor::OnEnteredWorld();

      // For test, undo the kinematic flag so that we can apply velocity to remote hover vehicles so that 
      // they will impact us and make us bounce back
      NxActor *physActor = GetPhysicsHelper()->GetPhysXObject();
      if (physActor != NULL)
         physActor->clearBodyFlag(NX_BF_KINEMATIC);

      // override the flying property

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::UpdateSoundEffects(float deltaTime)
   {
      ////////////////////////////////////////////////////////////
      // do sound here
      // if the vehicle is moving

      //if(mSndVehicleIdleLoop == NULL)
      //   return;

      //if(mSndVehicleIdleLoop->IsPlaying() == false)
      //   mSndVehicleIdleLoop->Play();


      SimCore::Actors::BasePhysicsVehicleActor::UpdateSoundEffects(deltaTime);
   }


  ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
      //osg::ref_ptr<osgSim::DOFTransform> Wheel[4];
      //osg::ref_ptr<osgSim::DOFTransform> steeringWheel;
      if(!insideVehicle)
      {
         //Wheel[BACK_RIGHT] = GetNodeCollector()->GetDOFTransform("dof_wheel_rt_02");
      }

      //if(!insideVehicle)
      //{
            //osg::Vec3 HPR;
            //HPR[0] = GetFourWheelPhysicsHelper()->GetWheelShape(i)->getSteerAngle();
            //   HPR[1] = GetFourWheelPhysicsHelper()->GetAxleRotationOne();
            //HPR[2] = 0.0f;
            //Wheel[i]->setCurrentHPR(HPR);
      //}
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::ResetVehicle()
   {
      SimCore::Actors::BasePhysicsVehicleActor::ResetVehicle();
      
      //if(mSndIgnition != NULL)
      //{
      //   if(!mSndIgnition->IsPlaying())
      //      mSndIgnition->Play();
      //}
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      dtCore::Keyboard *keyboard = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
      if(keyboard == NULL)
         return;

      bool accelForward = false, accelReverse = false, accelLeft = false, accelRight = false;
      float currentMPH = GetMPH(); // speed, not a velocity with direction

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

      GetHoverPhysicsHelper()->UpdateVehicle(deltaTime, currentMPH, 
         accelForward, accelReverse, accelLeft, accelRight);

      // Jump button
      if( ! IsMobilityDisabled() && GetHasDriver() )
      {
         mTimeTillJumpReady -= deltaTime;
         if (keyboard->GetKeyState(' ') && mTimeTillJumpReady < 0.0f)
         {
            GetHoverPhysicsHelper()->DoJump(deltaTime);
            mTimeTillJumpReady = 2.5f;
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::RepositionVehicle(float deltaTime)
   {
      // Note - this should be refactored. There should be a base physics vehicle HELPER. 
      // See nxageiaFourWheelActor::RepositionVehicle() for more info.

      SimCore::Actors::BasePhysicsVehicleActor::RepositionVehicle(deltaTime);
      //GetFourWheelPhysicsHelper()->RepositionVehicle(deltaTime);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::AgeiaPrePhysicsUpdate()
   {
      // The PRE physics update is only trapped if we are remote. It updates the physics 
      // engine and moves the vehicle to where we think it is now (based on Dead Reckoning)

      if (IsRemote())
      {
         osg::Matrix rot = GetMatrixNode()->getMatrix();
         NxActor* physObject = GetPhysicsHelper()->GetPhysXObject();

         // In order to make our vehicle bounce, make sure we have the velocity set. Otherwise, it 
         // dead reckons really fast, but from the physics perspective looks like it's not moving.
         osg::Vec3 velocity = GetVelocityVector();
         NxVec3 physVelocity(velocity[0], velocity[1], velocity[2]);
         physObject->setLinearVelocity(physVelocity );

         if(physObject != NULL)
         {
            physObject->setGlobalPosition(NxVec3(rot.operator ()(3,0), rot.operator ()(3,1), rot.operator ()(3,2)));
            physObject->setGlobalOrientation(
               NxMat33( NxVec3(rot.operator ()(0,0), rot.operator ()(0,1), rot.operator ()(0,2)),
               NxVec3(rot.operator ()(1,0), rot.operator ()(1,1), rot.operator ()(1,2)),
               NxVec3(rot.operator ()(2,0), rot.operator ()(2,1), rot.operator ()(2,2))));
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActor::AgeiaPostPhysicsUpdate()
   {
      // This is ONLY called if we are LOCAL (we put the check here just in case... )

      if (!IsRemote())
      {
         // For this hover vehicle, we really only want to push our translation, not our rotation. 
         // We want the bounce in place, as a sphere, not roll around visible - ugh... seasick ... vomit!
         NxActor* physXActor = GetHoverPhysicsHelper()->GetPhysXObject();
         if(!physXActor->isSleeping())
         {
            dtCore::Transform ourTransform;
            GetTransform(ourTransform);
            ourTransform.SetTranslation(physXActor->getGlobalPosition()[0], 
               physXActor->getGlobalPosition()[1], physXActor->getGlobalPosition()[2]);
            SetTransform(ourTransform);
         }
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
      const std::string& VEH_GROUP   = "Vehicle Property Values";
      const std::string& SOUND_GROUP = "Sound Property Values";

      SimCore::Actors::BasePhysicsVehicleActorProxy::BuildPropertyMap();

      HoverVehicleActor  &actor = static_cast<HoverVehicleActor &>(GetGameActor());

      //AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
      //   "VEHICLE_INSIDE_MODEL", "VEHICLE_INSIDE_MODEL_PATH", dtDAL::MakeFunctor(actor, 
      //   &HoverVehicleActor::SetVehicleInsideModel),
      //   "What is the filepath / string of the inside model", VEH_GROUP));
      
      //AddProperty(new dtDAL::FloatActorProperty("SOUND_BRAKE_SQUEAL_AMOUNT", "How much MPH for Squeal Brake",
      //   dtDAL::MakeFunctor(actor, &HoverVehicleActor ::SetSound_brake_squeal_amount),
      //   dtDAL::MakeFunctorRet(actor, &HoverVehicleActor ::GetSound_brake_squeal_amount),
      //   "How many mph does the car have to go to squeal used with BRAKE_STOP_NOW_BRAKE_TIME", SOUND_GROUP));

      //AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
      //   "SOUND_EFFECT_IGNITION", "SFX Ignition Path", dtDAL::MakeFunctor(actor, 
      //   &HoverVehicleActor::SetSound_effect_ignition),
      //   "What is the filepath / string of the sound effect", SOUND_GROUP));

   }

   ///////////////////////////////////////////////////////////////////////////////////
   HoverVehicleActorProxy::~HoverVehicleActorProxy(){}
   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::CreateActor()
   {
      SetActor(*new HoverVehicleActor(*this));

      SimCore::Actors::BaseEntity* entityActor = dynamic_cast<SimCore::Actors::BaseEntity*> (GetActor());
      if( entityActor != NULL )
      {
         entityActor->InitDeadReckoningHelper();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void HoverVehicleActorProxy::OnEnteredWorld()
   {
      //RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

      SimCore::Actors::BasePhysicsVehicleActorProxy::OnEnteredWorld();
   }

} // namespace 
#endif
