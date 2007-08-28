/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology, BMH Operation.
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
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/invokable.h>

#include <dtCore/system.h>

#include <dtDAL/project.h>

#include <dtUtil/mathdefines.h>

#include <dtABC/application.h>

#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/Components/BaseInputComponent.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#if (defined (WIN32) || defined (_WIN32) || defined (__WIN32__))
   const std::string projectContext = "DVTEProject";
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   const std::string projectContext = "DVTEProject";
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif
 
const std::string &igRegistry = "IG";

using dtCore::RefPtr;
 
class MessageTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(MessageTests);

      CPPUNIT_TEST(TestStealthActorMessage);
		CPPUNIT_TEST(TestDetonationMessage);
      CPPUNIT_TEST(TestTimeQueryMessage);
      CPPUNIT_TEST(TestTimeValueMessage);
      CPPUNIT_TEST(TestAttachToActorMessage);
      CPPUNIT_TEST(TestDetachOnActorDeletedMessage);
      CPPUNIT_TEST(TestToolMessageTypes);
      CPPUNIT_TEST(TestMagnificationMessage);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         dtCore::System::GetInstance().Start();
         dtDAL::Project::GetInstance().SetContext(projectContext, true);
         mApp = new dtABC::Application;
         mGM = new dtGame::GameManager(*new dtCore::Scene());
         mGM->SetApplication(*mApp);
         mGM->LoadActorRegistry(igRegistry);
         RefPtr<dtGame::DeadReckoningComponent> drComp = new dtGame::DeadReckoningComponent;
         mGM->AddComponent(*drComp, dtGame::GameManager::ComponentPriority::NORMAL);
         
         SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());
      }
      
      void tearDown()
      {
         mGM->DeleteAllActors(true);
         mGM->UnloadActorRegistry(igRegistry);
         mGM = NULL;
         mApp = NULL;
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
            
            RefPtr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::STEALTH_ACTOR_FOV);
            CPPUNIT_ASSERT_MESSAGE("The message factory failed to create the stealth actor FOV message", msg.valid());

            SimCore::StealthActorUpdatedMessage *saum = static_cast<SimCore::StealthActorUpdatedMessage*>(msg.get());
            
            osg::Vec3 vec(1, 5, 4);
            
            saum->SetTranslation(vec);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTranslation should return what was set", vec, saum->GetTranslation());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::TRANSLATION) != NULL);

            saum->SetRotation(vec);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetRotation should return what was set",  vec, saum->GetRotation());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::ROTATION) != NULL);

            float testFloat = 14.55f;
            
            saum->SetHorizontalFOV(testFloat);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetHorizontalFOV should return what was set", testFloat, saum->GetHorizontalFOV());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::HORIZONTAL_FOV) != NULL);

            saum->SetVerticalFOV(testFloat);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetVerticalFOV should return what was set", testFloat, saum->GetVerticalFOV());
            CPPUNIT_ASSERT(saum->GetParameter(SimCore::StealthActorUpdatedMessage::HORIZONTAL_FOV) != NULL);

         }
         catch(const dtUtil::Exception &ex)
         {
            CPPUNIT_FAIL("Exception caught: " + ex.What());
         }
      }
      
      template <class T, class V>
      void TestParameter(dtGame::Message& msg, const std::string& name, V value)
      {
         RefPtr<dtGame::MessageParameter> param = msg.GetParameter(name);
         CPPUNIT_ASSERT(param.valid());
         
         CPPUNIT_ASSERT(dynamic_cast<T*>(param.get()) != NULL);
         CPPUNIT_ASSERT_EQUAL(value,
            static_cast<T*>(param.get())->GetValue());
      }

      void TestTimeQueryMessage()
      {
         try
         {
            RefPtr<SimCore::TimeQueryMessage> msg;
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
            RefPtr<SimCore::TimeValueMessage> msg;
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
            RefPtr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::DETONATION);
            
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
      
      void TestAttachToActorMessage()
      {
         try
         {
            RefPtr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
            if (!msg.valid())
               throw dtUtil::Exception(dtGame::ExceptionEnum::GENERAL_GAMEMANAGER_EXCEPTION, 
               "The message factory failed to create the attach to actor message", __FILE__, __LINE__);
            
            SimCore::AttachToActorMessage *aam = static_cast<SimCore::AttachToActorMessage*>(msg.get());
            
            dtCore::UniqueId id;
            
            aam->SetAttachToActor(id);
            CPPUNIT_ASSERT(aam->GetAttachToActor() == id);

            aam->SetAttachToActor(dtCore::UniqueId(""));
            CPPUNIT_ASSERT(aam->GetAttachToActor().ToString().empty());
         
            RefPtr<SimCore::Actors::PlatformActorProxy> t80Proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, t80Proxy);
            CPPUNIT_ASSERT(t80Proxy.valid());
            RefPtr<SimCore::Actors::BaseEntity> t80Actor = dynamic_cast<SimCore::Actors::BaseEntity*>(t80Proxy->GetActor());
            CPPUNIT_ASSERT(t80Actor.valid());

            RefPtr<SimCore::Actors::StealthActorProxy> playerProxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, playerProxy);
            CPPUNIT_ASSERT(playerProxy.valid());
            RefPtr<SimCore::Actors::StealthActor> playerActor = dynamic_cast<SimCore::Actors::StealthActor*>(playerProxy->GetActor());
            CPPUNIT_ASSERT(playerActor.valid());

            mGM->AddActor(*t80Proxy, true, false);
            mGM->AddActor(*playerProxy, true, false);

            dtCore::Transform someOtherPosition(1888.8, 1776, 1134.4311);
            t80Actor->SetTransform(someOtherPosition, dtCore::Transformable::REL_CS);

            dtCore::Transform originalTransform;
            playerActor->GetTransform(originalTransform);

            msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
            CPPUNIT_ASSERT(msg.valid());
            SimCore::AttachToActorMessage &atamsg = static_cast<SimCore::AttachToActorMessage&>(*msg);
            atamsg.SetAboutActorId(playerActor->GetUniqueId());
            RefPtr<dtGame::MachineInfo> machineInfo = new dtGame::MachineInfo;
            atamsg.SetSource(*machineInfo);
            atamsg.SetAttachToActor(t80Actor->GetUniqueId());
            mGM->SendMessage(atamsg);

            SLEEP(10);
            dtCore::System::GetInstance().Step();
            
            dtGame::Invokable *invoke = playerActor->GetGameActorProxy().GetInvokable("AttachToActor");
            CPPUNIT_ASSERT_MESSAGE("The AttachToActor invokable should not be NULL", invoke != NULL);
            invoke->Invoke(atamsg);

            dtCore::Transform newTransform, tankTransform;
            playerActor->GetTransform(newTransform);
            t80Actor->GetTransform(tankTransform);

            osg::Vec3 playerPos = originalTransform.GetTranslation(), 
                      newPos    = newTransform.GetTranslation(),
                      tankPos   = tankTransform.GetTranslation();
            
            //Checking the X and Y for now because the Z position is not the same.  It's up some.
            CPPUNIT_ASSERT_MESSAGE("The player X position should equal the tank position", newPos.x() == tankPos.x());
            CPPUNIT_ASSERT_MESSAGE("The player Y position should equal the tank position", newPos.y() == tankPos.y());

            CPPUNIT_ASSERT_MESSAGE("The old position should not equal the new position", playerPos != newPos);
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
            RefPtr<SimCore::Actors::PlatformActorProxy> t80Proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, t80Proxy);
            CPPUNIT_ASSERT(t80Proxy.valid());
            RefPtr<SimCore::Actors::BaseEntity> t80Actor = dynamic_cast<SimCore::Actors::BaseEntity*>(t80Proxy->GetActor());
            CPPUNIT_ASSERT(t80Actor.valid());
            RefPtr<SimCore::Actors::StealthActorProxy> playerProxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, playerProxy);
            CPPUNIT_ASSERT(playerProxy.valid());
            RefPtr<SimCore::Actors::StealthActor> playerActor = dynamic_cast<SimCore::Actors::StealthActor*>(playerProxy->GetActor());
            CPPUNIT_ASSERT(playerActor.valid());

            mGM->AddActor(*t80Proxy, true, false);
            mGM->AddActor(*playerProxy, true, false);

            static_cast<SimCore::Actors::StealthActor&>(playerProxy->GetGameActor()).SetAttachOffset(osg::Vec3(0, 0, 0));

            osg::Vec3 tankPos(0, 100, 0);
            t80Proxy->SetTranslation(tankPos);
            
            RefPtr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
            CPPUNIT_ASSERT(msg.valid());

            SimCore::AttachToActorMessage &aam = static_cast<SimCore::AttachToActorMessage&>(*msg);
            aam.SetAttachToActor(t80Proxy->GetId());
            aam.SetAboutActorId(playerProxy->GetId());
            RefPtr<dtGame::MachineInfo> machineInfo = new dtGame::MachineInfo;
            aam.SetSource(*machineInfo);

            mGM->SendMessage(aam);
            SLEEP(10);
            dtCore::System::GetInstance().Step();

            dtGame::Invokable *invoke = playerActor->GetGameActorProxy().GetInvokable("AttachToActor");
            CPPUNIT_ASSERT_MESSAGE("The AttachToActor invokable should not be NULL", invoke != NULL);
            invoke->Invoke(aam);

            CPPUNIT_ASSERT_MESSAGE("The player should now be attached to the T80", playerActor->GetParent() == t80Proxy->GetActor());
            
            msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_DELETED);
            CPPUNIT_ASSERT(msg.valid());
            msg->SetAboutActorId(t80Proxy->GetId());
            msg->SetSource(*machineInfo);
            mGM->SendMessage(*msg);

            SLEEP(10);
            dtCore::System::GetInstance().Step();
            
            invoke = playerActor->GetGameActorProxy().GetInvokable("Detach");
            CPPUNIT_ASSERT(invoke != NULL);
            invoke->Invoke(*msg);

            CPPUNIT_ASSERT_MESSAGE("The player should no longer be attached to the T80", playerActor->GetParent() == NULL);
            dtCore::Transform xform;
            playerActor->GetTransform(xform, dtCore::Transformable::ABS_CS);
            osg::Vec3 playerPos = xform.GetTranslation();
            
            CPPUNIT_ASSERT_MESSAGE("The player's position should not have changed", 
               dtUtil::Equivalent(playerPos, tankPos, 0.001f));
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

            RefPtr<dtGame::Message> msg = mGM->GetMessageFactory().CreateMessage(SimCore::MessageType::MAGNIFICATION);
            CPPUNIT_ASSERT(msg.valid());
            SimCore::MagnificationMessage &magMsg = static_cast<SimCore::MagnificationMessage&>(*msg);
            magMsg.SetSource(*new dtGame::MachineInfo);
            const float mag = 3.42f;
            magMsg.SetMagnification(mag);
            float value = magMsg.GetMagnification();
            CPPUNIT_ASSERT_MESSAGE("GetMagnification should return what was set", value == mag);

            RefPtr<SimCore::Actors::PlatformActorProxy> proxy;
            RefPtr<SimCore::Actors::StealthActorProxy> stealthProxy;

            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy);
            CPPUNIT_ASSERT(proxy.valid());
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, stealthProxy);
            CPPUNIT_ASSERT(stealthProxy.valid());

            mGM->AddActor(*proxy, false, false);
            mGM->AddActor(*stealthProxy, false, false);

            mGM->SendMessage(magMsg);

            SLEEP(5);
            dtCore::System::GetInstance().Step();

            osg::Vec3 defaultScale(1.0f, 1.0f, 1.0f), magScale(mag, mag, mag);
            osg::Vec3 scale = static_cast<SimCore::Actors::StealthActor&>(stealthProxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The stealth actor should NOT have had its scale set with the other actors", dtUtil::Equivalent(scale, defaultScale, 0.001f));
         
            scale = static_cast<SimCore::Actors::BaseEntity&>(proxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The proxy should have had its scale set", 
               dtUtil::Equivalent(scale, magScale, 0.001f));

            magMsg.SetMagnification(defaultScale.x());
            mGM->SendMessage(magMsg);

            SLEEP(5);
            dtCore::System::GetInstance().Step();

            scale = static_cast<SimCore::Actors::BaseEntity&>(proxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT(dtUtil::Equivalent(scale, defaultScale, 0.001f));

            // All changes backed out now, can test with input component
            RefPtr<SimCore::Components::BaseInputComponent> bic = new SimCore::Components::BaseInputComponent("TestInputComponent");
            CPPUNIT_ASSERT(bic.valid());

            mGM->AddComponent(*bic, dtGame::GameManager::ComponentPriority::NORMAL);
            float BICscale = bic->GetEntityMagnification();
            
            CPPUNIT_ASSERT(BICscale == 1.0f);

            bic->HandleKeyPressed(NULL, Producer::Key_Page_Up, Producer::KeyChar_Page_Up);
            
            BICscale = bic->GetEntityMagnification();

            CPPUNIT_ASSERT(BICscale == 2.0f);

            SLEEP(5);
            dtCore::System::GetInstance().Step();

            scale = static_cast<SimCore::Actors::StealthActor&>(stealthProxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("Again, the stealth actor should NOT have scaled", 
               dtUtil::Equivalent(scale, defaultScale, 0.001f));

            scale = static_cast<SimCore::Actors::BaseEntity&>(proxy->GetGameActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The entity should have scaled to twice the BIC scale", 
               dtUtil::Equivalent(scale, osg::Vec3(BICscale, BICscale, BICscale), 0.001f));

            const unsigned int size = 10;
            RefPtr<SimCore::Actors::BaseEntityActorProxy> proxies[size];

            for(unsigned int i = 0; i < size; i++)
            {
               mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxies[i]);
               CPPUNIT_ASSERT(proxies[i].valid());
               mGM->AddActor(*proxies[i], false, false);
            }

            magMsg.SetMagnification(mag);
            mGM->SendMessage(magMsg);

            SLEEP(5);
            dtCore::System::GetInstance().Step();

            for(unsigned int i = 0; i < size; i++)
            {
               scale = static_cast<SimCore::Actors::BaseEntity&>(proxies[i]->GetGameActor()).GetScaleMagnification();
               CPPUNIT_ASSERT_MESSAGE("Each entity should have been scaled", 
                  dtUtil::Equivalent(scale, magScale, 0.001f));
               mGM->DeleteActor(*proxies[i]);
               proxies[i] = NULL;
            }
             
            dtCore::UniqueId id;
            RefPtr<dtGame::Message> rmtMsg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_CREATED);
            rmtMsg->SetAboutActorId(id);
            rmtMsg->SetSource(*new dtGame::MachineInfo);
            static_cast<dtGame::ActorUpdateMessage&>(*rmtMsg).SetActorType(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE);
            mGM->SendMessage(*rmtMsg);

            SLEEP(5);
            dtCore::System::GetInstance().Step();

            scale = dynamic_cast<SimCore::Actors::BaseEntity&>(*mGM->FindGameActorById(id)->GetActor()).GetScaleMagnification();
            CPPUNIT_ASSERT_MESSAGE("The new entity added should have been scaled automatically", 
               dtUtil::Equivalent(scale, magScale, 0.001f));
         }
         catch(const dtUtil::Exception &e)
         {
            CPPUNIT_FAIL("Exception caught: " + e.What());
         }
      }

   private:

      RefPtr<dtGame::GameManager> mGM;
      RefPtr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MessageTests);

