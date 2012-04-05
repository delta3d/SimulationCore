/* -*-c++-*-
* Simulation Core - BaseEntityActorProxyTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2010, Alion Science and Technology Corporation
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
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <dtGame/gamemanager.h>
#include <dtGame/gameactor.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>

#include <dtCore/actorproperty.h>
#include <dtCore/enginepropertytypes.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/refptr.h>
#include <dtCore/observerptr.h>

#include <dtUtil/macros.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/datapathutils.h>

#include <SimCore/Actors/LogicConditionalActor.h>
#include <SimCore/Actors/LogicOnEventActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtCore/transformableactorproxy.h>
#include <dtCore/gameevent.h>
#include <dtCore/gameeventmanager.h>


#include <TestComponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#ifdef DELTA_WIN32
   #include <dtUtil/mswin.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

//using dtCore::RefPtr;
//using dtCore::ObserverPtr;


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
class LogicActorTests: public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(LogicActorTests);
      CPPUNIT_TEST(TestConditionalActor);
      CPPUNIT_TEST(TestLogicOnEventActor);
   CPPUNIT_TEST_SUITE_END();

   public:
      void setUp();
      void tearDown();

      void TestConditionalActor();
      void TestLogicOnEventActor();

   private:
      void CheckGameEventReceived(dtCore::GameEvent *gameEvent, bool shouldExist, const std::string& message);

      dtCore::RefPtr<dtGame::GameManager> mGM;
      dtCore::RefPtr<TestComponent> mTestComponent;

};

CPPUNIT_TEST_SUITE_REGISTRATION(LogicActorTests);

///////////////////////////////////////////////////////////////////////
void LogicActorTests::setUp()
{
   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();
   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   mGM->SetApplication(GetGlobalApplication());

   // Test component tracks messages and game events
   mTestComponent = new TestComponent;
   mGM->AddComponent(*mTestComponent, dtGame::GameManager::ComponentPriority::NORMAL);
}

///////////////////////////////////////////////////////////////////////
void LogicActorTests::tearDown()
{
   mTestComponent->reset();
   mTestComponent = NULL;

   if (mGM.valid())
   {
      mGM->DeleteAllActors(true);
      mGM = NULL;
   }
   dtCore::System::GetInstance().Stop();
}

///////////////////////////////////////////////////////////////////////
void LogicActorTests::TestConditionalActor()
{
   dtCore::RefPtr<SimCore::Actors::LogicConditionalActorProxy> lcap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::LOGIC_CONDITIONAL_ACTOR_TYPE, lcap);
   CPPUNIT_ASSERT(lcap.valid());

   SimCore::Actors::LogicConditionalActor& lcapActor = 
      static_cast<SimCore::Actors::LogicConditionalActor&>(*lcap->GetActor());
   CPPUNIT_ASSERT_MESSAGE("Default condition should be false", !lcapActor.GetIsTrue());
   CPPUNIT_ASSERT_MESSAGE("Default true event should be NULL", lcapActor.GetTrueEvent() == NULL);
   CPPUNIT_ASSERT_MESSAGE("Default false event should be NULL", lcapActor.GetFalseEvent() == NULL);

   dtCore::RefPtr<dtCore::GameEvent> testTrueEvent = new dtCore::GameEvent("LogicTestTRUEEvent");
   //dtCore::GameEventManager::GetInstance().AddEvent(*testTrueEvent);
   lcapActor.SetTrueEvent(testTrueEvent);
   CPPUNIT_ASSERT_MESSAGE("True event should have been set", lcapActor.GetTrueEvent() == testTrueEvent);

   dtCore::RefPtr<dtCore::GameEvent> testFalseEvent = new dtCore::GameEvent("LogicTestFALSEEvent");
   //dtCore::GameEventManager::GetInstance().AddEvent(*testFalseEvent);
   lcapActor.SetFalseEvent(testFalseEvent);
   CPPUNIT_ASSERT_MESSAGE("False event should have been set", lcapActor.GetFalseEvent() == testFalseEvent);

   lcapActor.SetIsTrue(true);
   CPPUNIT_ASSERT_MESSAGE("Is True should be true", lcapActor.GetIsTrue());
}

///////////////////////////////////////////////////////////////////////
void LogicActorTests::CheckGameEventReceived(dtCore::GameEvent *gameEvent, bool shouldBeFound, const std::string& errorMessage)
{
   std::vector<dtCore::RefPtr<const dtGame::Message> > msgs = mTestComponent->GetReceivedProcessMessages();
   bool wasFound = false;

   for (unsigned int i = 0; i < msgs.size() && !wasFound; ++i)
   {
      if (msgs[i]->GetMessageType() == dtGame::MessageType::INFO_GAME_EVENT)
      {
         const dtGame::GameEventMessage& gem = static_cast<const dtGame::GameEventMessage&>(*msgs[i]);         
         wasFound = (gameEvent == gem.GetGameEvent());
      }
   }

   CPPUNIT_ASSERT_EQUAL_MESSAGE(errorMessage, shouldBeFound, wasFound);
}

///////////////////////////////////////////////////////////////////////
void LogicActorTests::TestLogicOnEventActor()
{
   // Our test layout looks like the following
   // Conditional 1
   //     - true - Event 1True
   //     - false - Event 1False
   // Conditional 2 
   //     - true - Event 2True
   //     - false - Event 2False
   // OnEvent 
   //     - ConditionalMetEvent
   //     - test 1)OR and 2) AND
   //     - Conditional 1
   //     - Conditional 2

   // Conditional 1
   dtCore::RefPtr<SimCore::Actors::LogicConditionalActorProxy> lcap1;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::LOGIC_CONDITIONAL_ACTOR_TYPE, lcap1);
   CPPUNIT_ASSERT(lcap1.valid());
   SimCore::Actors::LogicConditionalActor& lcap1Actor = static_cast<SimCore::Actors::LogicConditionalActor&>(*lcap1->GetActor());
   mGM->AddActor(*lcap1);

   // Conditional 2
   dtCore::RefPtr<SimCore::Actors::LogicConditionalActorProxy> lcap2;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::LOGIC_CONDITIONAL_ACTOR_TYPE, lcap2);
   CPPUNIT_ASSERT(lcap2.valid());
   SimCore::Actors::LogicConditionalActor& lcap2Actor = static_cast<SimCore::Actors::LogicConditionalActor&>(*lcap2->GetActor());
   mGM->AddActor(*lcap2);

   // On Event Actor
   dtCore::RefPtr<SimCore::Actors::LogicOnEventActorProxy> onEventProxy;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::LOGIC_ON_EVENT_ACTOR_TYPE, onEventProxy);
   CPPUNIT_ASSERT(onEventProxy.valid());
   SimCore::Actors::LogicOnEventActor& onEventActor = static_cast<SimCore::Actors::LogicOnEventActor&>(onEventProxy->GetGameActor());
   mGM->AddActor(*onEventProxy, false, true);

   // Event 1 True
   dtCore::RefPtr<dtCore::GameEvent> true1Event = new dtCore::GameEvent("Test 1 True Event");
   dtCore::GameEventManager::GetInstance().AddEvent(*true1Event);
   lcap1Actor.SetTrueEvent(true1Event.get());
   // Event 1 False
   dtCore::RefPtr<dtCore::GameEvent> false1Event = new dtCore::GameEvent("Test 1 False Event");
   dtCore::GameEventManager::GetInstance().AddEvent(*false1Event);
   lcap1Actor.SetFalseEvent(false1Event.get());

   // Event 2 True
   dtCore::RefPtr<dtCore::GameEvent> true2Event = new dtCore::GameEvent("Test 2 True Event");
   dtCore::GameEventManager::GetInstance().AddEvent(*true2Event);
   lcap2Actor.SetTrueEvent(true2Event.get());
   // Event 2 False
   dtCore::RefPtr<dtCore::GameEvent> false2Event = new dtCore::GameEvent("Test 2 False Event");
   dtCore::GameEventManager::GetInstance().AddEvent(*false2Event);
   lcap2Actor.SetFalseEvent(false2Event.get());

   // ConditionalsMetEvent
   dtCore::RefPtr<dtCore::GameEvent> conditionsMetEvent = new dtCore::GameEvent("Test Conditions Met Event");
   dtCore::GameEventManager::GetInstance().AddEvent(*conditionsMetEvent);
   onEventActor.SetEventToFire(conditionsMetEvent.get());

   // Add conditions to the actor
   onEventActor.AddConditional(lcap1->GetId());
   onEventActor.AddConditional(lcap2->GetId());
   //std::vector<dtCore::UniqueId> conditionsArray;
   //conditionsArray.push_back(lcap1->GetId());
   //conditionsArray.push_back(lcap2->GetId());
   //onEventActor.ConditionalArraySetValue(conditionsArray);
   //onEventActor.ConditionalArraySetIndex(0);   
   //CPPUNIT_ASSERT_EQUAL_MESSAGE("1st conditional should be lcap1", 
   //   onEventActor.GetChildConditional(), lcap1->GetId());
   //onEventActor.ConditionalArraySetIndex(1);
   //CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd conditional should be lcap2", 
   //   onEventActor.GetChildConditional(), lcap2->GetId());

   // Test logic type
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Default Condition should be AND", 
      onEventActor.GetLogicType(), SimCore::Actors::LogicOnEventActorProxy::LogicTypeEnum::BOOLEAN_AND);
   onEventActor.SetLogicType(SimCore::Actors::LogicOnEventActorProxy::LogicTypeEnum::BOOLEAN_OR);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be set to OR", 
      onEventActor.GetLogicType(), SimCore::Actors::LogicOnEventActorProxy::LogicTypeEnum::BOOLEAN_OR);
   CPPUNIT_ASSERT_MESSAGE("Should initially be false", !onEventActor.GetCurrentStatus());
   onEventActor.SetLogicType(SimCore::Actors::LogicOnEventActorProxy::LogicTypeEnum::BOOLEAN_AND);
   CPPUNIT_ASSERT_MESSAGE("Should return to false", !onEventActor.GetCurrentStatus());

   // Fire Event 1 True
   // Test that NOT true & NOT fired conditionMet Event
   mTestComponent->reset();
   onEventActor.SendGameEventMessage(*true1Event);
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("Non-match should set current status to false.(A)", !onEventActor.GetCurrentStatus());
   CheckGameEventReceived(conditionsMetEvent.get(), false, "AND condition should not have been met yet");

   // Fire Event 2 True
   // Test that IS true and DID fire condition Met Event
   mTestComponent->reset();
   onEventActor.SendGameEventMessage(*true2Event);
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("Full match should set current status to true.(B)", onEventActor.GetCurrentStatus());
   CheckGameEventReceived(conditionsMetEvent.get(), true, "AND condition should NOW be met");

   // Fire Event 2 False
   // Test that is now false and did NOT fire condition met event
   mTestComponent->reset();
   onEventActor.SendGameEventMessage(*false2Event);
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("Partial match should set current status to false.(C)", !onEventActor.GetCurrentStatus());
   CheckGameEventReceived(conditionsMetEvent.get(), false, "AND condition is no longer met. ");

   // Set to OR
   onEventActor.SetLogicType(SimCore::Actors::LogicOnEventActorProxy::LogicTypeEnum::BOOLEAN_OR);

   // Fire Event 1 True - AGAIN
   // Test that is now true and DID fire condition met event
   mTestComponent->reset();
   onEventActor.SendGameEventMessage(*true1Event);
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("Partial match with OR should set current status to true.(D)", onEventActor.GetCurrentStatus());
   CheckGameEventReceived(conditionsMetEvent.get(), true, "OR condition should NOW be met. ");

   // Fire Event 2 False 
   // Test that is still true and DID fire condition met event
   mTestComponent->reset();
   onEventActor.SendGameEventMessage(*false2Event);
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("Partial match with OR should set current status to true.(E)", onEventActor.GetCurrentStatus());
   CheckGameEventReceived(conditionsMetEvent.get(), true, "OR condition should still be met. ");

   // Fire Event 1 False
   // Test that is now false and did NOT fire condition met event
   mTestComponent->reset();
   onEventActor.SendGameEventMessage(*false1Event);
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("No matching conditions with OR should set current status to false.(F)", !onEventActor.GetCurrentStatus());
   CheckGameEventReceived(conditionsMetEvent.get(), false, "OR condition should no longer be met. ");

}

