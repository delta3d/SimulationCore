/* -*-c++-*-
* Simulation Core - ViewerMessageProcessorTests (.h & .cpp) - Using 'The MIT License'
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

#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <dtGame/gamemanager.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/deadreckoningcomponent.h>

#include <dtDAL/project.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtAudio/audiomanager.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/timer.h>

#include <dtUtil/macros.h>

#include <osg/Vec3>

#include <UnitTestMain.h>
#include <dtABC/application.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {

      class ViewerMessageProcessorTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(ViewerMessageProcessorTests);

            CPPUNIT_TEST(TestAcceptPlayer);
            CPPUNIT_TEST(TestProcessLocalUpdateActor);
            CPPUNIT_TEST(TestPlayerEnteredWorldMessage);
            CPPUNIT_TEST(TestTimeValueMessageReceive);
            CPPUNIT_TEST(TestTimeValueMessageReceiveWithScale);
            CPPUNIT_TEST(TestTimeValueMessageReceivePaused);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            void TestAcceptPlayer();
            void TestProcessLocalUpdateActor();
            void TestPlayerEnteredWorldMessage();
            void TestTimeValueMessageReceive();
            void TestTimeValueMessageReceiveWithScale();
            void TestTimeValueMessageReceivePaused();

         private:

            RefPtr<dtGame::GameManager> mGM;
            RefPtr<dtGame::MachineInfo> mMachineInfo;
            RefPtr<ViewerMessageProcessor> mVMP;

            void TestAcceptPlayer(bool remote);
            RefPtr<SimCore::TimeValueMessage> BuildAndSetupTimeValueMessage();
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(ViewerMessageProcessorTests);

      void ViewerMessageProcessorTests::setUp()
      {
         try
         {
            dtCore::System::GetInstance().Start();
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
            mGM->SetApplication(GetGlobalApplication());

            mMachineInfo = new dtGame::MachineInfo;

            dtCore::RefPtr<dtGame::DeadReckoningComponent> drComp = new dtGame::DeadReckoningComponent();
            mVMP = new ViewerMessageProcessor;

            mGM->AddComponent(*mVMP, dtGame::GameManager::ComponentPriority::HIGHEST);
            mGM->AddComponent(*drComp, dtGame::GameManager::ComponentPriority::NORMAL);
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }
      }

      void ViewerMessageProcessorTests::tearDown()
      {
         dtCore::System::GetInstance().SetPause(false);
         dtCore::System::GetInstance().Stop();
         mVMP = NULL;

         mGM->DeleteAllActors(true);

         mGM = NULL;
         mMachineInfo = NULL;
      }

      void ViewerMessageProcessorTests::TestAcceptPlayer()
      {
         TestAcceptPlayer(true);
         TestAcceptPlayer(false);
      }

      void ViewerMessageProcessorTests::TestAcceptPlayer(bool remote)
      {
         RefPtr<SimCore::Actors::BaseEntityActorProxy> entityAP;

         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, entityAP);
         CPPUNIT_ASSERT(entityAP.valid());

         mGM->AddActor(*entityAP, remote, false);

         if (remote)
            CPPUNIT_ASSERT_MESSAGE("The Viewer Message Processor should not accept remote players.", !mVMP->AcceptPlayer(*entityAP));
         else
            CPPUNIT_ASSERT_MESSAGE("The Viewer Message Processor should accept local players.", mVMP->AcceptPlayer(*entityAP));
      }

      void ViewerMessageProcessorTests::TestProcessLocalUpdateActor()
      {
         RefPtr<dtGame::GameActorProxy> proxy;
         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy);
         CPPUNIT_ASSERT(proxy.valid());
         mGM->AddActor(*proxy, true, false);
         RefPtr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED);
         CPPUNIT_ASSERT(msg.valid());

         osg::Vec3 newPosition(0, 100, 0);
         dtGame::ActorUpdateMessage &aumsg = static_cast<dtGame::ActorUpdateMessage&>(*msg);
         dtGame::MessageParameter *param = aumsg.AddUpdateParameter("Last Known Translation", dtDAL::DataType::VEC3);
         CPPUNIT_ASSERT(param != NULL);
         static_cast<dtGame::Vec3MessageParameter*>(param)->SetValue(newPosition);
         aumsg.SetSource(*mMachineInfo);
         aumsg.SetAboutActorId(proxy->GetId());
         mGM->SendMessage(aumsg);

         dtCore::AppSleep(10);
         dtCore::System::GetInstance().Step();

         osg::Vec3 position = static_cast<dtDAL::Vec3ActorProperty*>(proxy->GetProperty("Last Known Translation"))->GetValue();
         CPPUNIT_ASSERT_MESSAGE("The property should have been set correctly on the proxy.", position == newPosition);
      }

      void ViewerMessageProcessorTests::TestPlayerEnteredWorldMessage()
      {
         dtGame::DeadReckoningComponent* drComp;
         mGM->GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME, drComp);

         RefPtr<dtGame::GameActorProxy> proxy;
         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, proxy);
         CPPUNIT_ASSERT(proxy.valid());
         mGM->AddActor(*proxy, false, false);
         RefPtr<dtGame::Message> msg;
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD, msg);
         CPPUNIT_ASSERT(msg.valid());
         msg->SetSource(*mMachineInfo);
         msg->SetAboutActorId(proxy->GetId());
         mGM->SendMessage(*msg);

         dtCore::AppSleep(10);
         dtCore::System::GetInstance().Step();

         CPPUNIT_ASSERT_MESSAGE("The ViewerMessageProcessor should have received a player entered world message, and set its internal accordingly.",
            mVMP->GetPlayerActor() == proxy->GetDrawable());

         CPPUNIT_ASSERT_MESSAGE("The eye point on the DeadReckoningComponent should have been set by the ViewerMessageProcessor.",
            drComp->GetEyePointActor() == proxy->GetDrawable());
      }

      RefPtr<SimCore::TimeValueMessage> ViewerMessageProcessorTests::BuildAndSetupTimeValueMessage()
      {
         dtCore::UniqueId timeMasterId;

         dtGame::MessageFactory& mf = mGM->GetMessageFactory();
         RefPtr<dtGame::MachineInfo> testMachInfo = new dtGame::MachineInfo;
         RefPtr<SimCore::TimeValueMessage> timeValue;
         mf.CreateMessage(SimCore::MessageType::TIME_VALUE, timeValue);
         timeValue->SetSource(*testMachInfo);

         timeValue->SetPaused(false);
         timeValue->SetTimeMaster(timeMasterId.ToString());
         timeValue->SetQueryTransmitRealTime((mGM->GetRealClockTime()/1000L) - 5000);
         timeValue->SetQueryReceivedRealTime((mGM->GetRealClockTime()/1000L) - 3000);
         timeValue->SetValueTransmitRealTime((mGM->GetRealClockTime()/1000L) - 2000);
         timeValue->SetSynchronizedTime(45000);

         timeValue->SetTimeScale(1.0f);
         timeValue->SetSenderName(mVMP->GetTimeSyncSenderName());

         return timeValue;

      }

      void ViewerMessageProcessorTests::TestTimeValueMessageReceive()
      {
         // step now so that the time between now and next step is almost
         // 0, otherwise the code would get messed up.
         dtCore::System::GetInstance().Step();
         RefPtr<SimCore::TimeValueMessage> timeValue = BuildAndSetupTimeValueMessage();

         mGM->SendMessage(*timeValue);
         dtCore::System::GetInstance().Step();

         // The math is seems mystical here, but 47 should be the value in seconds because the
         // the average latencey based on the send and receive times should say we should add 2 seconds
         // to the clock to make it sync up.
         CPPUNIT_ASSERT_EQUAL(dtCore::Timer_t(47), dtCore::Timer_t(mGM->GetSimulationTime()));
         CPPUNIT_ASSERT(!mGM->IsPaused());

         /// The processor currently takes all time-sync messages.
/*         timeValue = BuildAndSetupTimeValueMessage();

         timeValue->SetSenderName("someone else");

         // step now so that the time between now and next step is almost
         // 0, otherwise the code would get messed up.
         mGM->SendMessage(*timeValue);
         dtCore::System::GetInstance().Step();

         // The math is seems mystical here, but 47 should be the value in seconds because the
         // the average latencey based on the send and receive times should say we should add 2 seconds
         // to the clock to make it sync up.
         CPPUNIT_ASSERT_EQUAL(dtCore::Timer_t(47), dtCore::Timer_t(mGM->GetSimulationClockTime() / 1e6L));
         CPPUNIT_ASSERT(!mGM->IsPaused());*/
      }

      void ViewerMessageProcessorTests::TestTimeValueMessageReceiveWithScale()
      {
         RefPtr<SimCore::TimeValueMessage> timeValue = BuildAndSetupTimeValueMessage();

         // set the scale to 3.0 to make sure that is taken into account when adjusting the time.
         timeValue->SetTimeScale(3.0f);

         // step now so that the time between now and next step is almost
         // 0, otherwise the code would get messed up.
         dtCore::System::GetInstance().Step();


         mGM->SendMessage(*timeValue);
         dtCore::System::GetInstance().Step();

         // this is 51 because the time latency was 2 seconds but with a scale of 3.
         CPPUNIT_ASSERT_EQUAL(dtCore::Timer_t(51), dtCore::Timer_t(mGM->GetSimulationTime()));

         CPPUNIT_ASSERT_EQUAL_MESSAGE("After handling a time value message, the scale on the gm should have been set to match.",
               timeValue->GetTimeScale(), mGM->GetTimeScale());
         CPPUNIT_ASSERT(!mGM->IsPaused());
      }

      void ViewerMessageProcessorTests::TestTimeValueMessageReceivePaused()
      {
         // step now so that the time between now and next step is almost
         // 0, otherwise the code would get messed up.
         dtCore::System::GetInstance().Step();
         RefPtr<SimCore::TimeValueMessage> timeValue = BuildAndSetupTimeValueMessage();

         // set the scale to 3.0 to make sure that is taken into account when adjusting the time.
         timeValue->SetPaused(true);

         mGM->SendMessage(*timeValue);
         dtCore::System::GetInstance().Step();

         // this is 45 because the message send 45 and the time does not pass when paused.
         CPPUNIT_ASSERT_EQUAL(dtCore::Timer_t(45), dtCore::Timer_t(mGM->GetSimulationTime()));

         CPPUNIT_ASSERT(mGM->IsPaused());
      }
   }
}
