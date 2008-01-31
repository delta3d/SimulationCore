/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <StealthViewer/Qt/MapSelectDialog.h>
#include <StealthViewer/Qt/ui_MapSelectDialogUi.h>
#include <dtDAL/project.h>

namespace StealthQt
{
   MapSelectDialog::MapSelectDialog(QWidget *parent) :   
      QDialog(parent), 
      mUi(new Ui::MapSelectDialog)
   {
      mUi->setupUi(this);

      const std::set<std::string> &maps = dtDAL::Project::GetInstance().GetMapNames();
      for(std::set<std::string>::const_iterator i = maps.begin(); i != maps.end(); ++i)
      {
         QString str = (*i).c_str();
         mUi->mMapSelectListWidget->addItem(str);
      }

      connect(mUi->mMapSelectListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), 
              this,                      SLOT(OnListItemDoubleClicked(QListWidgetItem*)));
   }

   MapSelectDialog::~MapSelectDialog()
   {
      delete mUi;
      mUi = NULL;
   }

   QListWidgetItem* MapSelectDialog::GetSelectedItem()
   {
      return mUi->mMapSelectListWidget->currentItem();
   }

   void MapSelectDialog::OnListItemDoubleClicked(QListWidgetItem *item)
   {
      mUi->mMapSelectListWidget->setCurrentItem(item);
      accept();
   }
}
