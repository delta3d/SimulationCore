/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/MainWindow.h>

#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>

namespace StealthQt
{
   dtCore::RefPtr<StealthViewerData> StealthViewerData::mInstance(NULL);

   StealthViewerData::StealthViewerData() : 
      mSettings(new StealthViewerSettings), 
      mGeneralConfigObject(new StealthGM::PreferencesGeneralConfigObject), 
      mEnvironmentConfigObject(new StealthGM::PreferencesEnvironmentConfigObject),
      mToolsConfigObject(new StealthGM::PreferencesToolsConfigObject), 
      mCameraConfigObject(new StealthGM::ControlsCameraConfigObject),
      mRecordConfigObject(new StealthGM::ControlsRecordConfigObject),
      mPlaybackConfigObject(new StealthGM::ControlsPlaybackConfigObject), 
      mMainWindow(NULL)
   {

   }

   StealthViewerData::~StealthViewerData()
   {
      delete mSettings;
      mSettings = NULL;
   }

   StealthViewerData& StealthViewerData::GetInstance()
   {
      if(!mInstance.valid())
         mInstance = new StealthViewerData;

      return *mInstance; 
   }
}
