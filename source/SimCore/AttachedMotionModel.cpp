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

#include <SimCore/AttachedMotionModel.h>

namespace SimCore
{
   
   IMPLEMENT_MANAGEMENT_LAYER(AttachedMotionModel);
   
   /**
    * Constructor.
    *
    * @param keyboard the keyboard instance, or 0 to
    * avoid creating default input mappings
    * @param mouse the mouse instance, or 0 to avoid
    * creating default input mappings
    */
   AttachedMotionModel::AttachedMotionModel(dtCore::Keyboard* keyboard,
                                    dtCore::Mouse* mouse) : MotionModel("AttachedMotionModel"),
      mMaximumMouseTurnSpeed(90.0f), // 1440.0f - old value when we used time
      mKeyboardTurnSpeed(70.0f), 
      mCenterMouse(true)
   {
      RegisterInstance(this);
      
      if(keyboard != 0 && mouse != 0)
      {
         SetDefaultMappings(keyboard, mouse);
      }
      
      dtCore::System::GetInstance().TickSignal.connect_slot(this, &AttachedMotionModel::OnSystem);

      mMouse = mouse;
   }
   
   /**
    * Destructor.
    */
   AttachedMotionModel::~AttachedMotionModel()
   {
      DeregisterInstance(this);
   }
         
   void AttachedMotionModel::SetEnabled(bool enabled)
   {
      if(enabled)
      {
         if(mCenterMouse)
            mMouse->SetPosition(0.0f,0.0f);
      }
   
      dtCore::MotionModel::SetEnabled(enabled);
   }
   
   void AttachedMotionModel::SetDefaultMappings(dtCore::Keyboard* keyboard, dtCore::Mouse* mouse)
   {
      if(!mDefaultInputDevice.valid())
      {
         mDefaultInputDevice = new dtCore::LogicalInputDevice;
         
         mLeftRightMouseMovement = new dtCore::AxisToAxis(mouse->GetAxis(0));
        dtCore::Axis* leftRightMouseMovement = mDefaultInputDevice->AddAxis(
            "left/right mouse movement",
            mLeftRightMouseMovement.get()
            );
       
         mUpDownMouseMovement = new dtCore::AxisToAxis(mouse->GetAxis(1));
        dtCore::Axis* upDownMouseMovement = mDefaultInputDevice->AddAxis(
            "up/down mouse movement",
            mUpDownMouseMovement.get()
            );

         mArrowKeysLeftRightMapping = new dtCore::ButtonsToAxis(
            keyboard->GetButton(osgGA::GUIEventAdapter::KEY_Left),
            keyboard->GetButton(osgGA::GUIEventAdapter::KEY_Right)
         );
         dtCore::Axis* arrowKeysLeftAndRight = mDefaultInputDevice->AddAxis(
            "arrow keys left/right",
            mArrowKeysLeftRightMapping.get()
         );

         mArrowKeysUpDownMapping = new dtCore::ButtonsToAxis(
            keyboard->GetButton(osgGA::GUIEventAdapter::KEY_Up),
            keyboard->GetButton(osgGA::GUIEventAdapter::KEY_Down)
         );
         
         dtCore::Axis* arrowKeysUpAndDown = mDefaultInputDevice->AddAxis(
            "arrow keys left/right",
            mArrowKeysUpDownMapping.get()
         );
         
         mTurnLeftRightMouseAxis = mDefaultInputDevice->AddAxis(
            "default turn left/right",
            new dtCore::AxesToAxis(leftRightMouseMovement)
         );

         mTurnLeftRightKeyAxis = mDefaultInputDevice->AddAxis(
            "default turn left/right",
            new dtCore::AxisToAxis(arrowKeysLeftAndRight)
         );
   
        mLookUpDownMouseAxis = mDefaultInputDevice->AddAxis(
            "default look up/down",
            new dtCore::AxisToAxis(upDownMouseMovement)
         );

         mLookUpDownKeyAxis = mDefaultInputDevice->AddAxis(
            "default look up/down",
            new dtCore::AxisToAxis(arrowKeysUpAndDown)
         );
            
      }
      
   }
            
   void AttachedMotionModel::OnSystem(const dtUtil::RefString& phase, double deltaSim, double deltaReal)
   {
      if (phase == dtCore::System::MESSAGE_POST_EVENT_TRAVERSAL &&
         GetTarget() != NULL &&
         IsEnabled())
      {
         //get the real time. 
         const double deltaFrameTime = deltaReal;
   
         dtCore::Transform transform;
   
         GetTarget()->GetTransform(transform, dtCore::Transformable::REL_CS);
   
         osg::Vec3 xyz, hpr;
         
         transform.GetRotation(hpr);
   
         if (mTurnLeftRightKeyAxis->GetState() != 0)
         {
            hpr[0] -= float(mTurnLeftRightKeyAxis->GetState() * mKeyboardTurnSpeed * deltaFrameTime);
         }
         else if (mTurnLeftRightMouseAxis->GetState() != 0)
         {
            float change = float(mTurnLeftRightMouseAxis->GetState() * mMaximumMouseTurnSpeed);// * deltaFrameTime);
            //prevent huge jumps.
            dtUtil::Clamp(change, -15.0f, 15.0f);
            hpr[0] -= change;
            mTurnLeftRightMouseAxis->SetState(0.0f);//necessary to stop camera drifting down
            if(mCenterMouse)
               mMouse->SetPosition(0.0f,0.0f); //keeps cursor at center of screen
         }

         if (mLookUpDownKeyAxis->GetState() != 0)
         {
            hpr[1] += float(mLookUpDownKeyAxis->GetState() * mKeyboardTurnSpeed * deltaFrameTime);
         } 
         else if (mLookUpDownMouseAxis->GetState() != 0)
         {
            float change = float(mLookUpDownMouseAxis->GetState() * mMaximumMouseTurnSpeed);// * deltaFrameTime);
            //prevent huge jumps.
            dtUtil::Clamp(change, -15.0f, 15.0f);
            hpr[1] += change;
            mLookUpDownMouseAxis->SetState(0.0f);//necessary to stop camera drifting down
            if(mCenterMouse)
               mMouse->SetPosition(0.0f,0.0f);//keeps cursor at center of screen
         }
   
         //hpr[1] = 0.0f; //set to 0 to stop camera translating above terrain
   
         hpr[2] = 0.0f;
         transform.SetRotation(hpr);
               
         GetTarget()->SetTransform(transform, dtCore::Transformable::REL_CS); 
      }
   }
}
