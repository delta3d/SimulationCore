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
#ifndef _CONTROLS_PLAYBACK_CONFIG_OBJECT_H_
#define _CONTROLS_PLAYBACK_CONFIG_OBJECT_H_

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

#include <string>

namespace StealthGM
{
   class STEALTH_GAME_EXPORT ControlsPlaybackConfigObject : public ConfigurationObjectInterface
   {
      public:

         /// Constructor
         ControlsPlaybackConfigObject();

         /**
          * Applys changes on the game manager
          */
         virtual void ApplyChanges(dtGame::GameManager &gameManager);

         /**
          * Sets the intput file name
          * @param filename The new filename
          */
         void SetInputFilename(const std::string &filename) { mInputFilename = filename; SetIsUpdated(true); }
      
         /**
          * Returns the current input file name
          * @return mInputFilename
          */
         const std::string& GetInputFilename() const { return mInputFilename; }

         /**
          * Sets showing the advanced options
          * @param enable True to enable, false to disable
          */
         void SetShowAdvancedOptions(bool enable) { mShowAdvancedOptions = enable; SetIsUpdated(true); }

         /**
          * Returns true if advanced options are showing
          * @return mShowAdvancedOptions
          */
         bool GetShowAdvancedOptions() const { return mShowAdvancedOptions; }

         /**
          * Begins a playback
          */
         void BeginPlayback() { mBeginPlayback = true; SetIsUpdated(true); }

         /**
          * Stops a playback
          */
         void EndPlayback() { mEndPlayback = true; SetIsUpdated(true); }

         /**
          * Jumps to the next key frame
          */
         void JumpToNextKeyFrame() { mJumpToNextKeyFrame = true; SetIsUpdated(true); }

         /**
          * Jumps to the previous key frame
          */
         void JumpToPreviousKeyFrame() { mJumpToPreviousKeyFrame = true; SetIsUpdated(true); }

         /**
          * Jumps to a key frame by name
          * @param kfName The name of the key frame
          */
         void JumpToKeyFrame(const std::string &name) { mKeyFrameName = name; SetIsUpdated(true); }

         /**
          * Restarts the playback
          */
         void RestartPlayback() { mRestartPlayback = true; SetIsUpdated(true); }

         /**
          * Sets the playback speed
          * @float speed The new speed
          */
         void SetPlaybackSpeed(float speed) { mPlaybackSpeed = speed; SetIsUpdated(true); }

         /**
          * Returns the playback speed
          * @return mPlaybackSpeed
          */
         float GetPlaybackSpeed() const { return mPlaybackSpeed; }

         /**
          * Returns true if we are playing back
          * @return mIsPlayingBack
          */
         bool GetIsPlayingBack() const { return mIsPlayingBack; }

      protected:

         /// Destructor
         virtual ~ControlsPlaybackConfigObject();

      private:

         std::string mInputFilename;
         bool mShowAdvancedOptions;
         bool mBeginPlayback;
         bool mEndPlayback;
         bool mJumpToNextKeyFrame;
         bool mJumpToPreviousKeyFrame;
         bool mRestartPlayback; 
         float mPlaybackSpeed;
         bool mIsPlayingBack;
         std::string mKeyFrameName;
      };
}

#endif
