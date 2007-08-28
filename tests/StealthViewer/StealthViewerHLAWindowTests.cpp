/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <cppunit/extensions/HelperMacros.h>
#include <StealthQt/HLAWindow.h>
#include <StealthQt/ui_HLAWindowUi.h>
#include <StealthQt/StealthViewerSettings.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <dtGame/gamemanager.h>
#include <dtABC/application.h>
#include <QtGui/QApplication>

class SubHLAWindow : public StealthQt::HLAWindow
{
   public:

      SubHLAWindow(dtGame::GameManager &gm, 
                   QWidget *parent = NULL, 
                   StealthQt::StealthViewerSettings *settings = NULL, 
                   bool isConnected = false) : 
            StealthQt::HLAWindow(gm, parent, settings, isConnected)
      {

      }

      virtual ~SubHLAWindow()
      {

      }

      Ui::HLAWindow& GetUI() { return *mUi; }
};

class StealthViewerHLAWindowTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(StealthViewerHLAWindowTests);

      CPPUNIT_TEST(TestDisplaySavedConnections);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp();

      void tearDown();

      void TestDisplaySavedConnections();

   private:

      QApplication *mQApp;
      dtCore::RefPtr<dtGame::GameManager> mGM;
      dtCore::RefPtr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthViewerHLAWindowTests);

void StealthViewerHLAWindowTests::setUp()
{
   std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory();

#if defined (_MSC_VER) && defined (_DEBUG)
   dir += "bin/StealthViewerHLAWindowTestsd.exe";
#elif defined (_MSC_VER)
   dir += "bin/StealthViewerHLAWindowTests.exe";
#else
   dir += "bin/StealthViewerHLAWindowTests";
#endif

   int numParams = 1;
   const char *exe = dir.c_str();

   mQApp = new QApplication(numParams, const_cast<char**>(&exe));

   mApp = new dtABC::Application;

   mGM = new dtGame::GameManager(*mApp->GetScene());
}

void StealthViewerHLAWindowTests::tearDown()
{
   delete mQApp;
   mQApp = NULL;

   mGM = NULL;
   mApp = NULL;
}

void StealthViewerHLAWindowTests::TestDisplaySavedConnections()
{
   StealthQt::StealthViewerSettings settings("UnitTest");
   settings.ClearAllSettings(true);

   SubHLAWindow *window = new SubHLAWindow(*mGM, NULL, &settings);
   
   QListWidgetItem *shouldBeNULL = window->GetUI().mNetworkListWidget->item(0);

   CPPUNIT_ASSERT_MESSAGE("Initially, the widget should be empty", 
       shouldBeNULL == NULL);

   delete window;
   window = NULL;

   // Clear just in case another unit test already created this file
   settings.ClearAllSettings(true);
   CPPUNIT_ASSERT(settings.GetNumConnections() == 0);

   QStringList testProps;
   testProps.push_back(QString("TestName"));
   testProps.push_back(QString("TestMap"));
   testProps.push_back(QString("TestConfig"));
   testProps.push_back(QString("TestFed"));
   testProps.push_back(QString("TestFedex"));
   testProps.push_back(QString("TestFedName"));
   testProps.push_back(QString("TestRidFile"));

   settings.AddConnection(testProps[0], testProps[1], testProps[2], 
      testProps[3], testProps[4], testProps[5], testProps[6]);

   CPPUNIT_ASSERT_EQUAL((unsigned int)(1), settings.GetNumConnections());

   window = new SubHLAWindow(*mGM, NULL, &settings);

   int count = window->GetUI().mNetworkListWidget->count();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The number of items in the widget should equal the number\
                                 of saved connections.", 1, count);

   QListWidgetItem *item = window->GetUI().mNetworkListWidget->item(0);
   CPPUNIT_ASSERT(item != NULL);

   CPPUNIT_ASSERT_MESSAGE("Now that a settings file has been created with data\
                          written to it, the list widget should contain the name\
                          of the object written.", 
                          item->text() == QString("TestName"));

   settings.ClearAllSettings(true);

   delete window;
   window = NULL;

   window = new SubHLAWindow(*mGM, NULL, &settings);

   count = window->GetUI().mNetworkListWidget->count();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The number of items in the widget should default to 0", 
                                 0, count);

   item = window->GetUI().mNetworkListWidget->item(0);
   CPPUNIT_ASSERT(item == NULL);
}

