/* -*-c++-*-
* Simulation Core - StealthViewerHLAOptionsTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, Alion Science and Technology Corporation
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
* @author Eddie Johnson
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <StealthViewer/Qt/HLAOptions.h>
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

      QApplication* mQApp;

};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthViewerHLAOptionsTests);

void StealthViewerHLAOptionsTests::setUp()
{
#if defined (_MSC_VER) && defined (_DEBUG)
   static std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory() + "bin/StealthViewerSettingsTestsd.exe";
#elif defined (_MSC_VER)
   static std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory() + "bin/StealthViewerSettingsTests.exe";
#else
   static std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory() + "bin/StealthViewerSettingsTests";
#endif

   static int numParams = 1;
   static const char* exe = dir.c_str();
  
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
   TestConvertFileName("C:\\ProjectAssets", "C:\\ProjectAssets\\CEGUI\\cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("c:\\ProjectAssets", "C:\\ProjectAssets\\CEGUI\\cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("C:\\ProjectAssets", "c:\\ProjectAssets\\CEGUI\\cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("/development/test/ProjectAssets", "/development/test/ProjectAssets/CEGUI/cegui.config", "CEGUI:cegui.config");
   TestConvertFileName("C:/junk", "/development/test/ProjectAssets/CEGUI/cegui.config", "");
   TestConvertFileName("", "/development/test/ProjectAssets/CEGUI/cegui.config", "");
   TestConvertFileName("/development/test/DVTEP", "/development/test/ProjectAssets/CEGUI/cegui.config", "");
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
