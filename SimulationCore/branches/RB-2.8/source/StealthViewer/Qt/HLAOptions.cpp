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
 * @author Eddie Johnson, Curtiss Murphy
 */
#include <prefix/StealthQtPrefix.h>
#include <StealthViewer/Qt/HLAOptions.h>
#include <ui_HLAOptionsUi.h>
#include <StealthViewer/Qt/MapSelectDialog.h>
#include <StealthViewer/Qt/FederationFileResourceBrowser.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>

#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QDialog>
#include <QtGui/QVBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QTreeWidget>
#include <QtGui/QFileDialog>

#include <dtHLAGM/hlacomponentconfig.h>
#include <dtHLAGM/hlacomponent.h>

#include <dtGame/gamemanager.h>

#include <dtCore/scene.h>

#include <dtCore/project.h>

#include <dtUtil/macros.h>
#include <dtUtil/datapathutils.h>

#include <osgDB/FileNameUtils>

#include <cctype>

namespace StealthQt
{

   ////////////////////////////////////////////////////////////////////
   HLAOptions::HLAOptions(QWidget *parent, 
                          const QString &connectionName, 
                          bool isEditMode) : 
      QDialog(parent), 
      mUi(new Ui::HLAOptions), 
      mCaseSensitiveFilePaths(false), 
      mIsEditMode(isEditMode)
   {
      mUi->setupUi(this);

      ConnectSlots();

#ifdef DIS_CONNECTIONS_AVAILABLE
      // Add the DIS option if we have that option set
      mUi->mConnectionTypeCombo->addItem(QString::fromStdString(StealthViewerSettings::CONNECTIONTYPE_DIS));
#endif

      // Check name of previous entry before defaulting
      PopulateFields(connectionName);

      if(mIsEditMode)
         StealthViewerData::GetInstance().SetOldConnectionName(connectionName);
   }

   ////////////////////////////////////////////////////////////////////
   HLAOptions::~HLAOptions()
   {
      delete mUi;
      mUi = NULL;
   }

   ////////////////////////////////////////////////////////////////////
   QString HLAOptions::GetConnectionName() const
   {
      return mUi->mConnectionNameLineEdit->text();
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::ConnectSlots()
   {
      connect(mUi->mMapToolButton,        SIGNAL(clicked(bool)), 
             this,                        SLOT(OnMapToolButtonClicked(bool)));
      
      connect(mUi->mFedFileToolButton,    SIGNAL(clicked(bool)), 
             this,                        SLOT(OnFedResourceToolButtonClicked(bool)));
      
      connect(mUi->mConfigFileToolButton, SIGNAL(clicked(bool)),
             this,                        SLOT(OnConfigResourceToolButtonClicked(bool)));

      connect(mUi->mButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), 
             this,                        SLOT(OnOk(bool)));

      connect(mUi->mButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), 
             this,                        SLOT(OnCancel(bool)));

      connect(mUi->mRidFileToolButton, SIGNAL(clicked(bool)), 
             this,                     SLOT(OnRidFileToolButtonClicked(bool)));

      connect(mUi->mDISActorToolButton, SIGNAL(clicked(bool)), 
              this,                     SLOT(OnActorXMLFileToolButtonClicked(bool)));

      connect(mUi->mConnectionTypeCombo, SIGNAL(currentIndexChanged(const QString&)),
             this,                     SLOT(OnConnectionTypeComboChanged(const QString&)));
      connect(mUi->mRTIStandardCombo, SIGNAL(currentIndexChanged(const QString&)),
             this,                     SLOT(OnRTIStandardComboChanged(const QString&)));
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnMapToolButtonClicked(bool checked)
   {
      MapSelectDialog dlg(this);
      if(dlg.exec() == QDialog::Accepted)
      {
         QListWidgetItem *item = dlg.GetSelectedItem();
         mUi->mMapLineEdit->setText(item->text());
      }
   }

   ////////////////////////////////////////////////////////////////////
   QString HLAOptions::FindFile(const QString& caption, const QString& startingSubDir, const QString& filter)
   {
      const std::string& context = dtCore::Project::GetInstance().GetContext();
      QString dir  = tr(context.c_str()) + tr("/") + startingSubDir;
      
      QString file = QFileDialog::getOpenFileName(this, caption, 
            dir, filter);

      if(file.isEmpty())
         return file;

      QString qContext = tr(context.c_str());
               
      QString displayName;

      for (unsigned i = 0; displayName.isEmpty() && i < dtCore::Project::GetInstance().GetContextSlotCount(); ++i)
      {
         displayName = ConvertFileName(file, QString(dtCore::Project::GetInstance().GetContext(i).c_str()));
      }

      if (displayName.size() == 0)
      {
         QMessageBox::warning(this, "Invalid selection", tr("The file selected must be within the ") + qContext + tr(" context") , 
                  QMessageBox::Ok, QMessageBox::Ok);
      }

      return displayName;
   }
   
   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnFedResourceToolButtonClicked(bool checked)
   {
      QString result = FindFile(QString("Select a federation resource"), 
            QString("Federations"), QString("Federation Files(*.fed *.xml)"));

      if(!result.isEmpty())
         mUi->mFedFileLineEdit->setText(result);
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnConfigResourceToolButtonClicked(bool checked)
   {
      QString result = FindFile(QString("Select a configuration resource"), 
            QString("Federations"), QString("Configuration Files(*.xml)"));

      /////////////////////////////////////////////////////////////////////
      // Simulate this being loaded to try and catch any exceptions thrown
      // so we can immediately notify the user
      //dtCore::RefPtr<dtHLAGM::HLAComponent> dummyComp = new dtHLAGM::HLAComponent;
      //dtHLAGM::HLAComponentConfig config;
      //result.replace(":", "/");
      //try
      {
      //   config.LoadConfiguration(*dummyComp, result.toStdString());   
      }
      //catch(const dtUtil::Exception &e)
      {
      //   QMessageBox::critical(this, tr("Error"), 
      //                         tr("The configuration resource you have selected is not valid. ") + 
      //                         tr("Please check the file format and try again. ") + 
      //                         tr("Error message to follow: ") + tr(e.What().c_str()), 
       //                        QMessageBox::Ok);
       //  return;
      }
      //////////////////////////////////////////////////////////////////////

      if(!result.isEmpty())
         mUi->mConfigFileLineEdit->setText(result);   
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnRidFileToolButtonClicked(bool checked)
   {
      QString result = FindFile(QString("Select a rid file"), 
            QString("Federations"), QString("RID Files(*.rid *.rid-mc)"));

      if(!result.isEmpty())
         mUi->mRidFileLineEdit->setText(result);   
   }

   ////////////////////////////////////////////////////////////////////////////////
   void HLAOptions::OnActorXMLFileToolButtonClicked(bool checked /*= false*/)
   {
      QString file = QFileDialog::getOpenFileName(this, QString("Select an actor XML file"), 
         QString(), QString("XML Files(*.xml)"));

      if(!file.isEmpty())
         mUi->mDISActorEdit->setText(file);   
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnOk(bool checked)
   {
      int serverGameVersion = 1;
      QString name    = mUi->mConnectionNameLineEdit->text(), 
              map     = mUi->mMapLineEdit->text(), 
              config  = mUi->mConfigFileLineEdit->text(),
              fedFile = mUi->mFedFileLineEdit->text(), 
              fedex   = mUi->mFedExLineEdit->text(), 
              fedName = mUi->mFederateNameLineEdit->text(), 
              ridFile = mUi->mRidFileLineEdit->text(),
              serverIPAddress = mUi->mServerIPAddressEdit->text(),
              serverPort = mUi->mServerPortEdit->text(),
              serverGameName = mUi->mServerGameNameEdit->text(), 
              serverGameVersionStr = mUi->mServerGameVersionEdit->text(),
              disIPAddress = mUi->mDISIPAddressEdit->text(),
              actorXMLFile = mUi->mDISActorEdit->text();
      QString connectionType = mUi->mConnectionTypeCombo->currentText();
      unsigned int disPort = mUi->mDISPortEdit->value();
      bool disBroadcast = mUi->mDISBroadcastPort->isChecked();
      unsigned int disMTU  = mUi->mDISMTUEdit->value();
      unsigned char disExerciseID = mUi->mDISExerciseIDEdit->value();
      unsigned short disSiteID        = mUi->mDISSiteIDEdit->value(),
                     disApplicationID = mUi->mDISApplicationIDEdit->value();

      QString rtiStandard = mUi->mRTIStandardCombo->currentText();
      if (rtiStandard.compare("Other", Qt::CaseInsensitive) == 0)
      {
         rtiStandard = mUi->mRTIStandardManualEdit->text();
      }


          //itemText(mUi->mConnectionTypeCombo->getCurrentIndex()).toStdString();


      if (connectionType.toStdString() != StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER && 
         connectionType.toStdString() != StealthViewerSettings::CONNECTIONTYPE_HLA &&
         connectionType.toStdString() != StealthViewerSettings::CONNECTIONTYPE_DIS)
      {
         QMessageBox::information(this, tr("Error"), tr("Please select a connection type."), 
            QMessageBox::Ok);

         reject();
         return;
      }

      if(name.isEmpty())
      {
         QMessageBox::information(this, tr("Error"), tr("Please enter a connection name."), 
            QMessageBox::Ok);

         reject();
         return;
      }
// The
//      if(map.isEmpty())
//      {
//         QMessageBox::information(this, tr("Error"), tr("Please select a map file."),
//            QMessageBox::Ok);
//
//         reject();
//         return;
//      }

      /////////////////////////////////////////////
      // HLA SETTINGS 
      if (connectionType.toStdString() == StealthViewerSettings::CONNECTIONTYPE_HLA)
      {
         if(config.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please select a configuration file."), 
               QMessageBox::Ok);

            reject();
            return;
         }
         if(fedFile.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please select a federation file."), 
               QMessageBox::Ok);

            reject();
            return;
         }

         if(fedex.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please enter a federation execution name."), 
               QMessageBox::Ok);

            reject();
            return;
         }

         if(fedName.isEmpty())
         {
            fedName = "Stealth Viewer";
         }
      }

      ////////////////////////////////////////////
      // CLIENT - SERVER OPTIONS
      if (connectionType.toStdString() == StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER)
      {
         if(serverIPAddress.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please enter a server IP Address."), 
               QMessageBox::Ok);

            reject();
            return;
         }

         if(serverPort.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please enter a server port."), 
               QMessageBox::Ok);

            reject();
            return;
         }

         if(serverGameName.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please enter a game name (ex Demo)."), 
               QMessageBox::Ok);

            reject();
            return;
         }

         if(serverGameVersionStr.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please enter a game version (ex 1, 2, 3)."), 
               QMessageBox::Ok);

            reject();
            return;
         }
         else 
         {
            bool isInt = false;
            serverGameVersion = serverGameVersionStr.toInt(&isInt);
            if (!isInt)
            {
               QMessageBox::information(this, tr("Error"), tr("Game Version should be an integer (ex 1, 2, 3)."), 
                  QMessageBox::Ok);

               reject();
               return;
            }
         }
      }

      ////////////////////////////////////////////
      // DIS OPTIONS
      if (connectionType.toStdString() == StealthViewerSettings::CONNECTIONTYPE_DIS)
      {
         if(disIPAddress.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please enter a DIS IP Address."), 
               QMessageBox::Ok);

            reject();
            return;
         }

         if(actorXMLFile.isEmpty())
         {
            QMessageBox::information(this, tr("Error"), tr("Please enter an Actor XML file."), 
               QMessageBox::Ok);

            reject();
            return;
         }
      }

      bool success = StealthViewerData::GetInstance().GetSettings().AddConnection
         (name, map, config, fedFile, fedex, fedName, ridFile, rtiStandard, connectionType,
         serverIPAddress, serverPort, serverGameName, serverGameVersionStr,
         disIPAddress,
         disPort,
         disBroadcast,
         disExerciseID,
         disSiteID,
         disApplicationID,
         disMTU,
         actorXMLFile,
         mIsEditMode);

      if(!success)
      {
         mUi->mConnectionNameLineEdit->setText(tr(""));
         reject();
         return;
      }

      accept();
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnCancel(bool checked)
   {
      reject();
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::PopulateFields(const QString &connectionName)
   {
      if(connectionName.isEmpty())
      {
         mUi->mFederateNameLineEdit->setText(tr("Stealth Viewer"));
         OnConnectionTypeComboChanged(QString(StealthViewerSettings::CONNECTIONTYPE_NONE.Get().c_str()));
         return;
      }

      StealthViewerSettings &settings = StealthViewerData::GetInstance().GetSettings();

      QStringList list = settings.GetConnectionProperties(connectionName);
      if(list.isEmpty())
      {
         LOG_ERROR("Failed to find properties for connection: " + connectionName.toStdString());
         return;
      }

      mUi->mConnectionNameLineEdit->setText(list[0]);
      mUi->mMapLineEdit->setText(list[1]);
      mUi->mConfigFileLineEdit->setText(list[2]);
      mUi->mFedFileLineEdit->setText(list[3]);
      mUi->mFedExLineEdit->setText(list[4]);
      mUi->mFederateNameLineEdit->setText(list[5]);
      mUi->mRidFileLineEdit->setText(list[6]);
      int rtiStandardComboIndex =  mUi->mRTIStandardCombo->currentIndex();

      if (!list.value(7).isEmpty())
      {
         rtiStandardComboIndex = mUi->mRTIStandardCombo->findText(list.value(7));
      }

      if (rtiStandardComboIndex >= 0)
      {
         mUi->mRTIStandardCombo->setCurrentIndex(rtiStandardComboIndex);
         mUi->mRTIStandardManualEdit->setText("");
      }
      else
      {
         // Used funny case to make sure insensitive find works.
         rtiStandardComboIndex = mUi->mRTIStandardCombo->findText("oThEr", static_cast<Qt::MatchFlags>(Qt::MatchStartsWith));
         if (rtiStandardComboIndex < 0)
         {
            mUi->mRTIStandardCombo->addItem("Other", QVariant());
            rtiStandardComboIndex = mUi->mRTIStandardCombo->findText("Other", static_cast<Qt::MatchFlags>(Qt::MatchStartsWith));
         }
         mUi->mRTIStandardCombo->setCurrentIndex(rtiStandardComboIndex);
         mUi->mRTIStandardManualEdit->setText(list.value(7));
      }
      // Have to call this in case reading the value from the config doesn't change the combo box value.
      OnRTIStandardComboChanged(mUi->mRTIStandardCombo->currentText());
      
      // Get the new connection settings
      //text 7 is Connection Type. Defaults to HLA
      // value returns a default string if out of bounds, list[8] might crash
      QString connectionType = list.value(8);
      mUi->mServerIPAddressEdit->setText(list.value(9));
      mUi->mServerPortEdit->setText(list.value(10));
      mUi->mServerGameNameEdit->setText(list.value(11));
      mUi->mServerGameVersionEdit->setText(list.value(12));

      // Load DIS Settings
      mUi->mDISIPAddressEdit->setText(list.value(13));
      mUi->mDISPortEdit->setValue(list.value(14).toUInt());
      mUi->mDISBroadcastPort->setChecked(list.value(15) == "true" ? true : false);
      mUi->mDISExerciseIDEdit->setValue(list.value(16).toUInt());
      mUi->mDISSiteIDEdit->setValue(list.value(17).toUShort());
      mUi->mDISApplicationIDEdit->setValue(list.value(18).toUShort());
      mUi->mDISMTUEdit->setValue(list.value(19).toUInt());
      mUi->mDISActorEdit->setText(list.value(20));

      // values 20 and 21 are in the HLA part above :-/

      // We default to HLA in all cases unless ClientServer explictly set.
      // This supports backward compatibility with existing systems.
      if (connectionType.toStdString() == StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER)
      {
         mUi->mConnectionTypeCombo->setCurrentIndex(2);
         OnConnectionTypeComboChanged(QString(StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.Get().c_str()));
      }
      else if (connectionType.toStdString() == StealthViewerSettings::CONNECTIONTYPE_DIS)
      {
         mUi->mConnectionTypeCombo->setCurrentIndex(3);
         OnConnectionTypeComboChanged(QString(StealthViewerSettings::CONNECTIONTYPE_DIS.Get().c_str()));
      }
      else 
      {
         mUi->mConnectionTypeCombo->setCurrentIndex(1);
         OnConnectionTypeComboChanged(QString(StealthViewerSettings::CONNECTIONTYPE_HLA.Get().c_str()));
      }

   }

   ////////////////////////////////////////////////////////////////////
   QString HLAOptions::ConvertFileName(const QString &file, const QString& projectDir) const
   {
      if(projectDir.isEmpty())
         return tr("");
      
      QString project = projectDir;
      project.replace("\\", "/");
      QString modifiedFile(file);
      modifiedFile.replace("\\", "/");
      
      std::string projectPath = project.toStdString();
      if(*projectPath.rbegin() == '/')
         projectPath.erase(projectPath.begin() + (projectPath.size() - 1));
      
      std::string filePath = modifiedFile.toStdString();
      
      if(filePath.size() <= projectPath.size() + 1)
      {
         return tr("");
      }
      
      // Sometimes on windows, the case of the drive letter comes up being different
      // it matches the case on the other files because it asks the system for the paths.
      if(projectPath.substr(1, 2) == ":/")
      {
         projectPath[0] = toupper(projectPath[0]);
         filePath[0] = toupper(filePath[0]);
      }

      std::string tempFilePath    = filePath, 
                  tempProjectPath = projectPath;

      if(!mCaseSensitiveFilePaths)
      {
         for(size_t i = 0; i < tempFilePath.size(); i++)
            tempFilePath[i] = toupper(tempFilePath[i]);

         for(size_t i = 0; i < tempProjectPath.size(); i++)
            tempProjectPath[i] = toupper(tempProjectPath[i]);
      }
      
      if(tempFilePath.substr(0, tempProjectPath.size()) != tempProjectPath || 
         tempFilePath[tempProjectPath.size()] != '/')
         return tr("");
         
      std::string relPath = filePath.substr(projectPath.size() + 1);

      QString displayName = tr(relPath.c_str());

      // Convert to resource descriptor format
      displayName.replace("/", ":");

      return displayName;
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnConnectionTypeComboChanged(const QString& text)
   {
      const std::string connectionType = text.toStdString();

      if (connectionType == StealthViewerSettings::CONNECTIONTYPE_HLA)
      {
         mUi->mHLAOptionsGroup->show();
         mUi->mClientServerGroup->hide();
         mUi->mDISGroup->hide();
      }
      else if (connectionType == StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER)
      {
         mUi->mHLAOptionsGroup->hide();
         mUi->mClientServerGroup->show();
         mUi->mDISGroup->hide();
      }
      else if (connectionType == StealthViewerSettings::CONNECTIONTYPE_DIS)
      {
         mUi->mHLAOptionsGroup->hide();
         mUi->mClientServerGroup->hide();
         mUi->mDISGroup->show();
      }
      else  // unknown still. 
      {
         mUi->mHLAOptionsGroup->hide();
         mUi->mClientServerGroup->hide();
         mUi->mDISGroup->hide();
      }
   }

   ////////////////////////////////////////////////////////////////////
   void HLAOptions::OnRTIStandardComboChanged(const QString& text)
   {
      // mode "other" a funny case so it can always be tested that the insensitivity works.
      bool makeVisible = text.indexOf(QString("OtHeR"), 0, Qt::CaseInsensitive) >= 0;
      mUi->mRTIStandardManualEdit->setVisible(makeVisible);
   }

}
