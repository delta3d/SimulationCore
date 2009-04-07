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

#include <QtGui/QDoubleValidator>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>

#include <StealthViewer/Qt/AdditionalViewEditDialog.h>
#include <StealthViewer/Qt/FOVWidget.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <ui_AdditionalViewDialog.h>

namespace StealthQt
{
   ////////////////////////////////////////////////////////////////////////
   AdditionalViewEditDialog::AdditionalViewEditDialog(StealthGM::ViewWindowWrapper& viewWindow,
            QWidget* parent, Qt::WindowFlags f)
   : QDialog(parent, f)
   , mUi(new Ui::AdditionalViewDialogUi)
   , mFOVWidget(new FOVWidget)
   , mViewWindow(&viewWindow)
   , mAngleValidator(new QDoubleValidator(-180, 180, 10, this))
   {
      Init();
   }

   ////////////////////////////////////////////////////////////////////////
   void AdditionalViewEditDialog::Init()
   {
      mUi->setupUi(this);

      //QVBoxLayout* boxLayout = new QVBoxLayout(mUi->mFOVPane);
      //mUi->mFOVPane->setLayout(boxLayout);
      //boxLayout->addWidget(mFOVWidget);
      mUi->mFOVPane->layout()->addWidget(mFOVWidget);
      mFOVWidget->SetViewWindow(mViewWindow.get());
      mFOVWidget->Init();

      mUi->mViewAzimuth->setValidator(mAngleValidator);
      mUi->mViewElevation->setValidator(mAngleValidator);

      mUi->mNameEdit->setText(tr(mViewWindow->GetName().c_str()));
      //Negating the heading so it rotates to the positive in the left direction as per the military.
      mUi->mViewAzimuth->setText(QString::number(-mViewWindow->GetAttachCameraRotation()[0], 'g', 5));
      mUi->mViewElevation->setText(QString::number(mViewWindow->GetAttachCameraRotation()[1], 'g', 5));

      connect(mUi->mNameEdit, SIGNAL(textChanged(const QString&)),
               this,          SLOT(UpdateName(const QString&)));
      connect(mUi->mViewAzimuth, SIGNAL(textChanged(const QString&)),
               this,          SLOT(UpdateHeading(const QString&)));
      connect(mUi->mViewElevation, SIGNAL(textChanged(const QString&)),
               this,          SLOT(UpdatePitch(const QString&)));

      UpdateName(mUi->mNameEdit->text());
   }

   ////////////////////////////////////////////////////////////////////////
   bool AdditionalViewEditDialog::NameValid(const std::string& stdName)
   {
      StealthGM::ViewWindowConfigObject& viewConfig =
        StealthViewerData::GetInstance().GetViewWindowConfigObject();

      StealthGM::ViewWindowWrapper* vww = viewConfig.GetViewWindow(stdName);
      return !stdName.empty() && (vww == NULL || vww == mViewWindow.get());
   }

   ////////////////////////////////////////////////////////////////////////
   void AdditionalViewEditDialog::UpdateName(const QString& name)
   {
      std::string stdName = name.toStdString();
      mViewWindow->SetName(stdName);
      mViewWindow->SetWindowTitle(stdName);

      bool canClose = NameValid(stdName);

      mUi->buttonBox->setEnabled(canClose);
   }

   ////////////////////////////////////////////////////////////////////////
   void AdditionalViewEditDialog::UpdateHeading(const QString& heading)
   {
      // We use the negative azimuth because the military does azimuth clockwise while hpr has a
      // counter-clockwise heading.
      osg::Vec3 hpr = mViewWindow->GetAttachCameraRotation();
      hpr[0] = -(heading.toFloat());
      mViewWindow->SetAttachCameraRotation(hpr);
   }

   ////////////////////////////////////////////////////////////////////////
   void AdditionalViewEditDialog::UpdatePitch(const QString& pitch)
   {
      osg::Vec3 hpr = mViewWindow->GetAttachCameraRotation();
      hpr[1] = pitch.toFloat();
      mViewWindow->SetAttachCameraRotation(hpr);
   }

   ////////////////////////////////////////////////////////////////////////
   AdditionalViewEditDialog::~AdditionalViewEditDialog()
   {
      delete mUi;
      mUi = NULL;
      delete mFOVWidget;
      mFOVWidget = NULL;
   }

   void AdditionalViewEditDialog::closeEvent(QCloseEvent* e)
   {
      if (!NameValid(mUi->mNameEdit->text().toStdString()))
      {
         e->ignore();
         if (mUi->mNameEdit->text().isEmpty())
         {
            QMessageBox::information(this, tr("Invalid Name"),
                     "View names may not be empty.");
         }
         else
         {
            QMessageBox::information(this, tr("Invalid Name"),
                     "The View name is either invalid or is a duplicate of another view.");
         }
      }
   }
}
