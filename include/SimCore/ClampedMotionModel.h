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

#ifndef CLAMPED_MOTION_MODEL_H
#define CLAMPED_MOTION_MODEL_H

#include <SimCore/Export.h>
#include <SimCore/AttachedMotionModel.h>
#include <SimCore/Components/ArticulationHelper.h>

#include <dtCore/observerptr.h>
#include <dtCore/mouse.h>

#include <osgSim/DOFTransform>

namespace dtCore
{
   //forward declaration
   class Scene;
   class Keyboard;
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
   class SIMCORE_EXPORT ClampedMotionModel : public AttachedMotionModel
   {
      DECLARE_MANAGEMENT_LAYER(ClampedMotionModel)

      public:

         /**
          * Constructor.
          *
          * @param keyboard the keyboard instance, or NULL to avoid creating default input mappings
          * @param mouse the mouse instance, or NULL to avoid creating default input mappings
          */
         ClampedMotionModel(dtCore::Keyboard* keyboard = NULL,
                           dtCore::Mouse* mouse = NULL);

         /**
          * Destructor.
          */
         virtual ~ClampedMotionModel();
                                 
         /**
          * Message handler callback.
          *
          * @param data the message data
          */
         virtual void OnMessage(MessageData *data);

         /**
          * Set the max degrees this motion model can turn in relation to
          * the parent transform. Left/Right limits are tied together currently.
          * Setting the limit overrides the use of SetEnabledLeftRight().
          *
          * @param leftRightLimit The max degrees the motion model can turn left and right
          * from the attach-to object's heading.
          * Values <= 0.0 mean no movement allowed.
          */
         void SetLeftRightLimit( double leftRightLimit ) 
         { 
            mLeftRightLimit = leftRightLimit; 
            SetLeftRightEnabled(leftRightLimit > 0.0);
         }
         double GetLeftRightLimit() const { return mLeftRightLimit; }

         /**
          * Set the max degrees this motion model can turn in relation to
          * the parent transform. 
          * Setting the limit overrides the use of SetEnabledUpDown().
          *
          * @param upDownLimit The max degrees the motion model can turn up and down (matching)
          * from the attach-to object's heading.
          * Values <= 0.0 mean no movement. Pass in a positive.
          */
         void SetUpDownLimit( double upDownLimit ) 
         { 
            SetUpDownLimit(upDownLimit, upDownLimit);
         }

         // Note - GetUpDownLimit makes no sense if they are not the same.
         // Setting the limit overrides the use of SetEnabledUpDown()
         double GetUpDownLimit() const { return mUpLimit; }
         void SetUpDownLimit( double upLimit, double downLimit) {
            mUpLimit = upLimit; 
            mDownLimit = downLimit;
            SetUpDownEnabled(upLimit > 0.0 || downLimit > 0.0);
         }

         /**
          * Determines if this motion model needs a keyboard key held to
          * change its orientation
          *
          * @param useKey 
          */
         void SetFreeLookByKey( bool useKey ) { mFreeLookByKey = useKey; }
         bool GetFreeLookByKey() const { return mFreeLookByKey; }

         /**
          * Determines if this motion model needs a mouse button held to
          * change the orientation
          *
          * @param useButton
          */
         void SetFreeLookByMouseButton( bool useButton ) { mFreeLookByMouseButton = useButton; }
         bool GetFreeLookByMouseButton() const { return mFreeLookByMouseButton; }

         /**
          * Determines if this motion model should recenter the mouse
          * pointer back to the center of the application window's origin.
          */
         void SetRecenterMouse( bool recenter ) { mRecenterMouse = recenter; }
         bool GetRecenterMouse() const { return mRecenterMouse; }


         /**
          * Determines if this motion model should reset the rotation of
          * the target when the free-look key is released.
          */
         void SetResetRotation( bool reset ) { mResetRotation = reset; }
         bool GetResetRotation() const { return mResetRotation; }

         /**
          * Reverses left/right rotation of the motion model
          */
         void SetReverseLeftRight( bool reverse ) { mReverseLeftRight = reverse; }
         bool IsReverseLeftRight() const { return mReverseLeftRight; }
         
         /**
          * Reverses up/down rotation of the motion model
          */
         void SetReverseUpDown( bool reverse ) { mReverseUpDown = reverse; }
         bool IsReverseUpDown() const { return mReverseUpDown; }

         /**
          * Allows vertical rotation
          */
         void SetUpDownEnabled( bool enabled ) { mEnabledUpDown = enabled; }
         bool IsUpDownEnabled() const { return mEnabledUpDown; }

         /**
          * Allows horizontal rotation
          */
         void SetLeftRightEnabled( bool enabled ) { mEnabledLeftRight = enabled; }
         bool IsLeftRightEnabled() const { return mEnabledLeftRight; }

         /**
          * Set the default rest rotation (orientation) in degrees of 
          * heading-pitch-roll to which the motion model should re-orient 
          * the target object, if ResetRotation has been set to TRUE.
          */
         void SetRestingRotation( const osg::Vec3& hpr ) { mRestingRotation = hpr; }
         const osg::Vec3& GetRestingRotation() const { return mRestingRotation; }
         
         /**
          * Determines if a keyboard key must be held to cause this motion model to rotate
          */
         void SetFreeLookKey( int key ) { mFreeLookKey = key; }
         int GetFreeLookKey() const { return mFreeLookKey; }

         /**
          * Determines if a mouse button must be held to cause this motion model to rotate
          */
         void SetFreeLookMouseButton( dtCore::Mouse::MouseButton button ) { mFreeLookMouseButton = button; }
         dtCore::Mouse::MouseButton GetFreeLookMouseButton() const { return mFreeLookMouseButton; }

         /**
          * Determines if the modifier key is being held.
          * This method is can be overridden to change the key 
          * or program application that controls free-look.
          * The default is the Left Control key.
          */
         virtual bool IsKeyHeld() const;
         virtual bool IsMouseButtonHeld() const;

         /**
          * Determines if either a key or mouse button is held to cause this
          * motion model to rotate.
          * NOTE: returns TRUE if nothing is required to be held
          */
         virtual bool IsFreeLookHeld();

         /**
          * @return the difference in Heading/Pitch/Roll from last orientation
          *         to the current orientation.
          */
         const osg::Vec3& GetHPRChange() const { return mHPRChange; }

         void SetArticulationHelper( SimCore::Components::ArticulationHelper* articHelper ) { mArticHelper = articHelper; }
         SimCore::Components::ArticulationHelper* GetArticulationHelper() { return mArticHelper.get(); }
         
         
         // Setting the DOF will cause this motion model to work directly
         // against a sub node of the target Drawable.  This would allow 
         // you to have 2 motion models against the same actor if for instance
         // it has a turret and a ring mount. 
         void SetTargetDOF( osgSim::DOFTransform* transform ) { mDOF = transform; }
         osgSim::DOFTransform* GetTargetDOF() { return mDOF.get(); }
         const osgSim::DOFTransform* GetTargetDOF() const { return mDOF.get(); }

         // Determines whether we working with the DOF or just the node and gets the rotation
         void SetTargetsRotation(const osg::Vec3 &newHpr);
         osg::Vec3 GetTargetsRotation() const;

         // This function should only be used for unit tests.
         // The test flag allows the unit tests to run without requiring
         // the mouse to have focus on the window.
         void SetTestMode( bool testMode ) { mTestMode = testMode; }
         bool GetTestMode() const { return mTestMode; }
        
      private:

         // The key that needs to be held to enable free-look
         int mFreeLookKey;
         dtCore::Mouse::MouseButton mFreeLookMouseButton;

         // Determines if a modifier keyboard key needs to be held to
         // change the motion model's orientation.
         bool mFreeLookByKey;

         // Determines if a modifier mouse button needs to be held to
         // change the motion model's orientation.
         bool mFreeLookByMouseButton;

         // Determines if the mouse needs to be re-centered if
         // free-look has started from a non-free-look state.
//         bool mFreeLookWasHeld;

         // Determines if the mouse needs to be re-centered if
         // free-look has started from a non-free-look state.
//        bool mFreeLookMouseButtonWasHeld;

         // Controls whether the mouse pointer is reset to the 
         // center of the application window or not.
         bool mRecenterMouse;

         // Controls whether the rotation should pop back to a
         // rest rotation.
         bool mResetRotation;

         // Reverses left/right rotation 
         bool mReverseLeftRight;

         // Reverses up/down rotation
         bool mReverseUpDown;

         // Allow vertical rotation
         bool mEnabledUpDown;

         // Allow horizontal rotation
         bool mEnabledLeftRight;

         // Max degrees the motion model can turn left and right
         // from the attach-to object's heading.
         // Values less than 0.0 mean no limit.
         double mLeftRightLimit;

         // Max degrees the motion model can turn up and down
         // from the attach-to object's heading
         // Values less than 0.0 mean no limit.
         //double mUpDownLimit;
         double mDownLimit;
         double mUpLimit;

         // The default rest rotation (orientation) in degrees of heading-pitch-roll
         // to which the motion model should re-orient the target object, if
         // ResetRotation has been set to TRUE.
         osg::Vec3 mRestingRotation;

         // The last known change in Heading/Pitch/Roll
         osg::Vec3 mHPRChange;

         dtCore::Keyboard* mKeyboard;

         // The articulation helper 
         dtCore::RefPtr<SimCore::Components::ArticulationHelper> mArticHelper;

         dtCore::ObserverPtr<osgSim::DOFTransform> mDOF;

         bool mTestMode;
//         float mTestTimeSincePrint;
//         int mTestNumberOfZeros;
//         int mTestNumberOfCalls;
   };
}
 
#endif
