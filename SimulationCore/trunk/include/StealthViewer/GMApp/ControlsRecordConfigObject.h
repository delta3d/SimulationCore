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
#include <StealthGM/ConfigurationObjectInterface.h>
#include <StealthGM/Export.h>

#include <string>

namespace StealthGM
{
   class STEALTH_GAME_EXPORT ControlsRecordConfigObject : public ConfigurationObjectInterface
   {
      public:

         /// Constructor
         ControlsRecordConfigObject();

         /**
          * Applys changes on the game manager
          */
         virtual void ApplyChanges(dtGame::GameManager &gameManager);

         /**
          * Sets the output file name
          * @param filename The name of the file
          */
         void SetOutputFilename(const std::string &filename) { mOutputFilename = filename; SetIsUpdated(true); }

         /**
          * Gets the current output file name
          * @return mOutputFilename
          */
         const std::string& GetOutputFilename() const { return mOutputFilename; }

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
          * Sets auto key framing
          * @param enable True to auto key frame every mAutoKeyFrameInterval
          */
         void SetAutoKeyFrame(bool enable) { mAutoKeyFrame = enable; SetIsUpdated(true); }

         /**
          * Returns true if auto key framing
          */
         bool GetAutoKeyFrame() const { return mAutoKeyFrame; }

         /**
          * Sets the auto key frame interval
          * @param mins The number of minutes
          */
         void SetAutoKeyFrameInterval(int mins) { mAutoKeyFrameInterval = mins; SetIsUpdated(true); }
         
         /**
          * Returns the auto key frame interval
          * @return mAutoKeyFrameInterval
          */
         int GetAutoKeyFrameInterval() const { return mAutoKeyFrameInterval; }

         /**
          * Sets recording on the next tick
          * @param enable True to record, false to stop
          */
         void StartRecording() { mStartRecording = true; SetIsUpdated(true); }

         /**
          * Adds a time marker
          */
         void AddKeyFrame() { mAddKeyFrame = true; SetIsUpdated(true); } 

         /**
          * Stops recording next tick
          */
         void StopRecording() { mStopRecording = true; SetIsUpdated(true); }

         bool GetIsRecording() const { return mIsRecording; }

         /**
          * Disconnects from the federation
          */
         void DisconnectFromFederation() { mDisconnect = true; SetIsUpdated(true); }

         /**
          * Joins the federation
          */
         void JoinFederation() { mJoinFederation = true; SetIsUpdated(true); }

      protected:

         /// Destructor
         virtual ~ControlsRecordConfigObject();

      private:

         std::string mOutputFilename;
         bool mShowAdvancedOptions;
         bool mStartRecording;
         bool mStopRecording;
         bool mAutoKeyFrame;
         int  mAutoKeyFrameInterval;
         bool mAddKeyFrame;   
         bool mIsRecording;
         bool mDisconnect;
         bool mJoinFederation;
   }; 
}
