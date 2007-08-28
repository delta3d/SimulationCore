/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <QtGui/QDialog>

// Foward declarations
namespace Ui
{
   class HLAOptions;
}

class QListWidgetItem;

namespace StealthQt
{
   class MapSelectDialog;
   class FederationFileResourceBrowser;

   class HLAOptions : public QDialog
   {
      Q_OBJECT

      public:

         /// Constructor
         HLAOptions(QWidget *parent = NULL, 
                    const QString &connectionName = "",
                    bool isEditMode = false);

         /// Destructor
         virtual ~HLAOptions();

         /// Returns the name of the connection created
         QString GetConnectionName() const;

         /// Sets checking for file paths case sensitively
         void SetCaseSensitiveFilePaths(bool enable) { mCaseSensitiveFilePaths = enable; }

         /// Returns true if paths are case sensitive
         bool GetCaseSensitiveFilePaths() const { return mCaseSensitiveFilePaths; }

      protected slots:

         /// Called when the map tool button is clicked
         void OnMapToolButtonClicked(bool checked = false);

         /// Called when the federation file tool button is clicked
         void OnFedResourceToolButtonClicked(bool checked = false);

         /// Called when the configuration file tool button is clicked
         void OnConfigResourceToolButtonClicked(bool checked = false);

         /// Called when OK is clicked
         void OnOk(bool checked = false);

         /// Called when Cancel is clicked
         void OnCancel(bool checked = false);

         /// Called when the rid file tool button is clicked
         void OnRidFileToolButtonClicked(bool checked = false);

      protected:

         QString FindFile(const QString& caption, const QString& startingSubDir, const QString& filter);

         /// Private helper method to populate text fields
         /// This method is called if the connectionName parameter is 
         /// supplied to the constructor
         void PopulateFields(const QString &connectionName);

         /// Private helper method to connect the slots
         void ConnectSlots();

         /// Private helper method to display a file by project context path
         QString ConvertFileName(const QString &file, const QString& projectDir) const;

         Ui::HLAOptions *mUi;

         bool mCaseSensitiveFilePaths;

         bool mIsEditMode;
   };
}
