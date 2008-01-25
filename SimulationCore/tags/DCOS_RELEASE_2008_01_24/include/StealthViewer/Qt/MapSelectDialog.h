/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <QtGui/QDialog>

// Forward declarations
namespace Ui
{
   class MapSelectDialog;
}

class QListWidgetItem;

namespace StealthQt
{
   class MapSelectDialog : public QDialog
   {
      Q_OBJECT 

      public:

         /// Constructor
         MapSelectDialog(QWidget *parent = NULL);

         /// Destructor
         virtual ~MapSelectDialog();
         
         /// Returns the currently selected item
         QListWidgetItem* GetSelectedItem();

      protected slots:

         /// Called when a list item is double clicked
         void OnListItemDoubleClicked(QListWidgetItem *item);

      protected:

         Ui::MapSelectDialog *mUi;
   };
}

