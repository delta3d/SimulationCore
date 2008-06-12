/* -*-c++-*-
* Driver Demo
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*
* @author Curtiss Murphy
*/
#ifndef DRIVER_INPUT_COMPONENT_H_
#define DRIVER_INPUT_COMPONENT_H_

#include <DriverExport.h>
#include <GameAppComponent.h>
#include <DriverHUD.h>

#include <SimCore/Components/BaseInputComponent.h>
#include <SimCore/MessageType.h>
#include <SimCore/PlayerMotionModel.h>
#include <SimCore/Actors/WeaponActor.h>

namespace dtAudio
{
   class Sound;
}

namespace dtGame
{
   class Message;
}

namespace dtHLAGM
{
   class HLAComponent;
}

namespace dtUtil
{
   class Enumeration;
}

namespace SimCore
{
   class ClampedMotionModel;
   class StealthMotionModel;

   namespace Actors
   {
      class HumanWithPhysicsActor;
      class HumanWithPhysicsActorProxy;
      class NxAgeiaFourWheelVehicleActor;
      class NxAgeiaFourWheelVehicleActorProxy;
      class InteriorActor;
      class VehicleInterface;
   }

   namespace Tools
   {
      class Tool;
  }
}
namespace DriverDemo
{
   class GameAppComponent;
   
   ////////////////////////////////////////////////////////////////////////////////
   // INPUT COMPONENT CODE
   ////////////////////////////////////////////////////////////////////////////////
   class DRIVER_DEMO_EXPORT DriverInputComponent : public SimCore::Components::BaseInputComponent
   {
      private:
         typedef SimCore::Components::BaseInputComponent BaseClass;

      public:
         
         
         /// Constructor
         DriverInputComponent(const std::string& name = BaseClass::DEFAULT_NAME);
   
         void ProcessMessage(const dtGame::Message &message);
   
         virtual bool HandleKeyPressed(const dtCore::Keyboard* keyboard, int key);
   
         virtual bool HandleKeyReleased(const dtCore::Keyboard* keyboard, int key);
   
         virtual bool HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button);
   
         virtual bool HandleButtonReleased(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button);
         
         void HandleTurretEnabled( bool enable );
   
         void SetConnectionParameters(const std::string& executionName,
            const std::string& fedFile,
            const std::string& federateName);
   
         SimCore::Tools::Tool* GetTool(SimCore::MessageType &type);
         void AddTool(SimCore::Tools::Tool &tool, SimCore::MessageType &type);
         void RemoveTool(SimCore::MessageType &type);
         bool IsToolEnabled(SimCore::MessageType &type) const;
         void UpdateTools( float timeDelta );
         void ToggleTool(SimCore::MessageType &msgType);
         SimCore::MessageType& GetEnabledTool() const;
         void DisableAllTools();
         void SetToolEnabled(SimCore::MessageType &toolType, bool enable);

         //SimCore::Actors::CameraAttachmentActor* StealthInputComponent::GetAttachmentActor();
   
         // Attach the player to the specified vehicle onto certain DOFs, determined
         // by the application's set SimulationMode.
         // @param vehicle The physics vehicle onto which the player should be attached.
         //
         // NOTE: The relevant simulation mode must be set prior to calling this function.
         //       This function will position the player based on the simulation mode.
         void AttachToVehicle( SimCore::Actors::Platform& vehicle );
   
         // Detach the player from the current vehicle to which he is attached.
         // This function will enable the walk motion model that controls the player.
         void DetachFromVehicle();
   
         // sets the mVehicle, used through gameappcomponent
         void SetCurrentVehicle( SimCore::Actors::Platform& vehicle) {mVehicle = &vehicle;}
   
         // This function is called by Attach to vehicle when the
         // application is in GUNNER mode.
         // This does the dirty work of setting up the ring mount DOF
         // with a motion model.
         //
         // @param vehicle The Vehicle that has the ring mount.
         // @return TRUE if attach was successful
         //
         // NOTE: This is automatically called by AttachToVehicle.
         bool AttachToRingmount( SimCore::Actors::Platform& vehicle );
         
         void SetPlayer( SimCore::Actors::StealthActor* actor );
         SimCore::Actors::Platform* GetVehicle();
   
         // Stores the default camera perspective to be set
         // when the camera is exiting tool perspectives
         // such as those found in Binoculars and LRF.
         // This is a temporary work around for IPT1.
         void SetDefaultCameraPerspective( 
            float horizontalFOV, float verticalFOV, 
            float nearClip, float farClip )
         {
            mHorizontalFOV = horizontalFOV;
            mVerticalFOV = verticalFOV;
            mNearClip = nearClip;
            mFarClip = farClip;
         }
   
         // Sets up the player, motion models, and other objects needed
         // over all application modes.
         void InitializePlayer( SimCore::Actors::StealthActor& player );
     
         // Creates all the weapons (from prototypes) that this simulator will be using.
         void InitializeWeapons();
   
         // Create all sounds used in this simulator.
         void InitializeSounds( SimCore::Actors::StealthActor& player );
   
         // Create an instance of a single weapon from a prototype in the loaded map.
         // @param weaponName The name of the weapon as it is found in the map file.
         // @param shooterName The name of the PhysX particle system as it is found in the map file.
         // @param flashEffectFile The file name of the particle system to be used
         //        as the flash effect for the specified weapon
         // @param outWeapon Reference pointer that will capture the new weapon instance.
         // @return TRUE if the weapon was successfully created.
         bool CreateWeapon( const std::string& weaponName, const std::string& shooterName,
            const std::string& flashEffectFile,
            dtCore::RefPtr<SimCore::Actors::WeaponActor>& outWeapon );
   
         // Assign the current weapon that is being used.
         // This function will do all the necessary work of removing the old weapon
         // references and set up references to the new specified weapon.
         // @param weapon The weapon instance that will be swapped into the scene
         //        and used as the current weapon. Specify NULL to not used any weapons.
         void SetWeapon( SimCore::Actors::WeaponActor* weapon );
   
         // Cycle between the currently loaded list weapons.
         // @param direction The index offset into the weapon list that points to
         //        the weapon to which to be cycled.
         void CycleWeapon( int direction );
   
         void SetStartPosition( const osg::Vec3& position ) { mStartPosition = position; }
         const osg::Vec3& GetStartPosition() const { return mStartPosition; }
   
      protected:
   
         /// Destructor
         virtual ~DriverInputComponent();
   
         //void Cycle(bool forward, bool attach);
         void ToggleEntityShaders();
   
         void ResetTurnSpeeds();
   
         // This function tracks states such as the vehicle's damage state.
         // In the case of damage state changes, this function will do the required
         // setup of the vehicle object exterior, important for the Gunner simulation.
         // @param simDelta The change in simulation time since the last frame.
         // @param realDelta The change in real world time since the last frame.
         void UpdateStates( float simDelta, float realDelta );
   
         void UpdateSounds();
   
         // Capture the DOFs of the specified vehicle.
         // @param vehicle The vehicle from which to obtain the DOFs needed by the
         //        gunner and driver simulators
         void GetVehicleDOFs( SimCore::Actors::Platform& vehicle );
   
         void EnableMotionModels( bool enable );
   
         // Used in deleting all weapons when the player disembarks from a vehicle.
         void DeleteWeapons();
      
         // Hack - Curttest code to create a test target actor
         //void CreateTarget();
   
         void SetViewMode();

      private:
   
         // ?:???? void UpdateInteriorModel();
         void StopAnyWeaponsFiring();

         // The common DOF names found on most vehicle models 
         const std::string DOF_NAME_WEAPON_PIVOT;
         const std::string DOF_NAME_WEAPON_STEM;
         const std::string DOF_NAME_RINGMOUNT;
         const std::string DOF_NAME_RINGMOUNT_SEAT;
   
         void HandleHelpPressed();
         DriverHUD* GetHUDComponent();
   
         dtCore::RefPtr<DriverHUD>    mHUDComponent;
         dtCore::UniqueId              mCurrentActorId;
         
         // logging methods and vars
         void JoinFederation();
         void LeaveFederation();
         
         // Variables needed for joining and leaving a federation
         //std::string mExecutionName;
         //std::string mFedFile;
         //std::string mFederateName;
         bool mIsConnected;
         bool mUsePhysicsDemoMode;
         dtCore::RefPtr<dtHLAGM::HLAComponent> mHLA;
   
         // Tools
         std::map<SimCore::MessageType*, dtCore::RefPtr<SimCore::Tools::Tool> > mToolList;
   
         // Simulator specific objects
         dtCore::RefPtr<SimCore::Actors::HumanWithPhysicsActorProxy> mPlayerAvatarProxy;
         dtCore::RefPtr<SimCore::Actors::HumanWithPhysicsActor> mPlayerAvatar;
         dtCore::RefPtr<SimCore::Actors::Platform> mVehicle;
         dtCore::RefPtr<SimCore::Actors::PlatformActorProxy> mVehicleProxy;
         // ??? dtCore::RefPtr<SimCore::Actors::InteriorActor> mInterior;
         dtCore::RefPtr<SimCore::Actors::WeaponActor> mWeapon;                    // current weapon
         std::vector<dtCore::RefPtr<SimCore::Actors::WeaponActor> > mWeaponList;  // all weapons
         unsigned mWeaponIndex;
      
         // Special DOF's acquired from loaded models
         dtCore::RefPtr<dtCore::Transformable> mSeat;
   
         // Direct DOF references used by controls states
         osg::observer_ptr<osgSim::DOFTransform> mDOFSeat;
         osg::observer_ptr<osgSim::DOFTransform> mDOFRing;
         osg::observer_ptr<osgSim::DOFTransform> mDOFWeapon;
         osg::observer_ptr<osgSim::DOFTransform> mDOFWeaponStem;
         float mDOFWeaponStemOffset; // offset up/down of the weapon mount
         float mDOFWeaponStemOffsetLimit; // the maximum up/down displacement of the weapon mount (in meters)
   
         // The original orientations of the above DOFs prior to modification.
         // These will be used to reset the orientations of the DOFs when detaching
         // the player from the current vehicle.
         osg::Vec3 mDOFRingOriginalHPR;
         osg::Vec3 mDOFWeaponOriginalHPR;
   
         // Gunner app related motion models
         dtCore::RefPtr<SimCore::ClampedMotionModel> mRingMM; // moves the seat
         dtCore::RefPtr<SimCore::ClampedMotionModel> mWeaponMM; // moves the weapon pivot
         //dtCore::RefPtr<SimCore::PlayerMotionModel> mWalkMM; // moves the seat (the player's root transformable)
   
         // View points
         dtCore::RefPtr<dtCore::Transformable> mWeaponEyePoint; // added to the weapon pivot DOF
         //dtCore::RefPtr<dtCore::Transformable> mSitEyePoint; // added to seat DOF
         //dtCore::RefPtr<dtCore::Transformable> mStandEyePoint; // added to seat DOF
         //ViewMode mViewMode;
   
         // Store the camera perspective to be set after coming
         // out of a tool's change in perspective.
         float mHorizontalFOV;
         float mVerticalFOV;
         float mNearClip;
         float mFarClip;
   
         // Gunner specific key/button states
         bool mRingKeyHeld;
         bool mRingButtonHeld;
         bool mMotionModelsEnabled;
   
         // Gunner/Driver related states
         bool mPlayerAttached;
         SimCore::Actors::BaseEntityActorProxy::DamageStateEnum* mLastDamageState; 
         // the last reported state of the vehicle
   
         // Initial starting position
         osg::Vec3 mStartPosition;
   
         // HACK: variables for hacking the ephemeris to update to the correct time of day
         float mEnvUpdateTime;
         int mEnvUpdateAttempts;
   
         // Sounds effects no contained by a vehicle directly
         dtCore::RefPtr<dtAudio::Sound> mSoundTurretTurnStart;
         dtCore::RefPtr<dtAudio::Sound> mSoundTurretTurn;
         dtCore::RefPtr<dtAudio::Sound> mSoundTurretTurnEnd;
         dtCore::RefPtr<dtAudio::Sound> mSoundAmbient;
   
         // Sound file names
         static const std::string SOUND_TURRET_TURN_START;
         static const std::string SOUND_TURRET_TURN;
         static const std::string SOUND_TURRET_TURN_END;
         static const std::string SOUND_AMBIENT;
   };
}
#endif
