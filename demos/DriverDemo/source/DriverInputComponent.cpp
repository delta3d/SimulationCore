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
#include <DriverInputComponent.h>
#include <DriverHUD.h>
#include <DriverActorRegistry.h>

#include <dtABC/application.h>

#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>

#include <dtDAL/actorproxy.h>
#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtCore/logicalinputdevice.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtUtil/nodecollector.h>

#include <dtUtil/enumeration.h>
#include <dtUtil/nodeprintout.h>

#include <SimCore/Actors/MunitionParticlesActor.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <SimCore/Actors/TextureProjectorActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/MissileActor.h> // for missile load testing
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/VehicleInterface.h>
#include <SimCore/Actors/VehicleAttachingConfigActor.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

#include <SimCore/StealthMotionModel.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/CommandLineObject.h>
#include <SimCore/ClampedMotionModel.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Tools/Tool.h>
#include <SimCore/Utilities.h>

#include <dtGame/actorupdatemessage.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messagetype.h>
#include <dtGame/gameactor.h>
#include <dtGame/binarylogstream.h>
#include <dtGame/logtag.h>
#include <dtGame/logkeyframe.h>
#include <dtGame/logstatus.h>
#include <dtGame/loggermessages.h>
#include <dtGame/logcontroller.h>
#include <dtGame/serverloggercomponent.h>
#include <dtGame/gameapplication.h>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>

#include <iostream>


//#ifdef AGEIA_PHYSICS
#include <dtPhysics/physicscomponent.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/Actors/HumanWithPhysicsActor.h>
#include <dtDAL/project.h>
//#endif

// TEST INCLUDES --- START
#include <dtDAL/groupactorproperty.h>
// TEST INCLUDES --- END

#include <DriverArticulationHelper.h>
#include <HoverVehicleActor.h>
#include <HoverTargetActor.h>

#include <SimCore/Actors/FlareActor.h>
#include <dtUtil/matrixutil.h>

#include <SimCore/BaseGameEntryPoint.h>
#ifdef BUILD_HLA
#include <dtHLAGM/hlacomponent.h>
#endif
using std::shared_ptr;

namespace DriverDemo
{

   // CurtTest
//   static DVTE::Actors::FloatTargetActor* mStaticLastTarget = nullptr;

   ////////////////////////////////////////////////////////////////////////////////
   // NOTE: The files referenced here WILL change; and may even be contained in
   //       another actor that will allow these to be set by an xml file.
   //       For now, the referenced files are place holders for the real
   //       sound effects to be used.
   const dtUtil::RefString DriverInputComponent::SOUND_TURRET_TURN_START("Sounds/RAW_SFX_Turrent_Slide.wav");
   const dtUtil::RefString DriverInputComponent::SOUND_TURRET_TURN("Sounds/RAW_SFX_Turrent_Slide.wav");
   const dtUtil::RefString DriverInputComponent::SOUND_TURRET_TURN_END("Sounds/RAW_SFX_Turrent_Slide.wav");

   const dtUtil::RefString DriverInputComponent::DOF_NAME_WEAPON_PIVOT("dof_gun_01");
   const dtUtil::RefString DriverInputComponent::DOF_NAME_WEAPON_FIRE_POINT("dof_hotspot_01");
   const dtUtil::RefString DriverInputComponent::DOF_NAME_RINGMOUNT("dof_turret_01");
   const dtUtil::RefString DriverInputComponent::DOF_NAME_VIEW_01("dof_view_01");
   const dtUtil::RefString DriverInputComponent::DOF_NAME_VIEW_02("dof_view_02");
   const dtUtil::RefString DriverInputComponent::DOF_NAME_VIEW_DEFAULT(DriverInputComponent::DOF_NAME_VIEW_02.Get());

   ////////////////////////////////////////////////////////////////////////////////
   DriverInputComponent::DriverInputComponent(const std::string& name) :
      BaseClass(name),
      mUsePhysicsDemoMode(false),
      mHorizontalFOV(60.0f),
      mVerticalFOV(60.0f),
      mNearClip(SimCore::BaseGameEntryPoint::PLAYER_NEAR_CLIP_PLANE),
      mFarClip(SimCore::BaseGameEntryPoint::PLAYER_FAR_CLIP_PLANE),
      mRingKeyHeld(false),
      mRingButtonHeld(false),
      mMotionModelsEnabled(false),
      mPlayerAttached(false),
      mLastDamageState(&SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE),
      mViewNodeName(DOF_NAME_VIEW_DEFAULT.Get()),
      mEnvUpdateTime(0.0f),
      mEnvUpdateAttempts(2),
      mDRGhostMode(NONE)
   {
      mMachineInfo = new dtGame::MachineInfo;

      SetStartPosition(osg::Vec3(100.0,100.0,20.0));
   }

   ////////////////////////////////////////////////////////////////////////////////
   DriverInputComponent::~DriverInputComponent()
   {
      if( mSoundAmbient.valid() )
      {
         mSoundAmbient.release();
         mSoundAmbient = nullptr;
      }
      if( mSoundTurretTurn.valid() )
      {
         mSoundTurretTurn.release();
         mSoundTurretTurn = nullptr;
      }
      if( mSoundTurretTurnStart.valid() )
      {
         mSoundTurretTurnStart.release();
         mSoundTurretTurnStart = nullptr;
      }
      if( mSoundTurretTurnEnd.valid() )
      {
         mSoundTurretTurnEnd.release();
         mSoundTurretTurnEnd = nullptr;
      }

      mDRGhostActorProxy = nullptr;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& msgType = message.GetMessageType();
      if (msgType == dtGame::MessageType::TICK_LOCAL)
      {
         const dtGame::TickMessage& tick = static_cast<const dtGame::TickMessage&>(message);
         UpdateStates( tick.GetDeltaSimTime(), tick.GetDeltaRealTime() );
      }

      // Tool update message.
      else if(dynamic_cast<const SimCore::ToolMessage*>(&message) != nullptr)
      {
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
         SetToolEnabled(
            static_cast<SimCore::MessageType&>(const_cast<dtGame::MessageType&>(toolMsg.GetMessageType())),
            toolMsg.GetEnabled());
      }

      // A Local Player entered world, so create our motion models
      else if (msgType == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD &&
         message.GetSource() == GetGameManager()->GetMachineInfo())
      {
         dtGame::GameActorProxy* stealthProxy = GetGameManager()->FindGameActorById(message.GetAboutActorId());
         if(stealthProxy == nullptr)
         {
            GetLogger().LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "A player entered world message was received, but the about actor id does not refer to a Game Actor in the Game Manager.");
            return;
         }

         SimCore::Actors::StealthActor* stealthActor
            = dynamic_cast<SimCore::Actors::StealthActor*>(&stealthProxy->GetGameActor());
         if(stealthActor == nullptr)
         {
            GetLogger().LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "A player entered world message was received, but the actor is not the right type.");
         }
         else if(!stealthProxy->IsRemote())
         {
            InitializePlayer( *stealthActor );
            InitializeSounds( *stealthActor );
         }
      }

      // Actor was deleted. Clear out our stealth or vehicle if appropriate
      else if (msgType == dtGame::MessageType::INFO_ACTOR_DELETED)
      {
         const dtCore::UniqueId& id = message.GetAboutActorId();
         SimCore::Actors::StealthActor* stealth = GetStealthActor();
         if (stealth != nullptr && stealth->GetUniqueId() == id)
         {
            SetStealthActor(nullptr);
         }
         else if (mVehicle.valid() && mVehicle->GetUniqueId() == id)
         {
            DriverDemo::GameAppComponent* gameAppComponent;
            GetGameManager()->GetComponentByName(DriverDemo::GameAppComponent::DEFAULT_NAME, gameAppComponent);
            if (gameAppComponent != nullptr)
            {
               GetLogger().LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                  "Vehicle deleted.  This is normal during shutdown.");
               mVehicle = nullptr;
            }
         }
      }

      // Map first loaded. This only happens once, so we can do most of our inits now.
      else if(msgType == dtGame::MessageType::INFO_MAP_LOADED)
      {
         dtGame::GameManager& gameManager = *GetGameManager();

         DriverDemo::GameAppComponent* gameAppComponent;
         gameManager.GetComponentByName(DriverDemo::GameAppComponent::DEFAULT_NAME, gameAppComponent);
         if(gameAppComponent != nullptr)
         {
            // Look up our prototype vehicle and create a new instance of one that we can drive!
            SimCore::Actors::BasePhysicsVehicleActor* vehicle = gameAppComponent->CreateNewVehicle();

            if(vehicle != nullptr)
            {
               // Setup our articulation helper for the vehicle
               std::shared_ptr<DriverArticulationHelper> articHelper = new DriverArticulationHelper;
               articHelper->SetEntity( vehicle );
               vehicle->SetArticulationHelper( articHelper.get() );

               // This method does all the cool stuff!!!
               AttachToVehicle(*vehicle);
            }
            else
            {
               LOG_ERROR("NO vehicle was found after Map was loaded. Check prototypes to ensure vehicle exists.");
            }

         }

      }
      else if(msgType == dtGame::MessageType::INFO_RESTARTED)
      {
         // Nothing to do at the moment.  This message is ALWAYS the very first message sent by the GM.
      }
   }


   ////////////////////////////////////////////////////////////////////////////////
/*   void DriverInputComponent::StopAnyWeaponsFiring()
   {
      // Depress the weapon trigger if the mode is about to change.
      if( mWeapon.valid() )
      {
         mWeapon->SetTriggerHeld( false );
         SimCore::MunitionParticlesActorProxy* proxy = mWeapon->GetShooter();
         if( proxy != nullptr )
         {
            // Clear out the shooter's bullets, otherwise a crash
            // may occur asynchronously.
            SimCore::MunitionParticlesActor* particles;
            proxy->GetActor(particles);
            if( particles != nullptr )
            {
               particles->ResetParticleSystem();
            }
         }
      }
   }
*/
   ////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)
   {
      GameAppComponent* gameAppComponent = dynamic_cast<GameAppComponent*>(GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME));

      //keyboard->GetKeyState(Producer::Key)
      bool handled = true;
      switch(key)
      {
         case 'c':
         {
            if( mHUDComponent.valid() )
            {
               mHUDComponent->CycleCoordinateSystem();
            }
         }
         break;

         case osgGA::GUIEventAdapter::KEY_F1:
         {
            GetHUDComponent()->SetHelpEnabled( ! GetHUDComponent()->IsHelpEnabled() );
         }
         break;

         case osgGA::GUIEventAdapter::KEY_F3:
         {
            if( ! mVehicle->IsRemote() )
            {
               mVehicle->SetHeadLightsEnabled( ! mVehicle->IsHeadLightsEnabled() );
               std::vector<dtUtil::RefString> paramNames;
               paramNames.push_back(SimCore::Actors::PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED);
               mVehicle->GetGameActorProxy().NotifyPartialActorUpdate( paramNames );
            }
         }
         break;

         // Toggles the weapon motion models
         case osgGA::GUIEventAdapter::KEY_Control_L:
         {
            if(gameAppComponent != nullptr)
            {
               if( mWeaponMM.valid() )
               {
                  mWeaponMM->SetEnabled( false );

                  // Reset the weapons pitch so that the players head does not
                  // inherit any more pitch to its orientation than expected.
                  osg::Vec3 hpr = mWeaponMM->GetTargetsRotation();
                  hpr.y() = 0.0f;
                  mWeaponMM->SetTargetsRotation( hpr );
               }
            }
         }
         break;

         // Switch Weapons
         case 'h':
         {
            if(!mWeaponList.empty())
            {
               mWeaponIndex ++;
               mWeaponIndex = (mWeaponIndex >= mWeaponList.size()) ? 0 : mWeaponIndex;
               SetWeapon( mWeaponList[mWeaponIndex].get() );
            }
         }
         break;

         // T spawns target test actors.
         case 't':
         {
            CreateTarget();
         }
         break;

         case '.':
         {
            ToggleView();
         }
         break;

         case 'l':
         {
            if( mWeapon.valid() )
            {
               mWeapon->SetAmmoCount( 1000 );
            }
         }
         break;

         case osgGA::GUIEventAdapter::KEY_Delete:
            {
               // Delete all if shift held, otherwise, just one.
               bool deleteAll = keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Shift_L) ||
                  keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Shift_R);
               KillEnemy(deleteAll);
            }
            break;

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

         case 'u':
         {
            ToggleWeatherStateIfDevMode();
         }
         break;


         // R is the player hack for 'fix me!'. It cures your damage state, removes flames, and also moves
         // you slightly up each frame. This helps when you are stuck in when we get stuck in geometry.
         case 'r':
         {
            //SimCore::Actors::BasePhysicsVehicleActor* vehicle = GetVehicle();
            if(gameAppComponent != nullptr && mVehicle.valid())
            {
               SimCore::Components::MunitionsComponent *munitionsComp =
                  static_cast<SimCore::Components::MunitionsComponent*>
                  (GetGameManager()->GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME));
               munitionsComp->SetDamage( *mVehicle, SimCore::Components::DamageType::DAMAGE_NONE );

               EnableMotionModels( true );

               mVehicle->RepositionVehicle(1.0f/60.0f * 4);
               mVehicle->SetFlamesPresent(false);
            }
         }
         break;

         case osgGA::GUIEventAdapter::KEY_F7:
         {
            ToggleTool(SimCore::MessageType::NIGHT_VISION);
         }
         break;


         /////////////////////////////////
         // EXIT APPLICATION
         /////////////////////////////////

         // Note - Alt X is also trapped by the base class. We mark this as not handled and allow the
         // parent class to handle it as an exit. But we trap it here to make sure we leave the HLA
         // federation before we go.  I'm not sure this is still necessary now that the HLA behavior
         // is separated out. CMM
         case 'x':
            {
               dtABC::Application& app = GetGameManager()->GetApplication();
               if (app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_L) ||
                  app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_R))
               {
#ifdef BUILD_HLA
                  dtHLAGM::HLAComponent* hlaComp = nullptr;
                  // Attempt the disconnect from the network
                  GetGameManager()->GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME, hlaComp);
                  if( hlaComp != nullptr )
                  {
                     hlaComp->LeaveFederationExecution();
                  }
#endif
                  handled = false;
               }
               break;
            }

         default:
            // Implemented to get rid of warnings on g++
            handled = false;
         break;
      }

      if(!handled)
         return BaseClass::HandleKeyPressed(keyboard, key);

      return handled;
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::HandleKeyReleased(const dtCore::Keyboard* keyboard, int key)
   {
      return BaseClass::HandleKeyReleased(keyboard, key);
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button)
   {
      bool handled = false;


      // Left button is fire!  Boom baby!
      if( button == dtCore::Mouse::LeftButton )
      {
         if(mWeapon.valid() && mVehicle.valid()
            && mVehicle->GetDamageState() != SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED )
         {
            mWeapon->SetTriggerHeld( true );
         }
      }
      // Right button should turn turret.
      else if( button == dtCore::Mouse::RightButton )
      {
         if( ! mRingButtonHeld )
         {
            HandleTurretEnabled( true );
            mRingButtonHeld = true;

            if(mVehicle.valid() && IsVehiclePivotable(*mVehicle))
            {
               mRingMM->SetTarget( nullptr );
               mRingMM->SetTargetDOF( mDOFRing.get() );
            }
         }
      }

      if(!handled)
         return BaseClass::HandleButtonPressed(mouse, button);

      return handled;
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::HandleButtonReleased(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button)
   {
      bool handled = false;

      GameAppComponent* gameAppComponent;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);
      if( gameAppComponent != nullptr )
      {
         // stop firing
         if( button == dtCore::Mouse::LeftButton )
         {
            if( mWeapon.valid() )
            {
               mWeapon->SetTriggerHeld( false );
            }
         }
         else if( button == dtCore::Mouse::RightButton )
         {
            if( mRingButtonHeld )
            {
               HandleTurretEnabled( false );
               mRingButtonHeld = false;

               if(mVehicle.valid() && IsVehiclePivotable(*mVehicle))
               {
                  mRingMM->SetTargetsRotation( osg::Vec3() );
                  mRingMM->SetTarget( mVehicle.get() );
                  mRingMM->SetTargetDOF( nullptr );
               }
            }
         }
      }

      if(!handled)
         return BaseClass::HandleButtonReleased(mouse, button);

      return handled;
   }

   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::HandleTurretEnabled( bool enable )
   {
      bool fireEnabled = false;
      if( enable && mVehicle.valid() )
      {
         fireEnabled = mVehicle->GetFlamesPresent();
         enable = ! fireEnabled;
      }

      mWeaponMM->SetEnabled( ! fireEnabled );

      if( enable )
      {
         // Don't let anything else move when moving the turret
         mWeaponMM->SetLeftRightEnabled( false );
         if( ! fireEnabled )
         {
            if(mSoundTurretTurnStart.valid())
            {
               mSoundTurretTurnStart->Play();
            }
            //mRingMM->SetEnabled( true );
         }
      }
      else
      {
         //mRingMM->SetEnabled( false );
         if( ! fireEnabled )
         {
            //mAttachedMM->SetEnabled( true );
            mWeaponMM->SetLeftRightEnabled( true );
            if(mSoundTurretTurnEnd.valid())
            {
               mSoundTurretTurnEnd->Play();
            }
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   DriverHUD* DriverInputComponent::GetHUDComponent()
   {
      if(!mHUDComponent.valid())
      {
         mHUDComponent = dynamic_cast<DriverHUD*>
            (GetGameManager()->GetComponentByName(DriverHUD::DEFAULT_NAME));
      }

      return mHUDComponent.get();
   }

   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::HandleHelpPressed()
   {
      if( GetHUDComponent() != nullptr )
      {
         if (SimCore::Components::HUDState::HELP == GetHUDComponent()->GetHUDState())
            mHUDComponent->CycleToNextHUDState(); // already in help, so toggle it off
         else
            mHUDComponent->SetHUDState(SimCore::Components::HUDState::HELP);
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ToggleTool(SimCore::MessageType &msgType)
   {
      // Depress the weapon trigger if a tool is being changed.
      if( mWeapon.valid() )
      {
         mWeapon->SetTriggerHeld( false );
      }

      std::shared_ptr<dtGame::Message> msg = GetGameManager()->GetMessageFactory().CreateMessage(msgType);
      SimCore::Tools::Tool *tool = GetTool(msgType);
      if(tool == nullptr)
      {
         // DEBUG: LOG_ERROR("Received a tool message from a tool the player does not have");
         return;
      }

      bool enable = !tool->IsEnabled();

      if( enable )
      {
         DisableAllTools();
      }

      static_cast<SimCore::ToolMessage*>(msg.get())->SetEnabled(enable);

      msg->SetAboutActorId(GetStealthActor()->GetUniqueId());
      msg->SetSource(*mMachineInfo);
      GetGameManager()->SendMessage(*msg);

   }

   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::AddTool(SimCore::Tools::Tool &tool, SimCore::MessageType &type)
   {
      if(!SimCore::MessageType::IsValidToolType(type))
      {
         LOG_ERROR("Tried to add a tool with an invalid type");
         return;
      }
      if(mToolList.find(&type) == mToolList.end())
         mToolList.insert(std::make_pair(&type, std::shared_ptr<SimCore::Tools::Tool>(&tool)));
      else
         LOG_ERROR("AddTool tried to add a tool more than once.");
   }

   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::RemoveTool(SimCore::MessageType &type)
   {
      std::map<SimCore::MessageType*, std::shared_ptr<SimCore::Tools::Tool> >::iterator i =  mToolList.find(&type);
      if(i == mToolList.end())
      {
         LOG_ERROR("RemoveTool tried to remove a tool that doesn't exist");
         return;
      }
      mToolList.erase(i);
   }

   /////////////////////////////////////////////////////////////////////////////////
   SimCore::Tools::Tool* DriverInputComponent::GetTool(SimCore::MessageType &type)
   {
      std::map<SimCore::MessageType*, std::shared_ptr<SimCore::Tools::Tool> >::iterator i =
         mToolList.find(&type);

      return i == mToolList.end() ? nullptr : i->second.get();
   }

   /////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::IsToolEnabled(SimCore::MessageType &type) const
   {
      std::map<SimCore::MessageType*, std::shared_ptr<SimCore::Tools::Tool> >::const_iterator i =
         mToolList.find(&type);

      return i == mToolList.end() ? false : i->second->IsEnabled();
   }

   /////////////////////////////////////////////////////////////////////////////////
   SimCore::MessageType& DriverInputComponent::GetEnabledTool() const
   {
      std::map<SimCore::MessageType*, std::shared_ptr<SimCore::Tools::Tool> >::const_iterator i;
      for(i = mToolList.begin(); i != mToolList.end(); ++i)
      {
         if (i->second->IsEnabled())
            return *i->first;
      }
      return SimCore::MessageType::NO_TOOL;
   }

   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::DisableAllTools()
   {
      // Turn off all other tools (this could be NO_TOOL)
      std::map<SimCore::MessageType*, std::shared_ptr<SimCore::Tools::Tool> >::iterator i;
      for(i = mToolList.begin(); i != mToolList.end(); ++i)
      {
         if(i->second->IsEnabled())
         {
            i->second->Enable(false);
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::SetToolEnabled(SimCore::MessageType &toolType, bool enable)
   {
      SimCore::Tools::Tool* tool = GetTool(toolType);

      if( tool != nullptr )
      {
         if( enable != tool->IsEnabled() )
         {
            tool->Enable(enable);
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::InitializePlayer(SimCore::Actors::StealthActor& player)
   {
      // Capture the player
      SetStealthActor(&player);

      // Prevent attached geometry from going invisible
      GetStealthActor()->SetAttachAsThirdPerson(true);

      // Create eye points to which to attach the player for various vantage points. If you wanted to
      // create an offset for standing or sitting inside the vehicle, you'd do it here.
      mWeaponEyePoint = new dtCore::Transformable("WeaponEyePoint");

      // Create the seat
      mSeat = new dtCore::Transformable("PlayerSeat");
      mSeat->AddChild(GetStealthActor());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::AttachToVehicle( SimCore::Actors::BasePhysicsVehicleActor& vehicle )
   {
      // NOTE - The camera sits at the bottom of a VERY large hierarchy of DoF's. Looks like this:
      //     Vehicle (center of vehicle)
      //       - Ring Mount (often swivels left/right)
      //           - mDoFWeapon (pivots about weapon pivot point)
      //               - mWeapon (3D model of weapon)
      //                   - mWeaponEyePoint (offset for human eyepoint)
      //                       - StealthActor (yay!  almost there)
      //                           - camera

      GameAppComponent* gameAppComponent;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);

      // Cleanly dissociate from the current vehicle before associating to another vehicle.
      //DetachFromVehicle();
      GetHUDComponent()->SetWeapon( nullptr );

      // Change to the new vehicle.
      mCurrentActorId = vehicle.GetGameActorProxy().GetId();
      mVehicle = &vehicle;
      mVehicleProxy = dynamic_cast<SimCore::Actors::BasePhysicsVehicleActorProxy*>(&vehicle.GetGameActorProxy()); // Keep the actor from being deleted.
      SimCore::Actors::BasePhysicsVehicleActor* curVehicle = static_cast<SimCore::Actors::BasePhysicsVehicleActor*>(mVehicle.get());

      // Attach the HUD to the new vehicle
      GetHUDComponent()->SetVehicle(&vehicle);
      SimCore::Components::MunitionsComponent* munComp
         = dynamic_cast<SimCore::Components::MunitionsComponent*>
         (GetGameManager()->GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME));
      if( munComp != nullptr )
      {
         GetHUDComponent()->SetDamageHelper( munComp->GetHelperByEntityId(vehicle.GetUniqueId()) );
      }

      // Attach the persistent objects to the new vehicle.
      if( curVehicle != nullptr )
      {
         // Get the weapon & ring mount attachment DOFs
         GetVehicleDOFs( *curVehicle );

         // Store the original orientations of the newly acquired DOFs
         if( mDOFRing.valid() ) { mDOFRingOriginalHPR = mDOFRing->getCurrentHPR(); }
         if( mDOFWeapon.valid() ) { mDOFWeaponOriginalHPR = mDOFWeapon->getCurrentHPR(); }

         // Observe the vehicle's current damage state. Set the observed state
         // here, on initial attach, so that the UpdatedStates function will not
         mLastDamageState = &curVehicle->GetDamageState();

         // this is the GUNNER application
         // Attach the player root transformable to the vehicle's ring's seat DOF
         // rather than directly to the vehicle itself.
         AttachToView( DOF_NAME_VIEW_DEFAULT.Get() );

         // Let this vehicle know it has a driver if the player is in driver mode.
         mVehicle->SetHasDriver( true );
      }


      // Get Stealth Actor ready for attachment by removing it from the scene
      if( GetStealthActor()->GetParent() != nullptr )
      {
         // Set the attach state.
         mPlayerAttached = true;
      }

      // Setup the weapon actor if one needs to be attached.
      if( ! mWeapon.valid() )
      {
         if( mWeaponList.empty() )
         {
            // Assuming no weapons have been loaded. Load all needed weapons.
            InitializeWeapons();
         }

         // Set the default weapon.
         if( ! mWeaponList.empty() )
         {
            SetWeapon( mWeaponList[mWeaponIndex].get() );
         }
      }

      // Adjust the seat and give it a motion model
      // to be moved with the turret ring mount.
      AttachToRingmount( vehicle );

      // The weapon will need to know the vehicle's ID so that it can be sent
      // out in shot fired messages.
      mWeapon->SetOwner( &vehicle.GetGameActorProxy() );

      // Tie the weapon to the HUD to show the ammo meter.
      mHUDComponent->SetWeapon( mWeapon.get() );

      SetViewMode();

      //std::shared_ptr<dtUtil::NodePrintOut> nodePrinter = new dtUtil::NodePrintOut();
      //std::string nodes = nodePrinter->CollectNodeData(*vehicle.GetNonDamagedFileNode());
      //std::cout << " --------- NODE PRINT OUT FOR VEHICLE --------- " << std::endl;
      //std::cout << nodes.c_str() << std::endl;


   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::DetachFromVehicle()
   {
      // CURT - Do we need this method at all?

      // Update HUD
      GetHUDComponent()->SetWeapon( nullptr );
      GetHUDComponent()->SetVehicle( nullptr );
      GetHUDComponent()->SetDamageHelper( nullptr );

      if( mVehicle.valid() )
      {
         // Change the visibilities of both the interior and exterior.
         mVehicle->SetDrawingModel( true );

         // --- If the player was a driver, let the vehicle know its driver has left.
         //     This will prevent the vehicle from moving while using the player
         //     walk motion model input; avoids the "Night Rider car" effect.
         SimCore::Actors::VehicleInterface* vehicle
            = dynamic_cast<SimCore::Actors::VehicleInterface*>(mVehicle.get());
         if( vehicle != nullptr )
         {
            vehicle->SetHasDriver( false );
         }
         // --- The seat could be attached to either the vehicle directly or its ring mount DOF.
         //     This function will not assume the current simulation mode is the old
         //     or the new mode set prior to this function being called; thus remove
         //     from both possible parents.
         mVehicle->RemoveChild( mSeat.get() );

         AttachToView( "" );

         // Reset the original orientations of the vehicle's DOFs.
         if( mDOFRing.valid() )
         {
            mDOFRing->setCurrentHPR( mDOFRingOriginalHPR );
         }
         if( mDOFWeapon.valid() )
         {
            mDOFWeapon->setCurrentHPR( mDOFWeaponOriginalHPR );
         }

         // Offset the player relative to the vehicle
         dtCore::Transform xform;
         mVehicle->GetTransform( xform );
         xform.Move(osg::Vec3(3.0, 3.0, 3.0));
         mSeat->SetTransform( xform );

      }

      // Update the attach state.
      mPlayerAttached = false;

   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::SetViewMode()
   {
      if( mWeaponMM.valid())
      {
         bool flamesPresent = mVehicle.valid() && mVehicle->GetFlamesPresent();
         mWeaponMM->SetEnabled( !flamesPresent);
      }
   }


   //////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::SetPlayer( SimCore::Actors::StealthActor* actor )
   {
      SetStealthActor(actor);
   }

   //////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ResetTurnSpeeds()
   {
   }

   //////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::InitializeSounds( SimCore::Actors::StealthActor& player )
   {
      // NOTE: Turret sounds will have to be re-attached to the vehicle's turret DOF.

      // Turret Turn Start
      //mSoundTurretTurnStart = dtAudio::AudioManager::GetInstance().NewSound();
      //mSoundTurretTurnStart->LoadFile(SOUND_TURRET_TURN_START.c_str());
      //mSoundTurretTurnStart->ListenerRelative(true);
      //mSoundTurretTurnStart->SetLooping(false);
      //mSoundTurretTurnStart->SetMaxDistance(1.0f);
      //player.AddChild(mSoundTurretTurnStart.get());

      // Turret Turn
      //mSoundTurretTurn = dtAudio::AudioManager::GetInstance().NewSound();
      //mSoundTurretTurn->LoadFile(SOUND_TURRET_TURN.c_str());
      //mSoundTurretTurn->ListenerRelative(true);
      //mSoundTurretTurn->SetLooping(true);
      //mSoundTurretTurn->SetMaxDistance(1.0f);
      //player.AddChild(mSoundTurretTurn.get());

      // Turret Turn End
      //mSoundTurretTurnEnd = dtAudio::AudioManager::GetInstance().NewSound();
      //mSoundTurretTurnEnd->LoadFile(SOUND_TURRET_TURN_END.c_str());
      //mSoundTurretTurnEnd->ListenerRelative(true);
      //mSoundTurretTurnEnd->SetLooping(false);
      //mSoundTurretTurnEnd->SetMaxDistance(1.0f);
      //player.AddChild(mSoundTurretTurnEnd.get());

      // Ambient (environmental sounds)
      //mSoundAmbient = dtAudio::AudioManager::GetInstance().NewSound();;
      //mSoundAmbient->LoadFile(SOUND_AMBIENT.c_str());
      //mSoundAmbient->ListenerRelative(false);
      //mSoundAmbient->SetLooping(true);
      //mSoundAmbient->SetMaxDistance(1.0f);
      //player.AddChild(mSoundAmbient.get());
   }

   //////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::InitializeWeapons()
   {
      std::shared_ptr<SimCore::Actors::WeaponActor> curWeapon;
      GameAppComponent* gameAppComponent;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);

      // weapons -- for array iterator.
      // WEAPON 1
      CreateWeapon( "Weapon_Grenade",
         "Particle_System_Weapon_Grenade",
         "weapon_gun_flash.osg", curWeapon );
      if( curWeapon.valid() ) { mWeaponList.push_back( curWeapon.get() ); }
      curWeapon = nullptr;

      // WEAPON 2
      CreateWeapon( "Weapon_MachineGun",
         "Particle_System_Weapon_GunWithTracer",
         "weapon_gun_flash.osg", curWeapon );
      if( curWeapon.valid() ) { mWeaponList.push_back( curWeapon.get() ); }
      curWeapon = nullptr;

      mWeaponIndex = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::CreateWeapon( const std::string& weaponName,
         const std::string& shooterName, const std::string& flashEffectFile,
            std::shared_ptr<SimCore::Actors::WeaponActor>& outWeapon )
   {
      dtGame::GameManager* gm = GetGameManager();

      std::vector<dtDAL::ActorProxy*> proxyArray;
      gm->FindPrototypesByName( weaponName, proxyArray );

      if( proxyArray.empty() )
      {
         LOG_ERROR( "No weapon actor prototype could be loaded from the map." );
         return false;
      }

      // Create a new weapon proxy from the discovered prototype
      std::shared_ptr<SimCore::Actors::WeaponActorProxy> weaponProxy
         = dynamic_cast<SimCore::Actors::WeaponActorProxy*>
         (gm->CreateActorFromPrototype(proxyArray[0]->GetId()).get());
      outWeapon = weaponProxy.valid() ?
         static_cast<SimCore::Actors::WeaponActor*> (weaponProxy->GetDrawable()) : nullptr;

      if( ! outWeapon.valid() )
      {
         LOG_ERROR( "Failure creating weapon actor." );
         return false;
      }

      // Call OnEnterWorld
      gm->AddActor( *weaponProxy, false, false );
      // Pull the weapon out of the scene so its parent is nullptr.
      // This will help in attaching the weapon to a pivot.
      outWeapon->Emancipate();

      // Get ready to instantiate a shooter from a prototype.
      proxyArray.clear();
      gm->FindPrototypesByName( shooterName, proxyArray);

      if( ! proxyArray.empty() )
      {
         std::shared_ptr<dtDAL::ActorProxy> ourActualActorProxy
            = gm->CreateActorFromPrototype(proxyArray.front()->GetId());

         if(ourActualActorProxy != nullptr)
         {
            // Give the shooter unit access to the shooter so that it
            // can tell the shooter when to fire.
            SimCore::Actors::MunitionParticlesActorProxy* proxy =
               dynamic_cast<SimCore::Actors::MunitionParticlesActorProxy*>(ourActualActorProxy.get());
            outWeapon->SetShooter( proxy );

            // Initialize the physics based particle system/shooter,
            // only if the shooter unit was assigned a valid proxy.
            if( proxy != nullptr )
            {
               SimCore::Actors::MunitionParticlesActor* shooter
                  = dynamic_cast<SimCore::Actors::MunitionParticlesActor*>(ourActualActorProxy->GetDrawable());

               // Set other properties of the particle system
               shooter->SetWeapon( *outWeapon );

               // HACK: temporary play values
               shooter->SetFrequencyOfTracers( outWeapon->GetTracerFrequency() );

               // Place the shooter into the world
               gm->AddActor( shooter->GetGameActorProxy(), false, false );
               shooter->Emancipate();

               // Attach the shooter to the weapon's flash point
               outWeapon->AddChild(shooter, "hotspot_01");
            }
         }
      }

      // Create the flash effect for the weapon
      SimCore::Actors::WeaponFlashActor* flash = nullptr;
      std::shared_ptr<SimCore::Actors::WeaponFlashActorProxy> flashProxy;
      gm->CreateActor( *SimCore::Actors::EntityActorRegistry::WEAPON_FLASH_ACTOR_TYPE, flashProxy );
      flash = static_cast<SimCore::Actors::WeaponFlashActor*>(flashProxy->GetDrawable());
      std::stringstream flashFilePath;
      flashFilePath << "Particles/" << flashEffectFile;
      flash->SetParticleEffect( flashFilePath.str() );
      outWeapon->SetFlashActor( flash );
      outWeapon->AttachObject( *flash, "hotspot_01" );
      gm->AddActor( flash->GetGameActorProxy(), false, false );

      return true;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::SetWeapon( SimCore::Actors::WeaponActor* weapon )
   {
      // Remove all references to the current weapon
      mHUDComponent->SetWeapon( nullptr );

      // Clean up old weapon if there is already one set.
      if( mWeapon.valid() )
      {
         // Turn off the weapon if its trigger is currently held.
         mWeapon->SetTriggerHeld( false );

         // Remove the current weapon from both possible attach points.
         if( mDOFFirePoint.valid() )
         {
            mDOFFirePoint->removeChild( mWeapon->GetOSGNode() );
         }

         if( mWeaponEyePoint.valid() )
         {
            mWeapon->RemoveChild( mWeaponEyePoint.get() );
         }
      }

      // Make the change
      mWeapon = weapon;

      // Set reference to the new weapon
      mHUDComponent->SetWeapon( mWeapon.get() );

      if( mWeapon.valid() )
      {
         if( mDOFFirePoint.valid() )
         {
            mDOFFirePoint->addChild( mWeapon->GetOSGNode() );
         }

         dtCore::Transform xform;

         if( mWeaponEyePoint.valid() )
         {
            // Offset the eye point
            mWeapon->AddChild( mWeaponEyePoint.get() );
            xform.MakeIdentity();
            mWeaponEyePoint->SetTransform( xform, dtCore::Transformable::REL_CS );
         }

         // Set the owner
         if (mVehicle.valid())
            mWeapon->SetOwner(&mVehicle->GetGameActorProxy());
      }

   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::IsVehiclePivotable( const SimCore::Actors::BasePhysicsVehicleActor& vehicle ) const
   {
      const HoverVehicleActor* hoverActor = dynamic_cast<const HoverVehicleActor*>(&vehicle);
      return hoverActor != nullptr && hoverActor->GetVehicleIsTurret();
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::AttachToRingmount( SimCore::Actors::BasePhysicsVehicleActor& vehicle )
   {
      dtABC::Application& app = GetGameManager()->GetApplication();

      // Set the weapon motion model on the weapon pivot
      if( ! mWeaponMM.valid() )
      {
         mWeaponMM = new SimCore::ClampedMotionModel( app.GetKeyboard(), app.GetMouse() );
         mWeaponMM->SetLeftRightLimit( 0.0f );
         mWeaponMM->SetMaximumMouseTurnSpeed(70.0f);
         mWeaponMM->SetUpDownLimit( 45.0f, 15.0f );
         mWeaponMM->SetName("WeaponMM");
      }
      mWeaponMM->SetEnabled( true );
      mWeaponMM->SetTargetDOF( mDOFWeapon.get() );

      // create the attached motion model for the ring mount DOF
      if( ! mRingMM.valid() )
      {
         mRingMM = new SimCore::ClampedMotionModel( app.GetKeyboard(), app.GetMouse() );
         mRingMM->SetMaximumMouseTurnSpeed(40.0f);
         mRingMM->SetUpDownLimit( 0.0f );
         mRingMM->SetName("RingMM");
      }

      // Look up a weird quirk of the data (only set on the hover vehicle actor).
      // Is the turret 'hard-wired' to the vehicle? If so, the ring mount motion
      // model turns the vehicle, not just the ring mount DoF. This is true for
      // instance, with the Hover Vehicle.
      if(IsVehiclePivotable(vehicle))
         mRingMM->SetTarget(&vehicle);
      else
         mRingMM->SetTargetDOF( mDOFRing.get() );

      // Tell the MotionModels about the articulation helper - This allows the artic helper to
      // know about data changes which in turn, allows it to check for changes before publishing an update.
      if( mRingMM.valid() )
         mRingMM->SetArticulationHelper(vehicle.GetArticulationHelper());
      if( mWeaponMM.valid() )
         mWeaponMM->SetArticulationHelper(vehicle.GetArticulationHelper());


      mRingMM->SetEnabled(true);

      return true;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::UpdateStates( float simDelta, float realDelta )
   {
      GameAppComponent* gameAppComponent;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);

      // HACK: Force the time actor values to be applied to the ephemeris rendering.
      // The ephemeris updating is suspected to be asynchronous--the probable cause
      // for the time-of-day issue (sky does not match the time).
      if( mEnvUpdateAttempts > 0 )
      {
         if( mEnvUpdateTime >= 2.0f ) // time for update
         {
            SimCore::Components::WeatherComponent* comp
               = dynamic_cast<SimCore::Components::WeatherComponent*>(GetGameManager()
               ->GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME));
            if( comp != nullptr )
            {
               if( (mEnvUpdateAttempts%2) == 1 ) // time back
               {
                  IncrementTime(-5);
               }
               else // time forward
               {
                  IncrementTime(5);
               }
            }

            mEnvUpdateTime = 0.0f; // reset clock for another attempt
            --mEnvUpdateAttempts; // one attempt was burned
         }
         else // advance time toward next update
         {
            mEnvUpdateTime += realDelta;
         }
      }


      // Affect DOFs of the vehicle exterior if this is the gunner simulation mode.
      if( mVehicle.valid() && *mLastDamageState != mVehicle->GetDamageState() )
      {
         // Dissociate from the current DOFs and then
         // associate to the new DOFs
         GetVehicleDOFs( *mVehicle.get() );

         // Disable/enable motion models based on the new damage state.
         mLastDamageState = &mVehicle->GetDamageState();
         if( mVehicle->IsMobilityDisabled() )
         {
            // Mobility already disabled via the damage helper
         }
         if( mVehicle->IsFirepowerDisabled() )
         {
            mRingMM->SetEnabled( false );
            mWeaponMM->SetEnabled( false );
            if( mWeapon.valid() )
            {
               mWeapon->SetTriggerHeld( false );
            }
         }
      }

      // Update sounds that rely on updates in motion models, such as the
      // ring mount sliding sound.
      UpdateSounds();

   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::UpdateSounds()
   {
      bool vehicleTurretDisabled = false;
      if( mVehicle.valid() )
      {
         vehicleTurretDisabled = mVehicle->IsFirepowerDisabled();
      }

      bool playSound = false;
      if( mRingMM.valid() && ! vehicleTurretDisabled )
      {
         playSound = mRingMM->GetHPRChange().x() != 0.0;// && (mRingKeyHeld || mRingButtonHeld);
         if( playSound )
         {
            if( mSoundTurretTurn.valid() && ! mSoundTurretTurn->IsPlaying() )
            {
               mSoundTurretTurn->Play();
            }
         }
      }

      if( ! playSound )
      {
         if( mSoundTurretTurn.valid() && mSoundTurretTurn->IsPlaying() )
         {
            mSoundTurretTurn->Stop();
         }

         if( vehicleTurretDisabled )
         {
            if( mSoundTurretTurnStart.valid() && mSoundTurretTurnStart->IsPlaying() )
            {
               mSoundTurretTurnStart->Stop();
            }
            if( mSoundTurretTurnEnd.valid() && mSoundTurretTurnEnd->IsPlaying() )
            {
               mSoundTurretTurnEnd->Stop();
            }
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::GetVehicleDOFs( SimCore::Actors::BasePhysicsVehicleActor& vehicle )
   {
      mDOFSeat = vehicle.GetNodeCollector()->GetDOFTransform(mViewNodeName);
      mDOFRing = vehicle.GetNodeCollector()->GetDOFTransform(DOF_NAME_RINGMOUNT.Get());
      mDOFWeapon = vehicle.GetNodeCollector()->GetDOFTransform(DOF_NAME_WEAPON_PIVOT.Get());
      mDOFFirePoint = vehicle.GetNodeCollector()->GetDOFTransform(DOF_NAME_WEAPON_FIRE_POINT.Get());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::EnableMotionModels( bool enable )
   {
      GameAppComponent* gameAppComponent;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);
      mMotionModelsEnabled = enable;

      // Force all motion models off.
      if( mAttachedMM.valid() )
      {
         mAttachedMM->SetEnabled(false);
      }
      if( mRingMM.valid() ) { mRingMM->SetEnabled(false); }
      if( mWeaponMM.valid() ) { mWeaponMM->SetEnabled(false); }

      // Turn on motion models specific to the mode.
      if( enable )
      {
         if( mVehicle.valid() && ! mVehicle->GetFlamesPresent() )
         {
            if( mRingMM.valid() )
            {
               mRingMM->SetEnabled( true );
            }

            if( mWeaponMM.valid() )
            {
               mWeaponMM->SetEnabled( true );
            }
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::CreateTarget()
   {
      dtGame::GameManager* gm = GetGameManager();
      std::vector<dtDAL::ActorProxy*> foundProxies;

      float randChance = dtUtil::RandFloat(0.0, 1.0);
      std::string prototypeName("UNKNOWN");

      // 90% chance for a regular target, 10% for an exploding one
      if (randChance < 0.90)
      {
         prototypeName = "Hover_Target_Prototype";
      }
      else
      {
         prototypeName = "Hover_Exploding_Target_Prototype";
      }


      gm->FindPrototypesByName(prototypeName, foundProxies);
      if( foundProxies.empty() )
      {
         LOG_ERROR( "No Hover Target prototype could be loaded from the map." );
         return;
      }

      // Create a new target from the discovered prototype
      std::shared_ptr<dtGame::GameActorProxy> newTargetProxy
         = dynamic_cast<dtGame::GameActorProxy*>
         (gm->CreateActorFromPrototype(foundProxies[0]->GetId()).get());

      // Add the actor to the game manager.
      if (newTargetProxy.valid())
      {
         GetGameManager()->AddActor(*newTargetProxy.get(), false, true);
      }
      else
         LOG_ERROR( "Failure creating Hover Target actor." );
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ToggleView()
   {
      AttachToView( mViewNodeName == DOF_NAME_VIEW_01.Get()
         ? DOF_NAME_VIEW_02.Get() : DOF_NAME_VIEW_01.Get() );
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::AttachToView( const std::string& viewNodeName )
   {
      if( mVehicle.valid() )
      {
         if( mDOFSeat.valid() )
         {
            mDOFSeat->removeChild( mSeat->GetOSGNode() );
         }

         mDOFSeat = mVehicle->GetNodeCollector()->GetDOFTransform(viewNodeName/*mViewNodeName*/);

         if( mDOFSeat.valid() )
         {
            mViewNodeName = viewNodeName;
            mDOFSeat->addChild( mSeat->GetOSGNode() );
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ToggleDRGhost()
   {
      SimCore::Actors::BasePhysicsVehicleActor* mPhysVehicle =
         dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor*>(mVehicle.get());

      // basic error check.
      if (!mVehicle.valid() || mPhysVehicle == nullptr)
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
            SimCore::Actors::DRGhostActor* actor = nullptr;
            mDRGhostActorProxy->GetActor(actor);
            actor->SetSlavedEntity(mVehicle);
            GetGameManager()->AddActor(*mDRGhostActorProxy, false, false);
         }

         mDRGhostMode = GHOST_ON;
      }

      // state - GHOST_ON, go to NONE
      else if (mDRGhostMode == GHOST_ON)
      {
         LOG_ALWAYS(" --- Removing DR Ghost.");
         CleanUpDRGhost();
      }

   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::CleanUpDRGhost()
   {
      if (mDRGhostActorProxy.valid())
      {
         GetGameManager()->DeleteActor(*mDRGhostActorProxy.get());
         mDRGhostActorProxy = nullptr;
         mDRGhostMode = NONE;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::KillEnemy(bool killAllEnemies)
   {
      // We use the Munitions Component to do the damage to the object
      SimCore::Components::MunitionsComponent* munitionsComp = nullptr;
      GetGameManager()->GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME, munitionsComp);
      if (munitionsComp == nullptr)
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
         SimCore::Actors::BaseEntity* entity = dynamic_cast<SimCore::Actors::BaseEntity*>(allGameActors[i]->GetDrawable());
         if (entity != nullptr && entity->GetDamageState() != SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED && 
            (allGameActors[i]->GetActorType() == *DriverActorRegistry::HOVER_TARGET_ACTOR_TYPE ||
            allGameActors[i]->GetActorType() == *DriverActorRegistry::HOVER_EXPLODING_TARGET_ACTOR_TYPE))
         {
            munitionsComp->SetDamage(*entity, SimCore::Components::DamageType::DAMAGE_KILL);
            //SimCore::Components::DamageStateEnum::DESTROYED);

            // Stop after one or keep going for all
            if (!killAllEnemies)
               break;
         }
      }

   }

}
