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
   class FederationFileResourceBrowser;
}

class QTreeWidget;

namespace StealthQt
{
   class FederationFileResourceBrowser : public QDialog
   {
      public:

         /// Constructor
         FederationFileResourceBrowser(QWidget *parent = NULL);

         /// Destructor
         virtual ~FederationFileResourceBrowser();

         /// Accessor to the tree list
         QTreeWidget& GetTreeWidget();

      protected:

         Ui::FederationFileResourceBrowser *mUi;
   };
}
