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
