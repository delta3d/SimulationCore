#ifndef CEGUIUTILS_H_
#define CEGUIUTILS_H_

#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>
#include <dtGUI/dtgui.h>

namespace SimCore
{
   inline void SetupCEGUI()
   {
      // NOTE: code is borrowed from BaseHUD constructor

      std::string guiScheme = "CEGUI/schemes/WindowsLook.scheme";

      std::string path = dtCore::FindFileInPathList(guiScheme);
      if(path.empty())
      {
         throw dtUtil::Exception("Failed to find the scheme file.",
            __FILE__, __LINE__);
      }

      std::string dir = path.substr(0, path.length() - (guiScheme.length() - 5));
      dtUtil::FileUtils::GetInstance().PushDirectory(dir);
      try
      {
         CEGUI::SchemeManager::getSingleton().loadScheme(path);
      }
      catch(CEGUI::Exception &e)
      {
         std::ostringstream oss;
         oss << "CEGUI while setting up BaseHUD: " << e.getMessage().c_str();
         throw dtUtil::Exception(oss.str(), __FILE__, __LINE__);
      }
      dtUtil::FileUtils::GetInstance().PopDirectory();

      //CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
      //CEGUI::System::getSingleton().setDefaultFont("DejaVuSans-10");
   }
   
}


#endif /*CEGUIUTILS_H_*/
