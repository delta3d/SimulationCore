/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <cppunit/extensions/HelperMacros.h>
#include <StealthViewer/Qt/HLAWindow.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <dtDAL/map.h>
#include <dtGame/gamemanager.h>
#include <dtABC/application.h>
#include <QtGui/QApplication>

#include <UnitTestMain.h>

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

   mApp = &GetGlobalApplication();

   mGM = new dtGame::GameManager(*mApp->GetScene());
   mGM->SetApplication( *mApp );
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

   StealthQt::HLAWindow *window = new StealthQt::HLAWindow(*mGM, NULL, &settings);
   
   QListWidgetItem *shouldBeNULL = window->GetNetworkListWidget()->item(0);

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

   window = new StealthQt::HLAWindow(*mGM, NULL, &settings);

   int count = window->GetNetworkListWidget()->count();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The number of items in the widget should equal the number\
                                 of saved connections.", 1, count);

   QListWidgetItem *item = window->GetNetworkListWidget()->item(0);
   CPPUNIT_ASSERT(item != NULL);

   CPPUNIT_ASSERT_MESSAGE("Now that a settings file has been created with data\
                          written to it, the list widget should contain the name\
                          of the object written.", 
                          item->text() == QString("TestName"));

   settings.ClearAllSettings(true);

   delete window;
   window = NULL;

   window = new StealthQt::HLAWindow(*mGM, NULL, &settings);

   count = window->GetNetworkListWidget()->count();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The number of items in the widget should default to 0", 
                                 0, count);

   item = window->GetNetworkListWidget()->item(0);
   CPPUNIT_ASSERT(item == NULL);
}

