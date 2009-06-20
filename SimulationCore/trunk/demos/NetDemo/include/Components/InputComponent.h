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
 * David Guthrie
 */
#ifndef RES_GAME_INPUT_COMPONENT
#define RES_GAME_INPUT_COMPONENT

#include <DemoExport.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/refptr.h>
#include <SimCore/Components/BaseInputComponent.h>
#include <dtCore/observerptr.h>
#include <osgSim/DOFTransform>

//#include <SimCore/MessageType.h>
//#include <SimCore/PlayerMotionModel.h>
//#include <SimCore/Actors/WeaponActor.h>

//namespace osgSim
//{
//   class DOFTransform;
//}
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

   ////////////////////////////////////////////////////////////////////
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

      protected:
         virtual ~InputComponent();
         void FireSomething();
         void DoRayCast();
         void SetupMaterialsAndTerrain();
         void UpdateHelpers();
         GameLogicComponent* GetLogicComponent();

         void HandleActorUpdateMessage(const dtGame::Message& msg);
         bool IsVehiclePivotable();
         void DetachFromCurrentVehicle();

         void AttachToVehicle(SimCore::Actors::Platform* vehicle);
         void EnableMotionModels();

         void HandleStateChangeMessage(
            const SimCore::Components::GameStateChangedMessage& stateChange);

         /// Sending in a vehicle will cause an attach, sending NULL will detach
         void SendAttachOrDetachMessage(const dtCore::UniqueId& vehicleId, const std::string& dofName);

      private:
         dtCore::RefPtr<SimCore::Actors::Platform> mVehicle;
         dtCore::RefPtr<dtCore::FlyMotionModel> mMotionModel;
         dtCore::ObserverPtr<osgSim::DOFTransform> mDOFRing;
         dtCore::ObserverPtr<osgSim::DOFTransform> mDOFWeapon;
         dtCore::RefPtr<SimCore::ClampedMotionModel> mRingMM; // moves the seat
         dtCore::RefPtr<SimCore::ClampedMotionModel> mWeaponMM; // moves the weapon pivot
         std::vector<std::string> mViewPointList;
         unsigned mCurrentViewPointIndex;
         bool mIsInGameState;
   };
}

#endif
