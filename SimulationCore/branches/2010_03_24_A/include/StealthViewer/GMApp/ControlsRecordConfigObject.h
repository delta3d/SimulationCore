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
#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

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
         void DisconnectFromNetwork() { mDisconnect = true; SetIsUpdated(true); }

         /**
          * Joins the federation
          */
         void JoinNetwork() { mJoinNetwork = true; SetIsUpdated(true); }


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
         bool mJoinNetwork;
         bool mRequestKeyFrameUpdate;
         double mLastRequestTime;
   }; 
}
