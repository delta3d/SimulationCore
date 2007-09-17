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
#include <SimCore/Components/HLAConnectionComponent.h>

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

      dtGame::GMComponent *component = 
         gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME);

      StealthInputComponent *inputComponent = 
         static_cast<StealthInputComponent*>(component);

      // Shouldn't happen, but better safe than sorry
      if(inputComponent == NULL)
         return;

      component = gameManager.GetComponentByName(SimCore::Components::HLAConnectionComponent::DEFAULT_NAME);
      SimCore::Components::HLAConnectionComponent *hlaComp = 
         static_cast<SimCore::Components::HLAConnectionComponent*>(component);

      if(mBeginPlayback)
      {
         inputComponent->SetLogFileName(GetInputFilename());
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
         mJumpToNextKeyFrame = false;
      }

      if(!mKeyFrameName.empty())
      {
         inputComponent->HandleGotoKeyFrame(mKeyFrameName);
         mKeyFrameName.clear();
      }

      if(mJumpToPreviousKeyFrame)
      {
         inputComponent->HandleGotoKeyFrame(false);
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
