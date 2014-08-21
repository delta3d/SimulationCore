/* -*-c++-*-
* Stealth Viewer
* Copyright 2007-2008, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
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
 * @author Eddie Johnson, Curtiss Murphy
 */
// For the Force and Domain Enums.  Thas has to be first because of a macro conflict with Qt.
#include <prefix/StealthQtPrefix.h>
#include <SimCore/Actors/BaseEntity.h>

#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/MainWindow.h>
#include <StealthViewer/Qt/ViewDockWidget.h>
#include <ui_MainWindowUi.h>

#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>

#include <SimCore/Components/LabelManager.h>
#include <SimCore/UnitEnums.h>

#include <osg/io_utils>

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>

//For an ugly hack...
#include <dtCore/system.h>
#include <vector>

namespace StealthQt
{
   const dtUtil::RefString StealthViewerSettings::CONNECTIONTYPE_NONE("(Select Type)");
   const dtUtil::RefString StealthViewerSettings::CONNECTIONTYPE_HLA("HLA");
   const dtUtil::RefString StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER("Client-Server");
   const dtUtil::RefString StealthViewerSettings::CONNECTIONTYPE_DIS("DIS");

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
   const QString StealthViewerSettings::RTI_STANDARD_VERSION("RTI_STANDARD_VERSION");
   const QString StealthViewerSettings::CONNECTION_TYPE("CONNECTION_TYPE");
   const QString StealthViewerSettings::SERVER_IP_ADDRESS("SERVER_IP_ADDRESS");
   const QString StealthViewerSettings::SERVER_PORT("SERVER_PORT");
   const QString StealthViewerSettings::SERVER_GAMENAME("SERVER_GAMENAME");
   const QString StealthViewerSettings::SERVER_GAMEVERSION("SERVER_GAMEVERSION");
   const QString StealthViewerSettings::DIS_IP_ADDRESS("DIS_IP_ADDRESS");
   const QString StealthViewerSettings::DIS_PORT("DIS_PORT");
   const QString StealthViewerSettings::DIS_BROADCAST("DIS_BROADCAST");
   const QString StealthViewerSettings::DIS_EXERCISE_ID("DIS_EXERCISE_ID");
   const QString StealthViewerSettings::DIS_SITE_ID("DIS_SITE_ID");
   const QString StealthViewerSettings::DIS_APPLICATION_ID("DIS_APPLICATION_ID");
   const QString StealthViewerSettings::DIS_MTU("DIS_MTU");
   const QString StealthViewerSettings::DIS_ACTOR_XML_FILE("DIS_ACTOR_XML_FILE");

   const QString StealthViewerSettings::GENERAL_GROUP("GENERAL_GROUP");
      const QString StealthViewerSettings::DOCK_STATE("DOCK_STATE");
      const QString StealthViewerSettings::WINDOW_GEOMETRY("WINDOW_GEOMETRY");
      const QString StealthViewerSettings::AUTO_REFRESH_ENTITY_INFO("AUTO_REFRESH_ENTITY_INFO");

   const QString StealthViewerSettings::PREFERENCES_GENERAL_GROUP("PREFERENCES_GENERAL_GROUP");
      const QString StealthViewerSettings::ATTACH_MODE("ATTACH_MODE");
      const QString StealthViewerSettings::ATTACH_POINT_NODE("ATTACH_POINT_NODE");
      const QString StealthViewerSettings::ATTACH_ROTATION("ATTACH_ROTATION");
      const QString StealthViewerSettings::AUTO_ATTACH_TO_CALLSIGN("AUTO_ATTACH_TO_CALLSIGN");
      const QString StealthViewerSettings::SHOULD_AUTO_ATTACH("SHOULD_AUTO_ATTACH");
      const QString StealthViewerSettings::CAMERA_COLLISION("CAMERA_COLLISION");
      const QString StealthViewerSettings::PERFORMANCE("PERFORMANCE");
      const QString StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS("SHOW_ADVANCED_GENERAL_OPTIONS");
      const QString StealthViewerSettings::LOD_SCALE("LOD_SCALE");
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
      const QString StealthViewerSettings::SHOW_COMPASS_360("SHOW_COMPASS_360");
      const QString StealthViewerSettings::SHOW_BINOCULAR_IMAGE("SHOW_BINOCULAR_IMAGE");
      const QString StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT("SHOW_DISTANCE_TO_OBJECT");
      const QString StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT("SHOW_ELEVATION_OF_OBJECT");
      const QString StealthViewerSettings::MAGNIFICATION("MAGNIFICATION");
      const QString StealthViewerSettings::AUTO_ATTACH_ON_SELECTION("AUTO_ATTACH_ON_SELECTION");
      const QString StealthViewerSettings::DISTANCE_UNIT("DISTANCE_UNIT");
      const QString StealthViewerSettings::ANGLE_UNIT("ANGLE_UNIT");

   const QString StealthViewerSettings::PREFERENCES_VISIBILITY_GROUP("PREFERENCES_VISIBILITY_GROUP");
      const QString StealthViewerSettings::SHOW_LABELS("SHOW_LABELS");
      const QString StealthViewerSettings::SHOW_LABELS_FOR_ENTITIES("SHOW_LABELS_FOR_ENTITIES");
      const QString StealthViewerSettings::SHOW_LABELS_FOR_BLIPS("SHOW_LABELS_FOR_BLIPS");
      const QString StealthViewerSettings::SHOW_LABELS_FOR_TRACKS("SHOW_LABELS_FOR_TRACKS");
      const QString StealthViewerSettings::LABEL_MAX_DISTANCE("LABEL_MAX_DISTANCE");
      const QString StealthViewerSettings::SHOW_PLATFORMS("SHOW_PLATFORMS");
      const QString StealthViewerSettings::SHOW_HUMANS("SHOW_HUMANS");
      const QString StealthViewerSettings::SHOW_TRACKS("SHOW_TRACKS");
      const QString StealthViewerSettings::SHOW_BLIPS("SHOW_BLIPS");
      const QString StealthViewerSettings::SHOW_BFG("SHOW_BATTLEFIELD_GRAPHICS");
      const QString StealthViewerSettings::SHOW_ENUM_PREFIX("SHOW_ENUM_");
      const QString StealthViewerSettings::BFG_CLOSE_TOPS("BATTLEFIELD_GRAPHICS_CLOSE_TOPS");

   const QString StealthViewerSettings::CONTROLS_CAMERA_GROUP("CONTROLS_CAMERA_GROUP");

   const QString StealthViewerSettings::CONTROLS_RECORD_GROUP("CONTROLS_RECORD_GROUP");
      const QString StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS("SHOW_ADVANCED_RECORD_OPTIONS");
      const QString StealthViewerSettings::RECORD_OUTPUT_FILE("RECORD_OUTPUT_FILE");
      const QString StealthViewerSettings::AUTO_KEY_FRAME("AUTO_KEY_FRAME");
      const QString StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL("AUTO_KEY_FRAME_INTERVAL");

   const QString StealthViewerSettings::CONTROLS_PLAYBACK_GROUP("CONTROLS_PLAYBACK_GROUP");
      const QString StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS("SHOW_ADVANCED_PLAYBACK_OPTIONS");
      const QString StealthViewerSettings::PLAYBACK_LOOP_CONTINUOUSLY("PLAYBACK_LOOP_CONTINUOUSLY");
      const QString StealthViewerSettings::PLAYBACK_INPUT_FILE("PLAYBACK_INPUT_FILE");
      const QString StealthViewerSettings::PLAYBACK_SPEED("PLAYBACK_SPEED");

   const QString StealthViewerSettings::VIEW_WINDOW_GROUP("VIEW_WINDOW_GROUP");
      const QString StealthViewerSettings::NEAR_CLIPPING_PLANE("NEAR_CLIPPING_PLANE");
      const QString StealthViewerSettings::FAR_CLIPPING_PLANE("FAR_CLIPPING_PLANE");
      const QString StealthViewerSettings::FOV_ASPECT_OR_HORIZONTAL("FOV_ASPECT_OR_HORIZONTAL");
      const QString StealthViewerSettings::FOV_ASPECT_RATIO("FOV_ASPECT_RATIO");
      const QString StealthViewerSettings::FOV_HORIZONTAL("FOV_HORIZONTAL");
      const QString StealthViewerSettings::FOV_VERTICAL_FOR_ASPECT("FOV_VERTICAL_FOR_ASPECT");
      const QString StealthViewerSettings::FOV_VERTICAL_FOR_HORIZONTAL("FOV_VERTICAL_FOR_HORIZONTAL");

      const QString StealthViewerSettings::ADDITIONAL_VIEW_ROTATION("ADDITIONAL_VIEW_ROTATION");
      const QString StealthViewerSettings::ADDITIONAL_VIEW_NAME("ADDITIONAL_VIEW_NAME");
      const QString StealthViewerSettings::ADDITIONAL_VIEW_TITLE("ADDITIONAL_VIEW_TITLE");

   /////////////////////////////////////////////////////////////////////
   StealthViewerSettings::StealthViewerSettings(const QString& applicationName)
   : QSettings(QSettings::IniFormat,
                QSettings::UserScope,
                StealthViewerSettings::ORGANIZATION,
                applicationName)
   , mNumConnections(0)
   , mIsLoadingFromIni(false)
   {
      ParseIniFile();
   }

   /////////////////////////////////////////////////////////////////////
   StealthViewerSettings::~StealthViewerSettings()
   {

   }

   /////////////////////////////////////////////////////////////////////
   bool StealthViewerSettings::AddConnection(const QString& name,
      const QString& mapResource, const QString& configResource, const QString& fedResource,
      const QString& fedex, const QString& federateName, const QString& ridFile,
      const QString& rtiStandard,
      const QString& connectionType, const QString& serverIPAddress,
      const QString& serverPort, const QString& serverGameName, const QString& serverGameVersion,
      const QString& disIPAddress, const unsigned int& disPort,
      bool disBroadcast,
      const unsigned char& disExerciseID, const unsigned short& disSiteID,
      const unsigned short& disApplicationID, const unsigned int& disMTU,
      const QString& actorXMLFile, bool isEditMode)
   {
      // CONNECTION values
      if (name.isEmpty() || connectionType.isEmpty())
      {
         std::string error = "Could not add the current connection [" + name.toStdString() + "] because it contained empty data.";
         LOG_ERROR(error);
         return false;
      }
      else if (connectionType.toStdString() == CONNECTIONTYPE_HLA &&
         (configResource.isEmpty() || fedResource.isEmpty() || fedex.isEmpty() || 
         federateName.isEmpty() || rtiStandard.isEmpty()))
      {
         std::string error = "Could not add the current HLA connection [" + name.toStdString() + "] because it contained empty data.";
         LOG_ERROR(error);
         return false;
      }
      else if (connectionType.toStdString() == CONNECTIONTYPE_CLIENTSERVER &&
         (serverIPAddress.isEmpty() || serverPort.isEmpty() || serverGameName.isEmpty() || serverGameVersion.isEmpty())) // Note - don't check name or version
      {
         std::string error = "Could not add the current Client Server connection [" + name.toStdString() + "] because it contained empty data.";
         LOG_ERROR(error);
         return false;
      }
      else if (connectionType.toStdString() == CONNECTIONTYPE_DIS &&
         (disIPAddress.isEmpty() || actorXMLFile.isEmpty() || disPort == 0))
      {
         std::string error = "Could not add the current DIS connection [" + name.toStdString() + "] because it contained empty data.";
         LOG_ERROR(error);
         return false;
      }


      if (!isEditMode && !mIsLoadingFromIni)
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

      if ((!ContainsConnection(name) || mIsLoadingFromIni) && !isEditMode)
      {
         QString groupName = StealthViewerSettings::CONNECTION + QString::number(mNumConnections);
         beginGroup(groupName);

            setValue(StealthViewerSettings::NAME,               name);
            setValue(StealthViewerSettings::MAP_RESOURCE,       mapResource);
            setValue(StealthViewerSettings::CONFIG_RESOURCE,    configResource);
            setValue(StealthViewerSettings::FED_RESOURCE,       fedResource);
            setValue(StealthViewerSettings::FEDEX,              fedex);
            setValue(StealthViewerSettings::FEDERATE_NAME,      federateName);
            setValue(StealthViewerSettings::RID_FILE,           ridFile);
            setValue(StealthViewerSettings::RTI_STANDARD_VERSION,rtiStandard);
            setValue(StealthViewerSettings::CONNECTION_TYPE,    connectionType);
            setValue(StealthViewerSettings::SERVER_IP_ADDRESS,  serverIPAddress);
            setValue(StealthViewerSettings::SERVER_PORT,        serverPort);
            setValue(StealthViewerSettings::SERVER_GAMENAME,    serverGameName);
            setValue(StealthViewerSettings::SERVER_GAMEVERSION, serverGameVersion);
            setValue(StealthViewerSettings::DIS_IP_ADDRESS,     disIPAddress);
            setValue(StealthViewerSettings::DIS_PORT,           disPort);
            setValue(StealthViewerSettings::DIS_BROADCAST,      disBroadcast);
            setValue(StealthViewerSettings::DIS_EXERCISE_ID,    disExerciseID);
            setValue(StealthViewerSettings::DIS_SITE_ID,        disSiteID);
            setValue(StealthViewerSettings::DIS_APPLICATION_ID, disApplicationID);
            setValue(StealthViewerSettings::DIS_MTU,            disMTU);
            setValue(StealthViewerSettings::DIS_ACTOR_XML_FILE, actorXMLFile);

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

               setValue(StealthViewerSettings::NAME,               name);
               setValue(StealthViewerSettings::MAP_RESOURCE,       mapResource);
               setValue(StealthViewerSettings::CONFIG_RESOURCE,    configResource);
               setValue(StealthViewerSettings::FED_RESOURCE,       fedResource);
               setValue(StealthViewerSettings::FEDEX,              fedex);
               setValue(StealthViewerSettings::FEDERATE_NAME,      federateName);
               setValue(StealthViewerSettings::RID_FILE,           ridFile);
               setValue(StealthViewerSettings::RTI_STANDARD_VERSION,rtiStandard);
               setValue(StealthViewerSettings::CONNECTION_TYPE,    connectionType);
               setValue(StealthViewerSettings::SERVER_IP_ADDRESS,  serverIPAddress);
               setValue(StealthViewerSettings::SERVER_PORT,        serverPort);
               setValue(StealthViewerSettings::SERVER_GAMENAME,    serverGameName);
               setValue(StealthViewerSettings::SERVER_GAMEVERSION, serverGameVersion);
               setValue(StealthViewerSettings::DIS_IP_ADDRESS,     disIPAddress);
               setValue(StealthViewerSettings::DIS_PORT,           disPort);
               setValue(StealthViewerSettings::DIS_BROADCAST,      disBroadcast);
               setValue(StealthViewerSettings::DIS_EXERCISE_ID,    disExerciseID);
               setValue(StealthViewerSettings::DIS_SITE_ID,        disSiteID);
               setValue(StealthViewerSettings::DIS_APPLICATION_ID, disApplicationID);
               setValue(StealthViewerSettings::DIS_MTU,            disMTU);
               setValue(StealthViewerSettings::DIS_ACTOR_XML_FILE, actorXMLFile);

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

   /////////////////////////////////////////////////////////////////////
   QStringList StealthViewerSettings::GetConnectionProperties(const QString& connectionName)
   {
      QStringList list;

      std::map<QString, unsigned int>::iterator itor = mConnectionNameMap.find(connectionName);
      if(itor == mConnectionNameMap.end())
         return list;

      QString group = StealthViewerSettings::CONNECTION + QString::number(itor->second);

      list = LoadConnectionProperties(group);

      return list;
   }

   /////////////////////////////////////////////////////////////////////
   QStringList StealthViewerSettings::GetConnectionNames() const
   {
      QStringList results;
      bool found = false;
      unsigned int count = 0;
      do
      {
         QString connection = StealthViewerSettings::CONNECTION + QString::number(count) +
            "/" + StealthViewerSettings::NAME;

         if (contains(connection))
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

   /////////////////////////////////////////////////////////////////////
   bool StealthViewerSettings::ContainsConnection(const QString& connectionName)
   {
      return (!GetConnectionProperties(connectionName).isEmpty());
   }

   /////////////////////////////////////////////////////////////////////
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
         AddConnection(list[0],
                       list[1],
                       list[2],
                       list[3],
                       list[4],
                       list[5],
                       list[6],
                       list[7],
                       list[8],
                       list[9],
                       list[10],
                       list[11],
                       list[12],
                       list[13],           //dis IP
                       list[14].toUInt(),  //DIS Port
                       list[15] == "true" ? true : false, //DIS Broadcast (bool)
                       list[16].toUShort(), //DIS exercise ID
                       list[17].toUShort(), //DIS Site ID
                       list[18].toUShort(), //DIS Application ID
                       list[19].toUInt(),   //DIS MTU
                       list[20]             //DIS ActorMapping file
         );
      }

      mIsLoadingFromIni = false;
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::RemoveConnection(const QString& connectionName)
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
                       connectionsToAdd[i][6], connectionsToAdd[i][7], connectionsToAdd[i][8],
                       connectionsToAdd[i][9],
                       connectionsToAdd[i][10], connectionsToAdd[i][11], connectionsToAdd[i][12],
                       connectionsToAdd[i][13], //DIS IP
                       connectionsToAdd[i][14].toInt(), //DIS port
                       connectionsToAdd[i][15] == "true" ? true : false, //DIS Broadcast (bool)
                       connectionsToAdd[i][16].toUInt(), //DIS exercize
                       connectionsToAdd[i][17].toUShort(), //DIS site ID
                       connectionsToAdd[i][18].toUShort(), //DIS Application ID
                       connectionsToAdd[i][19].toUInt(), //DIS MTU
                       connectionsToAdd[i][20] //DIS actor mapping file
         );
      }

      emit ItemDeleted(connectionName);
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::ClearAllSettings(bool clearFileAlso)
   {
      mConnectionNameMap.clear();
      mNumConnections = 0;

      if(clearFileAlso)
         clear();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WritePreferencesToFile(bool writeWindowState)
   {
      if (writeWindowState)
      {
         beginGroup(StealthViewerSettings::GENERAL_GROUP);

            setValue(StealthViewerSettings::DOCK_STATE,
               StealthViewerData::GetInstance().GetMainWindow()->saveState(StealthViewerSettings::WINDOW_DOCK_ID));

            setValue(StealthViewerSettings::WINDOW_GEOMETRY,
               StealthViewerData::GetInstance().GetMainWindow()->saveGeometry());

            setValue(StealthViewerSettings::AUTO_REFRESH_ENTITY_INFO,
               StealthViewerData::GetInstance().GetGeneralConfigObject().GetAutoRefreshEntityInfoWindow());

         endGroup();
      }
      // Low cyclomatic complexity
      WritePreferencesGeneralGroupToFile();
      WritePreferencesEnvironmentGroupToFile();
      WritePreferencesToolsGroupToFile();
      WritePreferencesVisibilityGroupToFile();

      WriteControlsRecordGroupToFile();
      WriteControlsPlaybackGroupToFile();

      WriteViewWindowGroupToFile();

      sync();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WritePreferencesGeneralGroupToFile()
   {
      StealthGM::PreferencesGeneralConfigObject& genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_GENERAL_GROUP);

         setValue(StealthViewerSettings::ATTACH_MODE, genConfig.GetAttachMode().GetName().c_str());
         setValue(StealthViewerSettings::ATTACH_POINT_NODE, genConfig.GetAttachPointNodeName().c_str());

         std::ostringstream ss;
         ss << genConfig.GetInitialAttachRotationHPR();
         setValue(StealthViewerSettings::ATTACH_ROTATION, ss.str().c_str());
         setValue(StealthViewerSettings::SHOULD_AUTO_ATTACH, genConfig.GetShouldAutoAttachToEntity());
         setValue(StealthViewerSettings::AUTO_ATTACH_TO_CALLSIGN, genConfig.GetAutoAttachEntityCallsign().c_str());


         setValue(StealthViewerSettings::CAMERA_COLLISION, genConfig.GetEnableCameraCollision());
         setValue(StealthViewerSettings::PERFORMANCE, genConfig.GetPerformanceMode().GetName().c_str());
         setValue(StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS, genConfig.GetShowAdvancedOptions());
         setValue(StealthViewerSettings::LOD_SCALE, genConfig.GetLODScale());
         setValue(StealthViewerSettings::RECONNECT_ON_STARTUP, genConfig.GetReconnectOnStartup());
         setValue(StealthViewerSettings::STARTUP_CONNECTION_NAME, genConfig.GetStartupConnectionName().c_str());

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WritePreferencesEnvironmentGroupToFile()
   {
      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_ENVIRONMENT_GROUP);

         setValue(StealthViewerSettings::USE_NETWORK_SETTINGS, envConfig.GetUseNetworkSettings());

         setValue(StealthViewerSettings::USE_THEMED_SETTINGS, envConfig.GetUseThemedSettings());
         //setValue(StealthViewerSettings::WEATHER_THEME, envConfig.GetWeatherTheme().GetName().c_str());
         //setValue(StealthViewerSettings::TIME_THEME, envConfig.GetTimeTheme().GetName().c_str());

         setValue(StealthViewerSettings::USE_CUSTOM_SETTINGS, envConfig.GetUseCustomSettings());
         setValue(StealthViewerSettings::CUSTOM_HOUR, envConfig.GetCustomHour());
         setValue(StealthViewerSettings::CUSTOM_MINUTE, envConfig.GetCustomMinute());
         setValue(StealthViewerSettings::CUSTOM_SECOND, envConfig.GetCustomSecond());
         //setValue(StealthViewerSettings::CUSTOM_VISIBILITY, envConfig.GetVisibility().GetName().c_str());
         //setValue(StealthViewerSettings::CUSTOM_CLOUD_COVER, envConfig.GetCloudCover().GetName().c_str());

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WritePreferencesToolsGroupToFile()
   {
      StealthGM::PreferencesToolsConfigObject& toolsObject =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_TOOLS_GROUP);

         setValue(StealthViewerSettings::COORDINATE_SYSTEM, toolsObject.GetCoordinateSystem().GetName().c_str());
         setValue(StealthViewerSettings::SHOW_COMPASS_360, toolsObject.GetShowCompass360());
         setValue(StealthViewerSettings::SHOW_BINOCULAR_IMAGE, toolsObject.GetShowBinocularImage());
         setValue(StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT, toolsObject.GetShowDistanceToObject());
         setValue(StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT, toolsObject.GetShowElevationOfObject());
         setValue(StealthViewerSettings::MAGNIFICATION, toolsObject.GetMagnification());
         setValue(StealthViewerSettings::AUTO_ATTACH_ON_SELECTION, toolsObject.GetAutoAttachOnSelection());
         setValue(StealthViewerSettings::DISTANCE_UNIT, toolsObject.GetLengthUnit().GetName().c_str());
         setValue(StealthViewerSettings::ANGLE_UNIT, toolsObject.GetAngleUnit().GetName().c_str());

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WritePreferencesVisibilityGroupToFile()
   {
      StealthGM::PreferencesVisibilityConfigObject& visObject =
         StealthViewerData::GetInstance().GetVisibilityConfigObject();

      beginGroup(StealthViewerSettings::PREFERENCES_VISIBILITY_GROUP);
         SimCore::Components::LabelOptions labelOptions = visObject.GetLabelOptions();
         SimCore::VisibilityOptions& visOptions = visObject.GetEntityOptions();
         SimCore::BasicVisibilityOptions basicOptions = visOptions.GetBasicOptions();

         setValue(StealthViewerSettings::SHOW_LABELS, labelOptions.ShowLabels());
         setValue(StealthViewerSettings::SHOW_LABELS_FOR_BLIPS, labelOptions.ShowLabelsForBlips());
         setValue(StealthViewerSettings::SHOW_LABELS_FOR_ENTITIES, labelOptions.ShowLabelsForEntities());
         setValue(StealthViewerSettings::SHOW_LABELS_FOR_TRACKS, labelOptions.ShowLabelsForPositionReports());
         setValue(StealthViewerSettings::LABEL_MAX_DISTANCE, labelOptions.GetMaxLabelDistance());

         setValue(StealthViewerSettings::SHOW_PLATFORMS, basicOptions.mPlatforms);
         setValue(StealthViewerSettings::SHOW_HUMANS, basicOptions.mDismountedInfantry);
         setValue(StealthViewerSettings::SHOW_TRACKS, basicOptions.mTracks);
         setValue(StealthViewerSettings::SHOW_BLIPS, basicOptions.mSensorBlips);
         setValue(StealthViewerSettings::SHOW_BFG, basicOptions.mBattlefieldGraphics);

         setValue(StealthViewerSettings::BFG_CLOSE_TOPS, visObject.GetBFGCloseTops());

         const std::vector<SimCore::Actors::BaseEntityActorProxy::ForceEnum*>& forces =
            SimCore::Actors::BaseEntityActorProxy::ForceEnum::EnumerateType();
         for (size_t i = 0; i < forces.size(); ++i)
         {
            SimCore::Actors::BaseEntityActorProxy::ForceEnum* curForce = forces[i];

            QString key = SHOW_ENUM_PREFIX + tr(curForce->GetName().c_str());
            setValue(key, basicOptions.IsEnumVisible(*curForce));
         }

         const std::vector<SimCore::Actors::BaseEntityActorProxy::DomainEnum*>& domains =
            SimCore::Actors::BaseEntityActorProxy::DomainEnum::EnumerateType();
         for (size_t i = 0; i < domains.size(); ++i)
         {
            SimCore::Actors::BaseEntityActorProxy::DomainEnum* curDomain = domains[i];

            QString key = SHOW_ENUM_PREFIX + tr(curDomain->GetName().c_str());
            setValue(key, basicOptions.IsEnumVisible(*curDomain));
         }
      endGroup();
   }


   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WriteControlsRecordGroupToFile()
   {
      StealthGM::ControlsRecordConfigObject& recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      beginGroup(StealthViewerSettings::CONTROLS_RECORD_GROUP);

         setValue(StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS, recordObject.GetShowAdvancedOptions());
         setValue(StealthViewerSettings::RECORD_OUTPUT_FILE, recordObject.GetOutputFilename().c_str());
         setValue(StealthViewerSettings::AUTO_KEY_FRAME, recordObject.GetAutoKeyFrame());
         setValue(StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL, recordObject.GetAutoKeyFrameInterval());

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WriteControlsPlaybackGroupToFile()
   {
      StealthGM::ControlsPlaybackConfigObject& pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      beginGroup(StealthViewerSettings::CONTROLS_PLAYBACK_GROUP);

         setValue(StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS, pbObject.GetShowAdvancedOptions());
         setValue(StealthViewerSettings::PLAYBACK_LOOP_CONTINUOUSLY, pbObject.GetLoopContinuously());
         setValue(StealthViewerSettings::PLAYBACK_INPUT_FILE, pbObject.GetInputFilename().c_str());
         setValue(StealthViewerSettings::PLAYBACK_SPEED, pbObject.GetPlaybackSpeed());

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::WriteViewWindowGroupToFile()
   {
      // remove all view settings.
      QStringList groups = childGroups();
      for(int i = 0; i < groups.size(); i++)
      {
         QString group = groups[i];
         if (group.left(StealthViewerSettings::VIEW_WINDOW_GROUP.size()) == StealthViewerSettings::VIEW_WINDOW_GROUP)
         {
            remove(group);
         }
      }

      // General Preferences
      StealthGM::ViewWindowConfigObject& viewConfig =
         StealthViewerData::GetInstance().GetViewWindowConfigObject();

      StealthGM::ViewWindowWrapper& mainViewWrapper = viewConfig.GetMainViewWindow();

      beginGroup(StealthViewerSettings::VIEW_WINDOW_GROUP);
         setValue(StealthViewerSettings::NEAR_CLIPPING_PLANE, viewConfig.GetNearClippingPlane());
         setValue(StealthViewerSettings::FAR_CLIPPING_PLANE, viewConfig.GetFarClippingPlane());

         setValue(StealthViewerSettings::FOV_ASPECT_OR_HORIZONTAL, mainViewWrapper.UseAspectRatioForFOV());
         setValue(StealthViewerSettings::FOV_ASPECT_RATIO, mainViewWrapper.GetFOVAspectRatio());
         setValue(StealthViewerSettings::FOV_HORIZONTAL, mainViewWrapper.GetFOVHorizontal());
         setValue(StealthViewerSettings::FOV_VERTICAL_FOR_ASPECT, mainViewWrapper.GetFOVVerticalForAspect());
         setValue(StealthViewerSettings::FOV_VERTICAL_FOR_HORIZONTAL, mainViewWrapper.GetFOVVerticalForHorizontal());

      endGroup();

      std::vector<StealthGM::ViewWindowWrapper*> viewWrappers;
      viewConfig.GetAllViewWindows(viewWrappers);

      for (unsigned i=0; i < viewWrappers.size(); ++i)
      {
         QString currentViewGroup = StealthViewerSettings::VIEW_WINDOW_GROUP + QString::number(i);
         StealthGM::ViewWindowWrapper& currViewWrapper = *viewWrappers[i];

         beginGroup(currentViewGroup);
            setValue(StealthViewerSettings::ADDITIONAL_VIEW_NAME, tr(currViewWrapper.GetName().c_str()));
            setValue(StealthViewerSettings::ADDITIONAL_VIEW_TITLE, tr(currViewWrapper.GetWindowTitle().c_str()));

            setValue(StealthViewerSettings::FOV_ASPECT_OR_HORIZONTAL, currViewWrapper.UseAspectRatioForFOV());
            setValue(StealthViewerSettings::FOV_ASPECT_RATIO, currViewWrapper.GetFOVAspectRatio());
            setValue(StealthViewerSettings::FOV_HORIZONTAL, currViewWrapper.GetFOVHorizontal());
            setValue(StealthViewerSettings::FOV_VERTICAL_FOR_ASPECT, currViewWrapper.GetFOVVerticalForAspect());
            setValue(StealthViewerSettings::FOV_VERTICAL_FOR_HORIZONTAL, currViewWrapper.GetFOVVerticalForHorizontal());

            dtCore::DeltaWin::PositionSize ps;
            ps = currViewWrapper.GetWindow().GetPosition();

            std::ostringstream ss;
            ss << ps.mX << " ";
            ss << ps.mY << " ";
            ss << ps.mWidth << " ";
            ss << ps.mHeight;

            setValue(StealthViewerSettings::WINDOW_GEOMETRY, tr(ss.str().c_str()));

            ss.str("");
            ss << currViewWrapper.GetAttachCameraRotation();
            setValue(StealthViewerSettings::ADDITIONAL_VIEW_ROTATION, ss.str().c_str());

         endGroup();
      }
//      // ugly hack to make the windows initialize before the settings are read.
//      dtCore::System::GetInstance().Step();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadPreferences()
   {
      LoadPreferencesGeneral();
      LoadPreferencesEnvironment();
      LoadPreferencesTools();
      LoadPreferencesVisibility();

      LoadControlsRecord();
      LoadControlsPlayback();
      LoadViewWindowGroup();

      // window state must be last so that additional views are created first.
      beginGroup(StealthViewerSettings::GENERAL_GROUP);

         if (contains(StealthViewerSettings::DOCK_STATE))
         {
            QByteArray state = value(StealthViewerSettings::DOCK_STATE).toByteArray();
            StealthViewerData::GetInstance().GetMainWindow()->restoreState(state, StealthViewerSettings::WINDOW_DOCK_ID);
         }

         if (contains(StealthViewerSettings::WINDOW_GEOMETRY))
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
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadPreferencesGeneral()
   {
      beginGroup(StealthViewerSettings::PREFERENCES_GENERAL_GROUP);

         StealthGM::PreferencesGeneralConfigObject& genConfig =
            StealthViewerData::GetInstance().GetGeneralConfigObject();

         if (contains(StealthViewerSettings::ATTACH_MODE))
         {
            QString savedValue =
               value(StealthViewerSettings::ATTACH_MODE).toString();

            genConfig.SetAttachMode(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::ATTACH_POINT_NODE))
         {
            QString savedValue =
               value(StealthViewerSettings::ATTACH_POINT_NODE).toString();

            genConfig.SetAttachPointNodeName(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::ATTACH_ROTATION))
         {
            std::string savedValue =
               value(StealthViewerSettings::ATTACH_ROTATION).toString().toStdString();
            std::istringstream iss;
            iss.str(savedValue);
            osg::Vec3 value;
            iss >> value.x();
            iss >> value.y();
            iss >> value.z();

            genConfig.SetInitialAttachRotationHPR(value);
         }

         if (contains(StealthViewerSettings::AUTO_ATTACH_TO_CALLSIGN))
         {
            QString savedValue =
               value(StealthViewerSettings::AUTO_ATTACH_TO_CALLSIGN).toString();

            genConfig.SetAutoAttachEntityCallsign(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::SHOULD_AUTO_ATTACH))
         {
            bool savedValue =
               value(StealthViewerSettings::SHOULD_AUTO_ATTACH).toBool();

            genConfig.SetShouldAutoAttachToEntity(savedValue);
         }

         if (contains(StealthViewerSettings::CAMERA_COLLISION))
         {
            bool savedValue = value(StealthViewerSettings::CAMERA_COLLISION).toBool();
            genConfig.SetCameraCollision(savedValue);
         }

         if (contains(StealthViewerSettings::PERFORMANCE))
         {
            QString savedValue = value(StealthViewerSettings::PERFORMANCE).toString();
            genConfig.SetPerformanceMode(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ADVANCED_GENERAL_OPTIONS).toBool();
            genConfig.SetShowAdvancedOptions(savedValue);
         }

         if (contains(StealthViewerSettings::LOD_SCALE))
         {
            float savedValue = float(value(StealthViewerSettings::LOD_SCALE).toDouble());
            genConfig.SetLODScale(savedValue);
         }

         bool connectValue = true;
         QString name;
         if (contains(StealthViewerSettings::RECONNECT_ON_STARTUP))
         {
            connectValue = value(StealthViewerSettings::RECONNECT_ON_STARTUP).toBool();
         }

         if (contains(StealthViewerSettings::STARTUP_CONNECTION_NAME))
         {
            name = value(StealthViewerSettings::STARTUP_CONNECTION_NAME).toString();
         }

         genConfig.SetReconnectOnStartup(connectValue, name.toStdString());

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadPreferencesEnvironment()
   {
      beginGroup(StealthViewerSettings::PREFERENCES_ENVIRONMENT_GROUP);

         StealthGM::PreferencesEnvironmentConfigObject& envConfig =
            StealthViewerData::GetInstance().GetEnvironmentConfigObject();

         if (contains(StealthViewerSettings::USE_NETWORK_SETTINGS))
         {
            bool savedValue = value(StealthViewerSettings::USE_NETWORK_SETTINGS).toBool();
            if(savedValue)
               envConfig.SetUseNetworkSettings();
         }

         if (contains(StealthViewerSettings::USE_THEMED_SETTINGS))
         {
            bool savedValue = value(StealthViewerSettings::USE_THEMED_SETTINGS).toBool();
            if(savedValue)
               envConfig.SetUseThemedSettings();
         }

         if (contains(StealthViewerSettings::WEATHER_THEME))
         {
            QString savedValue = value(StealthViewerSettings::WEATHER_THEME).toString();
            //envConfig.SetWeatherTheme(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::TIME_THEME))
         {
            QString savedValue = value(StealthViewerSettings::TIME_THEME).toString();
            //envConfig.SetTimeTheme(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::USE_CUSTOM_SETTINGS))
         {
            bool savedValue = value(StealthViewerSettings::USE_CUSTOM_SETTINGS).toBool();
            if(savedValue)
               envConfig.SetUseCustomSettings();
         }

         if (contains(StealthViewerSettings::CUSTOM_HOUR))
         {
            int savedValue = value(StealthViewerSettings::CUSTOM_HOUR).toInt();
            envConfig.SetCustomHour(savedValue);
         }

         if (contains(StealthViewerSettings::CUSTOM_MINUTE))
         {
            int savedValue = value(StealthViewerSettings::CUSTOM_MINUTE).toInt();
            envConfig.SetCustomMinute(savedValue);
         }

         if (contains(StealthViewerSettings::CUSTOM_SECOND))
         {
            int savedValue = value(StealthViewerSettings::CUSTOM_SECOND).toInt();
            envConfig.SetCustomSecond(savedValue);
         }

         if (contains(StealthViewerSettings::CUSTOM_VISIBILITY))
         {
            QString savedValue = value(StealthViewerSettings::CUSTOM_VISIBILITY).toString();
            //envConfig.SetVisibility(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::CUSTOM_CLOUD_COVER))
         {
            QString savedValue = value(StealthViewerSettings::CUSTOM_CLOUD_COVER).toString();
            //envConfig.SetCloudCover(savedValue.toStdString());
         }

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadPreferencesTools()
   {
      beginGroup(StealthViewerSettings::PREFERENCES_TOOLS_GROUP);

         StealthGM::PreferencesToolsConfigObject& toolsConfig =
            StealthViewerData::GetInstance().GetToolsConfigObject();

         if (contains(StealthViewerSettings::COORDINATE_SYSTEM))
         {
            QString savedValue = value(StealthViewerSettings::COORDINATE_SYSTEM).toString();
            StealthGM::PreferencesToolsConfigObject::CoordinateSystem* coordSystem =
               StealthGM::PreferencesToolsConfigObject::CoordinateSystem::GetValueForName(savedValue.toStdString());

            if (coordSystem != NULL)
               toolsConfig.SetCoordinateSystem(*coordSystem);
         }

         if (contains(StealthViewerSettings::SHOW_COMPASS_360))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_COMPASS_360).toBool();
            toolsConfig.SetShowCompass360(savedValue);
         }

         if (contains(StealthViewerSettings::SHOW_BINOCULAR_IMAGE))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_BINOCULAR_IMAGE).toBool();
            toolsConfig.SetShowBinocularImage(savedValue);
         }

         if (contains(StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_DISTANCE_TO_OBJECT).toBool();
            toolsConfig.SetShowDistanceToObject(savedValue);
         }

         if (contains(StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ELEVATION_OF_OBJECT).toBool();
            toolsConfig.SetShowElevationOfObject(savedValue);
         }

         if (contains(StealthViewerSettings::MAGNIFICATION))
         {
            float savedValue = float(value(StealthViewerSettings::MAGNIFICATION).toDouble());
            toolsConfig.SetMagnification(savedValue);
         }

         if (contains(StealthViewerSettings::AUTO_ATTACH_ON_SELECTION))
         {
            bool savedValue = value(StealthViewerSettings::AUTO_ATTACH_ON_SELECTION).toBool();
            toolsConfig.SetAutoAttachOnSelection(savedValue);
         }

         if (contains(StealthViewerSettings::DISTANCE_UNIT))
         {
            QString savedValue =
               value(StealthViewerSettings::DISTANCE_UNIT).toString();

            toolsConfig.SetLengthUnit(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::ANGLE_UNIT))
         {
            QString savedValue =
               value(StealthViewerSettings::ANGLE_UNIT).toString();

            toolsConfig.SetAngleUnit(savedValue.toStdString());
         }

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadPreferencesVisibility()
   {
      StealthGM::PreferencesVisibilityConfigObject& visObject =
         StealthViewerData::GetInstance().GetVisibilityConfigObject();

      beginGroup(PREFERENCES_VISIBILITY_GROUP);
         SimCore::Components::LabelOptions labelOptions = visObject.GetLabelOptions();
         SimCore::VisibilityOptions& visOptions = visObject.GetEntityOptions();
         SimCore::BasicVisibilityOptions basicOptions = visOptions.GetBasicOptions();

         if (contains(SHOW_LABELS))
         {
            labelOptions.SetShowLabels(value(SHOW_LABELS).toBool());
         }

         if (contains(SHOW_LABELS_FOR_BLIPS))
         {
            labelOptions.SetShowLabelsForBlips(value(SHOW_LABELS_FOR_BLIPS).toBool());
         }

         if (contains(SHOW_LABELS_FOR_ENTITIES))
         {
            labelOptions.SetShowLabelsForEntities(value(SHOW_LABELS_FOR_ENTITIES).toBool());
         }

         if (contains(SHOW_LABELS_FOR_TRACKS))
         {
            labelOptions.SetShowLabelsForPositionReports(value(SHOW_LABELS_FOR_TRACKS).toBool());
         }

         if (contains(LABEL_MAX_DISTANCE))
         {
            labelOptions.SetMaxLabelDistance(value(LABEL_MAX_DISTANCE).toDouble());
         }


         if (contains(SHOW_PLATFORMS))
         {
            basicOptions.mPlatforms = value(SHOW_PLATFORMS).toBool();
         }

         if (contains(SHOW_HUMANS))
         {
            basicOptions.mDismountedInfantry = value(SHOW_HUMANS).toBool();
         }

         if (contains(SHOW_TRACKS))
         {
            basicOptions.mTracks = value(SHOW_TRACKS).toBool();
         }

         if (contains(SHOW_BLIPS))
         {
            basicOptions.mSensorBlips = value(SHOW_BLIPS).toBool();
         }

         if (contains(SHOW_BFG))
         {
            basicOptions.mBattlefieldGraphics = value(SHOW_BFG).toBool();
         }

         if (contains(BFG_CLOSE_TOPS))
         {
            visObject.SetBFGCloseTops(value(BFG_CLOSE_TOPS).toBool());
         }

         const std::vector<SimCore::Actors::BaseEntityActorProxy::ForceEnum*>& forces =
            SimCore::Actors::BaseEntityActorProxy::ForceEnum::EnumerateType();
         for (size_t i = 0; i < forces.size(); ++i)
         {
            SimCore::Actors::BaseEntityActorProxy::ForceEnum* curForce = forces[i];

            QString key = SHOW_ENUM_PREFIX + tr(curForce->GetName().c_str());
            if (contains(key))
            {
               basicOptions.SetEnumVisible(*curForce, value(key).toBool());
            }
         }

         const std::vector<SimCore::Actors::BaseEntityActorProxy::DomainEnum*>& domains =
            SimCore::Actors::BaseEntityActorProxy::DomainEnum::EnumerateType();
         for (size_t i = 0; i < domains.size(); ++i)
         {
            SimCore::Actors::BaseEntityActorProxy::DomainEnum* curDomain = domains[i];

            QString key = SHOW_ENUM_PREFIX + tr(curDomain->GetName().c_str());
            if (contains(key))
            {
               basicOptions.SetEnumVisible(*curDomain, value(key).toBool());
            }
         }

         visOptions.SetBasicOptions(basicOptions);
         visObject.SetLabelOptions(labelOptions);

      endGroup();

   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadControlsRecord()
   {
      beginGroup(StealthViewerSettings::CONTROLS_RECORD_GROUP);

         StealthGM::ControlsRecordConfigObject& recordConfig =
            StealthViewerData::GetInstance().GetRecordConfigObject();

         if (contains(StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ADVANCED_RECORD_OPTIONS).toBool();
            recordConfig.SetShowAdvancedOptions(savedValue);
         }

         if (contains(StealthViewerSettings::RECORD_OUTPUT_FILE))
         {
            QString savedValue = value(StealthViewerSettings::RECORD_OUTPUT_FILE).toString();
            recordConfig.SetOutputFilename(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::AUTO_KEY_FRAME))
         {
            bool savedValue = value(StealthViewerSettings::AUTO_KEY_FRAME).toBool();
            recordConfig.SetAutoKeyFrame(savedValue);
         }

         if (contains(StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL))
         {
            int savedValue = value(StealthViewerSettings::AUTO_KEY_FRAME_INTERVAL).toInt();
            recordConfig.SetAutoKeyFrameInterval(savedValue);
         }

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadControlsPlayback()
   {
      beginGroup(StealthViewerSettings::CONTROLS_PLAYBACK_GROUP);

         StealthGM::ControlsPlaybackConfigObject& playbackConfig =
            StealthViewerData::GetInstance().GetPlaybackConfigObject();

         if (contains(StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS))
         {
            bool savedValue = value(StealthViewerSettings::SHOW_ADVANCED_PLAYBACK_OPTIONS).toBool();
            playbackConfig.SetShowAdvancedOptions(savedValue);
         }

         if (contains(StealthViewerSettings::PLAYBACK_LOOP_CONTINUOUSLY))
         {
            bool savedValue = value(StealthViewerSettings::PLAYBACK_LOOP_CONTINUOUSLY).toBool();
            playbackConfig.SetLoopContinuously(savedValue);
         }

         if (contains(StealthViewerSettings::PLAYBACK_INPUT_FILE))
         {
            QString savedValue = value(StealthViewerSettings::PLAYBACK_INPUT_FILE).toString();
            playbackConfig.SetInputFilename(savedValue.toStdString());
         }

         if (contains(StealthViewerSettings::PLAYBACK_SPEED))
         {
            float savedValue = float(value(StealthViewerSettings::PLAYBACK_SPEED).toDouble());
            playbackConfig.SetPlaybackSpeed(savedValue);
         }

      endGroup();
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadFOVSettings(StealthGM::ViewWindowWrapper& viewWrapper)
   {
      if (contains(StealthViewerSettings::FOV_ASPECT_OR_HORIZONTAL))
      {
         bool savedValue =
            value(StealthViewerSettings::FOV_ASPECT_OR_HORIZONTAL).toBool();

         viewWrapper.SetUseAspectRatioForFOV(savedValue);
      }

      if (contains(StealthViewerSettings::FOV_ASPECT_RATIO))
      {
         QString savedValue =
            value(StealthViewerSettings::FOV_ASPECT_RATIO).toString();

         viewWrapper.SetFOVAspectRatio(savedValue.toFloat());
      }

      if (contains(StealthViewerSettings::FOV_HORIZONTAL))
      {
         QString savedValue =
            value(StealthViewerSettings::FOV_HORIZONTAL).toString();

         viewWrapper.SetFOVHorizontal(savedValue.toFloat());
      }

      if (contains(StealthViewerSettings::FOV_VERTICAL_FOR_ASPECT))
      {
         QString savedValue =
            value(StealthViewerSettings::FOV_VERTICAL_FOR_ASPECT).toString();

         viewWrapper.SetFOVVerticalForAspect(savedValue.toFloat());
      }

      if (contains(StealthViewerSettings::FOV_VERTICAL_FOR_HORIZONTAL))
      {
         QString savedValue =
            value(StealthViewerSettings::FOV_VERTICAL_FOR_HORIZONTAL).toString();

         viewWrapper.SetFOVVerticalForHorizontal(savedValue.toFloat());
      }

   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::LoadViewWindowGroup()
   {
      // General Preferences
      StealthGM::ViewWindowConfigObject& viewConfig =
         StealthViewerData::GetInstance().GetViewWindowConfigObject();

      StealthGM::ViewWindowWrapper& mainViewWrapper = viewConfig.GetMainViewWindow();

      beginGroup(StealthViewerSettings::VIEW_WINDOW_GROUP);
      if (contains(StealthViewerSettings::NEAR_CLIPPING_PLANE))
      {
         double savedValue =
            value(StealthViewerSettings::NEAR_CLIPPING_PLANE).toDouble();
         viewConfig.SetNearClippingPlane(savedValue);
      }

      if (contains(StealthViewerSettings::FAR_CLIPPING_PLANE))
      {
         double savedValue =
            value(StealthViewerSettings::FAR_CLIPPING_PLANE).toDouble();
         viewConfig.SetFarClippingPlane(savedValue);
      }

      LoadFOVSettings(mainViewWrapper);
      endGroup();

      for (unsigned i=0; i < 100; ++i)
      {
         QString currentViewGroup = StealthViewerSettings::VIEW_WINDOW_GROUP + QString::number(i);
         if (childGroups().contains(currentViewGroup))
         {
            beginGroup(currentViewGroup);
            std::string newViewName;
            if (contains(StealthViewerSettings::ADDITIONAL_VIEW_NAME))
            {
               newViewName =
                  value(StealthViewerSettings::ADDITIONAL_VIEW_NAME).toString().toStdString();
            }

            dtCore::RefPtr<StealthGM::ViewWindowWrapper> newViewWrapper =
               StealthViewerData::GetInstance().GetMainWindow()->GetViewDockWidget().CreateNewViewWindow(newViewName);

            if (contains(StealthViewerSettings::ADDITIONAL_VIEW_TITLE))
            {
               std::string savedValue =
                  value(StealthViewerSettings::ADDITIONAL_VIEW_TITLE).toString().toStdString();

               newViewWrapper->SetWindowTitle(savedValue);
            }

            if (contains(StealthViewerSettings::WINDOW_GEOMETRY))
            {
               std::string savedValue = value(StealthViewerSettings::WINDOW_GEOMETRY).toString().toStdString();

               dtCore::DeltaWin::PositionSize ps;
               std::istringstream iss;
               iss.str(savedValue);
               iss >> ps.mX;
               iss >> ps.mY;
               iss >> ps.mWidth;
               iss >> ps.mHeight;

               if (ps.mWidth < 20)
               {
                  ps.mWidth = 20;
               }
               if (ps.mHeight < 20)
               {
                  ps.mHeight = 20;
               }

               newViewWrapper->GetWindow().SetPosition(ps);
            }

            LoadFOVSettings(*newViewWrapper);

            if (contains(StealthViewerSettings::ADDITIONAL_VIEW_ROTATION))
            {
               std::string savedValue =
                  value(StealthViewerSettings::ADDITIONAL_VIEW_ROTATION).toString().toStdString();
               std::istringstream iss;
               iss.str(savedValue);
               osg::Vec3 value;
               iss >> value.x();
               iss >> value.y();
               iss >> value.z();
               newViewWrapper->SetAttachCameraRotation(value);
            }

            newViewWrapper->SetAttachToCamera(viewConfig.GetMainViewWindow().GetView().GetCamera());

            viewConfig.AddViewWindow(*newViewWrapper);
            endGroup();
         }
      }
   }

   /////////////////////////////////////////////////////////////////////
   void StealthViewerSettings::RemovePreferences(QStringList& groups)
   {
      for(int i = 0; i < groups.size(); i++)
      {
         QString group = groups[i];

         //if(group == StealthViewerSettings::GENERAL_GROUP                 ||
         //   group == StealthViewerSettings::PREFERENCES_GENERAL_GROUP     ||
         //   group == StealthViewerSettings::PREFERENCES_ENVIRONMENT_GROUP ||
         //   group == StealthViewerSettings::PREFERENCES_TOOLS_GROUP       ||
         //   group == StealthViewerSettings::CONTROLS_CAMERA_GROUP         ||
         //   group == StealthViewerSettings::CONTROLS_RECORD_GROUP         ||
         //   group == StealthViewerSettings::CONTROLS_PLAYBACK_GROUP       ||
         //   group.left(StealthViewerSettings::VIEW_WINDOW_GROUP.size()) == StealthViewerSettings::VIEW_WINDOW_GROUP)
         QString connectionStr(StealthViewerSettings::CONNECTION);
         if (!group.startsWith(connectionStr)) 
         {
            groups.erase(groups.begin() + i);
            i = 0;
         }
      }
   }

   /////////////////////////////////////////////////////////////////////
   QStringList StealthViewerSettings::LoadConnectionProperties(const QString& group)
   {
      QStringList props;

      beginGroup(group);

         if (contains(StealthViewerSettings::NAME))
            props.push_back(value(StealthViewerSettings::NAME).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::MAP_RESOURCE))
            props.push_back(value(StealthViewerSettings::MAP_RESOURCE).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::CONFIG_RESOURCE))
            props.push_back(value(StealthViewerSettings::CONFIG_RESOURCE).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::FED_RESOURCE))
            props.push_back(value(StealthViewerSettings::FED_RESOURCE).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::FEDEX))
            props.push_back(value(StealthViewerSettings::FEDEX).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::FEDERATE_NAME))
            props.push_back(value(StealthViewerSettings::FEDERATE_NAME).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::RID_FILE))
            props.push_back(value(StealthViewerSettings::RID_FILE).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::RTI_STANDARD_VERSION))
            props.push_back(value(StealthViewerSettings::RTI_STANDARD_VERSION).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::CONNECTION_TYPE))
            props.push_back(value(StealthViewerSettings::CONNECTION_TYPE).toString());
         else
         {
            QString hlaString(tr(CONNECTIONTYPE_HLA.Get().c_str()));
            props.push_back(hlaString); // Default to HLA for backwards compatibility
         }

         if (contains(StealthViewerSettings::SERVER_IP_ADDRESS))
            props.push_back(value(StealthViewerSettings::SERVER_IP_ADDRESS).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::SERVER_PORT))
            props.push_back(value(StealthViewerSettings::SERVER_PORT).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::SERVER_GAMENAME))
            props.push_back(value(StealthViewerSettings::SERVER_GAMENAME).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::SERVER_GAMEVERSION))
            props.push_back(value(StealthViewerSettings::SERVER_GAMEVERSION).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_IP_ADDRESS))
            props.push_back(value(StealthViewerSettings::DIS_IP_ADDRESS).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_PORT))
            props.push_back(value(StealthViewerSettings::DIS_PORT).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_BROADCAST))
            props.push_back(value(StealthViewerSettings::DIS_BROADCAST).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_EXERCISE_ID))
            props.push_back(value(StealthViewerSettings::DIS_EXERCISE_ID).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_SITE_ID))
            props.push_back(value(StealthViewerSettings::DIS_SITE_ID).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_APPLICATION_ID))
            props.push_back(value(StealthViewerSettings::DIS_APPLICATION_ID).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_MTU))
            props.push_back(value(StealthViewerSettings::DIS_MTU).toString());
         else
            props.push_back(tr(""));

         if (contains(StealthViewerSettings::DIS_ACTOR_XML_FILE))
            props.push_back(value(StealthViewerSettings::DIS_ACTOR_XML_FILE).toString());
         else
            props.push_back(tr(""));

      endGroup();

      return props;
   }
}
