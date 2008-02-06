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

#include <prefix/SimCorePrefix-src.h>
#include <dtCore/base.h>
#include <dtCore/deltadrawable.h>
#include <dtCore/isector.h>
#include <dtCore/keyboard.h>
#include <dtCore/logicalinputdevice.h>
#include <dtCore/mouse.h>
#include <dtCore/scene.h>
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
      : dtCore::FPSMotionModel(keyboard,mouse),
      mGroundClearance(1.5),
      mElevation(0.0)
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   PlayerMotionModel::~PlayerMotionModel()
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   void PlayerMotionModel::SetCollidableGeometry(dtCore::DeltaDrawable* geometry)
   {
      if( mIsector.valid() == false )
      {
         mIsector = new dtCore::Isector();
      }
      mIsector->SetGeometry( geometry );
   }

   //////////////////////////////////////////////////////////////////////////         
   const dtCore::DeltaDrawable* PlayerMotionModel::GetCollidableGeometry() const
   {
      return mIsector.valid() ? mIsector->GetQueryRoot() : NULL;
   }

   //////////////////////////////////////////////////////////////////////////         
   void PlayerMotionModel::ResetIsector( const osg::Vec3& camPosition )
   {
      if( ! mIsector.valid() )
      {
         return;
      }

      osg::Vec3 vec( 0.0, 0.0, 1.0 );
      double isectorSpan = 100.0;
      mIsector->Reset();
      mIsector->SetStartPosition( camPosition + osg::Vec3(0.0,0.0,-isectorSpan) );
      mIsector->SetDirection( vec );
      mIsector->SetLength( isectorSpan * 2.0 );
   }

   //////////////////////////////////////////////////////////////////////////         
   void PlayerMotionModel::CollideWithGround()
   {
      // Obtain the camera position
      osg::Vec3 xyz, hpr;
      dtCore::Transform transform;
      GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
      transform.GetTranslation(xyz);

      // Ensure isector ray is in the correct
      // position, orientation and length.
      ResetIsector( xyz );

      // If there was a collision...
      if( mIsector->Update() )
      {
         // Get the collision point
         osg::Vec3 hitPt( xyz[0], xyz[1], 0.0 );
         mIsector->GetHitPoint( hitPt, 0 );

         // Account for clearance from the ground
         hitPt[2] += mGroundClearance;

         // Correct camera Z position if
         // camera is inside the terrain.
//            if( hitPt[2] >= xyz[2] )
         {
            //set our new position/rotation
            xyz[2] = hitPt[2];
            transform.SetTranslation(xyz);
            GetTarget()->SetTransform(transform, dtCore::Transformable::ABS_CS); 
         }
      }

      // Capture the elevation
      mElevation = xyz[2];
      transform.GetRotation(mRotation);
      transform.GetTranslation(mPosition);
   }

   //////////////////////////////////////////////////////////////////////////         
   void PlayerMotionModel::OnMessage(MessageData *data)
   {
      if(data->message == "preframe" &&
         GetTarget() != NULL &&
         IsEnabled())
      {
         osg::Vec3 movement, position;
         dtCore::Transform transform;
         // Get the position before the motion model update happens
         GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
         position = transform.GetTranslation();

         // Update the player
         dtCore::FPSMotionModel::OnMessage(data);

#ifdef AGEIA_PHYSICS
         // If physics, force the physics to update the player
         SimCore::Actors::HumanWithPhysicsActor* player = dynamic_cast<SimCore::Actors::HumanWithPhysicsActor*>(GetTarget());
         if(player != NULL)
         {
            // Get the new translation
            GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
            movement = transform.GetTranslation();

            // Set back to the old translation
            transform.SetTranslation(position);
            GetTarget()->SetTransform(transform);
            //CollideWithGround();

            // Physics update on the player
            player->SetMovementTransform(movement);
         }
#endif
      }
   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::Actors::Platform* PlayerMotionModel::CheckWithCloseToVehicle()
   {
      SimCore::Actors::HumanWithPhysicsActor* player = dynamic_cast<SimCore::Actors::HumanWithPhysicsActor*>(GetTarget());
      if(player != NULL)
      {
         SimCore::Components::PortalComponent* portalComponent; 
         player->GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::PortalComponent::DEFAULT_NAME,
                  portalComponent);
         
         if(portalComponent != NULL)
         {
            if(portalComponent->GetNumberOfPortals() > 0)
            {
               dtCore::Transform transform;
               osg::Vec3 position;
               GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
               position = transform.GetTranslation();
               std::vector<dtGame::GameActorProxy*> toFillIn;
               portalComponent->FindPortalsInRange(position, 3.0f, toFillIn);
               if(toFillIn.size())
                  return dynamic_cast<SimCore::Actors::Platform*>(toFillIn.front()->GetActor());
            }
         }
      }
      return NULL;
   }
}
