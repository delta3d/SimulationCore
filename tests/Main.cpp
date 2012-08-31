/* -*-c++-*-
* Simulation Core - allTests\Main.cpp (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2005-2008, Alion Science and Technology Corporation
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
* @author Eddie Johnson
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestRunner.h>

#include <cppunit/TestListener.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <ctime>    // for clock()

#include <dtAudio/audiomanager.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>
#include <dtUtil/datapathutils.h>

#include <dtCore/timer.h>
#include <dtCore/deltawin.h>
#include <dtCore/shadermanager.h>
#include <dtCore/system.h>

#include <dtDAL/project.h>
#include <dtDAL/map.h>
#include <dtDAL/librarymanager.h>

#include <dtABC/application.h>

#include <CEGUI/CEGUIVersion.h>

#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
#include <dtGUI/ceuidrawable.h>
#else
#include <dtGUI/gui.h>
#endif
#include <dtGUI/scriptmodule.h>

#include <SimCore/BaseGameEntryPoint.h>

#include <sstream>
#include <cmath>
#include <stdexcept>

#ifdef None
#undef None
#endif
#include <CEGUI.h>

static std::ostringstream mSlowTests;

static dtCore::RefPtr<dtABC::Application> globalApplication;
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
static dtCore::RefPtr<dtGUI::CEUIDrawable> globalGUI;
#else
static dtCore::RefPtr<dtGUI::GUI> globalGUI;
#endif

dtABC::Application& GetGlobalApplication() { return *globalApplication; }
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
dtGUI::CEUIDrawable& GetGlobalCEGUIDrawable() { return *globalGUI; }
#else
dtGUI::GUI& GetGlobalGUI() { return *globalGUI; }
#endif

class TimingListener : public CppUnit::TestListener
{
  public:
     void startTest( CppUnit::Test *test )
     {
        mFailure = false;
        mTestClockStart = mTestClock.Tick();
     }

      void endTest( CppUnit::Test *test )
      {
         // handle timing - for checking slow tests
         std::ostringstream testResult;
         dtCore::Timer_t testClockStop = mTestClock.Tick();
         double timeDelta = mTestClock.DeltaSec(mTestClockStart, testClockStop);
         timeDelta = (floor(timeDelta * 10000.0)) / 10000.0; // force data truncation
         testResult << test->getName()  << ((!mFailure) ? ": OK " : ": FAILURE ") <<
            ": time [" << timeDelta << "]";
         if (timeDelta > 0.7)
         {
            testResult << " *** SLOW TEST ***";
            mSlowTests << testResult.str() << std::endl;
         }
         std::cerr << testResult.str() << std::endl;
      }

      void addFailure( const CppUnit::TestFailure &failure )
      {
         mFailure = true;
      }

    private:
       dtCore::Timer mTestClock;
       dtCore::Timer_t mTestClockStart;
       bool mFailure;
};

void SetupCEGUI(dtABC::Application& app)
{
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
   const std::string guiScheme = "CEGUI/schemes/WindowsLook.scheme";
   globalGUI = new dtGUI::CEUIDrawable(app.GetWindow(),
            app.GetKeyboard(), app.GetMouse(), new dtGUI::ScriptModule());

   std::string path = dtUtil::FindFileInPathList(guiScheme);
   if (path.empty())
   {
      throw dtUtil::Exception("Failed to find the scheme file.",
         __FILE__, __LINE__);
   }

   std::string dir = path.substr(0, path.length() - (guiScheme.length() - 5));
   //dtUtil::FileUtils::GetInstance().PushDirectory(dir);
   try
   {
      CEGUI::SchemeManager::getSingleton().loadScheme(path);
#else
   globalGUI = new dtGUI::GUI(app.GetCamera(),
            app.GetKeyboard(), app.GetMouse());
   globalGUI->SetScriptModule(new dtGUI::ScriptModule());
//   std::string ceguiDir(dtDAL::Project::GetInstance().GetContext(0));
//   globalGUI->SetResourceGroupDirectory("schemes", ceguiDir);
//   globalGUI->SetResourceGroupDirectory("imagesets", ceguiDir);
//   globalGUI->SetResourceGroupDirectory("looknfeel", ceguiDir);
//   globalGUI->SetResourceGroupDirectory("layouts", ceguiDir);
//   globalGUI->SetResourceGroupDirectory("fonts", ceguiDir);
   try
   {
      //std::cout << "CEGUI in: " << ceguiDir << "\n\n";
      globalGUI->LoadScheme("CEGUI/schemes/WindowsLook.scheme");
#endif
   }
   catch (const CEGUI::Exception& ex)
   {
      //make sure the directory gets popped if it fails.
      //dtUtil::FileUtils::GetInstance().PopDirectory();
      std::ostringstream ss;
      ss << ex.getMessage();
      LOG_ERROR(ss.str());
      throw;
   }
   //dtUtil::FileUtils::GetInstance().PopDirectory();
}

#ifndef TEST_ROOT
#define TEST_ROOT ../
#endif

#define _GET_PATH(testpath) #testpath
#define GET_PATH(testpath) _GET_PATH(testpath)

int main (int argc, char* argv[])
{
   const std::string executable = argv[0];
   bool changeDir = true;
   std::string singleSuiteName;
   std::string singleTestName;

   std::string currArg;
   for (int arg = 1; arg < argc; ++arg)
   {
      currArg = argv[arg];
      if (currArg == "-nochdir")
      {
         changeDir = false;
      }
      else if (singleSuiteName.empty())
      {
         singleSuiteName = currArg;
      }
      else if (singleSuiteName.empty())
      {
         singleTestName = currArg;
      }
      else
      {
         std::cerr << "Ignoring argument: " << currArg << std::endl;
      }
   }

   if (changeDir)
   {
      std::string path = GET_PATH(TEST_ROOT);
      LOG_ALWAYS("The test root is: " + path);
      LOG_ALWAYS(std::string("Changing to directory \"") + path + dtUtil::FileUtils::PATH_SEPARATOR + ".");

      try
      {
         dtUtil::FileUtils::GetInstance().ChangeDirectory(path);
      }
      catch(const dtUtil::Exception& ex)
      {
         ex.LogException(dtUtil::Log::LOG_ERROR);
      }
   }

   dtAudio::AudioManager::Instantiate();

   globalApplication = new dtABC::Application("tests/config.xml");
   globalApplication->GetWindow()->SetPosition(0, 0, 50, 50);
   globalApplication->Config();

   try
   {
      //Force this to false because many of the tests expect it to be false.
      dtCore::System::GetInstance().SetUseFixedTimeStep(false);
      dtUtil::SetDataFilePathList(dtUtil::GetDeltaDataPathList());
      dtDAL::Project::GetInstance().SetContext("demos/ProjectAssets_Demos");
      dtDAL::Project::GetInstance().AddContext("ProjectAssets_Shared");
      dtDAL::LibraryManager::GetInstance().LoadActorRegistry(SimCore::BaseGameEntryPoint::LIBRARY_NAME);
      SetupCEGUI(*globalApplication);
   }
   catch(const dtUtil::Exception& ex)
   {
      LOG_ERROR(ex.ToString());
      globalApplication = NULL;
      return 1;
   }
   catch(const CEGUI::Exception& e)
   {
      //already printed.
      globalApplication = NULL;
      return 1;
   }

   CPPUNIT_NS::TestResultCollector collectedResults;

   try
   {
      dtCore::Timer testsClock;
      dtCore::Timer_t testsTimerStart = testsClock.Tick();

      CPPUNIT_NS::TestResult testResult;
      testResult.addListener(&collectedResults);
      TimingListener timelistener;
      testResult.addListener(&timelistener);

      // setup the test runner - does all the work
      CPPUNIT_NS::TestRunner testRunner;
      CPPUNIT_NS::Test *fullTestSuite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

      // Check for a specific test suite - if passed in, we only run the one suite
      if (singleSuiteName != "")
      {
         LOG_ALWAYS(std::string("Single suite name detected in arguments. Attempting to load a single test named [") + singleSuiteName + std::string("]."));
         CPPUNIT_NS::Test *suiteTest = fullTestSuite->findTest(singleSuiteName);
         if (suiteTest == NULL)
         {
            std::cerr << " *** FAILED to find test suite named [" << singleSuiteName <<
               "]. Please check suite name. The name should match what was used in the registration line, " <<
               "\'CPPUNIT_TEST_SUITE(MyTests)\' would be \'MyTests\'. Aborting test." <<  std::endl;
         }
         // Check for a specific test within the specific suite
         else if (singleTestName != "")
         {
            LOG_ALWAYS(std::string("Individual test name detected in arguments. Attempting to load test [") + singleTestName + std::string("]."));
            CPPUNIT_NS::Test *individualTest = suiteTest->findTest(singleTestName);
            if (individualTest == NULL)
            {
               std::cerr << " *** FAILED to individual test [" << singleTestName <<
                  "] inside suite [" << singleSuiteName << "]. Please check suite name. " <<
                  "The name should match what was used in the registration line, " <<
                  "\'CPPUNIT_TEST(TestFunction)\' would be \'TestFunction\'. Aborting test." <<  std::endl;
            }
            else
            {
               LOG_ALWAYS(std::string("   *** Found test suite and single test[ ") +
                  singleTestName + std::string("].  Starting run."));
               testRunner.addTest(individualTest);
            }
         }
         // We found the single suite, no individual test
         else
         {
            LOG_ALWAYS(std::string("Found single test suite. Starting run."));
            testRunner.addTest(suiteTest);
         }
      }
      else
      {
         LOG_ALWAYS(std::string("No arguments detected.  Running all tests!  Pass the suite name as 1st arg to run a single suite. For single test, pass test name as 2nd arg."));
         testRunner.addTest(fullTestSuite);
      }

      // Go to it!!!  Run this puppy!
      testRunner.run(testResult);

      CPPUNIT_NS::CompilerOutputter compilerOutputter(&collectedResults,std::cerr);
      compilerOutputter.write();

      // print out slow tests and total time.
      dtCore::Timer_t testsTimerStop = testsClock.Tick();
      double timeDelta = testsClock.DeltaSec(testsTimerStart, testsTimerStop);
      timeDelta = (floor(timeDelta * 10000.0)) / 10000.0; // force data truncation
      if(!mSlowTests.str().empty())
      {
         std::cerr << " <<< SLOW TEST RESULTS ::: START >>> " << std::endl <<
            mSlowTests.str() << " <<< SLOW TEST RESULTS ::: END ::: TotalTime[" <<
            timeDelta << "] >>> " << std::endl;
      }
      else
      {
         std::cerr << " <<< Total Run Time " << timeDelta << " >>> " << std::endl;
      }
   }
   catch (std::invalid_argument &ie)
   {
      std::cerr << " <<< Invalid argument occurred. Likely, the suite name or test name are invalid or not found. " <<
         " For tests, be sure to include the class name like [MyClass::TestStuff]. Or, see cppunit.sourceforge.net for more info.  Error: [" <<
         ie.what() << "]. >>> " << std::endl;
   }
   catch (std::exception &e)
   {
      std::cerr << " <<< Exception occurred. Error: [" << e.what() << "]. >>> " << std::endl;
   }
   catch (...)
   {
      std::cerr << " <<< Exception occurred while running main.cpp for this unit test. No other info available >>> " << std::endl;
   }

   dtCore::ShaderManager::GetInstance().Clear();

   osgDB::Registry::instance()->clearObjectCache();

   globalApplication = NULL;
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
   globalGUI->ShutdownGUI();
#endif
   globalGUI = NULL;

   dtDAL::LibraryManager::GetInstance().UnloadActorRegistry(SimCore::BaseGameEntryPoint::LIBRARY_NAME);
   dtAudio::AudioManager::Destroy();

   return collectedResults.wasSuccessful () ? 0 : 1;
}

