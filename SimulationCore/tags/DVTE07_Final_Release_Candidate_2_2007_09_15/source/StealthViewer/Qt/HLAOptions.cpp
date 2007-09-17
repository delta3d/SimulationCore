/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <StealthViewer/Qt/HLAOptions.h>
#include <StealthViewer/Qt/ui_HLAOptionsUi.h>
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
#include <dtCore/globals.h>
#include <dtDAL/project.h>
#include <dtUtil/macros.h>
#include <osgDB/FileNameUtils>

#include <cctype>

namespace StealthQt
{
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

      // Check name of previous entry before defaulting
      PopulateFields(connectionName);
   }

   HLAOptions::~HLAOptions()
   {
      delete mUi;
      mUi = NULL;
   }

   QString HLAOptions::GetConnectionName() const
   {
      return mUi->mConnectionNameLineEdit->text();
   }

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
   }

   void HLAOptions::OnMapToolButtonClicked(bool checked)
   {
      MapSelectDialog dlg(this);
      if(dlg.exec() == QDialog::Accepted)
      {
         QListWidgetItem *item = dlg.GetSelectedItem();
         mUi->mMapLineEdit->setText(item->text());
      }
   }

   QString HLAOptions::FindFile(const QString& caption, const QString& startingSubDir, const QString& filter)
   {
      const std::string &context = dtDAL::Project::GetInstance().GetContext();
      QString dir  = tr(context.c_str()) + tr("/") + startingSubDir;
      
      QString file = QFileDialog::getOpenFileName(this, caption, 
            dir, filter);

      if(file.isEmpty())
         return file;

      QString qContext = tr(context.c_str());
               
      QString displayName = ConvertFileName(file, qContext);

      if (displayName.size() == 0)
         QMessageBox::warning(this, "Invalid selection", tr("The file selected must be within the ") + qContext + tr(" context") , 
                  QMessageBox::Ok, QMessageBox::Ok);
         
      return displayName;
   }
   
   void HLAOptions::OnFedResourceToolButtonClicked(bool checked)
   {

      QString result = FindFile(QString("Select a federation resource"), 
            QString("Federations"), QString("Federation Files(*.fed)"));

      if (!result.isEmpty())
         mUi->mFedFileLineEdit->setText(result);
   }

   void HLAOptions::OnConfigResourceToolButtonClicked(bool checked)
   {
      QString result = FindFile(QString("Select a configuration resource"), 
            QString("Federations"), QString("Configuration Files(*.xml)"));

      if (!result.isEmpty())
         mUi->mConfigFileLineEdit->setText(result);   
   }

   void HLAOptions::OnRidFileToolButtonClicked(bool checked)
   {
      QString result = FindFile(QString("Select a rid file"), 
            QString("Federations"), QString("RID Files(*.rid *.rid-mc)"));

      if (!result.isEmpty())
         mUi->mRidFileLineEdit->setText(result);   
   }

   void HLAOptions::OnOk(bool checked)
   {
      QString name    = mUi->mConnectionNameLineEdit->text(), 
              map     = mUi->mMapLineEdit->text(), 
              config  = mUi->mConfigFileLineEdit->text(),
              fedFile = mUi->mFedFileLineEdit->text(), 
              fedex   = mUi->mFedExLineEdit->text(), 
              fedName = mUi->mFederateNameLineEdit->text(), 
              ridFile = mUi->mRidFileLineEdit->text();

      if(name.isEmpty())
      {
         QMessageBox::information(this, tr("Error"), tr("Please enter a connection name."), 
            QMessageBox::Ok);

         reject();
         return;
      }
      if(map.isEmpty())
      {
         QMessageBox::information(this, tr("Error"), tr("Please select a map file."), 
            QMessageBox::Ok);

         reject();
         return;
      }
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
      if(ridFile.isEmpty())
      {
         ridFile = "RTI.rid";
      }

      bool success = 
         StealthViewerData::GetInstance().GetSettings().AddConnection(name, 
                                                                      map, 
                                                                      config, 
                                                                      fedFile, 
                                                                      fedex, 
                                                                      fedName, 
                                                                      ridFile, 
                                                                      mIsEditMode);

      if(!success)
      {
         mUi->mConnectionNameLineEdit->setText(tr(""));
         reject();
         return;
      }

      accept();
   }

   void HLAOptions::OnCancel(bool checked)
   {
      reject();
   }

   void HLAOptions::PopulateFields(const QString &connectionName)
   {
      if(connectionName.isEmpty())
      {
         mUi->mFederateNameLineEdit->setText(tr("Stealth Viewer"));
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
   }

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
}
