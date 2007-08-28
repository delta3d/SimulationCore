/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <QtGui/QDialog>

namespace dtGame
{
   class GameManager;
}

namespace SimCore
{
   namespace Components
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

         SimCore::Components::HLAConnectionComponent *mHLAComp;

         QString mCurrentConnectionName;
   };
}
