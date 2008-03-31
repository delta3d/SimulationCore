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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <dtGame/serverloggercomponent.h>
#include <dtGame/logcontroller.h>
#include <dtGame/logkeyframe.h>
#include <SimCore/HLA/HLAConnectionComponent.h>

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
            SimCore::HLA::HLAConnectionComponent* hlaConnectComp;
            gameManager.GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME, hlaConnectComp);

            inputComponent->SetConnectionParameters(hlaConnectComp->GetFedEx(), 
                                                    hlaConnectComp->GetFedFile(), 
                                                    hlaConnectComp->GetFedName(), 
                                                    hlaConnectComp->GetRidFile());
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
