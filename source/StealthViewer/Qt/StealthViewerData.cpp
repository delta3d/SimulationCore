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
