/* -*-c++-*-
* Stealth Viewer - StealthViewerData (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*
* @author Eddie Johnson, Curtiss Murphy
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
