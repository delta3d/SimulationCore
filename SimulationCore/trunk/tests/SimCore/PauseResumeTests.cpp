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
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtGame/gamemanager.h> 
#include <dtGame/actorupdatemessage.h>
#include <dtGame/defaultmessageprocessor.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/BaseEntity.h>

#if (defined (WIN32) || defined (_WIN32) || defined (__WIN32__))
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using dtCore::RefPtr;

class PauseResumeTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(PauseResumeTests);

      CPPUNIT_TEST(TestPauseAndResume);
      CPPUNIT_TEST(TestRewind);

   CPPUNIT_TEST_SUITE_END();

public:

   void setUp();
   void tearDown();

   PauseResumeTests()
   {

   }
   ~PauseResumeTests()
   {

   }

   void TestPauseAndResume();
   void TestRewind();

private:

   RefPtr<dtGame::GameManager> mGM;
   RefPtr<dtGame::MachineInfo> mMachineInfo;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PauseResumeTests);

void PauseResumeTests::setUp()
{
   try
   {
      dtCore::System::GetInstance().Start();
      RefPtr<dtCore::Scene> scene = new dtCore::Scene;
      mGM = new dtGame::GameManager(*scene);
      mMachineInfo = new dtGame::MachineInfo;
   }
   catch (const dtUtil::Exception& ex)
   {
      ex.LogException(dtUtil::Log::LOG_ERROR);
   }
}

void PauseResumeTests::tearDown()
{
   dtCore::System::GetInstance().Stop();

   mGM->DeleteAllActors(true);
   mGM = NULL;
   mMachineInfo = NULL;
}

void PauseResumeTests::TestPauseAndResume()
{
   mGM->AddComponent(*new dtGame::DefaultMessageProcessor, dtGame::GameManager::ComponentPriority::HIGHEST);
   mGM->AddComponent(*new dtGame::DeadReckoningComponent, dtGame::GameManager::ComponentPriority::HIGHER);

   RefPtr<dtGame::GameActorProxy> proxy = mGM->CreateRemoteGameActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE);
   CPPUNIT_ASSERT(proxy.valid());
   SimCore::Actors::BaseEntityActorProxy *gap = dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>(proxy.get());
   CPPUNIT_ASSERT(gap != NULL);
   mGM->AddActor(*gap, true, false);

   mGM->SetPaused(true);
   CPPUNIT_ASSERT(mGM->IsPaused());

   static_cast<dtDAL::Vec3ActorProperty*>(gap->GetProperty("Velocity Vector"))->SetValue(osg::Vec3(0, 10, 0));
   static_cast<dtDAL::Vec3ActorProperty*>(gap->GetProperty("Last Known Translation"))->SetValue(osg::Vec3(0, 0, 0));
   static_cast<dtDAL::Vec3ActorProperty*>(gap->GetProperty("Last Known Rotation"))->SetValue(osg::Vec3(0, 0, 0));
   static_cast<dtDAL::EnumActorProperty<dtGame::DeadReckoningAlgorithm>*>(gap->GetProperty("Dead Reckoning Algorithm"))->SetValue(dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);
   osg::Vec3 oldPos = gap->GetTranslation();

   //Step to init the dr stuff, since sim time change should be 0.
   dtCore::System::GetInstance().Step();
   SLEEP(5);
   dtCore::System::GetInstance().Step();

   osg::Vec3 newPos = gap->GetTranslation();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The system is paused, the new translation should be equal to the old translation", newPos, oldPos);
   
   mGM->SetPaused(false);
   CPPUNIT_ASSERT(!mGM->IsPaused());
   
   SLEEP(5);
   dtCore::System::GetInstance().Step();

   newPos = gap->GetTranslation();
   CPPUNIT_ASSERT_MESSAGE("The system is unpaused, the proxy should now be dead reckoning.", newPos != oldPos);

   mGM->SetPaused(true);
   CPPUNIT_ASSERT(mGM->IsPaused());
   oldPos = newPos;

   SLEEP(5);
   dtCore::System::GetInstance().Step();

   newPos = gap->GetTranslation();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The system is paused, the new translation should equal the old translation", newPos, oldPos);
}

void PauseResumeTests::TestRewind()
{

}
