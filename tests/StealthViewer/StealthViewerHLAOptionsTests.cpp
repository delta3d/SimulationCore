/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <cppunit/extensions/HelperMacros.h>
#include <StealthQt/HLAOptions.h>
#include <StealthQt/ui_HLAOptionsUi.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <dtCore/globals.h>
#include <QtGui/QApplication>

class SubHLAOptions : public StealthQt::HLAOptions
{
   public:

      SubHLAOptions(QWidget *parent = NULL, const QString &connection = "") :
         StealthQt::HLAOptions(parent, connection)
      {

      }

      virtual ~SubHLAOptions()
      {

      }

      QString ConvertFileName(const QString &file, const QString& projectDir)
      {
         return HLAOptions::ConvertFileName(file, projectDir);
      }

   private:
};

class StealthViewerHLAOptionsTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(StealthViewerHLAOptionsTests);

      CPPUNIT_TEST(TestHLAOptionsDefaults);
      CPPUNIT_TEST(TestConvertFileName);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp();

      void tearDown();

      void TestHLAOptionsDefaults();
      void TestConvertFileName();

   private:
      void TestConvertFileName(const QString& testFile, const QString& projectDir, const QString& expectedResult);

      QApplication *mQApp;

};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthViewerHLAOptionsTests);

void StealthViewerHLAOptionsTests::setUp()
{
   std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory();

#if defined (_MSC_VER) && defined (_DEBUG)
   dir += "bin/StealthViewerHLAOptionsTestsd.exe";
#elif defined (_MSC_VER)
   dir += "bin/StealthViewerHLAOptionsTests.exe";
#else
   dir += "bin/StealthViewerHLAOptionsTests";
#endif

   int numParams = 1;
   const char *exe = dir.c_str();
  
   mQApp = new QApplication(numParams, const_cast<char**>(&exe));
}

void StealthViewerHLAOptionsTests::tearDown()
{
   delete mQApp;
   mQApp = NULL;
}

void StealthViewerHLAOptionsTests::TestHLAOptionsDefaults()
{
   SubHLAOptions options;
   CPPUNIT_ASSERT(options.GetConnectionName().isEmpty());
}

void StealthViewerHLAOptionsTests::TestConvertFileName()
{
   TestConvertFileName("C:\\DVTEProject", "C:\\DVTEProject\\CEGUI\\cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("c:\\DVTEProject", "C:\\DVTEProject\\CEGUI\\cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("C:\\DVTEProject", "c:\\DVTEProject\\CEGUI\\cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("/development/test/DVTEProject", "/development/test/DVTEProject/CEGUI/cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("C:/junk", "/development/test/DVTEProject/CEGUI/cegui.config", "");
   TestConvertFileName("", "/development/test/DVTEProject/CEGUI/cegui.config", "");
   TestConvertFileName("/development/test/DVTEP", "/development/test/DVTEProject/CEGUI/cegui.config", "");
   TestConvertFileName("/development/test/cegui", "/development/test/cegui.config", "");
}

void StealthViewerHLAOptionsTests::TestConvertFileName(const QString& projectDir, const QString& testFile, const QString& expectedResult)
{
   SubHLAOptions options;

   CPPUNIT_ASSERT_MESSAGE("Case sensitive file paths should be false by default",
      !options.GetCaseSensitiveFilePaths());

   options.SetCaseSensitiveFilePaths(true);
   CPPUNIT_ASSERT(options.GetCaseSensitiveFilePaths());

   QString rdFormat = options.ConvertFileName(testFile, projectDir);
   CPPUNIT_ASSERT_MESSAGE("The format should be: \"" + expectedResult.toStdString() + "\" but it is \"" + rdFormat.toStdString() + "\"."
         "\nInput data is " + projectDir.toStdString() + " with " + testFile.toStdString(),  expectedResult == rdFormat);

   options.SetCaseSensitiveFilePaths(false);
   CPPUNIT_ASSERT(!options.GetCaseSensitiveFilePaths());

   rdFormat = options.ConvertFileName(testFile, projectDir);
   QString tempExpectedResult = expectedResult.toUpper();

   CPPUNIT_ASSERT_MESSAGE("The format should be: \"" + tempExpectedResult.toStdString() + "\" but it is \"" + rdFormat.toStdString() + "\"."
      "\nInput data is " + projectDir.toStdString() + " with " + testFile.toStdString(),  expectedResult == rdFormat);
}
