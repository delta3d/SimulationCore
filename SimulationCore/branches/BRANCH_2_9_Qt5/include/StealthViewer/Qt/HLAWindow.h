/* -*-c++-*-
* Stealth Viewer - HLAWindow (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
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
* @author Eddie Johnson, Curtiss Murphy
*/
#include <dtUtil/warningdisable.h>
DT_DISABLE_WARNING_ALL_START
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
DT_DISABLE_WARNING_END

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

         /// Connects to the network. Returns true for success
         bool SetConnectionValues(QStringList& properties);

         // Connects to network with the last set values.  Returns true for success
         bool Connect();
         // Disconnects the network connection.  Passing false will result it only unloading maps if they are associated with the connection.
         void Disconnect(bool alwaysUnloadMaps);

         /// This exists mainly for the unit tests;
         QListWidget* GetNetworkListWidget();

      signals:

         void ConnectedToNetwork(QString connectionName);
         void ConnectedToNetworkFailed(QString connectionName);

         void DisconnectedFromNetwork(bool mapsUnloaded);

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
         void UpdateConnectText();

         Ui::HLAWindow *mUi;

         bool mIsConnected;

         SimCore::HLA::HLAConnectionComponent *mHLAComp;

         QString mCurrentConnectionName;

         bool mCancelConnectProcess;
   };
}
