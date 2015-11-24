/* -*-c++-*-
* Simulation Core - ToolTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2005-2008, Alion Science and Technology Corporation
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
#include <dtGame/gamemanager.h>
#include <dtGame/exceptionenum.h>
#include <SimCore/Tools/Binoculars.h>
#include <SimCore/Tools/LaserRangeFinder.h>
#include <SimCore/Tools/Compass.h>
#include <SimCore/Tools/GPS.h>
#include <CEGUI/CEGUIVersion.h>
#include <dtGUI/gui.h>
#include <dtABC/application.h>
#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/deltawin.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/datapathutils.h>
#include <dtCore/actortype.h>
#include <dtCore/enginepropertytypes.h>
#include <SimCore/Actors/PlayerActor.h>

#include <UnitTestMain.h>

#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>


using dtCore::RefPtr;

class ToolTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(ToolTests);

      CPPUNIT_TEST(TestBinoculars);
      CPPUNIT_TEST(TestLRF);
      CPPUNIT_TEST(TestCompass);
      CPPUNIT_TEST(TestGPS);
      CPPUNIT_TEST(TestPlayerProperty);
      CPPUNIT_TEST(TestEnableToolProperty);

   CPPUNIT_TEST_SUITE_END();

public:

   void setUp();
   void tearDown();
   void TestBinoculars();
   void TestLRF();
   void TestCompass();
   void TestGPS();
   void TestPlayerProperty();
   void TestEnableToolProperty();

   ToolTests()
   {

   }
   ~ToolTests()
   {

   }

private:

   RefPtr<dtGame::GameManager> mGM;
   RefPtr<dtGUI::GUI> mGUI;
   // The constructor call to the GUI member assumes that an
   // application has been instantiated.
   RefPtr<dtABC::Application> mApp;
   RefPtr<SimCore::Actors::PlayerActor> mPlayerActor;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ToolTests);

void ToolTests::setUp()
{
   // Initialize CEGUI
   try
   {
      dtCore::System::GetInstance().Start();
      mApp = &GetGlobalApplication();

      mGM = new dtGame::GameManager(*mApp->GetScene());
      mGM->SetApplication( *mApp );

      dtCore::System::GetInstance().Step();

      mGUI = &GetGlobalGUI();
   }
   catch(const dtUtil::Exception& e)
   {
      CPPUNIT_FAIL(e.What());
   }

   try
   {
      const dtCore::ActorType *type = mGM->FindActorType("Player Actor", "Player Actor");
      CPPUNIT_ASSERT(type != NULL);
      RefPtr<dtCore::ActorProxy> proxy = mGM->CreateActor(*type);
      CPPUNIT_ASSERT(proxy.valid());
      mPlayerActor = dynamic_cast<SimCore::Actors::PlayerActor*>(proxy->GetDrawable());
      CPPUNIT_ASSERT(mPlayerActor.valid());
      mGM->AddActor(mPlayerActor->GetGameActorProxy(), false, false);
   }
   catch (const dtUtil::Exception& ex)
   {
      ex.LogException(dtUtil::Log::LOG_ERROR);
   }
}

void ToolTests::tearDown()
{
   if (mGM.valid())
   {
      dtCore::System::GetInstance().Stop();
      mGM->DeleteAllActors(true);
   }

   mGM = NULL;
   mPlayerActor = NULL;

   mGUI = NULL;
   mApp = NULL;
}

void ToolTests::TestBinoculars()
{
   RefPtr<SimCore::Tools::Binoculars> binos;
   try
   {
      binos = new SimCore::Tools::Binoculars(*mApp->GetCamera(), NULL);
      binos->SetPlayerActor(mPlayerActor.get());
   }
   catch(const CEGUI::Exception& e)
   {
      CPPUNIT_FAIL(e.getMessage().c_str());
   }

   CPPUNIT_ASSERT(binos->GetShowDistance());
   CPPUNIT_ASSERT(binos->GetShowReticle());
   CPPUNIT_ASSERT(binos->GetShowElevation());
   CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, binos->GetZoomFactor(), 0.1f);

   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled by default", !binos->IsEnabled());
   binos->Enable(true);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be enabled", binos->IsEnabled());
   binos->Enable(false);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled", !binos->IsEnabled());

}

void ToolTests::TestLRF()
{
   RefPtr<SimCore::Tools::LaserRangeFinder> lrf;
   try
   {
      lrf = new SimCore::Tools::LaserRangeFinder(*mApp->GetCamera(), NULL);
      lrf->SetPlayerActor(mPlayerActor.get());
   }
   catch(CEGUI::Exception& e)
   {
      CPPUNIT_FAIL(e.getMessage().c_str());
   }

   CPPUNIT_ASSERT_MESSAGE("Laser range finder should be disabled by default", !lrf->IsEnabled());
   lrf->Enable(true);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be enabled", lrf->IsEnabled());
   lrf->Enable(false);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled", !lrf->IsEnabled());

   float degrees = lrf->CalculateDegrees(90, 0);
   CPPUNIT_ASSERT_MESSAGE("The degrees should be 0 since a distance of 0 is not valid", degrees == 0.0f);
}

void ToolTests::TestCompass()
{
   RefPtr<SimCore::Tools::Compass> compass;
   try
   {
      compass = new SimCore::Tools::Compass(NULL, *dtABC::Application::GetInstance(0)->GetCamera(), 0.0f);
      compass->SetPlayerActor(mPlayerActor.get());
   }
   catch(CEGUI::Exception& e)
   {
      CPPUNIT_FAIL(e.getMessage().c_str());
   }

   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled by default", !compass->IsEnabled());
   compass->Enable(true);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be enabled", compass->IsEnabled());
   compass->Enable(false);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled", !compass->IsEnabled());
}

void ToolTests::TestGPS()
{
   RefPtr<SimCore::Tools::GPS> gps;
   try
   {
      gps = new SimCore::Tools::GPS(NULL);
      gps->SetPlayerActor(mPlayerActor.get());
   }
   catch(CEGUI::Exception& e)
   {
      CPPUNIT_FAIL(e.getMessage().c_str());
   }

   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled by default", !gps->IsEnabled());
   gps->Enable(true);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be enabled", gps->IsEnabled());
   gps->Enable(false);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled", !gps->IsEnabled());
}


void ToolTests::TestPlayerProperty()
{
   RefPtr<SimCore::Tools::Binoculars> binos = new SimCore::Tools::Binoculars(*mApp->GetCamera(), NULL);
   CPPUNIT_ASSERT(binos->GetPlayerActor() == NULL);
   binos->SetPlayerActor(mPlayerActor.get());
   CPPUNIT_ASSERT(binos->GetPlayerActor() == mPlayerActor.get());
}

void ToolTests::TestEnableToolProperty()
{
   /*mPlayerActor->AddTool(*binos, SimCore::MessageType::BINOCULARS);
   mPlayerActor->AddTool(*compass, SimCore::MessageType::COMPASS);
   mPlayerActor->AddTool(*lrf, SimCore::MessageType::LASER_RANGE_FINDER);
   mPlayerActor->AddTool(*gps, SimCore::MessageType::GPS);

   binos->SetPlayerActor(mPlayerActor.get());
   compass->SetPlayerActor(mPlayerActor.get());
   lrf->SetPlayerActor(mPlayerActor.get());
   gps->SetPlayerActor(mPlayerActor.get());
   */
   //TODO this needs to be reworked because all the enable code is in VFSTInputComponent

   /*mPlayerActor->SetEnabledTool(SimCore::MessageType::BINOCULARS);
   CPPUNIT_ASSERT_MESSAGE("The binoculars should be enabled", binos->IsEnabled());
   CPPUNIT_ASSERT_MESSAGE("The return value should be binoculars", mPlayerActor->GetEnabledTool() == SimCore::MessageType::BINOCULARS);
   mPlayerActor->SetEnabledTool(SimCore::MessageType::NO_TOOL);
   CPPUNIT_ASSERT_MESSAGE("The binoculars should not be enabled", !binos->IsEnabled());
   CPPUNIT_ASSERT_MESSAGE("The return value should be no tool", mPlayerActor->GetEnabledTool() == SimCore::MessageType::NO_TOOL);
   mPlayerActor->SetEnabledTool(SimCore::MessageType::COMPASS);
   mPlayerActor->SetEnabledTool(SimCore::MessageType::LASER_RANGE_FINDER);
   CPPUNIT_ASSERT_MESSAGE("The compass should no longer be enabled", !compass->IsEnabled());
   CPPUNIT_ASSERT_MESSAGE("The LRF should be enabled", lrf->IsEnabled());
   CPPUNIT_ASSERT_MESSAGE("The return value should be the LRF", mPlayerActor->GetEnabledTool() == SimCore::MessageType::LASER_RANGE_FINDER);

   SimCore::Actors::PlayerActorProxy &pap = static_cast<SimCore::Actors::PlayerActorProxy&>(mPlayerActor->GetGameActorProxy());
   dtCore::ActorProperty *ap = pap.GetProperty("Enabled Tool");
   CPPUNIT_ASSERT_MESSAGE("The Enabled Tool property should not be NULL", ap != NULL);
   dtCore::AbstractEnumActorProperty *aep = dynamic_cast<dtCore::AbstractEnumActorProperty*>(ap);
   CPPUNIT_ASSERT(aep != NULL);
   aep->SetEnumValue(SimCore::MessageType::GPS);
   CPPUNIT_ASSERT_MESSAGE("GetEnumValue should return GPS", aep->GetEnumValue() == SimCore::MessageType::GPS);
   CPPUNIT_ASSERT_MESSAGE("The player should have the correct tool enabled", mPlayerActor->GetEnabledTool() == SimCore::MessageType::GPS);
   mPlayerActor->SetEnabledTool(SimCore::MessageType::NO_TOOL);
   CPPUNIT_ASSERT_MESSAGE("The property should have updated from the player", aep->GetEnumValue() == SimCore::MessageType::NO_TOOL);

   mPlayerActor->SetEnabledTool(const_cast<SimCore::MessageType&>(SimCore::MessageType::ATTACH_TO_ACTOR));
   CPPUNIT_ASSERT_MESSAGE("Trying to set an invalid tool should not have worked", mPlayerActor->GetEnabledTool() == SimCore::MessageType::NO_TOOL);
   mPlayerActor->AddTool(*binos, const_cast<SimCore::MessageType&>(SimCore::MessageType::STEALTH_ACTOR_FOV));
   SimCore::Tools::Tool *tool = mPlayerActor->GetTool(const_cast<SimCore::MessageType&>(SimCore::MessageType::STEALTH_ACTOR_FOV));
   CPPUNIT_ASSERT_MESSAGE("The invalid tool should be NULL", tool == NULL);
   */
}
