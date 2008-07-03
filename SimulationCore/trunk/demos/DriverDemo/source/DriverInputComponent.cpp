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
#include <dtCore/shadermanager.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shaderparamfloat.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/nodecollector.h>

#include <dtUtil/enumeration.h>
#include <dtUtil/nodeprintout.h>

#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <SimCore/Actors/TextureProjectorActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/MissileActor.h> // for missile load testing
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/VehicleInterface.h>
#include <SimCore/PlayerMotionModel.h>
#include <SimCore/Actors/VehicleAttachingConfigActor.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

#include <SimCore/StealthMotionModel.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/CommandLineObject.h>
#include <SimCore/ClampedMotionModel.h>
#include <SimCore/PlayerMotionModel.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/PlayerMotionModel.h>
#include <SimCore/WeaponTypeEnum.h>
#include <SimCore/Tools/Tool.h>

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

#include <dtHLAGM/hlacomponent.h>


#ifdef AGEIA_PHYSICS
#include <NxAgeiaWorldComponent.h>
//#include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/Actors/NECCBoatActor.h>
#include <SimCore/Actors/HumanWithPhysicsActor.h>
#include <dtDAL/project.h>
#endif

// TEST INCLUDES --- START
#include <dtDAL/groupactorproperty.h>
// TEST INCLUDES --- END

#include <DriverArticulationHelper.h>
#include <HoverVehicleActor.h>
#include <HoverTargetActor.h>

#include <SimCore/Actors/FlareActor.h>
#include <dtUtil/matrixutil.h>

#include <SimCore/BaseGameEntryPoint.h>

using dtCore::RefPtr;

namespace DriverDemo
{

   // CurtTest
//   static DVTE::Actors::FloatTargetActor* mStaticLastTarget = NULL;

   ////////////////////////////////////////////////////////////////////////////////
   // NOTE: The files referenced here WILL change; and may even be contained in
   //       another actor that will allow these to be set by an xml file.
   //       For now, the referenced files are place holders for the real
   //       sound effects to be used.
   const std::string DriverInputComponent::SOUND_TURRET_TURN_START("Sounds/RAW_SFX_Turrent_Slide.wav");
   const std::string DriverInputComponent::SOUND_TURRET_TURN("Sounds/RAW_SFX_Turrent_Slide.wav");
   const std::string DriverInputComponent::SOUND_TURRET_TURN_END("Sounds/RAW_SFX_Turrent_Slide.wav");
   //const std::string DriverInputComponent::SOUND_AMBIENT("Sounds/Ambient_Crickets.wav");
   
   ////////////////////////////////////////////////////////////////////////////////
   DriverInputComponent::DriverInputComponent(const std::string& name) :
      BaseClass(name),
      DOF_NAME_WEAPON_PIVOT("dof_gun_01"),//"dof_gun_attach"),
      DOF_NAME_WEAPON_STEM("dof_gun_01"),
      DOF_NAME_RINGMOUNT("dof_turret_01"),
      DOF_NAME_RINGMOUNT_SEAT("dof_seat_gunner"),
      mIsConnected(false),
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
      mEnvUpdateTime(0.0f),
      mEnvUpdateAttempts(2)
   {
      mLogger = &dtUtil::Log::GetInstance("DriverInputComponent.cpp");
      mMachineInfo = new dtGame::MachineInfo;
   
      SetStartPosition(osg::Vec3(100.0,100.0,20.0));
   }
   
   ////////////////////////////////////////////////////////////////////////////////
   DriverInputComponent::~DriverInputComponent()
   {
      mHLA = NULL;
      if( mSoundAmbient.valid() )
      {
         mSoundAmbient.release();
         mSoundAmbient = NULL;
      }
      if( mSoundTurretTurn.valid() )
      {
         mSoundTurretTurn.release();
         mSoundTurretTurn = NULL;
      }
      if( mSoundTurretTurnStart.valid() )
      {
         mSoundTurretTurnStart.release();
         mSoundTurretTurnStart = NULL;
      }
      if( mSoundTurretTurnEnd.valid() )
      {
         mSoundTurretTurnEnd.release();
         mSoundTurretTurnEnd = NULL;
      }
   }
   
   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& msgType = message.GetMessageType();
      if (msgType == dtGame::MessageType::TICK_LOCAL)
      {
         const dtGame::TickMessage& tick = static_cast<const dtGame::TickMessage&>(message);
         UpdateTools( tick.GetDeltaSimTime() );
         UpdateStates( tick.GetDeltaSimTime(), tick.GetDeltaRealTime() );
   
         // Update the view height distance - helps the fog decay nicely based on height.
         // Note, in the vehicle, it shouldn't go 'up' much, but it helps the comp to keep things in sync
         SimCore::Components::WeatherComponent* weatherComp;
         GetGameManager()->GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME, weatherComp);
         if(weatherComp != NULL && mStealthActor.valid())
         {
            dtCore::Transform xform;
            mStealthActor->GetTransform( xform );
            osg::Vec3 pos;
            xform.GetTranslation(pos);
            weatherComp->SetViewElevation(pos[2]);
         }

         //UpdateInteriorModel();
      }

      // Tool update message.
      else if(dynamic_cast<const SimCore::ToolMessage*>(&message) != NULL)
      {
         const SimCore::ToolMessage& toolMsg = static_cast<const SimCore::ToolMessage&>(message);
         SetToolEnabled(
            static_cast<SimCore::MessageType&>(const_cast<dtGame::MessageType&>(toolMsg.GetMessageType())),
            toolMsg.IsEnabled());
      }

      // A Local Player entered world, so create our motion models
      else if (msgType == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD && 
         message.GetSource() == GetGameManager()->GetMachineInfo())
      {
         dtGame::GameActorProxy *stealthProxy = GetGameManager()->FindGameActorById(message.GetAboutActorId());
         if(stealthProxy == NULL)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "A player entered world message was received, but the about actor id does not refer to a Game Actor in the Game Manager.");
            return;
         }
   
         SimCore::Actors::StealthActor *stealthActor
            = dynamic_cast<SimCore::Actors::StealthActor*>(&stealthProxy->GetGameActor());
         if(stealthActor == NULL)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
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
         if (mStealthActor.valid() && mStealthActor->GetUniqueId() == id)
         {
            mStealthActor = NULL;
         }
         else if (mVehicle.valid() && mVehicle->GetUniqueId() == id)
         {
            DriverDemo::GameAppComponent* gameAppComponent;
            GetGameManager()->GetComponentByName(DriverDemo::GameAppComponent::DEFAULT_NAME, gameAppComponent);
            if(gameAppComponent != NULL)
            {
               // WE'RE TOTALLY HOSED IF SOMEONE DELETES OUR VEHICLE AT THIS POINT!
               mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__, 
                  "Someone deleted our vehicle! OMG! How rude! We're in big trouble!!!");
               mVehicle = NULL;
            }
         }
      }

      // Map first loaded. This only happens once, so we can do most of our inits now.
      else if(msgType == dtGame::MessageType::INFO_MAP_LOADED)
      {
         dtGame::GameManager &gameManager = *GetGameManager();
          
         DriverDemo::GameAppComponent* gameAppComponent;
         gameManager.GetComponentByName(DriverDemo::GameAppComponent::DEFAULT_NAME, gameAppComponent);
         if(gameAppComponent != NULL)
         {
            // Look up our prototype vehicle and create a new instance of one that we can drive! 
            SimCore::Actors::BasePhysicsVehicleActor* vehicle = gameAppComponent->CreateNewVehicle();
           
            if(vehicle != NULL)
            {
               // Setup our articulation helper for the vehicle
               dtCore::RefPtr<DriverArticulationHelper> articHelper = new DriverArticulationHelper;
               articHelper->SetEntity( vehicle );
               vehicle->SetArticulationHelper( articHelper.get() );

               // Tell the MotionModels about the articulation helper.
               if( mRingMM.valid() )
                  mRingMM->SetArticulationHelper( articHelper.get() );
               if( mWeaponMM.valid() )
                  mWeaponMM->SetArticulationHelper( articHelper.get() );

               // Register a munitions component to the vehicle
               SimCore::Components::MunitionsComponent* munitionsComp;
               gameManager.GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME, munitionsComp);
               if( munitionsComp != NULL )
               {
                  munitionsComp->Register( *vehicle );
               }

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
   void DriverInputComponent::StopAnyWeaponsFiring()
   {
      // Depress the weapon trigger if the mode is about to change.
      if( mWeapon.valid() )
      {
         mWeapon->SetTriggerHeld( false );
         NxAgeiaMunitionsPSysActorProxy* proxy = mWeapon->GetShooter();
         if( proxy != NULL )
         {
            // Clear out the shooter's bullets, otherwise a crash
            // may occur asynchronously.
            NxAgeiaMunitionsPSysActor* particles;
            proxy->GetActor(particles);
            if( particles != NULL )
            {
               particles->ResetParticleSystem();
            }
         }
      }
   }
   
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
               std::vector<std::string> paramNames;
               paramNames.push_back(SimCore::Actors::PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED);
               mVehicle->GetGameActorProxy().NotifyPartialActorUpdate( paramNames );
            }
         }
         break;
   
         // Toggles the weapon motion models 
         case osgGA::GUIEventAdapter::KEY_Control_L:
         {
            if(gameAppComponent != NULL)
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
               if( mRingMM.valid() )
               {
                  mRingMM->SetEnabled( false );
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

         case 'l':
         {
            if( mWeapon.valid() ) 
            { 
               mWeapon->SetAmmoCount( 1000 ); 
            }
         }
         break;

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
   
         case 'p':
         {
            std::string developerMode;
            developerMode = GetGameManager()->GetConfiguration().GetConfigPropertyValue
               (SimCore::BaseGameEntryPoint::CONFIG_PROP_DEVELOPERMODE, "false");
            if (developerMode == "true" || developerMode == "1")
               ToggleEntityShaders();
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

         // R is the player hack for 'fix me!'. It cures your damage state, removes flames, and also moves
         // you slightly up each frame. This helps when you are stuck in when we get stuck in geometry. 
         case 'r':
         {
            //SimCore::Actors::BasePhysicsVehicleActor* vehicle = GetVehicle();
            if(gameAppComponent != NULL && mVehicle.valid())
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
   #ifndef __APPLE__               
               if (app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_L) ||
                  app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_R))
   #endif
                  
               {
                  // Attempt the disconnect from the network
                  mHLA = static_cast<dtHLAGM::HLAComponent*>(GetGameManager()->GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME));
                  if( mHLA.valid() )
                  {
                     mHLA->LeaveFederationExecution();
                     mIsConnected = false;
                  }
                  handled = false;
               }
               break;
            }
   
         default:
            // Implemented to get rid of warnings in Linux
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
      bool handled = false;
   
      GameAppComponent* gameAppComponent = NULL;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);
         
      if(!handled)
         return BaseClass::HandleKeyReleased(keyboard, key);
   
      return handled;
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button)
   {
      bool handled = false;
   
      GameAppComponent* gameAppComponent;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);
      if(gameAppComponent != NULL)
      {

         // Left button is fire!  Boom baby!
         if( button == dtCore::Mouse::LeftButton )
         {
            if(mWeapon.valid() && mVehicle.valid() 
               && mVehicle->GetDamageState() != SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED )
            { 
               mWeapon->SetTriggerHeld( true );
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
      if( gameAppComponent != NULL)
      {
         // stop firing
         if( button == dtCore::Mouse::LeftButton )
         {
            if( mWeapon.valid() ) 
            { 
               mWeapon->SetTriggerHeld( false ); 
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
         fireEnabled = mVehicle->IsFlamesPresent();
         enable = ! fireEnabled;
      }

      mWeaponMM->SetEnabled( ! fireEnabled );
   
      if( enable )
      {
         // Don't let anything else move when moving the turret
         mWeaponMM->SetLeftRightEnabled( false );
         if( ! fireEnabled )
         {
            if (mSoundTurretTurnStart.valid())
               mSoundTurretTurnStart->Play();
            mRingMM->SetEnabled( true );
         }
      }
      else
      {
         mRingMM->SetEnabled( false );
         if( ! fireEnabled )
         {
            //mAttachedMM->SetEnabled( true );
            mWeaponMM->SetLeftRightEnabled( true );
            if (mSoundTurretTurnEnd.valid())
               mSoundTurretTurnEnd->Play();
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
      if( GetHUDComponent() != NULL )
      {
         if (SimCore::Components::HUDState::HELP == GetHUDComponent()->GetHUDState())
            mHUDComponent->CycleToNextHUDState(); // already in help, so toggle it off
         else
            mHUDComponent->SetHUDState(SimCore::Components::HUDState::HELP);
      }
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ToggleEntityShaders()
   {
      dtCore::ShaderManager::GetInstance().ReloadAndReassignShaderDefinitions("Shaders/ShaderDefs.xml");
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::ToggleTool(SimCore::MessageType &msgType)
   {
      // Depress the weapon trigger if a tool is being changed.
      if( mWeapon.valid() )
      {
         mWeapon->SetTriggerHeld( false );
      }
   
      dtCore::RefPtr<dtGame::Message> msg = GetGameManager()->GetMessageFactory().CreateMessage(msgType);
      SimCore::Tools::Tool *tool = GetTool(msgType);
      if(tool == NULL)
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
   
      msg->SetAboutActorId(mStealthActor->GetUniqueId());
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
         mToolList.insert(std::make_pair(&type, dtCore::RefPtr<SimCore::Tools::Tool>(&tool)));
      else
         LOG_ERROR("AddTool tried to add a tool more than once.");
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::RemoveTool(SimCore::MessageType &type)
   {
      std::map<SimCore::MessageType*, dtCore::RefPtr<SimCore::Tools::Tool> >::iterator i =  mToolList.find(&type);
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
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i = 
         mToolList.find(&type);
   
      return i == mToolList.end() ? NULL : i->second.get();
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::IsToolEnabled(SimCore::MessageType &type) const
   {
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::const_iterator i = 
         mToolList.find(&type);
   
      return i == mToolList.end() ? false : i->second->IsEnabled(); 
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::UpdateTools( float timeDelta )
   {
      //SimCore::Tools::Compass* compass = static_cast<SimCore::Tools::Compass*>(GetTool(SimCore::MessageType::COMPASS));
      //if( compass != NULL && compass->IsEnabled() )
      //{
   	//   compass->Update( timeDelta );
      //}
   
  }
   
   /////////////////////////////////////////////////////////////////////////////////
   SimCore::MessageType& DriverInputComponent::GetEnabledTool() const
   {
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::const_iterator i;
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
      std::map<SimCore::MessageType*, RefPtr<SimCore::Tools::Tool> >::iterator i;
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

      if( tool != NULL )
      {
         if( enable != tool->IsEnabled() )
         {
            tool->Enable(enable);
         }
      }
   }
   
   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::InitializePlayer( SimCore::Actors::StealthActor& player )
   {
      dtGame::GameManager* gm = GetGameManager();
      dtABC::Application& app = gm->GetApplication();
   

      // Capture the player
      mStealthActor = &player;
   
      // Prevent attached geometry from going invisible
      mStealthActor->SetAttachAsThirdPerson(true); 
   
      // Create eye points to which to attach the player for various vantage points. If you wanted to 
      // create an offset for standing or sitting inside the vehicle, you'd do it here.
      mWeaponEyePoint = new dtCore::Transformable("WeaponEyePoint");
   
      // Create the seat
      mSeat = new dtCore::Transformable("PlayerSeat");
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
      GetHUDComponent()->SetWeapon( NULL );
   
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
      if( munComp != NULL )
      {
         GetHUDComponent()->SetDamageHelper( munComp->GetHelperByEntityId(vehicle.GetUniqueId()) );
      }
   
      // Attach the persistent objects to the new vehicle.
      if( curVehicle != NULL )
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
         if( mDOFSeat.valid() )
         {
            mDOFSeat->addChild( mSeat->GetOSGNode() );
         }
   
         // Let this vehicle know it has a driver if the player is in driver mode.
         mVehicle->SetHasDriver( true );
      }
   
   
      // Get Stealth Actor ready for attachment by removing it from the scene
      if( mStealthActor->GetParent() != NULL )
      {
         // Set the attach state.
         mPlayerAttached = true;
   
         // Remove from the scene
         mStealthActor->Emancipate();
      }
   
      // Offset the player vantage point.
      osg::Vec3 rotationOffset;

      mStealthActor->SetAttachOffset( osg::Vec3(0.0, 0.0, 0.0) ); // GUNNER

      // Position the seat DOF
      dtCore::Transform xform;
      xform.SetTranslation( mStealthActor->GetAttachOffset() );
      xform.SetRotation(rotationOffset);
      mSeat->SetTransform( xform, dtCore::Transformable::REL_CS );
   
      // Attach the player to the sitting view point by default
      //SetViewMode( VIEW_MODE_SIT );
   
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

      dtCore::RefPtr<dtUtil::NodePrintOut> nodePrinter = new dtUtil::NodePrintOut();
      std::string nodes = nodePrinter->CollectNodeData(*vehicle.GetNonDamagedFileNode());
      std::cout << " --------- NODE PRINT OUT FOR VEHICLE --------- " << std::endl;
      std::cout << nodes.c_str() << std::endl;


   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::DetachFromVehicle()
   {
      // CURT - Do we need this method at all?

      // Update HUD
      GetHUDComponent()->SetWeapon( NULL );
      GetHUDComponent()->SetVehicle( NULL );
      GetHUDComponent()->SetDamageHelper( NULL );
   
      if( mVehicle.valid() )
      {
         // Change the visibilities of both the interior and exterior.
         mVehicle->SetDrawingModel( true );
   
         // --- If the player was a driver, let the vehicle know its driver has left.
         //     This will prevent the vehicle from moving while using the player
         //     walk motion model input; avoids the "Night Rider car" effect.
         SimCore::Actors::VehicleInterface* vehicle 
            = dynamic_cast<SimCore::Actors::VehicleInterface*>(mVehicle.get());
         if( vehicle != NULL )
         {
            vehicle->SetHasDriver( false );
         }
         // --- The seat could be attached to either the vehicle directly or its ring mount DOF.
         //     This function will not assume the current simulation mode is the old
         //     or the new mode set prior to this function being called; thus remove
         //     from both possible parents.
         mVehicle->RemoveChild( mSeat.get() );
         if( mDOFSeat.valid() ) { mDOFSeat->removeChild( mSeat->GetOSGNode() ); }
   
         // Reset the original orientations of the vehicle's DOFs.
         if( mDOFRing.valid() ) { mDOFRing->setCurrentHPR( mDOFRingOriginalHPR ); }
         if( mDOFWeapon.valid() ) { mDOFWeapon->setCurrentHPR( mDOFWeaponOriginalHPR ); }
   
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
      // Set NEW mode
      if( mWeaponEyePoint.valid() )
      {
         dtCore::Transform xform;
         mStealthActor->GetTransform( xform, dtCore::Transformable::REL_CS );
         xform.SetRotation( osg::Vec3(0.0,0.0,0.0) );
         mStealthActor->SetTransform( xform, dtCore::Transformable::REL_CS );
         mWeaponEyePoint->AddChild( mStealthActor.get() );
      }

      if( mWeaponMM.valid())
      {
         bool flamesPresent = mVehicle.valid() && mVehicle->IsFlamesPresent();
         mWeaponMM->SetEnabled( !flamesPresent);
      }
   
   }
   

   //////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::SetPlayer( SimCore::Actors::StealthActor* actor )
   {
      mStealthActor = actor;
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
      //mSoundTurretTurnStart->SetMinDistance(1.0f);
      //mSoundTurretTurnStart->SetMaxDistance(1.0f);
      //player.AddChild(mSoundTurretTurnStart.get());
   
      // Turret Turn
      //mSoundTurretTurn = dtAudio::AudioManager::GetInstance().NewSound();
      //mSoundTurretTurn->LoadFile(SOUND_TURRET_TURN.c_str());
      //mSoundTurretTurn->ListenerRelative(true);
      //mSoundTurretTurn->SetLooping(true);
      //mSoundTurretTurn->SetMinDistance(1.0f);
      //mSoundTurretTurn->SetMaxDistance(1.0f);
      //player.AddChild(mSoundTurretTurn.get());
   
      // Turret Turn End
      //mSoundTurretTurnEnd = dtAudio::AudioManager::GetInstance().NewSound();
      //mSoundTurretTurnEnd->LoadFile(SOUND_TURRET_TURN_END.c_str());
      //mSoundTurretTurnEnd->ListenerRelative(true);
      //mSoundTurretTurnEnd->SetLooping(false);
      //mSoundTurretTurnEnd->SetMinDistance(1.0f);
      //mSoundTurretTurnEnd->SetMaxDistance(1.0f);
      //player.AddChild(mSoundTurretTurnEnd.get());
   
      // Ambient (environmental sounds)
      //mSoundAmbient = dtAudio::AudioManager::GetInstance().NewSound();;
      //mSoundAmbient->LoadFile(SOUND_AMBIENT.c_str());
      //mSoundAmbient->ListenerRelative(false);
      //mSoundAmbient->SetLooping(true);
      //mSoundAmbient->SetMinDistance(1.0f);
      //mSoundAmbient->SetMaxDistance(1.0f);
      //player.AddChild(mSoundAmbient.get());
   }
   
   //////////////////////////////////////////////////////////////////////////
   void DriverInputComponent::InitializeWeapons()
   {
      dtCore::RefPtr<SimCore::Actors::WeaponActor> curWeapon;
      GameAppComponent* gameAppComponent;
      GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComponent);
   
      // weapons -- for array iterator.
      // WEAPON 1
      CreateWeapon( "Weapon_Grenade",
         "Particle_System_Weapon_Grenade",
         "weapon_gun_flash.osg", curWeapon );
      if( curWeapon.valid() ) { mWeaponList.push_back( curWeapon.get() ); }
      curWeapon = NULL;

      // WEAPON 2
      CreateWeapon( "Weapon_MachineGun",
         "Particle_System_Weapon_GunWithTracer",
         "weapon_gun_flash.osg", curWeapon );
      if( curWeapon.valid() ) { mWeaponList.push_back( curWeapon.get() ); }
      curWeapon = NULL;
   
      mWeaponIndex = 0;
   }
   
   //////////////////////////////////////////////////////////////////////////
   bool DriverInputComponent::CreateWeapon( const std::string& weaponName,
         const std::string& shooterName, const std::string& flashEffectFile,
            dtCore::RefPtr<SimCore::Actors::WeaponActor>& outWeapon )
   {
      dtGame::GameManager* gm = GetGameManager();
   
      std::vector<dtDAL::ActorProxy*> toFill;
      gm->FindPrototypesByName( weaponName, toFill );
   
      if( toFill.empty() )
      {
         LOG_ERROR( "No weapon actor prototype could be loaded from the map." );
         return false;
      }
   
      // Create a new weapon proxy from the discovered prototype
      dtCore::RefPtr<SimCore::Actors::WeaponActorProxy> weaponProxy 
         = dynamic_cast<SimCore::Actors::WeaponActorProxy*>
         (gm->CreateActorFromPrototype(toFill[0]->GetId()).get());
      outWeapon = weaponProxy.valid() ?
         static_cast<SimCore::Actors::WeaponActor*> (weaponProxy->GetActor()) : NULL;
   
      if( ! outWeapon.valid() )
      {
         LOG_ERROR( "Failure creating weapon actor." );
         return false;
      }
   
      // Call OnEnterWorld
      gm->AddActor( *weaponProxy, false, false );
      // Pull the weapon out of the scene so its parent is NULL.
      // This will help in attaching the weapon to a pivot.
      outWeapon->Emancipate();
   
      // Get ready to instantiate a shooter from a prototype.
      toFill.clear();
      gm->FindPrototypesByName( shooterName ,toFill);
   
      if( ! toFill.empty() )
      {
         dtCore::RefPtr<dtDAL::ActorProxy> ourActualActorProxy 
            = gm->CreateActorFromPrototype(toFill.front()->GetId());
   
         if(ourActualActorProxy != NULL)
         {
            // Give the shooter unit access to the shooter so that it
            // can tell the shooter when to fire.
            NxAgeiaMunitionsPSysActorProxy* proxy = 
               dynamic_cast<NxAgeiaMunitionsPSysActorProxy*>(ourActualActorProxy.get());
            outWeapon->SetShooter( proxy );
   
            // Initialize the physics based particle system/shooter,
            // only if the shooter unit was assigned a valid proxy.
            if( proxy != NULL )
            {
               NxAgeiaMunitionsPSysActor* shooter
                  = dynamic_cast<NxAgeiaMunitionsPSysActor*>(ourActualActorProxy->GetActor());
   
               // Set other properties of the particle system
               shooter->SetWeapon( *outWeapon );
   
               // HACK: temporary play values
               shooter->SetFrequencyOfTracers( outWeapon->GetTracerFrequency() );
   
               // Place the shooter into the world
               gm->AddActor( shooter->GetGameActorProxy(), false, false );
               shooter->Emancipate();
   
               // Attach the shooter to the weapon's flash point
               outWeapon->AttachObject( *shooter, "hotspot_01" );
            }
         }
      }
   
      // Create the flash effect for the weapon
      SimCore::Actors::WeaponFlashActor* flash = NULL;
      dtCore::RefPtr<SimCore::Actors::WeaponFlashActorProxy> flashProxy;
      gm->CreateActor( *SimCore::Actors::EntityActorRegistry::WEAPON_FLASH_ACTOR_TYPE, flashProxy );
      flash = static_cast<SimCore::Actors::WeaponFlashActor*>(flashProxy->GetActor());
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
      mHUDComponent->SetWeapon( NULL );

      // Clean up old weapon if there is already one set.
      if( mWeapon.valid() )
      {
         // Turn off the weapon if its trigger is currently held.
         mWeapon->SetTriggerHeld( false );
   
         // Remove the current weapon from both possible attach points.
         if( mDOFWeapon.valid() )
         {
            mDOFWeapon->removeChild( mWeapon->GetOSGNode() );
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
         if( mDOFWeapon.valid() )
         {
            mDOFWeapon->addChild( mWeapon->GetOSGNode() );
         }
   
         dtCore::Transform xform;
   
         if( mWeaponEyePoint.valid() )
         {
            // Offset the eye point
            mWeapon->AddChild( mWeaponEyePoint.get() );
            xform.SetTranslation( 0.0f, 0.0f, 0.0f );
            mWeaponEyePoint->SetTransform( xform, dtCore::Transformable::REL_CS );
         }
   
         // Offset the weapon
         xform.SetTranslation( 0.0, 0.0, 0.0 );
         mWeapon->SetTransform( xform, dtCore::Transformable::REL_CS );
      }
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
         mWeaponMM->SetUpDownLimit( 45.0f );
         mWeaponMM->SetName("WeaponMM");
      }
      mWeaponMM->SetEnabled( true );
      mWeaponMM->SetTargetDOF( mDOFWeapon.get() );
   
      // create the attached motion model for the ring mount DOF
      if( ! mRingMM.valid() )
      {
         mRingMM = new SimCore::ClampedMotionModel( app.GetKeyboard(), app.GetMouse() );
         mRingMM->SetMaximumMouseTurnSpeed(1440.0f*4.0f);
         mRingMM->SetUpDownLimit( 0.0f );
         mRingMM->SetName("RingMM");
      }

      // Look up a weird quirk of the data (only set on the hover vehicle actor).
      // Is the turret 'hard-wired' to the vehicle? If so, the ring mount motion 
      // model turns the vehicle, not just the ring mount DoF. This is true for 
      // instance, with the Hover Vehicle. 
      HoverVehicleActor *hoverActor = dynamic_cast<HoverVehicleActor *> (&vehicle);
      if (hoverActor != NULL && hoverActor->GetVehicleIsTurret())
         mRingMM->SetTarget(&vehicle);
      else
         mRingMM->SetTargetDOF( mDOFRing.get() );

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
            if( comp != NULL )
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
      mDOFSeat = vehicle.GetNodeCollector()->GetDOFTransform(DOF_NAME_RINGMOUNT_SEAT);
      mDOFRing = vehicle.GetNodeCollector()->GetDOFTransform(DOF_NAME_RINGMOUNT);
      mDOFWeapon = vehicle.GetNodeCollector()->GetDOFTransform(DOF_NAME_WEAPON_PIVOT);
      //mDOFWeaponStem = vehicle.GetNodeCollector()->GetDOFTransform(DOF_NAME_WEAPON_STEM);
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
         if( mVehicle.valid() && ! mVehicle->IsFlamesPresent() )
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
      
      gm->FindPrototypesByName("Hover_Target_Prototype", foundProxies);
      if( foundProxies.empty() )
      {
         LOG_ERROR( "No Hover Target prototype could be loaded from the map." );
         return;
      }

      // Create a new target from the discovered prototype
      dtCore::RefPtr<DriverDemo::HoverTargetActorProxy> newTargetProxy 
         = dynamic_cast<DriverDemo::HoverTargetActorProxy*>
         (gm->CreateActorFromPrototype(foundProxies[0]->GetId()).get());

      // Add the actor to the game manager.
      if (newTargetProxy.valid())
      {
         //DriverDemo::HoverTargetActor* targetActor;
         //targetProxy->GetActor(targetActor);
         GetGameManager()->AddActor(*newTargetProxy.get(), false, true);
      }
      else
         LOG_ERROR( "Failure creating Hover Target actor." );
   }

}
