/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <dtGame/serverloggercomponent.h>
#include <dtGame/logcontroller.h>
#include <dtGame/logkeyframe.h>
#include <SimCore/Components/HLAConnectionComponent.h>

namespace StealthGM
{
   ControlsRecordConfigObject::ControlsRecordConfigObject() : 
      mShowAdvancedOptions(false), 
      mStartRecording(false), 
      mStopRecording(false), 
      mAutoKeyFrame(false), 
      mAutoKeyFrameInterval(5), 
      mAddKeyFrame(false), 
      mIsRecording(false), 
      mDisconnect(false), 
      mJoinFederation(false)
   {

   }

   ControlsRecordConfigObject::~ControlsRecordConfigObject()
   {

   }

   void ControlsRecordConfigObject::ApplyChanges(dtGame::GameManager &gameManager)
   {
      if(!IsUpdated())
         return;

      dtGame::GMComponent *component = 
         gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME);

      StealthInputComponent *inputComponent = 
         static_cast<StealthInputComponent*>(component);

      // Shouldn't happen, but better safe than sorry
      if(inputComponent == NULL)
         return;

      if(mStartRecording)
      {
         inputComponent->SetLogFileName(mOutputFilename);
         inputComponent->EnableRecord();
         mStartRecording = false;
      }

      if(mAddKeyFrame)
      {
         std::ostringstream oss;
         oss << "KeyFrame " << gameManager.GetSimulationTime();

         dtGame::LogKeyframe frame;
         frame.SetName(oss.str());
         frame.SetDescription("This key frame (time marker) was generated from \
                               the Stealth Viewer application");
         frame.SetSimTimeStamp(gameManager.GetSimulationTime());
         inputComponent->HandleAddKeyFrame(frame);
         inputComponent->HandleGetKeyFrames();
         mAddKeyFrame = false;
      }

      if(mStopRecording)
      {
         inputComponent->EnableIdle();
         mStopRecording = false;
      }

      if(mDisconnect)
      {
         if(inputComponent->IsConnected())
            inputComponent->LeaveFederation();
         mDisconnect = false;
      }

      if(mJoinFederation)
      {
         if(!inputComponent->IsConnected())
         {
            SimCore::Components::HLAConnectionComponent &hlaConnectComp = 
               static_cast<SimCore::Components::HLAConnectionComponent&>
               (*gameManager.GetComponentByName(SimCore::Components::HLAConnectionComponent::DEFAULT_NAME));

            inputComponent->SetConnectionParameters(hlaConnectComp.GetFedEx(), 
                                                    hlaConnectComp.GetFedFile(), 
                                                    hlaConnectComp.GetFedName(), 
                                                    hlaConnectComp.GetRidFile());
            inputComponent->JoinFederation();
         }
         mJoinFederation = false;
      }

      // Convert from minutes to seconds
      // The UI gives the option in minutes, as this is recommended for performance
      // We don't want some user setting his auto interval to 10 seconds and thinking
      // our app sucks. 
      if(mAutoKeyFrame)
      {
         inputComponent->HandleSetAutoKeyFrameInterval(double(mAutoKeyFrameInterval * 60.0));
      }
      else
      {
         // Disable
         inputComponent->HandleSetAutoKeyFrameInterval(0.0);
      }

      mIsRecording = inputComponent->IsInRecord();

      SetIsUpdated(false);
   }
}
