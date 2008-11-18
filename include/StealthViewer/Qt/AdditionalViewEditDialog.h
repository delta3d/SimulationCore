/* -*-c++-*-
 * Stealth Viewer - AdditionalViewDockWidget (.h & .cpp) - Using 'The MIT License'
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
 * David Guthrie
 */

#ifndef ADDITIONALVIEWEDITDIALOG_H_
#define ADDITIONALVIEWEDITDIALOG_H_

#include <QtGui/QDialog>

#include <StealthViewer/GMApp/ViewWindowConfigObject.h>

class QDoubleValidator;

namespace Ui
{
   class AdditionalViewDialogUi;
}

namespace StealthQt
{
   class FOVWidget;

   /**
    * Editor dialog for additional views.
    */
   class AdditionalViewEditDialog : public QDialog
   {
      Q_OBJECT
   public:

      AdditionalViewEditDialog(StealthGM::ViewWindowWrapper& viewWindow,
               QWidget *parent = NULL, Qt::WindowFlags f = 0);

      virtual ~AdditionalViewEditDialog();

   public slots:
      void UpdateName(const QString& name);
      void UpdateHeading(const QString& heading);
      void UpdatePitch(const QString& pitch);
   protected:
      void closeEvent(QCloseEvent* e);
   private:
      void Init();
      bool NameValid(const std::string& stdName);

      Ui::AdditionalViewDialogUi* mUi;
      FOVWidget* mFOVWidget;
      dtCore::RefPtr<StealthGM::ViewWindowWrapper> mViewWindow;
      QDoubleValidator* mAngleValidator;
   };

}

#endif /* ADDITIONALVIEWEDITDIALOG_H_ */
