/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008, Alion Science and Technology
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
 * David Guthrie, Curtiss Murphy
 */
#ifndef RES_GAME_INPUT_COMPONENT
#define RES_GAME_INPUT_COMPONENT

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <DemoExport.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/refptr.h>
#include <SimCore/Components/BaseInputComponent.h>
#include <dtCore/observerptr.h>
#include <osgSim/DOFTransform>
#include <SimCore/Actors/DRGhostActor.h>

//#include <SimCore/MessageType.h>
//#include <SimCore/PlayerMotionModel.h>
//#include <SimCore/Actors/WeaponActor.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtGame
{
   class ActorUpdateMessage;
}
namespace SimCore
{
   class ClampedMotionModel;
   class StealthMotionModel;
   namespace Actors
   {
      class Platform;
   }
   namespace Components
   {
      class GameStateChangedMessage;
   }
}

namespace NetDemo
{
   class GameLogicComponent;
   class MessageType;

   /////////////////////////////////////////////////////////////////////////////
   // CODE
   /////////////////////////////////////////////////////////////////////////////
   class NETDEMO_EXPORT InputComponent : public SimCore::Components::BaseInputComponent
   {
      public:
         typedef SimCore::Components::BaseInputComponent BaseClass;

         // The common DOF names found on most vehicle models
         static const dtUtil::RefString DOF_NAME_WEAPON_PIVOT;
         static const dtUtil::RefString DOF_NAME_WEAPON_FIRE_POINT;
         static const dtUtil::RefString DOF_NAME_RINGMOUNT;
         static const dtUtil::RefString DOF_NAME_VIEW_01;
         static const dtUtil::RefString DOF_NAME_VIEW_02;
         static const dtUtil::RefString DOF_TOPDOWN_VIEW_01;
         static const dtUtil::RefString DOF_TOPDOWN_VIEW_02;

         /// Constructor
         InputComponent(const std::string& name = dtGame::BaseInputComponent::DEFAULT_NAME);

         virtual void ProcessMessage(const dtGame::Message& message);

         virtual void OnAddedToGM();

         virtual void OnRemovedFromGM();

         /**
          * KeyboardListener call back- Called when a key is pressed.
          * Override this if you want to handle this listener event.
          * Default handles the Escape key to quit.
          *
          * @param keyboard the source of the event
          * @param key the key pressed
          * @param character the corresponding character
          */
         virtual bool HandleKeyPressed(const dtCore::Keyboard* keyboard, int key);

         bool HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button);

         bool HandleButtonReleased(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button);

         /// Updates the debug info structure on the Game Logic component and sends a UI_DEBUGINFO_UPDATED message
         void UpdateDebugInfo(bool sendUpdateMessage);

         /// Toggles the Debug Info flag and sends a message - called when F2 is pressed
         void ToggleDebugInfo();

      protected:
         virtual ~InputComponent();
         void FireSomething();
         void DoRayCast();
         void SetupMaterialsAndTerrain();
         void UpdateHelpers();
         GameLogicComponent* GetLogicComponent();

         void HandleActorUpdateMessage(const dtGame::ActorUpdateMessage& msg);
         bool IsVehiclePivotable();
         void DetachFromCurrentVehicle();

         void AttachToVehicle(SimCore::Actors::Platform* vehicle);
         void SetEnableHeadlight(bool b);
         void EnableMotionModels();

         void AddTopDownNode(float height, const dtUtil::RefString& nodeName);

         void HandleStateChangeMessage(
            const SimCore::Components::GameStateChangedMessage& stateChange);

         /// Sending in a vehicle will cause an attach, sending NULL will detach
         void SendAttachOrDetachMessage(const dtCore::UniqueId& vehicleId, const std::string& dofName);

         /// Send a simple message to trigger other parts of the game system.
         void SendSimpleMessage(const NetDemo::MessageType& messageType);

         /// Clean up method for the dead reckoning ghost actor
         void CleanUpDRGhost();
         /// Create or destroy the dead reckoning ghost actor
         void ToggleDRGhost();

         /// Turns on/off using velocity settings for DR testing. 
         void ToggleVelocityDR();

         /// Cycles between one of the debug toggle modes. Use with ToggleCurrentDebugState() 
         void ChangeDebugToggleMode();

         /// Changes the current debug toggle mode.
         void ToggleCurrentDebugMode();

         /// Debug toggle - changes whether we use the DR fixed blend time or let it do an avg based on update rate
         void ToggleFixedBlendTime();

         /// Debug toggle - changes between use Cubic Spline or Linear for Dead Reckoning
         void ToggleUseCubicSplineForDR();

         /// Debug toggle - changes between publishing the angular velocity or not (for DR).
         void TogglePublishAngularVelocity();

         /// Debug toggle - changes between the various dead reckoning algorithms (static, velocity, etc)
         void ToggleDeadReckoningAlgorithm();

         /// Turns on/off the use of ground clamping (aka flying) related to Dead reckoning
         void ToggleGroundClamping();

         /// Increase or decrease the publish rate (1.10 increases time, 0.90 decreases time
         /// increase or decrease the number of publishes per second (usually +1 or -1)
         void ModifyVehiclePublishRate(int incrementValue); // float scaleFactor = 1.0f);

         /// Increase or decrease the DR smoothing rate (1.10 increases time, 0.90 decreases time
         void ModifyVehicleSmoothingRate(float scaleFactor = 1.0f);
         
         /// Resets the DR values as best as we can
         void ResetTestingValues();

         /// Kills one or more enemies instantly.
         void KillEnemy(bool killAllEnemies);


      private:
         enum DR_GHOST_MODE { NONE = 1, GHOST_ON, ATTACH_TO_GHOST, HIDE_REAL, DETACH_FROM_VEHICLE };
         DR_GHOST_MODE mDRGhostMode;
         enum DEBUG_TOGGLE_MODE { DEBUG_TOGGLE_DR_ALGORITHM, DEBUG_TOGGLE_PUBLISH_ANGULAR_VELOCITY, 
            DEBUG_TOGGLE_DR_WITH_CUBIC_SPLINE, DEBUG_TOGGLE_GROUND_CLAMPING, DEBUG_FIXED_BLEND_TIME};
         DEBUG_TOGGLE_MODE mDebugToggleMode; 

         dtCore::RefPtr<SimCore::Actors::Platform> mVehicle;
         dtCore::RefPtr<SimCore::Actors::DRGhostActorProxy> mDRGhostActorProxy;
         dtCore::RefPtr<dtCore::FlyMotionModel> mMotionModel;
         dtCore::ObserverPtr<osgSim::DOFTransform> mDOFRing;
         dtCore::ObserverPtr<osgSim::DOFTransform> mDOFWeapon;
         dtCore::RefPtr<SimCore::ClampedMotionModel> mRingMM; // moves the seat
         dtCore::RefPtr<SimCore::ClampedMotionModel> mWeaponMM; // moves the weapon pivot
         std::vector<std::string> mViewPointList;
         unsigned mCurrentViewPointIndex;
         bool mIsInGameState;
         float mOriginalPublishTimesPerSecond;
         int mMaxPublishRate;
   };
}

#endif
