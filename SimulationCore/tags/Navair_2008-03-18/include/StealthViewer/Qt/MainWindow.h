/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 * @author David Guthrie
 */
#ifndef DELTA_MAIN_WINDOW
#define DELTA_MAIN_WINDOW

#include <QtGui/QMainWindow>
#include <QtCore/QTimer>
#include <dtCore/refptr.h>
#include <dtCore/sigslot.h>
#include <dtGame/gamemanager.h>

namespace dtGame
{
   class GameApplication;
   class LogKeyframe;
}

namespace dtQt
{
   class OSGAdapterWidget;
}

namespace Ui
{
   class MainWindow;
}

class QCloseEvent;
class QDateTime;
class QTableWidgetItem;
class QDoubleValidator;
class QListWidgetItem;

namespace StealthQt 
{
   
   /**
    * This class is the main window of the application.  It contains the menu bar,
    * toolbar, statusbar, and main UI interface.
    */
   class MainWindow : public QMainWindow, public sigslot::has_slots<>
   {
      Q_OBJECT

      public:

         /**
          * Constructor
          * @param app The game application to render.
          */
         MainWindow(int appArgc, char* appArgv[], const std::string& appLibName);

         /// Destructor
         virtual ~MainWindow();

         /**
          * Filters events
          */
         virtual bool eventFilter(QObject *object, QEvent *event);

         /**
          * Slot called from dtGame when key frames are received
          * @param keyFrames a list of the key frames
          */
         void RecordKeyFrameSlot(const std::vector<dtGame::LogKeyframe> &keyFrames);
         
         /**
          * Slot called from dtGame when key frames are received
          * @param a list of the key frames
          */
         void PlaybackKeyFrameSlot(const std::vector<dtGame::LogKeyframe> &keyFrames);

      public slots:

         /**
          * Starts a new wait cursor.  You MUST call endWaitCursor() for each 
          * startWaitCursor().
          *
          * @Note - This behavior is extremely trivial, but is pulled to mainWindow for 
          * future expansion
          */
         void StartWaitCursor();

         /** 
          * Ends a previously started wait cursor.  You must call this for each startWaitCursor().
          *
          * @Note - This behavior is extremely trivial, but is pulled to mainWindow for 
          * future expansion
          */
         void EndWaitCursor();

         ///////////////////////////////////////////////////////////////////////
         // CONTROLS WINDOW
         ///////////////////////////////////////////////////////////////////////

         ///////////////////////////////////////////////////////////////////////
         // Camera Tab
         ///////////////////////////////////////////////////////////////////////

         ///////////////////////////////////////////////////////////////////////
         // Record Tab
         ///////////////////////////////////////////////////////////////////////
         /// Called when the start button is clicked for recording
         void OnRecordStartButtonClicked(bool checked = false);

         /// Called when the Record file tool button is clicked
         void OnRecordFileToolButtonClicked(bool checked = false);

         /// Called when the Show Advanced options box is changed
         void OnShowAdvancedRecordOptionsChanged(int state);

         /// Called when the add time marker button is clicked
         void OnAddTimeMarkerClicked(bool checked = false);

         /// Called when the automatic time marker check box is changed
         void OnAutoTimeMarkerCheckBoxChanged(int state);

         /// Called when the auto interval spin box is changed
         void OnAutoTimeMarkerSpinBoxChanged(int value);

         ///////////////////////////////////////////////////////////////////////
         // Playback Tab
         ///////////////////////////////////////////////////////////////////////
         /// Called when switch to playback mode is clicked
         void OnSwitchToPlaybackModeButtonClicked(bool checked = false);

         /// Called when the output file tool button is clicked
         void OnPlaybackFileToolButtonClicked(bool checked = false);

         /// Called when the restart button is clicked
         void OnPlaybackRestartButtonClicked(bool checked = false);

         /// Called when the jump to previous TM button is clicked
         void OnPlaybackJumpToPrevTimeMarkerButtonClicked(bool checked = false);

         /// Called when the play button is clicked
         void OnPlaybackPlayButtonClicked(bool checked = false);

         /// Called when jump to next TM button is clicked
         void OnPlaybackJumpToNextTimeMarkerButtonClicked(bool checked = false);

         /// Called when the Show Advanced options box is changed
         void OnShowAdvancedPlaybackOptionsChanged(int state);

         /// Called when playback speed is adjusted
         void OnPlaybackSpeedChanged(const QString &newText);

         /// Called when the jump to TM button is clicked
         void OnPlaybackJumpToTimeMarkerButtonClicked(bool checked = false);

         /// Called when the jump to TM button is clicked
         void OnPlaybackJumpToTimeMarkerButtonClicked(const QString &itemName);

         /// Called when a time marker is selected
         void OnPlaybackTimeMarkerSelected(const QString &text);
         ///////////////////////////////////////////////////////////////////////

         ///////////////////////////////////////////////////////////////////////
         // Search Tab
         ///////////////////////////////////////////////////////////////////////

         // Called when the search is initiated
         void OnEntitySearchSearchButtonClicked(bool checked = false);

         // Called when the attach button is clicked
         void OnEntitySearchAttachButtonClicked(bool checked = false);

         ///////////////////////////////////////////////////////////////////////
         // PREFERENCES WINDOW
         ///////////////////////////////////////////////////////////////////////

         /// Called when the HLAWindow action is triggered
         void OnHLAWindowActionTriggered();

         /// Called when the Full Screen action is triggered
         void OnFullScreenActionTriggered();

         /// Called when the show Controls action is triggered
         void OnShowControlsActionTriggered();

         /// Called when the show Entity Info action is triggered
         void OnShowEntityInfoActionTriggered();

         /// Called when the show Preferences action is triggered
         void OnShowPreferencesActionTriggered();

         /////////////////////////////////////////////////////////////////////////
         // General Tab
         /////////////////////////////////////////////////////////////////////////
         /// Called when the attach mode is changed
         void OnAttachModeChanged(const QString &text);

         /// Called when camera collision is enabled
         void OnCameraCollisionChanged(int state);

         /// Called when LOD scale is changed
         void OnLODScaleChanged(const QString &text);

         /// Called when the near clip plane is changed
         void OnNearClippingPlaneChanged(const QString &text);

         /// Called when the far clip plane is changed
         void OnFarClipplingPlaneChanged(const QString &text);

         /// Called when the show advanced options check box is checked
         void OnShowAdvancedGeneralOptions(int state);

         /////////////////////////////////////////////////////////////////////////
         // Environment Tab
         /////////////////////////////////////////////////////////////////////////
         /// Called when the weather theme radio button is clicked
         void OnWeatherThemedRadioButtonClicked(bool checked = false);

         /// Called when the weather theme radio button is clicked
         void OnWeatherCustomRadioButtonClicked(bool checked = false);

         /// Called when the weather theme radio button is clicked
         void OnWeatherNetworkRadioButtonClicked(bool checked = false);

         /// Called when the time of day time box is changed
         void OnTimeOfDayChanged(const QTime &newTime);

         /// Called when the weather theme is changed
         void OnWeatherThemeChanged(const QString &text);

         /// Called when the time theme is changed
         void OnTimeThemeChanged(const QString &text);

         /// Called when the visibility is changed
         void OnVisibilityChanged(const QString &text);

         /// Called when the cloud cover is changed
         void OnCloudCoverChanged(const QString &text);

         /////////////////////////////////////////////////////////////////////////
         // Tools Tab
         /////////////////////////////////////////////////////////////////////////
         /// Called when the coordinate system has been changed
         void OnToolsCoordinateSystemChanged(const QString &text);

         /// Called when binocular image being shown is changed
         void OnShowBinocularImageChanged(int state);

         /// Called when the distance to object being show is changed
         void OnShowDistanceToObjectChanged(int state);

         /// Called when the elevation of object being show is changed
         void OnShowElevationOfObjectChanged(int state);

         /// Called when the magnification is changed
         void OnMagnificationChanged(int value);

         /// Called when automatically attach on selection is changed
         void OnAutoAttachOnSelectionChanged(int state);
         /////////////////////////////////////////////////////////////////////////

         /// Called when we connect to HLA
         void OnConnectToHLA(QString connectionName);

         /// Called when we disconnect from HLA
         void OnDisconnectFromHLA();

         /// Called when the seconds timer elapses
         void OnSecondTimerElapsed();

         /// Called when the duration time elapses
         void OnDurationTimerElapsed();

         /// Called when the HLA error checking timer elapses
         void OnHLAErrorTimerElapsed();

         /// Called when an entity is selected from the search list
         void PopulateEntityInfoWindow(bool notUsed = false);

         /// Called when an entity is double clicked in the search list
         void PopulateEntityInfoWindow(QTableWidgetItem *item);

         /// Called when the timer elapses
         void OnGenericTickTimerElapsed();

         /// Called when the entity info timer elapses
         void OnRefreshEntityInfoTimerElapsed();

         /// Called when the auto refresh button is changed
         void OnAutoRefreshEntityInfoCheckBoxChanged(int state);

         /// Called when a time marker is double clicked in the list
         void OnTimeMarkerDoubleClicked(QListWidgetItem *item);

      protected:

         /// Called when the window is about to be show, or has just been shown.
         void showEvent(QShowEvent* event);

         /**
          * Called when the window receives the event to close itself.
          */
         void closeEvent(QCloseEvent *e);

         Ui::MainWindow* mUi;
         
      private:

         void InitGameApp(dtQt::OSGAdapterWidget& oglWidget, int appArgc, char* appArgv[], 
                  const std::string& appLibName);

         /**
          * Connects the signals and slots the main window needs.
          */
         void ConnectSlots();

         /**
          * Registers the configuration objects with the ViewerComponent
          */
         void AddConfigObjectsToViewerComponent();

         /**
          * Updates UI controls from the loaded preferences
          */
         void UpdateUIFromPreferences();

         /**
          * Enables or disables all of the playback buttons at once
          */
         void EnablePlaybackButtons(bool enable);

         /**
          * Connects the sig slots
          */
         void ConnectSigSlots();

         /**
          * Forces a reconnect
          */
         void ReconnectToHLA();

         /**
          * Clears data on disconnect
          */
         void ClearData();

         bool mIsPlaybackMode;

         bool mIsRecording;

         bool mIsPlayingBack;

         QString mCurrentConnectionName;

         QTimer mDurationTimer;
         QTimer mGenericTickTimer;
         QTimer mRefreshEntityInfoTimer;
         QTimer mHLAErrorTimer;

         dtCore::RefPtr<dtGame::GameApplication> mApp;
         bool mIsConnectedToHLA;

         std::vector<dtCore::ObserverPtr<dtGame::GameActorProxy> > mFoundActors;

         QDoubleValidator *mDoubleValidator;

         bool mShowMissingEntityInfoErrorMessage;
   };
}

#endif
