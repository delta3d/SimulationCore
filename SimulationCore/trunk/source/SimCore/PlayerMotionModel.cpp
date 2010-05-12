/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
* circumstances in which the U. S. Government may have rights in the software.hor Chris Rodgers
 */

#include <prefix/SimCorePrefix.h>
#include <dtCore/base.h>
#include <dtCore/deltadrawable.h>
#include <dtCore/isector.h>
#include <dtCore/keyboard.h>
#include <dtCore/logicalinputdevice.h>
#include <dtCore/mouse.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/transform.h>
#include <dtABC/application.h>

#include <SimCore/PlayerMotionModel.h>
#include <SimCore/Components/PortalComponent.h>

#include <SimCore/Actors/HumanWithPhysicsActor.h>
#include <SimCore/Actors/Platform.h>

namespace SimCore
{
   IMPLEMENT_MANAGEMENT_LAYER(PlayerMotionModel);
   
   //////////////////////////////////////////////////////////////////////////         
   // PLAYER MOTION MODEL CODE
   //////////////////////////////////////////////////////////////////////////         
   PlayerMotionModel::PlayerMotionModel(dtCore::Keyboard* keyboard,
                                    dtCore::Mouse* mouse) 
      : dtCore::FPSMotionModel(keyboard,mouse)
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   PlayerMotionModel::~PlayerMotionModel()
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   void PlayerMotionModel::OnMessage(MessageData *data)
   {
      if(data->message == dtCore::System::MESSAGE_POST_EVENT_TRAVERSAL &&
         GetTarget() != NULL &&
         IsEnabled())
      {
         osg::Vec3 movement, position;
         dtCore::Transform transform;
         // Get the position before the motion model update happens
         GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
         transform.GetTranslation(position);

         // Update the player
         dtCore::FPSMotionModel::OnMessage(data);

         // If physics, force the physics to update the player
         SimCore::Actors::HumanWithPhysicsActor* player = dynamic_cast<SimCore::Actors::HumanWithPhysicsActor*>(GetTarget());
         if(player != NULL)
         {
            // Get the new translation
            GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
            transform.GetTranslation(movement);

            // Set back to the old translation
            transform.SetTranslation(position);
            GetTarget()->SetTransform(transform);
            //CollideWithGround();

            // Physics update on the player
            player->SetMovementTransform(movement);
         }
      }
   }
}
