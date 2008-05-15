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
* circumstances in which the U. S. Government may have rights in the software.
 * @author Chris Rodgers
 */

/////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
/////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
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
   /////////////////////////////////////////////////////////////////////////////
   // STEALTH MOTION MODEL CODE
   /////////////////////////////////////////////////////////////////////////////
   const float StealthMotionModel::DEFAULT_GROUND_CLEARANCE = 1.25f;
   const float StealthMotionModel::DEFAULT_SPEED_LIMIT_MIN = 0.5f;
   const float StealthMotionModel::DEFAULT_SPEED_LIMIT_MAX = 6000.0f;

   /////////////////////////////////////////////////////////////////////////////
   StealthMotionModel::StealthMotionModel(
      dtCore::Keyboard* keyboard, dtCore::Mouse* mouse, bool useSimTimeForSpeed )
      : dtCore::FlyMotionModel( keyboard, mouse, useSimTimeForSpeed )
      , mCollideWithGround(false)
      , mGroundClearance(DEFAULT_GROUND_CLEARANCE)
      , mSpeedLimitMin(DEFAULT_SPEED_LIMIT_MIN) // meters per second
      , mSpeedLimitMax(DEFAULT_SPEED_LIMIT_MAX) // meters per second
      , mScene(NULL)
      , mIsector(NULL)
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   StealthMotionModel::~StealthMotionModel()
   {
      if( mIsector.valid() )
      {
         mIsector->SetScene(NULL);
      }
      mIsector = NULL;
      mScene = NULL;
   }

   /////////////////////////////////////////////////////////////////////////////
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

   /////////////////////////////////////////////////////////////////////////////
   void StealthMotionModel::SetCollidableGeometry(dtCore::DeltaDrawable* geometry)
   {
      if( mIsector.valid() == false )
      {
         mIsector = new dtCore::Isector();
      }
      mIsector->SetGeometry( geometry );
   }

   /////////////////////////////////////////////////////////////////////////////
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

   /////////////////////////////////////////////////////////////////////////////
   dtCore::Scene& StealthMotionModel::GetScene()
   {
      return *mScene;
   }

   /////////////////////////////////////////////////////////////////////////////
   const dtCore::Scene& StealthMotionModel::GetScene() const
   {
      return *mScene;
   }

   /////////////////////////////////////////////////////////////////////////////
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

   /////////////////////////////////////////////////////////////////////////////
   void StealthMotionModel::EstimateElevation()
   {
      // Obtain the motion model's position (ie, the camera)
      osg::Vec3 xyz;
      dtCore::Transform transform;
      GetTarget()->GetTransform(transform, dtCore::Transformable::ABS_CS);
      transform.GetTranslation(xyz);
      mElevation = xyz[2];
   }

   /////////////////////////////////////////////////////////////////////////////
   void StealthMotionModel::OnMessage(MessageData *data)
   {
      // Get the max speed of the motion model.
      // Ensure that the motion model does not travel any faster
      // than the imposed minimum and maximum speed limits.
      //
      // This is is performed here because the Input Component
      // of the application may have set the speed past the limits
      // and may have not checked the limits.
      //
      // Minimum speed check.
      if( GetMaximumFlySpeed() < GetFlySpeedLimitMin() )
      {
         // Enforce minimum speed.
         SetMaximumFlySpeed( GetFlySpeedLimitMin() );
      }
      // Maximum speed check.
      if( GetMaximumFlySpeed() > GetFlySpeedLimitMax() )
      {
         // Enforce maximum speed.
         SetMaximumFlySpeed( GetFlySpeedLimitMax() );
      }

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

   /////////////////////////////////////////////////////////////////////////////
   void StealthMotionModel::SetFlySpeedLimitMin( float speedLimit )
   {
      mSpeedLimitMin = speedLimit;
   }

   /////////////////////////////////////////////////////////////////////////////
   float StealthMotionModel::GetFlySpeedLimitMin() const
   {
      return mSpeedLimitMin;
   }

   /////////////////////////////////////////////////////////////////////////////
   void StealthMotionModel::SetFlySpeedLimitMax( float speedLimit )
   {
      mSpeedLimitMax = speedLimit;
   }

   /////////////////////////////////////////////////////////////////////////////
   float StealthMotionModel::GetFlySpeedLimitMax() const
   {
      return mSpeedLimitMax;
   }

}
