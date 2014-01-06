/* -*-c++-*-
 * Stealth Viewer - FOV Group Box (.h & .cpp) - Using 'The MIT License'
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
 * @author David Guthrie
 */

#include <prefix/StealthQtPrefix.h>
#include <QtCore/QString>
#include <QtCore/QString>

#include <QtGui/QDoubleValidator>

#include <StealthViewer/Qt/FOVWidget.h>
#include <StealthViewer/Qt/StealthViewerData.h>

#include <StealthViewer/GMApp/ViewWindowConfigObject.h>

#include <ui_FOVGroupBox.h>

namespace StealthQt
{
   ///////////////////////////////////////////////////////////////////////////////
   FOVWidget::FOVWidget()
   : mUi(new Ui::FOVWidgetUi)
   , mGtZeroValidator(new QDoubleValidator(0, DBL_MAX, 5, this))
   , mFOVValidator(new QDoubleValidator(2.0, 179.0, 2, this))
   {
      mUi->setupUi(this);

      mUi->mFOVAspectRatioText->setValidator(mGtZeroValidator);
      mUi->mFOVHorizontalText->setValidator(mFOVValidator);
      mUi->mFOVVerticalwHorizontalText->setValidator(mFOVValidator);
      mUi->mFOVVerticalwAspectText->setValidator(mFOVValidator);

      connect(mUi->mFOVAspectVerticalRadio, SIGNAL(toggled(bool)),
               this,                        SLOT(OnFOVAspectVerticalToggled(bool)));
      connect(mUi->mFOVAspectRatioText,     SIGNAL(textChanged(const QString&)),
               this,                        SLOT(OnFOVChange(const QString&)));
      connect(mUi->mFOVHorizontalText,      SIGNAL(textChanged(const QString&)),
               this,                        SLOT(OnFOVChange(const QString&)));
      connect(mUi->mFOVVerticalwAspectText, SIGNAL(textChanged(const QString&)),
               this,                        SLOT(OnFOVChange(const QString&)));
      connect(mUi->mFOVVerticalwHorizontalText, SIGNAL(textChanged(const QString&)),
               this,                        SLOT(OnFOVChange(const QString&)));

      connect(mUi->mFOVResetButton, SIGNAL(clicked(bool)),
               this,                SLOT(OnFOVReset(bool)));
   }

   ///////////////////////////////////////////////////////////////////////////////
   FOVWidget::~FOVWidget()
   {
      delete mUi;
      mUi = nullptr;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void FOVWidget::Init()
   {
      if (!mViewWindow.valid())
      {
         return;
      }

      AssignFOVUiValuesFromConfig();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void FOVWidget::SetViewWindow(StealthGM::ViewWindowWrapper* viewWindow)
   {
      mViewWindow = viewWindow;
   }

   ///////////////////////////////////////////////////////////////////////////////
   StealthGM::ViewWindowWrapper* FOVWidget::GetViewWindow()
   {
      return mViewWindow.get();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void FOVWidget::OnFOVChange(const QString&)
   {

      QString textValue = mUi->mFOVAspectRatioText->text();
      mViewWindow->SetFOVAspectRatio(textValue.toFloat());

      textValue = mUi->mFOVHorizontalText->text();
      mViewWindow->SetFOVHorizontal(textValue.toFloat());

      textValue = mUi->mFOVVerticalwAspectText->text();
      mViewWindow->SetFOVVerticalForAspect(textValue.toFloat());

      textValue = mUi->mFOVVerticalwHorizontalText->text();
      mViewWindow->SetFOVVerticalForHorizontal(textValue.toFloat());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void FOVWidget::OnFOVReset(bool)
   {
      mViewWindow->FOVReset();
      AssignFOVUiValuesFromConfig();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void FOVWidget::AssignFOVUiValuesFromConfig()
   {

      mUi->mFOVAspectRatioText->blockSignals(true);
      mUi->mFOVHorizontalText->blockSignals(true);
      mUi->mFOVVerticalwAspectText->blockSignals(true);
      mUi->mFOVVerticalwHorizontalText->blockSignals(true);

      mUi->mFOVAspectRatioText->setText(QString::number(mViewWindow->GetFOVAspectRatio(), 'g', 5));
      mUi->mFOVHorizontalText->setText(QString::number(mViewWindow->GetFOVHorizontal(), 'g', 5));
      mUi->mFOVVerticalwAspectText->setText(QString::number(mViewWindow->GetFOVVerticalForAspect(), 'g', 5));
      mUi->mFOVVerticalwHorizontalText->setText(QString::number(mViewWindow->GetFOVVerticalForHorizontal(), 'g', 5));

      mUi->mFOVAspectRatioText->blockSignals(false);
      mUi->mFOVHorizontalText->blockSignals(false);
      mUi->mFOVVerticalwAspectText->blockSignals(false);
      mUi->mFOVVerticalwHorizontalText->blockSignals(false);

      mUi->mFOVAspectVerticalRadio->setChecked(mViewWindow->UseAspectRatioForFOV());
      mUi->mFOVHorizontalVerticalRadio->setChecked(!mViewWindow->UseAspectRatioForFOV());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void FOVWidget::OnFOVAspectVerticalToggled(bool checked)
   {
      mViewWindow->SetUseAspectRatioForFOV(checked);
   }


}
