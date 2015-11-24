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

#include <dtUtil/warningdisable.h>
DT_DISABLE_WARNING_ALL_START
#include <QtWidgets/QDialog>
DT_DISABLE_WARNING_END

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

      typedef QDialog BaseClass;

      AdditionalViewEditDialog(StealthGM::ViewWindowWrapper& viewWindow,
               QWidget *parent = NULL, Qt::WindowFlags f = 0);

      virtual ~AdditionalViewEditDialog();

      void SetCancelButtonVisible(bool visible);

      virtual void accept();
      virtual void reject();

   public slots:
      void UpdateName(const QString& name);
      void UpdateHeading(const QString& heading);
      void UpdatePitch(const QString& pitch);
   protected:
      void showEvent(QShowEvent* e);
   private:
      void Init();
      bool NameValid(const std::string& stdName);

      void SetOkButtonEnabled(bool enabled);

      Ui::AdditionalViewDialogUi* mUi;
      FOVWidget* mFOVWidget;
      dtCore::RefPtr<StealthGM::ViewWindowWrapper> mViewWindow;
      QDoubleValidator* mAngleValidator;

      // Fields for storing information prior to editing a view.
      std::string mPrevName;
   };

}

#endif /* ADDITIONALVIEWEDITDIALOG_H_ */
