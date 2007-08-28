/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine 
 * Copyright (C) 2004-2005 Alion Science and Technology 
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

#ifndef PLAYER_MOTION_MODEL_H
#define PLAYER_MOTION_MODEL_H

#include <SimCore/Export.h>
#include <dtCore/fpsmotionmodel.h>
#include <dtCore/refptr.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   struct MessageData;
   class DeltaDrawable;
   class Isector;
   class Keyboard;
   class Mouse;
}

namespace SimCore
{
   namespace Actors
   {
      class Platform;
   }

   namespace Components
   {
   
      //////////////////////////////////////////////////////////////////////////
      // PLAYER MOTION MODEL
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT PlayerMotionModel : public dtCore::FPSMotionModel
      {
         DECLARE_MANAGEMENT_LAYER(PlayerMotionModel)
   
         public:
   
            // Constructor.
            // @param keyboard the keyboard instance, or NULL to avoid creating default input mappings
            // @param mouse the mouse instance, or NULL to avoid creating default input mappings
            PlayerMotionModel(dtCore::Keyboard* keyboard = NULL,
                              dtCore::Mouse* mouse = NULL);
   
            // Destructor
            virtual ~PlayerMotionModel();

            void SetGroundClearance( float groundClearance ) { mGroundClearance = groundClearance; }

            float GetGroundClearance() const { return (float)mGroundClearance; }

            void SetCollidableGeometry( dtCore::DeltaDrawable* geometry );
            const dtCore::DeltaDrawable* GetCollidableGeometry() const;

            const osg::Vec3& GetRotation() const { return mRotation; }
            const osg::Vec3& GetPosition() const { return mPosition; }

            // Gets the recently calculated elevation.
            // @return elevation above sea level measured in meters
            double GetElevation() const { return mElevation; }

            void ResetIsector( const osg::Vec3& camPosition );

            void CollideWithGround();

            // Message handler callback.
            // @param data the message data
            virtual void OnMessage(MessageData *data);
            
            SimCore::Actors::Platform* CheckWithCloseToVehicle();
            //virtual void SetEnabled(bool enabled);

      private:

            // The metric distance the camera should stay away from terrain.
            double mGroundClearance;

            // The elevation above sea level measured in meters.
            double mElevation;

            // An ISector reference used in ground collision.
            dtCore::RefPtr<dtCore::Isector> mIsector; 

            osg::Vec3 mRotation;
            osg::Vec3 mPosition;
      };
   }
}

#endif
