/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#ifndef STEALTH_VIEWER_SETTINGS
#define STEALTH_VIEWER_SETTINGS

#include <QtCore/QSettings>

namespace StealthQt
{
   class StealthViewerSettings : public QSettings
   {
      Q_OBJECT

      public:

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

         static const QString GENERAL_GROUP;
            static const int WINDOW_DOCK_ID = 0;
            static const QString DOCK_STATE;
            static const QString WINDOW_GEOMETRY;
            static const QString AUTO_REFRESH_ENTITY_INFO;

         static const QString PREFERENCES_GENERAL_GROUP;
            static const QString ATTACH_MODE;
            static const QString CAMERA_COLLISION;
            static const QString PERFORMANCE;
            static const QString SHOW_ADVANCED_GENERAL_OPTIONS;
            static const QString LOD_SCALE;
            static const QString NEAR_CLIPPING_PLANE;
            static const QString FAR_CLIPPING_PLANE;
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
            static const QString SHOW_BINOCULAR_IMAGE;
            static const QString SHOW_DISTANCE_TO_OBJECT;
            static const QString SHOW_ELEVATION_OF_OBJECT;
            static const QString MAGNIFICATION;
            static const QString AUTO_ATTACH_ON_SELECTION;

         static const QString CONTROLS_CAMERA_GROUP;
         
         static const QString CONTROLS_RECORD_GROUP;
            static const QString SHOW_ADVANCED_RECORD_OPTIONS;
            static const QString RECORD_OUTPUT_FILE;
            static const QString AUTO_KEY_FRAME;
            static const QString AUTO_KEY_FRAME_INTERVAL;

         static const QString CONTROLS_PLAYBACK_GROUP;
            static const QString SHOW_ADVANCED_PLAYBACK_OPTIONS;
            static const QString PLAYBACK_INPUT_FILE;
            static const QString PLAYBACK_SPEED;

         /// Constructor
         StealthViewerSettings(const QString &applicationName = StealthViewerSettings::APPLICATION);

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
          * @param isEditMode True if you are editing an existing connection
          * @return True on success
          */
         bool AddConnection(const QString &name, 
                            const QString &mapResource, 
                            const QString &configResource, 
                            const QString &fedResource, 
                            const QString &fedex, 
                            const QString &federateName, 
                            const QString &ridFile, 
                            bool isEditMode = false);

         /**
          * Returns a string list of the properties in the following order:
          * name, map resource, config resource, fed resource, fedex, federateName
          * If not found, the list will be empty
          * @return a list of the properties
          */
         QStringList GetConnectionProperties(const QString &connectionName);

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
         bool ContainsConnection(const QString &connectionName);

         /**
          * Deletes a connection from the ini file
          */
         void RemoveConnection(const QString &connectionName);

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
          */
         void WritePreferencesToFile();

         /**
          * Loads the preferences from ther input file
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

         /**
          * Writes the Controls Record group to the output file
          */
         void WriteControlsRecordGroupToFile();

         /**
          * Writes the Controls Playback group to the output file
          */
         void WriteControlsPlaybackGroupToFile();

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
          * Loads the controls record
          */
         void LoadControlsRecord();

         /**
          * Loads the controls playback
          */
         void LoadControlsPlayback();

         /**
          * Removes preferences from child groups
          */
         void RemovePreferences(QStringList &groups);

         /**
          * Reads connection properties from a file and fills out a list
          * @param group The group to read from
          * @return the list
          */
         QStringList LoadConnectionProperties(const QString &group);

         std::map<QString, unsigned int> mConnectionNameMap;
         unsigned int mNumConnections;
   };
}

#endif
