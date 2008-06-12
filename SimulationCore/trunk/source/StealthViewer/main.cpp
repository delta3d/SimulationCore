/* -*-c++-*-
* Stealth Viewer
* Copyright 2007-2008, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*/
#include <prefix/SimCorePrefix-src.h>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QSplashScreen>
#include <QtGui/QPixmap>

#include <StealthViewer/Qt/MainWindow.h>

#include <dtUtil/log.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/macros.h>
#include <dtCore/globals.h>
#include <dtCore/system.h>
#include <dtAudio/audiomanager.h>
#include <dtDAL/project.h>
#include <dtDAL/librarymanager.h>

#include <sstream>

const int appArgc = 9;
static char* appArgv[appArgc] = 
{
   "GameStart",
   "--UI", "1",
   "--statisticsInterval", "30",
   "--enableLogging", "1",
   "--enablePlayback", "1",
};

int main(int argc, char *argv[])
{
   // Log to log file, and not to console
   dtUtil::Log::GetInstance().SetOutputStreamBit(dtUtil::Log::TO_FILE);

#ifdef DELTA_WIN32

   const std::string &executable = argv[0];
   dtUtil::FileInfo info = dtUtil::FileUtils::GetInstance().GetFileInfo(executable);
   if(info.fileType == dtUtil::FILE_NOT_FOUND)
   {
      LOG_ERROR(std::string("Unable to change to the directory of application \"")
         + executable + "\": file not found.");
   }
   else
   {
      std::string path = info.path;
      LOG_INFO("The path to the executable is: " + path);
      LOG_INFO("Changing to directory \"" + path + dtUtil::FileUtils::PATH_SEPARATOR + "..\".");
      try 
      {
         if(!info.path.empty())
            dtUtil::FileUtils::GetInstance().ChangeDirectory(path);

         dtUtil::FileUtils::GetInstance().ChangeDirectory("..");
      } 
      catch(const dtUtil::Exception &ex)
      {
         ex.LogException(dtUtil::Log::LOG_ERROR);
      }
   }

#endif

   dtAudio::AudioManager::Instantiate();
   dtAudio::AudioManager::GetInstance().Config(AudioConfigData(32));

   //dtCore::SetDataFilePathList(".:" + dtCore::GetDeltaDataPathList());
   int result;
   QApplication app(argc, argv);

   try
   {
      //dtUtil::Log::GetInstance().SetLogLevel(dtUtil::Log::LOG_INFO);

      //Now that everything is initialized, show the main window.
      //Construct the application... 
      std::string appLibName("StealthGMApp");
      StealthQt::MainWindow mainWindow(appArgc, appArgv, appLibName);

      dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
      dtCore::System::GetInstance().Start();
      dtCore::System::GetInstance().Config();
      result = app.exec();
      dtCore::System::GetInstance().Stop();
   }
   catch(const dtUtil::Exception &e)
   {
      e.LogException(dtUtil::Log::LOG_ERROR);
      std::ostringstream ss;
      ss << "Exception (" << e.TypeEnum() << "): " << e.What()
         << "\n\tLine: " << e.Line() << " File: " << e.File(); 

      QMessageBox::critical(NULL,"Exception",ss.str().c_str(),
                           QMessageBox::Ok,QMessageBox::NoButton);

      dtAudio::AudioManager::Destroy();

      return 1;
   }
   catch(const std::exception &e)
   {
      QString message("A standard exception has been thrown. The exception message is: ");
      message += e.what();

      QMessageBox::critical(NULL, "Exception", message, QMessageBox::Ok);

      dtAudio::AudioManager::Destroy();

      return 1;
   }
   catch(...)
   {
      QString message("An unknown exception has been thrown. Please try restarting the application."); 

      QMessageBox::critical(NULL, "Exception", message, QMessageBox::Ok);

      dtAudio::AudioManager::Destroy();

      return 1;
   }

    dtAudio::AudioManager::Destroy();

    return result;
}

