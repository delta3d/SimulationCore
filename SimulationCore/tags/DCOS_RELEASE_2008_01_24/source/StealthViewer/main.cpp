/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
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

const int appArgc = 13;
static char* appArgv[appArgc] = 
{
   "GameStart",
   "--UI", "1",
   "--statisticsInterval", "30",
   "--enableLogging", "1",
   "--enablePlayback", "1",
   "--hasBinos", 
   "--hasCompass",  
   "--hasGPS",
   "--hasNightVis"
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

   dtCore::SetDataFilePathList(".:" +
                               dtCore::GetDeltaDataPathList() + ":" +
                               "source/StealthViewer") ;
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
