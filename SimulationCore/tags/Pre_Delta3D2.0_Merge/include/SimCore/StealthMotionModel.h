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
#ifndef STEALTH_MOTION_MODEL_H_
#define STEALTH_MOTION_MODEL_H_

#include <dtCore/flymotionmodel.h>
#include <SimCore/Export.h>

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
   class SIMCORE_EXPORT StealthMotionModel : public dtCore::FlyMotionModel
   {
      public:

         /**
          * Constructor.
          *
          * @param keyboard the keyboard instance, or NULL to
          * avoid creating default input mappings
          * @param mouse the mouse instance, or NULL to avoid
          * creating default input mappings
          * @param useSimTimeForSpeed true if the motion model should use the 
          * simulation time, which can be scaled, for motion or false if it 
          * should use the real time.
          */
         StealthMotionModel(dtCore::Keyboard* keyboard = NULL,
                  dtCore::Mouse* mouse = NULL, 
                  bool useSimTimeForSpeed = true);

         void SetCollideWithGround( bool collideWithGround ) { mCollideWithGround = collideWithGround; }

         bool GetCollideWithGround() const { return mCollideWithGround; }

         void SetGroundClearance( float groundClearance ) { mGroundClearance = groundClearance; }

         float GetGroundClearance() const { return mGroundClearance; }


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

         const osg::Vec3& GetRotation() const { return mRotation; }
         const osg::Vec3& GetPosition() const { return mPosition; }

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
          * The elevation above sea level measured in meters.
          */
         double mElevation;

         /**
          * A reference to the Scene, used for ground following.
          */
         dtCore::RefPtr<dtCore::Scene> mScene;

         /**
          * An ISector reference used in ground collision.
          */
         dtCore::RefPtr<dtCore::Isector> mIsector; 

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