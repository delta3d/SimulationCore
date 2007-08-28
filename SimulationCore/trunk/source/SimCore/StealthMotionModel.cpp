/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
 * @author Chris Rodgers
 */
#include <prefix/dvteprefix-src.h>
#include <dtCore/keyboard.h>
#include <dtCore/mouse.h>
#include <dtCore/transform.h>
#include <dtCore/scene.h>
#include <dtCore/deltadrawable.h>
#include <dtCore/logicalinputdevice.h>
#include <dtCore/isector.h>
#include <osg/Vec3>

#include <SimCore/StealthMotionModel.h>

namespace SimCore
{
   namespace Components
   {
      StealthMotionModel::StealthMotionModel( dtCore::Keyboard* keyboard,
            dtCore::Mouse* mouse, 
            bool useSimTimeForSpeed )
            : dtCore::FlyMotionModel( keyboard, mouse, useSimTimeForSpeed ),
            mCollideWithGround(false), mGroundClearance(1.25f),
            mScene(NULL), mIsector(NULL)
            {
            }

      StealthMotionModel::~StealthMotionModel()
      {
         if( mIsector.valid() )
         {
            mIsector->SetScene(NULL);
         }
         mIsector = NULL;
         mScene = NULL;
      }

      /**
       * Sets the active Scene, which is used for ground following.
       *
       * @param scene the active scene
       */
      void StealthMotionModel::SetScene(dtCore::Scene& scene)
      {
         mScene = &scene;
         if( mIsector.valid() == false )
         {
            mIsector = new dtCore::Isector( mScene.get() );
         }
         else
         {
            mIsector->SetScene( mScene.get() );
         }
      }

      /**
       * Sets the only active collidable geometry, which is used for ground following.
       *
       * @param geometry to which to collide exclusively
       */
      void StealthMotionModel::SetCollidableGeometry(dtCore::DeltaDrawable* geometry)
      {
         if( mIsector.valid() == false )
         {
            mIsector = new dtCore::Isector();
         }
         mIsector->SetGeometry( geometry );
      }


      /**
       * Resets the isector collision ray back to default parameters
       */
      void StealthMotionModel::ResetIsector( const osg::Vec3& camPosition )
      {
         if( ! mIsector.valid() )
         {
            return;
         }

         osg::Vec3 vec( 0.0f, 0.0f, 1.0f );
         float isectorSpan = GetMaximumFlySpeed() * 1.1f;
         if( isectorSpan < 100.0f )
         {
            isectorSpan = 100.0f; 
         }

         mIsector->Reset();
         mIsector->SetStartPosition( camPosition + osg::Vec3(0.0f,0.0f,-isectorSpan) );
         mIsector->SetDirection( vec );
         mIsector->SetLength( isectorSpan * 2.0f );
      }

      /**
       * Returns the active Scene.
       *
       * @return the active Scene
       */
      dtCore::Scene& StealthMotionModel::GetScene()
      {
         return *mScene;
      }

      /**
       * Corrections camera position if colliding with ground.
       */
      void StealthMotionModel::CollideWithGround()
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
            osg::Vec3 hitPt( xyz[0], xyz[1], 0.0f );
            mIsector->GetHitPoint( hitPt, 0 );

            // Account for clearance from the ground
            hitPt[2] += mGroundClearance;

            // Correct camera Z position if
            // camera is inside the terrain.
            if( hitPt[2] >= xyz[2] )
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

      ///////////////////////////////////////////////////////
      void StealthMotionModel::EstimateElevation()
      {
         // Obtain the motion model's position (ie, the camera)
         osg::Vec3 xyz;
         dtCore::Transform transform;
         GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
         transform.GetTranslation(xyz);
         mElevation = xyz[2];
      }


      /////////////////////////////////////////////////////////////////
      void StealthMotionModel::OnMessage(MessageData *data)
      {
         // Handle the regular inherited motion
         FlyMotionModel::OnMessage(data);

         // Collide with ground
         if(mScene.valid() && GetTarget() != NULL && IsEnabled() && 
               (data->message == "preframe" || data->message == "pause"))
         {  
            if (mCollideWithGround)
               CollideWithGround(); 
            else 
               EstimateElevation();
         }
      }

   }
}
