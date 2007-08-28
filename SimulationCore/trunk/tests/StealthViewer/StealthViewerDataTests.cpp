/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <cppunit/extensions/HelperMacros.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <QtGui/QApplication>

class StealthViewerDataTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(StealthViewerDataTests);


   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp();

      void tearDown();

      QApplication *mQApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthViewerDataTests);

void StealthViewerDataTests::setUp()
{
   std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory();

#if defined (_MSC_VER) && defined (_DEBUG)
   dir += "bin/StealthViewerDataTestsd.exe";
#elif defined (_MSC_VER)
   dir += "bin/StealthViewerDataTests.exe";
#else
   dir += "bin/StealthViewerDataTests";
#endif

   int numParams = 1;
   const char *exe = dir.c_str();

   mQApp = new QApplication(numParams, const_cast<char**>(&exe));
}

void StealthViewerDataTests::tearDown()
{
   delete mQApp;
   mQApp = NULL;
}
