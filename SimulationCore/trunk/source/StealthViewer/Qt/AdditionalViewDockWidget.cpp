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

#include <QtOpenGL/QGLWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtGui/QMainWindow>
#include <QtGui/QVBoxLayout>

#include <StealthViewer/Qt/AdditionalViewDockWidget.h>
#include <StealthViewer/Qt/ViewDockWidget.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <StealthViewer/Qt/MainWindow.h>

#include <dtQt/osggraphicswindowqt.h>

#include <dtCore/deltawin.h>
#include <osgViewer/GraphicsWindow>

namespace StealthQt
{
   ////////////////////////////////////////////////////////////
   AdditionalViewDockWidget::AdditionalViewDockWidget(QWidget* parent, const QGLWidget* sharedContextWidget)
   : dtQt::OSGAdapterWidget(false, parent, sharedContextWidget, Qt::Tool | Qt::Window)
   , mGLWidget(NULL)
   {
      setFocusPolicy(Qt::StrongFocus);
      setLayout(new QVBoxLayout(this));

      setMinimumHeight(300);
      setMinimumWidth(400);
      // No docking allowed.
      //setAllowedAreas(Qt::NoDockWidgetArea);
//      connect(this, SIGNAL(topLevelChanged(bool)),
//              this, SLOT(OnTopLevelChanged(bool))
//               );
//      connect(this, SIGNAL(visibilityChanged(bool)),
//              this, SLOT(OnVisibilityChanged(bool))
//               );
   }

   ////////////////////////////////////////////////////////////
   AdditionalViewDockWidget::~AdditionalViewDockWidget()
   {
   }

//   ////////////////////////////////////////////////////////////
//   void AdditionalViewDockWidget::SetQGLWidget(QGLWidget* widgetChild)
//   {
//      if (mGLWidget != NULL)
//      {
//         layout()->removeWidget(mGLWidget);
//      }
//      mGLWidget = widgetChild;
//      //setWidget(mGLWidget);
//      if (mGLWidget != NULL)
//      {
//         layout()->addWidget(mGLWidget);
//      }
//   }
//
//   ////////////////////////////////////////////////////////////
//   QGLWidget* AdditionalViewDockWidget::GetQGLWidget()
//   {
//      return mGLWidget;
//   }

   ////////////////////////////////////////////////////////////
   void AdditionalViewDockWidget::RequestClose()
   {
      StealthViewerData::GetInstance().GetMainWindow()->GetViewDockWidget().OnAdditionalViewClosed(*this);
   }

   ////////////////////////////////////////////////////////////
   void AdditionalViewDockWidget::SetViewWindowWrapper(StealthGM::ViewWindowWrapper* wrapper)
   {
      mViewWrapper = wrapper;
   }

   ////////////////////////////////////////////////////////////
   StealthGM::ViewWindowWrapper* AdditionalViewDockWidget::GetViewWindowWrapper()
   {
      return mViewWrapper.get();
   }

   ////////////////////////////////////////////////////////////
   void AdditionalViewDockWidget::closeEvent(QCloseEvent* e)
   {
      if (e->spontaneous())
      {
         e->ignore();
         if (QMessageBox::question(this, "Close",
                  "Do you want to quit the Stealth Viewer?",
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
         {
            StealthViewerData::GetInstance().GetMainWindow()->close();
         }
         //RequestClose();
      }
      else
      {
         mViewWrapper = NULL;
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   AdditionalViewDockWidget* AdditionalViewDockWidget::GetDockWidgetForViewWindow(StealthGM::ViewWindowWrapper& wrapper)
   {
      osgViewer::GraphicsWindow* gw = wrapper.GetWindow().GetOsgViewerGraphicsWindow();
      dtQt::OSGGraphicsWindowQt* gwQt = dynamic_cast<dtQt::OSGGraphicsWindowQt*>(gw);
      if (gwQt != NULL)
      {
         QGLWidget* widget = gwQt->GetQGLWidget();
         if (widget != NULL)
         {
            //return dynamic_cast<AdditionalViewDockWidget*>(widget->parentWidget());
            return dynamic_cast<AdditionalViewDockWidget*>(widget);
         }
      }
      return NULL;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void AdditionalViewDockWidget::CreateAndInitFromViewWrapper(StealthGM::ViewWindowWrapper& wrapper)
   {
      osgViewer::GraphicsWindow* gw = wrapper.GetWindow().GetOsgViewerGraphicsWindow();
      dtQt::OSGGraphicsWindowQt* gwQt = dynamic_cast<dtQt::OSGGraphicsWindowQt*>(gw);
      if (gwQt != NULL)
      {
         QGLWidget* widget = gwQt->GetQGLWidget();
         if (widget == NULL)
         {
            QMessageBox::critical(StealthViewerData::GetInstance().GetMainWindow(), "Internal Error",
                     "Error Creating Additional 3D view.  OpenGL Widget was not created properly",
                     QMessageBox::Ok, QMessageBox::Ok);
            return;
         }
         AdditionalViewDockWidget* dockWidget = dynamic_cast<AdditionalViewDockWidget*>(widget);
         dtCore::DeltaWin::PositionSize ps = wrapper.GetWindow().GetPosition();
         dockWidget->setGeometry(ps.mX, ps.mY, ps.mWidth, ps.mHeight);
         //dockWidget->SetQGLWidget(widget);
         dockWidget->SetViewWindowWrapper(&wrapper);
         //addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
         //dockWidget->setFloating(true);
         //dockWidget->setWindowTitle(wrapper.GetWindowTitle().c_str());
         //dockWidget->setParent(StealthViewerData::GetInstance().GetMainWindow());
         dockWidget->setWindowFlags(Qt::Tool | Qt::Window | Qt::WindowStaysOnTopHint);
         dockWidget->show();
         gw->resized(ps.mX, ps.mY, ps.mWidth, ps.mHeight);
      }

   }

   ///////////////////////////////////////////////////////////////////////////////
   void AdditionalViewDockWidget::ShutdownOnViewDestroy(StealthGM::ViewWindowWrapper& wrapper)
   {
      AdditionalViewDockWidget* dockWidget =
         AdditionalViewDockWidget::GetDockWidgetForViewWindow(wrapper);
      if (dockWidget != NULL)
      {
//         QMainWindow* qm = dynamic_cast<QMainWindow*>(dockWidget->parentWidget());
//         if (qm != NULL)
//         {
//            qm->removeDockWidget(dockWidget);
//         }

//         dockWidget->setParent(NULL);
//         dockWidget->close();
//         QGLWidget* glWidget = dockWidget->GetQGLWidget();
//         glWidget->setParent(NULL);
         delete dockWidget;
      }
   }
}
