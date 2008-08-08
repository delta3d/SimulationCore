/* -*-c++-*-
* Simulation Core - StealthViewerSettingsTests (.h & .cpp) - Using 'The MIT License'
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
#include <cppunit/extensions/HelperMacros.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <QtGui/QApplication>
#include <QtCore/QStringList>

class SubStealthViewerSettings : public StealthQt::StealthViewerSettings
{
   public:

      SubStealthViewerSettings(const QString &appName) :
         StealthQt::StealthViewerSettings(appName)
      {

      }

      virtual ~SubStealthViewerSettings()
      {

      }

      void ParseIniFile()
      {
         StealthQt::StealthViewerSettings::ParseIniFile();
      }
};

class StealthViewerSettingsTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(StealthViewerSettingsTests);

        CPPUNIT_TEST(TestParseIniFile);
        CPPUNIT_TEST(TestAddConnection);
        CPPUNIT_TEST(TestRemoveConnection);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp();

      void tearDown();

      void TestParseIniFile();
      void TestAddConnection();
      void TestRemoveConnection();

      QApplication *mQApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthViewerSettingsTests);

void StealthViewerSettingsTests::setUp()
{
   std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory();

#if defined (_MSC_VER) && defined (_DEBUG)
   dir += "bin/StealthViewerSettingsTestsd.exe";
#elif defined (_MSC_VER)
   dir += "bin/StealthViewerSettingsTests.exe";
#else
   dir += "bin/StealthViewerSettingsTests";
#endif

   int numParams = 1;
   const char *exe = dir.c_str();

   mQApp = new QApplication(numParams, const_cast<char**>(&exe));
}

void StealthViewerSettingsTests::tearDown()
{
   delete mQApp;
   mQApp = NULL;
}

void StealthViewerSettingsTests::TestParseIniFile()
{
   // Ensure data does not exist from a previous unit test run
   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);
   CPPUNIT_ASSERT_MESSAGE("There should not be any connections added", !settings.GetNumConnections());

   settings.ParseIniFile();
   CPPUNIT_ASSERT_MESSAGE("There should not be any connections added", !settings.GetNumConnections());

   settings.AddConnection(QString("TestName"), QString("TestMap"),   QString("TestConfig"), 
                          QString("TestFed"),  QString("TestFedex"), QString("TestFedName"),
                          QString("TestRid"));

   CPPUNIT_ASSERT_EQUAL(1U, settings.GetNumConnections());
   settings.ClearAllSettings(false);
   CPPUNIT_ASSERT_EQUAL(0U, settings.GetNumConnections());

   settings.ParseIniFile();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal settings were deleted and reparsed from the ini file", 
                                 1U, settings.GetNumConnections());

   settings.ClearAllSettings(true);
   CPPUNIT_ASSERT_EQUAL(0U, settings.GetNumConnections());

   QStringList testProps;
   testProps.push_back(QString("TestName"));
   testProps.push_back(QString("TestMap"));
   testProps.push_back(QString("TestConfig"));
   testProps.push_back(QString("TestFed"));
   testProps.push_back(QString("TestFedex"));
   testProps.push_back(QString("TestFedName"));
   testProps.push_back(QString("TestRid"));

   settings.AddConnection(testProps[0], testProps[1], testProps[2], 
      testProps[3], testProps[4], testProps[5], testProps[6]);

   CPPUNIT_ASSERT_EQUAL(1U, settings.GetNumConnections());

   settings.ParseIniFile();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Reparsing the ini file should NOT add a duplicate connection", 
      1U, settings.GetNumConnections());
}

void StealthViewerSettingsTests::TestAddConnection()
{
   // Ensure data does not exist from a previous unit test run
   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);

   CPPUNIT_ASSERT(settings.GetNumConnections() == 0);

   QStringList testProps;
   testProps.push_back(QString("TestName"));
   testProps.push_back(QString("TestMap"));
   testProps.push_back(QString("TestConfig"));
   testProps.push_back(QString("TestFed"));
   testProps.push_back(QString("TestFedex"));
   testProps.push_back(QString("TestFedName"));
   testProps.push_back(QString("TestRid"));

   settings.AddConnection(testProps[0], testProps[1], testProps[2], 
                          testProps[3], testProps[4], testProps[5], 
                          testProps[6]);

   CPPUNIT_ASSERT_EQUAL((unsigned int)(1), settings.GetNumConnections());

   QStringList result = settings.GetConnectionProperties(testProps[0]);
   CPPUNIT_ASSERT(!result.isEmpty());

   for(int i = 0; i < result.size(); i++)
   {
      QString temp = result[i];
      CPPUNIT_ASSERT(temp == testProps[i]);
   }
}

void StealthViewerSettingsTests::TestRemoveConnection()
{
   // Ensure data does not exist from a previous unit test run
   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);

   CPPUNIT_ASSERT(settings.GetNumConnections() == 0);

   const unsigned int numConnections = 3;
   for(unsigned int i = 0; i < numConnections; i++)
   {
      settings.AddConnection(QString("Name")     + QString::number(i), 
                             QString("Map")      + QString::number(i), 
                             QString("Config")   + QString::number(i),
                             QString("FedFile")  + QString::number(i),
                             QString("FedEx")    + QString::number(i),
                             QString("FedName")  + QString::number(i),
                             QString("RidFile")  + QString::number(i));
   }

   CPPUNIT_ASSERT_EQUAL(numConnections, settings.GetNumConnections());

   settings.RemoveConnection(QString("Name") + QString::number(1));

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Connection removed. Number of connections should have decremented.", 
                                 numConnections - 1, settings.GetNumConnections());

   QStringList shouldBeEmpty = settings.GetConnectionProperties(QString("Name") + QString::number(1));

   CPPUNIT_ASSERT_MESSAGE("Searching for the removed connection should fail.", 
                           shouldBeEmpty.isEmpty());

   QStringList resultOne = settings.GetConnectionProperties(QString("Name") + QString::number(0));
   CPPUNIT_ASSERT(!resultOne.isEmpty());
   QString name = resultOne[0], map = resultOne[1], config = resultOne[2], 
           fedFile = resultOne[3], fedex = resultOne[4], fedName = resultOne[5],
           ridFile = resultOne[6];

   CPPUNIT_ASSERT(name == (QString("Name") + QString::number(0)));
   CPPUNIT_ASSERT(map == (QString("Map") + QString::number(0)));
   CPPUNIT_ASSERT(config == (QString("Config") + QString::number(0)));
   CPPUNIT_ASSERT(fedFile == (QString("FedFile") + QString::number(0)));
   CPPUNIT_ASSERT(fedex == (QString("FedEx") + QString::number(0)));
   CPPUNIT_ASSERT(fedName == (QString("FedName") + QString::number(0)));
   CPPUNIT_ASSERT(ridFile == (QString("RidFile") + QString::number(0)));

   QStringList resultTwo = settings.GetConnectionProperties(QString("Name") + QString::number(2));
   CPPUNIT_ASSERT(!resultTwo.isEmpty());
   CPPUNIT_ASSERT(resultTwo[0] == (QString("Name") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[1] == (QString("Map") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[2] == (QString("Config") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[3] == (QString("FedFile") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[4] == (QString("FedEx") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[5] == (QString("FedName") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[6] == (QString("RidFile") + QString::number(2)));

   settings.RemoveConnection(QString("Name") + QString::number(0));
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Connection removed. Number of connections should have decremented.", 
      numConnections - 2, settings.GetNumConnections());

   shouldBeEmpty = settings.GetConnectionProperties(QString("Name") + QString::number(0));
   CPPUNIT_ASSERT(shouldBeEmpty.isEmpty());

   resultTwo = settings.GetConnectionProperties(QString("Name") + QString::number(2));
   CPPUNIT_ASSERT(!resultTwo.isEmpty());
   CPPUNIT_ASSERT(resultTwo[0] == (QString("Name") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[1] == (QString("Map") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[2] == (QString("Config") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[3] == (QString("FedFile") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[4] == (QString("FedEx") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[5] == (QString("FedName") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[6] == (QString("RidFile") + QString::number(2)));

   settings.RemoveConnection(QString("Name") + QString::number(2));
   shouldBeEmpty = settings.GetConnectionProperties(QString("Name") + QString::number(2));
   CPPUNIT_ASSERT(shouldBeEmpty.isEmpty());
}
