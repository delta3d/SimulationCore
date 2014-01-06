/* -*-c++-*-
* Driver Demo - DriverInputComponent (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
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
#include <SimCore/Actors/DRGhostActor.h>
#include <dtUtil/refcountedbase.h>

#include <osgSim/DOFTransform>

namespace dtAudio
{
   class Sound;
}

namespace dtGame
{
   class Message;
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
      class BasePhysicsVehicleActor;
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
   //
   // NOTE - The camera sits at the bottom of a VERY large hierarchy of DoF's. Looks like this:
   //     Vehicle (center of vehicle)
   //       - Ring Mount (often swivels left/right)
   //           - mDoFWeapon (pivots about weapon pivot point)
   //               - mWeapon (3D model of weapon)
   //                   - mWeaponEyePoint (offset for human eyepoint)
   //                       - StealthActor (yay!  almost there)
   //                           - camera
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

         // GUI Tool Methods: to be moved to a new GUI Component.
         SimCore::Tools::Tool* GetTool(SimCore::MessageType &type);
         void AddTool(SimCore::Tools::Tool &tool, SimCore::MessageType &type);
         void RemoveTool(SimCore::MessageType &type);
         bool IsToolEnabled(SimCore::MessageType &type) const;
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
         void AttachToVehicle( SimCore::Actors::BasePhysicsVehicleActor& vehicle );

         // Detach the player from the current vehicle to which he is attached.
         // This function will enable the walk motion model that controls the player.
         void DetachFromVehicle();

         // sets the mVehicle, used through gameappcomponent
         void SetCurrentVehicle( SimCore::Actors::BasePhysicsVehicleActor& vehicle) {mVehicle = &vehicle;}

         // Determines if the vehicle can be "turned on a dime".
         bool IsVehiclePivotable( const SimCore::Actors::BasePhysicsVehicleActor& vehicle ) const;

            // This function is called by Attach to vehicle when the
         // application is in GUNNER mode.
         // This does the dirty work of setting up the ring mount DOF
         // with a motion model.
         //
         // @param vehicle The Vehicle that has the ring mount.
         // @return TRUE if attach was successful
         //
         // NOTE: This is automatically called by AttachToVehicle.
         bool AttachToRingmount( SimCore::Actors::BasePhysicsVehicleActor& vehicle );

         void SetPlayer( SimCore::Actors::StealthActor* actor );
         //SimCore::Actors::BasePhysicsVehicleActor* GetVehicle();

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
            std::shared_ptr<SimCore::Actors::WeaponActor>& outWeapon );

         // Assign the current weapon that is being used.
         // This function will do all the necessary work of removing the old weapon
         // references and set up references to the new specified weapon.
         // @param weapon The weapon instance that will be swapped into the scene
         //        and used as the current weapon. Specify nullptr to not used any weapons.
         void SetWeapon( SimCore::Actors::WeaponActor* weapon );

         // Cycle between the currently loaded list weapons.
         // @param direction The index offset into the weapon list that points to
         //        the weapon to which to be cycled.
         void CycleWeapon( int direction );

         void SetStartPosition( const osg::Vec3& position ) { mStartPosition = position; }
         const osg::Vec3& GetStartPosition() const { return mStartPosition; }


         /// Method to create a test target that can be shot
         void CreateTarget();

      protected:

         /// Destructor
         virtual ~DriverInputComponent();

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
         void GetVehicleDOFs( SimCore::Actors::BasePhysicsVehicleActor& vehicle );

         void EnableMotionModels( bool enable );

         // Used in deleting all weapons when the player disembarks from a vehicle.
         void DeleteWeapons();

         // Hack - Curttest code to create a test target actor
         //void CreateTarget();

         void SetViewMode();

         void ToggleView();
         void AttachToView( const std::string& viewNodeName );

         /// Clean up method for the dead reckoning ghost actor
         void CleanUpDRGhost();
         /// Create or destroy the dead reckoning ghost actor
         void ToggleDRGhost();

         void KillEnemy(bool killAllEnemies);

      private:

         void HandleHelpPressed();
         DriverHUD* GetHUDComponent();

         std::shared_ptr<dtGame::MachineInfo> mMachineInfo;

         std::shared_ptr<DriverHUD>    mHUDComponent;
         dtCore::UniqueId              mCurrentActorId;

         // logging methods and vars
         void JoinFederation();
         void LeaveFederation();

         // Variables needed for joining and leaving a federation
         //std::string mExecutionName;
         //std::string mFedFile;
         //std::string mFederateName;
         bool mUsePhysicsDemoMode;

         // Tools
         std::map<SimCore::MessageType*, std::shared_ptr<SimCore::Tools::Tool> > mToolList;

         // Simulator specific objects
         std::shared_ptr<SimCore::Actors::HumanWithPhysicsActorProxy> mPlayerAvatarProxy;
         std::shared_ptr<SimCore::Actors::HumanWithPhysicsActor> mPlayerAvatar;
         std::shared_ptr<SimCore::Actors::BasePhysicsVehicleActor> mVehicle;
         std::shared_ptr<SimCore::Actors::BasePhysicsVehicleActorProxy> mVehicleProxy;
         // ??? std::shared_ptr<SimCore::Actors::InteriorActor> mInterior;
         std::shared_ptr<SimCore::Actors::WeaponActor> mWeapon;                    // current weapon
         std::vector<std::shared_ptr<SimCore::Actors::WeaponActor> > mWeaponList;  // all weapons
         unsigned mWeaponIndex;

         // Special DOF's acquired from loaded models
         std::shared_ptr<dtCore::Transformable> mSeat;

         // Direct DOF references used by controls states
         osg::observer_ptr<osgSim::DOFTransform> mDOFSeat;
         osg::observer_ptr<osgSim::DOFTransform> mDOFRing;
         osg::observer_ptr<osgSim::DOFTransform> mDOFWeapon;
         osg::observer_ptr<osgSim::DOFTransform> mDOFFirePoint;
         //osg::observer_ptr<osgSim::DOFTransform> mDOFWeaponStem;
         //float mDOFWeaponStemOffset; // offset up/down of the weapon mount
         //float mDOFWeaponStemOffsetLimit; // the maximum up/down displacement of the weapon mount (in meters)

         // The original orientations of the above DOFs prior to modification.
         // These will be used to reset the orientations of the DOFs when detaching
         // the player from the current vehicle.
         osg::Vec3 mDOFRingOriginalHPR;
         osg::Vec3 mDOFWeaponOriginalHPR;

         // Gunner app related motion models
         std::shared_ptr<SimCore::ClampedMotionModel> mRingMM; // moves the seat
         std::shared_ptr<SimCore::ClampedMotionModel> mWeaponMM; // moves the weapon pivot
         //std::shared_ptr<SimCore::PlayerMotionModel> mWalkMM; // moves the seat (the player's root transformable)

         // View points
         std::shared_ptr<dtCore::Transformable> mWeaponEyePoint; // added to the weapon pivot DOF
         //std::shared_ptr<dtCore::Transformable> mSitEyePoint; // added to seat DOF
         //std::shared_ptr<dtCore::Transformable> mStandEyePoint; // added to seat DOF
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

         // View mode variable.
         std::string mViewNodeName;

         // Initial starting position
         osg::Vec3 mStartPosition;

         // HACK: variables for hacking the ephemeris to update to the correct time of day
         float mEnvUpdateTime;
         int mEnvUpdateAttempts;

         // Sounds effects no contained by a vehicle directly
         std::shared_ptr<dtAudio::Sound> mSoundTurretTurnStart;
         std::shared_ptr<dtAudio::Sound> mSoundTurretTurn;
         std::shared_ptr<dtAudio::Sound> mSoundTurretTurnEnd;
         std::shared_ptr<dtAudio::Sound> mSoundAmbient;

         // Sound file names
         static const dtUtil::RefString SOUND_TURRET_TURN_START;
         static const dtUtil::RefString SOUND_TURRET_TURN;
         static const dtUtil::RefString SOUND_TURRET_TURN_END;
         static const dtUtil::RefString SOUND_AMBIENT;

         // The common DOF names found on most vehicle models
         static const dtUtil::RefString DOF_NAME_WEAPON_PIVOT;
         static const dtUtil::RefString DOF_NAME_WEAPON_FIRE_POINT;
         static const dtUtil::RefString DOF_NAME_RINGMOUNT;
         static const dtUtil::RefString DOF_NAME_VIEW_01;
         static const dtUtil::RefString DOF_NAME_VIEW_02;
         static const dtUtil::RefString DOF_NAME_VIEW_DEFAULT;

         enum DR_GHOST_MODE { NONE = 1, GHOST_ON };
         DR_GHOST_MODE mDRGhostMode;
         std::shared_ptr<SimCore::Actors::DRGhostActorProxy> mDRGhostActorProxy;

   };
}
#endif
