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
 * @author David Guthrie
 */
#include <prefix/SimCorePrefix-src.h>
#include <string>
#include <vector>
#include <set>

#include <cppunit/extensions/HelperMacros.h>

#include <dtABC/application.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h> 
#include <dtGame/messagefactory.h> 
#include <dtGame/exceptionenum.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/deltawin.h>

#include <dtUtil/mathdefines.h>

#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/ViewerNetworkPublishingComponent.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/StealthActor.h>

#include "TestComponent.h"

using dtCore::RefPtr;

class ViewerNetworkPublishingComponentTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(ViewerNetworkPublishingComponentTests);

      CPPUNIT_TEST(TestStealthActorMessages);
      CPPUNIT_TEST(TestTimeQueryMessagePropagation);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         mApp = new dtABC::Application("config.xml");
         mApp->GetWindow()->SetPosition(0, 0, 50, 50);
         mGM = new dtGame::GameManager(*new dtCore::Scene());
         mGM->SetApplication(*mApp);
         RefPtr<SimCore::Components::ViewerNetworkPublishingComponent> rulesComp = new SimCore::Components::ViewerNetworkPublishingComponent;         
         mGM->AddComponent(*rulesComp, dtGame::GameManager::ComponentPriority::NORMAL);
         mTestComp = new TestComponent;
         mGM->AddComponent(*mTestComp, dtGame::GameManager::ComponentPriority::NORMAL);
         
         SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());
         
         RefPtr<dtDAL::ActorProxy> ap = mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE);
         
         CPPUNIT_ASSERT(ap.valid());
         
         mStealthActor = 
            dynamic_cast<SimCore::Actors::StealthActor*>(ap->GetActor());
            
         CPPUNIT_ASSERT(mStealthActor.valid());
         mGM->AddActor(mStealthActor->GetGameActorProxy(), false, true);
                  
         dtCore::System::GetInstance().Start();
      }
      
      void tearDown()
      {
         dtCore::System::GetInstance().Stop();
         
         mTestComp = NULL;
         mApp = NULL;
         mGM->DeleteAllActors(true);
         mGM = NULL;
      }

      void TestStealthActorMessages()
      {
         InternalTestStealthActorMessages(false);
         
         std::vector<std::string> params;
         
         params.push_back("Last Known Rotation");
         params.push_back("Last Known Translation");
         
         InternalTestStealthActorMessages(true, params);
         
         params.clear();         
         params.push_back("Last Known Rotation");
         
         InternalTestStealthActorMessages(true, params);

         params.clear();         
         params.push_back("Last Known Translation");
         
         InternalTestStealthActorMessages(true, params);

         params.clear();         
         
         InternalTestStealthActorMessages(true, params);

      }

      void TestTimeQueryMessagePropagation()
      {
         dtGame::MessageFactory& mf = mGM->GetMessageFactory();
         RefPtr<SimCore::TimeQueryMessage> timeQuery;
         mf.CreateMessage(SimCore::MessageType::TIME_QUERY, timeQuery);
         mGM->SendMessage(*timeQuery);
         mTestComp->reset();
         dtCore::System::GetInstance().Step();
         dtCore::System::GetInstance().Step();
         
         const dtGame::Message* foundMessage = mTestComp->FindDispatchNetworkMessageOfType(SimCore::MessageType::TIME_QUERY).get();

         CPPUNIT_ASSERT_MESSAGE("A time query message should have been dispatched to the network.", foundMessage != NULL);
         CPPUNIT_ASSERT_MESSAGE("The time query message dispatched should be the same as the one sent.",
               *foundMessage == *timeQuery);
      }
      
   private:
   
      void InternalTestStealthActorMessages(bool partial, const std::vector<std::string>& params = std::vector<std::string>())
      {
         mTestComp->reset();

         osg::Vec3 testVec(1.1f, 2.2f, 3.3f);
         //account for the different order of rotation vecs.
         osg::Vec3 testRot(testVec.z(), testVec.x(), testVec.y());

         //Must set both the last known and the actual values so the message and 
         //and actor will match after being stepped
         mStealthActor->SetLastKnownTranslation(testVec);
         mStealthActor->SetLastKnownRotation(testRot);

         dtCore::Transform xform;
         mStealthActor->GetTransform(xform, dtCore::Transformable::ABS_CS);
         xform.SetTranslation(testVec);
         xform.SetRotation(testRot);
         mStealthActor->SetTransform(xform, dtCore::Transformable::ABS_CS);

         if (partial)
         {
            RefPtr<dtGame::ActorUpdateMessage> updateMessage = 
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
            RefPtr<const SimCore::StealthActorUpdatedMessage> msg = 
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

            CPPUNIT_ASSERT_MESSAGE("No stealth actor rotation message should have been sent",
              !result);
         }

         if (!partial || paramSet.find("Last Known Translation") != paramSet.end())
         {
            RefPtr<const SimCore::StealthActorUpdatedMessage> msg = 
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
            CPPUNIT_ASSERT_MESSAGE("No stealth actor Translation message should have been sent",
              !result);
         }
                                    
         mTestComp->reset();
         
      }
      
      RefPtr<dtGame::GameManager> mGM;
      RefPtr<TestComponent> mTestComp;
      RefPtr<SimCore::Actors::StealthActor> mStealthActor;
      RefPtr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ViewerNetworkPublishingComponentTests);
