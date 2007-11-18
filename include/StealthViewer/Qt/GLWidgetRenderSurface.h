/*
 *  RenderSurfaceQt.h
 *  DVTE
 *
 *  Created by David Guthrie on 2/7/07.
 *  Copyright 2007 Alion Science and Technology. All rights reserved.
 *
 */

#ifndef GL_WIDGET_RENDER_SURFACE
#define GL_WIDGET_RENDER_SURFACE

#include <dtCore/refptr.h>
#include <QtOpenGL/QGLWidget>
#include <QtCore/QTimer>

namespace dtCore
{
   class DeltaWin;
   class Camera;
}

namespace StealthQt
{
   struct KeyMapTable;
   
   class GLWidgetRenderSurface : public QGLWidget 
   {
      public:
         GLWidgetRenderSurface(dtCore::DeltaWin& deltaWin, dtCore::Camera& camera, 
               QWidget* parent = NULL, const QGLWidget* shareWidget = NULL, Qt::WindowFlags f = 0);
               
      protected:
      
         virtual void resizeGL( int width, int height );
         virtual void initializeGL();
         virtual void paintGL();
         
         
         void keyPressEvent(QKeyEvent* event);

         void keyReleaseEvent(QKeyEvent* event);

         void mousePressEvent(QMouseEvent* event);

         void mouseReleaseEvent(QMouseEvent* event);

         void mouseMoveEvent(QMouseEvent* event);

         static QGLFormat GetDefaultGLFormat();

      private: 
         int GetKey(QKeyEvent& event) const;
         
         void ConvertMouseXYFromWindowSpace(int x, int y, float& fx, float& fy) const;
         
         std::map<int, KeyMapTable> mKeyMapping;
         
         dtCore::RefPtr<dtCore::DeltaWin> mDeltaWin;
         dtCore::RefPtr<dtCore::Camera> mCamera;
         QTimer mTimer;
         
   };
   
}
#endif
