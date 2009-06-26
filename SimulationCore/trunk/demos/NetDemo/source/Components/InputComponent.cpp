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
 David Guthrie
*/
#include <dtUtil/mswin.h>
#include <Components/InputComponent.h>

#include <osgGA/GUIEventAdapter>

#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtABC/application.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/deltawin.h>
#include <dtCore/shadermanager.h>

#include <SimCore/Utilities.h>
#include <SimCore/BaseGameEntryPoint.h>
#include <SimCore/ClampedMotionModel.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/GameState/GameStateChangeMessage.h>

#include <States.h>
#include <ActorRegistry.h>
#include <Components/GameLogicComponent.h>
#include <Actors/PlayerStatusActor.h>
#include <Actors/DRGhostActor.h>
// TEMP STUFF FOR VEHICLE
#include <Actors/HoverVehicleActor.h>
#include <Actors/HoverVehiclePhysicsHelper.h>
#include <Actors/EnemyMine.h>

#include <osg/io_utils>
#include <iostream>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>


namespace NetDemo
{
   const dtUtil::RefString InputComponent::DOF_NAME_WEAPON_PIVOT("dof_gun_01");
   const dtUtil::RefString InputComponent::DOF_NAME_WEAPON_FIRE_POINT("dof_hotspot_01");
   const dtUtil::RefString InputComponent::DOF_NAME_RINGMOUNT("dof_turret_01");
   const dtUtil::RefString InputComponent::DOF_NAME_VIEW_01("dof_view_01");
   const dtUtil::RefString InputComponent::DOF_NAME_VIEW_02("dof_view_02");

   //////////////////////////////////////////////////////////////
   InputComponent::InputComponent(const std::string& name)
      : SimCore::Components::BaseInputComponent(name)
      , mCurrentViewPointIndex(0)
      , mIsInGameState(false)
      , mOriginalPublishTimesPerSecond(3.0f)
   {
      mViewPointList.push_back(DOF_NAME_WEAPON_PIVOT);
      mViewPointList.push_back(DOF_NAME_RINGMOUNT);
      mViewPointList.push_back(DOF_NAME_VIEW_01);
      mViewPointList.push_back(DOF_NAME_VIEW_02);
   }

   //////////////////////////////////////////////////////////////
   InputComponent::~InputComponent()
   {

   }

   //////////////////////////////////////////////////////////////
   void  InputComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& msgType = message.GetMessageType();
      if (msgType == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD
               && message.GetSource() == GetGameManager()->GetMachineInfo())
      {
         dtGame::GameActorProxy* stealthProxy = NULL;
         GetGameManager()->FindGameActorById(message.GetAboutActorId(), stealthProxy);
         if (stealthProxy == NULL)
         {
            LOG_ERROR("Got a player entered world message, but no player was found.")
            return;
         }
         else if (!stealthProxy->IsRemote()) // Somebody else's player.
         {
            SimCore::Actors::StealthActor* stealthActor = NULL;
            stealthProxy->GetActor(stealthActor);
            SetStealthActor(stealthActor);

            // We start with observer motion model. When we detach from vehicles, we go back to this.
            // When we attach to vehicles, this gets trampled.
            mMotionModel->SetTarget(stealthActor);
            EnableMotionModels();
         }
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == msgType)
      {
         HandleActorUpdateMessage(static_cast<const dtGame::ActorUpdateMessage&>(message));
      }
      else if (SimCore::MessageType::GAME_STATE_CHANGED == msgType)
      {
         HandleStateChangeMessage(static_cast<const SimCore::Components::GameStateChangedMessage&>(message));
      }
      else if (dtGame::MessageType::INFO_MAP_LOADED == msgType)
      {
      }
      else if (dtGame::MessageType::TICK_LOCAL == msgType)
      {
         // Disable motion models if we lost firepower - this should be done elsewhere, by a message or something
         if(mVehicle.valid() && mVehicle->IsFirepowerDisabled())
         {
            if (mRingMM->IsEnabled())
               mRingMM->SetEnabled(false);
            if (mWeaponMM->IsEnabled())
               mWeaponMM->SetEnabled(false);
            //mWeapon->SetTriggerHeld( false );
         }
      }
      // Actor was deleted. Clear out our stealth or vehicle if appropriate
      else if (dtGame::MessageType::INFO_ACTOR_DELETED == msgType)
      {
         const dtCore::UniqueId& id = message.GetAboutActorId();
         SimCore::Actors::StealthActor* stealth = GetStealthActor();
         if (stealth != NULL && stealth->GetUniqueId() == id)
         {
            SetStealthActor(NULL); // this is VERY VERY bad btw. We assume we are shutting down or something
         }
         else if (mVehicle.valid() && mVehicle->GetUniqueId() == id)
         {
            DetachFromCurrentVehicle();
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void InputComponent::HandleStateChangeMessage( const SimCore::Components::GameStateChangedMessage& stateChange )
   {
      const SimCore::Components::StateType& state = stateChange.GetNewState();

      if (state == NetDemoState::STATE_GAME_RUNNING)
      {
         mIsInGameState = true;
      }
      else
      {
         CleanUpDRGhost();
         mIsInGameState = false;
      }
      EnableMotionModels();
   }

   //////////////////////////////////////////////////////////////
   void InputComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();
      dtABC::Application& app = GetGameManager()->GetApplication();
      mMotionModel = new dtCore::FlyMotionModel(app.GetKeyboard(),app.GetMouse(), dtCore::FlyMotionModel::OPTION_DEFAULT);

      mRingMM = new SimCore::ClampedMotionModel(app.GetKeyboard(), app.GetMouse());
      mRingMM->SetMaximumMouseTurnSpeed(40.0f);
      mRingMM->SetUpDownLimit(0.0f);
      mRingMM->SetName("RingMM");

      mWeaponMM = new SimCore::ClampedMotionModel(app.GetKeyboard(), app.GetMouse());
      mWeaponMM->SetLeftRightLimit(0.0f);
      mWeaponMM->SetMaximumMouseTurnSpeed(70.0f);
      mWeaponMM->SetUpDownLimit(45.0f, 15.0f); // should probably be per vehicle
      mWeaponMM->SetName("WeaponMM");

   }

   //////////////////////////////////////////////////////////////
   void InputComponent::OnRemovedFromGM()
   {
      BaseClass::OnRemovedFromGM();

      DetachFromCurrentVehicle();

      mMotionModel = NULL;
      mRingMM = NULL;
      mWeaponMM = NULL;
      mVehicle = NULL;
      mDOFRing = NULL;
      mDOFWeapon = NULL;
      mRingMM = NULL;
      mWeaponMM = NULL;
   }

   ////////////////////////////////////////////////////////////////////
   void InputComponent::HandleActorUpdateMessage(const dtGame::ActorUpdateMessage& updateMessage)
   {
      // PLAYER STATUS - if it's ours, then update our attached vehicle
      if (updateMessage.GetActorType() == NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE &&
         updateMessage.GetSource() == GetGameManager()->GetMachineInfo())
      {
         // Find the actor in the GM - assume not null, else we're doomed anyway.
         PlayerStatusActorProxy* playerProxy;
         GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId(), playerProxy);
         PlayerStatusActor& playerStatus = playerProxy->GetActorAsPlayerStatus();

         // If we don't have a vehicle yet, or our current vehicle is different, then attach to it.
         if (!mVehicle.valid() || mVehicle->GetUniqueId() != playerStatus.GetAttachedVehicleID())
         {
            // Find our vehicle - we assume it exists... if not, we'll crash
            SimCore::Actors::PlatformActorProxy* vehicleProxy = NULL;
            GetGameManager()->FindActorById(playerStatus.GetAttachedVehicleID(), vehicleProxy);
            if (vehicleProxy != NULL)
            {
               SimCore::Actors::Platform* vehicle = NULL;
               vehicleProxy->GetActor(vehicle);
               AttachToVehicle(vehicle);
            }
            else // no vehicle to attach to, so just detach
            {
               DetachFromCurrentVehicle();
            }

         }

      }

   }

   //////////////////////////////////////////////////////////////
   bool InputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)
   {
      bool keyUsed = true;
      switch(key)
      {
         case 'p':
         {
            if (SimCore::Utils::IsDevModeOn(*GetGameManager()))
            {
               dtCore::ShaderManager::GetInstance().ReloadAndReassignShaderDefinitions("Shaders/ShaderDefs.xml");
               //ToggleEntityShaders();
               LOG_ALWAYS("Reloading All Shaders...");
            }
            break;
         }

         case '6':
            {
               ToggleDRGhost();
               break;
            }

         case 't':
            {
               /////////////////////////////////////////////////////////
               LOG_ALWAYS("TEST - HACK - CREATING TARGET!!! ");
               // Hack stuff - add a vehicle here. For testing purposes.
               dtCore::RefPtr<dtGame::GameActorProxy> testEnemyMine = NULL;
               SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
                  "Enemy Mine Prototype", testEnemyMine, "Check your additional maps in config.xml (compare to config_example.xml).");
               GetGameManager()->AddActor(*testEnemyMine, false, true);

               break;
            }

         case osgGA::GUIEventAdapter::KEY_Insert:
            {
               std::string developerMode;
               developerMode = GetGameManager()->GetConfiguration().GetConfigPropertyValue
                  (SimCore::BaseGameEntryPoint::CONFIG_PROP_DEVELOPERMODE, "false");
               if (developerMode == "true" || developerMode == "1")
               {
                  GetGameManager()->GetApplication().SetNextStatisticsType();
               }
            }
            break;

         case 'o':
            {
               // Go forward 5 mins in time
               IncrementTime(+5);
            }
            break;

         case 'i':
            {
               // go back 5 mins in time
               IncrementTime(-5);
            }
            break;

         case 'v':
            {
               if (mVehicle.valid())
               {
                  mCurrentViewPointIndex = (mCurrentViewPointIndex + 1) % mViewPointList.size();
                  SendAttachOrDetachMessage(mVehicle->GetUniqueId(), mViewPointList[mCurrentViewPointIndex]);
               }
            }
            break;
         case osgGA::GUIEventAdapter::KEY_Escape:
            {
               // Escapce key should act as one would expect, to escape from the
               // program in some manner, even if it means going through the menu system.
               GetLogicComponent()->DoStateTransition(&Transition::TRANSITION_BACK);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_Tab:
            {
               dtABC::Application& app = GetGameManager()->GetApplication();
               app.GetWindow()->SetFullScreenMode(!app.GetWindow()->GetFullScreenMode());
            }
            break;

         default:
            keyUsed = false;
      }

      if(!keyUsed)
         return BaseClass::HandleKeyPressed(keyboard, key);
      else
         return keyUsed;
   }

   /////////////////////////////////////////////////////////////////////////////
   GameLogicComponent* InputComponent::GetLogicComponent()
   {
      GameLogicComponent* comp = NULL;
      GetGameManager()->GetComponentByName( GameLogicComponent::DEFAULT_NAME, comp );

      if (comp == NULL)
      {
         LOG_ERROR( "Input Component cannot access the Game App Component." );
      }

      return comp;
   }

   /////////////////////////////////////////////////////////////////////////////
   void InputComponent::AttachToVehicle(SimCore::Actors::Platform* vehicle)
   {
      DetachFromCurrentVehicle();

      mVehicle = vehicle;
      if (vehicle == NULL) return;

      // NOTE - The camera sits at the bottom of a VERY large hierarchy of DoF's. Looks like this:
      //     Vehicle (center of vehicle)
      //       - Ring Mount (often swivels left/right)
      //           - mDoFWeapon (pivots about weapon pivot point)
      //               - mWeapon (3D model of weapon)
      //                   - mWeaponEyePoint (offset for human eyepoint)
      //                       - StealthActor (yay!  almost there)
      //                           - camera

      // Get the DOF's
      mDOFRing = mVehicle->GetNodeCollector()->GetDOFTransform(DOF_NAME_RINGMOUNT.Get());
      // Check for DOF's - toss an error or something
      if (!mDOFRing.valid())
      {
         LOG_ERROR("CRITICAL ERROR attaching to vehicle[" + vehicle->GetName() + "]. No DOF[" + DOF_NAME_RINGMOUNT.Get() + "]");
         //return;
      }
      mDOFWeapon = mVehicle->GetNodeCollector()->GetDOFTransform(DOF_NAME_WEAPON_PIVOT.Get());
      if (!mDOFWeapon.valid())
      {
         LOG_ERROR("CRITICAL ERROR attaching to vehicle[" + mVehicle->GetName() + "]. No DOF[" + DOF_NAME_WEAPON_PIVOT.Get() + "]");
         //return;
      }

      SendAttachOrDetachMessage(mVehicle->GetUniqueId(), DOF_NAME_WEAPON_PIVOT.Get());

      ///////////////////////////////////////////
      // Setup our Motion Models
      mWeaponMM->SetTargetDOF(mDOFWeapon.get());

      // Look up a weird quirk of the data (only set on the hover vehicle actor).
      // Is the turret 'hard-wired' to the vehicle? If so, the ring mount motion
      // model turns the vehicle, not just the ring mount DoF. This is true for
      // instance, with the Hover Vehicle.
      if(IsVehiclePivotable())
         mRingMM->SetTarget(mVehicle.get());
      else
         mRingMM->SetTargetDOF(mDOFRing.get());

      // Tell the MotionModels about the articulation helper - This allows the artic helper to
      // know about data changes which in turn, allows it to check for changes before publishing an update.
      mRingMM->SetArticulationHelper(mVehicle->GetArticulationHelper());
      mWeaponMM->SetArticulationHelper(mVehicle->GetArticulationHelper());

      EnableMotionModels();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InputComponent::DetachFromCurrentVehicle()
   {
      if (mVehicle.valid())
      {
         //mDOFWeapon->removeChild(GetStealthActor()->GetOSGNode());
         mWeaponMM->SetTargetDOF(NULL);
         mRingMM->SetTarget(NULL);
         mRingMM->SetTargetDOF(NULL);

         SendAttachOrDetachMessage(dtCore::UniqueId(""), "");

         // Re-enable our base motion model - so we have something
         mMotionModel->SetEnabled(GetStealthActor() != NULL);
      }

      mVehicle = NULL;
      EnableMotionModels();
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool InputComponent::IsVehiclePivotable()
   {
      HoverVehicleActor* hoverActor = dynamic_cast<HoverVehicleActor*>(mVehicle.get());
      return hoverActor != NULL && hoverActor->GetVehicleIsTurret();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::EnableMotionModels()
   {
      if (!mIsInGameState || !mVehicle.valid())
      {
         mWeaponMM->SetEnabled(false);
         mRingMM->SetEnabled(false);

         mMotionModel->SetEnabled(true);
      }
      else //if (mVehicle.valid()) //  Attached
      {
         bool enableVehicleModels = !mVehicle->IsFlamesPresent();
         mWeaponMM->SetEnabled(enableVehicleModels);
         mRingMM->SetEnabled(enableVehicleModels);

         mMotionModel->SetEnabled(false);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::SendAttachOrDetachMessage(
      const dtCore::UniqueId& vehicleId, const std::string& dofName)
   {
      dtCore::RefPtr<SimCore::AttachToActorMessage> msg;
      GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR, msg);
      msg->SetAboutActorId(GetStealthActor()->GetUniqueId());
      msg->SetAttachToActor(vehicleId);
      msg->SetAttachPointNodeName(dofName);
      GetGameManager()->SendMessage(*msg.get());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleDRGhost()
   {
      SimCore::Actors::BasePhysicsVehicleActor *mPhysVehicle =
         dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor*>(mVehicle.get());

      // If it already exists, then kill it.
      if (mDRGhostActorProxy.valid())
      {
         CleanUpDRGhost();
      }

      // Else, create it and put it in the world
      else if (mVehicle.valid() && mPhysVehicle != NULL)
      {
         LOG_ALWAYS("TEST - Enabling Ghost Dead Reckoning behavior.");
         GetGameManager()->CreateActor(*NetDemoActorRegistry::DR_GHOST_ACTOR_TYPE, mDRGhostActorProxy);
         if (mDRGhostActorProxy.valid())
         {
            mOriginalPublishTimesPerSecond = mPhysVehicle->GetTimesASecondYouCanSendOutAnUpdate();
            mPhysVehicle->SetTimesASecondYouCanSendOutAnUpdate(1.0f);
            mDRGhostActorProxy->GetActorAsDRGhost().SetSlavedEntity(mVehicle.get());
            GetGameManager()->AddActor(*mDRGhostActorProxy, false, false);
         }

      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::CleanUpDRGhost()
   {
      if (mDRGhostActorProxy.valid())
      {
         GetGameManager()->DeleteActor(*mDRGhostActorProxy.get());
         mDRGhostActorProxy = NULL;
         SimCore::Actors::BasePhysicsVehicleActor *mPhysVehicle =
            dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor*>(mVehicle.get());
         if (mVehicle.valid() && mPhysVehicle != NULL)
         {
            mPhysVehicle->SetTimesASecondYouCanSendOutAnUpdate(mOriginalPublishTimesPerSecond);
         }
      }

   }

}

