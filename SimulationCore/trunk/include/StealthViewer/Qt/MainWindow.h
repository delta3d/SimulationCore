/* -*-c++-*-
* Stealth Viewer - MainWindow (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, Alion Science and Technology Corporation
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
* @author Eddie Johnson
* @author David Guthrie
* @author Curtiss Murphy
*/
#ifndef DELTA_MAIN_WINDOW
#define DELTA_MAIN_WINDOW

#include <QtGui/QMainWindow>
#include <QtGui/QCheckBox>
#include <QtCore/QTimer>
#include <dtCore/refptr.h>
#include <dtCore/sigslot.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>
#include <vector>
#include <dtQt/deltastepper.h>

namespace dtGame
{
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
class QComboBox;

namespace StealthQt
{

   class ViewDockWidget;
   class AdditionalViewDockWidget;

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

         /// @return the dock widget that handles views.
         ViewDockWidget& GetViewDockWidget();

      signals:
         /// Fired when one of the additional 3D views is closed.
         void AdditionalViewClosed(AdditionalViewDockWidget&);

      public slots:

         /**
          * Starts a new wait cursor.  You MUST call endWaitCursor() for each
          * startWaitCursor().
          *
          * @note - This behavior is extremely trivial, but is pulled to mainWindow for
          * future expansion
          */
         void StartWaitCursor();

         /**
          * Ends a previously started wait cursor.  You must call this for each startWaitCursor().
          *
          * @note - This behavior is extremely trivial, but is pulled to mainWindow for
          * future expansion
          */
         void EndWaitCursor();

         ///////////////////////////////////////////////////////////////////////
         // CONTROLS WINDOW
         ///////////////////////////////////////////////////////////////////////

         ///////////////////////////////////////////////////////////////////////
         // Camera Tab
         ///////////////////////////////////////////////////////////////////////
         void OnWarpToLatLon(bool checked = false);
         void OnWarpToMGRS(bool checked = false);
         void OnWarpToXYZ(bool checked = false);

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

         /// Called when the Loop Continuously option box is changed
         void OnLoopContinuouslyChanged(int state);

         /// Called when playback speed is adjusted
         void OnPlaybackSpeedChanged(const QString& newText);

         /// Called when the jump to TM button is clicked
         void OnPlaybackJumpToTimeMarkerButtonClicked(bool checked = false);

         /// Called when the jump to TM button is clicked
         void OnPlaybackJumpToTimeMarkerButtonClicked(const QString& itemName);

         /// Called when a time marker is selected
         void OnPlaybackTimeMarkerSelected(const QString& text);
         ///////////////////////////////////////////////////////////////////////

         ///////////////////////////////////////////////////////////////////////
         // Search Tab
         ///////////////////////////////////////////////////////////////////////

         // Called when the search is initiated
         void OnEntitySearchSearchButtonClicked(bool checked = false);

         // Called when Detach is pressed - detach from the entity.
         void OnEntitySearchDetachButtonClicked(bool checked = false);

         // Called when the attach button is clicked
         void OnEntitySearchAttachButtonClicked(bool checked = false);

         ///////////////////////////////////////////////////////////////////////
         // PREFERENCES WINDOW
         ///////////////////////////////////////////////////////////////////////

         /// Called when the Map Window action is triggered
         void OnMapWindowActionTriggered();

         /// Called when the ConnectionWindow action is triggered
         void OnConnectionWindowActionTriggered();

         /// Called when the Full Screen action is triggered
         void OnFullScreenActionTriggered();

         /// Called when the show Controls action is triggered
         void OnShowControlsActionTriggered();

         /// Called when the show Entity Info action is triggered
         void OnShowEntityInfoActionTriggered();

         /// Called when the show Preferences action is triggered
         void OnShowPreferencesActionTriggered();

         /// Called when the menu option to show/hide the view ui is triggered
         void OnShowViewUIActionTriggered();

         /////////////////////////////////////////////////////////////////////////
         // General Tab
         /////////////////////////////////////////////////////////////////////////
         /// Called when the attach mode is changed
         void OnAttachModeChanged(const QString& text);

         /// Called when the Attach Node Name control changes.
         void OnAttachNodeNameChanged(const QString& text);

         /// Called when the relative attach azimuth angle control changes.
         void OnAttachAzimuthChanged(const QString& text);

         /// turned on when auto attach is enabled.
         void OnAutoAttachToggled(bool checked);

         /// turned on when auto attach is enabled.
         void OnAutoAttachEntityNameChanged(const QString& text);

         /// Called when camera collision is enabled
         void OnCameraCollisionChanged(int state);

         /// Called when LOD scale is changed
         void OnLODScaleChanged(const QString& text);

         /// Called when the near clip plane is changed
         void OnNearClippingPlaneChanged(const QString& text);

         /// Called when the far clip plane is changed
         void OnFarClipplingPlaneChanged(const QString& text);

         /// Called when the user selects a new unit to use when displaying lengths.
         void OnUnitOfLengthChanged(const QString& text);

         /// Called when the user selects a new unit to use when displaying angles.
         void OnUnitOfAngleChanged(const QString& text);

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

         /// Called when 360 compass being shown is changed
         void OnShowCompass360Changed(int state);

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
         void OnConnectToNetwork(QString connectionName);

         /// Called when we disconnect from HLA
         void OnDisconnectFromNetwork(bool disableUI);

         /// Called when the seconds timer elapses
         void OnSecondTimerElapsed();

         /// Called when the duration time elapses
         void OnDurationTimerElapsed();

         /// Called when the HLA error checking timer elapses
         void OnHLAErrorTimerElapsed();

         /// Called when an entity is selected from the search list
         void PopulateEntityInfoWindowDontAttach(bool notUsed = false);

         /// Called when an entity is double clicked in the search list
         void PopulateEntityInfoWindowAndAttach(QTableWidgetItem* currentItem);

         void DoPopulateEntityInfoWindow(QTableWidgetItem* currentItem, bool attach);

         /// Called when the timer elapses
         void OnGenericTickTimerElapsed();

         /// Called when the entity info timer elapses
         void OnRefreshEntityInfoTimerElapsed();

         /// Called when the auto refresh button is changed
         void OnAutoRefreshEntityInfoCheckBoxChanged(int state);

         /// Called when a time marker is double clicked in the list
         void OnTimeMarkerDoubleClicked(QListWidgetItem *item);

         /// Called when any of the label check boxes are toggled in the label visibility group
         void OnVisLabelsToggled(bool);

         /// Called when any of the non-label check boxes are toggled.
         void OnVisibilityOptionToggled(bool);

         /// Called when the label visibility distance is changed
         void OnVisLabelsDistanceChanged(const QString& text);

      protected:

         /// Called when the window is about to be show, or has just been shown.
         void showEvent(QShowEvent* event);

         /**
          * Called when the window receives the event to close itself.
          */
         void closeEvent(QCloseEvent *e);

         /**
          * Many of the combo boxes are backed by an enumeration, this fills the combo box and sets it to
          * point to the given value.
          */
         virtual void FillAndSetComboBox(const std::vector<dtUtil::Enumeration*>& enums,
                  QComboBox& combo, const dtUtil::Enumeration& enumValue);

         /// Some visibility check boxes are generated from enumerations, this adds them.
         virtual void AddVisibilityCheckBoxes();

         /**
          * Updates UI controls from the loaded preferences
          */
         virtual void UpdateUIFromPreferences();

         /**
          * Enables all the runtime UI controls and starts ticking the sim.
          */
         void EnableGeneralUIAndTick();

         /**
          * Disables all the runtime UI controls and stops ticking the sim.
          */
         void DisableGeneralUIAndTick();

      private:

         void ParseCommandLine();

         void PreShowUIControlInit();

         void InitGameApp(int appArgc, char* appArgv[],
                  const std::string& appLibName);
//         void InitGameApp(QGLWidget& oglWidget, int appArgc, char* appArgv[],
//                     const std::string& appLibName);

         /**
          * Connects the signals and slots the main window needs.
          */
         void ConnectSlots();

         /**
          * Registers the configuration objects with the ViewerComponent
          */
         void AddConfigObjectsToViewerComponent();

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

         /**
          * Shows an error that occured while updating the entity info window. Made into a method cause it's called from several places
          */
         void ShowEntityErrorMessage(QTableWidgetItem* currentItem);

         /**
          * Updates the data fields of the Entity Info window. Made into a method cause it's called from several places
          */
         void UpdateEntityInfoData(dtGame::GameActorProxy& parent);

         /**
          * There is only one of 3 coordinate systems active at any one time. Therefore, hide 2 of the 3 possible
          * sets of data fields in the EntityInfo Window.
          */
         void ShowOrHideEntityInfoPositionFields(const dtUtil::Enumeration& system);

         /**
          * Compute a human readable degree with only 2 decimal places. Internally, degrees
          * are 180 (turn left) to 0 (straight) to -180 (turn right). That's backwards who
          * expect it to be like a compass from 0 to 360. So swap the negative and adjust it.
          */
         float ComputeHumanReadableDirection(float flippedOrientation);

         /**
          * This method is used by the custom clock to wrap both minutes and seconds when
          * the user is advancing the time a lot. TimeUnits should be 60. Threshold
          * should be between 2 and 5. It returns -1 if it wrapped down, 0 if no change,
          * or +1 if it wrapped up. So, previous of 60 and new of 0 will return +1.
          */
         int AutoWrapTime(int previousTime, int newTime, int timeUnits, int timeThreshold);

         /**
          * Shows the correct Warp UI.  Using a base enum here to avoid including things
          * for a private method.
          */
         void SelectCorrectWarpToUI(dtUtil::Enumeration& enumValue);

         bool mIsPlaybackMode;

         bool mIsRecording;

         bool mIsPlayingBack;

         double mRecordingStartTime;
         double mRecordingStopTime;

         QString mCurrentConnectionName;

         dtQt::DeltaStepper mSimTicker;
         QTimer mDurationTimer;
         QTimer mGenericTickTimer;
         QTimer mRefreshEntityInfoTimer;
         QTimer mHLAErrorTimer;

         dtGame::GameApplicationLoader* mGameLoader;
         dtCore::RefPtr<dtGame::GameManager> mGM;
         bool mIsConnectedToANetwork;

         std::vector<dtCore::ObserverPtr<dtGame::GameActorProxy> > mFoundActors;

         QDoubleValidator* mLODScaleValidator;
         QDoubleValidator* mLatValidator;
         QDoubleValidator* mLonValidator;
         QDoubleValidator* mXYZValidator;
         QDoubleValidator* mGtZeroValidator;

         bool mShowMissingEntityInfoErrorMessage;

         // These values hold the previous setting of the custom time control. Used
         // to help scroll the clock forward.
         int mPreviousCustomHour;
         int mPreviousCustomMinute;
         int mPreviousCustomSecond;

         ViewDockWidget* mViewDockWidget;

         std::vector<QCheckBox*> mVisibilityCheckBoxes;

         Ui::MainWindow* mUi;
   };
}

#endif
