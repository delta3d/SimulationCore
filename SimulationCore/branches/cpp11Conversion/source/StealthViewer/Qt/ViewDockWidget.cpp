/* -*-c++-*-
 * Stealth Viewer - ViewDockWidget (.h & .cpp) - Using 'The MIT License'
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

#include <prefix/StealthQtPrefix.h>
#include <QtCore/QList>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QMessageBox>

#include <StealthViewer/Qt/ViewDockWidget.h>
#include <StealthViewer/Qt/FOVWidget.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/AdditionalViewEditDialog.h>
#include <StealthViewer/Qt/AdditionalViewDockWidget.h>
#include <StealthViewer/Qt/MainWindow.h>
#include <dtQt/osggraphicswindowqt.h>

#include <StealthViewer/GMApp/ViewWindowConfigObject.h>
#include <ui_Views.h>

namespace StealthQt
{

   ////////////////////////////////////////////////////////////////////////
   ViewDockWidget::ViewDockWidget()
   : mUi(new Ui::ViewDockWidgetUi)
   , mFOVWidget(new FOVWidget)
   {
      Init();
   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::Init()
   {
      mUi->setupUi(this);

      mUi->mFOVPane->layout()->addWidget(mFOVWidget);

      connect(mUi->mNewViewButton, SIGNAL(clicked(bool)),
               this,                SLOT(OnNewViewClicked(bool)));
      connect(mUi->mEditViewButton, SIGNAL(clicked(bool)),
               this,                SLOT(OnEditViewClicked(bool)));
      connect(mUi->mDeleteViewButton, SIGNAL(clicked(bool)),
               this,                SLOT(OnDeleteViewClicked(bool)));
      connect(mUi->mViewWindowListWidget, SIGNAL(itemSelectionChanged()),
               this,                SLOT(OnViewListSelectionChanged()));
   }

   ////////////////////////////////////////////////////////////////////////
   ViewDockWidget::~ViewDockWidget()
   {
      delete mUi;
      mUi = nullptr;
      delete mFOVWidget;
      mFOVWidget = nullptr;
   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::LoadSettings()
   {
       StealthGM::ViewWindowConfigObject& viewConfig =
         StealthViewerData::GetInstance().GetViewWindowConfigObject();
       mFOVWidget->SetViewWindow(&viewConfig.GetMainViewWindow());
       mFOVWidget->Init();

       ResetList();

   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::ResetList()
   {
      StealthGM::ViewWindowConfigObject& viewConfig =
        StealthViewerData::GetInstance().GetViewWindowConfigObject();

      QStringList viewNames;
      std::vector<StealthGM::ViewWindowWrapper*> viewWindows;
      viewConfig.GetAllViewWindows(viewWindows);
      for (size_t i = 0; i < viewWindows.size(); ++i)
      {
         viewNames << viewWindows[i]->GetName().c_str();
      }

      mUi->mViewWindowListWidget->clear();
      mUi->mViewWindowListWidget->addItems(viewNames);

      OnViewListSelectionChanged();
   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::OnNewViewClicked(bool)
   {
      StealthGM::ViewWindowConfigObject& viewConfig =
        StealthViewerData::GetInstance().GetViewWindowConfigObject();

      std::shared_ptr<StealthGM::ViewWindowWrapper> newViewWrap = CreateNewViewWindow("");

      AdditionalViewEditDialog dialog(*newViewWrap, this);
      dialog.SetCancelButtonVisible(true);
      if (dialog.exec() == QDialog::Accepted)
      {
         newViewWrap->SetAttachToCamera(viewConfig.GetMainViewWindow().GetView().GetCamera());
         viewConfig.AddViewWindow(*newViewWrap);
         ResetList();
      }
      else
      {
         newViewWrap = nullptr;
      }
   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::OnEditViewClicked(bool)
   {
      std::shared_ptr<dtCore::View> view = new dtCore::View("");

      StealthGM::ViewWindowConfigObject& viewConfig =
        StealthViewerData::GetInstance().GetViewWindowConfigObject();

      QListWidgetItem* item = mUi->mViewWindowListWidget->currentItem();
      if (item == nullptr)
      {
         // Shouldn't be enabled if there is no selected item.
         mUi->mEditViewButton->setEnabled(false);
         return;
      }

      StealthGM::ViewWindowWrapper* viewWindow = viewConfig.GetViewWindow(item->text().toStdString());

      if (viewWindow != nullptr)
      {
         std::string oldName = viewWindow->GetName();
         AdditionalViewEditDialog dialog(*viewWindow, this);
         dialog.SetCancelButtonVisible(false);
         dialog.exec();

         if (viewWindow->GetName() != oldName)
         {
            viewWindow->SetWindowTitle(viewWindow->GetName());
            viewConfig.UpdateViewName(oldName);
            AdditionalViewDockWidget* widget = AdditionalViewDockWidget::GetDockWidgetForViewWindow(*viewWindow);
            widget->setWindowTitle(tr(viewWindow->GetName().c_str()));
            ResetList();
         }
      }
      else
      {
         QMessageBox::critical(this, tr("Error"), tr("No View named \"") + item->text() + tr("\" exists in the config."),
                  QMessageBox::Ok, QMessageBox::Ok);
      }
   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::OnDeleteViewClicked(bool)
   {
      StealthGM::ViewWindowConfigObject& viewConfig =
        StealthViewerData::GetInstance().GetViewWindowConfigObject();

      QList<QListWidgetItem*> selectedItems = mUi->mViewWindowListWidget->selectedItems();
      QList<QListWidgetItem*>::iterator i, iend;

      std::string currentItemText;
      i = selectedItems.begin();
      iend = selectedItems.end();
      for (; i != iend; ++i)
      {
         mUi->mViewWindowListWidget->removeItemWidget(*i);
         currentItemText = (*i)->text().toStdString();
         StealthGM::ViewWindowWrapper* viewWindow = viewConfig.GetViewWindow(currentItemText);
         if (viewWindow != nullptr)
         {
            viewConfig.RemoveViewWindow(*viewWindow);
         }
      }
      ResetList();
   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::OnViewListSelectionChanged()
   {
      QList<QListWidgetItem*> selectedItems = mUi->mViewWindowListWidget->selectedItems();
      mUi->mEditViewButton->setEnabled(selectedItems.size() == 1);
      mUi->mDeleteViewButton->setEnabled(selectedItems.size() >= 1);
   }

   ////////////////////////////////////////////////////////////////////////
   void ViewDockWidget::OnAdditionalViewClosed(AdditionalViewDockWidget& widgetClosed)
   {
      StealthGM::ViewWindowConfigObject& viewConfig =
        StealthViewerData::GetInstance().GetViewWindowConfigObject();

      if (widgetClosed.GetViewWindowWrapper() != nullptr)
      {
         viewConfig.RemoveViewWindow(*widgetClosed.GetViewWindowWrapper());
      }
      else
      {
         // TODO error
      }
      ResetList();
   }

   ///////////////////////////////////////////////////////////////////
   std::shared_ptr<StealthGM::ViewWindowWrapper> ViewDockWidget::CreateNewViewWindow(const std::string& newViewName)
   {
      MainWindow& mainWindow = *StealthViewerData::GetInstance().GetMainWindow();
      //StealthGM::ViewWindowConfigObject& viewConfig =
      //  StealthViewerData::GetInstance().GetViewWindowConfigObject();

      QRect rect = mainWindow.frameGeometry();
      std::shared_ptr<dtCore::View> view = new dtCore::View(newViewName);
      dtCore::DeltaWin::DeltaWinTraits traits;
      traits.name = newViewName;
      traits.x = rect.top() + 50;
      traits.y = rect.left() + 50;
      traits.height = 256;
      traits.width = 256;
      traits.fullScreen = false;
      traits.showCursor = true;
      traits.realizeUponCreate = false;
      //traits.contextToShare = viewConfig.GetMainViewWindow().GetWindow().GetOsgViewerGraphicsWindow();

      std::shared_ptr<dtCore::DeltaWin> deltaWin =
         new dtCore::DeltaWin(traits);

      std::shared_ptr<StealthGM::ViewWindowWrapper> newViewWrapper =
         new StealthGM::ViewWindowWrapper(newViewName, *view, *deltaWin);

      newViewWrapper->SetInitCallback(StealthGM::ViewWindowWrapper::OperationCallback(&AdditionalViewDockWidget::CreateAndInitFromViewWrapper));
      newViewWrapper->SetRemoveCallback(StealthGM::ViewWindowWrapper::OperationCallback(&AdditionalViewDockWidget::ShutdownOnViewDestroy));

      osgViewer::GraphicsWindow* gw = newViewWrapper->GetWindow().GetOsgViewerGraphicsWindow();
      dtQt::OSGGraphicsWindowQt* gwQt = dynamic_cast<dtQt::OSGGraphicsWindowQt*>(gw);
      if (gwQt != nullptr)
      {
         QGLWidget* widget = gwQt->GetQGLWidget();
         if (widget == nullptr)
         {
            QMessageBox::critical(this, "Internal Error",
                     "Error Creating Additional 3D view.  OpenGL Widget was not created properly",
                     QMessageBox::Ok, QMessageBox::Ok);
            return nullptr;
         }
      }
      return newViewWrapper;
   }

}
