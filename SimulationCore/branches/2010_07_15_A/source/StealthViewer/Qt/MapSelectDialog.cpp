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
 */
#include <prefix/StealthQtPrefix.h>
#include <StealthViewer/Qt/MapSelectDialog.h>
#include <ui_MapSelectDialogUi.h>
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
