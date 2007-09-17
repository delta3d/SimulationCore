/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * @author Eddie Johnson
 */
#include <cppunit/extensions/HelperMacros.h>
#include <dtGame/gamemanager.h> 
#include <dtGame/exceptionenum.h>
#include <SimCore/Tools/Binoculars.h>
#include <SimCore/Tools/LaserRangeFinder.h>
#include <SimCore/Tools/Compass.h>
#include <SimCore/Tools/GPS.h>
#include <dtGUI/ceuidrawable.h>
#include <dtABC/application.h>
#include <dtCore/globals.h>
#include <dtCore/system.h>
#include <dtCore/deltawin.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <dtDAL/actortype.h>
#include <dtDAL/enginepropertytypes.h>
#include <SimCore/Actors/PlayerActor.h>

#include <CEGUIUtils.h>

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
   RefPtr<dtGUI::CEUIDrawable> mGUI;
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
      mApp = new dtABC::Application;
      mGM  = new dtGame::GameManager(*mApp->GetScene());
      mApp->GetWindow()->SetPosition(0, 0, 50, 50);
      mApp->Config();
      mGM->SetApplication(*mApp);
      dtCore::System::GetInstance().Step();

      mGUI = new dtGUI::CEUIDrawable(mApp->GetWindow());
      SimCore::SetupCEGUI();
   }
   catch(const dtUtil::Exception &e) 
   {
      CPPUNIT_FAIL(e.What());
   }

   try
   {
      const dtDAL::ActorType *type = mGM->FindActorType("Player Actor", "Player Actor");
      CPPUNIT_ASSERT(type != NULL);
      RefPtr<dtDAL::ActorProxy> proxy = mGM->CreateActor(*type);
      CPPUNIT_ASSERT(proxy.valid());
      mPlayerActor = dynamic_cast<SimCore::Actors::PlayerActor*>(proxy->GetActor());
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
   if (mGUI.valid())
   {
      mGUI->ShutdownGUI();
   }
   
   mApp         = NULL;
   mGM          = NULL;
   mGUI         = NULL;
   mPlayerActor = NULL;
}

void ToolTests::TestBinoculars()
{
   RefPtr<SimCore::Tools::Binoculars> binos;
   try
   {
      binos = new SimCore::Tools::Binoculars(*mApp->GetCamera(), NULL);
      binos->SetPlayerActor(mPlayerActor.get()); 
   }
   catch(CEGUI::Exception &e) 
   {
      CPPUNIT_FAIL(e.getMessage().c_str() + '\n');
   }

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
   catch(CEGUI::Exception &e) 
   {
      CPPUNIT_FAIL(e.getMessage().c_str() + '\n');
   }

   CPPUNIT_ASSERT_MESSAGE("Laser range finder should be disabled by default", !lrf->IsEnabled());
   lrf->Enable(true);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be enabled", lrf->IsEnabled());
   lrf->Enable(false);
   CPPUNIT_ASSERT_MESSAGE("Binoculars should be disabled", !lrf->IsEnabled());

   float mils = lrf->CalculateMils(90, 0);
   CPPUNIT_ASSERT_MESSAGE("The mils should be 0 since a distance of 0 is not valid", mils == 0.0f);
}

void ToolTests::TestCompass()
{
   RefPtr<SimCore::Tools::Compass> compass;
   try
   {
      compass = new SimCore::Tools::Compass(NULL, 0.0f); 
      compass->SetPlayerActor(mPlayerActor.get()); 
   }
   catch(CEGUI::Exception &e) 
   {
      CPPUNIT_FAIL(e.getMessage().c_str() + '\n');
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
   catch(CEGUI::Exception &e) 
   {
      CPPUNIT_FAIL(e.getMessage().c_str() + '\n');
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
   dtDAL::ActorProperty *ap = pap.GetProperty("Enabled Tool");
   CPPUNIT_ASSERT_MESSAGE("The Enabled Tool property should not be NULL", ap != NULL);
   dtDAL::AbstractEnumActorProperty *aep = dynamic_cast<dtDAL::AbstractEnumActorProperty*>(ap);
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
