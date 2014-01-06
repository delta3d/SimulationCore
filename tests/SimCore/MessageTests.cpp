/* -*-c++-*-
* Simulation Core - MessageTests (.h & .cpp) - Using 'The MIT License'
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

#include <dtGame/gamemanager.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/invokable.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/deltawin.h>
#include <dtUtil/nodecollector.h>

#include <dtDAL/project.h>
#include <dtDAL/map.h>

#include <dtUtil/mathdefines.h>
#include <dtUtil/macros.h>

#include <dtABC/application.h>

#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/Components/BaseInputComponent.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <osg/Group>
#include <osgSim/DOFTransform>

#include <UnitTestMain.h>

using std::shared_ptr;

class MessageTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(MessageTests);

      CPPUNIT_TEST(TestStealthActorMessage);
      CPPUNIT_TEST(TestDetonationMessage);
      CPPUNIT_TEST(TestTimeQueryMessage);
      CPPUNIT_TEST(TestTimeValueMessage);
      CPPUNIT_TEST(TestAttachToActorMessageNoSubNode);
      CPPUNIT_TEST(TestAttachToActorMessageWithSubNode);
      CPPUNIT_TEST(TestWarpToPositionMessage);
      CPPUNIT_TEST(TestDetachOnActorDeletedMessage);
      CPPUNIT_TEST(TestToolMessageTypes);
      CPPUNIT_TEST(TestMagnificationMessage);
      CPPUNIT_TEST(TestEmbeddedDataMessage);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         dtCore::System::GetInstance().Start();
         mApp = &GetGlobalApplication();

         mGM = new dtGame::GameManager(*mApp->GetScene());
         mGM->SetApplication(*mApp);
         std::shared_ptr<dtGame::DeadReckoningComponent> drComp = new dtGame::DeadReckoningComponent;
         mGM->AddComponent(*drComp, dtGame::GameManager::ComponentPriority::NORMAL);
      }

      void tearDown()
      {
         if (mGM.valid())
         {
            mGM->DeleteAllActors(true);
            mGM = nullptr;
         }
         mApp = nullptr;
         dtCore::System::GetInstance().SetPause(false);
         dtCore::System::GetInstance().Stop();
      }

      void TestToolMessageTypes()
      {
         //Make sure these are all true.
         CPPUNIT_ASSERT(SimCore::MessageType::IsValidToolType(SimCore::MessageType::BINOCULARS));
         CPPUNIT_ASSERT(SimCore::MessageType::IsValidToolType(SimCore::MessageType::GPS));
         CPPUNIT_ASSERT(SimCore::MessageType::IsValidToolType(SimCore::MessageType::COMPASS));
         CPPUNIT_ASSERT(SimCore::MessageType::IsValidToolType(SimCore::MessageType::LASER_RANGE_FINDER));
         CPPUNIT_ASSERT(SimCore::MessageType::IsValidToolType(SimCore::MessageType::NO_TOOL));
         CPPUNIT_ASSERT(SimCore::MessageType::IsValidToolType(SimCore::MessageType::NIGHT_VISION));

         //Make sure it will actually return false on an invalid one
         CPPUNIT_ASSERT(!SimCore::MessageType::IsValidToolType(SimCore::MessageType::STEALTH_ACTOR_FOV));
      }

      void TestStealthActorMessage()
      {
         try
         {

            std::shared_ptr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::STEALTH_ACTOR_FOV);
            CPPUNIT_ASSERT_MESSAGE("The message factory failed to create the stealth actor FOV message", msg.valid());

            SimCore::StealthActorUpdatedMessage *saum = static_cast<SimCore::StealthActorUpdatedMessage*>(msg.get());

            osg::Vec3 vec(1, 5, 4);

            saum->SetTranslation(vec);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTranslation should return what was set", vec, saum->GetTranslation());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::TRANSLATION) != nullptr);

            saum->SetRotation(vec);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetRotation should return what was set",  vec, saum->GetRotation());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::ROTATION) != nullptr);

            float testFloat = 14.55f;

            saum->SetHorizontalFOV(testFloat);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetHorizontalFOV should return what was set", testFloat, saum->GetHorizontalFOV());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::HORIZONTAL_FOV) != nullptr);

            saum->SetVerticalFOV(testFloat);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetVerticalFOV should return what was set", testFloat, saum->GetVerticalFOV());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::HORIZONTAL_FOV) != nullptr);

         }
         catch(const dtUtil::Exception &ex)
         {
            CPPUNIT_FAIL("Exception caught: " + ex.What());
         }
      }

      template <class T, class V>
      void TestParameter(dtGame::Message& msg, const std::string& name, V value)
      {
         std::shared_ptr<dtGame::MessageParameter> param = msg.GetParameter(name);
         CPPUNIT_ASSERT(param.valid());

         CPPUNIT_ASSERT(dynamic_cast<T*>(param.get()) != nullptr);
         CPPUNIT_ASSERT_EQUAL(value,
            static_cast<T*>(param.get())->GetValue());
      }

      void TestTimeQueryMessage()
      {
         try
         {
            std::shared_ptr<SimCore::TimeQueryMessage> msg;
            mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::TIME_QUERY, msg);
            CPPUNIT_ASSERT_MESSAGE("The message factory failed to create a TIME_QUERY message", msg.valid());

            CPPUNIT_ASSERT_EQUAL(std::string(""), msg->GetSenderName());
            msg->SetSenderName("jojo");
            CPPUNIT_ASSERT_EQUAL(std::string("jojo"), msg->GetSenderName());

            TestParameter<dtGame::StringMessageParameter, std::string>(*msg, SimCore::TimeQueryMessage::SENDER_NAME, "jojo");

            CPPUNIT_ASSERT_EQUAL((unsigned long)(0), msg->GetQueryTransmitRealTime());
            msg->SetQueryTransmitRealTime(34L);
            CPPUNIT_ASSERT_EQUAL((unsigned long)(34), msg->GetQueryTransmitRealTime());

            TestParameter<dtGame::UnsignedLongIntMessageParameter, unsigned long>(*msg, SimCore::TimeQueryMessage::QUERY_TRANSMIT_REAL_TIME, 34L);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL("Exception caught: " + ex.What());
         }
      }

      void TestTimeValueMessage()
      {
         try
         {
            std::shared_ptr<SimCore::TimeValueMessage> msg;
            mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::TIME_VALUE, msg);
            CPPUNIT_ASSERT_MESSAGE("The message factory failed to create a TIME_VALUE message", msg.valid());

            CPPUNIT_ASSERT_EQUAL((unsigned long)(0), msg->GetQueryTransmitRealTime());
            msg->SetQueryTransmitRealTime(34L);
            CPPUNIT_ASSERT_EQUAL((unsigned long)(34), msg->GetQueryTransmitRealTime());

            TestParameter<dtGame::UnsignedLongIntMessageParameter, unsigned long>(*msg, SimCore::TimeQueryMessage::QUERY_TRANSMIT_REAL_TIME, 34L);

            CPPUNIT_ASSERT_EQUAL((unsigned long)(0), msg->GetQueryReceivedRealTime());
            msg->SetQueryReceivedRealTime(32L);
            CPPUNIT_ASSERT_EQUAL((unsigned long)(32), msg->GetQueryReceivedRealTime());

            TestParameter<dtGame::UnsignedLongIntMessageParameter, unsigned long>(*msg, SimCore::TimeValueMessage::QUERY_RECEIVED_REAL_TIME, 32L);

            CPPUNIT_ASSERT_EQUAL((unsigned long)(0), msg->GetValueTransmitRealTime());
            msg->SetValueTransmitRealTime(36L);
            CPPUNIT_ASSERT_EQUAL((unsigned long)(36), msg->GetValueTransmitRealTime());

            TestParameter<dtGame::UnsignedLongIntMessageParameter, unsigned long>(*msg, SimCore::TimeValueMessage::VALUE_TRANSMIT_REAL_TIME, 36L);

            CPPUNIT_ASSERT_EQUAL((unsigned long)(0), msg->GetSynchronizedTime());
            msg->SetSynchronizedTime(40L);
            CPPUNIT_ASSERT_EQUAL((unsigned long)(40), msg->GetSynchronizedTime());

            TestParameter<dtGame::UnsignedLongIntMessageParameter, unsigned long>(*msg, SimCore::TimeValueMessage::SYNCHRONIZED_TIME, 40L);

            CPPUNIT_ASSERT_EQUAL(SimCore::TimeValueMessage::DEFAULT_TIME_SCALE, msg->GetTimeScale());
            msg->SetTimeScale(43.033f);
            CPPUNIT_ASSERT_EQUAL(43.033f, msg->GetTimeScale());

            TestParameter<dtGame::FloatMessageParameter, float>(*msg, SimCore::TimeValueMessage::TIME_SCALE, 43.033f);

            CPPUNIT_ASSERT_EQUAL(false, msg->IsPaused());
            msg->SetPaused(true);
            CPPUNIT_ASSERT_EQUAL(true, msg->IsPaused());

            TestParameter<dtGame::BooleanMessageParameter, bool>(*msg, SimCore::TimeValueMessage::PAUSED, true);

            CPPUNIT_ASSERT_EQUAL(std::string(""), msg->GetTimeMaster());
            msg->SetTimeMaster("jojo");
            CPPUNIT_ASSERT_EQUAL(std::string("jojo"), msg->GetTimeMaster());

            TestParameter<dtGame::StringMessageParameter, const std::string&>(*msg, SimCore::TimeValueMessage::TIME_MASTER, "jojo");
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL("Exception caught: " + ex.What());
         }
      }

      void TestDetonationMessage()
      {
         try
         {
            std::shared_ptr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::DETONATION);

            CPPUNIT_ASSERT_MESSAGE("The message factory failed to create the detonation message", msg.valid());

            SimCore::DetonationMessage *dm = static_cast<SimCore::DetonationMessage*>(msg.get());

            osg::Vec3f vec(1, 5, 4);
            dm->SetDetonationLocation(vec);
            CPPUNIT_ASSERT_MESSAGE("GetDetonationLocation should return what was set", dtUtil::Equivalent(dm->GetDetonationLocation(), vec, 0.001f));

            const char code = 'Q';
            dm->SetDetonationResultCode(code);
            CPPUNIT_ASSERT_MESSAGE("GetDetonationResultCode should return what was set", dm->GetDetonationResultCode() == code);

            vec.set(6, 4, 5);
            dm->SetFinalVelocityVector(vec);
            CPPUNIT_ASSERT_MESSAGE("GetFinalVelocityVector should return what was set", dtUtil::Equivalent(dm->GetFinalVelocityVector(), vec, 0.001f));

            const unsigned short int ushort = 63334;
            dm->SetEventIdentifier(ushort);
            CPPUNIT_ASSERT_MESSAGE("GetEventIdentifier should return what was set", dm->GetEventIdentifier() == ushort);

            SimCore::Actors::DetonationMunitionType& expectedValue = SimCore::Actors::DetonationMunitionType::LARGE_EXPLOSION;
            dm->SetMunitionType(SimCore::Actors::DetonationMunitionType::LARGE_EXPLOSION.GetName());
            CPPUNIT_ASSERT_MESSAGE("GetMunitionType should return what was set", dm->GetMunitionType() == expectedValue.GetName());
         }
         catch(const dtUtil::Exception &e)
         {
            CPPUNIT_FAIL("Exception caught: " + e.What());
         }
      }

      void TestAttachToActorMessageNoSubNode()
      {
         TestAttachToActorMessage(false);
      }

      void TestAttachToActorMessageWithSubNode()
      {
         TestAttachToActorMessage(true);
      }

      void TestAttachToActorMessage(bool useSubNode)
      {
         try
         {
            std::shared_ptr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
            CPPUNIT_ASSERT(msg.valid());

            SimCore::AttachToActorMessage *aam = static_cast<SimCore::AttachToActorMessage*>(msg.get());

            dtCore::UniqueId id;

            aam->SetAttachToActor(id);
            CPPUNIT_ASSERT(aam->GetAttachToActor() == id);

            aam->SetAttachToActor(dtCore::UniqueId(""));
            CPPUNIT_ASSERT(aam->GetAttachToActor().ToString().empty());

            std::shared_ptr<SimCore::Actors::PlatformActorProxy> t80Proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, t80Proxy);
            CPPUNIT_ASSERT(t80Proxy.valid());
            std::shared_ptr<SimCore::Actors::BaseEntity> t80Actor = dynamic_cast<SimCore::Actors::BaseEntity*>(t80Proxy->GetDrawable());
            CPPUNIT_ASSERT(t80Actor.valid());

            if (useSubNode)
            {
               dtDAL::ResourceDescriptor modelFile("StaticMeshes:T80:t80u_good.ive");

               CPPUNIT_ASSERT_NO_THROW_MESSAGE("The T80 Model does not exist", dtDAL::Project::GetInstance().GetResourcePath(modelFile));

               t80Proxy->SetNonDamagedResource(modelFile);
            }

            std::shared_ptr<SimCore::Actors::StealthActorProxy> playerProxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, playerProxy);
            CPPUNIT_ASSERT(playerProxy.valid());
            std::shared_ptr<SimCore::Actors::StealthActor> playerActor = dynamic_cast<SimCore::Actors::StealthActor*>(playerProxy->GetDrawable());
            CPPUNIT_ASSERT(playerActor.valid());

            mGM->AddActor(*t80Proxy, true, false);
            mGM->AddActor(*playerProxy, false, false);

            dtCore::Transform someOtherPosition(1888.8, 1776, 1134.4311);
            t80Actor->SetTransform(someOtherPosition, dtCore::Transformable::REL_CS);

            dtCore::Transform originalTransform;
            playerActor->GetTransform(originalTransform);

            std::shared_ptr<SimCore::AttachToActorMessage> atamsg;
            mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR, atamsg);
            CPPUNIT_ASSERT(atamsg.valid());
            atamsg->SetAboutActorId(playerActor->GetUniqueId());
            atamsg->SetAttachToActor(t80Actor->GetUniqueId());

            const std::string parentSubNodeName("dof_gun_01");
            if (useSubNode)
            {
               // The T80 model should have this node name.
               atamsg->SetAttachPointNodeName(parentSubNodeName);
            }

            const osg::Vec3 initialRotation(30.5f, 17.1f, 10.3f);
            atamsg->SetInitialAttachRotationHPR(initialRotation);

            mGM->SendMessage(*atamsg);

            dtCore::AppSleep(10);
            dtCore::System::GetInstance().Step();

            dtCore::Transform newTransform, tankTransform;
            playerActor->GetTransform(newTransform);
            t80Actor->GetTransform(tankTransform);

            osg::Vec3 playerPos, newPos, tankPos, tankRot;
            originalTransform.GetTranslation(playerPos);
            newTransform.GetTranslation(newPos);
            tankTransform.GetTranslation(tankPos);

            originalTransform.GetTranslation(playerPos);

            if (useSubNode)
            {
               std::shared_ptr<dtUtil::NodeCollector> nc = new dtUtil::NodeCollector(t80Actor->GetOSGNode(), dtUtil::NodeCollector::DOFTransformFlag);
               osg::Group* group = nc->GetDOFTransform(parentSubNodeName);
               CPPUNIT_ASSERT_MESSAGE("The dof node \"" + parentSubNodeName +
                        "\" should exist on the model, or the test won't work..", group != nullptr);
               CPPUNIT_ASSERT_EQUAL(size_t(1), playerActor->GetOSGNode()->getParents().size());
               CPPUNIT_ASSERT_MESSAGE("The stealth actor node should by a child of the dof node",
                        playerActor->GetOSGNode()->getParent(0) == group);
            }
            else
            {
               //Checking the X and Y for now because the Z position is not the same.  It's up some.
               CPPUNIT_ASSERT_MESSAGE("The player X position should equal the tank position", newPos.x() == tankPos.x());
               CPPUNIT_ASSERT_MESSAGE("The player Y position should equal the tank position", newPos.y() == tankPos.y());
            }

            CPPUNIT_ASSERT_MESSAGE("The old position should not equal the new position", playerPos != newPos);

            playerActor->GetTransform(newTransform, dtCore::Transformable::REL_CS);
            osg::Vec3 currRot;
            newTransform.GetRotation(currRot);
            CPPUNIT_ASSERT(dtUtil::Equivalent(currRot, initialRotation, 0.01f));
         }
         catch(const dtUtil::Exception &e)
         {
            CPPUNIT_FAIL("Exception caught: " + e.What());
         }
      }

      void TestDetachOnActorDeletedMessage()
      {
         try
         {
            std::shared_ptr<SimCore::Actors::PlatformActorProxy> t80Proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, t80Proxy);
            CPPUNIT_ASSERT(t80Proxy.valid());
            std::shared_ptr<SimCore::Actors::BaseEntity> t80Actor = dynamic_cast<SimCore::Actors::BaseEntity*>(t80Proxy->GetDrawable());
            CPPUNIT_ASSERT(t80Actor.valid());
            std::shared_ptr<SimCore::Actors::StealthActorProxy> playerProxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, playerProxy);
            CPPUNIT_ASSERT(playerProxy.valid());
            std::shared_ptr<SimCore::Actors::StealthActor> playerActor = dynamic_cast<SimCore::Actors::StealthActor*>(playerProxy->GetDrawable());
            CPPUNIT_ASSERT(playerActor.valid());

            osg::Vec3 tankPos(0, 100, 0);
            t80Actor->GetComponent<dtGame::DeadReckoningHelper>()->SetLastKnownTranslation(tankPos);

            mGM->AddActor(*t80Proxy, true, false);
            mGM->AddActor(*playerProxy, false, false);

            static_cast<SimCore::Actors::StealthActor&>(playerProxy->GetGameActor()).SetAttachOffset(osg::Vec3(0, 0, 0));

            std::shared_ptr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
            CPPUNIT_ASSERT(msg.valid());

            SimCore::AttachToActorMessage& aam = static_cast<SimCore::AttachToActorMessage&>(*msg);
            aam.SetAttachToActor(t80Proxy->GetId());
            aam.SetAboutActorId(playerProxy->GetId());

            mGM->SendMessage(aam);
            dtCore::AppSleep(10);
            dtCore::System::GetInstance().Step();

            dtGame::Invokable* invoke = playerActor->GetGameActorProxy().GetInvokable("AttachToActor");
            CPPUNIT_ASSERT_MESSAGE("The AttachToActor invokable should not be nullptr", invoke != nullptr);
            invoke->Invoke(aam);

            CPPUNIT_ASSERT_MESSAGE("The player should now be attached to the T80", playerActor->GetParent() == t80Proxy->GetDrawable());

            msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_DELETED);
            CPPUNIT_ASSERT(msg.valid());
            msg->SetAboutActorId(t80Proxy->GetId());
            mGM->SendMessage(*msg);

            dtCore::AppSleep(10);
            dtCore::System::GetInstance().Step();

            CPPUNIT_ASSERT_MESSAGE("The player should no longer be attached to the T80", playerActor->GetParent() == &mGM->GetScene());
            dtCore::Transform xform;
            playerActor->GetTransform(xform, dtCore::Transformable::ABS_CS);
            osg::Vec3 playerPos;
            xform.GetTranslation(playerPos);

            CPPUNIT_ASSERT_MESSAGE("The player's position should be the same as the tank.",
               dtUtil::Equivalent(playerPos, tankPos, 0.001f));
         }
         catch(const dtUtil::Exception &e)
         {
            CPPUNIT_FAIL("Exception caught: " + e.What());
         }
      }

      void TestWarpToPositionMessage()
      {
         try
         {
            std::shared_ptr<SimCore::Actors::PlatformActorProxy> t80Proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, t80Proxy);
            CPPUNIT_ASSERT(t80Proxy.valid());
            SimCore::Actors::BaseEntity* t80Actor;
            t80Proxy->GetActor(t80Actor);
            CPPUNIT_ASSERT(t80Actor != nullptr);

            std::shared_ptr<SimCore::Actors::StealthActorProxy> stealthProxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, stealthProxy);
            CPPUNIT_ASSERT(stealthProxy.valid());
            SimCore::Actors::StealthActor* stealthActor;
            stealthProxy->GetActor(stealthActor);
            CPPUNIT_ASSERT(stealthActor != nullptr);

            mGM->AddActor(*t80Proxy, true, false);
            mGM->AddActor(*stealthProxy, false, false);

            std::shared_ptr<SimCore::AttachToActorMessage> atamsg;
            mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR, atamsg);
            CPPUNIT_ASSERT(atamsg.valid());
            atamsg->SetAboutActorId(stealthActor->GetUniqueId());
            atamsg->SetAttachToActor(t80Actor->GetUniqueId());
            mGM->SendMessage(*atamsg);

            dtCore::AppSleep(10);
            dtCore::System::GetInstance().Step();

            //Quick attach
            CPPUNIT_ASSERT(stealthActor->IsAttachedToActor());

            dtCore::Transform originalTransform;
            stealthActor->GetTransform(originalTransform);

            std::shared_ptr<SimCore::StealthActorUpdatedMessage> msg;
            mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::REQUEST_WARP_TO_POSITION, msg);
            CPPUNIT_ASSERT(msg.valid());

            osg::Vec3 expPos(33.3, 22.9, -182.0);

            msg->SetAboutActorId(stealthActor->GetUniqueId());
            msg->SetTranslation(expPos);
            mGM->SendMessage(*msg);

            dtCore::AppSleep(10);
            dtCore::System::GetInstance().Step();

            dtCore::Transform newTransform, tankTransform;
            stealthActor->GetTransform(newTransform);

            osg::Vec3 stealthPos, newPos;
            originalTransform.GetTranslation(stealthPos);
            newTransform.GetTranslation(newPos);

            //Checking the X and Y for now because the Z position is not the same.  It's up some.
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The stealth X position should equal the tank position", expPos, newPos);

            CPPUNIT_ASSERT_MESSAGE("The old position should not equal the new position", stealthPos != newPos);

            CPPUNIT_ASSERT_MESSAGE("The stealth actor should not be attached to anything", !stealthActor->IsAttachedToActor());
         }
         catch(const dtUtil::Exception &e)
         {
            CPPUNIT_FAIL("Exception caught: " + e.What());
         }
      }

      // Tired of having to do this over and over
      void TestMagnificationMessage()
      {
         try
         {
            mGM->AddComponent(*new SimCore::Components::ViewerMessageProcessor, dtGame::GameManager::ComponentPriority::HIGHEST);

            std::shared_ptr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::MAGNIFICATION);
            CPPUNIT_ASSERT(msg.valid());
            SimCore::MagnificationMessage &magMsg = static_cast<SimCore::MagnificationMessage&>(*msg);
            magMsg.SetSource(*new dtGame::MachineInfo);
            const float mag = 3.42f;
            magMsg.SetMagnification(mag);
            float value = magMsg.GetMagnification();
            CPPUNIT_ASSERT_MESSAGE("GetMagnification should return what was set", value == mag);

            std::shared_ptr<SimCore::Actors::PlatformActorProxy> proxy;
            std::shared_ptr<SimCore::Actors::StealthActorProxy> stealthProxy;

            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy);
            CPPUNIT_ASSERT(proxy.valid());
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, stealthProxy);
            CPPUNIT_ASSERT(stealthProxy.valid());

            mGM->AddActor(*proxy, false, false);
            mGM->AddActor(*stealthProxy, false, false);

            mGM->SendMessage(magMsg);

            dtCore::AppSleep(5);
            dtCore::System::GetInstance().Step();

            osg::Vec3 defaultScale(1.0f, 1.0f, 1.0f), magScale(mag, mag, mag);
            osg::Vec3 scale = static_cast<SimCore::Actors::StealthActor&>(stealthProxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The stealth actor should NOT have had its scale set with the other actors", dtUtil::Equivalent(scale, defaultScale, 0.001f));

            scale = static_cast<SimCore::Actors::BaseEntity&>(proxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The proxy should have had its scale set",
               dtUtil::Equivalent(scale, magScale, 0.001f));

            magMsg.SetMagnification(defaultScale.x());
            mGM->SendMessage(magMsg);

            dtCore::AppSleep(5);
            dtCore::System::GetInstance().Step();

            scale = static_cast<SimCore::Actors::BaseEntity&>(proxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT(dtUtil::Equivalent(scale, defaultScale, 0.001f));

            // All changes backed out now, can test with input component
            std::shared_ptr<SimCore::Components::BaseInputComponent> bic = new SimCore::Components::BaseInputComponent("TestInputComponent");
            CPPUNIT_ASSERT(bic.valid());

            mGM->AddComponent(*bic, dtGame::GameManager::ComponentPriority::NORMAL);
            float BICscale = bic->GetEntityMagnification();

            CPPUNIT_ASSERT(BICscale == 1.0f);

            bic->HandleKeyPressed(nullptr, osgGA::GUIEventAdapter::KEY_Page_Up);

            BICscale = bic->GetEntityMagnification();

            CPPUNIT_ASSERT(BICscale == 2.0f);

            dtCore::AppSleep(5);
            dtCore::System::GetInstance().Step();

            scale = static_cast<SimCore::Actors::StealthActor&>(stealthProxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("Again, the stealth actor should NOT have scaled",
               dtUtil::Equivalent(scale, defaultScale, 0.001f));

            scale = static_cast<SimCore::Actors::BaseEntity&>(proxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The entity should have scaled to twice the BIC scale",
               dtUtil::Equivalent(scale, osg::Vec3(BICscale, BICscale, BICscale), 0.001f));

            const unsigned int size = 10;
            std::shared_ptr<SimCore::Actors::BaseEntityActorProxy> proxies[size];

            for(unsigned int i = 0; i < size; i++)
            {
               mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxies[i]);
               CPPUNIT_ASSERT(proxies[i].valid());
               mGM->AddActor(*proxies[i], false, false);
            }

            magMsg.SetMagnification(mag);
            mGM->SendMessage(magMsg);

            dtCore::AppSleep(5);
            dtCore::System::GetInstance().Step();

            for(unsigned int i = 0; i < size; i++)
            {
               scale = static_cast<SimCore::Actors::BaseEntity&>(proxies[i]->GetGameActor()).GetScaleMagnification();
               CPPUNIT_ASSERT_MESSAGE("Each entity should have been scaled",
                  dtUtil::Equivalent(scale, magScale, 0.001f));
               mGM->DeleteActor(*proxies[i]);
               proxies[i] = nullptr;
            }

            dtCore::UniqueId id;
            std::shared_ptr<dtGame::Message> rmtMsg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_CREATED);
            rmtMsg->SetAboutActorId(id);
            rmtMsg->SetSource(*new dtGame::MachineInfo);
            static_cast<dtGame::ActorUpdateMessage&>(*rmtMsg).SetActorType(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE);
            mGM->SendMessage(*rmtMsg);

            dtCore::AppSleep(5);
            dtCore::System::GetInstance().Step();

            scale = dynamic_cast<SimCore::Actors::BaseEntity&>(*mGM->FindGameActorById(id)->GetDrawable()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The new entity added should have been scaled automatically",
               dtUtil::Equivalent(scale, magScale, 0.001f));
         }
         catch(const dtUtil::Exception &e)
         {
            CPPUNIT_FAIL("Exception caught: " + e.What());
         }
      }

      void TestEmbeddedDataMessage()
      {
         try
         {

            std::shared_ptr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::INFO_EMBEDDED_DATA);
            CPPUNIT_ASSERT_MESSAGE("The message factory failed to create the message", msg.valid());

            SimCore::EmbeddedDataMessage* edm = static_cast<SimCore::EmbeddedDataMessage*>(msg.get());

            const std::string data("horch.83du982hu[;[h30938uh02huhal.8p2e23pi3a3i3233p3");
            edm->SetData(data);
            std::string fetchedData;
            edm->GetData(fetchedData);
            CPPUNIT_ASSERT_EQUAL(data, fetchedData);

            TestParameter<dtGame::StringMessageParameter, std::string>(*edm, SimCore::EmbeddedDataMessage::PARAM_DATA, data);

            unsigned short testVal = 21;
            edm->SetDataSize(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, edm->GetDataSize());

            TestParameter<dtGame::UnsignedShortIntMessageParameter, unsigned short>(*edm, SimCore::EmbeddedDataMessage::PARAM_DATA_SIZE, testVal);
         }
         catch(const dtUtil::Exception &ex)
         {
            CPPUNIT_FAIL("Exception caught: " + ex.What());
         }
      }

   private:

      std::shared_ptr<dtGame::GameManager> mGM;
      std::shared_ptr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MessageTests);

