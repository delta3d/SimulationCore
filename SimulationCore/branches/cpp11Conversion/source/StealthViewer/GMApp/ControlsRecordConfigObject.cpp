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
 * @author Curtiss Murphy
 */
#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <dtGame/serverloggercomponent.h>
#include <dtGame/logcontroller.h>
#include <dtGame/logkeyframe.h>
#include <SimCore/HLA/HLAConnectionComponent.h>
#include <dtUtil/datetime.h>
#include <dtCore/system.h>

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
      mJoinNetwork(false),
      mLastRequestTime(5.0)
   {

   }

   ControlsRecordConfigObject::~ControlsRecordConfigObject()
   {

   }

   void ControlsRecordConfigObject::ApplyChanges(dtGame::GameManager& gameManager)
   {
      StealthInputComponent* inputComponent = nullptr;
      gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME, inputComponent);

      SimCore::HLA::HLAConnectionComponent* hlaConnectComp;
      gameManager.GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME, hlaConnectComp);

      // Shouldn't happen, but better safe than sorry
      if(inputComponent == nullptr || hlaConnectComp == nullptr)
         return;

      // if nothing is updated, just check to see if we need to update our key frames.
      if(!IsUpdated())
      {
         // If it's been 5 seconds, then request a keyframe update. The update message will automatically
         // trigger an update of the UI via the slots in MainWindow.
         double currentTime = dtCore::System::GetInstance().GetSimTimeSinceStartup();
         double timeSinceLastUpdate = currentTime - mLastRequestTime;
         if (timeSinceLastUpdate > 5.0 && !inputComponent->IsInPlayback())
         {
            inputComponent->HandleGetKeyFrames();
            mLastRequestTime = currentTime;
         }
      }
      // otherwise, we have updates, so let's do them.
      else
      {
         if(mStartRecording)
         {
            inputComponent->SetLogFileName(mOutputFilename);
            inputComponent->EnableRecord();
            mStartRecording = false;
         }

         if(mAddKeyFrame)
         {
            std::string time = dtUtil::DateTime::ToString(time_t(gameManager.GetSimulationClockTime() / 1000000LL),
               dtUtil::DateTime::TimeFormat::CLOCK_TIME_24_HOUR_FORMAT);
            std::ostringstream oss;
            oss << "KeyFrame " << time;

            dtGame::LogKeyframe frame;
            frame.SetName(oss.str());
            frame.SetDescription("This key frame (time marker) was generated from \
                                  the Stealth Viewer application");
            frame.SetSimTimeStamp(gameManager.GetSimulationTime());
            inputComponent->HandleAddKeyFrame(frame);
            mAddKeyFrame = false;
         }

         if(mStopRecording)
         {
            inputComponent->EnableIdle();
            mStopRecording = false;
         }

         if(mDisconnect)
         {
            hlaConnectComp->Disconnect(false);
            //if(inputComponent->IsConnected())
            //   inputComponent->LeaveFederation();
            mDisconnect = false;
         }

         if(mJoinNetwork)
         {
            hlaConnectComp->DoReconnectToNetwork();
            //if(!inputComponent->IsConnected())
            //{
            //   inputComponent->SetConnectionParameters(hlaConnectComp->GetFedEx(),
            //      hlaConnectComp->GetFedFile(), hlaConnectComp->GetFedName(), hlaConnectComp->GetRidFile());
            //   inputComponent->JoinFederation();
            //}
            mJoinNetwork = false;
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

         // If we stopped a recording, switched to playback, turned on auto markers, added a keyframe,
         // or pretty much anything else, then we should get an updated keyframes
         inputComponent->HandleGetKeyFrames();

         SetIsUpdated(false);
      }
   }
}
