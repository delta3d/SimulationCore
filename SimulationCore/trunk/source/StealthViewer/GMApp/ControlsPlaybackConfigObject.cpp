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
 * @author Eddie Johnson, Curtiss Murphy
 */
#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <SimCore/HLA/HLAConnectionComponent.h>

namespace StealthGM
{
   ControlsPlaybackConfigObject::ControlsPlaybackConfigObject() :
      mShowAdvancedOptions(false), 
      mLoopContinuously(false),
      mBeginPlayback(false),
      mEndPlayback(false),
      mJumpToNextKeyFrame(false),
      mJumpToPreviousKeyFrame(false),
      mRestartPlayback(false), 
      mPlaybackSpeed(1.0f)
   {

   }

   ControlsPlaybackConfigObject::~ControlsPlaybackConfigObject()
   {

   }

   void ControlsPlaybackConfigObject::ApplyChanges(dtGame::GameManager &gameManager)
   {
      if(!IsUpdated())
         return;

      StealthInputComponent *inputComponent;
      gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME, inputComponent);

      // Shouldn't happen, but better safe than sorry
      if(inputComponent == NULL)
         return;

      SimCore::HLA::HLAConnectionComponent *hlaComp;
      gameManager.GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME, hlaComp);

      if(mBeginPlayback)
      {
         inputComponent->SetLogFileName(mInputFilename);
         inputComponent->EnablePlayback();
         mBeginPlayback = false;
      }

      if(mEndPlayback)
      {
         if(hlaComp != NULL)
            inputComponent->SetConnectionParameters(hlaComp->GetFedEx(), 
                                                    hlaComp->GetFedFile(), 
                                                    hlaComp->GetFedName());
         
         inputComponent->EnableIdle();
         mEndPlayback = false;
      }

      if(mJumpToNextKeyFrame)
      {
         inputComponent->HandleGotoKeyFrame(true);
         if(gameManager.IsPaused())
            gameManager.SetPaused(false);
         mJumpToNextKeyFrame = false;
      }

      if(!mKeyFrameName.empty())
      { 
         inputComponent->HandleGotoKeyFrame(mKeyFrameName);
         if(gameManager.IsPaused())
            gameManager.SetPaused(false);
         mKeyFrameName.clear();
      }

      if(mJumpToPreviousKeyFrame)
      {
         inputComponent->HandleGotoKeyFrame(false);
         if(gameManager.IsPaused())
            gameManager.SetPaused(false);
         mJumpToPreviousKeyFrame = false;
      }

      if(mRestartPlayback)
      {
         if(hlaComp != NULL)
            inputComponent->SetConnectionParameters(hlaComp->GetFedEx(), 
            hlaComp->GetFedFile(), 
            hlaComp->GetFedName());

         inputComponent->GotoFirstKeyframe();
         mRestartPlayback = false;
      }

      // Update the LoopContinuously flag.
      if (inputComponent->GetLoopContinuouslyInPlayback() != mLoopContinuously)
         inputComponent->SetLoopContinuouslyInPlayback(mLoopContinuously);

      inputComponent->HandleSpeedChange(mPlaybackSpeed);

      mIsPlayingBack = inputComponent->IsInPlayback();

      SetIsUpdated(false);
   }
}
