/**
 * David Guthrie
 */

#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

#include <StealthViewer/Qt/GLWidgetRenderSurface.h>

#include <dtCore/deltawin.h>
#include <dtCore/system.h>
#include <dtCore/inputcallback.h>
#include <dtCore/camera.h>

#include <Producer/RenderSurface>

using namespace Producer;

namespace StealthQt
{
   
   struct KeyMapTable
   {
      int mKey;
      Producer::KeyCharacter mKeyChar;
      Producer::KeyCharacter mShiftedKeyChar;
      Producer::KeyCharacter mNumLockedKeyChar;
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

            //          KeyboardKey       Normal KeyChar       Shifted KeyChar     Numlocked KeyChar
            //          ----------        --------------       ---------------     -----------------
            {          Qt::Key_unknown,      KeyChar_Unknown,      KeyChar_Unknown,      KeyChar_Unknown },
            {          Qt::Key_Escape,       KeyChar_Escape,       KeyChar_Escape,       KeyChar_Escape },
            {          Qt::Key_F1,           KeyChar_F1,           KeyChar_F1,           KeyChar_F1 },
            {          Qt::Key_F2,           KeyChar_F2,           KeyChar_F2,           KeyChar_F2 },
            {          Qt::Key_F3,           KeyChar_F3,           KeyChar_F3,           KeyChar_F3 },
            {          Qt::Key_F4,           KeyChar_F4,           KeyChar_F4,           KeyChar_F4 },
            {          Qt::Key_F5,           KeyChar_F5,           KeyChar_F5,           KeyChar_F5 },
            {          Qt::Key_F6,           KeyChar_F6,           KeyChar_F6,           KeyChar_F6 },
            {          Qt::Key_F7,           KeyChar_F7,           KeyChar_F7,           KeyChar_F7 },
            {          Qt::Key_F8,           KeyChar_F8,           KeyChar_F8,           KeyChar_F8 },
            {          Qt::Key_F9,           KeyChar_F9,           KeyChar_F9,           KeyChar_F9 },
            {          Qt::Key_F10,          KeyChar_F10,          KeyChar_F10,          KeyChar_F10 },
            {          Qt::Key_F11,          KeyChar_F11,          KeyChar_F11,          KeyChar_F11 },
            {          Qt::Key_F12,          KeyChar_F12,          KeyChar_F12,          KeyChar_F12 },
            {          Qt::Key_QuoteLeft,    KeyChar_quoteleft,   KeyChar_asciitilde,    KeyChar_quoteleft },
            {          Qt::Key_1,            KeyChar_1,       KeyChar_exclam,            KeyChar_1 },
            {          Qt::Key_2,            KeyChar_2,           KeyChar_at,            KeyChar_2 },
            {          Qt::Key_3,            KeyChar_3,   KeyChar_numbersign,            KeyChar_3 },
            {          Qt::Key_4,            KeyChar_4,       KeyChar_dollar,            KeyChar_4 },
            {          Qt::Key_5,            KeyChar_5,      KeyChar_percent,            KeyChar_5 },
            {          Qt::Key_6,            KeyChar_6,  KeyChar_asciicircum,            KeyChar_6 },
            {          Qt::Key_7,            KeyChar_7,    KeyChar_ampersand,            KeyChar_7 },
            {          Qt::Key_8,            KeyChar_8,     KeyChar_asterisk,            KeyChar_8 },
            {          Qt::Key_9,            KeyChar_9,    KeyChar_parenleft,            KeyChar_9 },
            {          Qt::Key_0,            KeyChar_0,   KeyChar_parenright,            KeyChar_0 },
            {          Qt::Key_Minus,        KeyChar_minus,   KeyChar_underscore,        KeyChar_minus },
            {          Qt::Key_Equal,        KeyChar_equal,         KeyChar_plus,        KeyChar_equal },
            {          Qt::Key_Backspace,    KeyChar_BackSpace,    KeyChar_BackSpace,    KeyChar_BackSpace },
            {          Qt::Key_Tab,          KeyChar_Tab,          KeyChar_Tab,          KeyChar_Tab },
            {          Qt::Key_A,            KeyChar_a,            KeyChar_A,            KeyChar_a },
            {          Qt::Key_B,            KeyChar_b,            KeyChar_B,            KeyChar_b },
            {          Qt::Key_C,            KeyChar_c,            KeyChar_C,            KeyChar_c },
            {          Qt::Key_D,            KeyChar_d,            KeyChar_D,            KeyChar_d },
            {          Qt::Key_E,            KeyChar_e,            KeyChar_E,            KeyChar_e },
            {          Qt::Key_F,            KeyChar_f,            KeyChar_F,            KeyChar_f },
            {          Qt::Key_G,            KeyChar_g,            KeyChar_G,            KeyChar_g },
            {          Qt::Key_H,            KeyChar_h,            KeyChar_H,            KeyChar_h },
            {          Qt::Key_I,            KeyChar_i,            KeyChar_I,            KeyChar_i },
            {          Qt::Key_J,            KeyChar_j,            KeyChar_J,            KeyChar_j },
            {          Qt::Key_K,            KeyChar_k,            KeyChar_K,            KeyChar_k },
            {          Qt::Key_L,            KeyChar_l,            KeyChar_L,            KeyChar_l },
            {          Qt::Key_M,            KeyChar_m,            KeyChar_M,            KeyChar_m },
            {          Qt::Key_N,            KeyChar_n,            KeyChar_N,            KeyChar_n },
            {          Qt::Key_O,            KeyChar_o,            KeyChar_O,            KeyChar_o },
            {          Qt::Key_P,            KeyChar_p,            KeyChar_P,            KeyChar_p },
            {          Qt::Key_Q,            KeyChar_q,            KeyChar_Q,            KeyChar_q },
            {          Qt::Key_R,            KeyChar_r,            KeyChar_R,            KeyChar_r },
            {          Qt::Key_S,            KeyChar_s,            KeyChar_S,            KeyChar_s },
            {          Qt::Key_T,            KeyChar_t,            KeyChar_T,            KeyChar_t },
            {          Qt::Key_U,            KeyChar_u,            KeyChar_U,            KeyChar_u },
            {          Qt::Key_V,            KeyChar_v,            KeyChar_V,            KeyChar_v },
            {          Qt::Key_W,            KeyChar_w,            KeyChar_W,            KeyChar_w },
            {          Qt::Key_X,            KeyChar_x,            KeyChar_X,            KeyChar_x },
            {          Qt::Key_Y,            KeyChar_y,            KeyChar_Y,            KeyChar_y },
            {          Qt::Key_Z,            KeyChar_z,            KeyChar_Z,            KeyChar_z },
            {Qt::Key_BracketLeft,  KeyChar_bracketleft,    KeyChar_braceleft,  KeyChar_bracketleft },
            {     Qt::Key_BracketRight, KeyChar_bracketright,   KeyChar_braceright, KeyChar_bracketright },
            {  Qt::Key_Backslash,    KeyChar_backslash,          KeyChar_bar,    KeyChar_backslash },
            {  Qt::Key_CapsLock,    KeyChar_Caps_Lock,    KeyChar_Caps_Lock,    KeyChar_Caps_Lock },
            {  Qt::Key_Semicolon,    KeyChar_semicolon,        KeyChar_colon,    KeyChar_semicolon },
            { Qt::Key_Apostrophe,   KeyChar_apostrophe,     KeyChar_quotedbl,   KeyChar_apostrophe },
            {     Qt::Key_Return,       KeyChar_Return,       KeyChar_Return,       KeyChar_Return },
            {    Qt::Key_Shift,      KeyChar_Shift_L,      KeyChar_Shift_L,      KeyChar_Shift_L },
            {      Qt::Key_Comma,        KeyChar_comma,         KeyChar_less,        KeyChar_comma },
            {     Qt::Key_Period,       KeyChar_period,      KeyChar_greater,       KeyChar_period },
            {      Qt::Key_Slash,        KeyChar_slash,     KeyChar_question,        KeyChar_slash },
            {    Qt::Key_Shift,      KeyChar_Shift_R,      KeyChar_Shift_R,      KeyChar_Shift_R },
            {  Qt::Key_Control,    KeyChar_Control_L,    KeyChar_Control_L,    KeyChar_Control_L },
            {    Qt::Key_Super_L,      KeyChar_Super_L,      KeyChar_Super_L,      KeyChar_Super_L },
            {      Qt::Key_Space,        KeyChar_space,        KeyChar_space,        KeyChar_space },
            {      Qt::Key_Alt,        KeyChar_Alt_L,        KeyChar_Alt_L,        KeyChar_Alt_L },
            {      Qt::Key_Alt,        KeyChar_Alt_R,        KeyChar_Alt_R,        KeyChar_Alt_R },
            {    Qt::Key_Super_R,      KeyChar_Super_R,      KeyChar_Super_R,      KeyChar_Super_R },
            {       Qt::Key_Menu,         KeyChar_Menu,         KeyChar_Menu,         KeyChar_Menu },
            {  Qt::Key_Control,    KeyChar_Control_R,    KeyChar_Control_R,    KeyChar_Control_R },
            {      Qt::Key_Print,      KeyChar_Sys_Req,      KeyChar_Sys_Req,      KeyChar_Sys_Req },
            {Qt::Key_ScrollLock,  KeyChar_Scroll_Lock,  KeyChar_Scroll_Lock,  KeyChar_Scroll_Lock },
            {      Qt::Key_Pause,        KeyChar_Break,        KeyChar_Break,        KeyChar_Break },
            {       Qt::Key_Home,         KeyChar_Home,         KeyChar_Home,         KeyChar_Home },
            {    Qt::Key_PageUp,      KeyChar_Page_Up,      KeyChar_Page_Up,      KeyChar_Page_Up },
            {        Qt::Key_End,          KeyChar_End,          KeyChar_End,          KeyChar_End },
            {  Qt::Key_PageDown,    KeyChar_Page_Down,    KeyChar_Page_Down,    KeyChar_Page_Down },
            {     Qt::Key_Delete,       KeyChar_Delete,       KeyChar_Delete,       KeyChar_Delete },
            {     Qt::Key_Insert,       KeyChar_Insert,       KeyChar_Insert,       KeyChar_Insert },
            {       Qt::Key_Left,         KeyChar_Left,         KeyChar_Left,         KeyChar_Left },
            {         Qt::Key_Up,           KeyChar_Up,           KeyChar_Up,           KeyChar_Up },
            {      Qt::Key_Right,        KeyChar_Right,        KeyChar_Right,        KeyChar_Right },
            {       Qt::Key_Down,         KeyChar_Down,         KeyChar_Down,         KeyChar_Down },
            {   Qt::Key_NumLock,     KeyChar_Num_Lock,     KeyChar_Num_Lock,     KeyChar_Num_Lock }
            /*{  Qt::Key_division,    KeyChar_KP_Divide,    KeyChar_KP_Divide,    KeyChar_KP_Divide },
     {Qt::Key_multiply,  KeyChar_KP_Multiply,  KeyChar_KP_Multiply,  KeyChar_KP_Multiply },
     {Qt::Key_minus,  KeyChar_KP_Subtract,  KeyChar_KP_Subtract,  KeyChar_KP_Subtract },
     {     Qt::Key_KP_Add,       KeyChar_KP_Add,       KeyChar_KP_Add,       KeyChar_KP_Add },
     {    Qt::Key_Home,      KeyChar_KP_Home,      KeyChar_KP_Home,            KeyChar_7 },
     {      Qt::Key_Up,        KeyChar_KP_Up,        KeyChar_KP_Up,            KeyChar_8 },
     { Qt::Key_Page_Up,   KeyChar_KP_Page_Up,   KeyChar_KP_Page_Up,            KeyChar_9 },
     {    Qt::Key_KP_Left,      KeyChar_KP_Left,      KeyChar_KP_Left,            KeyChar_4 },
     {   Qt::Key_KP_Begin,     KeyChar_KP_Begin,     KeyChar_KP_Begin,            KeyChar_5 },
     {   Qt::Key_KP_Right,     KeyChar_KP_Right,     KeyChar_KP_Right,            KeyChar_6 },
     {     Qt::Key_KP_End,       KeyChar_KP_End,       KeyChar_KP_End,            KeyChar_1 },
     {    Qt::Key_KP_Down,      KeyChar_KP_Down,      KeyChar_KP_Down,            KeyChar_2 },
     {     Qt::Key_KP_Page_Down, KeyChar_KP_Page_Down, KeyChar_KP_Page_Down,            KeyChar_3 },
     {  Qt::Key_KP_Insert,    KeyChar_KP_Insert,    KeyChar_KP_Insert,            KeyChar_0 },
     {  Qt::Key_KP_Delete,    KeyChar_KP_Delete,    KeyChar_KP_Delete,       KeyChar_period },
     {   Qt::Key_KP_Enter,     KeyChar_KP_Enter,     KeyChar_KP_Enter,     KeyChar_KP_Enter },
     {   Qt::Key_LAST_KEY,      KeyChar_Unknown,      KeyChar_Unknown,      KeyChar_Unknown },*/

      };

      mKeyMapping.clear();

      int n = sizeof(keyTable)/sizeof(KeyMapTable);
      for (int i = 0; i < n; ++i)
      {
         mKeyMapping.insert(std::make_pair(keyTable[i].mKey, keyTable[i]));
      }
   }    

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::resizeGL( int width, int height )
   {      
      mDeltaWin->GetRenderSurface()->setWindowRectangle(0, 0, width, height, false);
      mCamera->GetSceneHandler()->GetSceneView()->setViewport(0, 0, width, height);
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

      mTimer.start();
   }

   ///////////////////////////////////////////////////////////////////////////////
   Producer::KeyCharacter GLWidgetRenderSurface::GetProducerKey(QKeyEvent& event) const
   {
      std::map<int, KeyMapTable>::const_iterator i = mKeyMapping.find(event.key());
      
      if (i != mKeyMapping.end())
      {  
         if ((event.modifiers() | Qt::ShiftModifier) != 0)
            return i->second.mShiftedKeyChar;
         else
            return i->second.mKeyChar;
      }
      return Producer::KeyChar_Unknown;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::keyPressEvent( QKeyEvent* event )
   {
      mDeltaWin->GetInputCallback()->keyPress(GetProducerKey(*event));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::keyReleaseEvent( QKeyEvent* event )
   {
      mDeltaWin->GetInputCallback()->keyRelease(GetProducerKey(*event));
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
      mDeltaWin->GetInputCallback()->buttonPress(x, y, button);
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
      mDeltaWin->GetInputCallback()->buttonRelease(x, y, button);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GLWidgetRenderSurface::mouseMoveEvent( QMouseEvent* event )
   {
      float x, y;
      ConvertMouseXYFromWindowSpace(event->x(), event->y(), x, y);
      mDeltaWin->GetInputCallback()->mouseMotion(x, y);
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

