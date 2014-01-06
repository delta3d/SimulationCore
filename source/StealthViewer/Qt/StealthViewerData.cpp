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
#include <prefix/StealthQtPrefix.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/MainWindow.h>

#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>

namespace StealthQt
{
   std::shared_ptr<StealthViewerData> StealthViewerData::mInstance(nullptr);

   //////////////////////////////////////////////////////////////////////
   StealthViewerData::StealthViewerData()
   : mSettings(new StealthViewerSettings)
   , mEnvironmentConfigObject(new StealthGM::PreferencesEnvironmentConfigObject)
   , mGeneralConfigObject(new StealthGM::PreferencesGeneralConfigObject)
   , mToolsConfigObject(new StealthGM::PreferencesToolsConfigObject)
   , mVisibilityConfigObject(new StealthGM::PreferencesVisibilityConfigObject)
   , mCameraConfigObject(new StealthGM::ControlsCameraConfigObject)
   , mRecordConfigObject(new StealthGM::ControlsRecordConfigObject)
   , mPlaybackConfigObject(new StealthGM::ControlsPlaybackConfigObject)
   , mViewWindowConfigObject(new StealthGM::ViewWindowConfigObject)
   , mMainWindow(nullptr)
   {

   }

   //////////////////////////////////////////////////////////////////////
   StealthViewerData::~StealthViewerData()
   {
      delete mSettings;
      mSettings = nullptr;
   }

   //////////////////////////////////////////////////////////////////////
   StealthViewerData& StealthViewerData::GetInstance()
   {
      if(!mInstance.valid())
         mInstance = new StealthViewerData;

      return *mInstance;
   }

   //////////////////////////////////////////////////////////////////////
   void StealthViewerData::ChangeSettingsInstance(const std::string& instanceName)
   {
      delete mSettings;
      mSettings = new StealthViewerSettings(instanceName.c_str());
   }

   //////////////////////////////////////////////////////////////////////
   StealthViewerSettings& StealthViewerData::GetSettings()
   {
      return *mSettings;
   }

   //////////////////////////////////////////////////////////////////////
   const StealthViewerSettings& StealthViewerData::GetSettings() const
   {
      return *mSettings;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::PreferencesEnvironmentConfigObject& StealthViewerData::GetEnvironmentConfigObject()
   {
      return *mEnvironmentConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::PreferencesGeneralConfigObject& StealthViewerData::GetGeneralConfigObject()
   {
      return *mGeneralConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::PreferencesToolsConfigObject& StealthViewerData::GetToolsConfigObject()
   {
      return *mToolsConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::PreferencesVisibilityConfigObject& StealthViewerData::GetVisibilityConfigObject()
   {
      return *mVisibilityConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::ControlsCameraConfigObject& StealthViewerData::GetCameraConfigObject()
   {
      return *mCameraConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::ControlsRecordConfigObject& StealthViewerData::GetRecordConfigObject()
   {
      return *mRecordConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::ControlsPlaybackConfigObject& StealthViewerData::GetPlaybackConfigObject()
   {
      return *mPlaybackConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   StealthGM::ViewWindowConfigObject& StealthViewerData::GetViewWindowConfigObject()
   {
      return *mViewWindowConfigObject;
   }

   //////////////////////////////////////////////////////////////////////
   MainWindow* StealthViewerData::GetMainWindow()
   {
      return mMainWindow;
   }

   //////////////////////////////////////////////////////////////////////
   void StealthViewerData::SetMainWindow(MainWindow& window)
   {
      mMainWindow = &window;
   }

   //////////////////////////////////////////////////////////////////////
   void StealthViewerData::SetOldConnectionName(const QString& name)
   {
      mOldConnectionName = name;
   }

   //////////////////////////////////////////////////////////////////////
   const QString& StealthViewerData::GetOldConnectionName() const
   {
      return mOldConnectionName;
   }

   //////////////////////////////////////////////////////////////////////
   StealthViewerData::StealthViewerData(const StealthViewerData&) { }

   //////////////////////////////////////////////////////////////////////
   StealthViewerData& StealthViewerData::operator = (const StealthViewerData&) { return *this; }
}
