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
#include <prefix/SimCorePrefix-src.h>
#include <dtCore/scene.h>
#include <dtCore/keyboard.h>
#include <dtCore/mouse.h>
#include <dtCore/logicalinputdevice.h>
#include <dtCore/inputdevice.h>
#include <dtCore/system.h>
#include <dtCore/transform.h>
#include <dtCore/transformable.h>
#include <dtUtil/mathdefines.h>

#include <SimCore/ClampedMotionModel.h>

namespace SimCore
{
   IMPLEMENT_MANAGEMENT_LAYER(ClampedMotionModel);
   
   //////////////////////////////////////////////////////////////////////////         
   // Passenger Motion Model Code
   //////////////////////////////////////////////////////////////////////////         
   ClampedMotionModel::ClampedMotionModel(dtCore::Keyboard* keyboard,
                                    dtCore::Mouse* mouse) 
      : AttachedMotionModel(keyboard,mouse),
      mFreeLookKey(Producer::Key_Control_L),
      mFreeLookMouseButton(dtCore::Mouse::RightButton),
      mFreeLookByKey(false),
      mFreeLookByMouseButton(false),
      mFreeLookWasHeld(false),
      mFreeLookMouseButtonWasHeld(false),
      mRecenterMouse(false),
      mResetRotation(false),
      mReverseLeftRight(false),
      mReverseUpDown(false),
      mEnabledUpDown(true),
      mEnabledLeftRight(true),
      mLeftRightLimit(-1.0),
      mUpDownLimit(-1.0),
      mKeyboard(keyboard),
      mDOF(NULL),
      mTestMode(false)
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   ClampedMotionModel::~ClampedMotionModel()
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   void ClampedMotionModel::OnMessage(MessageData *data)
   {
      if(data->message == "preframe"
         && (GetTarget() != NULL || GetTargetDOF() != NULL) 
         && IsEnabled() 
         && ( GetMouse()->GetHasFocus() || mTestMode ) )
      {
         if( ! IsFreeLookHeld() )
         {
            if( mResetRotation )
            {
               SetTargetsRotation(mRestingRotation);
            }

            GetLeftRightMouseAxis()->SetState(0.0);
            GetUpDownMouseAxis()->SetState(0.0);
            if( mRecenterMouse )
            {
               GetMouse()->SetPosition(0.0f,0.0f);//keeps cursor at center of screen
            }

            mHPRChange.set( 0.0, 0.0, 0.0 );
         }
         else
         {
            // Reset the mouse position back to the center
            if( mRecenterMouse )
            {
               //GetMouse()->SetPosition(0.0f,0.0f);
            }

            // Do the original rotation
            //get the real time.
            const double deltaFrameTime = static_cast<const double*>(data->userData)[1];

            osg::Vec3 hpr = GetTargetsRotation();
            mHPRChange = hpr;

            bool setHPR = false;

            if( mEnabledLeftRight && GetLeftRightMouseAxis()->GetState() != 0)
            {
               setHPR = true;
               double change = GetLeftRightMouseAxis()->GetState() * GetMaximumMouseTurnSpeed() * deltaFrameTime;

               //std::cout << "Change LR: " << GetLeftRightMouseAxis()->GetState() << std::endl;
               //prevent huge jumps.
               dtUtil::Clamp(change, -15.0, 15.0);
               hpr[0] -= mReverseLeftRight ? -change : change;
               GetLeftRightMouseAxis()->SetState(0.0);//necessary to stop camera drifting down
               //GetMouse()->SetPosition(0.0f,0.0f);//keeps cursor at center of screen
            }

            if( mEnabledUpDown && GetUpDownMouseAxis()->GetState() != 0)
            {
               setHPR = true;
               double change = GetUpDownMouseAxis()->GetState() * GetMaximumMouseTurnSpeed() * deltaFrameTime;

               //prevent huge jumps.
               //std::cout << "Change UD: " << GetUpDownMouseAxis()->GetState() << std::endl;
               dtUtil::Clamp(change, -15.0, 15.0);
               hpr[1] += mReverseUpDown ? -change : change;
               GetUpDownMouseAxis()->SetState(0.0);//necessary to stop camera drifting down
               //GetMouse()->SetPosition(0.0f,0.0f);//keeps cursor at center of screen
            }

            // Clamp rotation
            if( mUpDownLimit >= 0.0 || mLeftRightLimit >= 0.0 )
            {
               // Use a local variable to get around Vec3 Real-Type ambiguity (double vs. float)
               double value;

               // Clamp up-down
               if( mUpDownLimit >= 0.0 )
               {
                  value = hpr[1];
                  dtUtil::Clamp( value, -mUpDownLimit, mUpDownLimit );
                  hpr[1] = value;
               }

               // Clamp left-right
               if( mLeftRightLimit >= 0.0 )
               {
                  value = hpr[0];
                  dtUtil::Clamp( value, -mLeftRightLimit, mLeftRightLimit );
                  hpr[0] = value;
               }
            }

            if(setHPR)
            {
               hpr[2] = 0.0f;
               SetTargetsRotation(hpr);   
               if( ! mTestMode )
               {
                  GetMouse()->SetPosition(0.0f,0.0f);//keeps cursor at center of screen
               }
            }

            // Get the current change in orientation
            mHPRChange = hpr - mHPRChange;
            
            if( mArticHelper.valid() )
            {
               if( mDOF.valid() )
               {
                  mArticHelper->HandleUpdatedDOFOrientation( *mDOF, mHPRChange, hpr );
               }
            }

      }
   
      }
   }

   //////////////////////////////////////////////////////////////////////////         
   void ClampedMotionModel::SetTargetsRotation(const osg::Vec3 &newHpr)
   {
      // the motion model is working directly against the drawable
      if (!mDOF.valid() && GetTarget() != NULL)
      {
         dtCore::Transform transform;
         GetTarget()->GetTransform(transform, dtCore::Transformable::REL_CS);
         transform.SetRotation( newHpr );
         GetTarget()->SetTransform(transform, dtCore::Transformable::REL_CS);
      }
      // the motion model is working against the child dof 
      else if (mDOF.valid())
      {
         mDOF->setCurrentHPR(newHpr * 0.0174533); // convert degrees to radians
      }
   }

   //////////////////////////////////////////////////////////////////////////         
   osg::Vec3 ClampedMotionModel::GetTargetsRotation() const
   {
      // the motion model is working directly against the drawable
      if (!mDOF.valid() && GetTarget() != NULL)
      {
         dtCore::Transform transform;
         osg::Vec3 hpr;
         GetTarget()->GetTransform(transform, dtCore::Transformable::REL_CS);
         transform.GetRotation(hpr);
         return hpr;
      }
      // the motion model is working against the child dof 
      else if (mDOF.valid())
      {
         return mDOF->getCurrentHPR() * 57.29578;
      }
      else
      {
         osg::Vec3 result;
         return result;
      }
   }

   //////////////////////////////////////////////////////////////////////////         
   bool ClampedMotionModel::IsKeyHeld() const
   {
      return mKeyboard->GetKeyState( mFreeLookKey );
   }

   //////////////////////////////////////////////////////////////////////////         
   bool ClampedMotionModel::IsMouseButtonHeld() const
   {
      return GetMouse() != NULL ? GetMouse()->GetButtonState( mFreeLookMouseButton ) : false;
   }

   //////////////////////////////////////////////////////////////////////////         
   bool ClampedMotionModel::IsFreeLookHeld()
   {
      if( ! mFreeLookByKey && ! mFreeLookByMouseButton ) { return true; }

      if( mFreeLookByKey && ! mFreeLookByMouseButton )
      {
         return IsKeyHeld();
      }
      else if( ! mFreeLookByKey && mFreeLookByMouseButton )
      {
         return IsMouseButtonHeld();
      }
      else if( mFreeLookByKey && mFreeLookByMouseButton )
      {
         return IsKeyHeld() || IsMouseButtonHeld();
      }
      return false;
   }
}
