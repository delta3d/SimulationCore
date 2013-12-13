/* -*-c++-*-
* Simulation Core - ConfigObjectTests (.h & .cpp) - Using 'The MIT License'
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

#include <StealthViewer/GMApp/ViewerConfigComponent.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
#include <StealthViewer/GMApp/ControlsRecordConfigObject.h>
#include <StealthViewer/GMApp/ControlsPlaybackConfigObject.h>
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>
#include <StealthViewer/GMApp/ViewWindowConfigObject.h>
#include <StealthViewer/GMApp/StealthHUD.h>

#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/LabelManager.h>
#include <SimCore/Components/BaseHUD.h>
#include <SimCore/Components/ViewerMessageProcessor.h>

#include <SimCore/Tools/Binoculars.h>

#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <SimCore/Actors/EphemerisEnvironmentActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/BattlefieldGraphicsActor.h>
#include <SimCore/UnitEnums.h>

#include <dtActors/engineactorregistry.h>

#include <dtGame/gamemanager.h>

#include <dtCore/scene.h>
#include <dtCore/system.h>

#include <dtCore/timer.h>

#include <osgViewer/GraphicsWindow>

#include <UnitTestMain.h>
#include <dtABC/application.h>

class ConfigObjectTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(ConfigObjectTests);

      CPPUNIT_TEST(TestViewWindowConfigObject);
      CPPUNIT_TEST(TestAdditionalViewWindows);

      CPPUNIT_TEST(TestPreferencesGeneralConfigObject);
      CPPUNIT_TEST(TestPreferencesEnvironmentConfigObject);
      CPPUNIT_TEST(TestPreferencesToolsConfigObject);
      CPPUNIT_TEST(TestPreferencesVisibilityConfigObject);

      CPPUNIT_TEST(TestControlsCameraConfigObject);
      CPPUNIT_TEST(TestControlsRecordConfigObject);
      CPPUNIT_TEST(TestControlsPlaybackConfigObject);

      CPPUNIT_TEST(TestPreferencesEnvironmentApplyChanges);

   CPPUNIT_TEST_SUITE_END();

public:

#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
   //////////////////////////////////////////////////////////////
   void setupCEGUI()
   {
      mMainGUIWindow = new SimCore::Components::HUDGroup("root","DefaultGUISheet");
      CEGUI::System::getSingleton().setGUISheet(mMainGUIWindow->GetCEGUIWindow());
   //SLEEP(200);
   }
#endif

   void setUp();
   void tearDown();

   void TestViewWindowConfigObject();
   void TestAdditionalViewWindows();
   void TestPreferencesGeneralConfigObject();
   void TestPreferencesEnvironmentConfigObject();

   void TestPreferencesToolsConfigObject();
   void TestPreferencesVisibilityConfigObject();

   void TestControlsCameraConfigObject();
   void TestControlsRecordConfigObject();
   void TestControlsPlaybackConfigObject();

   void TestPreferencesEnvironmentApplyChanges();


private:
   void CheckFOVDefaults(StealthGM::ViewWindowWrapper& viewConfig);

   void InitCallbackTest(StealthGM::ViewWindowWrapper& vw);

   void RemoveCallbackTest(StealthGM::ViewWindowWrapper& vw);

   dtCore::RefPtr<dtGame::GameManager> mGM;
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
      dtCore::RefPtr<dtGUI::CEUIDrawable> mGUI;
#else
      dtCore::RefPtr<dtGUI::GUI> mGUI;
#endif

   bool mInitCalled;
   bool mRemoveCalled;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigObjectTests);

void ConfigObjectTests::setUp()
{
   dtCore::System::GetInstance().Start();
   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   mGM->SetApplication(GetGlobalApplication());
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
   setupCEGUI();
#else
   mGUI = &GetGlobalGUI();
#endif
}

void ConfigObjectTests::tearDown()
{
   if (mGM.valid())
   {
      dtCore::ObserverPtr<dtGame::GameManager> gmOb = mGM.get();
      mGM = NULL;
      CPPUNIT_ASSERT(!gmOb.valid());
   }
   mGUI = NULL;
   dtCore::System::GetInstance().Stop();
}

void ConfigObjectTests::CheckFOVDefaults(StealthGM::ViewWindowWrapper& viewWrapper)
{
   CPPUNIT_ASSERT_MESSAGE("Default Use aspect ratio for fov should true.",
      viewWrapper.UseAspectRatioForFOV());

   CPPUNIT_ASSERT_MESSAGE("The default aspect ratio should be 1.6.",
      dtUtil::Equivalent(viewWrapper.GetFOVAspectRatio(), 1.6f));

   CPPUNIT_ASSERT_MESSAGE("The default horizontal fov should be 96.0.",
      dtUtil::Equivalent(viewWrapper.GetFOVHorizontal(), 96.0f));

   CPPUNIT_ASSERT_MESSAGE("The default vertical fov should be 60.0.",
      dtUtil::Equivalent(viewWrapper.GetFOVVerticalForAspect(), 60.0f));

   CPPUNIT_ASSERT_MESSAGE("The default vertical fov should be 60.0.",
      dtUtil::Equivalent(viewWrapper.GetFOVVerticalForHorizontal(), 60.0f));

   CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The default far clipping plane should be the same as the binoculars",
      viewWrapper.GetFarClippingPlane(), SimCore::Tools::Binoculars::FAR_CLIPPING_PLANE, 1.0);

   CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The default near clipping plane should be the same as the binoculars",
      viewWrapper.GetNearClippingPlane(), SimCore::Tools::Binoculars::NEAR_CLIPPING_PLANE, 0.1);
}

void ConfigObjectTests::TestViewWindowConfigObject()
{
   dtCore::RefPtr<StealthGM::ViewWindowConfigObject> viewConfig =
      new StealthGM::ViewWindowConfigObject;

   viewConfig->SetFarClippingPlane(10.0);
   CPPUNIT_ASSERT_EQUAL(10.0, viewConfig->GetFarClippingPlane());
   CPPUNIT_ASSERT(viewConfig->IsUpdated());
   viewConfig->SetIsUpdated(false);

   viewConfig->SetNearClippingPlane(1.0);
   CPPUNIT_ASSERT_EQUAL(1.0, viewConfig->GetNearClippingPlane());
   CPPUNIT_ASSERT(viewConfig->IsUpdated());
   viewConfig->SetIsUpdated(false);

   CPPUNIT_ASSERT_THROW(viewConfig->GetMainViewWindow(), dtUtil::Exception);

   dtCore::RefPtr<dtGame::GameManager> gm = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   gm->SetApplication(GetGlobalApplication());
   viewConfig->CreateMainViewWindow(*gm);

   CPPUNIT_ASSERT_NO_THROW(viewConfig->GetMainViewWindow());

   CheckFOVDefaults(viewConfig->GetMainViewWindow());

   viewConfig->GetMainViewWindow().SetUseAspectRatioForFOV(false);
   CPPUNIT_ASSERT_MESSAGE("Use aspect ratio for fov should now be false.",
      !viewConfig->GetMainViewWindow().UseAspectRatioForFOV());
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());
   viewConfig->GetMainViewWindow().SetIsUpdated(false);

   viewConfig->GetMainViewWindow().SetFOVAspectRatio(31.9f);
   CPPUNIT_ASSERT_MESSAGE("The aspect ratio should now be clamped at 10.",
      dtUtil::Equivalent(viewConfig->GetMainViewWindow().GetFOVAspectRatio(), 10.0f));
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());
   viewConfig->GetMainViewWindow().SetIsUpdated(false);

   viewConfig->GetMainViewWindow().SetFOVHorizontal(9373.3f);
   CPPUNIT_ASSERT_MESSAGE("The horizontal fov should now be clamped at 179.",
      dtUtil::Equivalent(viewConfig->GetMainViewWindow().GetFOVHorizontal(), 179.0f));
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());
   viewConfig->GetMainViewWindow().SetIsUpdated(false);

   viewConfig->GetMainViewWindow().SetFOVVerticalForAspect(1100.1f);
   CPPUNIT_ASSERT_MESSAGE("The vertical fov should now be clamped at 160.",
      dtUtil::Equivalent(viewConfig->GetMainViewWindow().GetFOVVerticalForAspect(), 160.0f));
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());
   viewConfig->GetMainViewWindow().SetIsUpdated(false);

   viewConfig->GetMainViewWindow().SetFOVVerticalForHorizontal(188.9f);
   CPPUNIT_ASSERT_MESSAGE("The vertical fov should now be clamped at 160.",
      dtUtil::Equivalent(viewConfig->GetMainViewWindow().GetFOVVerticalForHorizontal(), 160.0f));
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());
   viewConfig->GetMainViewWindow().SetIsUpdated(false);

   viewConfig->GetMainViewWindow().SetFarClippingPlane(10.0);
   CPPUNIT_ASSERT_EQUAL(10.0, viewConfig->GetMainViewWindow().GetFarClippingPlane());
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());
   viewConfig->GetMainViewWindow().SetIsUpdated(false);

   viewConfig->GetMainViewWindow().SetNearClippingPlane(1.0);
   CPPUNIT_ASSERT_EQUAL(1.0, viewConfig->GetMainViewWindow().GetNearClippingPlane());
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());
   viewConfig->GetMainViewWindow().SetIsUpdated(false);

   //The FOV reset should set update to true and then the values should be the defaults again.
   viewConfig->GetMainViewWindow().FOVReset();
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsUpdated());

   viewConfig->GetMainViewWindow().SetFarClippingPlane(SimCore::Tools::Binoculars::FAR_CLIPPING_PLANE);
   viewConfig->GetMainViewWindow().SetNearClippingPlane(SimCore::Tools::Binoculars::NEAR_CLIPPING_PLANE);

   CheckFOVDefaults(viewConfig->GetMainViewWindow());
}

void ConfigObjectTests::InitCallbackTest(StealthGM::ViewWindowWrapper& vw)
{
   mInitCalled = true;
}

void ConfigObjectTests::RemoveCallbackTest(StealthGM::ViewWindowWrapper& vw)
{
   mRemoveCalled = true;
}

void ConfigObjectTests::TestAdditionalViewWindows()
{
   std::string newViewOne("newView1");
   std::string newViewTwo("newView2");

   dtCore::RefPtr<dtCore::View> view = new dtCore::View("");
   dtCore::DeltaWin::DeltaWinTraits traits;
   traits.height = 50;
   traits.width = 50;
   traits.x = 50;
   traits.y = 50;
   traits.showCursor = true;
   traits.fullScreen = false;

   dtCore::RefPtr<dtCore::DeltaWin> deltaWin =
      new dtCore::DeltaWin(traits);

   dtCore::RefPtr<StealthGM::ViewWindowWrapper> newViewWrapper =
      new StealthGM::ViewWindowWrapper(newViewOne, *view, *deltaWin);

   mInitCalled = false;
   mRemoveCalled = false;

   CPPUNIT_ASSERT(!newViewWrapper->GetInitCallback().valid());
   newViewWrapper->SetInitCallback(StealthGM::ViewWindowWrapper::OperationCallback(this, &ConfigObjectTests::InitCallbackTest));
   CPPUNIT_ASSERT(newViewWrapper->GetInitCallback().valid());

   CPPUNIT_ASSERT(!newViewWrapper->GetRemoveCallback().valid());
   newViewWrapper->SetRemoveCallback(StealthGM::ViewWindowWrapper::OperationCallback(this, &ConfigObjectTests::RemoveCallbackTest));
   CPPUNIT_ASSERT(newViewWrapper->GetRemoveCallback().valid());

   CPPUNIT_ASSERT(newViewWrapper->GetAttachToCamera() == NULL);
   CPPUNIT_ASSERT(dtUtil::Equivalent(newViewWrapper->GetAttachCameraRotation(), osg::Vec3(0.0, 0.0, 0.0), 0.01f));

   CPPUNIT_ASSERT(!newViewWrapper->IsAddedToApplication());

   dtCore::RefPtr<StealthGM::ViewerConfigComponent> vcc = new StealthGM::ViewerConfigComponent;
   dtCore::RefPtr<StealthGM::ViewWindowConfigObject> viewConfig =
      new StealthGM::ViewWindowConfigObject;

   vcc->AddConfigObject(*viewConfig);
   mGM->AddComponent(*vcc, dtGame::GameManager::ComponentPriority::NORMAL);

   viewConfig->CreateMainViewWindow(*mGM);
   CPPUNIT_ASSERT(!viewConfig->GetMainViewWindow().IsAddedToApplication());

   viewConfig->AddViewWindow(*newViewWrapper);
   StealthGM::ViewWindowWrapper* lookedUpViewWrapper = viewConfig->GetViewWindow(newViewOne);
   CPPUNIT_ASSERT(lookedUpViewWrapper != NULL);
   CPPUNIT_ASSERT(lookedUpViewWrapper == newViewWrapper.get());

   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT(viewConfig->GetMainViewWindow().IsAddedToApplication());
   CPPUNIT_ASSERT(newViewWrapper->IsAddedToApplication());
   CPPUNIT_ASSERT(mInitCalled);
   CPPUNIT_ASSERT(!mRemoveCalled);

   newViewWrapper->SetName(newViewTwo);
   viewConfig->UpdateViewName(newViewOne);

   CPPUNIT_ASSERT(viewConfig->GetViewWindow(newViewOne) == NULL);
   CPPUNIT_ASSERT(viewConfig->GetViewWindow(newViewTwo) == newViewWrapper.get());

   dtCore::ObserverPtr<dtCore::DeltaWin> windowOb = &newViewWrapper->GetWindow();
   dtCore::ObserverPtr<dtCore::Camera> cameraOb = newViewWrapper->GetView().GetCamera();
   dtCore::ObserverPtr<dtCore::View> viewOb = &newViewWrapper->GetView();

   CPPUNIT_ASSERT(newViewWrapper->GetView().GetScene() == &mGM->GetScene());

   CPPUNIT_ASSERT(windowOb.valid());
   CPPUNIT_ASSERT(cameraOb.valid());
   CPPUNIT_ASSERT(viewOb.valid());

   viewConfig->RemoveViewWindow(*newViewWrapper);
   //
   CPPUNIT_ASSERT_MESSAGE("Remove should be called on the next step, not now.", !mRemoveCalled);

   lookedUpViewWrapper = viewConfig->GetViewWindow(newViewTwo);
   CPPUNIT_ASSERT(lookedUpViewWrapper == NULL);

   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT(mRemoveCalled);

   dtCore::ObserverPtr<StealthGM::ViewWindowWrapper> viewWrapperOb = newViewWrapper.get();
   dtCore::ObserverPtr<osgViewer::GraphicsWindow> graphicsWindowOb = deltaWin->GetOsgViewerGraphicsWindow();
   newViewWrapper = NULL;
   deltaWin = NULL;
   view = NULL;

   //Added a step because application doesn't delete views until frame end.
   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(!viewWrapperOb.valid());
   CPPUNIT_ASSERT(!windowOb.valid());
   CPPUNIT_ASSERT(!cameraOb.valid());
   CPPUNIT_ASSERT(!graphicsWindowOb.valid());
   CPPUNIT_ASSERT(!viewOb.valid());
}

void ConfigObjectTests::TestPreferencesGeneralConfigObject()
{
   dtCore::RefPtr<StealthGM::PreferencesGeneralConfigObject> genConfig =
      new StealthGM::PreferencesGeneralConfigObject;

   CPPUNIT_ASSERT_MESSAGE("The default camera collision flag should be true",
      genConfig->GetEnableCameraCollision());

   CPPUNIT_ASSERT_MESSAGE("The default LOD scale should be 1",
      genConfig->GetLODScale() == 1);

   CPPUNIT_ASSERT_MESSAGE("The default performance mode should be DEFAULT",
      genConfig->GetPerformanceMode() == StealthGM::PreferencesGeneralConfigObject::PerformanceMode::DEFAULT);

   CPPUNIT_ASSERT_EQUAL_MESSAGE("The default attach point node name should be empty",
      genConfig->GetAttachPointNodeName(), std::string());

   osg::Vec3 defaultInitialAttachRot(0.0, 0.0, 0.0);
   CPPUNIT_ASSERT_MESSAGE("The attach rotation should default to 0,0,0.",
      dtUtil::Equivalent(genConfig->GetInitialAttachRotationHPR(), defaultInitialAttachRot, 0.01f));

   CPPUNIT_ASSERT_MESSAGE("Auto attach to entity should default to false..",
      !genConfig->GetShouldAutoAttachToEntity());

   CPPUNIT_ASSERT_EQUAL(false, genConfig->GetShowAdvancedOptions());

   genConfig->SetAttachMode(StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON);
   CPPUNIT_ASSERT_EQUAL(StealthGM::PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON,
      genConfig->GetAttachMode());
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

   genConfig->SetAttachPointNodeName("frank");
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The attach point node name should be frank",
      genConfig->GetAttachPointNodeName(), std::string("frank"));
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

   osg::Vec3 newInitialAttachRot(3.3, 9.7, 8.4);
   genConfig->SetInitialAttachRotationHPR(newInitialAttachRot);
   CPPUNIT_ASSERT_MESSAGE("The attach rotation should have changed.",
      dtUtil::Equivalent(genConfig->GetInitialAttachRotationHPR(), newInitialAttachRot, 0.01f));
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

   genConfig->SetShouldAutoAttachToEntity(true);
   CPPUNIT_ASSERT_MESSAGE("Auto attach to entity should now be true.",
      genConfig->GetShouldAutoAttachToEntity());
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

   genConfig->SetCameraCollision(false);
   CPPUNIT_ASSERT_EQUAL(false, genConfig->GetEnableCameraCollision());
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

   genConfig->SetLODScale(2.0f);
   CPPUNIT_ASSERT_EQUAL(2.0f, genConfig->GetLODScale());
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

   genConfig->SetPerformanceMode(StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS);
   CPPUNIT_ASSERT_EQUAL(StealthGM::PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS,
      genConfig->GetPerformanceMode());
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

   genConfig->SetShowAdvancedOptions(true);
   CPPUNIT_ASSERT_EQUAL(true, genConfig->GetShowAdvancedOptions());
   CPPUNIT_ASSERT(genConfig->IsUpdated());
   genConfig->SetIsUpdated(false);

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

   dtCore::RefPtr<SimCore::Tools::Binoculars> binocs;
   try
   {
      binocs = new SimCore::Tools::Binoculars(*mGM->GetApplication().GetCamera(), NULL);
   }
   catch(const CEGUI::Exception& e)
   {
      CPPUNIT_FAIL(e.getMessage().c_str() + '\n');
   }

   toolsConfig->SetBinocularsTool(binocs.get());
   CPPUNIT_ASSERT(toolsConfig->GetBinocularsTool() == binocs.get());

   binocs->Enable(false);

   toolsConfig->ApplyChanges(*mGM);

   CPPUNIT_ASSERT(binocs->GetZoomFactor() == toolsConfig->GetMagnification());
   CPPUNIT_ASSERT(binocs->GetShowDistance() == toolsConfig->GetShowDistanceToObject());
   CPPUNIT_ASSERT(binocs->GetShowReticle() == toolsConfig->GetShowBinocularImage());
   CPPUNIT_ASSERT(binocs->GetShowElevation() == toolsConfig->GetShowElevationOfObject());

   CPPUNIT_ASSERT(toolsConfig->GetAngleUnit() == SimCore::UnitOfAngle::DEGREE);
   toolsConfig->SetAngleUnit(SimCore::UnitOfAngle::MIL);
   CPPUNIT_ASSERT(toolsConfig->GetAngleUnit() == SimCore::UnitOfAngle::MIL);
   CPPUNIT_ASSERT(toolsConfig->IsUpdated());
   toolsConfig->SetIsUpdated(false);

   CPPUNIT_ASSERT(toolsConfig->GetLengthUnit() == SimCore::UnitOfLength::METER);
   toolsConfig->SetLengthUnit(SimCore::UnitOfLength::YARD);
   CPPUNIT_ASSERT(toolsConfig->GetLengthUnit() == SimCore::UnitOfLength::YARD);
   CPPUNIT_ASSERT(toolsConfig->IsUpdated());
   toolsConfig->SetIsUpdated(false);
   toolsConfig = NULL;
}

void ConfigObjectTests::TestPreferencesVisibilityConfigObject()
{
   dtCore::RefPtr<StealthGM::PreferencesVisibilityConfigObject> visConfig =
      new StealthGM::PreferencesVisibilityConfigObject;

   SimCore::Components::LabelOptions options = visConfig->GetLabelOptions();
   options.SetShowLabels(false);
   options.SetShowLabelsForBlips(false);
   options.SetShowLabelsForEntities(false);
   options.SetShowLabelsForPositionReports(false);
   options.SetMaxLabelDistance(10.0f);

   SimCore::VisibilityOptions& visOptions = visConfig->GetEntityOptions();
   SimCore::BasicVisibilityOptions basicOptions = visOptions.GetBasicOptions();

   visConfig->SetLabelOptions(options);
   CPPUNIT_ASSERT(visConfig->IsUpdated());

   visConfig->SetIsUpdated(false);
   visConfig->SetEntityOptions(visOptions);
   CPPUNIT_ASSERT(visConfig->IsUpdated());

   SimCore::Components::LabelOptions options2 = visConfig->GetLabelOptions();
   CPPUNIT_ASSERT(options == options2);

   dtCore::RefPtr<dtGame::GameManager> gm = new dtGame::GameManager(*GetGlobalApplication().GetScene());

   GetGlobalApplication().GetWindow()->GetOsgViewerGraphicsWindow()->makeCurrent();

   dtCore::RefPtr<StealthGM::StealthHUD> hud = new StealthGM::StealthHUD(GetGlobalApplication().GetWindow());
   hud->SetupGUI(*new SimCore::Components::HUDGroup("hello"), 50, 50 );
   gm->AddComponent(*hud, dtGame::GameManager::ComponentPriority::NORMAL);

   dtCore::RefPtr<SimCore::Components::ViewerMessageProcessor> vmp = new SimCore::Components::ViewerMessageProcessor;
   gm->AddComponent(*vmp, dtGame::GameManager::ComponentPriority::HIGHEST);

   SimCore::Components::LabelOptions optionsApplied = hud->GetLabelManager().GetOptions();
   CPPUNIT_ASSERT(options != optionsApplied);

   visConfig->ApplyChanges(*gm);

   optionsApplied = hud->GetLabelManager().GetOptions();

   CPPUNIT_ASSERT(options == optionsApplied);
   CPPUNIT_ASSERT(&visConfig->GetEntityOptions() == &vmp->GetVisibilityOptions());

   // These should default to the same value.
   CPPUNIT_ASSERT_EQUAL(visConfig->GetBFGCloseTops(), SimCore::Actors::BattlefieldGraphicsActorProxy::GetGlobalEnableTopGeometry());
   bool oldValue = visConfig->GetBFGCloseTops();
   CPPUNIT_ASSERT(!visConfig->IsUpdated());
   visConfig->SetBFGCloseTops(!oldValue);
   CPPUNIT_ASSERT_EQUAL(!oldValue, visConfig->GetBFGCloseTops());
   CPPUNIT_ASSERT_EQUAL(oldValue, SimCore::Actors::BattlefieldGraphicsActorProxy::GetGlobalEnableTopGeometry());
   CPPUNIT_ASSERT(visConfig->IsUpdated());
   visConfig->ApplyChanges(*gm);
   CPPUNIT_ASSERT_EQUAL(!oldValue, SimCore::Actors::BattlefieldGraphicsActorProxy::GetGlobalEnableTopGeometry());
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
   gm->SetApplication(GetGlobalApplication());
   dtCore::RefPtr<SimCore::Components::WeatherComponent> weatherComponent =
      new SimCore::Components::WeatherComponent;

   gm->AddComponent(*weatherComponent, dtGame::GameManager::ComponentPriority::NORMAL);

   dtCore::RefPtr<SimCore::Actors::DayTimeActorProxy> dayTimeProxy;
   gm->CreateActor(*SimCore::Actors::EntityActorRegistry::DAYTIME_ACTOR_TYPE, dayTimeProxy);

   dtCore::RefPtr<SimCore::Actors::UniformAtmosphereActorProxy> atmosphereProxy;
   gm->CreateActor(*SimCore::Actors::EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE, atmosphereProxy);

   dtCore::RefPtr<SimCore::Actors::EphemerisEnvironmentActorProxy> envProxy;
   gm->CreateActor(*SimCore::Actors::EntityActorRegistry::ENVIRONMENT_ACTOR_TYPE, envProxy);

   gm->AddActor(*dayTimeProxy, false, false);
   gm->AddActor(*atmosphereProxy, false, false);

   try
   {
      gm->SetEnvironmentActor(envProxy.get());
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.ToString());
   }

   dtCore::AppSleep(10U);
   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(weatherComponent->GetDayTimeActor() != NULL);
   CPPUNIT_ASSERT(weatherComponent->GetAtmosphereActor() != NULL);

   CPPUNIT_ASSERT(weatherComponent->GetEphemerisEnvironment() != NULL);

   gm = NULL;
}

