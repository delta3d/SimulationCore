/* -*-c++-*-
* Stealth Viewer - main (.cpp) - Using 'The MIT License'
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
*/
#include <prefix/SimCorePrefix.h>
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

int main(int argc, char* argv[])
{
   // Log to log file, and not to console
   //dtUtil::Log::GetInstance().SetOutputStreamBit(dtUtil::Log::TO_FILE);
   // If you turn off console logging, most users will miss important messages - example, they crash
   // on startup and will have NO understanding of why they have crashed. We get posts all the time about
   // how users crashed Stealth Viewer with no idea what happened! At the very most, you
   // should set the log level to ERROR only like this: 
   //dtUtil::Log::GetInstance().SetLogLevel(dtUtil::Log::LOG_ERROR);

   dtAudio::AudioManager::Instantiate();

   //dtCore::SetDataFilePathList(".:" + dtCore::GetDeltaDataPathList());
   int result;
   QApplication app(argc, argv);

   try
   {
      //dtUtil::Log::GetInstance().SetLogLevel(dtUtil::Log::LOG_INFO);

      //Now that everything is initialized, show the main window.
      //Construct the application...
      const std::string appLibName("StealthGMApp");
      StealthQt::MainWindow mainWindow(appArgc, appArgv, appLibName);

      dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
      dtCore::System::GetInstance().Start();
      dtCore::System::GetInstance().Config();
      result = app.exec();
      dtCore::System::GetInstance().Stop();
   }
   catch(const dtUtil::Exception& ex)
   {
      QMessageBox::critical(NULL,"Exception", ex.ToString().c_str(),
                           QMessageBox::Ok,QMessageBox::NoButton);

      dtAudio::AudioManager::Destroy();

      return 1;
   }
   catch(const std::exception& e)
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
      throw;
   }

    dtAudio::AudioManager::Destroy();

    return result;
}

