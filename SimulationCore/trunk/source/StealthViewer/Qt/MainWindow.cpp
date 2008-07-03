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
 * @author David Guthrie
 * @author Curtiss Murphy
 */
#include <prefix/SimCorePrefix-src.h>

// This has to be above the QT includes because 'emit' conflicts
#include <SimCore/Actors/BaseEntity.h>

#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QVBoxLayout>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>
#include <QtGui/QDoubleValidator>
#include <QtGui/QScrollBar>
#include <QtGui/QKeyEvent>

#include <StealthViewer/Qt/MainWindow.h>
#include <ui_MainWindowUi.h>
#include <StealthViewer/Qt/HLAWindow.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/OSGAdapterWidget.h>
#include <StealthViewer/Qt/EntitySearch.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>

#include <StealthViewer/GMApp/ViewerConfigComponent.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/StealthHUD.h>

#include <SimCore/HLA/HLAConnectionComponent.h>
#include <SimCore/SimCoreVersion.h>

#include <dtUtil/stringutils.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/datetime.h>

#include <dtCore/deltawin.h>
#include <dtCore/camera.h>
#include <dtCore/globals.h>
#include <dtCore/transformable.h>
#include <dtCore/transform.h>

#include <dtDAL/actorproperty.h>

#include <dtGame/gameapplication.h>
#include <dtGame/gmcomponent.h>
#include <dtGame/logcontroller.h>

#include <dtHLAGM/hlacomponent.h>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Matrixd>
#include <osg/Math>

#include <osgDB/FileNameUtils>

#include <osgViewer/GraphicsWindow>
#include <cmath>
#include <cfloat>


class EmbeddedWindowSystemWrapper: public osg::GraphicsContext::WindowingSystemInterface
{
   public:
      EmbeddedWindowSystemWrapper(osg::GraphicsContext::WindowingSystemInterface& oldInterface):
         mInterface(&oldInterface)
         {
         }

      virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier =
         osg::GraphicsContext::ScreenIdentifier())
      {
         return mInterface->getNumScreens(screenIdentifier);
      }

      virtual void getScreenResolution(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier,
               unsigned int& width, unsigned int& height)
      {
         mInterface->getScreenResolution(screenIdentifier, width, height);
      }

      virtual bool setScreenResolution(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier,
               unsigned int width, unsigned int height)
      {
         return mInterface->setScreenResolution(screenIdentifier, width, height);
      }

      virtual bool setScreenRefreshRate(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier,
               double refreshRate)
      {
         return mInterface->setScreenRefreshRate(screenIdentifier, refreshRate);
      }

      virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
      {
         return new osgViewer::GraphicsWindowEmbedded(traits);
      }

   protected:
      virtual ~EmbeddedWindowSystemWrapper() {};
   private:
      dtCore::RefPtr<osg::GraphicsContext::WindowingSystemInterface> mInterface;
};


namespace StealthQt
{
   ///////////////////////////////////////////////////////////////////////////////
   static std::string WINDOW_TITLE_VERSION("[Unknown]");
   static const std::string WINDOW_TITLE( "Stealth Viewer");
   static const std::string WINDOW_TITLE_MODE_PLAYBACK( " [Playback Mode]" );
   static const std::string WINDOW_TITLE_MODE_RECORD( " [Record Mode]" );

   ///////////////////////////////////////////////////////////////////////////////
   MainWindow::MainWindow(int appArgc, char* appArgv[], const std::string& appLibName):
      mUi(new Ui::MainWindow),
      mIsPlaybackMode(false),
      mIsRecording(false),
      mIsPlayingBack(false),
      mRecordingStartTime(0.0),
      mIsConnectedToHLA(false),
      mLODScaleValidator(new QDoubleValidator(0, 10000, 5, this)),
      mLatValidator(new QDoubleValidator(-90, 90, 10, this)),
      mLonValidator(new QDoubleValidator(-180, 180, 10, this)),
      mXYZValidator(new QDoubleValidator(-DBL_MAX, DBL_MAX, 10, this)),
      mShowMissingEntityInfoErrorMessage(true),
      mPreviousCustomHour(-1),
      mPreviousCustomMinute(-1),
      mPreviousCustomSecond(-1)
      {
      mUi->setupUi(this);
      ConnectSlots();

      // Instantiate singletons
      StealthViewerData::GetInstance().SetMainWindow(*this);

      QWidget* glParent = new QWidget(this);

      //GLWidgetRenderSurface* oglWidget = new GLWidgetRenderSurface(*app.GetWindow(), *app.GetCamera(), glParent);

      dtQt::OSGAdapterWidget* oglWidget = new dtQt::OSGAdapterWidget(false, glParent);
      oglWidget->setFocusPolicy(Qt::StrongFocus);

      QHBoxLayout* hbLayout = new QHBoxLayout(glParent);
      hbLayout->setMargin(0);
      glParent->setLayout(hbLayout);
      hbLayout->addWidget(oglWidget);
      setCentralWidget(glParent);

      mUi->mControlsDockWidget->installEventFilter(this);
      mUi->mEntityInfoDockWidget->installEventFilter(this);
      mUi->mPreferencesDockWidget->installEventFilter(this);
      mUi->mSearchCallSignLineEdit->installEventFilter(this);
      mUi->mSearchEntityTableWidget->installEventFilter(this);

      mUi->mGeneralAdvancedPerformanceOptionsGroupBox->hide();
      mUi->mRecordTimeMarkersGroupBox->hide();
      mUi->mPlaybackTimeMarkersGroupBox->hide();

      mUi->mWeatherUseNetworkSettingsRadioButton->setChecked(true);

      ///////////////////////////////////////////////
      // Temporarily disable the incompatible buttons
      // @TODO These should be commented out when the
      // weather component is refactored to be the
      // finite place to handle weather changes
      ///////////////////////////////////////////////
      mUi->mWeatherThemedRadioButton->hide();
      mUi->mCustomVisibilityLabel->hide();
      mUi->mCustomVisibilityComboBox->hide();
      mUi->mCustomCloudCoverLabel->hide();
      mUi->mCustomCloudCoverComboBox->hide();
      ///////////////////////////////////////////////

      mUi->mNetSetGroupBox->show();
      mUi->mThemedSettingsGroupBox->hide();
      mUi->mCustomSettingsGroupBox->hide();
      mUi->mPlaybackOptionsGroupBox->hide();

      // Cannot start up in record mode
      mUi->mRecordStartButton->setEnabled(false);
      mUi->mRecordAddTimeMarkerButton->setEnabled(false);
      mUi->mRecordAutomaticTimeMarkersCheckBox->setEnabled(false);
      mUi->mRecordAutomaticTimeMarkersSpinBox->setEnabled(false);

      // Disable the dock widgets until we connect since you can't actually do anything
      // until a connection is made.
      mUi->mPreferencesDockWidget->setEnabled(false);
      mUi->mControlsDockWidget->setEnabled(false);
      mUi->mEntityInfoDockWidget->setEnabled(false);

      // Cannot start up in playback mode either
      EnablePlaybackButtons(false);

      WINDOW_TITLE_VERSION = " - [Rev " + SimCore::SIMCORE_SVN_REVISION;
      if (!SimCore::SIMCORE_SVN_DATE.empty())
         WINDOW_TITLE_VERSION += ", " + SimCore::SIMCORE_SVN_DATE.substr(0, 10);
      WINDOW_TITLE_VERSION += "] ";
      std::string WinTitle(StealthQt::WINDOW_TITLE + WINDOW_TITLE_VERSION);
      setWindowTitle(tr(WinTitle.c_str()));

      mUi->mRecordDurationLineEdit->setText("0");
      mUi->mPlaybackDurationLineEdit->setText("0");

      QStringList headers;
      headers << "Call Sign" << "Force" << "Damage State";
      mUi->mSearchEntityTableWidget->setHorizontalHeaderLabels(headers);
      mUi->mSearchEntityTableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
      mUi->mSearchEntityTableWidget->setRowCount(0);

      mDurationTimer.setInterval(1000);
      mDurationTimer.setSingleShot(false);

      mHLAErrorTimer.setInterval(10000);
      mHLAErrorTimer.setSingleShot(true);

      mUi->mWarpToLat->setValidator(mLatValidator);
      mUi->mWarpToLon->setValidator(mLonValidator);
      mUi->mWarpToLLElev->setValidator(mXYZValidator);

      mUi->mWarpToX->setValidator(mXYZValidator);
      mUi->mWarpToY->setValidator(mXYZValidator);
      mUi->mWarpToZ->setValidator(mXYZValidator);

      mUi->mWarpToMGRSElev->setValidator(mXYZValidator);

      mUi->mGeneralLODScaleLineEdit->setValidator(mLODScaleValidator);

      mUi->mControlsTabWidget->setUsesScrollButtons(true);

      mGenericTickTimer.setInterval(5000);
      mGenericTickTimer.setSingleShot(false);
      mGenericTickTimer.start();

      mRefreshEntityInfoTimer.setInterval(1000);
      mRefreshEntityInfoTimer.setSingleShot(false);
      mRefreshEntityInfoTimer.start();

      // Disable full screen
      mUi->mMenuWindow->removeAction(mUi->mActionFullScreen);

      show();

      InitGameApp(*oglWidget, appArgc, appArgv, appLibName);

      // Note, stuff dealing with the path has to be BELOW InitGameApp().
      // NOTE - This image fails to load if you have a space in the path (like Program Files)
      // under QT 4.3.3, at least on Windows XP. This works with version 4.3.0.
      const std::string helpImageResource("icons/help_controls_small.jpg");
      const std::string& file = dtCore::FindFileInPathList(helpImageResource);
      if (!file.empty())
      {
         QPixmap pixmap;
         if (!pixmap.load(tr(file.c_str())))
            LOG_ERROR("Couldn't load camera help image \"" + file + "\".");
         mUi->mControlsCameraImageLabel->setPixmap(pixmap);
         mUi->mControlsCameraImageLabel->setScaledContents(true);
      }
      else
      {
         LOG_ERROR("Couldn't find camera help image \"" + helpImageResource + "\".");
      }

      const std::string iconImageResource("icons/stealthviewer.png");
      const std::string& iconFile = dtCore::FindFileInPathList(iconImageResource);
      if (!iconFile.empty())
      {
         QIcon *icon = new QIcon;
         QPixmap pixmap;
         if (!pixmap.load(tr(iconFile.c_str())))
            LOG_ERROR("Couldn't load icon file \"" + iconFile + "\".");
         icon->addPixmap(pixmap);
         setWindowIcon(*icon);
         //setIconSize(QSize(32, 32));
      }
      else
      {
         LOG_ERROR("Couldn't find app icon \"" + iconImageResource + "\".");
      }

      AddConfigObjectsToViewerComponent();

      ConnectSigSlots();

      ReconnectToHLA();

      // hide the last update time fields until it can be useful (see comment in UpdateEntityInfoData())
      mUi->mEntityInfoLastUpdateTimeLineEdit->hide();
      mUi->mEntityInfoLastUpdateTimeLabel->hide();

      // At startup, we have to hide 2 of the 3 position sections of Entity Info.
      ShowOrHideEntityInfoPositionFields(StealthGM::PreferencesToolsConfigObject::CoordinateSystem::MGRS);
      }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::InitGameApp(dtQt::OSGAdapterWidget& oglWidget, int appArgc, char* appArgv[],
            const std::string& appLibName)
   {
      ///Reset the windowing system for osg to use
      osg::GraphicsContext::WindowingSystemInterface* winSys = osg::GraphicsContext::getWindowingSystemInterface();

      if (winSys != NULL)
      {
         osg::GraphicsContext::setWindowingSystemInterface(new EmbeddedWindowSystemWrapper(*winSys));
      }

      try
      {
         mApp = new dtGame::GameApplication(appArgc, appArgv);
         mApp->SetGameLibraryName(appLibName);
         oglWidget.SetGraphicsWindow(*mApp->GetWindow()->GetOsgViewerGraphicsWindow());
         //hack to make sure the opengl context stuff gets resized to fit the window
         oglWidget.GetGraphicsWindow().resized(0, 0, oglWidget.width(), oglWidget.height());
         mApp->Config();
      }
      catch (const dtUtil::Exception& ex)
      {
         ex.LogException(dtUtil::Log::LOG_ERROR);
         throw;
      }
   }

   ///////////////////////////////////////////////////////////////////
   MainWindow::~MainWindow()
   {
      delete mUi;
      mUi = NULL;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnHLAWindowActionTriggered()
   {
      SimCore::HLA::HLAConnectionComponent* comp = NULL;
      mApp->GetGameManager()->GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME, comp);

      if(comp == NULL)
      {
         throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
                  "Failed to locate the HLAConnectionComponent on the Game Manager. Aborting application.",
                  __FILE__, __LINE__);
      }

      // Even error states should be considered connected so that the UI will make you disconnect first.
      mIsConnectedToHLA = comp->GetConnectionState() !=
         SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED;

      HLAWindow window(*mApp->GetGameManager(), this, NULL, mIsConnectedToHLA, mCurrentConnectionName);

      connect(&window, SIGNAL(ConnectedToHLA(QString)), this, SLOT(OnConnectToHLA(QString)));
      connect(&window, SIGNAL(DisconnectedFromHLA()), this, SLOT(OnDisconnectFromHLA()));

      if(window.exec() == QDialog::Accepted)
      {
         // Retrieve data from labels and process the input.
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::showEvent(QShowEvent* event)
   {
      // The first, initial show event is not spontaneous because Qt sends it when the window is
      // made visible.  Minimizing or otherwise hiding windows and bringing them back can cause
      // spontaneous show events, which we don't want.
      if (!event->spontaneous())
      {
         mUi->mActionShowControls->setChecked(mUi->mControlsDockWidget->isVisible());
         mUi->mActionShowEntityInfo->setChecked(mUi->mEntityInfoDockWidget->isVisible());
         mUi->mActionShowPreferences->setChecked(mUi->mPreferencesDockWidget->isVisible());

         StealthViewerData::GetInstance().GetSettings().LoadPreferences();
         UpdateUIFromPreferences();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::closeEvent(QCloseEvent *e)
   {
      StealthViewerData::GetInstance().GetSettings().WritePreferencesToFile();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::ConnectSlots()
   {
      /////////////////////////////////////////////////////////
      // Window Actions
      /////////////////////////////////////////////////////////
      connect(mUi->mActionHLAConnections,  SIGNAL(triggered()), this, SLOT(OnHLAWindowActionTriggered()));
      connect(mUi->mActionFullScreen,      SIGNAL(triggered()), this, SLOT(OnFullScreenActionTriggered()));
      connect(mUi->mActionShowControls,    SIGNAL(triggered()), this, SLOT(OnShowControlsActionTriggered()));
      connect(mUi->mActionShowEntityInfo,  SIGNAL(triggered()), this, SLOT(OnShowEntityInfoActionTriggered()));
      connect(mUi->mActionShowPreferences, SIGNAL(triggered()), this, SLOT(OnShowPreferencesActionTriggered()));
      /////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////
      // Controls
      /////////////////////////////////////////////////////////
      connect(mUi->mWarpToLLButton, SIGNAL(clicked(bool)),
               this,                    SLOT(OnWarpToLatLon(bool)));

      connect(mUi->mWarpToMGRSButton, SIGNAL(clicked(bool)),
               this,                    SLOT(OnWarpToMGRS(bool)));

      connect(mUi->mWarpToXYZButton, SIGNAL(clicked(bool)),
               this,                    SLOT(OnWarpToXYZ(bool)));

      connect(mUi->mRecordStartButton, SIGNAL(clicked(bool)),
               this,                    SLOT(OnRecordStartButtonClicked(bool)));

      connect(mUi->mRecordFilePushButton, SIGNAL(clicked(bool)),
               this,                       SLOT(OnRecordFileToolButtonClicked(bool)));

      connect(mUi->mRecordShowAdvancedOptionsCheckBox, SIGNAL(stateChanged(int)),
               this,                                    SLOT(OnShowAdvancedRecordOptionsChanged(int)));

      connect(mUi->mRecordAddTimeMarkerButton, SIGNAL(clicked(bool)),
               this,                            SLOT(OnAddTimeMarkerClicked(bool)));

      connect(mUi->mRecordAutomaticTimeMarkersCheckBox, SIGNAL(stateChanged(int)),
               this,                                     SLOT(OnAutoTimeMarkerCheckBoxChanged(int)));

      connect(mUi->mRecordAutomaticTimeMarkersSpinBox, SIGNAL(valueChanged(int)),
               this,                                    SLOT(OnAutoTimeMarkerSpinBoxChanged(int)));

      connect(mUi->mPlaybackSwitchToPlaybackModePushButton, SIGNAL(clicked(bool)),
               this,                                         SLOT(OnSwitchToPlaybackModeButtonClicked(bool)));

      connect(mUi->mPlaybackFilePushButton, SIGNAL(clicked(bool)),
               this,                         SLOT(OnPlaybackFileToolButtonClicked(bool)));

      connect(mUi->mPlaybackStartOverPushButton, SIGNAL(clicked(bool)),
               this,                              SLOT(OnPlaybackRestartButtonClicked(bool)));

      connect(mUi->mPlaybackJumpToPrevTimeMarkerPushButton, SIGNAL(clicked(bool)),
               this,                                         SLOT(OnPlaybackJumpToPrevTimeMarkerButtonClicked(bool)));

      connect(mUi->mPlaybackPlayPushButton, SIGNAL(clicked(bool)),
               this,                         SLOT(OnPlaybackPlayButtonClicked(bool)));

      connect(mUi->mPlaybackJumpToNextTimeMarkerPushButton, SIGNAL(clicked(bool)),
               this,                                         SLOT(OnPlaybackJumpToNextTimeMarkerButtonClicked(bool)));

      connect(mUi->mPlaybackShowAdvancedOptionsCheckBox, SIGNAL(stateChanged(int)),
               this,                                      SLOT(OnShowAdvancedPlaybackOptionsChanged(int)));

      connect(mUi->mPlaybackPlaybackSpeedComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                                SLOT(OnPlaybackSpeedChanged(const QString&)));

      connect(mUi->mPlaybackJumpToTimeMarkerPushButton, SIGNAL(clicked(bool)),
               this,                                     SLOT(OnPlaybackJumpToTimeMarkerButtonClicked(bool)));

      connect(mUi->mPlaybackTimeMarkersTextBox, SIGNAL(currentTextChanged(const QString&)),
               this,                             SLOT(OnPlaybackTimeMarkerSelected(const QString&)));

      /////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////
      // Preferences
      /////////////////////////////////////////////////////////
      connect(mUi->mGeneralAttachModeComboBox,          SIGNAL(currentIndexChanged(const QString&)),
               this,                                     SLOT(OnAttachModeChanged(const QString&)));

      connect(mUi->mGeneralEnableCameraCollisionCheckBox, SIGNAL(stateChanged(int)),
               this,                                       SLOT(OnCameraCollisionChanged(int)));

      connect(mUi->mGeneralLODScaleLineEdit,              SIGNAL(textChanged(const QString&)),
               this,                                       SLOT(OnLODScaleChanged(const QString&)));

      connect(mUi->mGeneralNearClippingPlaneComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                                   SLOT(OnNearClippingPlaneChanged(const QString&)));

      connect(mUi->mGeneralFarClippingPlaneComboBox,  SIGNAL(currentIndexChanged(const QString&)),
               this,                                   SLOT(OnFarClipplingPlaneChanged(const QString&)));

      connect(mUi->mGeneralShowAdvancedOptionsCheckBox, SIGNAL(stateChanged(int)),
               this,                                     SLOT(OnShowAdvancedGeneralOptions(int)));

      connect(mUi->mToolsCoordinateSystemComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                                SLOT(OnToolsCoordinateSystemChanged(const QString&)));

      connect(mUi->mToolsMagnificationSpinBox, SIGNAL(valueChanged(int)),
               this,                            SLOT(OnMagnificationChanged(int)));

      connect(mUi->mToolsAutoAttachOnSelectionCheckBox, SIGNAL(stateChanged(int)),
               this,                                     SLOT(OnAutoAttachOnSelectionChanged(int)));

      connect(mUi->mToolsShowBinocularImageCheckBox, SIGNAL(stateChanged(int)),
               this,                                  SLOT(OnShowBinocularImageChanged(int)));

      connect(mUi->mToolsShowDistanceToObjectCheckBox, SIGNAL(stateChanged(int)),
               this,                                    SLOT(OnShowDistanceToObjectChanged(int)));

      connect(mUi->mToolsShowElevationOfObjectCheckBox, SIGNAL(stateChanged(int)),
               this,                                     SLOT(OnShowElevationOfObjectChanged(int)));

      connect(mUi->mWeatherUseNetworkSettingsRadioButton, SIGNAL(clicked(bool)),
               this,                                       SLOT(OnWeatherNetworkRadioButtonClicked(bool)));

      connect(mUi->mWeatherCustomRadioButton, SIGNAL(clicked(bool)),
               this,                           SLOT(OnWeatherCustomRadioButtonClicked(bool)));

      connect(mUi->mWeatherThemedRadioButton, SIGNAL(clicked(bool)),
               this,                           SLOT(OnWeatherThemedRadioButtonClicked(bool)));

      connect(mUi->mCustomTimeEdit, SIGNAL(timeChanged(const QTime&)),
               this,                 SLOT(OnTimeOfDayChanged(const QTime&)));

      connect(mUi->mCustomVisibilityComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                           SLOT(OnVisibilityChanged(const QString&)));

      connect(mUi->mCustomCloudCoverComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                           SLOT(OnCloudCoverChanged(const QString&)));

      connect(mUi->mTimeComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                           SLOT(OnTimeThemeChanged(const QString&)));

      connect(mUi->mThemeComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                           SLOT(OnWeatherThemeChanged(const QString&)));

      connect(mUi->mSearchInfoPushButton, SIGNAL(clicked(bool)),
               this,                       SLOT(PopulateEntityInfoWindow(bool)));

      connect(mUi->mSearchEntityTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
               this,                          SLOT(PopulateEntityInfoWindow(QTableWidgetItem*)));

      connect(mUi->mSearchSearchPushButton, SIGNAL(clicked(bool)),
               this,                         SLOT(OnEntitySearchSearchButtonClicked(bool)));

      connect(mUi->mSearchAttachPushButton, SIGNAL(clicked(bool)),
               this,                         SLOT(OnEntitySearchAttachButtonClicked(bool)));

      connect(mUi->mSearchDetachPushButton, SIGNAL(clicked(bool)),
               this,                         SLOT(OnEntitySearchDetachButtonClicked(bool)));

      connect(mUi->mEntityInfoAutoRefreshCheckBox, SIGNAL(stateChanged(int)),
               this,                                SLOT(OnAutoRefreshEntityInfoCheckBoxChanged(int)));

      connect(mUi->mPlaybackTimeMarkersTextBox, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
               this,                             SLOT(OnTimeMarkerDoubleClicked(QListWidgetItem*)));
      ////////////////////////////////////////////////////

      connect(&mDurationTimer, SIGNAL(timeout()), this, SLOT(OnDurationTimerElapsed()));

      connect(&mGenericTickTimer, SIGNAL(timeout()), this, SLOT(OnGenericTickTimerElapsed()));

      connect(&mRefreshEntityInfoTimer, SIGNAL(timeout()), this, SLOT(OnRefreshEntityInfoTimerElapsed()));

      connect(&mHLAErrorTimer, SIGNAL(timeout()), this, SLOT(OnHLAErrorTimerElapsed()));
   }

   ///////////////////////////////////////////////////////////////////////////////
   bool MainWindow::eventFilter(QObject *object, QEvent *event)
   {
      if(object == mUi->mControlsDockWidget)
      {
         if(event->type() == QEvent::Close)
         {
            mUi->mActionShowControls->setChecked(false);
            return true;
         }
      }
      else if(object == mUi->mEntityInfoDockWidget)
      {
         if(event->type() == QEvent::Close)
         {
            mUi->mActionShowEntityInfo->setChecked(false);
            return true;
         }
      }
      else if(object == mUi->mPreferencesDockWidget)
      {
         if(event->type() == QEvent::Close)
         {
            mUi->mActionShowPreferences->setChecked(false);
            return true;
         }
      }
      else if(object == mUi->mSearchCallSignLineEdit)
      {
         if(event->type() == QEvent::KeyPress)
         {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Return)
            {
               OnEntitySearchSearchButtonClicked();
               return true;
            }
         }
      }
      else if(object == mUi->mSearchEntityTableWidget)
      {
         if(event->type() == QEvent::KeyPress)
         {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_T)
            {
               Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
               if(modifiers & Qt::ControlModifier)
               {
                  OnEntitySearchAttachButtonClicked();
                  return true;
               }
            }
         }
      }

      return QMainWindow::eventFilter(object, event);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::StartWaitCursor()
   {
      // Note - calling this method multiple times will nest the wait cursors,
      // so remember to call endWaitCursor() for each one.
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::EndWaitCursor()
   {
      QApplication::restoreOverrideCursor();
   }

   ///////////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////////
   // WINDOW ACTIONS
   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnFullScreenActionTriggered()
   {
      //mApp->GetWindow()->SetFullScreenMode(mUi->mActionFullScreen->isChecked());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowControlsActionTriggered()
   {
      bool showWindow = mUi->mActionShowControls->isChecked();
      mUi->mActionShowControls->setChecked(showWindow);

      showWindow ? mUi->mControlsDockWidget->show() : mUi->mControlsDockWidget->hide();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowEntityInfoActionTriggered()
   {
      bool showWindow = mUi->mActionShowEntityInfo->isChecked();
      mUi->mActionShowEntityInfo->setChecked(showWindow);

      showWindow ? mUi->mEntityInfoDockWidget->show() : mUi->mEntityInfoDockWidget->hide();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowPreferencesActionTriggered()
   {
      bool showWindow = mUi->mActionShowPreferences->isChecked();
      mUi->mActionShowPreferences->setChecked(showWindow);

      showWindow ? mUi->mPreferencesDockWidget->show() : mUi->mPreferencesDockWidget->hide();
   }

   ///////////////////////////////////////////////////////////////////////////////
   // CONTROLS WINDOW ////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWarpToLatLon(bool checked)
   {
      StealthGM::ControlsCameraConfigObject& cameraObject =
         StealthViewerData::GetInstance().GetCameraConfigObject();
      double lat = mUi->mWarpToLat->text().toDouble();
      double lon = mUi->mWarpToLon->text().toDouble();
      double elev = mUi->mWarpToLLElev->text().toDouble();
      try
      {
         cameraObject.WarpToPosition(lat, lon, elev);
      }
      catch (const dtUtil::Exception& ex)
      {
         if (ex.TypeEnum() == dtUtil::CoordinateConversionExceptionEnum::INVALID_INPUT)
         {
            QMessageBox::critical(this, "Error", QString(ex.What().c_str()));
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWarpToMGRS(bool checked)
   {
      StealthGM::ControlsCameraConfigObject& cameraObject =
         StealthViewerData::GetInstance().GetCameraConfigObject();
      const std::string MGRS = mUi->mWarpToMGRS->text().toStdString();
      double elev = mUi->mWarpToMGRSElev->text().toDouble();
      try
      {
         cameraObject.WarpToPosition(MGRS, elev);
      }
      catch (const dtUtil::Exception& ex)
      {
         if (ex.TypeEnum() == dtUtil::CoordinateConversionExceptionEnum::INVALID_INPUT)
         {
            QMessageBox::information(this, "Invalid Input", QString(ex.What().c_str()));
         }
         else
         {
            QMessageBox::critical(this, "Error", QString(ex.What().c_str()));
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWarpToXYZ(bool checked)
   {
      StealthGM::ControlsCameraConfigObject& cameraObject =
         StealthViewerData::GetInstance().GetCameraConfigObject();
      osg::Vec3d xyz;
      xyz.x() = mUi->mWarpToX->text().toDouble();
      xyz.y() = mUi->mWarpToY->text().toDouble();
      xyz.z() = mUi->mWarpToZ->text().toDouble();
      try
      {
         cameraObject.WarpToPosition(xyz);
      }
      catch (const dtUtil::Exception& ex)
      {
         if (ex.TypeEnum() == dtUtil::CoordinateConversionExceptionEnum::INVALID_INPUT)
         {
            QMessageBox::critical(this, "Error", QString(ex.What().c_str()));
         }
      }
   }

   void MainWindow::OnRecordStartButtonClicked(bool checked)
   {
      StealthGM::ControlsRecordConfigObject& recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      if(recordObject.GetOutputFilename().empty())
      {
         QMessageBox::warning(this, tr("Please select an output file"),
                  tr("Please select an output file to send record data to."),
                  QMessageBox::Ok);

         return;
      }

      mIsRecording = !mIsRecording;

      if(mIsRecording)
      {
         recordObject.StartRecording();
         mUi->mRecordStartButton->setText(tr("Stop"));
         mUi->mRecordDurationLineEdit->setText("0");
         mDurationTimer.start();
         mRecordingStartTime = mApp->GetGameManager()->GetSimulationTime();
         mRecordingStopTime = mApp->GetGameManager()->GetSimulationTime();
      }
      else
      {
         recordObject.StopRecording();
         mUi->mRecordStartButton->setText(tr("Start"));
         mDurationTimer.stop();
      }

      mUi->mControlsPlaybackTab->setEnabled( ! mIsRecording );
      mUi->mRecordAddTimeMarkerButton->setEnabled(mIsRecording);
      mUi->mRecordAutomaticTimeMarkersCheckBox->setEnabled(mIsRecording);
      mUi->mRecordAutomaticTimeMarkersSpinBox->setEnabled(mIsRecording);

      // Prevent network connection changes if in record mode.
      mUi->mMenuNetwork->setEnabled( ! mIsRecording );

      // Prevent changing files when in record mode.
      mUi->mRecordFilePushButton->setEnabled( ! mIsRecording );

      // Change the window title to indicate record mode.
      const std::string& title = mIsRecording
      ? StealthQt::WINDOW_TITLE + StealthQt::WINDOW_TITLE_VERSION + StealthQt::WINDOW_TITLE_MODE_RECORD :
      StealthQt::WINDOW_TITLE + StealthQt::WINDOW_TITLE_VERSION;
      setWindowTitle(tr(title.c_str()));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnRecordFileToolButtonClicked(bool checked)
   {
      StealthGM::ControlsRecordConfigObject &recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      QString msgFile = QFileDialog::getSaveFileName(this, tr("Select an output message file"),
               tr(""), tr("*.dlm"));

      if(msgFile.isEmpty())
         return;

      dtUtil::FileUtils &instance = dtUtil::FileUtils::GetInstance();
      if(!instance.FileExists(msgFile.toStdString()))
      {
         // The file selected does not exist.
         // So, create it and prompt if the create fails.
         std::ofstream out(msgFile.toStdString().c_str());
         if(!out.is_open())
         {
            QMessageBox::warning(this, tr("Error"),
                     tr("An error occurred trying to create the \
                     file. Please select another file."),
                     QMessageBox::Ok);
            return;
         }
         out.close();
      }

      std::string msg = osgDB::getStrippedName(msgFile.toStdString());

      if(!msg.empty())
      {
         recordObject.SetOutputFilename(msgFile.toStdString());

         mUi->mRecordStartButton->setEnabled(true);
         mUi->mRecordFileLineEdit->setText(tr(msg.c_str()));
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowAdvancedRecordOptionsChanged(int state)
   {
      state == Qt::Checked ? mUi->mRecordTimeMarkersGroupBox->show() :
         mUi->mRecordTimeMarkersGroupBox->hide();

      StealthGM::ControlsRecordConfigObject &recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      recordObject.SetShowAdvancedOptions(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAddTimeMarkerClicked(bool checked)
   {
      StealthGM::ControlsRecordConfigObject &recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      recordObject.AddKeyFrame();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAutoTimeMarkerCheckBoxChanged(int state)
   {
      StealthGM::ControlsRecordConfigObject &recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      bool checked = state == Qt::Checked;

      recordObject.SetAutoKeyFrame(checked);
      mUi->mRecordAutomaticTimeMarkersSpinBox->setEnabled(checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAutoTimeMarkerSpinBoxChanged(int value)
   {
      StealthGM::ControlsRecordConfigObject &recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      recordObject.SetAutoKeyFrameInterval(value);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnSwitchToPlaybackModeButtonClicked(bool checked)
   {
      StealthGM::ControlsRecordConfigObject &recConfig =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      mIsPlaybackMode = !mIsPlaybackMode;

      if(mIsPlaybackMode)
      {
         recConfig.DisconnectFromFederation();
         mUi->mPlaybackOptionsGroupBox->show();
         mUi->mPlaybackSwitchToPlaybackModePushButton->setText(tr("End Playback Mode"));
      }
      else
      {
         // Turn off paused in case the playback ended and paused the GM.
         // It will continue to be paused afterward.
         if(mApp->GetGameManager()->IsPaused())
            mApp->GetGameManager()->SetPaused(false);

         recConfig.JoinFederation();
         mUi->mPlaybackOptionsGroupBox->hide();
         mUi->mPlaybackSwitchToPlaybackModePushButton->setText(tr("Switch to Playback Mode"));
      }

      //mUi->mControlsCameraTab->setEnabled(!mIsPlaybackMode);
      mUi->mControlsRecordTab->setEnabled(!mIsPlaybackMode);
      //mUi->mControlsSearchTab->setEnabled(!mIsPlaybackMode);

      //mUi->mPreferencesGeneralTab->setEnabled(!mIsPlaybackMode);
      //mUi->mPreferencesEnvironmentTab->setEnabled(!mIsPlaybackMode);
      //mUi->mPreferencesToolsTab->setEnabled(!mIsPlaybackMode);

      //mUi->mEntityInfoDockWidget->setEnabled(!mIsPlaybackMode);

      if(!mIsPlaybackMode)
      {
         if(mUi->mPlaybackShowAdvancedOptionsCheckBox->checkState() == Qt::Checked)
            mUi->mPlaybackTimeMarkersGroupBox->hide();
      }
      else
      {
         if(mUi->mPlaybackShowAdvancedOptionsCheckBox->checkState() == Qt::Checked)
            mUi->mPlaybackTimeMarkersGroupBox->show();
      }

      if(mIsPlaybackMode)
      {
         //if(recConfig.GetIsRecording())
         {
            //recConfig.StopRecording();
         }
      }

      // Restore default state. Exited without clicking Stop
      if(mIsPlayingBack)
         OnPlaybackPlayButtonClicked();

      // Prevent network connection changes if in playback mode.
      mUi->mMenuNetwork->setEnabled( ! mIsPlaybackMode );

      // Change the window title to indicate playback mode.
      const std::string& title = mIsPlaybackMode
      ? StealthQt::WINDOW_TITLE + StealthQt::WINDOW_TITLE_VERSION + StealthQt::WINDOW_TITLE_MODE_PLAYBACK :
      StealthQt::WINDOW_TITLE + StealthQt::WINDOW_TITLE_VERSION;
      setWindowTitle(tr(title.c_str()));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackFileToolButtonClicked(bool checked)
   {
      QString msgFile = QFileDialog::getOpenFileName(this, tr("Select an input message file"),
               tr(""), tr("*.dlm"));

      if(msgFile.isEmpty())
         return;

      std::string msg = osgDB::getStrippedName(msgFile.toStdString());

      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      pbObject.SetInputFilename(msgFile.toStdString());

      //EnablePlaybackButtons(true);
      mUi->mPlaybackPlayPushButton->setEnabled(true);
      mUi->mPlaybackFileLineEdit->setText(tr(msg.c_str()));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackRestartButtonClicked(bool checked)
   {
      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      if(pbObject.GetInputFilename().empty())
      {
         QMessageBox::warning(this, tr("Please select an input file"),
                  tr("Please select an input file that contains record data to playback"),
                  QMessageBox::Ok);

         return;
      }

      pbObject.RestartPlayback();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackJumpToPrevTimeMarkerButtonClicked(bool checked)
   {
      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      pbObject.JumpToPreviousKeyFrame();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackPlayButtonClicked(bool checked)
   {
      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      if(mUi->mPlaybackFileLineEdit->text().isEmpty())
      {
         QMessageBox::warning(this, tr("Please select an input file"),
                  tr("Please select an input file that contains record data to playback"),
                  QMessageBox::Ok);

         return;
      }

      mIsPlayingBack = !mIsPlayingBack;

      if(mIsPlayingBack)
      {
         mUi->mPlaybackPlayPushButton->setText(tr("Stop"));
         mUi->mPlaybackDurationLineEdit->setText("0");

         pbObject.BeginPlayback();
         mDurationTimer.start();
      }
      else
      {
         mUi->mPlaybackPlayPushButton->setText(tr("Play"));
         mUi->mSearchEntityTableWidget->clear();
         mUi->mPlaybackTimeMarkersTextBox->clear();

         pbObject.EndPlayback();
         mDurationTimer.stop();
      }

      mUi->mPlaybackStartOverPushButton->setEnabled(mIsPlayingBack);
      mUi->mPlaybackJumpToPrevTimeMarkerPushButton->setEnabled(mIsPlayingBack);
      mUi->mPlaybackJumpToNextTimeMarkerPushButton->setEnabled(mIsPlayingBack);

      // Prevent changing files if a play back is in progress.
      mUi->mPlaybackFilePushButton->setEnabled( ! mIsPlayingBack );
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackJumpToNextTimeMarkerButtonClicked(bool checked)
   {
      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      pbObject.JumpToNextKeyFrame();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowAdvancedPlaybackOptionsChanged(int state)
   {
      if(state == Qt::Checked && mIsPlayingBack)
      {
         mUi->mPlaybackTimeMarkersGroupBox->show();
      }
      else
      {
         mUi->mPlaybackTimeMarkersGroupBox->hide();
      }

      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      pbObject.SetShowAdvancedOptions(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackSpeedChanged(const QString &newText)
   {
      StealthGM::ControlsPlaybackConfigObject &pbObject =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      QString text = newText;

      // Remove the "times" symbol
      text.remove("x");

      float value = float(text.toDouble());

      pbObject.SetPlaybackSpeed(value);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackJumpToTimeMarkerButtonClicked(bool checked)
   {
      QListWidgetItem *currentItem = mUi->mPlaybackTimeMarkersTextBox->currentItem();
      if(currentItem != NULL)
      {
         OnPlaybackJumpToTimeMarkerButtonClicked(currentItem->text());
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackJumpToTimeMarkerButtonClicked(const QString &itemName)
   {
      if(!itemName.isEmpty())
      {
         StealthGM::ControlsPlaybackConfigObject &pbObject =
            StealthViewerData::GetInstance().GetPlaybackConfigObject();

         pbObject.JumpToKeyFrame(itemName.toStdString());
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnEntitySearchSearchButtonClicked(bool checked)
   {
      mUi->mSearchEntityTableWidget->clear();
      mFoundActors.clear();

      // Another weird Qt nuance. If you do not set the headers ever time
      // you update the table they are replaced with defaulted values (1, 2, 3, 4)
      QStringList headers;
      headers << "Call Sign" << "Force" << "Damage State";
      mUi->mSearchEntityTableWidget->setHorizontalHeaderLabels(headers);

      EntitySearch::FindEntities(mFoundActors,
               *mApp->GetGameManager(),
               mUi->mSearchCallSignLineEdit->text().toStdString(),
               mUi->mSearchForceComboBox->currentText().toStdString(),
               mUi->mSearchDamageStateComboBox->currentText().toStdString());

      // According to the Qt 4.2.3 docs, if you do not turn sorting off
      // and reenable after inserting into a table in a for loop you get
      // really weird results. This was experienced visually as well
      mUi->mSearchEntityTableWidget->setSortingEnabled(false);

      // Only use the exact number of rows needed
      mUi->mSearchEntityTableWidget->setRowCount(int(mFoundActors.size()));

      for(size_t i = 0; i < mFoundActors.size(); i++)
      {
         std::string name  = mFoundActors[i]->GetName(),
         force = mFoundActors[i]->GetProperty("Force Affiliation")->ToString(),
         ds    = mFoundActors[i]->GetProperty("Damage State")->ToString(),
         id    = mFoundActors[i]->GetId().ToString();

         QTableWidgetItem *nameItem  = new QTableWidgetItem(tr(name.c_str()));
         QTableWidgetItem *forceItem = new QTableWidgetItem(tr(force.c_str()));
         QTableWidgetItem *dsItem    = new QTableWidgetItem(tr(ds.c_str()));

         nameItem->setData(Qt::UserRole, tr(id.c_str()));
         forceItem->setData(Qt::UserRole, tr(id.c_str()));
         dsItem->setData(Qt::UserRole, tr(id.c_str()));

         mUi->mSearchEntityTableWidget->setItem(i, 0, nameItem);
         mUi->mSearchEntityTableWidget->setItem(i, 1, forceItem);
         mUi->mSearchEntityTableWidget->setItem(i, 2, dsItem);
         //mUi->mSearchEntityTableWidget->setItem(i, 3, newItem);
      }

      mUi->mSearchEntityTableWidget->setSortingEnabled(true);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnEntitySearchDetachButtonClicked(bool checked)
   {
      mUi->mSearchDetachPushButton->setEnabled(false); // disable immediately, although it will get enabled/disabled in the 1 second timer
      StealthViewerData::GetInstance().GetGeneralConfigObject().DetachFromActor();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnEntitySearchAttachButtonClicked(bool checked)
   {
      // Send attach message. Must go through config object so message is sent
      // on next tick

      // Make sure we don't index out of bounds of the found actors
      // (This should never happen, but better safe than sorry)
      // Current row should return an unsigned int. Not sure why it doesn't.
      unsigned int row = (unsigned int)(mUi->mSearchEntityTableWidget->currentRow());
      if(row >= mFoundActors.size())
         return;

      // Get the name item from the current row, which stores the unique ID as
      // its data.
      QTableWidgetItem *item = mUi->mSearchEntityTableWidget->currentItem();
      if(item != NULL)
      {
         QString id = item->data(Qt::UserRole).toString();

         // Retrieve proxy from the GM
         dtGame::GameActorProxy *proxy = mApp->GetGameManager()->FindGameActorById(id.toStdString());
         if(proxy != NULL)
         {
            StealthViewerData::GetInstance().GetGeneralConfigObject().AttachToActor(*proxy);
         }
         else
         {
            // Name is always in column 0
            int row = item->row();

            QTableWidgetItem *itemAt = mUi->mSearchEntityTableWidget->item(row, 0);

            QString message = tr("Could not attach to the actor named: ") +
            (itemAt != NULL ? itemAt->text() : item->text()) +
            tr(" because this actor has been removed from the scenario. Please select another actor");

            QMessageBox::warning(this, tr("Error attaching to actor"), message, QMessageBox::Ok);
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   // PREFERENCES WINDOW
   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAttachModeChanged(const QString &text)
   {
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      if(text.toStdString() == StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON.GetName())
         genConfig.SetAttachMode(StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON);
      else
         genConfig.SetAttachMode(StealthGM::PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnCameraCollisionChanged(int state)
   {
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetCameraCollision(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnLODScaleChanged(const QString &text)
   {
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetLODScale(text.toDouble());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnNearClippingPlaneChanged(const QString &text)
   {
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetNearClippingPlane(text.toDouble());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnFarClipplingPlaneChanged(const QString &text)
   {
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetFarClippingPlane(text.toDouble());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowAdvancedGeneralOptions(int state)
   {
      state == Qt::Checked ? mUi->mGeneralAdvancedPerformanceOptionsGroupBox->show() :
         mUi->mGeneralAdvancedPerformanceOptionsGroupBox->hide();

      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetShowAdvancedOptions(state == Qt::Checked);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWeatherThemedRadioButtonClicked(bool checked)
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      mUi->mThemedSettingsGroupBox->show();
      mUi->mCustomSettingsGroupBox->hide();
      mUi->mNetSetGroupBox->hide();

      envConfig.SetUseThemedSettings();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWeatherCustomRadioButtonClicked(bool checked)
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      // When we switch to custom, reset our previous settings back to none.
      mPreviousCustomHour = -1;
      mPreviousCustomMinute = -1;
      mPreviousCustomSecond = -1;

      mUi->mThemedSettingsGroupBox->hide();
      mUi->mCustomSettingsGroupBox->show();
      mUi->mNetSetGroupBox->hide();

      envConfig.SetUseCustomSettings();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWeatherNetworkRadioButtonClicked(bool checked)
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      mUi->mThemedSettingsGroupBox->hide();
      mUi->mCustomSettingsGroupBox->hide();
      mUi->mNetSetGroupBox->show();

      envConfig.SetUseNetworkSettings();
   }

   ///////////////////////////////////////////////////////////////////////////////
   // Themed Weather Settings
   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWeatherThemeChanged(const QString &text)
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      QString hack = tr("Theme ") + text;

      if(hack.toStdString() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CUSTOM.GetName())
         envConfig.SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CUSTOM);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CLEAR.GetName())
         envConfig.SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CLEAR);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FAIR.GetName())
         envConfig.SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FAIR);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FOGGY.GetName())
         envConfig.SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FOGGY);
      else
         envConfig.SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_RAINY);
   }

   void MainWindow::OnTimeThemeChanged(const QString &text)
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      QString hack = tr("Time ") + text;

      if(hack.toStdString() == dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DAWN.GetName())
         envConfig.SetTimeTheme(dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DAWN);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DAY.GetName())
         envConfig.SetTimeTheme(dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DAY);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DUSK.GetName())
         envConfig.SetTimeTheme(dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DUSK);
      else
         envConfig.SetTimeTheme(dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_NIGHT);
   }

   ///////////////////////////////////////////////////////////////////////////////
   // Custom Weather Settings
   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnTimeOfDayChanged(const QTime &newTime)
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      int newHour = newTime.hour();
      int newMinute = newTime.minute();
      int newSecond = newTime.second();
      bool timeChanged = false;

      // if they changed the custom time before, and we are near a border, and we
      // just crossed over the border, then we want to increase/decrease our hours or minutes.
      // This causes it to appear to 'wrap' the time. QT does not provide this on the time control.
      if (mPreviousCustomHour != -1 && mPreviousCustomHour == newHour)
      {
         int minuteChange = AutoWrapTime(mPreviousCustomSecond, newSecond, 60, 4);
         newMinute += minuteChange;
         int hourChange = 0;

         if (newMinute < 0) // seconds wrapped down
         {
            newMinute = 59;
            newHour -= 1;
         }
         else if (newMinute > 59) // seconds wrapped up
         {
            newMinute = 0;
            newHour += 1;
         }
         else  // handle minutes wrapping
         {
            hourChange = AutoWrapTime(mPreviousCustomMinute, newMinute, 60, 4);
            newHour += hourChange;
         }

         // correct hours
         if (newHour > 23)
            newHour = 0;
         else if (newHour < 0)
            newHour = 23;

         // If we got any sort of wrap, then we have a time change.
         if (minuteChange != 0 || hourChange != 0)
            timeChanged = true;
      }

      envConfig.SetCustomHour(newHour);
      envConfig.SetCustomMinute(newMinute);
      envConfig.SetCustomSecond(newSecond);

      // update our previous values for next time
      mPreviousCustomHour = newHour;
      mPreviousCustomMinute = newMinute;
      mPreviousCustomSecond = newSecond;

      // Make sure we update the QT control
      // note, this will cause this method to be called again - IMMEDIATELY.
      if (timeChanged)
      {
         QTime time(newHour, newMinute, newSecond);
         mUi->mCustomTimeEdit->setTime(time);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   int MainWindow::AutoWrapTime(int previousTime, int newTime, int timeUnits, int timeThreshold)
   {
      // attempt to wrap time. Units is 60 for minutes/seconds. Threadhold is how far
      // out you want to check, somewhere in the 3-6 range.
      bool oldTimeCloseToTopUnit = (timeUnits - previousTime) < timeThreshold;
      bool oldTimeCloseToZero = (previousTime < timeThreshold);
      bool newTimeCloseToTopUnit = (timeUnits - newTime) < timeThreshold;
      bool newTimeCloseToZero = (newTime < timeThreshold);
      int result = 0;

      if (oldTimeCloseToTopUnit && newTimeCloseToZero)
         result += 1;
      else if (oldTimeCloseToZero && newTimeCloseToTopUnit)
         result -= 1;

      return result;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnVisibilityChanged(const QString &text)
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      // Convert to match the name of the actor's enum. The full name
      // doesn't look pretty in the UI, so we shortened it
      QString hack = "Visibility " + text;

      if(hack.toStdString() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_UNLIMITED.GetName())
         envConfig.SetVisibility(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_UNLIMITED);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_FAR.GetName())
         envConfig.SetVisibility(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_FAR);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_MODERATE.GetName())
         envConfig.SetVisibility(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_MODERATE);
      else if(hack.toStdString() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_LIMITED.GetName())
         envConfig.SetVisibility(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_LIMITED);
      else
         envConfig.SetVisibility(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_CLOSE);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnCloudCoverChanged(const QString &text)
   {
      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      const std::string cloudCoverName = text.toStdString();
      dtActors::BasicEnvironmentActor::CloudCoverEnum* cloudCover = dtActors::BasicEnvironmentActor::CloudCoverEnum::GetValueForName(cloudCoverName);
      if (cloudCover != NULL)
         envConfig.SetCloudCover(*cloudCover);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnToolsCoordinateSystemChanged(const QString &text)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      const std::string coordType = text.toStdString();
      StealthGM::PreferencesToolsConfigObject::CoordinateSystem* coordSystem =
         StealthGM::PreferencesToolsConfigObject::CoordinateSystem::GetValueForName(coordType);
      if(coordSystem!= NULL)
      {
         toolsConfig.SetCoordinateSystem(*coordSystem);
         SelectCorrectWarpToUI(*coordSystem);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowBinocularImageChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject &toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetShowBinocularImage(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowDistanceToObjectChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject &toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetShowDistanceToObject(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowElevationOfObjectChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject &toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetShowElevationOfObject(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnMagnificationChanged(int value)
   {
      StealthGM::PreferencesToolsConfigObject &toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetMagnification(float(value));
   }
   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAutoAttachOnSelectionChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject &toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetAutoAttachOnSelection(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::AddConfigObjectsToViewerComponent()
   {
      dtGame::GameManager *gm = mApp->GetGameManager();
      dtGame::GMComponent *gmComp =
         gm->GetComponentByName(StealthGM::ViewerConfigComponent::DEFAULT_NAME);

      if(gmComp == NULL)
      {
         LOG_ERROR("Failed to find the ViewerConfigComponent on the Game Manager.");
      }

      StealthGM::ViewerConfigComponent &viewComp =
         static_cast<StealthGM::ViewerConfigComponent&>(*gmComp);

         StealthViewerData &instance = StealthViewerData::GetInstance();

         viewComp.AddConfigObject(instance.GetGeneralConfigObject());
         viewComp.AddConfigObject(instance.GetEnvironmentConfigObject());
         viewComp.AddConfigObject(instance.GetToolsConfigObject());

         viewComp.AddConfigObject(instance.GetCameraConfigObject());
         viewComp.AddConfigObject(instance.GetRecordConfigObject());
         viewComp.AddConfigObject(instance.GetPlaybackConfigObject());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void MainWindow::UpdateUIFromPreferences()
   {
      // General Preferences
      StealthGM::PreferencesGeneralConfigObject &genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      if(genConfig.GetAttachMode() == StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON)
      {
         mUi->mGeneralAttachModeComboBox->setCurrentIndex(0);
      }
      else if(genConfig.GetAttachMode() == StealthGM::PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON)
      {
         mUi->mGeneralAttachModeComboBox->setCurrentIndex(1);
      }
      else
      {
         LOG_ERROR("Unknown attach mode: " + genConfig.GetAttachMode().GetName());
      }

      mUi->mEntityInfoAutoRefreshCheckBox->setChecked(genConfig.GetAutoRefreshEntityInfoWindow());

      mUi->mGeneralEnableCameraCollisionCheckBox->setChecked(genConfig.GetEnableCameraCollision());

      int index = mUi->mGeneralFarClippingPlaneComboBox->findText(QString::number(genConfig.GetFarClippingPlane()));
      if (index >= 0)
      {
         mUi->mGeneralFarClippingPlaneComboBox->setCurrentIndex(index);
      }

      index = mUi->mGeneralNearClippingPlaneComboBox->findText(QString::number(genConfig.GetNearClippingPlane()));
      if (index >= 0)
      {
         mUi->mGeneralNearClippingPlaneComboBox->setCurrentIndex(index);
      }

      mUi->mGeneralLODScaleLineEdit->setText(QString::number(genConfig.GetLODScale()));

      if(genConfig.GetPerformanceMode() == StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS)
      {
         mUi->mGeneralPerformanceComboBox->setCurrentIndex(0);
      }
      else if(genConfig.GetPerformanceMode() == StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BETTER_GRAPHICS)
      {
         mUi->mGeneralPerformanceComboBox->setCurrentIndex(1);
      }
      else if(genConfig.GetPerformanceMode() == StealthGM::PreferencesGeneralConfigObject::PerformanceMode::DEFAULT)
      {
         mUi->mGeneralPerformanceComboBox->setCurrentIndex(2);
      }
      else if(genConfig.GetPerformanceMode() == StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BETTER_SPEED)
      {
         mUi->mGeneralPerformanceComboBox->setCurrentIndex(3);
      }
      else if(genConfig.GetPerformanceMode() == StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_SPEED)
      {
         mUi->mGeneralPerformanceComboBox->setCurrentIndex(4);
      }
      else
      {
         LOG_ERROR("Unknown performance mode: " + genConfig.GetPerformanceMode().GetName());
         mUi->mGeneralPerformanceComboBox->setCurrentIndex(2);
      }

      mUi->mGeneralShowAdvancedOptionsCheckBox->setChecked(genConfig.GetShowAdvancedOptions());

      // General Environment
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      // Ensure the correct box displays. Invoke the slot manually
      if(envConfig.GetUseNetworkSettings())
      {
         mUi->mWeatherUseNetworkSettingsRadioButton->setChecked(true);
         OnWeatherNetworkRadioButtonClicked();
      }
      else if(envConfig.GetUseThemedSettings())
      {
         mUi->mWeatherThemedRadioButton->setChecked(true);
         OnWeatherThemedRadioButtonClicked();
      }
      else if(envConfig.GetUseCustomSettings())
      {
         mUi->mWeatherCustomRadioButton->setChecked(true);
         OnWeatherCustomRadioButtonClicked();
      }

      if(envConfig.GetCloudCover() == dtActors::BasicEnvironmentActor::CloudCoverEnum::CLEAR)
      {
         mUi->mCustomCloudCoverComboBox->setCurrentIndex(0);
      }
      else if(envConfig.GetCloudCover() == dtActors::BasicEnvironmentActor::CloudCoverEnum::FEW)
      {
         mUi->mCustomCloudCoverComboBox->setCurrentIndex(1);
      }
      else if(envConfig.GetCloudCover() == dtActors::BasicEnvironmentActor::CloudCoverEnum::SCATTERED)
      {
         mUi->mCustomCloudCoverComboBox->setCurrentIndex(2);
      }
      else if(envConfig.GetCloudCover() == dtActors::BasicEnvironmentActor::CloudCoverEnum::BROKEN)
      {
         mUi->mCustomCloudCoverComboBox->setCurrentIndex(3);
      }
      else if(envConfig.GetCloudCover() == dtActors::BasicEnvironmentActor::CloudCoverEnum::OVERCAST)
      {
         mUi->mCustomCloudCoverComboBox->setCurrentIndex(4);
      }
      else
      {
         LOG_ERROR("Unknown cloud cover: " + envConfig.GetCloudCover().GetName());
      }

      QTime time(envConfig.GetCustomHour(),
               envConfig.GetCustomMinute(),
               envConfig.GetCustomSecond());
      mUi->mCustomTimeEdit->setTime(time);

      if(envConfig.GetTimeTheme() == dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DAWN)
      {
         mUi->mTimeComboBox->setCurrentIndex(0);
      }
      else if(envConfig.GetTimeTheme() == dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DAY)
      {
         mUi->mTimeComboBox->setCurrentIndex(1);
      }
      else if(envConfig.GetTimeTheme() == dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DUSK)
      {
         mUi->mTimeComboBox->setCurrentIndex(2);
      }
      else if(envConfig.GetTimeTheme() == dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_NIGHT)
      {
         mUi->mTimeComboBox->setCurrentIndex(3);
      }
      else
      {
         LOG_ERROR("Unknown time theme: " + envConfig.GetTimeTheme().GetName());
      }

      if(envConfig.GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CLEAR)
      {
         mUi->mThemeComboBox->setCurrentIndex(0);
      }
      else if(envConfig.GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CUSTOM)
      {
         mUi->mThemeComboBox->setCurrentIndex(1);
      }
      else if(envConfig.GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FAIR)
      {
         mUi->mThemeComboBox->setCurrentIndex(2);
      }
      else if(envConfig.GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FOGGY)
      {
         mUi->mThemeComboBox->setCurrentIndex(3);
      }
      else if(envConfig.GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_RAINY)
      {
         mUi->mThemeComboBox->setCurrentIndex(4);
      }
      else
      {
         LOG_ERROR("Unknown weather theme: " + envConfig.GetWeatherTheme().GetName());
      }

      if(envConfig.GetVisibility() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_UNLIMITED)
      {
         mUi->mCustomVisibilityComboBox->setCurrentIndex(0);
      }
      else if(envConfig.GetVisibility() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_FAR)
      {
         mUi->mCustomVisibilityComboBox->setCurrentIndex(1);
      }
      else if(envConfig.GetVisibility() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_MODERATE)
      {
         mUi->mCustomVisibilityComboBox->setCurrentIndex(2);
      }
      else if(envConfig.GetVisibility() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_LIMITED)
      {
         mUi->mCustomVisibilityComboBox->setCurrentIndex(3);
      }
      else if(envConfig.GetVisibility() == dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_CLOSE)
      {
         mUi->mCustomVisibilityComboBox->setCurrentIndex(4);
      }
      else
      {
         LOG_ERROR("Unknown visibility theme: " + envConfig.GetVisibility().GetName());
      }

      StealthGM::PreferencesToolsConfigObject &toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      mUi->mToolsAutoAttachOnSelectionCheckBox->setChecked(toolsConfig.GetAutoAttachOnSelection());

      if(toolsConfig.GetCoordinateSystem() == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::MGRS)
      {
         mUi->mToolsCoordinateSystemComboBox->setCurrentIndex(0);
      }
      else if(toolsConfig.GetCoordinateSystem() == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ)
      {
         mUi->mToolsCoordinateSystemComboBox->setCurrentIndex(1);
      }
      else if(toolsConfig.GetCoordinateSystem() == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::LAT_LON)
      {
         mUi->mToolsCoordinateSystemComboBox->setCurrentIndex(2);
      }
      else
      {
         LOG_ERROR("Unknown coordinate system: " + toolsConfig.GetCoordinateSystem().GetName());
      }

      mUi->mToolsMagnificationSpinBox->setValue(int(toolsConfig.GetMagnification()));
      mUi->mToolsShowBinocularImageCheckBox->setChecked(toolsConfig.GetShowBinocularImage());
      mUi->mToolsShowDistanceToObjectCheckBox->setChecked(toolsConfig.GetShowDistanceToObject());
      mUi->mToolsShowElevationOfObjectCheckBox->setChecked(toolsConfig.GetShowElevationOfObject());

      // Camera controls

      // Record controls
      StealthGM::ControlsRecordConfigObject &recordConfig =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      mUi->mRecordShowAdvancedOptionsCheckBox->setChecked(recordConfig.GetShowAdvancedOptions());
      bool enable = !recordConfig.GetOutputFilename().empty();
      if(enable)
      {
         mUi->mRecordFileLineEdit->setText(tr(recordConfig.GetOutputFilename().c_str()));
      }

      mUi->mRecordStartButton->setEnabled(enable);
      mUi->mRecordAddTimeMarkerButton->setEnabled(false);

      mUi->mRecordAutomaticTimeMarkersCheckBox->setChecked(recordConfig.GetAutoKeyFrame());
      mUi->mRecordAutomaticTimeMarkersSpinBox->setEnabled(recordConfig.GetAutoKeyFrame());
      mUi->mRecordAutomaticTimeMarkersSpinBox->setValue(recordConfig.GetAutoKeyFrameInterval());

      // Playback controls
      StealthGM::ControlsPlaybackConfigObject &playbackConfig =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      mUi->mPlaybackShowAdvancedOptionsCheckBox->setChecked(playbackConfig.GetShowAdvancedOptions());
      if(!playbackConfig.GetInputFilename().empty())
      {
         mUi->mPlaybackFileLineEdit->setText(tr(playbackConfig.GetInputFilename().c_str()));
         mUi->mPlaybackPlayPushButton->setEnabled(true);
      }

      float value = playbackConfig.GetPlaybackSpeed();
      if(value == 0.1f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(0);
      }
      else if(value == 0.25f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(1);
      }
      else if(value == 0.5f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(2);
      }
      else if(value == 1.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(3);
      }
      else if(value == 1.5f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(4);
      }
      else if(value == 2.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(5);
      }
      else if(value == 4.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(6);
      }
      else if(value == 8.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(7);
      }
      else if(value == 16.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(8);
      }
      else
      {
         std::ostringstream oss;
         oss << "Unsupported playback speed value: " << value;
         LOG_ERROR(oss.str());
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(3);
      }

      enable = !playbackConfig.GetInputFilename().empty();
      mUi->mPlaybackPlayPushButton->setEnabled(enable);
   }

   void MainWindow::RecordKeyFrameSlot(const std::vector<dtGame::LogKeyframe> &keyFrames)
   {
      mUi->mRecordTimeMarkersLineTextBox->clear();

      for(size_t i = 0; i < keyFrames.size(); i++)
      {
         mUi->mRecordTimeMarkersLineTextBox->addItem(
                  new QListWidgetItem(tr(keyFrames[i].GetName().c_str())));
      }
   }

   void MainWindow::PlaybackKeyFrameSlot(const std::vector<dtGame::LogKeyframe> &keyFrames)
   {
      mUi->mPlaybackTimeMarkersTextBox->clear();

      for(size_t i = 0; i < keyFrames.size(); i++)
      {
         mUi->mPlaybackTimeMarkersTextBox->addItem(
                  new QListWidgetItem(tr(keyFrames[i].GetName().c_str())));
      }
   }

   void MainWindow::EnablePlaybackButtons(bool enable)
   {
      mUi->mPlaybackStartOverPushButton->setEnabled(enable);
      mUi->mPlaybackJumpToPrevTimeMarkerPushButton->setEnabled(enable);
      mUi->mPlaybackPlayPushButton->setEnabled(enable);
      mUi->mPlaybackJumpToNextTimeMarkerPushButton->setEnabled(enable);
      //mUi->mPlaybackPlaybackSpeedComboBox->setEnabled(enable);
      mUi->mPlaybackJumpToTimeMarkerPushButton->setEnabled(enable);
   }

   void MainWindow::ConnectSigSlots()
   {
      dtGame::GMComponent *gmComp =
         mApp->GetGameManager()->GetComponentByName("LogController");
      if(gmComp == NULL)
         return;

      dtGame::LogController &logController = static_cast<dtGame::LogController&>(*gmComp);

      logController.SignalReceivedKeyframes().connect_slot(this, &MainWindow::RecordKeyFrameSlot);
      logController.SignalReceivedKeyframes().connect_slot(this, &MainWindow::PlaybackKeyFrameSlot);
   }

   void MainWindow::OnConnectToHLA(QString connectionName)
   {
      mIsConnectedToHLA = true;
      mCurrentConnectionName = connectionName;

      mUi->mPreferencesDockWidget->setEnabled(true);
      mUi->mControlsDockWidget->setEnabled(true);
      mUi->mEntityInfoDockWidget->setEnabled(true);

      mGenericTickTimer.start();
      mHLAErrorTimer.start();

      EndWaitCursor();
   }

   void MainWindow::OnDisconnectFromHLA()
   {
      mIsConnectedToHLA = false;
      mCurrentConnectionName = tr("");

      mUi->mPreferencesDockWidget->setEnabled(false);
      mUi->mControlsDockWidget->setEnabled(false);
      mUi->mEntityInfoDockWidget->setEnabled(false);

      mGenericTickTimer.stop();

      ClearData();
   }

   void MainWindow::OnSecondTimerElapsed()
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      if(envConfig.GetUseNetworkSettings())
      {
         // Time of day
         int hour = envConfig.GetNetworkHour(),
         min  = envConfig.GetNetworkMinute(),
         sec  = envConfig.GetNetworkSecond();

         QTime currentTime(hour, min, sec);
         mUi->mNetworkTimeTimeEdit->setTime(currentTime);

         // Visibility
         QString visibility = QString::number(envConfig.GetVisibilityDistance()) + tr(" meters");
         mUi->mVisibilityLineEdit->setText(visibility);

         // Weather
         mUi->mWeatherLineEdit->setText(tr(envConfig.GetPrecipitationAsString().c_str()));
      }
   }

   void MainWindow::OnDurationTimerElapsed()
   {
      dtGame::GameManager &gm = *mApp->GetGameManager();
      double curSimtime = gm.GetSimulationTime();
      mRecordingStopTime = curSimtime;

      if (mIsRecording || mIsPlayingBack)
      {
         if(mIsRecording)
         {
            std::string duration = dtUtil::DateTime::ToString(time_t(mRecordingStopTime - mRecordingStartTime),
                     dtUtil::DateTime::TimeFormat::CLOCK_TIME_24_HOUR_FORMAT);

            //mUi->mRecordDurationLineEdit->setText(!gm.IsPaused() ? QString(duration.c_str()): tr("Paused"));
            mUi->mRecordDurationLineEdit->setText(QString(duration.c_str()));
         }
         else if(mIsPlayingBack)
         {
            // Get the log controller
            dtGame::GMComponent *component = gm.GetComponentByName(dtGame::LogController::DEFAULT_NAME);
            if (component == NULL)
               return ; // Shouldn't happen, but just to be safe.
            dtGame::LogController &logController = static_cast<dtGame::LogController&>(*component);

            double durationTime = logController.GetLastKnownStatus().GetCurrentRecordDuration();
            std::string duration = dtUtil::DateTime::ToString(time_t(durationTime),
                     dtUtil::DateTime::TimeFormat::CLOCK_TIME_24_HOUR_FORMAT);
            //// QString::number(duration)  ... !gm.IsPaused() ? QString(duration.c_str()): tr("Paused")
            mUi->mPlaybackDurationLineEdit->setText(QString(duration.c_str()));
         }
      }
   }

   void MainWindow::OnHLAErrorTimerElapsed()
   {
      SimCore::HLA::HLAConnectionComponent *comp =
         static_cast<SimCore::HLA::HLAConnectionComponent*>
      (mApp->GetGameManager()->GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME));

      if(comp->GetConnectionState() == SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_ERROR)
      {
         QMessageBox::critical(this, tr("Error"),
                  tr("An error occurred while connecting to HLA. ") +
                  tr("Please check your connection settings from the Network tab ") +
                  tr("and ensure they are correct."),
                  QMessageBox::Ok);

         OnHLAWindowActionTriggered();
      }
      else if (comp->GetConnectionState() == SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_CONNECTING)
      {
         //fire the timer again if it's still running.
         mHLAErrorTimer.start();
      }
      else if (comp->GetConnectionState() == SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_CONNECTED)
      {
         // cause the custom time to be updated.
         OnTimeOfDayChanged(mUi->mCustomTimeEdit->time());
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::PopulateEntityInfoWindow(bool notUsed)
   {
      QTableWidgetItem *currentItem = mUi->mSearchEntityTableWidget->currentItem();
      if(currentItem == NULL)
         return;

      PopulateEntityInfoWindow(currentItem);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::PopulateEntityInfoWindow(QTableWidgetItem *currentItem)
   {
      unsigned int index = (unsigned int)(mUi->mSearchEntityTableWidget->currentRow());
      if(index > mFoundActors.size() || currentItem == NULL)
         return;

      QString id = currentItem->data(Qt::UserRole).toString();

      // Retrieve proxy from the GM
      dtGame::GameActorProxy *proxy = mApp->GetGameManager()->FindGameActorById(id.toStdString());
      if(proxy != NULL)
      {
         UpdateEntityInfoData(proxy);

         // This is the special case. See also OnRefreshEntityInfoTimerElapsed() for similar code and explanation.
         if(mUi->mToolsAutoAttachOnSelectionCheckBox->checkState() == Qt::Checked)
         {
            StealthViewerData::GetInstance().GetGeneralConfigObject().AttachToActor(*proxy);
         }
      }
      else
      {
         ShowEntityErrorMessage(currentItem);
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackTimeMarkerSelected(const QString &text)
   {
      mUi->mPlaybackJumpToTimeMarkerPushButton->setEnabled(!text.isEmpty());
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnGenericTickTimerElapsed()
   {
      StealthGM::PreferencesEnvironmentConfigObject &envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      OnSecondTimerElapsed();

      float visMeters = envConfig.GetVisibilityDistance();

      mUi->mVisibilityLineEdit->setText(QString::number(visMeters) + tr(" KM"));

      mUi->mWeatherLineEdit->setText(tr(envConfig.GetPrecipitationAsString().c_str()));
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ReconnectToHLA()
   {
      // Support passing in the connection name on the command line
      int argc    = qApp->argc();
      char **argv = qApp->argv();

      // Parse the name from the command line
      osg::ArgumentParser parser(&argc, argv);
      parser.getApplicationUsage()->addCommandLineOption("-connectionName","The name of the connection to auto connect to. ");

      std::string name;
      if(!parser.read("-connectionName", name))
      {
         // If you do NOT read from the command line, see if it is stored in the
         // preferences file
         if(StealthViewerData::GetInstance().GetGeneralConfigObject().GetReconnectOnStartup())
         {
            name =
               StealthViewerData::GetInstance().GetGeneralConfigObject().GetStartupConnectionName();
         }
      }

      // Nothing either way, peace out and start like normal
      if(name.empty() || QString(name.c_str()).toLower() == "none")
         return;

      // Look up the properties for the name
      QString connectionName = tr(name.c_str());
      QStringList connectionProps =
         StealthViewerData::GetInstance().GetSettings().GetConnectionProperties(connectionName);

      // Was the name in the file or on the command line actually valid?
      if(!StealthViewerData::GetInstance().GetSettings().ContainsConnection(connectionName) ||
               connectionProps.isEmpty())
      {
         // Apparently not
         QString message = tr("The application failed to reconnect to the connection named: ") +
         connectionName + tr(" . Please select a new federation to connect to from the Network tab.");
         QMessageBox::critical(this, tr("Failed to reconnect to the federation"), message, QMessageBox::Ok);

         // Peace out
         return;
      }

      // Make sure we still pick up the signals from these events. This is important
      // for the UI to update itself properly
      HLAWindow window(*mApp->GetGameManager(), this, NULL, mIsConnectedToHLA, mCurrentConnectionName);

      connect(&window, SIGNAL(ConnectedToHLA(QString)), this, SLOT(OnConnectToHLA(QString)));
      connect(&window, SIGNAL(DisconnectedFromHLA()), this, SLOT(OnDisconnectFromHLA()));

      // Begin wait cursor
      StartWaitCursor();

      // Kind of hackish, but connect through the window like you normally would
      // to preserve signal delegation
      window.SetConnectionValues(connectionProps);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnRefreshEntityInfoTimerElapsed()
   {
      // In addition to refreshing the entity, we also update the 'Detach' button.
      bool attached = StealthViewerData::GetInstance().GetGeneralConfigObject().IsStealthActorCurrentlyAttached();
      mUi->mSearchDetachPushButton->setEnabled(attached);

      // Now, update the entity info window
      if(mUi->mEntityInfoAutoRefreshCheckBox->isChecked())
      {
         // Nasty duplicated code.
         // This is a quick fix to stop the entity info window from
         // refreshing and causing an attach to the current actor
         // if the box is selected.

         // Simply call the same code with no attach call instead
         // of calling the method in order not to break existing
         // functionality.

         // Remember, you cannot change the method signatures to take a
         // flag or anything because the signatures between a SIGNAL
         // and SLOT HAVE to match EXACTLY.

         // I will fix this at a later date with a more robust approach
         // - Eddie

         //PopulateEntityInfoWindow();
         QTableWidgetItem *currentItem = mUi->mSearchEntityTableWidget->currentItem();
         unsigned int index = (unsigned int)(mUi->mSearchEntityTableWidget->currentRow());
         if(index > mFoundActors.size() || currentItem == NULL)
            return;

         QString id = currentItem->data(Qt::UserRole).toString();

         // Retrieve proxy from the GM
         dtGame::GameActorProxy *proxy = mApp->GetGameManager()->FindGameActorById(id.toStdString());
         if(proxy != NULL)
         {
            UpdateEntityInfoData(proxy);
         }
         else
         {
            ShowEntityErrorMessage(currentItem);
         }
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ShowEntityErrorMessage(QTableWidgetItem *currentItem)
   {
      if(mShowMissingEntityInfoErrorMessage)
      {
         if(mUi->mEntityInfoAutoRefreshCheckBox->isChecked())
            mShowMissingEntityInfoErrorMessage = false;

         QString message =
            tr("Could not find info for the actor named: ") +
            ((currentItem == NULL) ? tr("Unknown") : currentItem->text()) +
            tr(" because this actor has been removed from the scenario. Please select another actor");

         QMessageBox::warning(this, tr("Error finding info for actor"), message, QMessageBox::Ok);
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::UpdateEntityInfoData(dtGame::GameActorProxy *proxy)
   {
      StealthGM::PreferencesToolsConfigObject &toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();
      const StealthGM::PreferencesToolsConfigObject::CoordinateSystem &system = toolsConfig.GetCoordinateSystem();

      ShowOrHideEntityInfoPositionFields(system);

      if(proxy != NULL)
      {
         std::ostringstream oss;
         // Get the StealthHUD so we can get the coordinate Converter. Makes our coordinates be location specific.
         StealthGM::StealthHUD* hudComponent = dynamic_cast<StealthGM::StealthHUD*>
         (mApp->GetGameManager()->GetComponentByName(StealthGM::StealthHUD::DEFAULT_NAME));
         if(hudComponent == NULL)
         {
            throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
                     "Failed to locate the StealthHUD Component on the Game Manager. Critical failure.",
                     __FILE__, __LINE__);
         }
         dtUtil::Coordinates& coordinateConverter = hudComponent->GetCoordinateConverter();

         // Decided not to use proxy->GetTranslation and proxy->GetRotation() based on David's
         // recommendation that GetRotation is screwy and will be refactored at some point because
         // it changes the HPR order to RHP.
         const dtCore::Transformable *t = static_cast<const dtCore::Transformable*>(proxy->GetActor());
         dtCore::Transform trans;
         t->GetTransform(trans, dtCore::Transformable::REL_CS);
         osg::Vec3 pos;
         trans.GetTranslation(pos);
         osg::Vec3 rot;
         trans.GetRotation(rot);
         //osg::Vec3 pos = proxy->GetTranslation(), rot = proxy->GetRotation();


         // For the 3 types of positions. We just go ahead and set all 3. It only happens once a
         // second and that way, if the user switches at any time, the data is already valid.
         //mUi->mEntityInfoPositionLineEdit->setText(tr(oss.str().c_str()));
         //oss << "X:" << pos[0] << " Y:" << pos[1] << " Z:" << pos[2];

         // Set the Lat Lon pos
         //coordinateConverter.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
         const osg::Vec3d& globePos = coordinateConverter.ConvertToRemoteTranslation(pos);
         oss.str("");
         oss << globePos[0];
         mUi->mEntityInfoPosLatEdit->setText(tr(oss.str().c_str()));
         oss.str("");
         oss << globePos[1];
         mUi->mEntityInfoPosLonEdit->setText(tr(oss.str().c_str()));

         // Set the MGRS pos
         std::string milgrid = coordinateConverter.XYZToMGRS(pos);
         mUi->mEntityInfoPositionMGRS->setText(tr(milgrid.c_str()));

         // Set the XYZ Pos
         oss.str("");
         oss << pos[0];
         mUi->mEntityInfoPosXEdit->setText(tr(oss.str().c_str()));
         oss.str("");
         oss << pos[1];
         mUi->mEntityInfoPosYEdit->setText(tr(oss.str().c_str()));
         oss.str("");
         oss << pos[2];
         mUi->mEntityInfoPosZEdit->setText(tr(oss.str().c_str()));


         // Heading
         float heading = ComputeHumanReadableDirection((float) rot[0]);
         oss.str("");
         oss << heading;
         mUi->mEntityInfoRotHeadEdit->setText(tr(oss.str().c_str()));
         // Pitch
         float pitch = ComputeHumanReadableDirection((float) rot[1]);
         oss.str("");
         oss << pitch;
         mUi->mEntityInfoRotPitchEdit->setText(tr(oss.str().c_str()));
         // Roll
         float roll = ComputeHumanReadableDirection((float) rot[2]);
         oss.str("");
         oss << roll;
         mUi->mEntityInfoRotRollEdit->setText(tr(oss.str().c_str()));

         // Calculate the directional speed from the entities velocity
         SimCore::Actors::BaseEntity *entity = dynamic_cast<SimCore::Actors::BaseEntity*>(proxy->GetActor());
         if (entity != NULL)
         {
            osg::Vec3 velocity = entity->GetVelocityVector();
            // speed is distance of velocity. Then, convert from m/s to MPH
            float speed = 2.237 * sqrt((float)(velocity[0] * velocity[0]) +
                     (float)(velocity[1] * velocity[1]) + (float)(velocity[2] * velocity[2]));
            speed = ((int)(speed * 100)) / 100.0f; // truncate to 2 decimal places
            oss.str("");
            oss << speed << " MPH";
            mUi->mEntityInfoSpeed->setText(tr(oss.str().c_str()));

            // compute the direction of movement
            float direction = atan2(-(float)velocity[0], (float) velocity[1]);
            direction = osg::RadiansToDegrees(direction);
            direction = ComputeHumanReadableDirection(direction);
            oss.str("");
            oss << direction;
            mUi->mEntityInfoSpeedDir->setText(tr(oss.str().c_str()));
         }
         else
         {
            mUi->mEntityInfoSpeed->setText(tr(""));
            mUi->mEntityInfoSpeedDir->setText(tr(""));
         }

         mUi->mEntityInfoCallSignLineEdit->setText(tr(proxy->GetName().c_str()));
         mUi->mEntityInfoForceLineEdit->setText(tr(proxy->GetProperty("Force Affiliation")->ToString().c_str()));
         mUi->mDamageStateLineEdit->setText(tr(proxy->GetProperty("Damage State")->ToString().c_str()));

         // NOTE: To avoid confusion.
         // Entity Type will write to Entity Type ID line edit
         // Mapping Name will write to Entity Type line edit.
         const dtDAL::ActorProperty* param
         = proxy->GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_ENTITY_TYPE);
         mUi->mEntityTypeIDLineEdit->setText(tr( param == NULL ? "" : param->ToString().c_str() ));

         param = proxy->GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_MAPPING_NAME);
         mUi->mEntityTypeLineEdit->setText(tr( param == NULL ? "" : param->ToString().c_str() ));

         // we hide the last update time now, since in reality, we can't use this field. The last update time
         // is really the last time that the entity trans or rotation was changed. But, if the entity is sitting
         // still, the last update time is never changed. This is further complicated when the sim time changes
         // after joining a federation, making this time something like 370000 seconds. It's just not helpful[
         //double lastUpdateTime = EntitySearch::GetLastUpdateTime(*proxy);
         //mUi->mEntityInfoLastUpdateTimeLineEdit->setText(QString::number(lastUpdateTime) + tr(" seconds ago"));

      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ShowOrHideEntityInfoPositionFields(const StealthGM::PreferencesToolsConfigObject::CoordinateSystem &system)
   {
      if (system == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::MGRS)
      {
         // Show the MGRS fields
         mUi->mEntityInfoPositionMGRS->show();
         mUi->mEntityInfoPositionLabel_MGRS->show();
         // Hide the XYZ fields
         mUi->mEntityInfoPositionLabel_XYZ->hide();
         mUi->mEntityInfoPos_XLabel->hide();
         mUi->mEntityInfoPosXEdit->hide();
         mUi->mEntityInfoPos_YLabel->hide();
         mUi->mEntityInfoPosYEdit->hide();
         mUi->mEntityInfoPos_ZLabel->hide();
         mUi->mEntityInfoPosZEdit->hide();
         // Hide the Lat Lon fields
         mUi->mEntityInfoPositionLabel_LatLon->hide();
         mUi->mEntityInfoPos_LatLabel->hide();
         mUi->mEntityInfoPosLatEdit->hide();
         mUi->mEntityInfoPos_LonLabel->hide();
         mUi->mEntityInfoPosLonEdit->hide();
      }
      else if (system == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::LAT_LON)
      {
         // Hide the MGRS fields
         mUi->mEntityInfoPositionMGRS->hide();
         mUi->mEntityInfoPositionLabel_MGRS->hide();
         // Show the XYZ fields
         mUi->mEntityInfoPositionLabel_XYZ->hide();
         mUi->mEntityInfoPos_XLabel->hide();
         mUi->mEntityInfoPosXEdit->hide();
         mUi->mEntityInfoPos_YLabel->hide();
         mUi->mEntityInfoPosYEdit->hide();
         mUi->mEntityInfoPos_ZLabel->hide();
         mUi->mEntityInfoPosZEdit->hide();
         // Hide the Lat Lon fields
         mUi->mEntityInfoPositionLabel_LatLon->show();
         mUi->mEntityInfoPos_LatLabel->show();
         mUi->mEntityInfoPosLatEdit->show();
         mUi->mEntityInfoPos_LonLabel->show();
         mUi->mEntityInfoPosLonEdit->show();
      }
      else // if (system == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ
      {
         // Hide the MGRS fields
         mUi->mEntityInfoPositionMGRS->hide();
         mUi->mEntityInfoPositionLabel_MGRS->hide();
         // Show the XYZ fields
         mUi->mEntityInfoPositionLabel_XYZ->show();
         mUi->mEntityInfoPos_XLabel->show();
         mUi->mEntityInfoPosXEdit->show();
         mUi->mEntityInfoPos_YLabel->show();
         mUi->mEntityInfoPosYEdit->show();
         mUi->mEntityInfoPos_ZLabel->show();
         mUi->mEntityInfoPosZEdit->show();
         // Hide the Lat Lon fields
         mUi->mEntityInfoPositionLabel_LatLon->hide();
         mUi->mEntityInfoPos_LatLabel->hide();
         mUi->mEntityInfoPosLatEdit->hide();
         mUi->mEntityInfoPos_LonLabel->hide();
         mUi->mEntityInfoPosLonEdit->hide();

      }
   }

   ///////////////////////////////////////////////////////////////////
   float MainWindow::ComputeHumanReadableDirection(float flippedOrientation)
   {
      float result = flippedOrientation;

      // convert to positive
      result = 360.0f - ((result < 0.0f) ? result + 360.0f : result);
      // remove the occasional 360
      result = (result >= 360.0) ? result - 360.0 : result;
      result = ((int)(result * 100)) / 100.0f; // truncate to 2 decimal places

      return result;
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnAutoRefreshEntityInfoCheckBoxChanged(int state)
   {
      bool isChecked = (state == Qt::Checked);
      if(isChecked)
         mShowMissingEntityInfoErrorMessage = true;
      StealthViewerData::GetInstance().GetGeneralConfigObject().SetAutoRefreshEntityInfoWindow(isChecked);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnTimeMarkerDoubleClicked(QListWidgetItem *item)
   {
      if(item != NULL)
      {
         OnPlaybackJumpToTimeMarkerButtonClicked(item->text());
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::SelectCorrectWarpToUI(dtUtil::Enumeration& enumValue)
   {
      mUi->mWarpToLatLonGroup->hide();
      mUi->mWarpToMGRSGroup->hide();
      mUi->mWarpToXYZGroup->hide();
      if(enumValue == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::MGRS)
      {
         mUi->mWarpToMGRSGroup->show();
      }
      else if(enumValue == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ)
      {
         mUi->mWarpToXYZGroup->show();
      }
      else if(enumValue == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::LAT_LON)
      {
         mUi->mWarpToLatLonGroup->show();
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ClearData()
   {
      // Search table results
      mUi->mSearchEntityTableWidget->clear();

      // Entity Info window
      mUi->mEntityInfoCallSignLineEdit->setText(tr(""));
      mUi->mEntityInfoSpeed->setText(tr(""));
      mUi->mEntityInfoSpeedDir->setText(tr(""));

      mUi->mEntityInfoForceLineEdit->setText(tr(""));
      //mUi->mEntityInfoLastUpdateTimeLineEdit->setText(tr("")); // hidden for now.
      mUi->mEntityInfoRotHeadEdit->setText(tr(""));
      mUi->mEntityInfoRotPitchEdit->setText(tr(""));
      mUi->mEntityInfoRotRollEdit->setText(tr(""));

      mUi->mEntityInfoPositionMGRS->setText(tr(""));
      // Hide the XYZ fields
      mUi->mEntityInfoPosXEdit->setText(tr(""));
      mUi->mEntityInfoPosYEdit->setText(tr(""));
      mUi->mEntityInfoPosZEdit->setText(tr(""));
      // Hide the Lat Lon fields
      mUi->mEntityInfoPosLatEdit->setText(tr(""));
      mUi->mEntityInfoPosLonEdit->setText(tr(""));
      //mUi->mEntityInfoPositionLineEdit->setText(tr("")); // replaced by the above fields
   }
}
