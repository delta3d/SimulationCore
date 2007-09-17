/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <StealthViewer/Qt/HLAWindow.h>
#include <StealthViewer/Qt/ui_HLAWindowUi.h>
#include <StealthViewer/Qt/HLAOptions.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <QtGui/QMessageBox>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/hlacomponentconfig.h>
#include <dtDAL/project.h>
#include <dtDAL/resourcedescriptor.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/macros.h>
#include <SimCore/Components/HLAConnectionComponent.h>

#ifdef DELTA_WIN32
   bool caseSensitive = false;
#else
   bool caseSensitive = true;
#endif

namespace StealthQt
{
   HLAWindow::HLAWindow(dtGame::GameManager &gm,
                        QWidget *parent, 
                        StealthViewerSettings *settings, 
                        bool isConnected, 
                        QString currentConnectionName) : 
      QDialog(parent), 
      mUi(new Ui::HLAWindow), 
      mIsConnected(isConnected), 
      mHLAComp(NULL), 
      mCurrentConnectionName(currentConnectionName), 
      mCancelConnectProcess(false)
   {
      mUi->setupUi(this);

      ConnectSlots();

      // Display saved connections
      QStringList toDisplay;
      toDisplay = (settings != NULL) ? settings->GetConnectionNames() 
                : StealthViewerData::GetInstance().GetSettings().GetConnectionNames();
                                      
      mUi->mNetworkListWidget->addItems(toDisplay);

      mHLAComp = 
         static_cast<SimCore::Components::HLAConnectionComponent*>(gm.GetComponentByName(SimCore::Components::HLAConnectionComponent::DEFAULT_NAME));

      mUi->mConnectPushButton->setEnabled(!mIsConnected && mUi->mNetworkListWidget->currentItem() != NULL);
      mUi->mDisconnectPushButton->setEnabled(mIsConnected);
      mUi->mEditPushButton->setEnabled(false);
      mUi->mDeletePushButton->setEnabled(false);

      if(!mCurrentConnectionName.isEmpty())
         mUi->mCurrentFederationLineEdit->setText(mCurrentConnectionName);

      bool connect = 
         StealthViewerData::GetInstance().GetGeneralConfigObject().GetReconnectOnStartup();

      mUi->mReconnectOnStartupCheckBox->setChecked(connect);
   }

   HLAWindow::~HLAWindow()
   {
      delete mUi;
      mUi = NULL;
   }

   void HLAWindow::ConnectSlots()
   {
      connect(mUi->mConnectPushButton,    SIGNAL(clicked(bool)), this, SLOT(OnConnect(bool)));
      connect(mUi->mDisconnectPushButton, SIGNAL(clicked(bool)), this, SLOT(OnDisconnect(bool)));
      connect(mUi->mClosePushButton,      SIGNAL(clicked(bool)), this, SLOT(OnClose(bool)));
      connect(mUi->mNewPushButton,        SIGNAL(clicked(bool)), this, SLOT(OnNew(bool)));
      connect(mUi->mEditPushButton,       SIGNAL(clicked(bool)), this, SLOT(OnEdit(bool)));
      connect(mUi->mDeletePushButton,     SIGNAL(clicked(bool)), this, SLOT(OnDelete(bool)));
   
      connect(mUi->mNetworkListWidget, 
              SIGNAL(currentTextChanged(const QString&)), 
              this, 
              SLOT(OnCurrentTextChanged(const QString&)));

      connect(&StealthViewerData::GetInstance().GetSettings(), 
               SIGNAL(ItemDeleted(QString)), 
               this, 
               SLOT(OnListItemDeleted(QString)));
   }

   void HLAWindow::OnConnect(bool checked)
   {
      if(mHLAComp != NULL)
      {
         if(mHLAComp->IsConnected())
         {
            OnDisconnect();

            // If the disconnect from current federation message box 
            // is picked to cancel, this flag will be set to true
            // in which case we need to leave this method.
            
            // This is done since OnDisconnect is a SLOT, it's return value 
            // CANNOT be changed to mismatch the SIGNAL prototype. 
            if(mCancelConnectProcess)
            {
               // Make sure to flip this back to false
               mCancelConnectProcess = false;
               return;
            }
         }
      }

      QListWidgetItem *currentItem = mUi->mNetworkListWidget->currentItem();
      if(currentItem == NULL)
      {
         QMessageBox::warning(this, tr("Error"),
            tr("Please select a federation connection from the list"), QMessageBox::Ok);
         return;
      }

      mUi->mCurrentFederationLineEdit->setText(currentItem->text());
      mCurrentConnectionName = currentItem->text();

      QString fedEx = currentItem->text(); 

      QStringList props = 
         StealthViewerData::GetInstance().GetSettings().GetConnectionProperties(fedEx);

      SetConnectionValues(props);

      mIsConnected = true;

      mUi->mDisconnectPushButton->setEnabled(true);
      mUi->mConnectPushButton->setEnabled(false);

      // connect to federation
      OnClose();

      accept();
   }

   void HLAWindow::OnDisconnect(bool checked)
   {
      // Check to see if we are connected first

      int result = QMessageBox::question(this, tr("Confirm Disconnection"), 
         tr("Are you sure you want to disconnect?"), QMessageBox::Yes, QMessageBox::No);

      if(result == QMessageBox::Yes)
      {
         // disconnect from federation
         if(mHLAComp != NULL)
            mHLAComp->Disconnect();

         mUi->mCurrentFederationLineEdit->setText("None");
         mCurrentConnectionName = mUi->mCurrentFederationLineEdit->text();
         mUi->mDisconnectPushButton->setEnabled(false);
         mUi->mConnectPushButton->setEnabled(true);
         
         if(mUi->mNetworkListWidget->currentItem() != NULL)
         {
            mUi->mEditPushButton->setEnabled(true);
            mUi->mDeletePushButton->setEnabled(true);
         }

         mIsConnected = false;

         emit DisconnectedFromHLA();
      }
      else
      {
         mCancelConnectProcess = true;
      }
   }

   void HLAWindow::OnClose(bool checked)
   {
      // Write the reconnect property to the settings file
      // This way it is updated whenever a new HLAWindow was closed
      StealthGM::PreferencesGeneralConfigObject &genConfig = 
         StealthViewerData::GetInstance().GetGeneralConfigObject();
      
      genConfig.SetReconnectOnStartup(mUi->mReconnectOnStartupCheckBox->isChecked(), 
         mCurrentConnectionName != "" ? mCurrentConnectionName.toStdString() : "");

      close();
   }

   void HLAWindow::OnNew(bool checked)
   {
      HLAOptions options(this);
      options.SetCaseSensitiveFilePaths(caseSensitive);
      if(options.exec() == QDialog::Accepted)
      {
         QString name = options.GetConnectionName();
         if(!name.isEmpty())
            mUi->mNetworkListWidget->addItem(name);
      }
   }

   void HLAWindow::OnEdit(bool checked)
   {
      QListWidgetItem *currentItem = mUi->mNetworkListWidget->currentItem();
      if(currentItem == NULL)
      {
         QMessageBox::information(this, tr("Error"), 
            tr("Please select a federation connection to edit."), QMessageBox::Ok);

         return;
      }

      HLAOptions options(this, currentItem->text(), true);
      options.SetCaseSensitiveFilePaths(caseSensitive);
      if(options.exec() == QDialog::Accepted)
      {
         QStringList toDisplay = 
            StealthViewerData::GetInstance().GetSettings().GetConnectionNames();

         mUi->mNetworkListWidget->clear();
         mUi->mNetworkListWidget->addItems(toDisplay);
      }
   }

   void HLAWindow::OnDelete(bool checked)
   {
      int result = 
         QMessageBox::information(this, tr("Confirm Delete"), 
            tr("Are you sure you want to delete this connection?"), 
            QMessageBox::Yes, QMessageBox::No);

      if(result == QMessageBox::No)
         return;

      QListWidgetItem *currentItem = mUi->mNetworkListWidget->currentItem();
      if(currentItem == NULL)
      {
         QMessageBox::information(this, tr("Error"), 
            tr("Please select a federation connection to delete."), QMessageBox::Ok);
         
         return;
      }

      StealthViewerSettings &settings = StealthViewerData::GetInstance().GetSettings();
      settings.RemoveConnection(currentItem->text());
   }

   void HLAWindow::OnListItemDeleted(QString name)
   {
      mUi->mNetworkListWidget->clear();
      StealthViewerSettings &settings = StealthViewerData::GetInstance().GetSettings();
      mUi->mNetworkListWidget->addItems(settings.GetConnectionNames());
   }

   void HLAWindow::SetConnectionValues(QStringList &properties)
   {
      try
      {
         dtDAL::Project& project = dtDAL::Project::GetInstance();
         const std::string& context = dtDAL::Project::GetInstance().GetContext();
         
         std::string fedex        = properties[4].toStdString();
         std::string map          = properties[1].toStdString();
         std::string config       = context + dtUtil::FileUtils::PATH_SEPARATOR + project.GetResourcePath(dtDAL::ResourceDescriptor(properties[2].toStdString()));
         std::string fedFile      = context + dtUtil::FileUtils::PATH_SEPARATOR + project.GetResourcePath(dtDAL::ResourceDescriptor(properties[3].toStdString()));
         std::string federateName = properties[5].toStdString();
         std::string ridFile      = context + dtUtil::FileUtils::PATH_SEPARATOR + project.GetResourcePath(dtDAL::ResourceDescriptor(properties[6].toStdString()));         

         if(mHLAComp != NULL)
         {
            mHLAComp->AddMap(map);
            mHLAComp->SetConfigFile(config);
            mHLAComp->SetFedFile(fedFile);
            mHLAComp->SetFedName(federateName);
            mHLAComp->SetFedEx(fedex);
            mHLAComp->SetRidFile(ridFile);

            mHLAComp->Connect();
            emit ConnectedToHLA(properties[0]);
         }
      }
      catch(const dtUtil::Exception &ex)
      {
         QMessageBox::information(this, tr("Error"), 
               tr("Error searching for HLA resource files. Unable to connect to federation: ") + tr(ex.ToString().c_str()), 
               QMessageBox::Ok);
         return;
      }      
   }

   void HLAWindow::OnCurrentTextChanged(const QString &str)
   {
      mUi->mConnectPushButton->setEnabled(!str.isEmpty() && str != mCurrentConnectionName);
      //mUi->mDisconnectPushButton->setEnabled(mIsConnected);
      mUi->mEditPushButton->setEnabled(!str.isEmpty() && !mIsConnected);
      mUi->mDeletePushButton->setEnabled(!str.isEmpty() && !mIsConnected);
   } 

   void HLAWindow::OnListItemActivated(QListWidgetItem *item)
   {
      if(item != NULL)
      {
         QString str = item->text();

         mUi->mEditPushButton->setEnabled(!str.isEmpty() && !mIsConnected);
         mUi->mDeletePushButton->setEnabled(!str.isEmpty() && !mIsConnected);
      }
   }
}
