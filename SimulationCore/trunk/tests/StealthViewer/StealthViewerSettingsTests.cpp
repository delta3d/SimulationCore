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
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <StealthViewer/Qt/StealthViewerSettings.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>
#include <SimCore/Components/LabelManager.h>
#include <SimCore/UnitEnums.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/project.h>
#include <QtGui/QApplication>
#include <QtCore/QStringList>
#include <StealthViewer/Qt/StealthViewerData.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

class SubStealthViewerSettings : public StealthQt::StealthViewerSettings
{
   public:

      SubStealthViewerSettings(const QString &appName)
      : StealthQt::StealthViewerSettings(appName)
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

      CPPUNIT_TEST(TestToolSettings);
      CPPUNIT_TEST(TestGeneralSettings);
      CPPUNIT_TEST(TestVisibilitySettings);
      CPPUNIT_TEST(TestParseIniFile);
      CPPUNIT_TEST(TestAddConnectionError);
      CPPUNIT_TEST(TestAddConnection);
      CPPUNIT_TEST(TestRemoveConnection);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp();

      void tearDown();

      void TestToolSettings();
      void TestGeneralSettings();
      void TestVisibilitySettings();
      void TestParseIniFile();
      void TestAddConnectionError();
      void TestAddConnection();
      void TestRemoveConnection();

      QApplication *mQApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthViewerSettingsTests);

void StealthViewerSettingsTests::setUp()
{
#if defined (_MSC_VER) && defined (_DEBUG)
   static std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory() + "bin/StealthViewerSettingsTestsd.exe";
#elif defined (_MSC_VER)
   static std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory() + "bin/StealthViewerSettingsTests.exe";
#else
   static std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory() + "bin/StealthViewerSettingsTests";
#endif

   static int numParams = 1;
   static const char *exe = dir.c_str();

   mQApp = new QApplication(numParams, const_cast<char**>(&exe));
}

void StealthViewerSettingsTests::tearDown()
{
   delete mQApp;
   mQApp = NULL;
}

void StealthViewerSettingsTests::TestToolSettings()
{
   StealthGM::PreferencesToolsConfigObject& toolsConfig =
      StealthQt::StealthViewerData::GetInstance().GetToolsConfigObject();

   StealthGM::ViewWindowConfigObject& viewConfig =
      StealthQt::StealthViewerData::GetInstance().GetViewWindowConfigObject();

   dtCore::RefPtr<dtGame::GameManager> gm = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   gm->SetApplication(GetGlobalApplication());
   viewConfig.CreateMainViewWindow(*gm);

   toolsConfig.SetAngleUnit(SimCore::UnitOfAngle::MIL);
   toolsConfig.SetLengthUnit(SimCore::UnitOfLength::FOOT);

   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);
   settings.WritePreferencesToFile(false);
   settings.LoadPreferences();

   CPPUNIT_ASSERT_EQUAL(SimCore::UnitOfAngle::MIL, toolsConfig.GetAngleUnit());
   CPPUNIT_ASSERT_EQUAL(SimCore::UnitOfLength::FOOT, toolsConfig.GetLengthUnit());
}

void StealthViewerSettingsTests::TestGeneralSettings()
{
   StealthGM::PreferencesGeneralConfigObject& genConfig =
      StealthQt::StealthViewerData::GetInstance().GetGeneralConfigObject();

   StealthGM::ViewWindowConfigObject& viewConfig =
      StealthQt::StealthViewerData::GetInstance().GetViewWindowConfigObject();

   dtCore::RefPtr<dtGame::GameManager> gm = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   gm->SetApplication(GetGlobalApplication());
   viewConfig.CreateMainViewWindow(*gm);

   genConfig.SetAttachMode(StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON);
   genConfig.SetAttachPointNodeName("frank");

   osg::Vec3 newInitialAttachRot(3.3, 9.7, 8.4);
   genConfig.SetInitialAttachRotationHPR(newInitialAttachRot);

   genConfig.SetShouldAutoAttachToEntity(true);

   genConfig.SetCameraCollision(false);
   genConfig.SetLODScale(2.0f);
   genConfig.SetPerformanceMode(StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS);
   genConfig.SetShowAdvancedOptions(true);
   genConfig.SetReconnectOnStartup(true, "booga");
   genConfig.SetAutoReconnect(true);
   genConfig.SetAutoReconnectTimeout(43);

   {
      viewConfig.GetMainViewWindow().SetUseAspectRatioForFOV(false);
      viewConfig.GetMainViewWindow().SetFOVAspectRatio(31.9f);
      viewConfig.GetMainViewWindow().SetFOVHorizontal(9373.3f);
      viewConfig.GetMainViewWindow().SetFOVVerticalForAspect(300.0f);
      viewConfig.GetMainViewWindow().SetFOVVerticalForHorizontal(-20.0f);
   }

   viewConfig.SetFarClippingPlane(10.0);
   viewConfig.SetNearClippingPlane(1.0);

   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);
   settings.WritePreferencesToFile(false);
   settings.LoadPreferences();

   CPPUNIT_ASSERT_EQUAL(StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON,
      genConfig.GetAttachMode());

   CPPUNIT_ASSERT_EQUAL_MESSAGE("The attach point node name should be frank",
      genConfig.GetAttachPointNodeName(), std::string("frank"));

   CPPUNIT_ASSERT_MESSAGE("The attach rotation should have changed.",
      dtUtil::Equivalent(genConfig.GetInitialAttachRotationHPR(), newInitialAttachRot, 0.01f));

   CPPUNIT_ASSERT_MESSAGE("The auto attach to entity should new be true.",
      genConfig.GetShouldAutoAttachToEntity());

   {
      CPPUNIT_ASSERT_MESSAGE("Use aspect ratio for fov should now be false.",
         !viewConfig.GetMainViewWindow().UseAspectRatioForFOV());

      CPPUNIT_ASSERT_MESSAGE("The aspect ratio should be 10.0.",
         dtUtil::Equivalent(viewConfig.GetMainViewWindow().GetFOVAspectRatio(), 10.0f));

      CPPUNIT_ASSERT_MESSAGE("The default horizontal fov should be 179.0.",
         dtUtil::Equivalent(viewConfig.GetMainViewWindow().GetFOVHorizontal(), 179.0f));

      CPPUNIT_ASSERT_MESSAGE("The default vertical fov should be 160.0.",
         dtUtil::Equivalent(viewConfig.GetMainViewWindow().GetFOVVerticalForAspect(), 160.0f));

      CPPUNIT_ASSERT_MESSAGE("The default vertical fov should be 1.0.",
         dtUtil::Equivalent(viewConfig.GetMainViewWindow().GetFOVVerticalForHorizontal(), 1.0f));
   }

   CPPUNIT_ASSERT_EQUAL(10.0, viewConfig.GetFarClippingPlane());
   CPPUNIT_ASSERT_EQUAL(1.0, viewConfig.GetNearClippingPlane());

   CPPUNIT_ASSERT_EQUAL(false, genConfig.GetEnableCameraCollision());

   CPPUNIT_ASSERT_EQUAL(2.0f, genConfig.GetLODScale());

   CPPUNIT_ASSERT_EQUAL(StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS,
      genConfig.GetPerformanceMode());

   CPPUNIT_ASSERT_EQUAL(true, genConfig.GetShowAdvancedOptions());

   CPPUNIT_ASSERT_EQUAL(true, genConfig.GetReconnectOnStartup());
   CPPUNIT_ASSERT_EQUAL(std::string("booga"), genConfig.GetStartupConnectionName());
   CPPUNIT_ASSERT_EQUAL(true, genConfig.GetAutoReconnect());
   CPPUNIT_ASSERT_EQUAL(43U, genConfig.GetAutoReconnectTimeout());
}

void StealthViewerSettingsTests::TestVisibilitySettings()
{
   StealthGM::PreferencesVisibilityConfigObject& visConfig =
      StealthQt::StealthViewerData::GetInstance().GetVisibilityConfigObject();

   bool newValue = !visConfig.GetBFGCloseTops();
   visConfig.SetBFGCloseTops(newValue);

   SimCore::Components::LabelOptions options = visConfig.GetLabelOptions();
   options.SetMaxLabelDistance(-1.0f);
   options.SetShowLabels(false);
   options.SetShowLabelsForEntities(false);
   options.SetShowLabelsForBlips(false);
   options.SetShowLabelsForPositionReports(false);
   visConfig.SetLabelOptions(options);

   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);
   settings.WritePreferencesToFile(false);
   //reset the values to defaults
   visConfig.SetLabelOptions(SimCore::Components::LabelOptions());
   settings.LoadPreferences();
   SimCore::Components::LabelOptions options2 = visConfig.GetLabelOptions();

   CPPUNIT_ASSERT(options == options2);

   options.SetMaxLabelDistance(1000.0f);
   options.SetShowLabels(true);
   options.SetShowLabelsForEntities(true);
   options.SetShowLabelsForBlips(true);
   options.SetShowLabelsForPositionReports(true);
   visConfig.SetLabelOptions(options);

   settings.WritePreferencesToFile(false);
   //reset the values to defaults
   visConfig.SetLabelOptions(SimCore::Components::LabelOptions());
   // set back to default.
   visConfig.SetBFGCloseTops(!newValue);

   settings.LoadPreferences();
   options2 = visConfig.GetLabelOptions();

   CPPUNIT_ASSERT(options == options2);
   CPPUNIT_ASSERT_EQUAL(newValue, visConfig.GetBFGCloseTops());
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
                          QString("TestRid"), QString("TestRTIStandard"), QString("HLA"), QString("TestIPAddress"),
                          QString("TestServerPort"), QString("TestServerGameName"), QString("1"),
                          QString("TestDISIPAddress"), 62040, "false", 1, 1500, 0,
                          0, QString("TestDISActorXMLFile"));

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
   testProps.push_back(QString("TestRTIStandard"));
   testProps.push_back(QString("HLA"));
   testProps.push_back(QString("TestIPAddress"));
   testProps.push_back(QString("TestServerPort"));
   testProps.push_back(QString("TestServerGameName"));
   testProps.push_back(QString("1"));
   testProps.push_back(QString("TestDISIPAddress"));
   testProps.push_back(QString("62040"));
   testProps.push_back(QString("false"));
   testProps.push_back(QString("1"));
   testProps.push_back(QString("1500"));
   testProps.push_back(QString("0"));
   testProps.push_back(QString("0"));
   testProps.push_back(QString("TestDISActorXMLFile"));

   settings.AddConnection(testProps[0], testProps[1], testProps[2],
      testProps[3], testProps[4], testProps[5], testProps[6], 
      testProps[7], testProps[8], testProps[9], testProps[10], testProps[11], testProps[12],
      testProps[13],
      testProps[14].toUInt(), testProps[15].toUInt(), testProps[16] == "true" ? true : false,
      testProps[17].toUShort(), testProps[18].toUShort(),
      testProps[19].toUInt(), testProps[20]);

   CPPUNIT_ASSERT_EQUAL(1U, settings.GetNumConnections());

   settings.ParseIniFile();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Reparsing the ini file should NOT add a duplicate connection",
      1U, settings.GetNumConnections());
}

void StealthViewerSettingsTests::TestAddConnectionError()
{
   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the name is not set.",
            !settings.AddConnection(QString(), "TestMap", "TestConfig",
            "Fed.fed", "Fedex", "TestFedName", QString(), "1.3", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(),
            "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995U,
            true, (unsigned char)(1), (unsigned short)1500U, (unsigned short)0U, 0U, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the map is not set.",
            !settings.AddConnection("TestName", "", "TestConfig",
            "Fed.fed", "Fedex", "TestFedName", "", "1.3", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the rti config is not set.",
            !settings.AddConnection("TestName", "TestMap", "",
            "Fed.fed", "Fedex", "TestFedName", "", "1.3", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the fed file is not set.",
            !settings.AddConnection("TestName", "TestMap", "TestConfig",
            "", "Fedex", "TestFedName", "", "1.3", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the fedex is not set.",
            !settings.AddConnection("TestName", "TestMap", "TestConfig",
            "Fed.fed", "", "TestFedName", "", "1.3", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the federation name is not set.",
            !settings.AddConnection("TestName", "TestMap", "TestConfig",
            "Fed.fed", "Fedex", "", "", "1.3", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should NOT fail if the rid file and all non-hla settings are empty.",
            settings.AddConnection("TestNameWork", "TestMap", "TestConfig",
            "Fed.fed", "Fedex", "TestFedName", "", "1.3", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(), "", "", "", "", "", 0,
            true, 0, 0, 0, 0, ""));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection type is not set.",
            !settings.AddConnection("TestName", "TestMap", "TestConfig",
            "Fed.fed", "Fedex", "TestFedName", "", "1.3", "", "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the rti standard is not set.",
            !settings.AddConnection("TestName", "TestMap", "TestConfig",
            "Fed.fed", "Fedex", "TestFedName", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_HLA.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should NOT fail if the connection is not HLA and the hla settings are empty.",
            settings.AddConnection("TestNameWork2", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection is Client Server and the server is empty.",
            !settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "", "8000", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection is Client Server and the port is empty.",
            !settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "127.0.0.1", "", "MyGame", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection is Client Server and the game name is empty.",
            !settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "127.0.0.1", "8000", "", "1.0", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection is Client Server and the version is empty.",
            !settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "127.0.0.1", "8000", "MyGame", "", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should NOT fail if the connection is Client Server and the DIS settings are empty.",
            settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "127.0.0.1", "8000", "MyGame", "1.0", "", 0,
            true, 0, 0, 0, 0, ""));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should NOT fail if the connection is DIS and the other settings are empty.",
            settings.AddConnection("TestNameWork3", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_DIS.c_str(), "", "", "", "", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection is DIS and the IP address is empty.",
            !settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "", "", "", "", "", 1995,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection is DIS and the port is 0.",
            !settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "", "", "", "", "192.168.0.255", 0,
            true, 1, 1500, 0, 0, "TestDISActorXMLFile"));

   CPPUNIT_ASSERT_MESSAGE("Adding connection should fail if the connection is DIS and the XML config is empty.",
            !settings.AddConnection("TestName", "TestMap", "",
            "", "", "", "", "", StealthQt::StealthViewerSettings::CONNECTIONTYPE_CLIENTSERVER.c_str(), "", "", "", "", "192.168.0.255", 1995,
            true, 1, 1500, 0, 0, ""));
}

void StealthViewerSettingsTests::TestAddConnection()
{
   // Ensure data does not exist from a previous unit test run
   SubStealthViewerSettings settings(QString("UnitTest"));
   settings.ClearAllSettings(true);

   CPPUNIT_ASSERT(settings.GetNumConnections() == 0);

   QStringList testProps;
   testProps.push_back(QString("TestName")); //0
   testProps.push_back(QString("TestMap"));  //1
   testProps.push_back(QString("TestConfig")); //2
   testProps.push_back(QString("TestFed")); //3
   testProps.push_back(QString("TestFedex"));  //4
   testProps.push_back(QString("TestFedName"));  //5
   testProps.push_back(QString("TestRid"));  //6
   testProps.push_back(QString("TestRTIStandard"));  //7
   testProps.push_back(QString("HLA"));  //8 //connection type
   testProps.push_back(QString("TestIPAddress")); //9
   testProps.push_back(QString("TestServerPort")); //10
   testProps.push_back(QString("TestServerGameName")); //11
   testProps.push_back(QString("1")); //12 //game version
   testProps.push_back(QString("TestDISIPAddress")); //13
   testProps.push_back(QString("62040"));//14 //dis port
   testProps.push_back(QString("false"));//15 //dis broadcast
   testProps.push_back(QString("1"));//16  //dis exercise id
   testProps.push_back(QString("1500"));//17 //dis site id
   testProps.push_back(QString("0"));//18 //dis app id
   testProps.push_back(QString("0"));//19 //dis mtu
   testProps.push_back(QString("TestDISActorXMLFile"));//20

   settings.AddConnection(testProps[0], testProps[1], testProps[2],
                          testProps[3], testProps[4], testProps[5],
                          testProps[6], testProps[7], testProps[8], testProps[9],
                          testProps[10], testProps[11], testProps[12],
                          testProps[13],
                          testProps[14].toUInt(), testProps[15] == "true" ? true : false,
                          testProps[16].toUInt(),
                          testProps[17].toUShort(), testProps[18].toUShort(),
                          testProps[19].toUInt(), testProps[20]);

   CPPUNIT_ASSERT_EQUAL((unsigned int)(1), settings.GetNumConnections());

   QStringList result = settings.GetConnectionProperties(testProps[0]);

   CPPUNIT_ASSERT(!result.isEmpty());

   for(int i = 0; i < result.size(); i++)
   {
      QString temp = result[i];
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Returned connection setting didn't match what was set:#" + dtUtil::ToString(i),
         testProps[i].toStdString(), temp.toStdString());
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
                             QString("RidFile")  + QString::number(i),
                             QString("RTIStandard")  + QString::number(i),
                             QString("HLA")  + QString::number(i),
                             QString("ServerIPAddress")  + QString::number(i),
                             QString("ServerPort")  + QString::number(i),
                             QString("ServerGameName")  + QString::number(i),
                             QString("12")  + QString::number(i),
                             QString("TestDISIPAddress") + QString::number(i),
                             (QString("62040") + QString::number(i)).toUInt(),
                             false,
                             (QString("1") + QString::number(i)).toUInt(),
                             (QString("1500") + QString::number(i)).toUShort(),
                             (QString("0") + QString::number(i)).toUShort(),
                             (QString("0") + QString::number(i)).toUInt(),
                             QString("TestDISActorXMLFile") + QString::number(i));
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
           ridFile = resultOne[6], rtiStandard = resultOne[7], connectionType = resultOne[8], serverIPAddress = resultOne[9],
           serverPort = resultOne[10], serverGameName = resultOne[11], serverGameVersion = resultOne[12];

   CPPUNIT_ASSERT_EQUAL(name.toStdString(), (QString("Name") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(map.toStdString(), (QString("Map") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(config.toStdString(), (QString("Config") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(fedFile.toStdString(), (QString("FedFile") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(fedex.toStdString(), (QString("FedEx") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(fedName.toStdString(), (QString("FedName") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(ridFile.toStdString(), (QString("RidFile") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(rtiStandard.toStdString(), (QString("RTIStandard") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(connectionType.toStdString(), (QString("HLA") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(serverIPAddress.toStdString(), (QString("ServerIPAddress") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(serverPort.toStdString(), (QString("ServerPort") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(serverGameName.toStdString(), (QString("ServerGameName") + QString::number(0)).toStdString());
   CPPUNIT_ASSERT_EQUAL(serverGameVersion.toStdString(), (QString("12") + QString::number(0)).toStdString());

   QStringList resultTwo = settings.GetConnectionProperties(QString("Name") + QString::number(2));
   CPPUNIT_ASSERT(!resultTwo.isEmpty());
   CPPUNIT_ASSERT(resultTwo[0] == (QString("Name") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[1] == (QString("Map") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[2] == (QString("Config") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[3] == (QString("FedFile") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[4] == (QString("FedEx") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[5] == (QString("FedName") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[6] == (QString("RidFile") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[7] == (QString("RTIStandard") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[8] == (QString("HLA") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[9] == (QString("ServerIPAddress") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[10] == (QString("ServerPort") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[11] == (QString("ServerGameName") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[12] == (QString("12") + QString::number(2)));

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
   CPPUNIT_ASSERT(resultTwo[7] == (QString("RTIStandard") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[8] == (QString("HLA") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[9] == (QString("ServerIPAddress") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[10] == (QString("ServerPort") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[11] == (QString("ServerGameName") + QString::number(2)));
   CPPUNIT_ASSERT(resultTwo[12] == (QString("12") + QString::number(2)));

   settings.RemoveConnection(QString("Name") + QString::number(2));
   shouldBeEmpty = settings.GetConnectionProperties(QString("Name") + QString::number(2));
   CPPUNIT_ASSERT(shouldBeEmpty.isEmpty());
}
