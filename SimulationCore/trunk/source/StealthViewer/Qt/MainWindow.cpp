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
#include <prefix/StealthQtPrefix.h>

// This has to be above the QT includes because 'emit' conflicts
#include <SimCore/Actors/BaseEntity.h>

#include <dtUtil/warningdisable.h>
DT_DISABLE_WARNING_ALL_START
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
#include <QtGui/QSpacerItem>
DT_DISABLE_WARNING_END

#include <dtQt/osgadapterwidget.h>
#include <dtQt/osggraphicswindowqt.h>
#include <dtQt/qtguiwindowsystemwrapper.h>

#include <StealthViewer/Qt/MainWindow.h>
#include <ui_MainWindowUi.h>
#include <StealthViewer/Qt/HLAWindow.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/EntitySearch.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/ViewDockWidget.h>
#include <StealthViewer/Qt/AdditionalViewDockWidget.h>
#include <StealthViewer/Qt/MapSelectDialog.h>

#include <StealthViewer/GMApp/StealthHUD.h>

#include <StealthViewer/GMApp/ViewerConfigComponent.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>

#include <SimCore/HLA/HLAConnectionComponent.h>
#include <SimCore/UnitEnums.h>
#include <SimCore/SimCoreVersion.h>
#include <SimCore/Utilities.h>

#include <dtUtil/stringutils.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/datetime.h>

#include <dtCore/camera.h>
#include <dtCore/system.h>
#include <dtCore/transformable.h>
#include <dtCore/transform.h>

#include <dtCore/project.h>
#include <dtCore/actorproperty.h>

#include <dtGame/gameapplication.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/gmcomponent.h>
#include <dtGame/logcontroller.h>
#include <dtGame/deadreckoninghelper.h>

#include <dtHLAGM/hlacomponent.h>

#include <dtUtil/warningdisable.h>
DT_DISABLE_WARNING_ALL_START
//So the Q_Object macro will work
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Matrixd>
#include <osg/Math>

#include <osgDB/FileNameUtils>
DT_DISABLE_WARNING_END

#include <cmath>
#include <cfloat>

#include <iostream>

namespace StealthQt
{
   class StealthWidgetFactory: public dtQt::GLWidgetFactory
   {
   public:
      ///Overwrite to generate a custom OSGAdapterWidget
      virtual dtQt::OSGAdapterWidget* CreateWidget(const QGLFormat& format, bool drawOnSeparateThread, QWidget* parent = NULL,
         const QGLWidget* shareWidget = NULL, Qt::WindowFlags f = 0)
      {
         static int count = 0;
         bool assignParent = false;
         if (count == 0)
         {
            assignParent = true;
         }
         else
         {
            f |= Qt::Tool;
            //f |= Qt::WindowStaysOnTopHint;
         }
         ++count;

         dtQt::OSGAdapterWidget* widget = NULL;
         if (assignParent)
         {
            mParent = new StealthQt::AdditionalViewDockWidget(format, parent, shareWidget, f);
            widget = mParent;

// AT last check, in windows, it had windowing problems on Win32 if mParent is NULL.
// In X11, the parnent MUST be null or the GLXContext can't be created for the extra views.
// On Mac OS X, it doesn't seem to make any different whether the parent of the extra windows is NULL or the main window.
#ifdef Q_WS_X11
            mParent = NULL;
#endif
         }
         else
         {
            widget = new StealthQt::AdditionalViewDockWidget(format, mParent, shareWidget, f);
         }

         return widget;
      }

      dtQt::OSGAdapterWidget* mParent;
   };

   ///////////////////////////////////////////////////////////////////////////////
   static std::string WINDOW_TITLE_VERSION("[Unknown]");
   static const std::string WINDOW_TITLE( "Stealth Viewer");
   static const std::string WINDOW_TITLE_MODE_PLAYBACK( " [Playback Mode]" );
   static const std::string WINDOW_TITLE_MODE_RECORD( " [Record Mode]" );

   ///////////////////////////////////////////////////////////////////////////////
   MainWindow::MainWindow(int appArgc, char* appArgv[], const std::string& appLibName)
   : mIsPlaybackMode(false)
   , mIsRecording(false)
   , mIsPlayingBack(false)
   , mRecordingStartTime(0.0)
   , mIsConnectedToANetwork(false)
   , mLatValidator(new QDoubleValidator(-90, 90, 10, this))
   , mLonValidator(new QDoubleValidator(-180, 180, 10, this))
   , mXYZValidator(new QDoubleValidator(-DBL_MAX, DBL_MAX, 10, this))
   , mGtZeroValidator(new QDoubleValidator(0, DBL_MAX, 5, this))
   , mShowMissingEntityInfoErrorMessage(true)
   , mPreviousCustomHour(-1)
   , mPreviousCustomMinute(-1)
   , mPreviousCustomSecond(-1)
   , mViewDockWidget(new ViewDockWidget)
   , mUi(new Ui::MainWindow)
   {
      mUi->setupUi(this);
      addDockWidget(Qt::LeftDockWidgetArea, mViewDockWidget);

      ConnectSlots();

      ParseCommandLine();

      // Instantiate singletons
      StealthViewerData::GetInstance().SetMainWindow(*this);

      PreShowUIControlInit();

      InitGameApp(appArgc, appArgv, appLibName);

      QWidget* glParent = new QWidget(this);

      QHBoxLayout* hbLayout = new QHBoxLayout(glParent);
      hbLayout->setMargin(0);
      glParent->setLayout(hbLayout);
      setCentralWidget(glParent);

      show();

      dtQt::OSGGraphicsWindowQt* graphicsWindow = dynamic_cast<dtQt::OSGGraphicsWindowQt*>(mGM->GetApplication().GetWindow()->GetOsgViewerGraphicsWindow());
      if (graphicsWindow != NULL)
      {
         QGLWidget* oglWidget = graphicsWindow->GetQGLWidget();
         if (oglWidget != NULL)
         {
            QRect r = oglWidget->geometry();
            graphicsWindow->resized(r.left(), r.top(), r.width(), r.height());
            //oglWidget->hide();
            hbLayout->addWidget(oglWidget);
            oglWidget->show();
         }
      }

      // This was coverted to a png from a jpg because of weird loading problems
      // on Windows XP
      dtCore::ResourceDescriptor helpImageResource("icons:help_controls_small.png");
      const std::string file = dtCore::Project::GetInstance().GetResourcePath(helpImageResource);
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
         LOG_ERROR("Couldn't find camera help image \"" + helpImageResource.GetResourceIdentifier() + "\".");
      }

      dtCore::ResourceDescriptor iconImageResource("icons:stealthviewer.png");
      const std::string iconFile = dtCore::Project::GetInstance().GetResourcePath(iconImageResource);
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
         LOG_ERROR("Couldn't find app icon \"" + iconImageResource.GetResourceIdentifier() + "\".");
      }

      AddConfigObjectsToViewerComponent();

      ConnectSigSlots();

      ReconnectToHLA();

      // hide the last update time fields until it can be useful (see comment in UpdateEntityInfoData())
      mUi->mEntityInfoLastUpdateTimeLineEdit->hide();
      mUi->mEntityInfoLastUpdateTimeLabel->hide();

      //Init the coordinate type.
      OnToolsCoordinateSystemChanged(mUi->mOptionsCoordinateSystemComboBox->currentText());

      mSimTicker.Start();
   }

   //////////////////////////////////////////////////////////
   ViewDockWidget& MainWindow::GetViewDockWidget()
   {
      return *mViewDockWidget;
   }

   //////////////////////////////////////////////////////////
   void MainWindow::ParseCommandLine()
   {
      // Support passing in the connection name on the command line
      int argc    = qApp->argc();
      char **argv = qApp->argv();

      // Parse the name from the command line
      osg::ArgumentParser parser(&argc, argv);
      parser.getApplicationUsage()->addCommandLineOption("-connectionName", "The name of the connection to auto connect to. ");
      parser.getApplicationUsage()->addCommandLineOption("-configurationName", "The name of the configuration settings to use.  "
               "This allows having separate sets of preferences on the same computer/login.");
      parser.getApplicationUsage()->addCommandLineOption("-h", "Print out this help information.");

      if (parser.read("-h"))
      {
         parser.getApplicationUsage()->write(std::cout, parser.getApplicationUsage()->getCommandLineOptions());
      }

      std::string name;
      if (parser.read("-configurationName", name))
      {
         StealthViewerData::GetInstance().ChangeSettingsInstance(name);
      }

      if (parser.read("-connectionName", name))
      {
         // Store the name here so it can be picked up later.
         // It's not really the "current" connection name.
         mCurrentConnectionName = name.c_str();
      }

   }

   //////////////////////////////////////////////////////////
   void MainWindow::PreShowUIControlInit()
   {
      mUi->mControlsDockWidget->installEventFilter(this);
      mUi->mEntityInfoDockWidget->installEventFilter(this);
      mUi->mPreferencesDockWidget->installEventFilter(this);
      mViewDockWidget->installEventFilter(this);
      mUi->mSearchCallSignLineEdit->installEventFilter(this);
      mUi->mSearchEntityTableWidget->installEventFilter(this);

      // Only hide panels for subsequent launches of the viewer
      // from the initial fist time launch. Some users may want
      // the the window to load at certain size, location and dock
      // configuration. Having some docks visible by default may
      // disturb the intended window dimensions.
      if ( ! StealthViewerData::GetInstance().GetSettings().HasSavedData())
      {
         mUi->mControlsDockWidget->hide();
         mUi->mEntityInfoDockWidget->hide();
         mUi->mPreferencesDockWidget->hide();
      }

      //mUi->mGeneralAdvancedPerformanceOptionsGroupBox->hide();
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
      mViewDockWidget->setEnabled(false);

      // Cannot start up in playback mode either
      EnablePlaybackButtons(false);

      WINDOW_TITLE_VERSION = " - [Rev " + SimCore::GetSimCoreRevision();
      if (!SimCore::GetSimCoreBuildDate().empty())
         WINDOW_TITLE_VERSION += ", " + SimCore::GetSimCoreBuildDate().substr(0, 10);
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

      mHLAErrorTimer.setInterval(1000);
      mHLAErrorTimer.setSingleShot(true);

      mUi->mWarpToLat->setValidator(mLatValidator);
      mUi->mWarpToLon->setValidator(mLonValidator);
      mUi->mWarpToLLElev->setValidator(mXYZValidator);

      mUi->mWarpToX->setValidator(mXYZValidator);
      mUi->mWarpToY->setValidator(mXYZValidator);
      mUi->mWarpToZ->setValidator(mXYZValidator);

      mUi->mWarpToMGRSElev->setValidator(mXYZValidator);

      mUi->mGeneralLODScaleLineEdit->setValidator(mGtZeroValidator);

      //The azimuth has the same range as a lat / lon
      mUi->mAttachAzimuth->setValidator(mLonValidator);

      mUi->mControlsTabWidget->setUsesScrollButtons(true);

      mGenericTickTimer.setInterval(5000);
      mGenericTickTimer.setSingleShot(false);
      mGenericTickTimer.start();

      mRefreshEntityInfoTimer.setInterval(1000);
      mRefreshEntityInfoTimer.setSingleShot(false);
      mRefreshEntityInfoTimer.start();

      // Disable full screen
      //mUi->mMenuWindow->removeAction(mUi->mActionFullScreen);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::InitGameApp(int appArgc, char* appArgv[],
            const std::string& appLibName)
//   void MainWindow::InitGameApp(QGLWidget& oglWidget, int appArgc, char* appArgv[],
//            const std::string& appLibName)
   {
      ///Reset the windowing system for osg to use
      osg::GraphicsContext::WindowingSystemInterface* winSys = osg::GraphicsContext::getWindowingSystemInterface();

      if (winSys != NULL)
      {
         dtQt::QtGuiWindowSystemWrapper* wsw = new dtQt::QtGuiWindowSystemWrapper(*winSys);
         wsw->SetGLWidgetFactory(new StealthWidgetFactory);
         osg::GraphicsContext::setWindowingSystemInterface(wsw);
      }

      try
      {
         mGameLoader = new dtGame::GameApplicationLoader(appArgc, appArgv);
         mGameLoader->SetGameLibraryName(appLibName);
         mGameLoader->Config();

         mGM = mGameLoader->GetGameManager();
         StealthViewerData::GetInstance().GetViewWindowConfigObject().CreateMainViewWindow(*mGM);
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
      mSimTicker.Stop();
      for (size_t i =0; i < mVisibilityCheckBoxes.size(); ++i)
      {
         mVisibilityCheckBoxes[i]->setUserData(0, NULL);
      }

      delete mUi;
      mUi = NULL;
      delete mViewDockWidget;
      mViewDockWidget = NULL;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnMapWindowActionTriggered()
   {
      if (mIsConnectedToANetwork)
      {
         QMessageBox::warning(this, "Currently connected", QString("Changing maps while a network connection is active is not supported.  Disconnect first."));
         return;
      }

      MapSelectDialog dlg(this);
      if(dlg.exec() == QDialog::Accepted)
      {
         QListWidgetItem* item = dlg.GetSelectedItem();
         std::string mapName = item->text().toStdString();
         SimCore::Utils::LoadMaps(*mGM, mapName);
         EnableGeneralUIAndTick();
      }

   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnConnectionWindowActionTriggered()
   {
      SimCore::HLA::HLAConnectionComponent* comp = NULL;
      mGM->GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME, comp);

      if (comp == NULL)
      {
         QMessageBox::critical(this, "Error", QString("Network connection subsystem failed to initialized.  The internal ConnectionComponent does not exist."));
      }

      // Even error states should be considered connected so that the UI will make you disconnect first.
      mIsConnectedToANetwork = comp->GetConnectionState() !=
         SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED;

      //mHLAErrorTimer.stop();
      HLAWindow window(*mGM, this, NULL, mIsConnectedToANetwork, mCurrentConnectionName);

      connect(&window, SIGNAL(ConnectedToNetwork(QString)), this, SLOT(OnConnectToNetwork(QString)));
      connect(&window, SIGNAL(ConnectedToNetworkFailed(QString)), this, SLOT(OnConnectToNetworkFailed(QString)));
      connect(&window, SIGNAL(DisconnectedFromNetwork(bool)), this, SLOT(OnDisconnectFromNetwork(bool)));

      if (window.exec() == QDialog::Accepted)
      {
         // Retrieve data from labels and process the input.
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::showEvent(QShowEvent* event)
   {
      mViewDockWidget->show();
      // The first, initial show event is not spontaneous because Qt sends it when the window is
      // made visible.  Minimizing or otherwise hiding windows and bringing them back can cause
      // spontaneous show events, which we don't want.
      if (!event->spontaneous())
      {
         StealthViewerData::GetInstance().GetSettings().LoadPreferences();
         UpdateUIFromPreferences();

         mUi->mActionShowControls->setChecked(mUi->mControlsDockWidget->isVisible());
         mUi->mActionShowEntityInfo->setChecked(mUi->mEntityInfoDockWidget->isVisible());
         mUi->mActionShowPreferences->setChecked(mUi->mPreferencesDockWidget->isVisible());
         mUi->mActionShowViewUI->setChecked(mViewDockWidget->isVisible());
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::closeEvent(QCloseEvent* e)
   {
      StealthViewerData::GetInstance().GetSettings().WritePreferencesToFile();

      StealthGM::ViewWindowConfigObject& viewConfig =
         StealthViewerData::GetInstance().GetViewWindowConfigObject();

      std::vector<StealthGM::ViewWindowWrapper*> viewWindows;
      viewConfig.GetAllViewWindows(viewWindows);
      std::vector<StealthGM::ViewWindowWrapper*>::iterator i, iend;
      i = viewWindows.begin();
      iend = viewWindows.end();
      for (; i != iend; ++i)
      {
         StealthGM::ViewWindowWrapper* vww = *i;
         AdditionalViewDockWidget* widget = AdditionalViewDockWidget::GetDockWidgetForViewWindow(*vww);
         if (widget != NULL)
         {
            widget->RequestClose();
         }
      }

      // Tick once to ensure all sub-windows/tool windows,
      // such as additional OSG views, have a chance to free
      // their contexts before the application ends. This
      // prevents a crash caused by out of order shutdown,
      // where OSG is too late to cleanup as the application
      // would have already ended.
      dtCore::System::GetInstance().Step();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::ConnectSlots()
   {
      /////////////////////////////////////////////////////////
      // Window Actions
      /////////////////////////////////////////////////////////
      connect(mUi->mActionMaps,            SIGNAL(triggered()), this, SLOT(OnMapWindowActionTriggered()));
      connect(mUi->mActionConnections,     SIGNAL(triggered()), this, SLOT(OnConnectionWindowActionTriggered()));
      connect(mUi->mActionFullScreen,      SIGNAL(triggered()), this, SLOT(OnFullScreenActionTriggered()));
      connect(mUi->mActionShowControls,    SIGNAL(triggered()), this, SLOT(OnShowControlsActionTriggered()));
      connect(mUi->mActionShowEntityInfo,  SIGNAL(triggered()), this, SLOT(OnShowEntityInfoActionTriggered()));
      connect(mUi->mActionShowPreferences, SIGNAL(triggered()), this, SLOT(OnShowPreferencesActionTriggered()));
      connect(mUi->mActionShowViewUI,      SIGNAL(triggered()), this, SLOT(OnShowViewUIActionTriggered()));
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

      connect(mUi->mPlaybackLoopCheckBox, SIGNAL(stateChanged(int)),
               this,                      SLOT(OnLoopContinuouslyChanged(int)));

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

      connect(mUi->mAttachNodeName,      SIGNAL(textChanged(const QString&)),
               this,                     SLOT(OnAttachNodeNameChanged(const QString&)));

      connect(mUi->mAttachAzimuth,       SIGNAL(textChanged(const QString&)),
               this,                     SLOT(OnAttachAzimuthChanged(const QString&)));

      connect(mUi->mAutoAttachCheckBox,  SIGNAL(toggled(bool)),
               this,                     SLOT(OnAutoAttachToggled(bool)));

      connect(mUi->mAutoAttachCallsign, SIGNAL(textChanged(const QString&)),
               this,                              SLOT(OnAutoAttachEntityNameChanged(const QString&)));

      connect(mUi->mGeneralEnableCameraCollisionCheckBox, SIGNAL(stateChanged(int)),
               this,                                      SLOT(OnCameraCollisionChanged(int)));

      connect(mUi->mGeneralLODScaleLineEdit,              SIGNAL(textChanged(const QString&)),
               this,                                      SLOT(OnLODScaleChanged(const QString&)));

      connect(mUi->mGeneralNearClippingPlaneComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                                  SLOT(OnNearClippingPlaneChanged(const QString&)));

      connect(mUi->mGeneralFarClippingPlaneComboBox,  SIGNAL(currentIndexChanged(const QString&)),
               this,                                  SLOT(OnFarClipplingPlaneChanged(const QString&)));

      connect(mUi->mDistanceUnitCombo,  SIGNAL(currentIndexChanged(const QString&)),
               this,                                  SLOT(OnUnitOfLengthChanged(const QString&)));

      connect(mUi->mAngleUnitCombo,  SIGNAL(currentIndexChanged(const QString&)),
               this,                                  SLOT(OnUnitOfAngleChanged(const QString&)));

      connect(mUi->mOptionsCoordinateSystemComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                                 SLOT(OnToolsCoordinateSystemChanged(const QString&)));

      connect(mUi->mToolsMagnificationSpinBox, SIGNAL(valueChanged(int)),
               this,                           SLOT(OnMagnificationChanged(int)));

      connect(mUi->mToolsAutoAttachOnSelectionCheckBox, SIGNAL(stateChanged(int)),
               this,                                    SLOT(OnAutoAttachOnSelectionChanged(int)));

      connect(mUi->mToolsShowCompass360CheckBox, SIGNAL(stateChanged(int)),
               this,                             SLOT(OnShowCompass360Changed(int)));

      connect(mUi->mToolsShowBinocularImageCheckBox, SIGNAL(stateChanged(int)),
               this,                                 SLOT(OnShowBinocularImageChanged(int)));

      connect(mUi->mToolsShowDistanceToObjectCheckBox, SIGNAL(stateChanged(int)),
               this,                                   SLOT(OnShowDistanceToObjectChanged(int)));

      connect(mUi->mToolsShowElevationOfObjectCheckBox, SIGNAL(stateChanged(int)),
               this,                                    SLOT(OnShowElevationOfObjectChanged(int)));

      connect(mUi->mWeatherUseNetworkSettingsRadioButton, SIGNAL(clicked(bool)),
               this,                                      SLOT(OnWeatherNetworkRadioButtonClicked(bool)));

      connect(mUi->mWeatherCustomRadioButton, SIGNAL(clicked(bool)),
               this,                          SLOT(OnWeatherCustomRadioButtonClicked(bool)));

      connect(mUi->mWeatherThemedRadioButton, SIGNAL(clicked(bool)),
               this,                          SLOT(OnWeatherThemedRadioButtonClicked(bool)));

      connect(mUi->mCustomTimeEdit, SIGNAL(timeChanged(const QTime&)),
               this,                SLOT(OnTimeOfDayChanged(const QTime&)));

      connect(mUi->mCustomVisibilityComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                          SLOT(OnVisibilityChanged(const QString&)));

      connect(mUi->mCustomCloudCoverComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,                          SLOT(OnCloudCoverChanged(const QString&)));

      connect(mUi->mTimeComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,              SLOT(OnTimeThemeChanged(const QString&)));

      connect(mUi->mThemeComboBox, SIGNAL(currentIndexChanged(const QString&)),
               this,               SLOT(OnWeatherThemeChanged(const QString&)));

      connect(mUi->mSearchInfoPushButton, SIGNAL(clicked(bool)),
               this,                      SLOT(PopulateEntityInfoWindowDontAttach(bool)));

      connect(mUi->mSearchEntityTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
               this,                         SLOT(PopulateEntityInfoWindowAndAttach(QTableWidgetItem*)));

      connect(mUi->mSearchSearchPushButton, SIGNAL(clicked(bool)),
               this,                         SLOT(OnEntitySearchSearchButtonClicked(bool)));

      connect(mUi->mSearchAttachPushButton, SIGNAL(clicked(bool)),
               this,                        SLOT(OnEntitySearchAttachButtonClicked(bool)));

      connect(mUi->mSearchDetachPushButton, SIGNAL(clicked(bool)),
               this,                        SLOT(OnEntitySearchDetachButtonClicked(bool)));

      connect(mUi->mEntityInfoAutoRefreshCheckBox, SIGNAL(stateChanged(int)),
               this,                               SLOT(OnAutoRefreshEntityInfoCheckBoxChanged(int)));

      connect(mUi->mPlaybackTimeMarkersTextBox, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
               this,                            SLOT(OnTimeMarkerDoubleClicked(QListWidgetItem*)));

      connect(mUi->mVisLabelGroup, SIGNAL(toggled(bool)),
               this,               SLOT(OnVisLabelsToggled(bool)));
      connect(mUi->mVisTrackChk,   SIGNAL(toggled(bool)),
               this,               SLOT(OnVisLabelsToggled(bool)));
      connect(mUi->mVisBlipChk,    SIGNAL(toggled(bool)),
               this,               SLOT(OnVisLabelsToggled(bool)));
      connect(mUi->mVisEntityChk,  SIGNAL(toggled(bool)),
               this,               SLOT(OnVisLabelsToggled(bool)));
      connect(mUi->mVisMaxDistCombo, SIGNAL(currentIndexChanged(const QString&)),
               this,                 SLOT(OnVisLabelsDistanceChanged(const QString&)));

      connect(mUi->mVisShowPlatforms,  SIGNAL(toggled(bool)),
               this,               SLOT(OnVisibilityOptionToggled(bool)));
      connect(mUi->mVisShowHumans,  SIGNAL(toggled(bool)),
               this,               SLOT(OnVisibilityOptionToggled(bool)));
      connect(mUi->mVisShowTracks,  SIGNAL(toggled(bool)),
               this,               SLOT(OnVisibilityOptionToggled(bool)));
      connect(mUi->mVisShowBlips,  SIGNAL(toggled(bool)),
               this,               SLOT(OnVisibilityOptionToggled(bool)));
      connect(mUi->mVisBFGCloseTops,  SIGNAL(toggled(bool)),
               this,               SLOT(OnVisibilityOptionToggled(bool)));
      connect(mUi->mVisShowBFG,  SIGNAL(toggled(bool)),
               this,               SLOT(OnVisibilityOptionToggled(bool)));

      ////////////////////////////////////////////////////

      connect(&mDurationTimer, SIGNAL(timeout()), this, SLOT(OnDurationTimerElapsed()));

      connect(&mGenericTickTimer, SIGNAL(timeout()), this, SLOT(OnGenericTickTimerElapsed()));

      connect(&mRefreshEntityInfoTimer, SIGNAL(timeout()), this, SLOT(OnRefreshEntityInfoTimerElapsed()));

      connect(&mHLAErrorTimer, SIGNAL(timeout()), this, SLOT(OnHLAErrorTimerElapsed()));

   }

   ///////////////////////////////////////////////////////////////////////////////
   bool MainWindow::eventFilter(QObject *object, QEvent *event)
   {
      if (event->type() == QEvent::Close)
      {
         if (object == mUi->mControlsDockWidget)
         {
            mUi->mActionShowControls->setChecked(false);
            return true;
         }
         else if (object == mUi->mEntityInfoDockWidget)
         {
            mUi->mActionShowEntityInfo->setChecked(false);
            return true;
         }
         else if (object == mUi->mPreferencesDockWidget)
         {
            mUi->mActionShowPreferences->setChecked(false);
            return true;
         }
         else if (object == mViewDockWidget)
         {
            mUi->mActionShowViewUI->setChecked(false);
            return true;
         }
      }
      else if (event->type() == QEvent::KeyPress)
      {
         QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
         if (object == mUi->mSearchCallSignLineEdit)
         {
            if (keyEvent->key() == Qt::Key_Return)
            {
               OnEntitySearchSearchButtonClicked();
               return true;
            }
         }
         else if (object == mUi->mSearchEntityTableWidget)
         {
            if (keyEvent->key() == Qt::Key_T)
            {
               Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
               if (modifiers & Qt::ControlModifier)
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
      if (mUi->mActionFullScreen->isChecked())
      {
         this->showFullScreen();
      }
      else
      {
         this->showNormal();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowControlsActionTriggered()
   {
      bool showWindow = mUi->mActionShowControls->isChecked();

      showWindow ? mUi->mControlsDockWidget->show() : mUi->mControlsDockWidget->hide();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowEntityInfoActionTriggered()
   {
      bool showWindow = mUi->mActionShowEntityInfo->isChecked();

      showWindow ? mUi->mEntityInfoDockWidget->show() : mUi->mEntityInfoDockWidget->hide();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowPreferencesActionTriggered()
   {
      bool showWindow = mUi->mActionShowPreferences->isChecked();

      showWindow ? mUi->mPreferencesDockWidget->show() : mUi->mPreferencesDockWidget->hide();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowViewUIActionTriggered()
   {
      bool showWindow = mUi->mActionShowViewUI->isChecked();

      showWindow ? mViewDockWidget->show() : mViewDockWidget->hide();
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
      catch (const dtUtil::CoordinateConversionInvalidInput& ex)
      {
         QMessageBox::critical(this, "Error", QString(ex.What().c_str()));
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
      catch (const dtUtil::CoordinateConversionInvalidInput& ex)
      {
            QMessageBox::information(this, "Invalid Input", QString(ex.What().c_str()));
      }
      catch (const dtUtil::Exception& ex)
      {
         QMessageBox::critical(this, "Error", QString(ex.What().c_str()));
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
      catch (const dtUtil::CoordinateConversionInvalidInput& ex)
      {
         QMessageBox::critical(this, "Error", QString(ex.What().c_str()));
      }
   }

   void MainWindow::OnRecordStartButtonClicked(bool checked)
   {
      StealthGM::ControlsRecordConfigObject& recordObject =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      if (recordObject.GetOutputFilename().empty())
      {
         QMessageBox::warning(this, tr("Please select an output file"),
                  tr("Please select an output file to send record data to."),
                  QMessageBox::Ok);

         return;
      }

      mIsRecording = !mIsRecording;

      if (mIsRecording)
      {
         recordObject.StartRecording();
         mUi->mRecordStartButton->setText(tr("Stop"));
         mUi->mRecordDurationLineEdit->setText("0");
         mDurationTimer.start();
         mRecordingStartTime = mGM->GetSimulationTime();
         mRecordingStopTime = mGM->GetSimulationTime();
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
      mUi->mMenuSetup->setEnabled( ! mIsRecording );

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

      if (msgFile.isEmpty())
         return;

      dtUtil::FileUtils &instance = dtUtil::FileUtils::GetInstance();
      if (!instance.FileExists(msgFile.toStdString()))
      {
         // The file selected does not exist.
         // So, create it and prompt if the create fails.
         std::ofstream out(msgFile.toStdString().c_str());
         if (!out.is_open())
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

      if (!msg.empty())
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

      if (mIsPlaybackMode)
      {
         recConfig.DisconnectFromNetwork();
         mUi->mPlaybackOptionsGroupBox->show();
         mUi->mPlaybackSwitchToPlaybackModePushButton->setText(tr("End Playback Mode"));
      }
      else
      {
         // Turn off paused in case the playback ended and paused the GM.
         // It will continue to be paused afterward.
         if (mGM->IsPaused())
            mGM->SetPaused(false);

         recConfig.JoinNetwork();
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

      if (!mIsPlaybackMode)
      {
         if (mUi->mPlaybackShowAdvancedOptionsCheckBox->checkState() == Qt::Checked)
            mUi->mPlaybackTimeMarkersGroupBox->hide();
      }
      else
      {
         if (mUi->mPlaybackShowAdvancedOptionsCheckBox->checkState() == Qt::Checked)
            mUi->mPlaybackTimeMarkersGroupBox->show();
      }

      if (mIsPlaybackMode)
      {
         //if (recConfig.GetIsRecording())
         {
            //recConfig.StopRecording();
         }
      }

      // Restore default state. Exited without clicking Stop
      if (mIsPlayingBack)
         OnPlaybackPlayButtonClicked();

      // Prevent network connection changes if in playback mode.
      mUi->mMenuSetup->setEnabled( ! mIsPlaybackMode );

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

      if (msgFile.isEmpty())
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

      if (pbObject.GetInputFilename().empty())
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

      if (mUi->mPlaybackFileLineEdit->text().isEmpty())
      {
         QMessageBox::warning(this, tr("Please select an input file"),
                  tr("Please select an input file that contains record data to playback"),
                  QMessageBox::Ok);

         return;
      }

      mIsPlayingBack = !mIsPlayingBack;

      if (mIsPlayingBack)
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
      if (state == Qt::Checked && mIsPlayingBack)
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
   void MainWindow::OnLoopContinuouslyChanged(int state)
   {
      if (mIsPlayingBack)
      {
         StealthGM::ControlsPlaybackConfigObject &pbObject =
            StealthViewerData::GetInstance().GetPlaybackConfigObject();
         pbObject.SetLoopContinuously(state == Qt::Checked);
      }
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
      if (currentItem != NULL)
      {
         OnPlaybackJumpToTimeMarkerButtonClicked(currentItem->text());
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnPlaybackJumpToTimeMarkerButtonClicked(const QString &itemName)
   {
      if (!itemName.isEmpty())
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
               *mGM,
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
      if (row >= mFoundActors.size())
         return;

      // Get the name item from the current row, which stores the unique ID as
      // its data.
      QTableWidgetItem *item = mUi->mSearchEntityTableWidget->currentItem();
      if (item != NULL)
      {
         const dtCore::UniqueId id(item->data(Qt::UserRole).toString().toStdString());

         // Retrieve proxy from the GM just to make sure it exists.
         dtGame::GameActorProxy* proxy = mGM->FindGameActorById(id);
         if (proxy != NULL)
         {
            StealthViewerData::GetInstance().GetGeneralConfigObject().AttachToActor(id);
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
      StealthGM::PreferencesGeneralConfigObject& genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      StealthGM::PreferencesGeneralConfigObject::AttachMode* mode =
         StealthGM::PreferencesGeneralConfigObject::AttachMode::GetValueForName(text.toStdString());
      if (mode != NULL)
      {
         genConfig.SetAttachMode(*mode);
      }
      else
      {
         QString message = tr("Unknown attach mode: \"") +
            (text) +
            tr("\". This implies a coding error");

         QMessageBox::warning(this, tr("Error setting attach mode"), message, QMessageBox::Ok);

         genConfig.SetAttachMode(StealthGM::PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAttachNodeNameChanged(const QString& text)
   {
      StealthGM::PreferencesGeneralConfigObject& genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetAttachPointNodeName(text.toStdString());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAttachAzimuthChanged(const QString& text)
   {
      StealthGM::PreferencesGeneralConfigObject& genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      // We use the negative azimuth because the military does azimuth clockwise while hpr has a
      // counter-clockwise heading.
      genConfig.SetInitialAttachRotationHPR(osg::Vec3(-(text.toFloat()), 0.0, 0.0));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAutoAttachToggled(bool checked)
   {
      StealthGM::PreferencesGeneralConfigObject& genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetShouldAutoAttachToEntity(checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAutoAttachEntityNameChanged(const QString& text)
   {
      StealthGM::PreferencesGeneralConfigObject& genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      genConfig.SetAutoAttachEntityCallsign(text.toStdString());
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
      StealthGM::ViewWindowConfigObject& viewConfig =
         StealthViewerData::GetInstance().GetViewWindowConfigObject();

      viewConfig.SetNearClippingPlane(text.toDouble());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnFarClipplingPlaneChanged(const QString &text)
   {
      StealthGM::ViewWindowConfigObject& viewConfig =
         StealthViewerData::GetInstance().GetViewWindowConfigObject();

      viewConfig.SetFarClippingPlane(text.toDouble());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnUnitOfLengthChanged(const QString& text)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetLengthUnit(text.toStdString());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnUnitOfAngleChanged(const QString& text)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetAngleUnit(text.toStdString());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnWeatherThemedRadioButtonClicked(bool checked)
   {
      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
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
   void MainWindow::OnWeatherThemeChanged(const QString& text)
   {
//      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
//         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      QString hack = tr("Theme ") + text;

      // TODO reimplement
   }

   void MainWindow::OnTimeThemeChanged(const QString& text)
   {
//      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
//         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      QString hack = tr("Time ") + text;

      // TODO reimplement
   }

   ///////////////////////////////////////////////////////////////////////////////
   // Custom Weather Settings
   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnTimeOfDayChanged(const QTime& newTime)
   {
      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
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
   void MainWindow::OnVisibilityChanged(const QString& text)
   {
//      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
//         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      // Convert to match the name of the actor's enum. The full name
      // doesn't look pretty in the UI, so we shortened it
      QString hack = "Visibility " + text;

      // TODO reimplement
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnCloudCoverChanged(const QString &text)
   {
//      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
//         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      const std::string cloudCoverName = text.toStdString();
      // TODO reimplement
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnToolsCoordinateSystemChanged(const QString& text)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      const std::string coordType = text.toStdString();
      StealthGM::PreferencesToolsConfigObject::CoordinateSystem* coordSystem =
         StealthGM::PreferencesToolsConfigObject::CoordinateSystem::GetValueForName(coordType);
      if (coordSystem!= NULL)
      {
         toolsConfig.SetCoordinateSystem(*coordSystem);
         SelectCorrectWarpToUI(*coordSystem);
         ShowOrHideEntityInfoPositionFields(*coordSystem);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowCompass360Changed(int state)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetShowCompass360(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowBinocularImageChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetShowBinocularImage(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowDistanceToObjectChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetShowDistanceToObject(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnShowElevationOfObjectChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetShowElevationOfObject(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnMagnificationChanged(int value)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetMagnification(float(value));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnAutoAttachOnSelectionChanged(int state)
   {
      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      toolsConfig.SetAutoAttachOnSelection(state == Qt::Checked);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::AddConfigObjectsToViewerComponent()
   {
      dtGame::GameManager* gm = mGM;
      dtGame::GMComponent* gmComp =
         gm->GetComponentByName(StealthGM::ViewerConfigComponent::DEFAULT_NAME);

      if (gmComp == NULL)
      {
         LOG_ERROR("Failed to find the ViewerConfigComponent on the Game Manager.");
      }

      StealthGM::ViewerConfigComponent& viewComp =
         static_cast<StealthGM::ViewerConfigComponent&>(*gmComp);

      StealthViewerData &instance = StealthViewerData::GetInstance();

      viewComp.AddConfigObject(instance.GetEnvironmentConfigObject());
      viewComp.AddConfigObject(instance.GetGeneralConfigObject());
      viewComp.AddConfigObject(instance.GetToolsConfigObject());
      viewComp.AddConfigObject(instance.GetVisibilityConfigObject());

      viewComp.AddConfigObject(instance.GetCameraConfigObject());
      viewComp.AddConfigObject(instance.GetRecordConfigObject());
      viewComp.AddConfigObject(instance.GetPlaybackConfigObject());
      viewComp.AddConfigObject(instance.GetViewWindowConfigObject());
   }

   ///////////////////////////////////////////////////////////////////////////////
   static void SetVisibilityUIValuesFromConfig(Ui::MainWindow& ui)
   {
      StealthGM::PreferencesVisibilityConfigObject& visConfig =
         StealthViewerData::GetInstance().GetVisibilityConfigObject();

      // Must copy the options because setting the options from the options object
      // can change the values because of the way the events fire... sheesh.
      SimCore::Components::LabelOptions options = visConfig.GetLabelOptions();

      ui.mVisTrackChk->setChecked(options.ShowLabelsForPositionReports());
      ui.mVisEntityChk->setChecked(options.ShowLabelsForEntities());
      ui.mVisBlipChk->setChecked(options.ShowLabelsForBlips());
      ui.mVisLabelGroup->setChecked(options.ShowLabels());
      float value = options.GetMaxLabelDistance();
      int closestIndex = 0;
      float closestValueDiff = FLT_MAX;
      if (value > 0.0f)
      {
         // the last item is "Unlimited", so ignore it.
         for (int i = 0; i < ui.mVisMaxDistCombo->count() - 1; ++i)
         {
            float itemValue = dtUtil::ToType<float>(ui.mVisMaxDistCombo->itemText(i).toStdString());
            float curDiff = std::abs(itemValue - value);
            if (curDiff < closestValueDiff)
            {
               closestValueDiff = curDiff;
               closestIndex = i;
            }
         }
         ui.mVisMaxDistCombo->setCurrentIndex(closestIndex);
      }
      else
      {
         //Set to unlimited
         ui.mVisMaxDistCombo->setCurrentIndex(ui.mVisMaxDistCombo->count() - 1);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void MainWindow::FillAndSetComboBox(const std::vector<dtUtil::Enumeration*>& enums, QComboBox& combo, const dtUtil::Enumeration& enumValue)
   {
      for (size_t i = 0; i < enums.size(); ++i)
      {
         dtUtil::Enumeration* currEnum = enums[i];
         combo.addItem(QString(currEnum->GetName().c_str()));
         if (*currEnum == enumValue)
         {
            combo.setCurrentIndex(i);
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void MainWindow::UpdateUIFromPreferences()
   {
      // General Preferences
      StealthGM::PreferencesGeneralConfigObject& genConfig =
         StealthViewerData::GetInstance().GetGeneralConfigObject();

      FillAndSetComboBox(StealthGM::PreferencesGeneralConfigObject::AttachMode::Enumerate(),
               *mUi->mGeneralAttachModeComboBox, genConfig.GetAttachMode());

      mUi->mAutoAttachCheckBox->setChecked(genConfig.GetShouldAutoAttachToEntity());
      mUi->mAutoAttachCallsign->setText(tr(genConfig.GetAutoAttachEntityCallsign().c_str()));
      mUi->mAttachNodeName->setText(tr(genConfig.GetAttachPointNodeName().c_str()));
      mUi->mAttachAzimuth->setText(tr(dtUtil::ToString(-genConfig.GetInitialAttachRotationHPR()[0]).c_str()));

      mUi->mEntityInfoAutoRefreshCheckBox->setChecked(genConfig.GetAutoRefreshEntityInfoWindow());

      mUi->mGeneralEnableCameraCollisionCheckBox->setChecked(genConfig.GetEnableCameraCollision());

      StealthGM::ViewWindowConfigObject& viewConfig =
         StealthViewerData::GetInstance().GetViewWindowConfigObject();

      int index = mUi->mGeneralFarClippingPlaneComboBox->findText(QString::number(viewConfig.GetFarClippingPlane()));
      if (index >= 0)
      {
         mUi->mGeneralFarClippingPlaneComboBox->setCurrentIndex(index);
      }

      //AssignFOVUiValuesFromConfig();

      index = mUi->mGeneralNearClippingPlaneComboBox->findText(QString::number(viewConfig.GetNearClippingPlane()));
      if (index >= 0)
      {
         mUi->mGeneralNearClippingPlaneComboBox->setCurrentIndex(index);
      }

      mUi->mGeneralLODScaleLineEdit->setText(QString::number(genConfig.GetLODScale()));

      FillAndSetComboBox(StealthGM::PreferencesGeneralConfigObject::PerformanceMode::Enumerate(),
               *mUi->mGeneralPerformanceComboBox, genConfig.GetPerformanceMode());

      mUi->mGeneralShowAdvancedOptionsCheckBox->setChecked(genConfig.GetShowAdvancedOptions());

      // General Environment
      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      // Ensure the correct box displays. Invoke the slot manually
      if (envConfig.GetUseNetworkSettings())
      {
         mUi->mWeatherUseNetworkSettingsRadioButton->setChecked(true);
         OnWeatherNetworkRadioButtonClicked();
      }
      else if (envConfig.GetUseThemedSettings())
      {
         mUi->mWeatherThemedRadioButton->setChecked(true);
         OnWeatherThemedRadioButtonClicked();
      }
      else if (envConfig.GetUseCustomSettings())
      {
         mUi->mWeatherCustomRadioButton->setChecked(true);
         OnWeatherCustomRadioButtonClicked();
      }

      QTime time(envConfig.GetCustomHour(),
               envConfig.GetCustomMinute(),
               envConfig.GetCustomSecond());
      mUi->mCustomTimeEdit->setTime(time);

      StealthGM::PreferencesToolsConfigObject& toolsConfig =
         StealthViewerData::GetInstance().GetToolsConfigObject();

      mUi->mToolsAutoAttachOnSelectionCheckBox->setChecked(toolsConfig.GetAutoAttachOnSelection());

      FillAndSetComboBox(StealthGM::PreferencesToolsConfigObject::CoordinateSystem::Enumerate(),
               *mUi->mOptionsCoordinateSystemComboBox, toolsConfig.GetCoordinateSystem());

      FillAndSetComboBox(SimCore::UnitOfLength::Enumerate(), *mUi->mDistanceUnitCombo, toolsConfig.GetLengthUnit());
      FillAndSetComboBox(SimCore::UnitOfAngle::Enumerate(), *mUi->mAngleUnitCombo, toolsConfig.GetAngleUnit());

      mUi->mToolsMagnificationSpinBox->setValue(int(toolsConfig.GetMagnification()));
      mUi->mToolsShowCompass360CheckBox->setChecked(toolsConfig.GetShowCompass360());
      mUi->mToolsShowBinocularImageCheckBox->setChecked(toolsConfig.GetShowBinocularImage());
      mUi->mToolsShowDistanceToObjectCheckBox->setChecked(toolsConfig.GetShowDistanceToObject());
      mUi->mToolsShowElevationOfObjectCheckBox->setChecked(toolsConfig.GetShowElevationOfObject());

      AddVisibilityCheckBoxes();

      // Record controls
      StealthGM::ControlsRecordConfigObject& recordConfig =
         StealthViewerData::GetInstance().GetRecordConfigObject();

      mUi->mRecordShowAdvancedOptionsCheckBox->setChecked(recordConfig.GetShowAdvancedOptions());
      bool enable = !recordConfig.GetOutputFilename().empty();
      if (enable)
      {
         mUi->mRecordFileLineEdit->setText(tr(recordConfig.GetOutputFilename().c_str()));
      }

      mUi->mRecordStartButton->setEnabled(enable);
      mUi->mRecordAddTimeMarkerButton->setEnabled(false);

      mUi->mRecordAutomaticTimeMarkersCheckBox->setChecked(recordConfig.GetAutoKeyFrame());
      mUi->mRecordAutomaticTimeMarkersSpinBox->setEnabled(recordConfig.GetAutoKeyFrame());
      mUi->mRecordAutomaticTimeMarkersSpinBox->setValue(recordConfig.GetAutoKeyFrameInterval());

      // Playback controls
      StealthGM::ControlsPlaybackConfigObject& playbackConfig =
         StealthViewerData::GetInstance().GetPlaybackConfigObject();

      mUi->mPlaybackShowAdvancedOptionsCheckBox->setChecked(playbackConfig.GetShowAdvancedOptions());
      mUi->mPlaybackLoopCheckBox->setChecked(playbackConfig.GetLoopContinuously());
      if (!playbackConfig.GetInputFilename().empty())
      {
         mUi->mPlaybackFileLineEdit->setText(tr(playbackConfig.GetInputFilename().c_str()));
         mUi->mPlaybackPlayPushButton->setEnabled(true);
      }

      float value = playbackConfig.GetPlaybackSpeed();
      if (value == 0.1f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(0);
      }
      else if (value == 0.25f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(1);
      }
      else if (value == 0.5f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(2);
      }
      else if (value == 1.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(3);
      }
      else if (value == 1.5f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(4);
      }
      else if (value == 2.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(5);
      }
      else if (value == 4.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(6);
      }
      else if (value == 8.0f)
      {
         mUi->mPlaybackPlaybackSpeedComboBox->setCurrentIndex(7);
      }
      else if (value == 16.0f)
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

      SetVisibilityUIValuesFromConfig(*mUi);
      mViewDockWidget->LoadSettings();
   }

   ///////////////////////////////////////////////////////////////
   void MainWindow::AddVisibilityCheckBoxes()
   {
      QFormLayout* visDomainForm = new QFormLayout;
      QFormLayout* visForceForm = new QFormLayout;
      mUi->mVisDomainGroup->setLayout(visDomainForm);
      mUi->mVisForceGroup->setLayout(visForceForm);

      SimCore::VisibilityOptions& options =
         StealthViewerData::GetInstance().GetVisibilityConfigObject().GetEntityOptions();
      SimCore::BasicVisibilityOptions basicOpts = options.GetBasicOptions();

      const std::vector<SimCore::Actors::BaseEntityActorProxy::DomainEnum*>& domainEnumVals =
         SimCore::Actors::BaseEntityActorProxy::DomainEnum::EnumerateType();

      mUi->mVisShowPlatforms->setChecked(basicOpts.mPlatforms);
      mUi->mVisShowHumans->setChecked(basicOpts.mDismountedInfantry);
      mUi->mVisShowBlips->setChecked(basicOpts.mSensorBlips);
      mUi->mVisShowTracks->setChecked(basicOpts.mTracks);
      mUi->mVisShowBFG->setChecked(basicOpts.mBattlefieldGraphics);

      mUi->mVisBFGCloseTops->setChecked(StealthViewerData::GetInstance().GetVisibilityConfigObject().GetBFGCloseTops());

      for (size_t i = 0; i < domainEnumVals.size(); ++i)
      {
         QCheckBox* check = new QCheckBox(tr(domainEnumVals[i]->GetDisplayName().c_str()));
         check->setChecked(basicOpts.IsEnumVisible(*domainEnumVals[i]));

         connect(check,  SIGNAL(toggled(bool)),
                  this,  SLOT(OnVisibilityOptionToggled(bool)));

         //Store the enum object so it be looked up later when a user toggles it.
         check->setUserData(0, (QObjectUserData*)domainEnumVals[i]);
         visDomainForm->addRow(NULL, check);
         mVisibilityCheckBoxes.push_back(check);
      }

      const std::vector<SimCore::Actors::BaseEntityActorProxy::ForceEnum*>& forceEnumVals =
         SimCore::Actors::BaseEntityActorProxy::ForceEnum::EnumerateType();

      for (size_t i = 0; i < forceEnumVals.size(); ++i)
      {
         QCheckBox* check = new QCheckBox(tr(forceEnumVals[i]->GetDisplayName().c_str()));

         check->setChecked(basicOpts.IsEnumVisible(*forceEnumVals[i]));

         connect(check,  SIGNAL(toggled(bool)),
                  this,  SLOT(OnVisibilityOptionToggled(bool)));

         //Store the enum object so it be looked up later when a user toggles it.
         check->setUserData(0, (QObjectUserData*)forceEnumVals[i]);
         visForceForm->addRow(NULL, check);
         mVisibilityCheckBoxes.push_back(check);
      }
   }

   ///////////////////////////////////////////////////////////////
   void MainWindow::RecordKeyFrameSlot(const std::vector<dtGame::LogKeyframe>& keyFrames)
   {
      mUi->mRecordTimeMarkersLineTextBox->clear();

      for(size_t i = 0; i < keyFrames.size(); i++)
      {
         mUi->mRecordTimeMarkersLineTextBox->addItem(
                  new QListWidgetItem(tr(keyFrames[i].GetName().c_str())));
      }
   }

   ///////////////////////////////////////////////////////////////
   void MainWindow::PlaybackKeyFrameSlot(const std::vector<dtGame::LogKeyframe>& keyFrames)
   {
      mUi->mPlaybackTimeMarkersTextBox->clear();

      for(size_t i = 0; i < keyFrames.size(); i++)
      {
         mUi->mPlaybackTimeMarkersTextBox->addItem(
                  new QListWidgetItem(tr(keyFrames[i].GetName().c_str())));
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::EnablePlaybackButtons(bool enable)
   {
      mUi->mPlaybackStartOverPushButton->setEnabled(enable);
      mUi->mPlaybackJumpToPrevTimeMarkerPushButton->setEnabled(enable);
      mUi->mPlaybackPlayPushButton->setEnabled(enable);
      mUi->mPlaybackJumpToNextTimeMarkerPushButton->setEnabled(enable);
      //mUi->mPlaybackPlaybackSpeedComboBox->setEnabled(enable);
      mUi->mPlaybackJumpToTimeMarkerPushButton->setEnabled(enable);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ConnectSigSlots()
   {
      dtGame::GMComponent* gmComp =
         mGM->GetComponentByName("LogController");
      if (gmComp == NULL)
         return;

      dtGame::LogController& logController = static_cast<dtGame::LogController&>(*gmComp);

      logController.SignalReceivedKeyframes().connect_slot(this, &MainWindow::RecordKeyFrameSlot);
      logController.SignalReceivedKeyframes().connect_slot(this, &MainWindow::PlaybackKeyFrameSlot);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnConnectToNetwork(QString connectionName)
   {
      mIsConnectedToANetwork = true;
      mCurrentConnectionName = connectionName;

      EnableGeneralUIAndTick();
      StealthGM::PreferencesGeneralConfigObject& genConfig = StealthViewerData::GetInstance().GetGeneralConfigObject();
      if (genConfig.GetAutoReconnect())
      {
         mHLAErrorTimer.setInterval(1000 * genConfig.GetAutoReconnectTimeout());
      }
      else
      {
         mHLAErrorTimer.setInterval(1000);
      }
      mHLAErrorTimer.start();
      mUi->mActionMaps->setEnabled(false);

      EndWaitCursor();
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnConnectToNetworkFailed(QString connectionName)
   {
      DisableGeneralUIAndTick();
      StealthGM::PreferencesGeneralConfigObject& genConfig = StealthViewerData::GetInstance().GetGeneralConfigObject();
      if (genConfig.GetAutoReconnect())
      {
         mHLAErrorTimer.setInterval(1000 * genConfig.GetAutoReconnectTimeout());
         mHLAErrorTimer.start();
      }
      EndWaitCursor();
   }


   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnDisconnectFromNetwork(bool disableUI)
   {
      mIsConnectedToANetwork = false;
      mCurrentConnectionName.clear();
      mHLAErrorTimer.stop();

      if (disableUI)
      {
         DisableGeneralUIAndTick();
      }
      mUi->mActionMaps->setEnabled(true);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::EnableGeneralUIAndTick()
   {
      mUi->mPreferencesDockWidget->setEnabled(true);
      mUi->mControlsDockWidget->setEnabled(true);
      mUi->mEntityInfoDockWidget->setEnabled(true);
      mViewDockWidget->setEnabled(true);

      mGenericTickTimer.start();
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::DisableGeneralUIAndTick()
   {
      mGenericTickTimer.stop();

      mUi->mPreferencesDockWidget->setEnabled(false);
      mUi->mControlsDockWidget->setEnabled(false);
      mUi->mEntityInfoDockWidget->setEnabled(false);
      mViewDockWidget->setEnabled(false);

      ClearData();
   }


   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnSecondTimerElapsed()
   {
      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      if (envConfig.GetUseNetworkSettings())
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

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnDurationTimerElapsed()
   {
      dtGame::GameManager& gm = *mGM;
      double curSimtime = gm.GetSimulationTime();
      mRecordingStopTime = curSimtime;

      if (mIsRecording || mIsPlayingBack)
      {
         if (mIsRecording)
         {
            std::string duration = dtUtil::DateTime::ToString(time_t(mRecordingStopTime - mRecordingStartTime),
                     dtUtil::DateTime::TimeFormat::CLOCK_TIME_24_HOUR_FORMAT);

            //mUi->mRecordDurationLineEdit->setText(!gm.IsPaused() ? QString(duration.c_str()): tr("Paused"));
            mUi->mRecordDurationLineEdit->setText(QString(duration.c_str()));
         }
         else if (mIsPlayingBack)
         {
            // Get the log controller
            dtGame::GMComponent* component = gm.GetComponentByName(dtGame::LogController::DEFAULT_NAME);
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

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnHLAErrorTimerElapsed()
   {
      SimCore::HLA::HLAConnectionComponent* comp =
         static_cast<SimCore::HLA::HLAConnectionComponent*>
      (mGM->GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME));

      if (comp->GetConnectionState() == SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_ERROR)
      {
         if (StealthViewerData::GetInstance().GetGeneralConfigObject().GetAutoReconnect())
         {
            ReconnectToHLA();
         }
         else
         {
            QMessageBox::critical(this, tr("Error"),
                     tr("An error occurred while connecting to the network. ") +
                     tr("Please check your connection settings from the Network tab ") +
                     tr("and ensure they are correct."),
                     QMessageBox::Ok);

            OnConnectionWindowActionTriggered();
         }
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
   void MainWindow::PopulateEntityInfoWindowDontAttach(bool notUsed)
   {
      QTableWidgetItem* currentItem = mUi->mSearchEntityTableWidget->currentItem();
      if (currentItem == NULL)
         return;

      DoPopulateEntityInfoWindow(currentItem, false);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::PopulateEntityInfoWindowAndAttach(QTableWidgetItem* currentItem)
   {
      DoPopulateEntityInfoWindow(currentItem, true);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::DoPopulateEntityInfoWindow(QTableWidgetItem* currentItem, bool attach)
   {
      unsigned int index = (unsigned int)(mUi->mSearchEntityTableWidget->currentRow());
      if (index > mFoundActors.size() || currentItem == NULL)
         return;

      QString id = currentItem->data(Qt::UserRole).toString();

      // Retrieve proxy from the GM
      dtGame::GameActorProxy *proxy = mGM->FindGameActorById(dtCore::UniqueId(id.toStdString()));
      if (proxy != NULL)
      {
         UpdateEntityInfoData(*proxy);

         // This is the special case. See also OnRefreshEntityInfoTimerElapsed() for similar code and explanation.
         if (attach && mUi->mToolsAutoAttachOnSelectionCheckBox->checkState() == Qt::Checked)
         {
            StealthViewerData::GetInstance().GetGeneralConfigObject().AttachToActor(proxy->GetId());
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
      StealthGM::PreferencesEnvironmentConfigObject& envConfig =
         StealthViewerData::GetInstance().GetEnvironmentConfigObject();

      OnSecondTimerElapsed();

      float visMeters = envConfig.GetVisibilityDistance();

      mUi->mVisibilityLineEdit->setText(QString::number(visMeters) + tr(" KM"));

      mUi->mWeatherLineEdit->setText(tr(envConfig.GetPrecipitationAsString().c_str()));

      SimCore::HLA::HLAConnectionComponent* comp = NULL;
      mGM->GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME, comp);

      if (comp->GetConnectionState() == SimCore::HLA::HLAConnectionComponent::ConnectionState::STATE_DISCONNECTED)
      {
         StealthGM::PreferencesGeneralConfigObject& genConfig = StealthViewerData::GetInstance().GetGeneralConfigObject();
         bool reconnectSucceeded = false;
         if (genConfig.GetAutoReconnect())
         {
            try
            {
               mGM->CloseCurrentMap();
            }
            catch (const dtUtil::Exception& ex)
            {
               QString message = tr("The application failed to unload the current set of maps "
                     "after an externally forced disconnect: ") + tr(ex.ToString().c_str());
               QMessageBox::critical(this, tr("Error"), message, QMessageBox::Ok);
            }

            ReconnectToHLA();
            reconnectSucceeded = true;
         }

         if (!reconnectSucceeded)
         {
            // Make sure we still pick up the signals from these events. This is important
            // for the UI to update itself properly
            HLAWindow window(*mGM, this, NULL, mIsConnectedToANetwork, mCurrentConnectionName);

            connect(&window, SIGNAL(ConnectedToNetwork(QString)), this, SLOT(OnConnectToNetwork(QString)));
            connect(&window, SIGNAL(ConnectedToNetworkFailed(QString)), this, SLOT(EndWaitCursor()));
            connect(&window, SIGNAL(DisconnectedFromNetwork(bool)), this, SLOT(OnDisconnectFromNetwork(bool)));

            window.Disconnect(true);
         }
      }

   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ReconnectToHLA()
   {
      std::string name = mCurrentConnectionName.toStdString();
      mCurrentConnectionName.clear();
      if (name.empty())
      {
         // If you do NOT read from the command line, see if it is stored in the
         // preferences file
         if (StealthViewerData::GetInstance().GetGeneralConfigObject().GetReconnectOnStartup())
         {
            name = StealthViewerData::GetInstance().GetGeneralConfigObject().GetStartupConnectionName();
         }
      }

      // Nothing either way, peace out and start like normal
      if (name.empty() || QString(name.c_str()).toLower() == "none")
         return;

      // Look up the properties for the name
      QString connectionName = QString(name.c_str());
      QStringList connectionProps =
         StealthViewerData::GetInstance().GetSettings().GetConnectionProperties(connectionName);

      // Was the name in the file or on the command line actually valid?
      if (!StealthViewerData::GetInstance().GetSettings().ContainsConnection(connectionName) ||
               connectionProps.isEmpty())
      {
         // Apparently not
         QString message = tr("The application failed to reconnect to the connection named: ") +
         connectionName + tr(" . Please select a new network to connect to from the Network tab.");
         QMessageBox::critical(this, tr("Failed to reconnect to the network"), message, QMessageBox::Ok);

         // Peace out
         return;
      }

      // Make sure we still pick up the signals from these events. This is important
      // for the UI to update itself properly
      HLAWindow window(*mGM, this, NULL, mIsConnectedToANetwork, mCurrentConnectionName);

      connect(&window, SIGNAL(ConnectedToNetwork(QString)), this, SLOT(OnConnectToNetwork(QString)));
      connect(&window, SIGNAL(ConnectedToNetworkFailed(QString)), this, SLOT(EndWaitCursor()));
      connect(&window, SIGNAL(DisconnectedFromNetwork(bool)), this, SLOT(OnDisconnectFromNetwork(bool)));

      // Begin wait cursor
      StartWaitCursor();

      // Kind of hackish, but connect through the window like you normally would
      // to preserve signal delegation
      window.SetConnectionValues(connectionProps);
   }

   /////////////////////////////////////////////////////////////
   //////
   void MainWindow::OnRefreshEntityInfoTimerElapsed()
   {
      // In addition to refreshing the entity, we also update the 'Detach' button.
      bool attached = StealthViewerData::GetInstance().GetGeneralConfigObject().IsStealthActorCurrentlyAttached();
      mUi->mSearchDetachPushButton->setEnabled(attached);

      // Now, update the entity info window
      if (mUi->mEntityInfoAutoRefreshCheckBox->isChecked())
      {
         //PopulateEntityInfoWindow();
         QTableWidgetItem* currentItem = mUi->mSearchEntityTableWidget->currentItem();
         unsigned int index = (unsigned int)(mUi->mSearchEntityTableWidget->currentRow());
         if (index > mFoundActors.size() || currentItem == NULL)
            return;

         DoPopulateEntityInfoWindow(currentItem, false);
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ShowEntityErrorMessage(QTableWidgetItem *currentItem)
   {
      if (mShowMissingEntityInfoErrorMessage)
      {
         if (mUi->mEntityInfoAutoRefreshCheckBox->isChecked())
            mShowMissingEntityInfoErrorMessage = false;

         QString message =
            tr("Could not find info for the actor named: ") +
            ((currentItem == NULL) ? tr("Unknown") : currentItem->text()) +
            tr(" because this actor has been removed from the scenario. Please select another actor");

         QMessageBox::warning(this, tr("Error finding info for actor"), message, QMessageBox::Ok);
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::UpdateEntityInfoData(dtGame::GameActorProxy& actor)
   {
      std::ostringstream oss;
      // Get the StealthHUD so we can get the coordinate Converter. Makes our coordinates be location specific.
      StealthGM::StealthHUD* hudComponent = dynamic_cast<StealthGM::StealthHUD*>
      (mGM->GetComponentByName(StealthGM::StealthHUD::DEFAULT_NAME));
      if (hudComponent == NULL)
      {
         throw dtGame::InvalidParameterException(
                  "Failed to locate the StealthHUD Component on the Game Manager. Critical failure.",
                  __FILE__, __LINE__);
      }
      dtUtil::Coordinates& coordinateConverter = hudComponent->GetCoordinateConverter();

      // Calculate the directional speed from the entities velocity
      SimCore::Actors::BaseEntity* entityDraw = NULL;
      actor.GetDrawable(entityDraw);

      dtCore::Transform trans;
      entityDraw->GetTransform(trans, dtCore::Transformable::REL_CS);
      osg::Vec3 pos;
      trans.GetTranslation(pos);
      osg::Vec3 rot;
      trans.GetRotation(rot);

      StealthGM::PreferencesToolsConfigObject& toolsConfig = StealthViewerData::GetInstance().GetToolsConfigObject();

      // Set the Lat Lon pos
      //coordinateConverter.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
      const osg::Vec3d& globePos = coordinateConverter.ConvertToRemoteTranslation(pos);
      oss.str("");
      oss << globePos.x();
      mUi->mEntityInfoPosLatEdit->setText(QString(oss.str().c_str()));
      oss.str("");
      oss << globePos.y();
      mUi->mEntityInfoPosLonEdit->setText(QString(oss.str().c_str()));
      oss.str("");
      oss.precision(5);
      oss << SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, toolsConfig.GetLengthUnit(), globePos.z());
      oss << " " << toolsConfig.GetLengthUnit().GetAbbreviation();
      mUi->mEntityInfoPosElevLL->setText(QString(oss.str().c_str()));

      // Set the MGRS pos
      std::string milgrid = coordinateConverter.XYZToMGRS(pos);
      mUi->mEntityInfoPositionMGRS->setText(QString(milgrid.c_str()));

      mUi->mEntityInfoPositionLabel_XYZ->setText(QString("Position (") +
               toolsConfig.GetLengthUnit().GetAbbreviation().c_str() + ")");
      // Set the XYZ Pos
      oss.str("");
      oss.precision(8);
      oss << SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, toolsConfig.GetLengthUnit(), pos.x());
      mUi->mEntityInfoPosXEdit->setText(QString(oss.str().c_str()));
      oss.str("");
      oss << SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, toolsConfig.GetLengthUnit(), pos.y());
      mUi->mEntityInfoPosYEdit->setText(QString(oss.str().c_str()));
      oss.str("");
      oss << SimCore::UnitOfLength::Convert(SimCore::UnitOfLength::METER, toolsConfig.GetLengthUnit(), pos.z());
      mUi->mEntityInfoPosZEdit->setText(QString(oss.str().c_str()));

      mUi->mEntityInfoRotationLabel->setText(QString("Rotation (")
               + toolsConfig.GetAngleUnit().GetAbbreviation().c_str() + ")");

      oss.precision(4);
      // Heading
      float heading = ComputeHumanReadableDirection(float(rot[0]));
      oss.str("");
      oss << SimCore::UnitOfAngle::Convert(SimCore::UnitOfAngle::DEGREE, toolsConfig.GetAngleUnit(), heading);
      mUi->mEntityInfoRotHeadEdit->setText(tr(oss.str().c_str()));
      // Pitch
      float pitch = ComputeHumanReadableDirection(float(rot[1]));
      oss.str("");
      oss << SimCore::UnitOfAngle::Convert(SimCore::UnitOfAngle::DEGREE, toolsConfig.GetAngleUnit(), pitch);
      mUi->mEntityInfoRotPitchEdit->setText(tr(oss.str().c_str()));
      // Roll
      float roll = ComputeHumanReadableDirection(float(rot[2]));
      oss.str("");
      oss << SimCore::UnitOfAngle::Convert(SimCore::UnitOfAngle::DEGREE, toolsConfig.GetAngleUnit(), roll);
      mUi->mEntityInfoRotRollEdit->setText(tr(oss.str().c_str()));

      osg::Vec3 velocity = entityDraw->GetComponent<dtGame::DeadReckoningActorComponent>()->GetLastKnownVelocity();
      // speed is distance of velocity. Then, convert from m/s to MPH
      float speed = velocity.length();
      oss.str("");
      oss.precision(4);
      // sea entities use knots.
      if (entityDraw->GetDomain() == SimCore::Actors::BaseEntityActorProxy::DomainEnum::SURFACE
               || entityDraw->GetDomain() == SimCore::Actors::BaseEntityActorProxy::DomainEnum::SUBMARINE
               || entityDraw->GetDomain() == SimCore::Actors::BaseEntityActorProxy::DomainEnum::AMPHIBIOUS)
      {
         // Convert to knots and truncate past two decimal places.
         oss << std::floor((speed * 1.94384449f) * 100.0f) / 100.0f << " KNOTS";
      }
      else
      {
         // Convert to miles per hour and truncate past two decimal places.
         oss << std::floor((speed * 2.23693629f) * 100.0f) / 100.0f << " MPH";
      }

      mUi->mEntityInfoSpeed->setText(tr(oss.str().c_str()));

      // compute the direction of movement
      float direction = atan2(-(float)velocity[0], (float) velocity[1]);
      direction = osg::RadiansToDegrees(direction);
      direction = ComputeHumanReadableDirection(direction);
      oss.str("");
      oss << SimCore::UnitOfAngle::Convert(SimCore::UnitOfAngle::DEGREE, toolsConfig.GetAngleUnit(), direction);
      oss << " " << toolsConfig.GetAngleUnit().GetAbbreviation();
      mUi->mEntityInfoSpeedDir->setText(tr(oss.str().c_str()));

      mUi->mEntityInfoCallSignLineEdit->setText(tr(actor.GetName().c_str()));
      mUi->mEntityInfoForceLineEdit->setText(tr(actor.GetProperty("Force Affiliation")->ToString().c_str()));
      mUi->mDamageStateLineEdit->setText(tr(actor.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_DAMAGE_STATE)->ToString().c_str()));

      // NOTE: To avoid confusion.
      // Entity Type will write to Entity Type ID line edit
      // Mapping Name will write to Entity Type line edit.
      const dtCore::ActorProperty* param
      = actor.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_ENTITY_TYPE_ID);
      mUi->mEntityTypeIDLineEdit->setText(tr( param == NULL ? "" : param->ToString().c_str() ));

      param = actor.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_MAPPING_NAME);
      mUi->mEntityTypeLineEdit->setText(tr( param == NULL ? "" : param->ToString().c_str() ));

      // we hide the last update time now, since in reality, we can't use this field. The last update time
      // is really the last time that the entity trans or rotation was changed. But, if the entity is sitting
      // still, the last update time is never changed. This is further complicated when the sim time changes
      // after joining a federation, making this time something like 370000 seconds. It's just not helpful[
      //double lastUpdateTime = EntitySearch::GetLastUpdateTime(*proxy);
      //mUi->mEntityInfoLastUpdateTimeLineEdit->setText(QString::number(lastUpdateTime) + tr(" seconds ago"));
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::ShowOrHideEntityInfoPositionFields(const dtUtil::Enumeration& system)
   {
      // Hide the MGRS fields
      mUi->mEntityInfoPositionMGRS->hide();
      mUi->mEntityInfoPositionLabel_MGRS->hide();
      // Hide the XYZ fields
      mUi->mEntityInfoPositionLabel_XYZ->hide();
      mUi->mEntityInfoPosXEdit->hide();
      mUi->mEntityInfoPosYEdit->hide();
      mUi->mEntityInfoPosZEdit->hide();
      // Hide the Lat Lon fields
      mUi->mEntityInfoPositionLabel_LatLon->hide();
      mUi->mEntityInfoPosLatEdit->hide();
      mUi->mEntityInfoPosLonEdit->hide();
      mUi->mEntityInfoPosElevLL->hide();

      if (system == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::MGRS)
      {
         // Show the MGRS fields
         mUi->mEntityInfoPositionMGRS->show();
         mUi->mEntityInfoPositionLabel_MGRS->show();
      }
      else if (system == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::LAT_LON)
      {
         // Hide the Lat Lon fields
         mUi->mEntityInfoPositionLabel_LatLon->show();
         mUi->mEntityInfoPosLatEdit->show();
         mUi->mEntityInfoPosLonEdit->show();
         mUi->mEntityInfoPosElevLL->show();
      }
      else // if (system == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ
      {
         // Show the XYZ fields
         mUi->mEntityInfoPositionLabel_XYZ->show();
         mUi->mEntityInfoPosXEdit->show();
         mUi->mEntityInfoPosYEdit->show();
         mUi->mEntityInfoPosZEdit->show();
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
      if (isChecked)
      {
         mShowMissingEntityInfoErrorMessage = true;
         mUi->mSearchInfoPushButton->setEnabled(false);
      }
      else
      {
         mUi->mSearchInfoPushButton->setEnabled(true);
      }
      StealthViewerData::GetInstance().GetGeneralConfigObject().SetAutoRefreshEntityInfoWindow(isChecked);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnTimeMarkerDoubleClicked(QListWidgetItem* item)
   {
      if (item != NULL)
      {
         OnPlaybackJumpToTimeMarkerButtonClicked(item->text());
      }
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnVisLabelsToggled(bool)
   {
      SimCore::Components::LabelOptions options =
         StealthViewerData::GetInstance().GetVisibilityConfigObject().GetLabelOptions();

      options.SetShowLabels(mUi->mVisLabelGroup->isChecked());
      options.SetShowLabelsForEntities(mUi->mVisEntityChk->isChecked());
      options.SetShowLabelsForBlips(mUi->mVisBlipChk->isChecked());
      options.SetShowLabelsForPositionReports(mUi->mVisTrackChk->isChecked());

      StealthViewerData::GetInstance().GetVisibilityConfigObject().SetLabelOptions(options);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void MainWindow::OnVisibilityOptionToggled(bool newValue)
   {
      SimCore::VisibilityOptions& options =
         StealthViewerData::GetInstance().GetVisibilityConfigObject().GetEntityOptions();
      SimCore::BasicVisibilityOptions basicOpts = options.GetBasicOptions();
      basicOpts.mPlatforms = mUi->mVisShowPlatforms->isChecked();
      basicOpts.mDismountedInfantry = mUi->mVisShowHumans->isChecked();
      basicOpts.mSensorBlips = mUi->mVisShowBlips->isChecked();
      basicOpts.mTracks = mUi->mVisShowTracks->isChecked();
      basicOpts.mBattlefieldGraphics = mUi->mVisShowBFG->isChecked();

      StealthViewerData::GetInstance().GetVisibilityConfigObject().SetBFGCloseTops(mUi->mVisBFGCloseTops->isChecked());

      for (size_t i = 0; i < mVisibilityCheckBoxes.size(); ++i)
      {
         QCheckBox* check = mVisibilityCheckBoxes[i];
         dtUtil::Enumeration* enumVal = (dtUtil::Enumeration*)check->userData(0);
         if (enumVal != NULL)
         {
            basicOpts.SetEnumVisible(*enumVal, check->isChecked());
         }
      }
      options.SetBasicOptions(basicOpts);
      StealthViewerData::GetInstance().GetVisibilityConfigObject().SetEntityOptions(options);
   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::OnVisLabelsDistanceChanged(const QString& text)
   {
      const std::string textValue = text.toStdString();

      SimCore::Components::LabelOptions options =
         StealthViewerData::GetInstance().GetVisibilityConfigObject().GetLabelOptions();

      if (textValue == "Unlimited")
      {
         options.SetMaxLabelDistance(-1);
      }
      else
      {
         float distance = dtUtil::ToType<float>(textValue);
         options.SetMaxLabelDistance(distance);
      }

      StealthViewerData::GetInstance().GetVisibilityConfigObject().SetLabelOptions(options);

   }

   ///////////////////////////////////////////////////////////////////
   void MainWindow::SelectCorrectWarpToUI(dtUtil::Enumeration& enumValue)
   {
      mUi->mWarpToLatLonGroup->hide();
      mUi->mWarpToMGRSGroup->hide();
      mUi->mWarpToXYZGroup->hide();
      if (enumValue == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::MGRS)
      {
         mUi->mWarpToMGRSGroup->show();
      }
      else if (enumValue == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ)
      {
         mUi->mWarpToXYZGroup->show();
      }
      else if (enumValue == StealthGM::PreferencesToolsConfigObject::CoordinateSystem::LAT_LON)
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
