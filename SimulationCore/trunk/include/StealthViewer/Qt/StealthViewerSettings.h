/* -*-c++-*-
* Stealth Viewer - StealthViewerSettings (.h&  .cpp) - Using 'The MIT License'
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
* @author David Guthrie, Eddie Johnson, Curtiss Murphy
*/
#ifndef STEALTH_VIEWER_SETTINGS
#define STEALTH_VIEWER_SETTINGS

#include <QtCore/QSettings>
#include <dtUtil/refstring.h>

namespace StealthGM
{
   class ViewWindowWrapper;
}

namespace StealthQt
{
   class StealthViewerSettings : public QSettings
   {
      Q_OBJECT

      public:
         // public strings for the properties
         static const dtUtil::RefString CONNECTIONTYPE_NONE;
         static const dtUtil::RefString CONNECTIONTYPE_HLA;
         static const dtUtil::RefString CONNECTIONTYPE_CLIENTSERVER;
         static const dtUtil::RefString CONNECTIONTYPE_DIS;

         static const QString ORGANIZATION;
         static const QString APPLICATION;

         static const QString CONNECTION;
         static const QString NAME;
         static const QString MAP_RESOURCE;
         static const QString CONFIG_RESOURCE;
         static const QString FED_RESOURCE;
         static const QString FEDEX;
         static const QString FEDERATE_NAME;
         static const QString RID_FILE;
         static const QString RTI_STANDARD_VERSION;
         static const QString CONNECTION_TYPE;
         static const QString SERVER_IP_ADDRESS;
         static const QString SERVER_PORT;
         static const QString SERVER_GAMENAME;
         static const QString SERVER_GAMEVERSION;
         static const QString DIS_IP_ADDRESS;
         static const QString DIS_PORT;
         static const QString DIS_BROADCAST;
         static const QString DIS_EXERCISE_ID;
         static const QString DIS_SITE_ID;
         static const QString DIS_APPLICATION_ID;
         static const QString DIS_MTU;
         static const QString DIS_ACTOR_XML_FILE;

         static const QString GENERAL_GROUP;
            static const int WINDOW_DOCK_ID = 0;
            static const QString DOCK_STATE;
            static const QString WINDOW_GEOMETRY;
            static const QString AUTO_REFRESH_ENTITY_INFO;

         static const QString PREFERENCES_GENERAL_GROUP;
            static const QString ATTACH_MODE;
            static const QString ATTACH_POINT_NODE;
            static const QString ATTACH_ROTATION;
            static const QString AUTO_ATTACH_TO_CALLSIGN;
            static const QString SHOULD_AUTO_ATTACH;
            static const QString CAMERA_COLLISION;
            static const QString PERFORMANCE;
            static const QString SHOW_ADVANCED_GENERAL_OPTIONS;
            static const QString LOD_SCALE;
            static const QString RECONNECT_ON_STARTUP;
            static const QString STARTUP_CONNECTION_NAME;

         static const QString PREFERENCES_ENVIRONMENT_GROUP;
            static const QString USE_NETWORK_SETTINGS;
            static const QString USE_THEMED_SETTINGS;
               static const QString WEATHER_THEME;
               static const QString TIME_THEME;
            static const QString USE_CUSTOM_SETTINGS;
               static const QString CUSTOM_HOUR;
               static const QString CUSTOM_MINUTE;
               static const QString CUSTOM_SECOND;
               static const QString CUSTOM_VISIBILITY;
               static const QString CUSTOM_CLOUD_COVER;

         static const QString PREFERENCES_TOOLS_GROUP;
            static const QString COORDINATE_SYSTEM;
            static const QString SHOW_COMPASS_360;
            static const QString SHOW_BINOCULAR_IMAGE;
            static const QString SHOW_DISTANCE_TO_OBJECT;
            static const QString SHOW_ELEVATION_OF_OBJECT;
            static const QString MAGNIFICATION;
            static const QString AUTO_ATTACH_ON_SELECTION;
            static const QString DISTANCE_UNIT;
            static const QString ANGLE_UNIT;

         static const QString PREFERENCES_VISIBILITY_GROUP;
            static const QString SHOW_LABELS;
            static const QString SHOW_LABELS_FOR_ENTITIES;
            static const QString SHOW_LABELS_FOR_BLIPS;
            static const QString SHOW_LABELS_FOR_TRACKS;
            static const QString LABEL_MAX_DISTANCE;

            static const QString SHOW_PLATFORMS;
            static const QString SHOW_HUMANS;
            static const QString SHOW_TRACKS;
            static const QString SHOW_BLIPS;
            static const QString SHOW_BFG;
            static const QString SHOW_ENUM_PREFIX;
            /// Close the tops on solid Battlefield graphics objects
            static const QString BFG_CLOSE_TOPS;

         static const QString CONTROLS_CAMERA_GROUP;

         static const QString CONTROLS_RECORD_GROUP;
            static const QString SHOW_ADVANCED_RECORD_OPTIONS;
            static const QString RECORD_OUTPUT_FILE;
            static const QString AUTO_KEY_FRAME;
            static const QString AUTO_KEY_FRAME_INTERVAL;

         static const QString CONTROLS_PLAYBACK_GROUP;
            static const QString SHOW_ADVANCED_PLAYBACK_OPTIONS;
            static const QString PLAYBACK_LOOP_CONTINUOUSLY;
            static const QString PLAYBACK_INPUT_FILE;
            static const QString PLAYBACK_SPEED;

         static const QString VIEW_WINDOW_GROUP;
            static const QString NEAR_CLIPPING_PLANE;
            static const QString FAR_CLIPPING_PLANE;
            static const QString FOV_ASPECT_OR_HORIZONTAL;
            static const QString FOV_ASPECT_RATIO;
            static const QString FOV_HORIZONTAL;
            static const QString FOV_VERTICAL_FOR_ASPECT;
            static const QString FOV_VERTICAL_FOR_HORIZONTAL;
            static const QString ADDITIONAL_VIEW_ROTATION;
            static const QString ADDITIONAL_VIEW_NAME;
            static const QString ADDITIONAL_VIEW_TITLE;

         /// Constructor
         StealthViewerSettings(const QString& applicationName = StealthViewerSettings::APPLICATION);

         /// Destructor
         virtual ~StealthViewerSettings();

         /**
          * Adds a new connection to the list
          * @param name The connection name
          * @param mapName The name of the map
          * @param configResource The configuration file
          * @param fedResource The federation file
          * @param fedex The name of the federation
          * @param federateName The name of the federate
          * @param ridFile The name of the rid file to use
          * @param connectionType Should be either 'HLA' or 'Client-Server'.
          * @param serverIPAddress The IP Address of the server to connect to in Client Server mode
          * @param serverPort The port of the server to connect to in Client Server mode
          * @param isEditMode True if you are editing an existing connection
          * @return True on success
          */
         bool AddConnection(const QString& name,
            const QString& mapResource,
            const QString& configResource,
            const QString& fedResource,
            const QString& fedex,
            const QString& federateName,
            const QString& ridFile,
            const QString& rtiStandard,
            const QString& connectionType,
            const QString& serverIPAddress,
            const QString& serverPort,
            const QString& serverGameName,
            const QString& serverGameVersion,
            const QString& disIPAddress, //13
            const unsigned int& disPort,
            bool disBroadcast,
            const unsigned char& disExerciseID,
            const unsigned short& disSiteID,
            const unsigned short& disApplicationID,
            const unsigned int& disMTU,
            const QString& actorXMLFile,
            bool isEditMode = false);

         /**
          * Returns a string list of the properties in the following order:
          * name, map resource, config resource, fed resource, fedex, federateName
          * If not found, the list will be empty
          * @return a list of the properties
          */
         QStringList GetConnectionProperties(const QString& connectionName);

         /**
          * Returns a list of all the connection names stored
          * If none are stored, the list will be empty
          * @return a QStringList of names
          */
         QStringList GetConnectionNames() const;

         /**
          * Returns true if a connection by this name already exists
          * @return true if the connection exists, false if it doesn't
          */
         bool ContainsConnection(const QString& connectionName);

         /**
          * Deletes a connection from the ini file
          */
         void RemoveConnection(const QString& connectionName);

         /**
          * Returns the number of connections created
          * @return mNumConnections
          */
         unsigned int GetNumConnections() const { return mNumConnections; }

         /**
          * Clears all settings stored on the class
          * @param True to clear settings in the file as well
          */
         void ClearAllSettings(bool clearFileAlso = false);

         /**
          * Writes all preferences to the output file
          * The parameter only exists to allow unit testing without a main window instance.
          * @param writeWindowState true if the windowing state should be written as well.  false if not.
          */
         void WritePreferencesToFile(bool writeWindowState = true);

         /**
          * Loads the preferences from the input file
          */
         void LoadPreferences();

      signals:

         /**
          * Sent when an item is deleted
          * @param The name of the item deleted
          */
         void ItemDeleted(QString name);

      protected:

         /// Private helper method to parse the ini file on startup
         /// Protected so it can be used in unit test subclasses
         void ParseIniFile();

      private:

         /**
          * Writes the Preferences General group to the output file
          */
         void WritePreferencesGeneralGroupToFile();

         /**
          * Writes the Preferences Environment group to the output file
          */
         void WritePreferencesEnvironmentGroupToFile();

         /**
          * Writes the Preferences General group to the output file
          */
         void WritePreferencesToolsGroupToFile();

         /// Writes the visibility options to the output file.
         void WritePreferencesVisibilityGroupToFile();

         /**
          * Writes the Controls Record group to the output file
          */
         void WriteControlsRecordGroupToFile();

         /**
          * Writes the Controls Playback group to the output file
          */
         void WriteControlsPlaybackGroupToFile();

         /// Writes the view/window configuration to a file.
         void WriteViewWindowGroupToFile();

         /**
          * Loads the General preferences
          */
         void LoadPreferencesGeneral();

         /**
          * Loads the environment preferences
          */
         void LoadPreferencesEnvironment();

         /**
          * Loads the tools preferences
          */
         void LoadPreferencesTools();

         /**
          * Loads the visibility preferences
          */
         void LoadPreferencesVisibility();

         /**
          * Loads the controls record
          */
         void LoadControlsRecord();

         /**
          * Loads the controls playback
          */
         void LoadControlsPlayback();

         /// Loads the view/window group settings from file.
         void LoadViewWindowGroup();

         /// Load the FOV settings into the view wrapper.
         void LoadFOVSettings(StealthGM::ViewWindowWrapper& viewWrapper);

         /**
          * Removes preferences from child groups
          */
         void RemovePreferences(QStringList& groups);

         /**
          * Reads connection properties from a file and fills out a list
          * @param group The group to read from
          * @return the list
          */
         QStringList LoadConnectionProperties(const QString& group);

         std::map<QString, unsigned int> mConnectionNameMap;
         unsigned int mNumConnections;
         bool mIsLoadingFromIni;
   };
}

#endif
