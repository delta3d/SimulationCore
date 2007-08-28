/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <StealthQt/StealthViewerData.h>
#include <StealthQt/StealthViewerSettings.h>
#include <StealthQt/MainWindow.h>

#include <StealthGM/PreferencesGeneralConfigObject.h>
#include <StealthGM/PreferencesToolsConfigObject.h>
#include <StealthGM/ControlsRecordConfigObject.h>
#include <StealthGM/ControlsPlaybackConfigObject.h>
#include <StealthGM/PreferencesEnvironmentConfigObject.h>
#include <StealthGM/ControlsCameraConfigObject.h>

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
