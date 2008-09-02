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
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/MainWindow.h>
#include <ui_MainWindowUi.h>

#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>

#include <SimCore/Components/LabelManager.h>

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>
#include <vector>

namespace StealthQt
{
   const QString StealthViewerSettings::ORGANIZATION("delta3d.org");
   const QString StealthViewerSettings::APPLICATION("Stealth Viewer");

   const QString StealthViewerSettings::CONNECTION("CONNECTION");
   const QString StealthViewerSettings::NAME("NAME");
   const QString StealthViewerSettings::MAP_RESOURCE("MAP_RESOURCE");
   const QString StealthViewerSettings::CONFIG_RESOURCE("CONFIG_RESOURCE");
   const QString StealthViewerSettings::FED_RESOURCE("FED_RESOURCE");
   const QString StealthViewerSettings::FEDEX("FEDEX");
   const QString StealthViewerSettings::FEDERATE_NAME("FEDERATE_NAME");
   const QString StealthViewerSettings::RID_FILE("RID_FILE");

   const QString StealthViewerSettings::GENERAL_GROUP("GENERAL_GROUP");
      const QString StealthViewerSettings::DOCK_STATE("DOCK_STATE");
      const QString StealthViewerSettings::WINDOW_GEOMETRY("WINDOW_GEOMETRY");
      const QString StealthViewerSettings::AUTO_REFRESH_ENTITY_INFO("AUTO_REFRESH_ENTITY_INFO");

   const QString StealthViewerSettings::PREFERENCES_GENERAL_GROUP("PREFERENCES_GENERAL_GROUP");
      const QString StealthViewerSettings::ATTACH_MODE("ATTACH_MODE");
      const QString StealthViewerSettings::CAMERA_COLLISION("CAMERA_COLLISION");
      const QString StealthViewerSettings::PERFORMANCE("PERFORMANCE");
      const QString StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS("SHOW_ADVANCED_GENERAL_OPTIONS");
      const QString StealthViewerSettings::LOD_SCALE("LOD_SCALE");
      const QString StealthViewerSettings::NEAR_CLIPPING_PLANE("NEAR_CLIPPING_PLANE");
      const QString StealthViewerSettings::FAR_CLIPPING_PLANE("FAR_CLIPPING_PLANE");
      const QString StealthViewerSettings::RECONNECT_ON_STARTUP("RECONNECT_ON_STARTUP");
      const QString StealthViewerSettings::STARTUP_CONNECTION_NAME("STARTUP_CONNECTION_NAME");

   const QString StealthViewerSettings::PREFERENCES_ENVIRONMENT_GROUP("PREFERENCES_ENVIRONMENT_GROUP");
      const QString StealthViewerSettings::USE_NETWORK_SETTINGS("USE_NETWORK_SETTINGS");
      const QString StealthViewerSettings::USE_THEMED_SETTINGS("USE_THEMED_SETTINGS");
         const QString StealthViewerSettings::WEATHER_THEME("WEATHER_THEME");
         const QString StealthViewerSettings::TIME_THEME("TIME_THEME");
      const QString StealthViewerSettings::USE_CUSTOM_SETTINGS("USE_CUSTOM_SETTINGS");
         const QString StealthViewerSettings::CUSTOM_HOUR("CUSTOM_HOUR");
         const QString StealthViewerSettings::CUSTOM_MINUTE("CUSTOM_MINUTE");
         const QString StealthViewerSettings::CUSTOM_SECOND("CUSTOM_SECOND");
         const QString StealthViewerSettings::CUSTOM_VISIBILITY("CUSTOM_VISIBILITY");
         const QString StealthViewerSettings::CUSTOM_CLOUD_COVER("CUSTOM_CLOUD_COVER");

   const QString StealthViewerSettings::PREFERENCES_TOOLS_GROUP("PREFERENCES_TOOLS_GROUP");
      const QString StealthViewerSettings::COORDINATE_SYSTEM("COORDINATE_SYSTEM");
      const QString StealthViewerSettings::SHOW_BINOCULAR_IMAGE("SHOW_BINOCULAR_IMAGE");
      const QString StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT("SHOW_DISTANCE_TO_OBJECT");
      const QString StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT("SHOW_ELEVATION_OF_OBJECT");
      const QString StealthViewerSettings::MAGNIFICATION("MAGNIFICATION");
      const QString StealthViewerSettings::AUTO_ATTACH_ON_SELECTION("AUTO_ATTACH_ON_SELECTION");

   const QString StealthViewerSettings::PREFERENCES_VISIBILITY_GROUP("PREFERENCES_VISIBILITY_GROUP");
      const QString StealthViewerSettings::SHOW_LABELS("SHOW_LABELS");
      const QString StealthViewerSettings::SHOW_LABELS_FOR_ENTITIES("SHOW_LABELS_FOR_ENTITIES");
      const QString StealthViewerSettings::SHOW_LABELS_FOR_BLIPS("SHOW_LABELS_FOR_BLIPS");
      const QString StealthViewerSettings::SHOW_LABELS_FOR_TRACKS("SHOW_LABELS_FOR_TRACKS");
      const QString StealthViewerSettings::LABEL_MAX_DISTANCE("LABEL_MAX_DISTANCE");

   const QString StealthViewerSettings::CONTROLS_CAMERA_GROUP("CONTROLS_CAMERA_GROUP");

   const QString StealthViewerSettings::CONTROLS_RECORD_GROUP("CONTROLS_RECORD_GROUP");
      const QString StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS("SHOW_ADVANCED_RECORD_OPTIONS");
      const QString StealthViewerSettings::RECORD_OUTPUT_FILE("RECORD_OUTPUT_FILE");
      const QString StealthViewerSettings::AUTO_KEY_FRAME("AUTO_KEY_FRAME");
      const QString StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL("AUTO_KEY_FRAME_INTERVAL");

   const QString StealthViewerSettings::CONTROLS_PLAYBACK_GROUP("CONTROLS_PLAYBACK_GROUP");
      const QString StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS("SHOW_ADVANCED_PLAYBACK_OPTIONS");
      const QString StealthViewerSettings::PLAYBACK_INPUT_FILE("PLAYBACK_INPUT_FILE");
      const QString StealthViewerSettings::PLAYBACK_SPEED("PLAYBACK_SPEED");

   StealthViewerSettings::StealthViewerSettings(const QString &applicationName) :
      QSettings(QSettings::IniFormat,
                QSettings::UserScope,
                StealthViewerSettings::ORGANIZATION,
                applicationName),
      mNumConnections(0),
      mIsLoadingFromIni(false)
   {
      ParseIniFile();
   }

   StealthViewerSettings::~StealthViewerSettings()
   {

   }

   bool StealthViewerSettings::AddConnection(const QString &name,
                                             const QString &mapResource,
                                             const QString &configResource,
                                             const QString &fedResource,
                                             const QString &fedex,
                                             const QString &federateName,
                                             const QString &ridFile,
                                             bool isEditMode)
   {
      if(name.isEmpty() || mapResource.isEmpty() || configResource.isEmpty() ||
         fedResource.isEmpty() || fedex.isEmpty() || federateName.isEmpty() ||
         ridFile.isEmpty())
      {
         LOG_ERROR("Could not add the current connection because it contained empty data.");
         return false;
      }

      if(!isEditMode && !mIsLoadingFromIni)
      {
         QStringList existingConnections = GetConnectionNames();
         for(int i = 0; i < existingConnections.size(); i++)
         {
            if(name == existingConnections[i])
            {
               QString message = tr("Failed to add the connection named: ") + name +
                  tr(" because a connection named: ") + name + tr(" already exists. ") +
                  tr("Please select a unique connection name.");

               QMessageBox::critical(NULL, tr("Failed to add the connection"),
                 message, QMessageBox::Ok);

               return false;
            }
         }
      }

      if((!ContainsConnection(name) || mIsLoadingFromIni) && !isEditMode)
      {
         QString groupName = StealthViewerSettings::CONNECTION + QString::number(mNumConnections);
         beginGroup(groupName);

            setValue(StealthViewerSettings::NAME,            name);
            setValue(StealthViewerSettings::MAP_RESOURCE,    mapResource);
            setValue(StealthViewerSettings::CONFIG_RESOURCE, configResource);
            setValue(StealthViewerSettings::FED_RESOURCE,    fedResource);
            setValue(StealthViewerSettings::FEDEX,           fedex);
            setValue(StealthViewerSettings::FEDERATE_NAME,   federateName);
            setValue(StealthViewerSettings::RID_FILE,        ridFile);

         endGroup();

         if (mConnectionNameMap.insert(std::make_pair(name, mNumConnections)).second)
         {
            ++mNumConnections;
         }
      }
      else
      {
         // Edit an existing connection
         std::map<QString, unsigned int>::iterator itor =
            mConnectionNameMap.find(
               StealthViewerData::GetInstance().GetOldConnectionName());

         if(itor != mConnectionNameMap.end())
         {
            QString groupName = StealthViewerSettings::CONNECTION + QString::number(itor->second);
            beginGroup(groupName);

               setValue(StealthViewerSettings::NAME,            name);
               setValue(StealthViewerSettings::MAP_RESOURCE,    mapResource);
               setValue(StealthViewerSettings::CONFIG_RESOURCE, configResource);
               setValue(StealthViewerSettings::FED_RESOURCE,    fedResource);
               setValue(StealthViewerSettings::FEDEX,           fedex);
               setValue(StealthViewerSettings::FEDERATE_NAME,   federateName);
               setValue(StealthViewerSettings::RID_FILE,        ridFile);

            endGroup();

            // The name of this connection was edited.
            // Since the connections are mapped by name we
            // need to readd it to the map, binded with the
            // same number since it is a simple edit, and
            // not a new connection
            unsigned int toReplace = itor->second;
            mConnectionNameMap.erase(itor);
            mConnectionNameMap.insert(std::make_pair(name, toReplace));
         }
      }

      return true;
   }

   QStringList StealthViewerSettings::GetConnectionProperties(const QString &connectionName)
   {
      QStringList list;

      std::map<QString, unsigned int>::iterator itor = mConnectionNameMap.find(connectionName);
      if(itor == mConnectionNameMap.end())
         return list;

      QString group = StealthViewerSettings::CONNECTION + QString::number(itor->second);

      list = LoadConnectionProperties(group);

      return list;
   }

   QStringList StealthViewerSettings::GetConnectionNames() const
   {
      QStringList results;
      bool found = false;
      unsigned int count = 0;
      do
      {
         QString connection = StealthViewerSettings::CONNECTION + QString::number(count) +
            "/" + StealthViewerSettings::NAME;

         if(contains(connection))
         {
            results.push_back(value(connection).toString());
            found = true;
         }
         else
         {
            found = false;
         }
         ++count;

      } while(count <= mNumConnections);

      return results;
   }

   bool StealthViewerSettings::ContainsConnection(const QString &connectionName)
   {
      return (!GetConnectionProperties(connectionName).isEmpty());
   }

   void StealthViewerSettings::ParseIniFile()
   {
      mIsLoadingFromIni = true;

      // Get the top level groups
      QStringList groups = childGroups();

      RemovePreferences(groups);

      for(int i = 0; i < groups.size(); i++)
      {
         QString group = groups[i];

         QStringList list = LoadConnectionProperties(group);

         // Add internally
         AddConnection(list[0], list[1], list[2],
                       list[3], list[4], list[5], list[6]);
      }

      mIsLoadingFromIni = false;
   }

   void StealthViewerSettings::RemoveConnection(const QString &connectionName)
   {
      std::map<QString, unsigned int>::iterator itor = mConnectionNameMap.find(connectionName);
      if(itor == mConnectionNameMap.end())
      {
         std::string message = "Failed to remove the connection: " +
                                connectionName.toStdString()
                               + " because a connection named: " +
                               connectionName.toStdString() + " does not exist.";

         LOG_ERROR(message.c_str());
         return;
      }

      // Locate the group to delete, then clear the file and write everything to it
      // that was there except the group
      QString toRemove = StealthViewerSettings::CONNECTION +
                         QString::number(itor->second);

      QStringList groups = childGroups();
      RemovePreferences(groups);
      groups.removeAll(toRemove);

      std::vector<QStringList> connectionsToAdd;

      for(int i = 0; i < groups.size(); i++)
      {
         QStringList item = LoadConnectionProperties(groups[i]);

         if(!item.isEmpty())
            connectionsToAdd.push_back(item);
      }

      // Clear the ini file, reset the counter, and clear the map before readding
      ClearAllSettings(true);

      // Add connections back
      for(size_t i = 0; i < connectionsToAdd.size(); i++)
      {
         AddConnection(connectionsToAdd[i][0], connectionsToAdd[i][1], connectionsToAdd[i][2],
                       connectionsToAdd[i][3], connectionsToAdd[i][4], connectionsToAdd[i][5],
                       connectionsToAdd[i][6]);
      }

      emit ItemDeleted(connectionName);
   }

   void StealthViewerSettings::ClearAllSettings(bool clearFileAlso)
   {
      mConnectionNameMap.clear();
      mNumConnections = 0;

      if(clearFileAlso)
         clear();
   }

   void StealthViewerSettings::WritePreferencesToFile()
   {
      beginGroup(StealthViewerSettings::GENERAL_GROUP);

         setValue(StealthViewerSettings::DOCK_STATE,
            StealthViewerData::GetInstance().GetMainWindow()->saveState(StealthViewerSettings::WINDOW_DOCK_ID));

         setValue(StealthViewerSettings::WINDOW_GEOMETRY,
            StealthViewerData::GetInstance().GetMainWindow()->saveGeometry());

         setValue(StealthViewerSettings::AUTO_REFRESH_ENTITY_INFO,
            StealthViewerData::GetInstance().GetGeneralConfigObject().GetAutoRefreshEntityInfoWindow());

      endGroup();

      // Low cyclomatic complexity
      WritePreferencesGeneralGroupToFile();
      WritePreferencesEnvironmentGroupToFile();
      WritePreferencesToolsGroupToFile();

      WriteControlsRecordGroupToFile();
      WriteControlsPlaybackGroupToFile();
   }

   void StealthViewerSettings::WritePreferencesGeneralGroupToFile()
   {
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_GENERAL_GROUP);

         setValue(StealthViewerSettings::ATTACH_MODE, genConfig.GetAttachMode().GetName().c_str());
         setValue(StealthViewerSettings::CAMERA_COLLISION, genConfig.GetEnableCameraCollision());
         setValue(StealthViewerSettings::PERFORMANCE, genConfig.GetPerformanceMode().GetName().c_str());
         setValue(StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS, genConfig.GetShowAdvancedOptions());
         setValue(StealthViewerSettings::LOD_SCALE, genConfig.GetLODScale());
         setValue(StealthViewerSettings::NEAR_CLIPPING_PLANE, genConfig.GetNearClippingPlane());
         setValue(StealthViewerSettings::FAR_CLIPPING_PLANE, genConfig.GetFarClippingPlane());
         setValue(StealthViewerSettings::RECONNECT_ON_STARTUP, genConfig.GetReconnectOnStartup());
         setValue(StealthViewerSettings::STARTUP_CONNECTION_NAME, genConfig.GetStartupConnectionName().c_str());

      endGroup();
   }

   void StealthViewerSettings::WritePreferencesEnvironmentGroupToFile()
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_ENVIRONMENT_GROUP);

         setValue(StealthViewerSettings::USE_NETWORK_SETTINGS, envConfig.GetUseNetworkSettings());

         setValue(StealthViewerSettings::USE_THEMED_SETTINGS, envConfig.GetUseThemedSettings());
         setValue(StealthViewerSettings::WEATHER_THEME, envConfig.GetWeatherTheme().GetName().c_str());
         setValue(StealthViewerSettings::TIME_THEME, envConfig.GetTimeTheme().GetName().c_str());

         setValue(StealthViewerSettings::USE_CUSTOM_SETTINGS, envConfig.GetUseCustomSettings());
         setValue(StealthViewerSettings::CUSTOM_HOUR, envConfig.GetCustomHour());
         setValue(StealthViewerSettings::CUSTOM_MINUTE, envConfig.GetCustomMinute());
         setValue(StealthViewerSettings::CUSTOM_SECOND, envConfig.GetCustomSecond());
         setValue(StealthViewerSettings::CUSTOM_VISIBILITY, envConfig.GetVisibility().GetName().c_str());
         setValue(StealthViewerSettings::CUSTOM_CLOUD_COVER, envConfig.GetCloudCover().GetName().c_str());

      endGroup();
   }

   void StealthViewerSettings::WritePreferencesToolsGroupToFile()
   {
      StealthGM::PreferencesToolsConfigObject& toolsObject =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_TOOLS_GROUP);

         setValue(StealthViewerSettings::COORDINATE_SYSTEM, toolsObject.GetCoordinateSystem().GetName().c_str());
         setValue(StealthViewerSettings::SHOW_BINOCULAR_IMAGE, toolsObject.GetShowBinocularImage());
         setValue(StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT, toolsObject.GetShowDistanceToObject());
         setValue(StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT, toolsObject.GetShowElevationOfObject());
         setValue(StealthViewerSettings::MAGNIFICATION, toolsObject.GetMagnification());
         setValue(StealthViewerSettings::AUTO_ATTACH_ON_SELECTION, toolsObject.GetAutoAttachOnSelection());

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WritePreferencesVisibilityGroupToFile()
   {
      StealthGM::PreferencesVisibilityConfigObject& visObject =
         StealthViewerData::GetInstance().GetVisibilityConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_VISIBILITY_GROUP);
         SimCore::Components::LabelOptions options = visObject.GetOptions();

         setValue(StealthViewerSettings::SHOW_LABELS, options.ShowLabels());
         setValue(StealthViewerSettings::SHOW_LABELS_FOR_BLIPS, options.ShowLabelsForBlips());
         setValue(StealthViewerSettings::SHOW_LABELS_FOR_ENTITIES, options.ShowLabelsForEntities());
         setValue(StealthViewerSettings::SHOW_LABELS_FOR_TRACKS, options.ShowLabelsForPositionReports());
         setValue(StealthViewerSettings::LABEL_MAX_DISTANCE, options.GetMaxLabelDistance());
      endGroup();
   }


   void StealthViewerSettings::WriteControlsRecordGroupToFile()
   {
      StealthGM::ControlsRecordConfigObject &recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      beginGroup(StealthViewerSettings::CONTROLS_RECORD_GROUP);

         setValue(StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS, recordObject.GetShowAdvancedOptions());
         setValue(StealthViewerSettings::RECORD_OUTPUT_FILE, recordObject.GetOutputFilename().c_str());
         setValue(StealthViewerSettings::AUTO_KEY_FRAME, recordObject.GetAutoKeyFrame());
         setValue(StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL, recordObject.GetAutoKeyFrameInterval());

      endGroup();
   }

   void StealthViewerSettings::WriteControlsPlaybackGroupToFile()
   {
      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      beginGroup(StealthViewerSettings::CONTROLS_PLAYBACK_GROUP);

         setValue(StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS, pbObject.GetShowAdvancedOptions());
         setValue(StealthViewerSettings::PLAYBACK_INPUT_FILE, pbObject.GetInputFilename().c_str());
         setValue(StealthViewerSettings::PLAYBACK_SPEED, pbObject.GetPlaybackSpeed());

      endGroup();
   }

   void StealthViewerSettings::LoadPreferences()
   {
      beginGroup(StealthViewerSettings::GENERAL_GROUP);

         if(contains(StealthViewerSettings::DOCK_STATE))
         {
            QByteArray state = value(StealthViewerSettings::DOCK_STATE).toByteArray();
            StealthViewerData::GetInstance().GetMainWindow()->restoreState(state, StealthViewerSettings::WINDOW_DOCK_ID);
         }

         if(contains(StealthViewerSettings::WINDOW_GEOMETRY))
         {
            QByteArray geometry = value(StealthViewerSettings::WINDOW_GEOMETRY).toByteArray();
            StealthViewerData::GetInstance().GetMainWindow()->restoreGeometry(geometry);
         }

         if (contains(StealthViewerSettings::AUTO_REFRESH_ENTITY_INFO))
         {
            bool enable =
               value(StealthViewerSettings::AUTO_REFRESH_ENTITY_INFO).toBool();

            StealthViewerData::GetInstance().GetGeneralConfigObject().SetAutoRefreshEntityInfoWindow(enable);
         }

      endGroup();

      LoadPreferencesGeneral();
      LoadPreferencesEnvironment();
      LoadPreferencesTools();

      LoadControlsRecord();
      LoadControlsPlayback();
   }

   void StealthViewerSettings::LoadPreferencesGeneral()
   {
      beginGroup(StealthViewerSettings::PREFERENCES_GENERAL_GROUP);

         StealthGM::PreferencesGeneralConfigObject &genConfig =
            StealthViewerData::GetInstance().GetGeneralConfigObject();

         if(contains(StealthViewerSettings::ATTACH_MODE))
         {
            QString savedValue =
               value(StealthViewerSettings::ATTACH_MODE).toString();

            genConfig.SetAttachMode(savedValue.toStdString());
         }

         if(contains(StealthViewerSettings::CAMERA_COLLISION))
         {
            bool savedValue = value(StealthViewerSettings::CAMERA_COLLISION).toBool();
            genConfig.SetCameraCollision(savedValue);
         }

         if(contains(StealthViewerSettings::PERFORMANCE))
         {
            QString savedValue = value(StealthViewerSettings::PERFORMANCE).toString();
            genConfig.SetPerformanceMode(savedValue.toStdString());
         }

         if(contains(StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS).toBool();
            genConfig.SetShowAdvancedOptions(savedValue);
         }

         if(contains(StealthViewerSettings::LOD_SCALE))
         {
            float savedValue = float(value(StealthViewerSettings::LOD_SCALE).toDouble());
            genConfig.SetLODScale(savedValue);
         }

         if(contains(StealthViewerSettings::NEAR_CLIPPING_PLANE))
         {
            double savedValue =
               value(StealthViewerSettings::NEAR_CLIPPING_PLANE).toDouble();
            genConfig.SetNearClippingPlane(savedValue);
         }

         if(contains(StealthViewerSettings::FAR_CLIPPING_PLANE))
         {
            double savedValue =
               value(StealthViewerSettings::FAR_CLIPPING_PLANE).toDouble();
            genConfig.SetFarClippingPlane(savedValue);
         }

         bool connectValue = true;
         QString name;
         if(contains(StealthViewerSettings::RECONNECT_ON_STARTUP))
         {
            connectValue = value(StealthViewerSettings::RECONNECT_ON_STARTUP).toBool();
         }

         if(contains(StealthViewerSettings::STARTUP_CONNECTION_NAME))
         {
            name = value(StealthViewerSettings::STARTUP_CONNECTION_NAME).toString();
         }

         genConfig.SetReconnectOnStartup(connectValue, name.toStdString());

      endGroup();
   }

   void StealthViewerSettings::LoadPreferencesEnvironment()
   {
      beginGroup(StealthViewerSettings::PREFERENCES_ENVIRONMENT_GROUP);

         StealthGM::PreferencesEnvironmentConfigObject &envConfig =
            StealthViewerData::GetInstance().GetEnvironmentConfigObject();

         if(contains(StealthViewerSettings::USE_NETWORK_SETTINGS))
         {
            bool savedValue = value(StealthViewerSettings::USE_NETWORK_SETTINGS).toBool();
            if(savedValue)
               envConfig.SetUseNetworkSettings();
         }

         if(contains(StealthViewerSettings::USE_THEMED_SETTINGS))
         {
            bool savedValue = value(StealthViewerSettings::USE_THEMED_SETTINGS).toBool();
            if(savedValue)
               envConfig.SetUseThemedSettings();
         }

         if(contains(StealthViewerSettings::WEATHER_THEME))
         {
            QString savedValue = value(StealthViewerSettings::WEATHER_THEME).toString();
            envConfig.SetWeatherTheme(savedValue.toStdString());
         }

         if(contains(StealthViewerSettings::TIME_THEME))
         {
            QString savedValue = value(StealthViewerSettings::TIME_THEME).toString();
            envConfig.SetTimeTheme(savedValue.toStdString());
         }

         if(contains(StealthViewerSettings::USE_CUSTOM_SETTINGS))
         {
            bool savedValue = value(StealthViewerSettings::USE_CUSTOM_SETTINGS).toBool();
            if(savedValue)
               envConfig.SetUseCustomSettings();
         }

         if(contains(StealthViewerSettings::CUSTOM_HOUR))
         {
            int savedValue = value(StealthViewerSettings::CUSTOM_HOUR).toInt();
            envConfig.SetCustomHour(savedValue);
         }

         if(contains(StealthViewerSettings::CUSTOM_MINUTE))
         {
            int savedValue = value(StealthViewerSettings::CUSTOM_MINUTE).toInt();
            envConfig.SetCustomMinute(savedValue);
         }

         if(contains(StealthViewerSettings::CUSTOM_SECOND))
         {
            int savedValue = value(StealthViewerSettings::CUSTOM_SECOND).toInt();
            envConfig.SetCustomSecond(savedValue);
         }

         if(contains(StealthViewerSettings::CUSTOM_VISIBILITY))
         {
            QString savedValue = value(StealthViewerSettings::CUSTOM_VISIBILITY).toString();
            envConfig.SetVisibility(savedValue.toStdString());
         }

         if(contains(StealthViewerSettings::CUSTOM_CLOUD_COVER))
         {
            QString savedValue = value(StealthViewerSettings::CUSTOM_CLOUD_COVER).toString();
            envConfig.SetCloudCover(savedValue.toStdString());
         }

      endGroup();
   }

   void StealthViewerSettings::LoadPreferencesTools()
   {
      beginGroup(StealthViewerSettings::PREFERENCES_TOOLS_GROUP);

         StealthGM::PreferencesToolsConfigObject &toolsConfig =
            StealthViewerData::GetInstance().GetToolsConfigObject();

         if(contains(StealthViewerSettings::COORDINATE_SYSTEM))
         {
            QString savedValue = value(StealthViewerSettings::COORDINATE_SYSTEM).toString();
            StealthGM::PreferencesToolsConfigObject::CoordinateSystem* coordSystem =
               StealthGM::PreferencesToolsConfigObject::CoordinateSystem::GetValueForName(savedValue.toStdString());

            if (coordSystem != NULL)
               toolsConfig.SetCoordinateSystem(*coordSystem);
         }

         if(contains(StealthViewerSettings::SHOW_BINOCULAR_IMAGE))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_BINOCULAR_IMAGE).toBool();
            toolsConfig.SetShowBinocularImage(savedValue);
         }

         if(contains(StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT).toBool();
            toolsConfig.SetShowDistanceToObject(savedValue);
         }

         if(contains(StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT).toBool();
            toolsConfig.SetShowElevationOfObject(savedValue);
         }

         if(contains(StealthViewerSettings::MAGNIFICATION))
         {
            float savedValue = float(value(StealthViewerSettings::MAGNIFICATION).toDouble());
            toolsConfig.SetMagnification(savedValue);
         }

         if(contains(StealthViewerSettings::AUTO_ATTACH_ON_SELECTION))
         {
            bool savedValue = value(StealthViewerSettings::AUTO_ATTACH_ON_SELECTION).toBool();
            toolsConfig.SetAutoAttachOnSelection(savedValue);
         }

      endGroup();
   }

   void StealthViewerSettings::LoadPreferencesVisibility()
   {
      StealthGM::PreferencesVisibilityConfigObject& visObject =
         StealthViewerData::GetInstance().GetVisibilityConfigObject();

      beginGroup(PREFERENCES_VISIBILITY_GROUP);
         SimCore::Components::LabelOptions options = visObject.GetOptions();

         if (contains(SHOW_LABELS))
         {
            options.SetShowLabels(value(SHOW_LABELS).toBool());
         }

         if (contains(SHOW_LABELS_FOR_BLIPS))
         {
            options.SetShowLabelsForBlips(value(SHOW_LABELS_FOR_BLIPS).toBool());
         }

         if (contains(SHOW_LABELS_FOR_ENTITIES))
         {
            options.SetShowLabelsForEntities(value(SHOW_LABELS_FOR_ENTITIES).toBool());
         }

         if (contains(SHOW_LABELS_FOR_TRACKS))
         {
            options.SetShowLabelsForPositionReports(value(SHOW_LABELS_FOR_TRACKS).toBool());
         }

         if (contains(LABEL_MAX_DISTANCE))
         {
            options.SetMaxLabelDistance(value(LABEL_MAX_DISTANCE).toDouble());
         }
      endGroup();
   }

   void StealthViewerSettings::LoadControlsRecord()
   {
      beginGroup(StealthViewerSettings::CONTROLS_RECORD_GROUP);

         StealthGM::ControlsRecordConfigObject &recordConfig =
            StealthViewerData::GetInstance().GetRecordConfigObject();

         if(contains(StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS).toBool();
            recordConfig.SetShowAdvancedOptions(savedValue);
         }

         if(contains(StealthViewerSettings::RECORD_OUTPUT_FILE))
         {
            QString savedValue = value(StealthViewerSettings::RECORD_OUTPUT_FILE).toString();
            recordConfig.SetOutputFilename(savedValue.toStdString());
         }

         if(contains(StealthViewerSettings::AUTO_KEY_FRAME))
         {
            bool savedValue = value(StealthViewerSettings::AUTO_KEY_FRAME).toBool();
            recordConfig.SetAutoKeyFrame(savedValue);
         }

         if(contains(StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL))
         {
            int savedValue = value(StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL).toInt();
            recordConfig.SetAutoKeyFrameInterval(savedValue);
         }

      endGroup();
   }

   void StealthViewerSettings::LoadControlsPlayback()
   {
      beginGroup(StealthViewerSettings::CONTROLS_PLAYBACK_GROUP);

         StealthGM::ControlsPlaybackConfigObject &playbackConfig =
            StealthViewerData::GetInstance().GetPlaybackConfigObject();

         if(contains(StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS).toBool();
            playbackConfig.SetShowAdvancedOptions(savedValue);
         }

         if(contains(StealthViewerSettings::PLAYBACK_INPUT_FILE))
         {
            QString savedValue = value(StealthViewerSettings::PLAYBACK_INPUT_FILE).toString();
            playbackConfig.SetInputFilename(savedValue.toStdString());
         }

         if(contains(StealthViewerSettings::PLAYBACK_SPEED))
         {
            float savedValue = float(value(StealthViewerSettings::PLAYBACK_SPEED).toDouble());
            playbackConfig.SetPlaybackSpeed(savedValue);
         }

      endGroup();
   }

   void StealthViewerSettings::RemovePreferences(QStringList &groups)
   {
      for(int i = 0; i < groups.size(); i++)
      {
         QString group = groups[i];

         if(group == StealthViewerSettings::GENERAL_GROUP                 ||
            group == StealthViewerSettings::PREFERENCES_GENERAL_GROUP     ||
            group == StealthViewerSettings::PREFERENCES_ENVIRONMENT_GROUP ||
            group == StealthViewerSettings::PREFERENCES_TOOLS_GROUP       ||
            group == StealthViewerSettings::CONTROLS_CAMERA_GROUP         ||
            group == StealthViewerSettings::CONTROLS_RECORD_GROUP         ||
            group == StealthViewerSettings::CONTROLS_PLAYBACK_GROUP)
         {
            groups.erase(groups.begin() + i);
            i = 0;
         }
      }
   }

   QStringList StealthViewerSettings::LoadConnectionProperties(const QString &group)
   {
      QStringList props;

      beginGroup(group);

         if(contains(StealthViewerSettings::NAME))
            props.push_back(value(StealthViewerSettings::NAME).toString());
         else
            props.push_back(tr(""));

         if(contains(StealthViewerSettings::MAP_RESOURCE))
            props.push_back(value(StealthViewerSettings::MAP_RESOURCE).toString());
         else
            props.push_back(tr(""));

         if(contains(StealthViewerSettings::CONFIG_RESOURCE))
            props.push_back(value(StealthViewerSettings::CONFIG_RESOURCE).toString());
         else
            props.push_back(tr(""));

         if(contains(StealthViewerSettings::FED_RESOURCE))
            props.push_back(value(StealthViewerSettings::FED_RESOURCE).toString());
         else
            props.push_back(tr(""));

         if(contains(StealthViewerSettings::FEDEX))
            props.push_back(value(StealthViewerSettings::FEDEX).toString());
         else
            props.push_back(tr(""));

         if(contains(StealthViewerSettings::FEDERATE_NAME))
            props.push_back(value(StealthViewerSettings::FEDERATE_NAME).toString());
         else
            props.push_back(tr(""));

         if(contains(StealthViewerSettings::RID_FILE))
            props.push_back(value(StealthViewerSettings::RID_FILE).toString());
         else
            props.push_back(tr(""));

      endGroup();

      return props;
   }
}
