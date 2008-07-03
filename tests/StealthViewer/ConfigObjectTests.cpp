/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>

#include <SimCore/Components/WeatherComponent.h>

#include <SimCore/Tools/Binoculars.h>

#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtActors/basicenvironmentactorproxy.h>
#include <dtActors/engineactorregistry.h>

#include <dtGame/gamemanager.h>

#include <dtCore/scene.h>
#include <dtCore/system.h>

#include <dtCore/timer.h>

class ConfigObjectTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(ConfigObjectTests);

      CPPUNIT_TEST(TestPreferencesGeneralConfigObject);
      CPPUNIT_TEST(TestPreferencesEnvironmentConfigObject);
      CPPUNIT_TEST(TestPreferencesToolsConfigObject);

      CPPUNIT_TEST(TestControlsCameraConfigObject);
      CPPUNIT_TEST(TestControlsRecordConfigObject);
      CPPUNIT_TEST(TestControlsPlaybackConfigObject);

      CPPUNIT_TEST(TestPreferencesEnvironmentApplyChanges);

   CPPUNIT_TEST_SUITE_END();

public:

   void setUp();
   void tearDown();

   void TestPreferencesGeneralConfigObject();
   void TestPreferencesEnvironmentConfigObject();

   void TestPreferencesToolsConfigObject();

   void TestControlsCameraConfigObject();
   void TestControlsRecordConfigObject();
   void TestControlsPlaybackConfigObject();

   void TestPreferencesEnvironmentApplyChanges();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigObjectTests);

void ConfigObjectTests::setUp()
{
   dtCore::System::GetInstance().Start();
}

void ConfigObjectTests::tearDown()
{
   dtCore::System::GetInstance().Stop();
}

void ConfigObjectTests::TestPreferencesGeneralConfigObject()
{
   dtCore::RefPtr<StealthGM::PreferencesGeneralConfigObject> genConfig =
      new StealthGM::PreferencesGeneralConfigObject;

   CPPUNIT_ASSERT_MESSAGE("The default attach mode should be Third Person",
      genConfig->GetAttachMode() == StealthGM::PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON);

   CPPUNIT_ASSERT_MESSAGE("The default camera collision flag should be true",
      genConfig->GetEnableCameraCollision());

   CPPUNIT_ASSERT_MESSAGE("The default far clipping plane should be the same as the binoculars",
      genConfig->GetFarClippingPlane() == SimCore::Tools::Binoculars::FAR_CLIPPING_PLANE);

   CPPUNIT_ASSERT_MESSAGE("The default LOD scale should be 1",
      genConfig->GetLODScale() == 1);

   CPPUNIT_ASSERT_MESSAGE("The default near clipping plane should be the same as the binoculars",
      genConfig->GetNearClippingPlane() == SimCore::Tools::Binoculars::NEAR_CLIPPING_PLANE);

   CPPUNIT_ASSERT_MESSAGE("The default performance mode should be DEFAULT",
      genConfig->GetPerformanceMode() == StealthGM::PreferencesGeneralConfigObject::PerformanceMode::DEFAULT);

   CPPUNIT_ASSERT_EQUAL(false, genConfig->GetShowAdvancedOptions());

   genConfig->SetAttachMode(StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON);
   CPPUNIT_ASSERT_EQUAL(StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON,
      genConfig->GetAttachMode());

   genConfig->SetCameraCollision(false);
   CPPUNIT_ASSERT_EQUAL(false, genConfig->GetEnableCameraCollision());

   genConfig->SetFarClippingPlane(10.0);
   CPPUNIT_ASSERT_EQUAL(10.0, genConfig->GetFarClippingPlane());

   genConfig->SetLODScale(2.0f);
   CPPUNIT_ASSERT_EQUAL(2.0f, genConfig->GetLODScale());

   genConfig->SetNearClippingPlane(1.0);
   CPPUNIT_ASSERT_EQUAL(1.0, genConfig->GetNearClippingPlane());

   genConfig->SetPerformanceMode(StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS);
   CPPUNIT_ASSERT_EQUAL(StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS,
      genConfig->GetPerformanceMode());

   genConfig->SetShowAdvancedOptions(true);
   CPPUNIT_ASSERT_EQUAL(true, genConfig->GetShowAdvancedOptions());

   genConfig = NULL;
}

void ConfigObjectTests::TestPreferencesEnvironmentConfigObject()
{
   dtCore::RefPtr<StealthGM::PreferencesEnvironmentConfigObject> envConfig =
      new StealthGM::PreferencesEnvironmentConfigObject;

   // Default values
   CPPUNIT_ASSERT_EQUAL(true, envConfig->GetUseNetworkSettings());
   CPPUNIT_ASSERT_EQUAL(false, envConfig->GetUseThemedSettings());
   CPPUNIT_ASSERT_EQUAL(false, envConfig->GetUseCustomSettings());
   CPPUNIT_ASSERT_EQUAL(0, envConfig->GetNetworkHour());
   CPPUNIT_ASSERT_EQUAL(0, envConfig->GetNetworkMinute());
   CPPUNIT_ASSERT_EQUAL(0, envConfig->GetNetworkSecond());
   CPPUNIT_ASSERT_EQUAL(0, envConfig->GetCustomHour());
   CPPUNIT_ASSERT_EQUAL(0, envConfig->GetCustomMinute());
   CPPUNIT_ASSERT_EQUAL(0, envConfig->GetCustomSecond());

   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::CloudCoverEnum::CLEAR,
      envConfig->GetCloudCover());

   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_UNLIMITED,
      envConfig->GetVisibility());

   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CUSTOM,
      envConfig->GetWeatherTheme());

   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DAY,
      envConfig->GetTimeTheme());

   // Sets and gets
   envConfig->SetUseThemedSettings();
   CPPUNIT_ASSERT_EQUAL(true, envConfig->GetUseThemedSettings());
   CPPUNIT_ASSERT_EQUAL(false, envConfig->GetUseCustomSettings());
   CPPUNIT_ASSERT_EQUAL(false, envConfig->GetUseNetworkSettings());

   envConfig->SetUseCustomSettings();
   CPPUNIT_ASSERT_EQUAL(true, envConfig->GetUseCustomSettings());
   CPPUNIT_ASSERT_EQUAL(false, envConfig->GetUseNetworkSettings());
   CPPUNIT_ASSERT_EQUAL(false, envConfig->GetUseThemedSettings());

   envConfig->SetNetworkHour(10);
   CPPUNIT_ASSERT_EQUAL(10, envConfig->GetNetworkHour());
   envConfig->SetNetworkHour(27);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The value should clamp", 23, envConfig->GetNetworkHour());

   envConfig->SetNetworkMinute(10);
   CPPUNIT_ASSERT_EQUAL(10, envConfig->GetNetworkMinute());
   envConfig->SetNetworkMinute(67);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The value should clamp", 59, envConfig->GetNetworkMinute());

   envConfig->SetNetworkSecond(10);
   CPPUNIT_ASSERT_EQUAL(10, envConfig->GetNetworkSecond());
   envConfig->SetNetworkSecond(67);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The value should clamp", 59, envConfig->GetNetworkSecond());

   envConfig->SetCustomHour(10);
   CPPUNIT_ASSERT_EQUAL(10, envConfig->GetCustomHour());
   envConfig->SetCustomHour(27);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The value should clamp", 23, envConfig->GetCustomHour());

   envConfig->SetCustomMinute(10);
   CPPUNIT_ASSERT_EQUAL(10, envConfig->GetCustomMinute());
   envConfig->SetCustomMinute(67);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The value should clamp", 59, envConfig->GetCustomMinute());

   envConfig->SetCustomSecond(10);
   CPPUNIT_ASSERT_EQUAL(10, envConfig->GetCustomSecond());
   envConfig->SetCustomSecond(67);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The value should clamp", 59, envConfig->GetCustomSecond());

   envConfig->SetCloudCover(dtActors::BasicEnvironmentActor::CloudCoverEnum::SCATTERED);
   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::CloudCoverEnum::SCATTERED,
      envConfig->GetCloudCover());

   envConfig->SetVisibility(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_LIMITED);
   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::VisibilityTypeEnum::VISIBILITY_LIMITED,
      envConfig->GetVisibility());

   envConfig->SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_RAINY);
   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_RAINY,
      envConfig->GetWeatherTheme());

   envConfig->SetTimeTheme(dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DUSK);
   CPPUNIT_ASSERT_EQUAL(dtActors::BasicEnvironmentActor::TimePeriodEnum::TIME_DUSK,
      envConfig->GetTimeTheme());

   envConfig = NULL;
}

void ConfigObjectTests::TestPreferencesToolsConfigObject()
{
   dtCore::RefPtr<StealthGM::PreferencesToolsConfigObject> toolsConfig =
      new StealthGM::PreferencesToolsConfigObject;

   // Defaults
   CPPUNIT_ASSERT_EQUAL(StealthGM::PreferencesToolsConfigObject::CoordinateSystem::MGRS,
      toolsConfig->GetCoordinateSystem());

   CPPUNIT_ASSERT_EQUAL(true, toolsConfig->GetShowBinocularImage());
   CPPUNIT_ASSERT_EQUAL(true, toolsConfig->GetShowDistanceToObject());
   CPPUNIT_ASSERT_EQUAL(7.0f, toolsConfig->GetMagnification());
   CPPUNIT_ASSERT_EQUAL(true, toolsConfig->GetAutoAttachOnSelection());

   toolsConfig->SetShowBinocularImage(false);
   CPPUNIT_ASSERT_EQUAL(false, toolsConfig->GetShowBinocularImage());

   toolsConfig->SetShowDistanceToObject(false);
   CPPUNIT_ASSERT_EQUAL(false, toolsConfig->GetShowDistanceToObject());

   toolsConfig->SetMagnification(1.5f);
   CPPUNIT_ASSERT_EQUAL(1.5f, toolsConfig->GetMagnification());

   toolsConfig->SetAutoAttachOnSelection(false);
   CPPUNIT_ASSERT_EQUAL(false, toolsConfig->GetAutoAttachOnSelection());

   toolsConfig = NULL;
}

void ConfigObjectTests::TestControlsCameraConfigObject()
{
   dtCore::RefPtr<StealthGM::ControlsCameraConfigObject> cameraConfig =
      new StealthGM::ControlsCameraConfigObject;
}

void ConfigObjectTests::TestControlsRecordConfigObject()
{
   dtCore::RefPtr<StealthGM::ControlsRecordConfigObject> recordConfig =
      new StealthGM::ControlsRecordConfigObject;

   // Defaults
   CPPUNIT_ASSERT(recordConfig->GetOutputFilename().empty());
   CPPUNIT_ASSERT(!recordConfig->GetShowAdvancedOptions());
   CPPUNIT_ASSERT(!recordConfig->GetAutoKeyFrame());
   CPPUNIT_ASSERT_EQUAL(5, recordConfig->GetAutoKeyFrameInterval());

   recordConfig->SetOutputFilename("Somefile");
   CPPUNIT_ASSERT_EQUAL(std::string("Somefile"), recordConfig->GetOutputFilename());

   recordConfig->SetShowAdvancedOptions(true);
   CPPUNIT_ASSERT(recordConfig->GetShowAdvancedOptions());

   recordConfig->SetAutoKeyFrame(true);
   CPPUNIT_ASSERT(recordConfig->GetAutoKeyFrame());

   recordConfig->SetAutoKeyFrameInterval(10);
   CPPUNIT_ASSERT_EQUAL(10, recordConfig->GetAutoKeyFrameInterval());

   recordConfig = NULL;
}

void ConfigObjectTests::TestControlsPlaybackConfigObject()
{
   dtCore::RefPtr<StealthGM::ControlsPlaybackConfigObject> playbackConfig =
      new StealthGM::ControlsPlaybackConfigObject;

   // Defaults
   CPPUNIT_ASSERT(playbackConfig->GetInputFilename().empty());
   CPPUNIT_ASSERT(!playbackConfig->GetShowAdvancedOptions());

   playbackConfig->SetInputFilename("SomeFile");
   CPPUNIT_ASSERT_EQUAL(std::string("SomeFile"), playbackConfig->GetInputFilename());

   playbackConfig->SetShowAdvancedOptions(true);
   CPPUNIT_ASSERT(playbackConfig->GetShowAdvancedOptions());

   playbackConfig = NULL;
}

void ConfigObjectTests::TestPreferencesEnvironmentApplyChanges()
{
   dtCore::RefPtr<StealthGM::PreferencesEnvironmentConfigObject> envConfig =
      new StealthGM::PreferencesEnvironmentConfigObject;

   dtCore::RefPtr<dtGame::GameManager> gm = new dtGame::GameManager(*new dtCore::Scene);
   dtCore::RefPtr<SimCore::Components::WeatherComponent> weatherComponent =
      new SimCore::Components::WeatherComponent;

   gm->AddComponent(*weatherComponent, dtGame::GameManager::ComponentPriority::NORMAL);

   dtCore::RefPtr<SimCore::Actors::DayTimeActorProxy> dayTimeProxy;
   gm->CreateActor(*SimCore::Actors::EntityActorRegistry::DAYTIME_ACTOR_TYPE, dayTimeProxy);

   dtCore::RefPtr<dtActors::BasicEnvironmentActorProxy> envProxy;
   gm->CreateActor(*dtActors::EngineActorRegistry::ENVIRONMENT_ACTOR_TYPE, envProxy);

   dtCore::RefPtr<SimCore::Actors::UniformAtmosphereActorProxy> atmosphereProxy;
   gm->CreateActor(*SimCore::Actors::EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE, atmosphereProxy);

   gm->AddActor(*dayTimeProxy, false, false);
   gm->AddActor(*atmosphereProxy, false, false);
   gm->SetEnvironmentActor(envProxy.get());

   dtCore::AppSleep(10U);
   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(weatherComponent->GetDayTimeActor() != NULL);
   CPPUNIT_ASSERT(weatherComponent->GetAtmosphereActor() != NULL);

   CPPUNIT_ASSERT_MESSAGE("Setting a normal environment on the game manager should NOT \
                          register on the WeatherComponent because it is no longer supported",
                          weatherComponent->GetEphemerisEnvironment() == NULL);

   gm = NULL;
}

