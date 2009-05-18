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
#include <dtGame/baseinputcomponent.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/refptr.h>
#include <dtPhysics/physicshelper.h>
#include <SimCore/Components/BaseInputComponent.h>

namespace NetDemo
{
   class NETDEMO_EXPORT InputComponent : public SimCore::Components::BaseInputComponent
   {
      public:
         typedef SimCore::Components::BaseInputComponent BaseClass;

         /// Constructor
         InputComponent(const std::string& name = dtGame::BaseInputComponent::DEFAULT_NAME);

         virtual void ProcessMessage(const dtGame::Message& message);

         /**
          *
          */
         virtual void OnAddedToGM();

         /**
          * Called when this component is removed from the GM
          */
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
      private:
         typedef std::vector<dtCore::RefPtr<dtPhysics::PhysicsHelper> > HelperList;
         dtCore::RefPtr<dtCore::FlyMotionModel> mMotionModel;
         HelperList mHelpers;
   };
}

#endif
