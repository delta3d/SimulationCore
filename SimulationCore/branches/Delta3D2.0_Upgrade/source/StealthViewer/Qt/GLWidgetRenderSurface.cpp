/**
 * David Guthrie
 */

#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

#include <StealthViewer/Qt/GLWidgetRenderSurface.h>

#include <dtCore/deltawin.h>
#include <dtCore/system.h>
//#include <dtCore/inputcallback.h>
#include <dtCore/camera.h>

#include <osgGA/GUIEventAdapter>

namespace StealthQt
{
   
   struct KeyMapTable
   {
      int mQtKey;
      int mOSGKey;
      int mShiftKey;
   };

   ///////////////////////////////////////////////////////////////////////////////
   QGLFormat GLWidgetRenderSurface::GetDefaultGLFormat()
   {
      QGLFormat f = QGLFormat::defaultFormat();
      f.setDirectRendering(true);
      f.setDoubleBuffer(true);
      f.setDepth(true);
      return f;
   }
   
   ///////////////////////////////////////////////////////////////////////////////
   GLWidgetRenderSurface::GLWidgetRenderSurface(dtCore::DeltaWin& deltaWin,
         dtCore::Camera& camera, QWidget* parent, const QGLWidget* shareWidget, Qt::WindowFlags f):
         QGLWidget(GetDefaultGLFormat(), parent, shareWidget, f),
         mDeltaWin(&deltaWin),
         mCamera(&camera)
   {
      //mCamera->SetAutoAspect(true);
      connect(&mTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
      mTimer.setInterval(10);
      setFocusPolicy(Qt::StrongFocus);
      setAutoBufferSwap(false);
      
      KeyMapTable keyTable[] = 
      {
        // Qt Key              OSG Key                                Shifted Key
        // ------              -------                                -----------
         //{ Qt::Key_unknown,    osgGA::GUIEventAdapter::KEY_Unknown,   osgGA::GUIEventAdapter::KEY_Unknown   },
         { Qt::Key_Escape,     osgGA::GUIEventAdapter::KEY_Escape,     osgGA::GUIEventAdapter::KEY_Escape    },
         { Qt::Key_F1,         osgGA::GUIEventAdapter::KEY_F1,         osgGA::GUIEventAdapter::KEY_F1        },
         { Qt::Key_F2,         osgGA::GUIEventAdapter::KEY_F2,         osgGA::GUIEventAdapter::KEY_F2        },
         { Qt::Key_F3,         osgGA::GUIEventAdapter::KEY_F3,         osgGA::GUIEventAdapter::KEY_F3        },
         { Qt::Key_F4,         osgGA::GUIEventAdapter::KEY_F4,         osgGA::GUIEventAdapter::KEY_F4        },
         { Qt::Key_F5,         osgGA::GUIEventAdapter::KEY_F5,         osgGA::GUIEventAdapter::KEY_F5        },
         { Qt::Key_F6,         osgGA::GUIEventAdapter::KEY_F6,         osgGA::GUIEventAdapter::KEY_F6        },
         { Qt::Key_F7,         osgGA::GUIEventAdapter::KEY_F7,         osgGA::GUIEventAdapter::KEY_F7        },
         { Qt::Key_F8,         osgGA::GUIEventAdapter::KEY_F8,         osgGA::GUIEventAdapter::KEY_F8        },
         { Qt::Key_F9,         osgGA::GUIEventAdapter::KEY_F9,         osgGA::GUIEventAdapter::KEY_F9        },
         { Qt::Key_F10,        osgGA::GUIEventAdapter::KEY_F10,        osgGA::GUIEventAdapter::KEY_F10       },
         { Qt::Key_F11,        osgGA::GUIEventAdapter::KEY_F11,        osgGA::GUIEventAdapter::KEY_F11       },
         { Qt::Key_F12,        osgGA::GUIEventAdapter::KEY_F12,        osgGA::GUIEventAdapter::KEY_F12       },
         { Qt::Key_Backspace,  osgGA::GUIEventAdapter::KEY_BackSpace,  osgGA::GUIEventAdapter::KEY_BackSpace },
         { Qt::Key_Tab,        osgGA::GUIEventAdapter::KEY_Tab,        osgGA::GUIEventAdapter::KEY_Tab       },
         { Qt::Key_CapsLock,   osgGA::GUIEventAdapter::KEY_Caps_Lock,  osgGA::GUIEventAdapter::KEY_Caps_Lock },
         { Qt::Key_Return,     osgGA::GUIEventAdapter::KEY_Return,     osgGA::GUIEventAdapter::KEY_Return    },
         { Qt::Key_Shift,      osgGA::GUIEventAdapter::KEY_Shift_L,    osgGA::GUIEventAdapter::KEY_Shift_L   },
         { Qt::Key_Shift,      osgGA::GUIEventAdapter::KEY_Shift_R,    osgGA::GUIEventAdapter::KEY_Shift_R   },
         { Qt::Key_Control,    osgGA::GUIEventAdapter::KEY_Control_L,  osgGA::GUIEventAdapter::KEY_Control_R },
         { Qt::Key_Space,      osgGA::GUIEventAdapter::KEY_Space,      osgGA::GUIEventAdapter::KEY_Space     },
         { Qt::Key_Alt,        osgGA::GUIEventAdapter::KEY_Alt_L,      osgGA::GUIEventAdapter::KEY_Alt_L     },
         { Qt::Key_Alt,        osgGA::GUIEventAdapter::KEY_Alt_R,      osgGA::GUIEventAdapter::KEY_Alt_R     },
         { Qt::Key_Menu,       osgGA::GUIEventAdapter::KEY_Menu,       osgGA::GUIEventAdapter::KEY_Menu      },
         { Qt::Key_Control,    osgGA::GUIEventAdapter::KEY_Control_L,  osgGA::GUIEventAdapter::KEY_Control_L },
         { Qt::Key_Control,    osgGA::GUIEventAdapter::KEY_Control_R,  osgGA::GUIEventAdapter::KEY_Control_R },
         { Qt::Key_Print,      osgGA::GUIEventAdapter::KEY_Print,      osgGA::GUIEventAdapter::KEY_Print     },
         { Qt::Key_ScrollLock, osgGA::GUIEventAdapter::KEY_Num_Lock,   osgGA::GUIEventAdapter::KEY_Num_Lock  },
         { Qt::Key_Pause,      osgGA::GUIEventAdapter::KEY_Pause,      osgGA::GUIEventAdapter::KEY_Pause     },
         { Qt::Key_Home,       osgGA::GUIEventAdapter::KEY_Home,       osgGA::GUIEventAdapter::KEY_Home      },
         { Qt::Key_PageUp,     osgGA::GUIEventAdapter::KEY_Page_Up,    osgGA::GUIEventAdapter::KEY_Page_Up   },
         { Qt::Key_End,        osgGA::GUIEventAdapter::KEY_End,        osgGA::GUIEventAdapter::KEY_End       },
         { Qt::Key_PageDown,   osgGA::GUIEventAdapter::KEY_Page_Down,  osgGA::GUIEventAdapter::KEY_Page_Down },
         { Qt::Key_Delete,     osgGA::GUIEventAdapter::KEY_Delete,     osgGA::GUIEventAdapter::KEY_Delete    },
         { Qt::Key_Insert,     osgGA::GUIEventAdapter::KEY_Insert,     osgGA::GUIEventAdapter::KEY_Insert    },
         { Qt::Key_Left,       osgGA::GUIEventAdapter::KEY_Left,       osgGA::GUIEventAdapter::KEY_Left      },
         { Qt::Key_Up,         osgGA::GUIEventAdapter::KEY_Up,         osgGA::GUIEventAdapter::KEY_Up        },
         { Qt::Key_Right,      osgGA::GUIEventAdapter::KEY_Right,      osgGA::GUIEventAdapter::KEY_Right     },
         { Qt::Key_Down,       osgGA::GUIEventAdapter::KEY_Down,       osgGA::GUIEventAdapter::KEY_Down      },
         { Qt::Key_NumLock,    osgGA::GUIEventAdapter::KEY_Num_Lock,   osgGA::GUIEventAdapter::KEY_Num_Lock  },
 
         { Qt::Key_QuoteLeft,    '"', '"' },
         { Qt::Key_1,            '1', '1' },
         { Qt::Key_2,            '2', '2' },
         { Qt::Key_3,            '3', '3' },
         { Qt::Key_4,            '4', '4' },
         { Qt::Key_5,            '5', '5' },
         { Qt::Key_6,            '6', '6' },
         { Qt::Key_7,            '7', '7' },
         { Qt::Key_8,            '8', '8' },
         { Qt::Key_9,            '9', '9' },
         { Qt::Key_0,            '0', '0' },
         { Qt::Key_Minus,        '-', '-' },
         { Qt::Key_Equal,        '=', '=' },
         
         { Qt::Key_A,            'a', 'A' },
         { Qt::Key_B,            'b', 'B' },
         { Qt::Key_C,            'c', 'C' },
         { Qt::Key_D,            'd', 'D' },
         { Qt::Key_E,            'e', 'E' },
         { Qt::Key_F,            'f', 'F' },
         { Qt::Key_G,            'g', 'G' },
         { Qt::Key_H,            'h', 'H' },
         { Qt::Key_I,            'i', 'I' },
         { Qt::Key_J,            'j', 'J' },
         { Qt::Key_K,            'k', 'K' },
         { Qt::Key_L,            'l', 'L' },
         { Qt::Key_M,            'm', 'M' },
         { Qt::Key_N,            'n', 'N' },
         { Qt::Key_O,            'o', 'O' },
         { Qt::Key_P,            'p', 'P' },
         { Qt::Key_Q,            'q', 'Q' },
         { Qt::Key_R,            'r', 'R' },
         { Qt::Key_S,            's', 'S' },
         { Qt::Key_T,            't', 'T' },
         { Qt::Key_U,            'u', 'U' },
         { Qt::Key_V,            'v', 'V' },
         { Qt::Key_W,            'w', 'W' },
         { Qt::Key_X,            'x', 'X' },
         { Qt::Key_Y,            'y', 'Y' },
         { Qt::Key_Z,            'z', 'Z' },

         { Qt::Key_Backslash,    '"\"', '|' },

         { Qt::Key_BracketLeft,  '[', '{' },
         { Qt::Key_BracketRight, ']', '}' },
         { Qt::Key_Semicolon,    ';', ':' },
         { Qt::Key_Comma,        ',', '<' },
         { Qt::Key_Period,       '.', '>' },
         { Qt::Key_Slash,        '/', '?' }, 
         
         //{ Qt::Key_Apostrophe,   '"'"' },
         //{ Qt::Key_Super_L,      },
         //{ Qt::Key_Super_R,      },
      };

      mKeyMapping.clear();

      int n = sizeof(keyTable)/sizeof(KeyMapTable);
      for (int i = 0; i < n; ++i)
      {
         mKeyMapping.insert(std::make_pair(keyTable[i].mQtKey, keyTable[i]));
      }
   }    

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::resizeGL( int width, int height )
   {   
      // TODO-UPGRADE
      //mDeltaWin->GetRenderSurface()->setWindowRectangle(0, 0, width, height, false);
      ////////////////////////////////
      mCamera->GetOSGCamera()->setViewport(0, 0, width, height);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::paintGL()
   {
      dtCore::System::GetInstance().Step();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::initializeGL()
   {  
      //have to set the window id and the context id.  Neither Qt nor Producer provide a means
      //get the current context, so here we are with the ifdefs.
      // TODO-UPGRADE
      /*
      mDeltaWin->GetRenderSurface()->setWindow(Producer::Window(winId()));
      mDeltaWin->GetRenderSurface()->setParentWindow(Producer::Window(parentWidget()->winId()));
      mDeltaWin->GetRenderSurface()->bindInputRectangleToWindowSize(true);
#if defined(_OSX_AGL_IMPLEMENTATION)
      mDeltaWin->GetRenderSurface()->setGLContext(aglGetCurrentContext());
#elif defined(_OSX_CGL_IMPLEMENTATION)
      mDeltaWin->GetRenderSurface()->setGLContext(CGLGetCurrentContext());
#elif defined(_X11_IMPLEMENTATION)
      mDeltaWin->GetRenderSurface()->setGLContext(glXGetCurrentContext());
#elif defined(_WIN32_IMPLEMENTATION)
      mDeltaWin->GetRenderSurface()->setGLContext(wglGetCurrentContext());
#endif
      ///////////////////////////////////////////////////
      */
      mTimer.start();
   }

   ///////////////////////////////////////////////////////////////////////////////
   int GLWidgetRenderSurface::GetKey(QKeyEvent& event) const
   {
      std::map<int, KeyMapTable>::const_iterator i = mKeyMapping.find(event.key());
      
      if (i != mKeyMapping.end())
      {  
         if ((event.modifiers() | Qt::ShiftModifier) != 0)
            return i->second.mShiftKey;
         else
            return i->second.mOSGKey;
      }
      return -1;//osgGA::GUIEventAdapter::KEY_Unknown;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::keyPressEvent( QKeyEvent* event )
   {
      // TODO-UPGRADE
      //mDeltaWin->GetInputCallback()->keyPress(GetKey(*event));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::keyReleaseEvent( QKeyEvent* event )
   {
      // TODO-UPGRADE
      //mDeltaWin->GetInputCallback()->keyRelease(GetKey(*event));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::mousePressEvent( QMouseEvent* event )
   {
      unsigned int button = 0;

      float x, y;
      ConvertMouseXYFromWindowSpace(event->x(), event->y(), x, y);

      switch(event->button())
      {
         case(Qt::LeftButton): 
            button = 1; 
         break;
         case(Qt::MidButton): 
            button = 2; 
         break;
         case(Qt::RightButton): 
            button = 3; 
         break;
         case(Qt::NoButton): 
            button = 0; 
         break;
         default: 
            button = 0; 
            break;
      }
      // TODO-UPGRADE
      //mDeltaWin->GetInputCallback()->buttonPress(x, y, button);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::mouseReleaseEvent( QMouseEvent* event )
   {
      unsigned int button = 0;
      float x, y;
      ConvertMouseXYFromWindowSpace(event->x(), event->y(), x, y);
      switch(event->button())
      {
         case(Qt::LeftButton): 
            button = 1; 
         break;
         case(Qt::MidButton): 
            button = 2; 
         break;
         case(Qt::RightButton): 
            button = 3; 
         break;
         case(Qt::NoButton): 
            button = 0; 
         break;
         default: 
            button = 0; 
            break;
      }
      // TODO-UPGRADE
      //mDeltaWin->GetInputCallback()->buttonRelease(x, y, button);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::mouseMoveEvent( QMouseEvent* event )
   {
      float x, y;
      ConvertMouseXYFromWindowSpace(event->x(), event->y(), x, y);
      // TODO-UPGRADE
      //mDeltaWin->GetInputCallback()->mouseMotion(x, y);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::ConvertMouseXYFromWindowSpace(int x, int y, float& fx, float& fy) const
   {
      float halfWidth = float(width()) / 2.0f;
      fx = (float(x) - halfWidth) / halfWidth;

      float halfHeight = float(height()) / 2.0f;
      fy = (float(-y) + halfHeight) / halfHeight;
   }

}

