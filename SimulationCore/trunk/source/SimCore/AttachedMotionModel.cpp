// walkmotionmodel.cpp: Implementation of the WalkMotionModel class.
//
//////////////////////////////////////////////////////////////////////
#include <prefix/dvteprefix-src.h>
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
      mMaximumMouseTurnSpeed(1440.0f),
      mKeyboardTurnSpeed(70.0f), 
      mCenterMouse(true)
   {
      RegisterInstance(this);
      
      if(keyboard != 0 && mouse != 0)
      {
         SetDefaultMappings(keyboard, mouse);
      }
      
      AddSender(&dtCore::System::GetInstance());

      mMouse = mouse;
   }
   
   /**
    * Destructor.
    */
   AttachedMotionModel::~AttachedMotionModel()
   {
      RemoveSender(&dtCore::System::GetInstance());
      
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
            keyboard->GetButton(Producer::Key_Left),
            keyboard->GetButton(Producer::Key_Right)
         );
         dtCore::Axis* arrowKeysLeftAndRight = mDefaultInputDevice->AddAxis(
            "arrow keys left/right",
            mArrowKeysLeftRightMapping.get()
         );

         mArrowKeysUpDownMapping = new dtCore::ButtonsToAxis(
            keyboard->GetButton(Producer::Key_Up),
            keyboard->GetButton(Producer::Key_Down)
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
            
   void AttachedMotionModel::OnMessage(MessageData *data)
   {
      if(data->message == "preframe" &&
         GetTarget() != NULL &&
         IsEnabled())
      {
         //get the real time.
         const double deltaFrameTime = static_cast<const double*>(data->userData)[1];
   
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
            float change = float(mTurnLeftRightMouseAxis->GetState() * mMaximumMouseTurnSpeed * deltaFrameTime);
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
            float change = float(mLookUpDownMouseAxis->GetState() * mMaximumMouseTurnSpeed * deltaFrameTime);
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
