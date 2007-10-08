/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <StealthViewer/Qt/FederationFileResourceBrowser.h>
#include <StealthViewer/Qt/ui_FederationFileResourceBrowserUi.h>
#include <QtGui/QHeaderView>

namespace StealthQt
{
   FederationFileResourceBrowser::FederationFileResourceBrowser(QWidget *parent) :
      QDialog(parent), 
      mUi(new Ui::FederationFileResourceBrowser)
   {
      mUi->setupUi(this);
      mUi->mFedrationFilesTreeWidget->header()->hide();
   }

   FederationFileResourceBrowser::~FederationFileResourceBrowser()
   {
      delete mUi;
      mUi = NULL;
   }

   QTreeWidget& FederationFileResourceBrowser::GetTreeWidget()
   {
      return *mUi->mFedrationFilesTreeWidget;
   }
}
