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
 * @author David Guthrie
*/

#ifndef ATTACHED_MOTION_MODEL_H
#define ATTACHED_MOTION_MODEL_H

#include <SimCore/Export.h>
#include <dtCore/motionmodel.h>


namespace dtCore
{
   //forward declaration
   class Scene;
   class Keyboard;
   class Mouse;
   class LogicalInputDevice;
   class ButtonAxisToAxis;
   class AxisToAxis;
   class Axis;
   class ButtonsToAxis;
   class LogicalAxis;
}

namespace SimCore
{
   /**
    * @brief A motion model used when someone is riding in a vehicle and can look around, but not move.
    * 
    * The motion can used the mouse or keyboard and uses the a First Person Shooter sort of motion.
    */
   class SIMCORE_EXPORT AttachedMotionModel : public dtCore::MotionModel
   {
      DECLARE_MANAGEMENT_LAYER(AttachedMotionModel)

      public:

         /**
          * Constructor.
          *
          * @param keyboard the keyboard instance, or NULL to avoid creating default input mappings
          * @param mouse the mouse instance, or NULL to avoid creating default input mappings
          */
         AttachedMotionModel(dtCore::Keyboard* keyboard = NULL,
                           dtCore::Mouse* mouse = NULL);

         /**
          * Destructor.
          */
         virtual ~AttachedMotionModel();
            
         /**
          * Enables or disables this motion model.
          *
          * @param enabled true to enable this motion model, false to disable it
          */
         virtual void SetEnabled(bool enabled);
         
         /**
          * Sets the input axes to a set of default mappings for mouse
          * and keyboard.
          *
          * @param keyboard the keyboard instance
          * @param mouse the mouse instance
          */
         void SetDefaultMappings(dtCore::Keyboard* keyboard, dtCore::Mouse* mouse);
                                                              
         /**
          * Sets the maximum turn speed for the mouse (degrees per second).
          *
          * @param newMaximumTurnSpeed the new maximum turn speed
          */
         void SetMaximumMouseTurnSpeed(float newMaximumTurnSpeed) { mMaximumMouseTurnSpeed = newMaximumTurnSpeed; } 
         
         /**
          * Returns the maximum turn speed for the keyboard (degrees per second).
          *
          * @return the current maximum turn speed
          */
         float GetMaximumMouseTurnSpeed() { return mMaximumMouseTurnSpeed; }

         /**
          * Sets the maximum turn speed for the mouse (degrees per second).
          *
          * @param newKeyboardTurnSpeed the new maximum turn speed
          */
         void SetKeyboardTurnSpeed(float newKeyboardTurnSpeed) { mKeyboardTurnSpeed = newKeyboardTurnSpeed; }
         
         /**
          * Returns the maximum turn speed for the keyboard (degrees per second).
          *
          * @return the current maximum turn speed
          */
         float GetKeyboardMouseTurnSpeed() { return mKeyboardTurnSpeed; }

                                 
         /**
          * Message handler callback.
          *
          * @param data the message data
          */
         void OnSystem(const dtUtil::RefString& phase, double deltaSim, double deltaReal);
;

         dtCore::LogicalAxis* GetLeftRightKeyAxis() { return mTurnLeftRightKeyAxis.get(); }
         const dtCore::LogicalAxis* GetLeftRightKeyAxis() const { return mTurnLeftRightKeyAxis.get(); }

         dtCore::LogicalAxis* GetLeftRightMouseAxis() { return mTurnLeftRightMouseAxis.get(); }
         const dtCore::LogicalAxis* GetLeftRightMouseAxis() const { return mTurnLeftRightMouseAxis.get(); }

         dtCore::LogicalAxis* GetUpDownKeyAxis() { return mLookUpDownKeyAxis.get(); }
         const dtCore::LogicalAxis* GetUpDownKeyAxis() const { return mLookUpDownKeyAxis.get(); }

         dtCore::LogicalAxis* GetUpDownMouseAxis() { return mLookUpDownMouseAxis.get(); }
         const dtCore::LogicalAxis* GetUpDownMouseAxis() const { return mLookUpDownMouseAxis.get(); }

         float GetKeyTurnSpeed() const { return mKeyboardTurnSpeed; }

         float GetMouseTurnSpeed() const { return mMaximumMouseTurnSpeed; }

         dtCore::Mouse* GetMouse() { return mMouse; }
         const dtCore::Mouse* GetMouse() const { return mMouse; }
         
         void SetCenterMouse(bool center) { mCenterMouse = center; }
         bool GetCenterMouse() const { return mCenterMouse; }

      private:
         ///A reference to the Scene, used for ground following.
         dtCore::RefPtr<dtCore::Scene> mScene;
         
         /// The default input device.
         dtCore::RefPtr<dtCore::LogicalInputDevice> mDefaultInputDevice;
                  
         /// The left/right mouse movement
         dtCore::RefPtr<dtCore::AxisToAxis> mLeftRightMouseMovement;
         
         /// The up/down mouse movement.
         dtCore::RefPtr<dtCore::AxisToAxis> mUpDownMouseMovement;
         
         /// The arrow key up/down mapping.
         dtCore::RefPtr<dtCore::ButtonsToAxis> mArrowKeysUpDownMapping;
         
         /// The arrow key left/right mapping.
         dtCore::RefPtr<dtCore::ButtonsToAxis> mArrowKeysLeftRightMapping;
                     
         /// The turn left/right axis for the keyboard.
         dtCore::RefPtr<dtCore::LogicalAxis> mTurnLeftRightKeyAxis;

         /// The turn left/right axis for the mouse.
         dtCore::RefPtr<dtCore::LogicalAxis> mTurnLeftRightMouseAxis;
         
         /// The look up/down axis for the keyboard.
         dtCore::RefPtr<dtCore::LogicalAxis> mLookUpDownKeyAxis;
         /// The look up/down axis for the mouse.
         dtCore::RefPtr<dtCore::LogicalAxis> mLookUpDownMouseAxis;
                                 
         /// The maximum turn speed for the mouse (degrees per second).
         float mMaximumMouseTurnSpeed;

         /// The turn speed for the keyboard (degrees per second).
         float mKeyboardTurnSpeed;
         
                dtCore::Mouse* mMouse;

         bool mCenterMouse;
   };
}

#endif
