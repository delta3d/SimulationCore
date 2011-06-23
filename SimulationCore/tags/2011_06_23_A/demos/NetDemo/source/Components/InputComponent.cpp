/*
* Copyright, 2009-2010, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
 David Guthrie, Curtiss Murphy
*/
#include <dtUtil/mswin.h>
#include <Components/InputComponent.h>
#include <Components/WeaponComponent.h>
#include <osgGA/GUIEventAdapter>

#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messagefactory.h>
#include <dtABC/application.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/deltawin.h>

#include <SimCore/Utilities.h>
#include <SimCore/BaseGameEntryPoint.h>
#include <SimCore/ClampedMotionModel.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/GameState/GameStateChangeMessage.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/BaseEntity.h>
#include <dtGame/drpublishingactcomp.h>


#include <NetDemoUtils.h>
#include <States.h>
#include <ActorRegistry.h>
#include <Components/GameLogicComponent.h>
#include <Actors/PlayerStatusActor.h>
// TEMP STUFF FOR VEHICLE
#include <Actors/HoverVehicleActor.h>
#include <Actors/HoverVehiclePhysicsHelper.h>
#include <Actors/EnemyMine.h>

#include <osg/io_utils>
#include <iostream>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>
#include <NetDemoMessageTypes.h>


namespace NetDemo
{
   const dtUtil::RefString InputComponent::DOF_NAME_WEAPON_PIVOT("dof_gun_01");
   const dtUtil::RefString InputComponent::DOF_NAME_WEAPON_FIRE_POINT("dof_hotspot_01");
   const dtUtil::RefString InputComponent::DOF_NAME_RINGMOUNT("dof_turret_01");
   const dtUtil::RefString InputComponent::DOF_NAME_VIEW_01("dof_view_01");
   const dtUtil::RefString InputComponent::DOF_NAME_VIEW_02("dof_view_02");
   const dtUtil::RefString InputComponent::DOF_TOPDOWN_VIEW_01("dof_topdown_view_01");
   const dtUtil::RefString InputComponent::DOF_TOPDOWN_VIEW_02("dof_topdown_view_02");

   //////////////////////////////////////////////////////////////
   InputComponent::InputComponent(const std::string& name)
      : SimCore::Components::BaseInputComponent(name)
      , mDRGhostMode(NONE)
      , mDebugToggleMode(DEBUG_TOGGLE_DR_ALGORITHM)
      , mCurrentViewPointIndex(0)
      , mIsInGameState(false)
      , mOriginalPublishTimesPerSecond(3.0f)
      , mMaxPublishRate(5)
   {
      mViewPointList.push_back(DOF_NAME_WEAPON_PIVOT);
      mViewPointList.push_back(DOF_NAME_RINGMOUNT);
      mViewPointList.push_back(DOF_NAME_VIEW_01);
      mViewPointList.push_back(DOF_NAME_VIEW_02);
      mViewPointList.push_back(DOF_TOPDOWN_VIEW_01);
      mViewPointList.push_back(DOF_TOPDOWN_VIEW_02);
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
      mMotionModel->SetMaximumFlySpeed(5.0f);
      mMotionModel->SetMaximumTurnSpeed(90.0f);


      mRingMM = new SimCore::ClampedMotionModel(app.GetKeyboard(), app.GetMouse());
      mRingMM->SetMaximumMouseTurnSpeed(40.0f);
      mRingMM->SetUpDownLimit(0.0f);
      mRingMM->SetName("RingMM");

      mWeaponMM = new SimCore::ClampedMotionModel(app.GetKeyboard(), app.GetMouse());
      mWeaponMM->SetLeftRightLimit(0.0f);
      mWeaponMM->SetMaximumMouseTurnSpeed(70.0f);
      mWeaponMM->SetUpDownLimit(45.0f, 15.0f); // should probably be per vehicle
      mWeaponMM->SetName("WeaponMM");

      UpdateDebugInfo(true);
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
         // Find the actor in the GM
         PlayerStatusActorProxy* playerProxy;
         GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId(), playerProxy);

         if (playerProxy == NULL) // Could be deleted or not fully created from partial
         {
            return;
         }

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

               //turn the headlights on by default
               SetEnableHeadlight(true);
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
         case '\\':
         case osgGA::GUIEventAdapter::KEY_Insert:
         {
            SetNextStatisticsIfDevMode();
         }
         break;

         case 'p':
         {
            ReloadShadersIfDevMode();
         }
         break;

         case 'P':
         {
            SetNextPhysicsDebugDrawIfDevMode();
         }
         break;

         case 'g':
            {
               ToggleDRGhost();
               break;
            }

         case 'V':
            {
               ToggleVelocityDR();
               break;
            }

         // CLEAR OUT THE GHOST PARTICLES
         case '4':
            {
               if (mDRGhostActorProxy.valid())
               {
                  SimCore::Actors::DRGhostActor* ghost = NULL;
                  mDRGhostActorProxy->GetActor(ghost);
                  ghost->ClearLinesAndParticles();

                  // toggle the visible arrows
                  ghost->SetArrowDrawScalar((ghost->GetArrowDrawScalar() > 0) ? 0.0f : 1.0f);
               }
               break;
            }

         case '5':
            {
               ModifyVehicleSmoothingRate(0.90);
               break;
            }
         case '6':
            {
               ModifyVehicleSmoothingRate(1.10);
               break;
            }
         case '7':
            {
               ModifyVehiclePublishRate(-1);//1.10);
               break;
            }
         case '8':
            {
               ModifyVehiclePublishRate(+1);//0.901);
               break;
            }
         case '9':
            {
               ChangeDebugToggleMode();
               //ResetTestingValues();
               break;
            }

         case '0':
            {
               ToggleCurrentDebugMode();
               //ToggleGroundClamping();
               break;
            }
         case 't':
            {
               /////////////////////////////////////////////////////////
               //LOG_ALWAYS("TEST - HACK - CREATING TARGET!!! ");
               // Hack stuff - add a vehicle here. For testing purposes.
               //dtCore::RefPtr<dtGame::GameActorProxy> testEnemyMine = NULL;
               //SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(),
               //   "Enemy Mine Prototype", testEnemyMine, "Check your additional maps in config.xml (compare to config_example.xml).");
               //GetGameManager()->AddActor(*testEnemyMine, false, true);

               break;
            }

         case osgGA::GUIEventAdapter::KEY_Delete:
            {
               // Delete all if shift held, otherwise, just one.
               bool deleteAll = keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Shift_L) ||
                  keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Shift_R);
               KillEnemy(deleteAll);
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

         case 'h':
            {
               WeaponComponent* comp = NULL;
               GetGameManager()->GetComponentByName(WeaponComponent::DEFAULT_NAME, comp);

               comp->SetCurrentWeaponIndex((comp->GetCurrentWeaponIndex() + 1) % comp->GetNumWeapons());
            }
            break;


         case 'l':
            {
               if(mVehicle.valid())
               {
                  SetEnableHeadlight(!mVehicle->IsHeadLightsEnabled());
               }
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

         case 'u':
            {
               ToggleWeatherStateIfDevMode();
            }
            break;

         case 'r':
            {
               if (mVehicle.valid())
               {
                  // Repair our damage state - done by the munitions component
                  SimCore::Components::MunitionsComponent *munitionsComp =
                     static_cast<SimCore::Components::MunitionsComponent*>
                     (GetGameManager()->GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME));
                  munitionsComp->SetDamage( *mVehicle, SimCore::Components::DamageType::DAMAGE_NONE );

                  // Turn off flames. Doesn't necessarily go out when damage is reset.
                  mVehicle->SetFlamesPresent(false);

                  EnableMotionModels();
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

         case osgGA::GUIEventAdapter::KEY_Up:
         case osgGA::GUIEventAdapter::KEY_Left:
            {
               SendSimpleMessage(NetDemo::MessageType::UI_OPTION_PREV);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_Down:
         case osgGA::GUIEventAdapter::KEY_Right:
            {
               SendSimpleMessage(NetDemo::MessageType::UI_OPTION_NEXT);
            }
            break;

         //case osgGA::GUIEventAdapter::KEY_Space:
         case osgGA::GUIEventAdapter::KEY_Return:
            {
               SendSimpleMessage(NetDemo::MessageType::UI_OPTION_SELECT);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F1:
            {
               SendSimpleMessage(NetDemo::MessageType::UI_HELP);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_F2:
            {
               ToggleDebugInfo();
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
   /////////////////////////////////////////////////////////////////////////////////
   bool InputComponent::HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button)
   {
      bool handled = false;


      if( button == dtCore::Mouse::LeftButton )
      {
         WeaponComponent* comp = NULL;
         GetGameManager()->GetComponentByName(WeaponComponent::DEFAULT_NAME, comp);
         if (comp != NULL)
         {
            comp->StartFiring();
         }
         handled = true;
      }

      if(!handled)
         return BaseClass::HandleButtonPressed(mouse, button);

      return handled;
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool InputComponent::HandleButtonReleased(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button)
   {
      bool handled = false;

      // stop firing
      if( button == dtCore::Mouse::LeftButton )
      {
         WeaponComponent* comp = NULL;
         GetGameManager()->GetComponentByName(WeaponComponent::DEFAULT_NAME, comp);
         if (comp != NULL)
         {
            comp->StopFiring();
         }
         handled = true;
      }

      if(!handled)
         return BaseClass::HandleButtonReleased(mouse, button);

      return handled;
   }


   /////////////////////////////////////////////////////////////////////////////
   GameLogicComponent* InputComponent::GetLogicComponent()
   {
      GameLogicComponent* comp = NULL;
      GetGameManager()->GetComponentByName(GameLogicComponent::DEFAULT_NAME, comp);

      if (comp == NULL)
      {
         LOG_INFO("Input Component cannot access the Game App Component. Usually happens when shutting down." );
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

      SendAttachOrDetachMessage(mVehicle->GetUniqueId(), DOF_NAME_VIEW_02.Get());

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

      /////////////////////////////////
      // Add 2 topdown nodes, for debugging purposes.
      AddTopDownNode(25.0f, DOF_TOPDOWN_VIEW_01);
      AddTopDownNode(35.0f, DOF_TOPDOWN_VIEW_02);

      UpdateDebugInfo(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   void InputComponent::AddTopDownNode(float height, const dtUtil::RefString& nodeName)
   {
      if (!mVehicle.valid())
      {
         return;
      }

      // Make sure the node doesn't already exist. 
      osg::Group* group = NULL;
      group = mVehicle->GetNodeCollector()->GetMatrixTransform(nodeName);
      if (group == NULL)
      {
         group = mVehicle->GetNodeCollector()->GetDOFTransform(nodeName);
      }

      // We didn't find it, so we continue.
      if (group == NULL)
      {
         // Create a new node
         dtCore::RefPtr<osg::MatrixTransform> topdownNode = new osg::MatrixTransform();
         // Set it's pos and name
         osg::Matrix transUp = osg::Matrix::translate(0.0f, 0.0f, height); // move us up X meters.
         osg::Matrix rotDown = osg::Matrix::rotate(-M_PI*0.5, 1, 0, 0); // look down at us
         osg::Matrix offset = rotDown * transUp;
         topdownNode->setMatrix(offset);
         topdownNode->setName(nodeName);
         // And add it as a child so we can attach to it.
         osg::ref_ptr<osg::Group> vehicleRootNode(dynamic_cast<osg::Group*>(mVehicle->GetOSGNode()));
         vehicleRootNode->addChild(topdownNode.get());
      }
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

      UpdateDebugInfo(true);
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
         bool enableVehicleModels = !mVehicle->GetFlamesPresent();
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
      msg->SetAboutActorId((GetStealthActor() == NULL) ? (dtCore::UniqueId("")) : GetStealthActor()->GetUniqueId());
      msg->SetAttachToActor(vehicleId);
      msg->SetAttachPointNodeName(dofName);
      GetGameManager()->SendMessage(*msg.get());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::SendSimpleMessage(const NetDemo::MessageType& messageType)
   {
      MessageUtils::SendSimpleMessage(messageType, *GetGameManager());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleDRGhost()
   {
      SimCore::Actors::BasePhysicsVehicleActor* mPhysVehicle =
         dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor*>(mVehicle.get());

      // basic error check.
      if (!mVehicle.valid() || mPhysVehicle == NULL)
      {
         CleanUpDRGhost();
         return;
      }

      // state - NONE, go to GHOST_ON
      if (mDRGhostMode == NONE)
      {
         // create ghost and add to world
         LOG_ALWAYS("Enabling Ghost Dead Reckoning behavior.");
         GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::DR_GHOST_ACTOR_TYPE, mDRGhostActorProxy);
         if (mDRGhostActorProxy.valid())
         {
            mOriginalPublishTimesPerSecond = mPhysVehicle->GetComponent<dtGame::DRPublishingActComp>()->GetMaxUpdateSendRate();
            SimCore::Actors::DRGhostActor* actor = NULL;
            mDRGhostActorProxy->GetActor(actor);
            actor->SetSlavedEntity(mVehicle);
            GetGameManager()->AddActor(*mDRGhostActorProxy, false, false);
         }
         mDRGhostMode = GHOST_ON;
      }

      // state - GHOST_ON, go to ATTACH_TO_GHOST
      else if (mDRGhostMode == GHOST_ON)
      {
         LOG_ALWAYS(" --- Attaching Camera to DR Ghost.");
         if (mDRGhostActorProxy.valid())
         {
            SendAttachOrDetachMessage(mDRGhostActorProxy->GetGameActor().GetUniqueId(), "");
         }
         mDRGhostMode = ATTACH_TO_GHOST;
      }

      // state - ATTACH_TO_GHOST, go to HIDE_REAL
      else if (mDRGhostMode == ATTACH_TO_GHOST)
      {
         LOG_ALWAYS(" --- Hiding Real Vehicle.");
         if (mDRGhostActorProxy.valid())
         {
            mPhysVehicle->SetVisible(false);
         }
         mDRGhostMode = HIDE_REAL;
      }

      // state - HIDE_REAL go to DETACH_FROM_VEHICLE
      else if (mDRGhostMode == HIDE_REAL)
      {
         LOG_ALWAYS(" --- Bye bye Ghost AND No attach to Real. Just sit still");
         mPhysVehicle->SetVisible(true);
         CleanUpDRGhost();
         mDRGhostMode = DETACH_FROM_VEHICLE;
      }

      // state - DETACH_FROM_VEHICLE, go to NONE
      else if (mDRGhostMode == DETACH_FROM_VEHICLE)
      {
         LOG_ALWAYS(" --- Removing DR and re-attaching to Real Vehicle.");
         SendAttachOrDetachMessage(mPhysVehicle->GetUniqueId(), mViewPointList[mCurrentViewPointIndex]);
         mDRGhostMode = NONE;
      }

      UpdateDebugInfo(true);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleVelocityDR()
   {
      dtABC::Application& app = GetGameManager()->GetApplication();
      bool ctrlIsPressed = app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Control_L) ||
         app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Control_R);

      if (mVehicle.valid())
      {
         dtGame::DRPublishingActComp* drPubAC = NULL;
         mVehicle->GetComponent(drPubAC);

         if (drPubAC != NULL)
         {
            if (!ctrlIsPressed)
            {
               drPubAC->SetPublishLinearVelocity(
                  !drPubAC->GetPublishLinearVelocity());
               std::cout << "TEST - Publish Linear Velocity changed to [" << drPubAC->GetPublishLinearVelocity() <<
                  "]. Ctrl to change VelDRDecision." << std::endl;
            }
            else
            {
               drPubAC->SetUseVelocityInDRUpdateDecision(
                  !drPubAC->GetUseVelocityInDRUpdateDecision());
               std::cout << "Toggle - UseVelocity in DR Update Decision changed to [" <<
                     drPubAC->GetUseVelocityInDRUpdateDecision() << "]." << std::endl;
            }
         }

         // This whole method is not really used anymore, so if needed in the future,
         // add a call to UpdateDebugInfo(true).  
         //UpdateDebugInfo(true);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::CleanUpDRGhost()
   {
      if (mDRGhostActorProxy.valid())
      {
         GetGameManager()->DeleteActor(*mDRGhostActorProxy.get());
         mDRGhostActorProxy = NULL;
         if (mVehicle.valid())
         {
            dtGame::DRPublishingActComp* drPubAC = NULL;
            mVehicle->GetComponent(drPubAC);
            if (drPubAC != NULL)
            {
               drPubAC->SetMaxUpdateSendRate(mOriginalPublishTimesPerSecond);
            }
         }
         mDRGhostMode = NONE;
      }

   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ModifyVehiclePublishRate(int incrementValue)
   {
      SimCore::Actors::BasePhysicsVehicleActor* physVehicle =
         dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor*>(mVehicle.get());
      if (physVehicle != NULL)
      {
         mMaxPublishRate += incrementValue;
         dtUtil::Clamp(mMaxPublishRate, 0, 60);
         dtGame::DRPublishingActComp* drPubAC = NULL;
         physVehicle->GetComponent(drPubAC);
         if (drPubAC != NULL)
         {
            drPubAC->SetMaxUpdateSendRate(float(mMaxPublishRate));
            std::cout << "TEST - Num Publishes Per Second[" << mMaxPublishRate <<  "]." << std::endl;
         }
         /*
         float timesPerSecondRate = physVehicle->GetDRPublishingActComp()->GetMaxUpdateSendRate();
         timesPerSecondRate *= scaleFactor;
         physVehicle->GetDRPublishingActComp()->SetMaxUpdateSendRate(timesPerSecondRate);
         float rateInSeconds = 1.0f / timesPerSecondRate;
         //mVehicle->GetDeadReckoningHelper().SetMaxRotationSmoothingTime(0.97 * rateInSeconds);
         //mVehicle->GetDeadReckoningHelper().SetMaxTranslationSmoothingTime(0.97 * rateInSeconds);
         std::cout << "TEST - Min time between publishes[" << rateInSeconds <<  "]." << std::endl;
         */

         UpdateDebugInfo(true);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ModifyVehicleSmoothingRate(float scaleFactor)
   {
      if (mVehicle.valid())
      {
         // Like the publish method, only works on the smoothing rate.
         dtABC::Application& app = GetGameManager()->GetApplication();
         bool ctrlIsPressed = app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Control_L) ||
            app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Control_R);

         dtGame::DeadReckoningHelper* drHelper = NULL;
         mVehicle->GetComponent(drHelper);

         if (drHelper != NULL)
         {
            if (!ctrlIsPressed)
            {
               float oldRate = drHelper->GetMaxTranslationSmoothingTime();
               float newRate = oldRate * scaleFactor;
               drHelper->SetMaxTranslationSmoothingTime(newRate);

               std::cout << "-- Changed TRANS Smoothing Time to [" << newRate <<  "]. Hold CTRL to change Rot." << std::endl;
            }
            else
            {
               float oldRate = drHelper->GetMaxRotationSmoothingTime();
               float newRate = oldRate * scaleFactor;
               drHelper->SetMaxRotationSmoothingTime(newRate);

               std::cout << "-- Changed ROT Smoothing Time to [" << newRate <<  "]." << std::endl;
            }
         }

      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ChangeDebugToggleMode()
   {
      if (mDebugToggleMode == DEBUG_TOGGLE_DR_ALGORITHM)
      {
         mDebugToggleMode = DEBUG_TOGGLE_PUBLISH_ANGULAR_VELOCITY;
         LOG_ALWAYS("DEBUG -- Changing Toggle Mode to PUBLISH_ANGULAR_VELOCITY. Press 0 to do toggle.");
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_PUBLISH_ANGULAR_VELOCITY)
      {
         mDebugToggleMode = DEBUG_TOGGLE_DR_WITH_CUBIC_SPLINE;
         LOG_ALWAYS("DEBUG -- Changing Toggle Mode to DR_WITH_CUBIC_SPLINE. Press 0 to do toggle.");
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_DR_WITH_CUBIC_SPLINE)
      {
         mDebugToggleMode = DEBUG_TOGGLE_GROUND_CLAMPING;
         LOG_ALWAYS("DEBUG -- Changing Toggle Mode to GROUND_CLAMPING. Press 0 to do toggle.");
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_GROUND_CLAMPING)
      {
         mDebugToggleMode = DEBUG_FIXED_BLEND_TIME;
         LOG_ALWAYS("DEBUG -- Changing Toggle Mode to DEBUG_FIXED_BLEND_TIME. Press 0 to do toggle.");
      }
      else if (mDebugToggleMode == DEBUG_FIXED_BLEND_TIME)
      {
         mDebugToggleMode = DEBUG_TOGGLE_DR_ALGORITHM;
         LOG_ALWAYS("DEBUG -- Changing Toggle Mode to DR_ALGORITHM. Press 0 to do toggle.");
      }

      UpdateDebugInfo(true);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleCurrentDebugMode()
   {
      if (mDebugToggleMode == DEBUG_TOGGLE_DR_ALGORITHM)
      {
         ToggleDeadReckoningAlgorithm();
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_PUBLISH_ANGULAR_VELOCITY)
      {
         TogglePublishAngularVelocity();
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_DR_WITH_CUBIC_SPLINE)
      {
         ToggleUseCubicSplineForDR();
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_GROUND_CLAMPING)
      {
         ToggleGroundClamping();
      }
      else if (mDebugToggleMode == DEBUG_FIXED_BLEND_TIME)
      {
         ToggleFixedBlendTime();
      }

      UpdateDebugInfo(true);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleFixedBlendTime()
   {
      // Toggle the publishing of angular velocity
      if (mVehicle.valid())
      {
         dtGame::DeadReckoningHelper* drHelper = NULL;
         mVehicle->GetComponent(drHelper);

         if (drHelper!= NULL)
         {
            if (drHelper->GetUseFixedSmoothingTime())
            {
               drHelper->SetUseFixedSmoothingTime(false);
               LOG_ALWAYS("TEST -- Toggling - Always Use Max Smoothing Time to FALSE");
            }
            else
            {
               drHelper->SetUseFixedSmoothingTime(true);
               LOG_ALWAYS("TEST -- Toggling - Always Use Max Smoothing Time to TRUE");
            }
         }
      }

   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::TogglePublishAngularVelocity()
   {
      // Toggle the publishing of angular velocity

      if (mVehicle.valid())
      {
         dtGame::DRPublishingActComp* drPubAC = NULL;
         mVehicle->GetComponent(drPubAC);

         if (drPubAC != NULL)
         {
            if (drPubAC->GetPublishAngularVelocity())
            {
               drPubAC->SetPublishAngularVelocity(false);
               LOG_ALWAYS("TEST -- Toggling - Publish Angular Velocity to FALSE");
            }
            else
            {
               drPubAC->SetPublishAngularVelocity(true);
               LOG_ALWAYS("TEST -- Toggling - Publish Angular Velocity to TRUE");
            }
         }
      }
      //GetLogicComponent()->GetDebugInfo().mCurDebugVar = "DR Algorithm";
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleUseCubicSplineForDR()
   {
      // Toggle the Use Cubic Spline
      if (mVehicle.valid())
      {
         dtGame::DeadReckoningHelper* drHelper = NULL;
         mVehicle->GetComponent(drHelper);

         if (drHelper != NULL)
         {
            if (drHelper->GetUseCubicSplineTransBlend())
            {
               drHelper->SetUseCubicSplineTransBlend(false);
               LOG_ALWAYS("TEST -- Toggling - Use CUBIC Spline - FALSE");
            }
            else
            {
               drHelper->SetUseCubicSplineTransBlend(true);
               LOG_ALWAYS("TEST -- Toggling - Use CUBIC Spline - TRUE");
            }
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleDeadReckoningAlgorithm()
   {
      if (mVehicle.valid())
      {
         dtGame::DeadReckoningHelper* drHelper = NULL;
         mVehicle->GetComponent(drHelper);

         if (drHelper != NULL)
         {
            if (drHelper->GetDeadReckoningAlgorithm()
               == dtGame::DeadReckoningAlgorithm::STATIC)
            {
               drHelper->SetDeadReckoningAlgorithm
                  (dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);
               LOG_ALWAYS("TEST -- Toggling - DR Algorithm to Velocity Only. ");
            }
            else if (drHelper->GetDeadReckoningAlgorithm()
               == dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY)
            {
               drHelper->SetDeadReckoningAlgorithm
                  (dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION);
               LOG_ALWAYS("TEST -- Toggling - DR Algorithm to Velocity AND Acceleration. ");
            }
            else if (drHelper->GetDeadReckoningAlgorithm()
               == dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION)
            {
               drHelper->SetDeadReckoningAlgorithm
                  (dtGame::DeadReckoningAlgorithm::STATIC);
               LOG_ALWAYS("TEST -- Toggling - DR Algorithm to STATIC . ");
            }
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleGroundClamping()
   {
      if (mVehicle.valid())
      {
         dtGame::DeadReckoningHelper* drHelper = NULL;
         mVehicle->GetComponent(drHelper);

         if (drHelper != NULL)
         {
            if (drHelper->GetGroundClampType() == dtGame::GroundClampTypeEnum::NONE)
            {
               LOG_ALWAYS("TEST -- Toggling - DR Ground Clamping Type is now [KEEP_ABOVE].");
               drHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::KEEP_ABOVE);
            }
            else if (drHelper->GetGroundClampType() == dtGame::GroundClampTypeEnum::KEEP_ABOVE)
            {
               LOG_ALWAYS("TEST -- Toggling - DR Ground Clamping Type is now [FULL].");
               drHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::FULL);
            }
            else if (drHelper->GetGroundClampType() == dtGame::GroundClampTypeEnum::FULL)
            {
               LOG_ALWAYS("TEST -- Toggling - DR Ground Clamping Type is now [NONE].");
               drHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::NONE);
            }
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ToggleDebugInfo()
   {
      GameLogicComponent* glComp = GetLogicComponent();

      if (glComp != NULL) 
      {
         glComp->GetDebugInfo().mShowDebugWindow = !glComp->GetDebugInfo().mShowDebugWindow;
         UpdateDebugInfo(true);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::UpdateDebugInfo(bool sendUpdateMessage)
   {
      GameLogicComponent* glComp = GetLogicComponent();

      if (glComp == NULL)
      {
         return; // nothing to do without a Game logic component
      }

      dtGame::DeadReckoningHelper* drHelper = NULL;
      dtGame::DRPublishingActComp* drPubAC = NULL;
      if (mVehicle.valid())
      {
         mVehicle->GetComponent(drHelper);
         mVehicle->GetComponent(drPubAC);
      }

      if (drHelper != NULL)
      {
         glComp->GetDebugInfo().mDRGroundClampStatus =
            drHelper->GetGroundClampType().GetName();
         glComp->GetDebugInfo().mDRAlgorithm =
            drHelper->GetDeadReckoningAlgorithm().GetName();
         glComp->GetDebugInfo().mDRUseSplines =
            drHelper->GetUseCubicSplineTransBlend();
         glComp->GetDebugInfo().mDRUseFixedBlend =
            drHelper->GetUseFixedSmoothingTime();
      }
      else
      {
         glComp->GetDebugInfo().mDRGroundClampStatus = "NA";
         glComp->GetDebugInfo().mDRAlgorithm = "NA";
         glComp->GetDebugInfo().mDRUseSplines = "NA";
      }


      if (drPubAC != NULL)
      {
         glComp->GetDebugInfo().mDRPublishRate = (int)
            drPubAC->GetMaxUpdateSendRate();
         glComp->GetDebugInfo().mDRPublishAngularVel =
            drPubAC->GetPublishAngularVelocity();
      }
      else  // not attached to a vehicle
      {
         glComp->GetDebugInfo().mDRAvgSpeed = 0.0f;
         glComp->GetDebugInfo().mDRAvgError = 0.0f;
         glComp->GetDebugInfo().mDRPublishRate = 0;
         glComp->GetDebugInfo().mDRUseFixedBlend = false;
         glComp->GetDebugInfo().mDRPublishAngularVel = false;
         glComp->GetDebugInfo().mDRGroundClampStatus = "NA";
         glComp->GetDebugInfo().mDRAlgorithm = "NA";
         glComp->GetDebugInfo().mDRUseSplines = "NA";
      }

      // Current Debug Var
      if (mDebugToggleMode == DEBUG_TOGGLE_DR_ALGORITHM)
      {
         glComp->GetDebugInfo().mCurDebugVar = "DR Algorithm";
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_PUBLISH_ANGULAR_VELOCITY)
      {
         glComp->GetDebugInfo().mCurDebugVar = "DR Publish Ang Vel";
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_DR_WITH_CUBIC_SPLINE)
      {
         glComp->GetDebugInfo().mCurDebugVar = "DR Blending Type";
      }
      else if (mDebugToggleMode == DEBUG_TOGGLE_GROUND_CLAMPING)
      {
         glComp->GetDebugInfo().mCurDebugVar = "DR Ground Clamp";
      }
      else if (mDebugToggleMode == DEBUG_FIXED_BLEND_TIME)
      {
         glComp->GetDebugInfo().mCurDebugVar = "DR Fixed Blend";
      }
      else
      {
         glComp->GetDebugInfo().mCurDebugVar = "UNKNOWN";
      }

      // GHOST MODE
      if (mDRGhostMode == NONE)
      {
         glComp->GetDebugInfo().mDRGhostMode = "OFF";
      }
      else if (mDRGhostMode == GHOST_ON)
      {
         glComp->GetDebugInfo().mDRGhostMode = "ON";
      }
      else if (mDRGhostMode == ATTACH_TO_GHOST)
      {
         glComp->GetDebugInfo().mDRGhostMode = "Attach Ghost";
      }
      else if (mDRGhostMode == HIDE_REAL)
      {
         glComp->GetDebugInfo().mDRGhostMode = "Hide Real";
      }
      else if (mDRGhostMode == DETACH_FROM_VEHICLE)
      {
         glComp->GetDebugInfo().mDRGhostMode = "Detached";
      }


      if (sendUpdateMessage)
      {
         SendSimpleMessage(NetDemo::MessageType::UI_DEBUGINFO_UPDATED);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::ResetTestingValues()
   {
      std::cout << "Resetting DR values such as publish and smoothing." << std::endl;
      if (mVehicle.valid())
      {
         dtGame::DRPublishingActComp* drPubAC = NULL;
         mVehicle->GetComponent(drPubAC);
         if (drPubAC != NULL)
         {
            drPubAC->SetMaxUpdateSendRate(3.0f);
            drPubAC->SetUseVelocityInDRUpdateDecision(true);
            drPubAC->SetPublishLinearVelocity(true);
         }

         dtGame::DeadReckoningHelper* drHelper = NULL;
         mVehicle->GetComponent(drHelper);
         if (drHelper != NULL)
         {
            drHelper->SetMaxRotationSmoothingTime(1.0f);
            drHelper->SetMaxTranslationSmoothingTime(1.0f);
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void InputComponent::KillEnemy(bool killAllEnemies)
   {
      // We use the Munitions Component to do the damage to the object
      SimCore::Components::MunitionsComponent* munitionsComp = NULL;
      GetGameManager()->GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME, munitionsComp);
      if (munitionsComp == NULL)
      {
         LOG_ERROR("No Munitions Component. ERROR!");
         return;
      }

      // Look for a enemy actor.
      std::vector<dtGame::GameActorProxy*> allGameActors;
      GetGameManager()->GetAllGameActors(allGameActors);
      // Iterate through all the game actors to find one of our enemies.
      unsigned int numActors = allGameActors.size();
      for(unsigned i = 0; i < numActors; i++)
      {
         // Find an entity that is not already destroyed and is also a mine, helix, etc...
         SimCore::Actors::BaseEntity* entity = dynamic_cast<SimCore::Actors::BaseEntity*>(allGameActors[i]->GetActor());
         if (entity != NULL && entity->GetDamageState() != SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED && 
            (allGameActors[i]->GetActorType() == *NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE ||
            allGameActors[i]->GetActorType() == *NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE))
         {
            munitionsComp->SetDamage(*entity, SimCore::Components::DamageType::DAMAGE_KILL);
               //SimCore::Components::DamageStateEnum::DESTROYED);

            // Stop after one or keep going for all
            if (!killAllEnemies)
               break;
         }
      }

   }

   void InputComponent::SetEnableHeadlight(bool b)
   {
      if(mVehicle.valid() && !mVehicle->IsRemote())
      {
         mVehicle->SetHeadLightsEnabled(b);

         SimCore::Components::RenderingSupportComponent* rsComp = NULL;
         SimCore::Components::RenderingSupportComponent::SpotLight* dl = NULL;

         GetGameManager()->GetComponentByName( SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsComp);

         if (rsComp != NULL)
         {
            // ...so that the head light effect can be accessed...
            dl = dynamic_cast<SimCore::Components::RenderingSupportComponent::SpotLight*>(rsComp->GetDynamicLight(mVehicle->GetHeadlightId()));

            // ...and if the light effect does not exist...
            if(dl != NULL)
            {
               dl->mTarget = mVehicle.get();
            }
         }

         mVehicle->GetGameActorProxy().NotifyFullActorUpdate();
      }
   }
}

