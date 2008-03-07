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
#include <QtGui/QDialog>

namespace dtGame
{
   class GameManager;
}

namespace SimCore
{
   namespace HLA
   {
      class HLAConnectionComponent;
   }
}

namespace Ui
{
   class HLAWindow;
}

class QListWidgetItem;

namespace StealthQt
{
   class StealthViewerSettings;

   class HLAWindow : public QDialog
   {
      Q_OBJECT

      public:

         /// Constructor
         HLAWindow(dtGame::GameManager &gm,
                   QWidget *parent = NULL, 
                   StealthViewerSettings *settings = NULL, 
                   bool isConnected = false, 
                   QString currentConnectionName = tr(""));

         /// Destructor
         virtual ~HLAWindow();

         /// Connects to HLA
         void SetConnectionValues(QStringList &properties);

      signals:

         void ConnectedToHLA(QString connectionName);

         void DisconnectedFromHLA();

      protected slots:

         /// Called when the connect button is clicked
         void OnConnect(bool checked = false);

         /// Called when the disconnect button is clicked
         void OnDisconnect(bool checked = false);

         /// Called when the close button is clicked
         void OnClose(bool checked = false);

         /// Called when the new button is clicked
         void OnNew(bool checked = false);

         /// Called when the edit button is clicked
         void OnEdit(bool checked = false);

         /// Called when the delete button is clicked
         void OnDelete(bool checked = false);

         /// Called when a list item is selected
         void OnCurrentTextChanged(const QString &str);

         /// Called when an item is deleted 
         void OnListItemDeleted(QString name);

         /// Called when an item is activated
         void OnListItemActivated(QListWidgetItem *item);

     protected:

         /// Private helper method to connect slots
         void ConnectSlots();

         Ui::HLAWindow *mUi;

         bool mIsConnected;

         SimCore::HLA::HLAConnectionComponent *mHLAComp;

         QString mCurrentConnectionName;

         bool mCancelConnectProcess;
   };
}
