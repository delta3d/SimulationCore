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

   class ViewWindowConfigObject;
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

         /**
          * Change the instance version of the Settings. This allows one to have multiple
          * sets of setting/configurations.
          */
         void ChangeSettingsInstance(const std::string& instanceName);

         /// Accessor to the internal settings
         StealthViewerSettings& GetSettings();

         /// Constant overload
         const StealthViewerSettings& GetSettings() const;

         /**
          * Returns the envirnonment config object
          * @return mEnvironmentConfigObject
          */
         StealthGM::PreferencesEnvironmentConfigObject& GetEnvironmentConfigObject();

         /**
          * Returns the general config object
          * @return mGeneralConfigObject
          */
         StealthGM::PreferencesGeneralConfigObject& GetGeneralConfigObject();

         /**
          * Returns the tools config object
          * @return mToolsConfigObject
          */
         StealthGM::PreferencesToolsConfigObject& GetToolsConfigObject();

         /**
          * Returns the visibility settings config object
          * @return mVisibilityConfigObject
          */
         StealthGM::PreferencesVisibilityConfigObject& GetVisibilityConfigObject();

         /**
          * Returns the camera config object
          * @return mCameraConfigObject
          */
         StealthGM::ControlsCameraConfigObject& GetCameraConfigObject();

         /**
          * Returns the record config object
          * @return mRecordConfigObject
          */
         StealthGM::ControlsRecordConfigObject& GetRecordConfigObject();

         /**
          * Returns the playback config object
          * @return mPlaybackConfigObject
          */
         StealthGM::ControlsPlaybackConfigObject& GetPlaybackConfigObject();

         /**
          * Returns the View Window Config Object
          * @return mViewWindowConfigObject
          */
         StealthGM::ViewWindowConfigObject& GetViewWindowConfigObject();

         /**
          * Returns a pointer to the main window of the application
          * @return mMainWindow
          */
         MainWindow* GetMainWindow();

         /**
          * Sets the main window
          * @param window The window
          */
         void SetMainWindow(MainWindow& window);

         /**
          * Sets the previous connection name
          * @param name The new name
          */
         void SetOldConnectionName(const QString& name);

         /**
          * Gets the old connection name
          * @return mOldConnectionName
          */
         const QString& GetOldConnectionName() const;

      protected:

         /// Destructor
         virtual ~StealthViewerData();

      private:

         /// Constructor
         StealthViewerData();

         /// Copy constructor
         StealthViewerData(const StealthViewerData&);

         /// Assignment operator
         StealthViewerData& operator = (const StealthViewerData&);

         static dtCore::RefPtr<StealthViewerData> mInstance;

         StealthViewerSettings* mSettings;

         StealthGM::PreferencesEnvironmentConfigObject* mEnvironmentConfigObject;
         StealthGM::PreferencesGeneralConfigObject* mGeneralConfigObject;
         StealthGM::PreferencesToolsConfigObject* mToolsConfigObject;
         StealthGM::PreferencesVisibilityConfigObject* mVisibilityConfigObject;

         StealthGM::ControlsCameraConfigObject* mCameraConfigObject;
         StealthGM::ControlsRecordConfigObject* mRecordConfigObject;
         StealthGM::ControlsPlaybackConfigObject* mPlaybackConfigObject;

         StealthGM::ViewWindowConfigObject* mViewWindowConfigObject;

         MainWindow* mMainWindow;

         QString mOldConnectionName;
   };
}

#endif
