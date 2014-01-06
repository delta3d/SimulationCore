/* -*-c++-*-
* Simulation Core - ViewerNetworkPublishingComponentTests (.h & .cpp) - Using 'The MIT License'
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
* @author David Guthrie
*/
#include <prefix/SimCorePrefix.h>
#include <string>
#include <vector>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

#include <dtABC/application.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/map.h>

#include <dtGame/gamemanager.h>
#include <dtGame/messagefactory.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/deadreckoninghelper.h>

#include <dtCore/system.h>
#include <dtCore/deltawin.h>
#include <dtCore/transform.h>

#include <dtUtil/mathdefines.h>

#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/ViewerNetworkPublishingComponent.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/StealthActor.h>

#include <dtGame/testcomponent.h>
#include <UnitTestMain.h>

using std::shared_ptr;

class ViewerNetworkPublishingComponentTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(ViewerNetworkPublishingComponentTests);

      CPPUNIT_TEST(TestStealthActorMessages);
      CPPUNIT_TEST(TestTimeQueryMessagePropagation);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         mApp = &GetGlobalApplication();

         mGM = new dtGame::GameManager(*mApp->GetScene());
         mGM->SetApplication( *mApp );
         std::shared_ptr<SimCore::Components::ViewerNetworkPublishingComponent> rulesComp = new SimCore::Components::ViewerNetworkPublishingComponent;
         mGM->AddComponent(*rulesComp, dtGame::GameManager::ComponentPriority::NORMAL);
         mTestComp = new dtGame::TestComponent;
         mGM->AddComponent(*mTestComp, dtGame::GameManager::ComponentPriority::NORMAL);

         std::shared_ptr<dtDAL::ActorProxy> ap = mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE);

         CPPUNIT_ASSERT(ap.valid());

         mStealthActor =
            dynamic_cast<SimCore::Actors::StealthActor*>(ap->GetDrawable());

         CPPUNIT_ASSERT(mStealthActor.valid());
         mGM->AddActor(mStealthActor->GetGameActorProxy(), false, true);

         dtCore::System::GetInstance().Start();
         dtCore::System::GetInstance().Step();
      }

      void tearDown()
      {
         dtCore::System::GetInstance().Stop();

         mTestComp = nullptr;
         mApp = nullptr;
         if (mGM.valid())
         {
            mGM->DeleteAllActors(true);
         }

         mGM = nullptr;
      }

      void TestStealthActorMessages()
      {
         InternalTestStealthActorMessages(false);

         std::vector<dtUtil::RefString> params;

         params.push_back(dtGame::DeadReckoningHelper::PROPERTY_LAST_KNOWN_ROTATION);
         params.push_back(dtGame::DeadReckoningHelper::PROPERTY_LAST_KNOWN_TRANSLATION);

         InternalTestStealthActorMessages(true, params);

         params.clear();
         params.push_back(dtGame::DeadReckoningHelper::PROPERTY_LAST_KNOWN_ROTATION);

         InternalTestStealthActorMessages(true, params);

         params.clear();
         params.push_back(dtGame::DeadReckoningHelper::PROPERTY_LAST_KNOWN_TRANSLATION);

         InternalTestStealthActorMessages(true, params);

         params.clear();
         // Must put one property or it will send them all.
         params.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_DAMAGE_STATE);

         InternalTestStealthActorMessages(true, params);

      }

      void TestTimeQueryMessagePropagation()
      {
         dtGame::MessageFactory& mf = mGM->GetMessageFactory();
         std::shared_ptr<SimCore::TimeQueryMessage> timeQuery;
         mf.CreateMessage(SimCore::MessageType::TIME_QUERY, timeQuery);
         mGM->SendMessage(*timeQuery);
         mTestComp->reset();
         dtCore::System::GetInstance().Step();
         dtCore::System::GetInstance().Step();

         const dtGame::Message* foundMessage = mTestComp->FindDispatchNetworkMessageOfType(SimCore::MessageType::TIME_QUERY).get();

         CPPUNIT_ASSERT_MESSAGE("A time query message should have been dispatched to the network.", foundMessage != nullptr);
         CPPUNIT_ASSERT_MESSAGE("The time query message dispatched should be the same as the one sent.",
               *foundMessage == *timeQuery);
      }

   private:

      void InternalTestStealthActorMessages(bool partial, const std::vector<dtUtil::RefString>& params = std::vector<dtUtil::RefString>())
      {
         mTestComp->reset();

         osg::Vec3 testVec(1.1f, 2.2f, 3.3f);
         //account for the different order of rotation vecs.
         osg::Vec3 testRot(testVec.z(), testVec.x(), testVec.y());

         dtGame::DeadReckoningHelper* drHelper = nullptr;
         mStealthActor->GetComponent(drHelper);
         //Must set both the last known and the actual values so the message and
         //and actor will match after being stepped
         drHelper->SetLastKnownTranslation(testVec);
         drHelper->SetLastKnownRotation(testRot);

         dtCore::Transform xform;
         mStealthActor->GetTransform(xform, dtCore::Transformable::ABS_CS);
         xform.SetTranslation(testVec);
         xform.SetRotation(testRot);
         mStealthActor->SetTransform(xform, dtCore::Transformable::ABS_CS);

         if (partial)
         {
            std::shared_ptr<dtGame::ActorUpdateMessage> updateMessage =
               static_cast<dtGame::ActorUpdateMessage*>(mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED).get());

            mStealthActor->GetGameActorProxy().PopulateActorUpdate(*updateMessage, params);
            mGM->SendMessage(*updateMessage);
         }
         else
         {
            mStealthActor->GetGameActorProxy().NotifyFullActorUpdate();
         }

         //it takes two frames for a message to get sent out because it is first processed in one, decided
         //that is should be sent, then sent.  In the third frame, the sent message is delivered.
         dtCore::System::GetInstance().Step();
         dtCore::System::GetInstance().Step();

         std::set<std::string> paramSet;

         paramSet.insert(params.begin(), params.end());

         if (!partial || paramSet.find("Last Known Rotation") != paramSet.end())
         {
            std::shared_ptr<const SimCore::StealthActorUpdatedMessage> msg =
               static_cast<const SimCore::StealthActorUpdatedMessage*>(mTestComp->
               FindDispatchNetworkMessageOfType(SimCore::MessageType::STEALTH_ACTOR_ROTATION).get());

            CPPUNIT_ASSERT_MESSAGE("A rotation message should have been sent.", msg.valid());

            CPPUNIT_ASSERT_MESSAGE("The about actor id on the stealth rotation message should match the actor value.",
               mStealthActor->GetUniqueId() == msg->GetAboutActorId());

            osg::Vec3 value = static_cast<dtDAL::Vec3ActorProperty*>(mStealthActor->GetGameActorProxy().GetProperty("Last Known Rotation"))->GetValue();
            osg::Vec3 msgRot = msg->GetRotation();
            CPPUNIT_ASSERT_MESSAGE("The rotation on the message should match the actor value.",
               dtUtil::Equivalent(value, msgRot, 1e-3f));
         }
         else
         {
            bool result = mTestComp->FindDispatchNetworkMessageOfType(SimCore::MessageType::STEALTH_ACTOR_ROTATION).valid();

            CPPUNIT_ASSERT_MESSAGE("No stealth actor rotation message should have been sent", !result);
         }

         if (!partial || paramSet.find("Last Known Translation") != paramSet.end())
         {
            std::shared_ptr<const SimCore::StealthActorUpdatedMessage> msg =
               static_cast<const SimCore::StealthActorUpdatedMessage*>(mTestComp->
               FindDispatchNetworkMessageOfType(SimCore::MessageType::STEALTH_ACTOR_TRANSLATION).get());

            CPPUNIT_ASSERT_MESSAGE("A Translation message should have been sent.", msg.valid());

            CPPUNIT_ASSERT_MESSAGE("The about actor id on the stealth rotation message should match the actor value.",
               mStealthActor->GetUniqueId() == msg->GetAboutActorId());
            osg::Vec3 value = static_cast<dtDAL::Vec3ActorProperty*>(mStealthActor->GetGameActorProxy().GetProperty("Last Known Translation"))->GetValue();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Translation on the message should match the actor value.",
               value,
               msg->GetTranslation());
         }
         else
         {
            bool result = mTestComp->FindDispatchNetworkMessageOfType(SimCore::MessageType::STEALTH_ACTOR_TRANSLATION).valid();
            CPPUNIT_ASSERT_MESSAGE("No stealth actor Translation message should have been sent", !result);
         }

         mTestComp->reset();

      }

      std::shared_ptr<dtGame::GameManager> mGM;
      std::shared_ptr<dtGame::TestComponent> mTestComp;
      std::shared_ptr<SimCore::Actors::StealthActor> mStealthActor;
      std::shared_ptr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ViewerNetworkPublishingComponentTests);
