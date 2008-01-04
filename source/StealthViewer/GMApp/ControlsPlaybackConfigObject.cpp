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
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <SimCore/HLA/HLAConnectionComponent.h>

namespace StealthGM
{
   ControlsPlaybackConfigObject::ControlsPlaybackConfigObject() :
      mShowAdvancedOptions(false), 
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

      inputComponent->HandleSpeedChange(mPlaybackSpeed);

      mIsPlayingBack = inputComponent->IsInPlayback();

      SetIsUpdated(false);
   }
}
