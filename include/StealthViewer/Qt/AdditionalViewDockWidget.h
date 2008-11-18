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


#ifndef ADDITIONALVIEWDOCKWIDGET_H_
#define ADDITIONALVIEWDOCKWIDGET_H_

//#include <QtGui/QDockWidget>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>
#include <StealthViewer/Qt/OSGAdapterWidget.h>

class QGLWidget;

namespace StealthQt
{

   /// Simple class to make it easy for code to tell if a dock window is an addition view.
   class AdditionalViewDockWidget: public dtQt::OSGAdapterWidget
   {
      Q_OBJECT;
   public:
      AdditionalViewDockWidget(QWidget* parent = NULL);
      virtual ~AdditionalViewDockWidget();

//      void SetQGLWidget(QGLWidget* widgetChild);
//      QGLWidget* GetQGLWidget();

      void SetViewWindowWrapper(StealthGM::ViewWindowWrapper*);
      StealthGM::ViewWindowWrapper* GetViewWindowWrapper();

      static void CreateAndInitFromViewWrapper(StealthGM::ViewWindowWrapper& wrapper);
      static void ShutdownOnViewDestroy(StealthGM::ViewWindowWrapper& wrapper);
      static AdditionalViewDockWidget* GetDockWidgetForViewWindow(StealthGM::ViewWindowWrapper& wrapper);

      void RequestClose();
   signals:
      void closeRequested(AdditionalViewDockWidget&);
   protected slots:
      void OnVisibilityChanged(bool isVisible);
      void OnTopLevelChanged(bool isTopLevel);
   protected:
      virtual void closeEvent(QCloseEvent *e);
   private:
      QGLWidget* mGLWidget;
      dtCore::RefPtr<StealthGM::ViewWindowWrapper> mViewWrapper;
   };
}

#endif /* ADDITIONALVIEWDOCKWIDGET_H_ */
