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
#ifndef STEALTH_MOTION_MODEL_H_
#define STEALTH_MOTION_MODEL_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtCore/flymotionmodel.h>
#include <SimCore/Export.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class Keyboard;
   class Mouse;
   class Scene;
   class DeltaDrawable;
   class Isector;
}



namespace SimCore
{
   /////////////////////////////////////////////////////////////////////////////
   // STEALTH MOTION MODEL CODE
   /////////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT StealthMotionModel : public dtCore::FlyMotionModel
   {
      public:
         static const float DEFAULT_GROUND_CLEARANCE; // Meters off surface
         static const float DEFAULT_SPEED_LIMIT_MIN; // Meters per second
         static const float DEFAULT_SPEED_LIMIT_MAX; // Meters per second

         /**
          * Constructor.
          *
          * @param keyboard the keyboard instance, or nullptr to
          * avoid creating default input mappings
          * @param mouse the mouse instance, or nullptr to avoid
          * creating default input mappings
          * @param useSimTimeForSpeed true if the motion model should use the 
          * simulation time, which can be scaled, for motion or false if it 
          * should use the real time.
          */
         StealthMotionModel(dtCore::Keyboard* keyboard = nullptr,
                  dtCore::Mouse* mouse = nullptr, 
                  dtCore::FlyMotionModel::BehaviorOptions options = dtCore::FlyMotionModel::OPTION_DEFAULT);

         void SetCollideWithGround( bool collideWithGround ) { mCollideWithGround = collideWithGround; }

         bool GetCollideWithGround() const { return mCollideWithGround; }

         void SetGroundClearance( float groundClearance ) { mGroundClearance = groundClearance; }

         float GetGroundClearance() const { return mGroundClearance; }

         //an internal offset is used with the ground clamping, this offset must be cleared when warping or attaching
         void ResetOffset();

         /**
          * Gets the recently calculated elevation.
          *
          * @return elevation above sea level measured in meters
          */
         double GetElevation() const { return mElevation; }

         /**
          * Sets the active Scene, which is used for ground following.
          *
          * @param scene the active scene
          */
         void SetScene(dtCore::Scene& scene);

         /**
          * Returns the active Scene.
          *
          * @return the active Scene
          */
         dtCore::Scene& GetScene();
         const dtCore::Scene& GetScene() const;

         /**
          * Sets the only active collidable geometry, which is used for ground following.
          *
          * @param geometry to which to collide exclusively
          */
         void SetCollidableGeometry(dtCore::DeltaDrawable* geometry);

         /**
          * Corrections camera position if colliding with ground.
          */
         void CollideWithGround();

         /**
          * The collide with ground also calculates the elevation.  This method
          * is called when we don't collide with the ground, to put the elevation as 
          * the camera's height. 
          */
         void EstimateElevation();

         /**
          * Message handler callback.
          *
          * @param data the message data
          */
         virtual void OnMessage(MessageData *data);

         /**
          * @return HPR orientation set on the last call to OnMessage.
          */
         const osg::Vec3& GetRotation() const { return mRotation; }

         /**
          * @return World position set on the last call to OnMessage.
          */
         const osg::Vec3& GetPosition() const { return mPosition; }

         /**
          * Set the slowest speed that the motion model should move.
          * @param speedLimit Meters per second
          */
         void SetFlySpeedLimitMin( float speedLimit );
         float GetFlySpeedLimitMin() const;

         /**
          * Set the fastest speed that the motion model should move.
          * @param speedLimit Meters per second
          */
         void SetFlySpeedLimitMax( float speedLimit );
         float GetFlySpeedLimitMax() const;

      protected:

         virtual ~StealthMotionModel();

      private:

         /**
          * Used to determine if the camera should collide with terrain.
          */
         bool mCollideWithGround;

         /**
          * The metric distance the camera should stay away from terrain.
          */
         float mGroundClearance;

         /**
          * Slowest speed the motion model should move (meters per second).
          */
         float mSpeedLimitMin;

         /**
          * Fastest speed the motion model should move (meters per second).
          */
         float mSpeedLimitMax;

         /**
          * The elevation above sea level measured in meters.
          */
         double mElevation;

         /**
         * The total distance in the Z direction that the object was moved during ground clamping.
         */
         float mZOffset;

         /**
          * A reference to the Scene, used for ground following.
          */
         std::shared_ptr<dtCore::Scene> mScene;

         /**
          * An ISector reference used in ground collision.
          */
         std::shared_ptr<dtCore::Isector> mIsector; 

         /**
          * Resets the isector collision ray back to default parameters
          *
          * @param position from which the isector should extend
          */
         void ResetIsector( const osg::Vec3& camPosition );

         osg::Vec3 mRotation;
         osg::Vec3 mPosition;

   };      
}

#endif
