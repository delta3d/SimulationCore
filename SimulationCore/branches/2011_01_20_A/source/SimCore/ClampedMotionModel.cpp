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
#include <prefix/SimCorePrefix.h>
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
      : AttachedMotionModel(keyboard,mouse)
      , mFreeLookKey(osgGA::GUIEventAdapter::KEY_Control_L)
      , mFreeLookMouseButton(dtCore::Mouse::RightButton)
      , mFreeLookByKey(false)
      , mFreeLookByMouseButton(false)
      , mFreeLookWasHeld(false)
      , mFreeLookMouseButtonWasHeld(false)
      , mRecenterMouse(false)
      , mResetRotation(false)
      , mReverseLeftRight(false)
      , mReverseUpDown(false)
      , mEnabledUpDown(true)
      , mEnabledLeftRight(true)
      , mLeftRightLimit(-1.0)
      , mDownLimit(-1.0)
      , mUpLimit(-1.0)
      , mKeyboard(keyboard)
      , mDOF(NULL)
      , mTestMode(false)
      , mTestTimeSincePrint(0.0f)
      , mTestNumberOfZeros(0)
      , mTestNumberOfCalls(0)
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   ClampedMotionModel::~ClampedMotionModel()
   {
   }

   //////////////////////////////////////////////////////////////////////////         
   void ClampedMotionModel::OnMessage(MessageData *data)
   {
      if(data->message == dtCore::System::MESSAGE_POST_EVENT_TRAVERSAL
         && (GetTarget() != NULL || GetTargetDOF() != NULL) 
         && IsEnabled() 
         && ( GetMouse()->GetHasFocus() || mTestMode ) )
      {
         // Curt hack
         float xCurPos,yCurPos;
         GetMouse()->GetPosition(xCurPos, yCurPos); 
         //if (xCurPos != 0.0f || yCurPos != 0.0f)
           // std::cout << "    -- Non zero POS x[" << xCurPos << "], y[" << yCurPos << "]." << std::endl;
         //else 
            //std::cout << "    -- ZERO ZERO ZERO cur pos." << std::endl;

         // Curt Hack - Mouse update test code         
         //const double myTime = static_cast<const double*>(data->userData)[1];
         //mTestNumberOfCalls ++;
         //mTestTimeSincePrint += myTime;
         //if (std::abs(GetUpDownMouseAxis()->GetState()) < 0.0001 && 
         //      std::abs(GetLeftRightMouseAxis()->GetState()) < 0.0001)
         //   mTestNumberOfZeros++; 
         //if (mTestTimeSincePrint > 1.0f)
         //{
         //   std::cout << "ClampMotionStats [" << mTestTimeSincePrint << "s] Calls[" << mTestNumberOfCalls << 
         //      "] NonZeros[" << (mTestNumberOfCalls - mTestNumberOfZeros) << "] FPS [" << 
         //      (int) (mTestNumberOfCalls/mTestTimeSincePrint) << "] UpdatesPerSec[" << 
         //      (float) ((mTestNumberOfCalls-mTestNumberOfZeros)/mTestTimeSincePrint) << "]." << std::endl;
         //   mTestNumberOfZeros = 0;
         //   mTestNumberOfCalls = 0;
         //   mTestTimeSincePrint = 0.0f;
         //}
         //// End Curt Mouse Test


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
               GetMouse()->SetPosition(0.0f,0.0f);
            }

            // Do the original rotation
            //get the real time.
            //const double deltaFrameTime = static_cast<const double*>(data->userData)[1];

            osg::Vec3 hpr = GetTargetsRotation();
            mHPRChange = hpr;

            bool setHPR = false;

            if( mEnabledLeftRight && GetLeftRightMouseAxis()->GetState() != 0)
            {
               setHPR = true;
               double change = GetLeftRightMouseAxis()->GetState() * GetMaximumMouseTurnSpeed();// * deltaFrameTime;

               //prevent huge jumps.
               dtUtil::Clamp(change, -15.0, 15.0);
               hpr[0] -= mReverseLeftRight ? -change : change;
               GetLeftRightMouseAxis()->SetState(0.0);//necessary to stop camera drifting down
            }

            if( mEnabledUpDown && GetUpDownMouseAxis()->GetState() != 0)
            {
               setHPR = true;
               double change = GetUpDownMouseAxis()->GetState() * GetMaximumMouseTurnSpeed();// * deltaFrameTime;

               //prevent huge jumps.
               dtUtil::Clamp(change, -15.0, 15.0);
               hpr[1] += mReverseUpDown ? -change : change;
               GetUpDownMouseAxis()->SetState(0.0);//necessary to stop camera drifting down
            }

            // Clamp rotation
            if( mUpLimit >= 0.0 || mDownLimit >= 0.0 || mLeftRightLimit >= 0.0 )
            {
               // Use a local variable to get around Vec3 Real-Type ambiguity (double vs. float)
               double value;

               // Clamp up-down - independently.
               if( mUpLimit >= 0.0 || mDownLimit >= 0.0)
               {
                  value = hpr[1];
                  if (mUpLimit >= 0.0)
                     dtUtil::Clamp( value, -89.0, mUpLimit );
                  if (mDownLimit >= 0.0)
                     dtUtil::Clamp( value, -mDownLimit, 89.0 );
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
               if (!mTestMode) // Don't move the mouse in unit tests...
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
