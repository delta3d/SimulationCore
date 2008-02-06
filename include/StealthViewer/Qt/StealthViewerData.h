/* -*-c++-*-
* Stealth Viewer
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
#ifndef STEALTH_VIEWER_DATA
#define STEALTH_VIEWER_DATA

#include <osg/Referenced>

#include <dtCore/refptr.h>

#include <QtCore/QString>

namespace StealthGM
{
   class PreferencesGeneralConfigObject;
   class PreferencesToolsConfigObject;
   class PreferencesVisibilityConfigObject;
   class PreferencesEnvironmentConfigObject;

   class ControlsCameraConfigObject;
   class ControlsRecordConfigObject;
   class ControlsPlaybackConfigObject;
}

namespace StealthQt
{
   class StealthViewerSettings;
   class MainWindow;

   class StealthViewerData : public osg::Referenced
   {
      public:

         /// Accessor to the instance
         static StealthViewerData& GetInstance(); 

         /// Accessor to the internal settings
         StealthViewerSettings& GetSettings() { return *mSettings; }

         /// Constant overload
         const StealthViewerSettings& GetSettings() const { return *mSettings; }

         /**
          * Returns the general config object
          * @return mGeneralConfigObject
          */
         StealthGM::PreferencesGeneralConfigObject& GetGeneralConfigObject() { return *mGeneralConfigObject; }

         /**
          * Returns the envirnonment config object
          * @return mEnvironmentConfigObject
          */
         StealthGM::PreferencesEnvironmentConfigObject& GetEnvironmentConfigObject() { return *mEnvironmentConfigObject; }

         /**
          * Returns the tools config object
          * @return mToolsConfigObject
          */
         StealthGM::PreferencesToolsConfigObject& GetToolsConfigObject() { return *mToolsConfigObject; }

         /**
          * Returns the camera config object
          * @return mCameraConfigObject
          */
         StealthGM::ControlsCameraConfigObject& GetCameraConfigObject() { return *mCameraConfigObject; }
         
         /**
          * Returns the record config object
          * @return mRecordConfigObject
          */
         StealthGM::ControlsRecordConfigObject& GetRecordConfigObject() { return *mRecordConfigObject; }
         
         /**
          * Returns the playback config object
          * @return mPlaybackConfigObject
          */
         StealthGM::ControlsPlaybackConfigObject& GetPlaybackConfigObject() { return *mPlaybackConfigObject; }
      
         /**
          * Returns a pointer to the main window of the application
          * @return mMainWindow
          */
         MainWindow* GetMainWindow() { return mMainWindow; }

         /**
          * Sets the main window
          * @param window The window
          */
         void SetMainWindow(MainWindow &window) { mMainWindow = &window; }

         /**
          * Sets the previous connection name
          * @param name The new name
          */
         void SetOldConnectionName(const QString &name) { mOldConnectionName = name; }

         /**
          * Gets the old connection name
          * @return mOldConnectionName
          */
         const QString& GetOldConnectionName() const { return mOldConnectionName; }

      protected:

         /// Destructor
         virtual ~StealthViewerData();

      private:

         /// Constructor
         StealthViewerData();

         /// Copy constructor
         StealthViewerData(const StealthViewerData &) { }

         /// Assignment operator 
         StealthViewerData& operator = (const StealthViewerData &) { return *this; }

         static dtCore::RefPtr<StealthViewerData> mInstance;

         StealthViewerSettings *mSettings;

         StealthGM::PreferencesGeneralConfigObject     *mGeneralConfigObject;
         StealthGM::PreferencesEnvironmentConfigObject *mEnvironmentConfigObject;
         StealthGM::PreferencesToolsConfigObject       *mToolsConfigObject;

         StealthGM::ControlsCameraConfigObject   *mCameraConfigObject;
         StealthGM::ControlsRecordConfigObject   *mRecordConfigObject;
         StealthGM::ControlsPlaybackConfigObject *mPlaybackConfigObject;

         MainWindow* mMainWindow;

         QString mOldConnectionName;
   };
}

#endif
