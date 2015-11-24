/* -*-c++-*-
* Stealth Viewer - MapSelectDialog (.h & .cpp) - Using 'The MIT License'
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
* @author Eddie Johnson
*/

#include <dtUtil/warningdisable.h>
DT_DISABLE_WARNING_ALL_START
#include <QtWidgets/QDialog>
DT_DISABLE_WARNING_END

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

