/* -*-c++-*-
* Simulation Core - LabelManagerTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
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
* David Guthrie
*/

#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <dtCore/camera.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/deltawin.h>
#include <dtCore/transform.h>
#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/exception.h>
#include <dtUtil/datapathutils.h>
#include <SimCore/Components/StealthHUDElements.h>
#include <SimCore/Components/LabelManager.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <osg/io_utils>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Shape>

#include <dtABC/application.h>

#include <UnitTestMain.h>

#include <CEGUI/CEGUIVersion.h>


//////////////////////////////////////////////////////////////
// UNIT TESTS
//////////////////////////////////////////////////////////////
class LabelManagerTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(LabelManagerTests);

      CPPUNIT_TEST(TestHUDLabel);
      CPPUNIT_TEST(TestCreateUpdate);
      CPPUNIT_TEST(TestLabelOptions);
      CPPUNIT_TEST(TestLabelColor);

   CPPUNIT_TEST_SUITE_END();


   public:

      //////////////////////////////////////////////////////////////
      void setUp()
      {
         // A window & camera are needed for GUI rendering
         mApp = &GetGlobalApplication();

         mGM = new dtGame::GameManager(*mApp->GetScene());
         mGM->SetApplication(*mApp);

#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
         mGUI = &GetGlobalCEGUIDrawable();
         mApp->GetScene()->AddDrawable(mGUI.get());
#else
         mGUI = &GetGlobalGUI();
#endif

         mLabelManager = new SimCore::Components::LabelManager();
         mLabelManager->SetGameManager(mGM.get());
         CPPUNIT_ASSERT(mLabelManager->GetGameManager() == mGM.get());

         dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
         dtCore::System::GetInstance().Start();

         setupCEGUI();

         mLabelManager->SetGUILayer(mMainGUIWindow.get());
         CPPUNIT_ASSERT(mLabelManager->GetGUILayer() == mMainGUIWindow.get());

         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, mPlatform1);
         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, mPlatform2);

         CPPUNIT_ASSERT(mPlatform1.valid());
         CPPUNIT_ASSERT(mPlatform2.valid());

         const std::string platName1("goober");
         const std::string platName2("gobstopper");
         mPlatform1->SetName(platName1);
         mPlatform2->SetName(platName2);

         mGM->AddActor(*mPlatform1, false, false);
         mGM->AddActor(*mPlatform2, false, false);

         //Must add some volume to the actors or the label code won't think the actor is visible.
         dtCore::RefPtr<osg::Geode> geode = new osg::Geode();
         osg::Box* box = new osg::Box(osg::Vec3(0.0f, 0.0f, 0.0f), 1.1f, 1.3f, 1.1f);
         osg::ShapeDrawable* sd = new osg::ShapeDrawable(box, NULL);
         geode->addDrawable(sd);

         mPlatform1->GetDrawable()->GetOSGNode()->asGroup()->addChild(geode.get());
         mPlatform2->GetDrawable()->GetOSGNode()->asGroup()->addChild(geode.get());

         dtCore::Transform xform;
         dtCore::Camera* camera = GetGlobalApplication().GetCamera();
         //Put the camera a the origin looking down the positive Y axis.
         xform.SetTranslation(0.0, 0.0, 0.0);
         xform.SetRotation(0.0, 0.0, 0.0);
         camera->SetTransform(xform);
         camera->UpdateViewMatrixFromTransform();
      }

      //////////////////////////////////////////////////////////////
      void tearDown()
      {
         dtCore::System::GetInstance().Stop();

         mPlatform1 = NULL;
         mPlatform2 = NULL;

         if(mGM.valid())
            mGM->DeleteAllActors(true);

         mLabelManager = NULL;
         mMainGUIWindow = NULL;


         mGM = NULL;
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
         if (mGUI.valid())
            mGUI->Emancipate();
#endif
         mGUI = NULL;
         mApp = NULL;
      }

      //////////////////////////////////////////////////////////////
      void setupCEGUI()
      {
         mMainGUIWindow = new SimCore::Components::HUDElement( "LabelLayer",
                  SimCore::Components::HUDElement::DEFAULT_BLANK_TYPE );
         mMainGUIWindow->SetDeleteWindowOnDestruct(true);
         //CEGUI::System::getSingleton().setGUISheet(mMainGUIWindow->GetCEGUIWindow());
      }

      void TestHUDLabel()
      {
         dtCore::RefPtr<SimCore::Components::HUDLabel> label = new SimCore::Components::HUDLabel(
                  "TestLabel", "Label/EntityLabel");

         std::string value("Lorem Ipsum");
         CPPUNIT_ASSERT_MESSAGE("HUD Label should NOT have a text value",
            label->GetText().empty());

         label->SetText(value);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("HUD Label should have a NEW text value",
            value, label->GetText());

         CPPUNIT_ASSERT_MESSAGE("HUD Label should NOT have a line2 value",
            label->GetLine2().empty());

         label->SetLine2(value);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("HUD Label should have a NEW line2 value",
            value, label->GetLine2());

         osg::Vec4 tempColor;
         label->GetColor(tempColor);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("HUD Label should be white.",
            osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f), tempColor);

         osg::Vec3 testColor3(0.3f, 0.8f, 0.9f);
         osg::Vec4 test3AsVec4(testColor3, 1.0f);
         label->SetColor(testColor3);
         label->GetColor(tempColor);
         CPPUNIT_ASSERT_MESSAGE("HUD Label should be the new color.",
                  dtUtil::Equivalent(test3AsVec4, tempColor, 0.01f));

         osg::Vec4 testColor(1.0f, 0.7f, 0.8f, 0.3f);
         label->SetColor(testColor);
         label->GetColor(tempColor);
         CPPUNIT_ASSERT_MESSAGE("HUD Label should be the new color.",
            dtUtil::Equivalent(testColor, tempColor, 0.01f));

         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The alpha should match the w of the color", testColor.w(), label->GetAlpha(), 0.01f);
         float newAlpha = 0.21f;
         label->SetAlpha(newAlpha);
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The alpha should have changed", newAlpha, label->GetAlpha(), 0.01f);

         testColor.w() = newAlpha;
         label->GetColor(tempColor);
         CPPUNIT_ASSERT_MESSAGE( "HUD Label color should match the new alpha.",
                  dtUtil::Equivalent(testColor, tempColor, 0.01f));

         const std::string colorPropVal = label->GetProperty(SimCore::Components::HUDLabel::PROPERTY_COLOR);
         CEGUI::colour cegguiColor(CEGUI::PropertyHelper::stringToColour(colorPropVal));
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The red should have changed", testColor.x(), cegguiColor.getRed(), 0.01f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The green should have changed", testColor.y(), cegguiColor.getGreen(), 0.01f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The blue should have changed", testColor.z(), cegguiColor.getBlue(), 0.01f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The alpha should have changed", testColor.w(), cegguiColor.getAlpha(), 0.01f);

         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The z depth should default to 0.", 0.0f, label->GetZDepth(), 0.01f);
         label->SetZDepth(newAlpha);
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The z depth should have changed.", newAlpha, label->GetZDepth(), 0.01f);

      }

      void TestCreateUpdate()
      {
         dtCore::Transform xform;

         SimCore::Actors::Platform* platActor = NULL;

         //In front of the camera
         osg::Vec3 plat1Pos(0.0, 5.0, 0.0);
         mPlatform1->GetDrawable(platActor);
         xform.SetTranslation(plat1Pos);
         platActor->SetTransform(xform);

         //behind the camera
         osg::Vec3 plat2Pos(0.0, -10.0, 0.0);
         mPlatform2->GetDrawable(platActor);
         xform.SetTranslation(plat2Pos);
         platActor->SetTransform(xform);

         mLabelManager->Update(0.016);

         //This tests both GetOrCreateLabel and AddLabel.
         dtCore::RefPtr<SimCore::Components::HUDLabel> label = mLabelManager->GetOrCreateLabel(*mPlatform1);
         dtCore::RefPtr<SimCore::Components::HUDLabel> label2 = mLabelManager->GetOrCreateLabel(*mPlatform2);
         CPPUNIT_ASSERT(label.valid());
         CPPUNIT_ASSERT(label2.valid());

         CPPUNIT_ASSERT(label->GetCEGUIWindow()->getParent() == mLabelManager->GetGUILayer()->GetCEGUIWindow());
         CPPUNIT_ASSERT(label2->GetCEGUIWindow()->getParent() == mLabelManager->GetGUILayer()->GetCEGUIWindow());

         dtCore::RefPtr<SimCore::Components::HUDLabel> label1Next = mLabelManager->GetOrCreateLabel(*mPlatform1);
         dtCore::RefPtr<SimCore::Components::HUDLabel> label2Next = mLabelManager->GetOrCreateLabel(*mPlatform2);
         CPPUNIT_ASSERT_MESSAGE("the label manager should return the same label because it should be in the cache", label == label1Next);
         CPPUNIT_ASSERT_MESSAGE("the label manager should not return the same label since the label was not in view, and is not cached",
                  label2 != label2Next);

         dtCore::ObserverPtr<SimCore::Components::HUDLabel> label2DeleteCheck = label2.get();
         label2 = NULL;
         CPPUNIT_ASSERT_MESSAGE("Labels should get deleted when all of their references go away.", !label2DeleteCheck.valid());


         //Now put the label out of range, but in front of the camera
         plat1Pos.set(0.0, mLabelManager->GetOptions().GetMaxLabelDistance() + 50.0, 0.0);
         mPlatform1->GetDrawable(platActor);
         xform.SetTranslation(plat1Pos);
         platActor->SetTransform(xform);

         mLabelManager->Update(0.016);

         label = mLabelManager->GetOrCreateLabel(*mPlatform1);
         label1Next = mLabelManager->GetOrCreateLabel(*mPlatform1);
         CPPUNIT_ASSERT_MESSAGE("the label manager should not have cached a label for platform1, because it is out of range.", label != label1Next);
      }

      void TestLabelOptions()
      {
         SimCore::Components::LabelOptions options;
         mLabelManager->SetOptions(options);
         SimCore::Components::LabelOptions gottenOptions = mLabelManager->GetOptions();
         CPPUNIT_ASSERT(options == gottenOptions);

         CPPUNIT_ASSERT_DOUBLES_EQUAL(500.0f, options.GetMaxLabelDistance(), 0.01f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(500.0f * 500.0f, options.GetMaxLabelDistance2(), 0.01f);
         float newValue = 23.23f;
         options.SetMaxLabelDistance(newValue);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(newValue, options.GetMaxLabelDistance(), 0.01f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(newValue * newValue, options.GetMaxLabelDistance2(), 0.01f);

         CPPUNIT_ASSERT_EQUAL(false, options.ShowDamageState());
         options.SetShowDamageState(true);
         CPPUNIT_ASSERT_EQUAL(true,  options.ShowDamageState());

         CPPUNIT_ASSERT_EQUAL(true, options.ShowLabels());
         options.SetShowLabels(false);
         CPPUNIT_ASSERT_EQUAL(false,  options.ShowLabels());

         CPPUNIT_ASSERT_EQUAL(true, options.ShowLabelsForEntities());
         options.SetShowLabelsForEntities(false);
         CPPUNIT_ASSERT_EQUAL(false,  options.ShowLabelsForEntities());

         CPPUNIT_ASSERT_EQUAL(true, options.ShowLabelsForBlips());
         options.SetShowLabelsForBlips(false);
         CPPUNIT_ASSERT_EQUAL(false,  options.ShowLabelsForBlips());

         CPPUNIT_ASSERT_EQUAL(true, options.ShowLabelsForPositionReports());
         options.SetShowLabelsForPositionReports(false);
         CPPUNIT_ASSERT_EQUAL(false,  options.ShowLabelsForPositionReports());

         gottenOptions = mLabelManager->GetOptions();
         CPPUNIT_ASSERT(options != gottenOptions);
         mLabelManager->SetOptions(options);
         gottenOptions = mLabelManager->GetOptions();
         CPPUNIT_ASSERT(options == gottenOptions);
      }

      void TestLabelColor()
      {
         std::vector<SimCore::Actors::BaseEntityActorProxy::ForceEnum*> forces = SimCore::Actors::BaseEntityActorProxy::ForceEnum::EnumerateType();

         for (size_t i = 0; i < forces.size(); ++i)
         {
            TestSingleLabelColor(*forces[i]);
         }
      }

   private:
      void TestSingleLabelColor(SimCore::Actors::BaseEntityActorProxy::ForceEnum& force)
      {
         dtCore::RefPtr<SimCore::Components::HUDLabel> label = mLabelManager->GetOrCreateLabel(*mPlatform1);
         SimCore::Actors::Platform* platform;
         mPlatform1->GetDrawable(platform);

         platform->SetForceAffiliation(force);
         mLabelManager->AssignLabelColor(*mPlatform1, *label);
         osg::Vec4 color;
         label->GetColor(color);
         std::string colorPropVal = label->GetProperty(force.GetName() + "_COLOR");
         CEGUI::colour ceguiColor(CEGUI::PropertyHelper::stringToColour(colorPropVal));
         osg::Vec4 expectedColor(ceguiColor.getRed(), ceguiColor.getGreen(), ceguiColor.getBlue(), ceguiColor.getAlpha());
         CPPUNIT_ASSERT(dtUtil::Equivalent(expectedColor, color, osg::Vec4::value_type(0.01f)));
      }

      dtCore::RefPtr<dtGame::GameManager> mGM;
      dtCore::RefPtr<dtABC::Application> mApp;
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
      dtCore::RefPtr<dtGUI::CEUIDrawable> mGUI;
#else
      dtCore::RefPtr<dtGUI::GUI> mGUI;
#endif
      dtCore::RefPtr<SimCore::Components::LabelManager> mLabelManager;
      dtCore::RefPtr<SimCore::Components::HUDElement> mMainGUIWindow;

      dtCore::RefPtr<SimCore::Actors::PlatformActorProxy> mPlatform1;
      dtCore::RefPtr<SimCore::Actors::PlatformActorProxy> mPlatform2;
};

CPPUNIT_TEST_SUITE_REGISTRATION(LabelManagerTests);


