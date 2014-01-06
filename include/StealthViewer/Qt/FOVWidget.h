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

#ifndef FOVWIDGET_H_
#define FOVWIDGET_H_

#include <QtGui/QWidget>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>

namespace Ui
{
   class FOVWidgetUi;
}

class QDoubleValidator;

namespace StealthQt
{

   /**
    * @class FOVWidget
    * The widget sets the field of view for a single camer view.
    */
   class FOVWidget: public QWidget
   {
      Q_OBJECT
   public:
      FOVWidget();
      virtual ~FOVWidget();

      /// Call this after the view window is set.
      void Init();

      void SetViewWindow(StealthGM::ViewWindowWrapper* viewWindow);
      StealthGM::ViewWindowWrapper* GetViewWindow();

   public slots:

      /// Fired when the aspect/vertical radio button is toggled.
      void OnFOVAspectVerticalToggled(bool checked = false);
      /// This is a catch all for all the different fov fields.
      void OnFOVChange(const QString& text);

      /// This handles the reset button for the field of view.
      void OnFOVReset(bool checked = false);

      /// Sets the Field of view UI values by reading them from the general config.
      void AssignFOVUiValuesFromConfig();
   private:
      Ui::FOVWidgetUi* mUi;
      std::shared_ptr<StealthGM::ViewWindowWrapper> mViewWindow;

      QDoubleValidator* mGtZeroValidator;
      QDoubleValidator* mFOVValidator;
   };

}

#endif /* FOVWIDGET_H_ */
