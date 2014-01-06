/* -*-c++-*-
 * SimulationCore
 * Copyright 2013, David Guthrie
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
#include <prefix/SimCorePrefix.h>
#include <SimCore/ActComps/KeyboardWheeledVehicleInputActComp.h>

#include <dtCore/keyboard.h>
#include <dtUtil/log.h>
#include <dtUtil/functor.h>
#include <dtUtil/getsetmacros.h>
#include <dtGame/invokable.h>
#include <dtGame/gameactor.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#include <dtABC/application.h>
#include <dtUtil/mathdefines.h>

#include <dtCore/propertymacros.h>

namespace SimCore
{
   namespace ActComps
   {

      KeyboardWheeledVehicleInputActComp::KeyboardWheeledVehicleInputActComp()
      : mNumUpdatesUntilFullSteeringAngle(25)
      , mLastSteeringAngle(0.0f)
      , mLastBrakesNormalized(0.0f)
      , mLastAcceleratorNormalized(0.0f)
      {
      }

      ////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(KeyboardWheeledVehicleInputActComp, int, NumUpdatesUntilFullSteeringAngle);

      ////////////////////////////////////////////////
      KeyboardWheeledVehicleInputActComp::~KeyboardWheeledVehicleInputActComp()
      {

      }

      ////////////////////////////////////////////////
      void KeyboardWheeledVehicleInputActComp::Update(const dtGame::TickMessage& msg)
      {
         dtGame::GameActor* actor = nullptr;
         GetOwner(actor);

         dtCore::Keyboard *keyboard = actor->GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
         if (keyboard == nullptr)
         {
            return;
         }

         if (keyboard->GetKeyState('w') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Up))
         {
            mLastAcceleratorNormalized = 1.0f;
         }
         else if (keyboard->GetKeyState('s') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Down))
         {
            mLastAcceleratorNormalized = -1.0f;
         }
         else
         {
            mLastAcceleratorNormalized = 0.0f;
         }

         if (keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
         {
            mLastBrakesNormalized = 1.0f;
         }
         else
         {
            mLastBrakesNormalized = 0.0f;
         }

         float updateCountInSecs = float(mNumUpdatesUntilFullSteeringAngle) / 60.00f;
         float angleChange = msg.GetDeltaSimTime() / updateCountInSecs;
         float recoverAngleChange = 2.0f * angleChange;

         bool turnLeft = keyboard->GetKeyState('a') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Left);
         bool turnRight = keyboard->GetKeyState('d') || keyboard->GetKeyState(osgGA::GUIEventAdapter::KEY_Right);

         // If you are turning the wheel or letting the wheel turn toward centered, then turn at double speed
         if (!turnRight && mLastSteeringAngle < -recoverAngleChange)
         {
            mLastSteeringAngle += recoverAngleChange;
         }
         else if (!turnLeft && mLastSteeringAngle > recoverAngleChange)
         {
            mLastSteeringAngle -= recoverAngleChange;
         }
         else if (turnLeft)
         {
            mLastSteeringAngle += angleChange;
         }
         else if (turnRight)
         {
            mLastSteeringAngle -= angleChange;
         }
         else
         {
            mLastSteeringAngle = 0.0f;
         }

         dtUtil::Clamp(mLastSteeringAngle, -1.0f, 1.0f);
      }


      void KeyboardWheeledVehicleInputActComp::BuildPropertyMap()
      {
         const dtUtil::RefString VEH_GROUP   = "Wheeled Vehicle Keyboard Input";

         typedef dtDAL::PropertyRegHelper<KeyboardWheeledVehicleInputActComp&, KeyboardWheeledVehicleInputActComp> PropRegType;
         PropRegType propRegHelper(*this, this, VEH_GROUP);

         DT_REGISTER_PROPERTY_WITH_LABEL(NumUpdatesUntilFullSteeringAngle, "Num updates until full steering angle",
                  "Number of update cycles to go from straight to full turn angle when using the keyboard.",
                  PropRegType, propRegHelper);

      }

      ////////////////////////////////////////////////
      void KeyboardWheeledVehicleInputActComp::OnEnteredWorld()
      {
         dtGame::GameActor* actor = nullptr;
         GetOwner(actor);

         if (actor == nullptr)
         {
            LOG_ERROR("The owner actor is not a transformable!  Can't rotate the wheels.");
            return;
         }

         std::string tickInvokable = "Tick Remote " + GetType().Get();
         if (actor->GetGameActorProxy().GetInvokable(tickInvokable) == nullptr)
         {
            actor->GetGameActorProxy().AddInvokable(*new dtGame::Invokable(tickInvokable,
               dtUtil::MakeFunctor(&KeyboardWheeledVehicleInputActComp::Update, this)));
         }
         actor->GetGameActorProxy().RegisterForMessages(dtGame::MessageType::TICK_LOCAL, tickInvokable);
      }

      ////////////////////////////////////////////////
      float KeyboardWheeledVehicleInputActComp::GetSteeringAngleNormalized()
      {
         return mLastSteeringAngle;
      }

      ////////////////////////////////////////////////
      float KeyboardWheeledVehicleInputActComp::GetBrakesNormalized()
      {
         return mLastBrakesNormalized;
      }

      ////////////////////////////////////////////////
      float KeyboardWheeledVehicleInputActComp::GetAcceleratorNormalized()
      {
         return mLastAcceleratorNormalized;
      }

   }
}
