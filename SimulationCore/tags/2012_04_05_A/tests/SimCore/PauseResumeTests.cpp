/* -*-c++-*-
* Simulation Core - PauseResumeTests (.h & .cpp) - Using 'The MIT License'
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
#include <dtGame/actorupdatemessage.h>
#include <dtGame/defaultmessageprocessor.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/timer.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/BaseEntity.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

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
      mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
      mGM->SetApplication(GetGlobalApplication());
      mMachineInfo = new dtGame::MachineInfo;
   }
   catch (const dtUtil::Exception& ex)
   {
      ex.LogException(dtUtil::Log::LOG_ERROR);
   }
}

void PauseResumeTests::tearDown()
{
   dtCore::System::GetInstance().SetPause(false);
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
   dtCore::AppSleep(5);
   dtCore::System::GetInstance().Step();

   osg::Vec3 newPos = gap->GetTranslation();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The system is paused, the new translation should be equal to the old translation", newPos, oldPos);

   mGM->SetPaused(false);
   CPPUNIT_ASSERT(!mGM->IsPaused());

   dtCore::AppSleep(5);
   dtCore::System::GetInstance().Step();

   newPos = gap->GetTranslation();
   CPPUNIT_ASSERT_MESSAGE("The system is unpaused, the proxy should now be dead reckoning.", newPos != oldPos);

   mGM->SetPaused(true);
   CPPUNIT_ASSERT(mGM->IsPaused());
   oldPos = newPos;

   dtCore::AppSleep(5);
   dtCore::System::GetInstance().Step();

   newPos = gap->GetTranslation();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The system is paused, the new translation should equal the old translation", newPos, oldPos);
}

void PauseResumeTests::TestRewind()
{

}
