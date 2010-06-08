/* -*-c++-*-
* Stealth Viewer - HLAOptions (.h & .cpp) - Using 'The MIT License'
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
#include <QtGui/QDialog>
#include <dtUtil/refstring.h>

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

   ////////////////////////////////////////////////////////////
   /// The main dialog for options when creating a network connection. Supports both HLA OR Client-Server
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

         /// Called when the actor xml file tool button is clicked
         void OnActorXMLFileToolButtonClicked(bool checked = false);

         /// Called when the Connection Type combo has been changed
         void OnConnectionTypeComboChanged(const QString &text);

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
