/* -*-c++-*-
* Simulation Core - TimedDeleterComponentTests (.h & .cpp) - Using 'The MIT License'
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
* @author Chris Rodgers
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtCore/system.h>
#include <dtUtil/refcountedbase.h>
#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>
#include <dtCore/timer.h>
#include <dtDAL/project.h>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/MessageType.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

namespace SimCore
{
   namespace Components
   {

      //////////////////////////////////////////////////////////////
      // Sub-classed TimedDeleterComponent
      //////////////////////////////////////////////////////////////

      // The following class is used to access protected functions
      // of the TimedDeleterComponent.
      class TestTimedDeleterComponent : public TimedDeleterComponent
      {
         public:
            TestTimedDeleterComponent();

            const std::string CreateTimerNameFromId( const dtCore::UniqueId& id ) const;

            void AddId( const dtCore::UniqueId& id, double simWaitTime );

            // The following returns the ids in the order they were created.
            // This is neede for a later test.
            void GetAddedIds( std::vector<dtCore::UniqueId>& listToFill ) const;

            void GetDeletedIds( std::vector<dtCore::UniqueId>& listToFill ) const;

            void RemoveId( const dtCore::UniqueId& id );

            void RemoveIdByTimerName( const std::string& timerName );

            void Clear();

            void SetAvoidTimeChangeMessage(bool avoid);

            void ProcessMessage( const dtGame::Message& message );

         protected:
            ~TestTimedDeleterComponent();

         private:
            std::vector<dtCore::UniqueId> mAddedIds;
            std::vector<dtCore::UniqueId> mDeletedIds;
            bool mAvoidTimeChangeMessage;
      };


      //////////////////////////////////////////////////////////////
      // Tests Object
      //////////////////////////////////////////////////////////////

      class TimedDeleterComponentTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(TimedDeleterComponentTests);

         CPPUNIT_TEST(TestAddAndRemove);
         CPPUNIT_TEST(TestTimedDeletes);
         CPPUNIT_TEST(TestMessageProcessing);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            void CreateTestActors( std::vector<std::shared_ptr<dtGame::GameActorProxy> >& listToFill, int numActors, bool putInGameManager = false );

            // Returns the total IDs registered to the component
            int RegisterActorIds( const std::vector<std::shared_ptr<dtGame::GameActorProxy> >& actorList );
            void SetSimTime( double newSimTime );
            void AdvanceSimTime( double deltaTime );

            void TestAddAndRemove();
            void TestTimedDeletes();
            void TestMessageProcessing();

         protected:
         private:

            std::shared_ptr<dtGame::GameManager> mGM;
            std::shared_ptr<TestTimedDeleterComponent> mDeleterComp;
            std::shared_ptr<dtGame::MachineInfo> mMachineInfo;
            std::vector<std::shared_ptr<dtGame::GameActorProxy> > mTestActors;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(TimedDeleterComponentTests);



      //////////////////////////////////////////////////////////////
      // Sub-classed object code
      //////////////////////////////////////////////////////////////

      TestTimedDeleterComponent::TestTimedDeleterComponent()
         : mAvoidTimeChangeMessage(false)
      {

      }

      //////////////////////////////////////////////////////////////
      TestTimedDeleterComponent::~TestTimedDeleterComponent()
      {

      }

      //////////////////////////////////////////////////////////////
      const std::string TestTimedDeleterComponent::CreateTimerNameFromId( const dtCore::UniqueId& id ) const
      {
         return TimedDeleterComponent::CreateTimerNameFromId(id);
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::AddId( const dtCore::UniqueId& id, double simWaitTime )
      {
         mAddedIds.push_back(id);
         TimedDeleterComponent::AddId( id, simWaitTime );
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::RemoveId( const dtCore::UniqueId& id )
      {
         mDeletedIds.push_back(id);
         TimedDeleterComponent::RemoveId(id);
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::RemoveIdByTimerName( const std::string& timerName )
      {
         // The following line depends on the generated name of a timer
         // being equal to the component name with the id appended.
         // +5 for "actor" that precedes the id.
         std::string idStr = timerName.substr(GetName().length());
         //std::cout << "Deleting index: " << idStr.c_str() << "\n\n";

         // This function internally calls the function RemoveId.
         TimedDeleterComponent::RemoveIdByTimerName(timerName);
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::Clear()
      {
         mAddedIds.clear();
         mDeletedIds.clear();
         TimedDeleterComponent::Clear();
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::SetAvoidTimeChangeMessage(bool avoid)
      {
         mAvoidTimeChangeMessage = avoid;
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::ProcessMessage( const dtGame::Message& message )
      {
         if( mAvoidTimeChangeMessage &&
            message.GetMessageType() == dtGame::MessageType::INFO_TIME_CHANGED )
         {
            return;
         }
         TimedDeleterComponent::ProcessMessage(message);
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::GetDeletedIds( std::vector<dtCore::UniqueId>& listToFill ) const
      {
         int limit = mDeletedIds.size();
         for( int id = 0; id < limit; id++ )
         {
            listToFill.push_back(mDeletedIds[id]);
         }
      }

      //////////////////////////////////////////////////////////////
      void TestTimedDeleterComponent::GetAddedIds( std::vector<dtCore::UniqueId>& listToFill ) const
      {
         int limit = mAddedIds.size();
         for( int id = 0; id < limit; id++ )
         {
            listToFill.push_back(mAddedIds[id]);
         }
      }


      //////////////////////////////////////////////////////////////
      // Tests code
      //////////////////////////////////////////////////////////////

      void TimedDeleterComponentTests::setUp()
      {
         try
         {
            dtCore::System::GetInstance().Start();
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

            mMachineInfo = new dtGame::MachineInfo;
            mDeleterComp = new TestTimedDeleterComponent;

            mGM->AddComponent(*mDeleterComp, dtGame::GameManager::ComponentPriority::NORMAL);
            mGM->AddComponent(*new dtGame::DeadReckoningComponent(),
                     dtGame::GameManager::ComponentPriority::NORMAL);
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }
      }

      //////////////////////////////////////////////////////////////
      void TimedDeleterComponentTests::tearDown()
      {
         mTestActors.clear();

         dtCore::System::GetInstance().SetPause(false);
         dtCore::System::GetInstance().Stop();

         mGM->DeleteAllActors(true);

         mGM = nullptr;
         mMachineInfo = nullptr;
      }


      //////////////////////////////////////////////////////////////
      void TimedDeleterComponentTests::CreateTestActors(
         std::vector<std::shared_ptr<dtGame::GameActorProxy> >& listToFill,
         int numActors, bool putInGameManager )
      {
         // Prepare lists memory
         listToFill.clear();
         listToFill.reserve(numActors);

         // Create the actors and add them to the GM

         std::shared_ptr<dtGame::GameActorProxy> curProxy;
         for( int index = 0; index < numActors; index++ )
         {
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE, curProxy);

            listToFill.push_back( curProxy.get() );

            if(putInGameManager)
            {
               mGM->AddActor(*curProxy, false, false);
            }
         }
      }


      //////////////////////////////////////////////////////////////
      int TimedDeleterComponentTests::RegisterActorIds( const std::vector<std::shared_ptr<dtGame::GameActorProxy> >& actorList )
      {
         int numActors = actorList.size();
         for( int actor = 0; actor < numActors; actor++ )
         {
            if(actorList[actor].valid())
            {
               // Time set to 10 seconds as a long life.
               // This function is mostly to test addition and removal
               // of IDs to the Deleter component.
               mDeleterComp->AddId(actorList[actor]->GetId(), 10.0);
            }
         }
         return mDeleterComp->GetIdCount();
      }

      //////////////////////////////////////////////////////////////
      void TimedDeleterComponentTests::SetSimTime( double newSimTime )
      {
         mGM->ChangeTimeSettings( newSimTime, mGM->GetTimeScale(), mGM->GetSimulationClockTime() );
         dtCore::AppSleep(1); dtCore::System::GetInstance().Step();
      }

      //////////////////////////////////////////////////////////////
      void TimedDeleterComponentTests::AdvanceSimTime( double deltaTime )
      {
         dtCore::AppSleep(10); dtCore::System::GetInstance().Step();
         //SetSimTime( mGM->GetSimulationTime() + deltaTime );
      }

      //////////////////////////////////////////////////////////////
      void TimedDeleterComponentTests::TestAddAndRemove()
      {
         // Create actors (at least 6) and add to GM
         unsigned maxActors = 6;
         CreateTestActors( mTestActors, maxActors );

         // Loop - Add a few actors to component
         unsigned componentNameLength = mDeleterComp->GetName().length();
         dtCore::UniqueId curID;
         std::string curTimerName;
         for( unsigned actor = 0; actor < maxActors; ++actor )
         {
            curID = mTestActors[actor]->GetId();

            // -- Test CreateTimerNameFromId
            curTimerName = mDeleterComp->CreateTimerNameFromId( curID );
            CPPUNIT_ASSERT_MESSAGE("Generated timer name should be valid",
               curTimerName.length() > componentNameLength );

            // -- Add actor to component
            mDeleterComp->AddId( curID, 1.0 );
            CPPUNIT_ASSERT_MESSAGE("Adding an ID should increase count", mDeleterComp->GetIdCount() == actor+1 );

            // -- Test GetIdIndex & GetTimerNameIndex
            CPPUNIT_ASSERT_MESSAGE("Test ID existence", mDeleterComp->HasId(curID) );
            CPPUNIT_ASSERT_MESSAGE("Test Timer existence", mDeleterComp->HasTimer(curTimerName) );
         }

         // Get all actor IDs
         std::vector<dtCore::UniqueId> ids;
         mDeleterComp->GetIds(ids);
         CPPUNIT_ASSERT_MESSAGE("Number of IDs registered should match the number of IDs returned",
            mDeleterComp->GetIdCount() == ids.size() );

         // Remove the actors
         for( unsigned actor = 0; actor < maxActors; ++actor )
         {
            curID = ids[actor];

            // -- Test GetAssociatedTimerName
            std::string curStrPtr = mDeleterComp->GetAssociatedTimerName(curID);
            CPPUNIT_ASSERT_MESSAGE("Returned timer name should be valid",
               curStrPtr.length() > componentNameLength );

            // Remove actor from component
            if( (actor % 2) == 0 )
            {
               mDeleterComp->RemoveIdByTimerName( curStrPtr );
            }
            else
            {
               mDeleterComp->RemoveId( curID );
            }

            // Test count
            CPPUNIT_ASSERT_MESSAGE("Removing an ID should decrease count",
               mDeleterComp->GetIdCount() == maxActors - actor - 1 );

         }



         // Add all actors again
         CreateTestActors( mTestActors, maxActors );
         CPPUNIT_ASSERT_MESSAGE("All actors should be registered",
            RegisterActorIds(mTestActors) == (int)maxActors );

         // Gather ids again
         mDeleterComp->GetIds(ids);

         // Test a single remove by ID
         // -- get an id from somewhere in the middle
         curID = ids[2];
         curTimerName = mDeleterComp->GetAssociatedTimerName(curID);
         mDeleterComp->RemoveId(curID);
         CPPUNIT_ASSERT_MESSAGE("Both the ID and timer should have been removed",
            !mDeleterComp->HasId(curID) && !mDeleterComp->HasTimer(curTimerName) );

         // Test a single remove by timer name
         // -- get an id from somewhere in the middle
         curID = ids[4];
         curTimerName = mDeleterComp->GetAssociatedTimerName(curID);
         mDeleterComp->RemoveIdByTimerName(curTimerName);
         CPPUNIT_ASSERT_MESSAGE("Both the timer and ID should have been removed",
            !mDeleterComp->HasId(curID) && !mDeleterComp->HasTimer(curTimerName) );


         // Check count
         CPPUNIT_ASSERT_MESSAGE("Only 2 IDs should have been removed",
            mDeleterComp->GetIdCount() == maxActors-2 );


         // Test Clear - Check count
         mDeleterComp->Clear();
         CPPUNIT_ASSERT_MESSAGE("All ids should be removed",
            mDeleterComp->GetIdCount() == 0 );
         CPPUNIT_ASSERT_MESSAGE("All actors should be removed from the GameManager",
            mGM->GetNumGameActors() == 0 );
      }

      //////////////////////////////////////////////////////////////
      void TimedDeleterComponentTests::TestMessageProcessing()
      {
         int maxActors = 6;
         CreateTestActors(mTestActors,maxActors);

         std::shared_ptr<dtGame::Message> msg =
            mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_DELETED);


         // Test ACTOR_DELETED message
         RegisterActorIds(mTestActors);
         msg->SetAboutActorId(mTestActors[0]->GetId());
         mDeleterComp->ProcessMessage(*msg);
         CPPUNIT_ASSERT_MESSAGE("Only one ID should be removed",
            (int)mDeleterComp->GetIdCount() == maxActors-1 );

         // Test TIME_ELAPSED message
         msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_TIMER_ELAPSED);
         dtGame::TimerElapsedMessage& timeElapseMessage =
            static_cast<dtGame::TimerElapsedMessage&> (*msg);
         timeElapseMessage.SetTimerName( mDeleterComp->GetAssociatedTimerName(mTestActors[1]->GetId()) );
         mDeleterComp->ProcessMessage(timeElapseMessage);
         CPPUNIT_ASSERT_MESSAGE("Only one ID should be removed",
            (int)mDeleterComp->GetIdCount() == maxActors-2 );

         // Test TIME_CAHNGED message
         // This is no longer the correct behavior. Timers do not get deleted when the time changes.
         //CreateTestActors(mTestActors,maxActors);
         //RegisterActorIds(mTestActors);
         //msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_TIME_CHANGED);
         //mDeleterComp->ProcessMessage(*msg);
         //CPPUNIT_ASSERT_MESSAGE("All ids should be removed",
         //   mDeleterComp->GetIdCount() == 0 );

         // Test MAP_LOAD message
         CreateTestActors(mTestActors,maxActors);
         RegisterActorIds(mTestActors);
         msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_MAP_LOADED);
         mDeleterComp->ProcessMessage(*msg);
         CPPUNIT_ASSERT_MESSAGE("All ids should be removed",
            mDeleterComp->GetIdCount() == 0 );

         // Test MAP_UNLOAD messages
         CreateTestActors(mTestActors,maxActors);
         RegisterActorIds(mTestActors);
         msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_MAP_UNLOADED);
         mDeleterComp->ProcessMessage(*msg);
         CPPUNIT_ASSERT_MESSAGE("All ids should be removed",
            mDeleterComp->GetIdCount() == 0 );

         // Test RESTART message
         CreateTestActors(mTestActors,maxActors);
         RegisterActorIds(mTestActors);
         msg = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_RESTARTED);
         mDeleterComp->ProcessMessage(*msg);
         CPPUNIT_ASSERT_MESSAGE("All ids should be removed",
            mDeleterComp->GetIdCount() == 0 );
      }

      //////////////////////////////////////////////////////////////
      void TimedDeleterComponentTests::TestTimedDeletes()
      {
         // Prevent time changes from clearing the component lists.
         mDeleterComp->SetAvoidTimeChangeMessage(true);
         // Process RESTART message
         dtCore::AppSleep(10); dtCore::System::GetInstance().Step();

         // Create actors (at least 6) and add to GM
         const int maxActors = 6;
         CreateTestActors( mTestActors, maxActors, true );

         // Set delete times
         mGM->SetPaused(false);
         SetSimTime(0.0);
         mDeleterComp->AddId( mTestActors[0]->GetId(), 0.01 );
         mDeleterComp->AddId( mTestActors[1]->GetId(), 0.04 );
         mDeleterComp->AddId( mTestActors[2]->GetId(), 0.02 );
         mDeleterComp->AddId( mTestActors[3]->GetId(), 0.05 );
         mDeleterComp->AddId( mTestActors[4]->GetId(), 0.03 );
         mDeleterComp->AddId( mTestActors[5]->GetId(), 0.06 );
         // Order of deletes should be:
         // 0, 2, 4, 1, 3, 5

         // Step and check correct delete order
         double timeStep = 0.1;

         // -- Step time forward & check if ID is gone
         AdvanceSimTime(timeStep);
         CPPUNIT_ASSERT_MESSAGE("ID 0 should be deleted",
            !mDeleterComp->HasId(mTestActors[0]->GetId()));
         // -- Step time forward & check if ID is gone
         AdvanceSimTime(timeStep);
         CPPUNIT_ASSERT_MESSAGE("ID 2 should be deleted",
            !mDeleterComp->HasId(mTestActors[2]->GetId()));
         // -- Step time forward & check if ID is gone
         AdvanceSimTime(timeStep);
         CPPUNIT_ASSERT_MESSAGE("ID 4 should be deleted",
            !mDeleterComp->HasId(mTestActors[4]->GetId()));
         // -- Step time forward & check if ID is gone
         AdvanceSimTime(timeStep);
         CPPUNIT_ASSERT_MESSAGE("ID 1 should be deleted",
            !mDeleterComp->HasId(mTestActors[1]->GetId()));
         // -- Step time forward & check if ID is gone
         AdvanceSimTime(timeStep);
         CPPUNIT_ASSERT_MESSAGE("ID 3 should be deleted",
            !mDeleterComp->HasId(mTestActors[3]->GetId()));
         // -- Step time forward & check if ID is gone
         AdvanceSimTime(timeStep);
         CPPUNIT_ASSERT_MESSAGE("ID 5 should be deleted",
            !mDeleterComp->HasId(mTestActors[5]->GetId()) );
         AdvanceSimTime(timeStep);


         // Test deleted order
         /* The following assumed that returned ids would be
         in the same order as contained by the component (when using vectors).
         Now the component uses maps, so the ids may not be in the order as they
         were added initially. The sub-classed deleter will contain the original order.*/

         // Get the order of the IDs
         std::vector<dtCore::UniqueId> ids;
         mDeleterComp->GetAddedIds(ids);
         std::vector<dtCore::UniqueId> deletedIds;
         mDeleterComp->GetDeletedIds(deletedIds);

         std::cout << "Total deletes: " << deletedIds.size() << "\n";
         CPPUNIT_ASSERT_MESSAGE("ID 0 should be deleted",
            deletedIds[0] == ids[0] );
         CPPUNIT_ASSERT_MESSAGE("ID 2 should be deleted",
            deletedIds[1] == ids[2] );
         CPPUNIT_ASSERT_MESSAGE("ID 4 should be deleted",
            deletedIds[2] == ids[4] );
         CPPUNIT_ASSERT_MESSAGE("ID 1 should be deleted",
            deletedIds[3] == ids[1] );
         CPPUNIT_ASSERT_MESSAGE("ID 3 should be deleted",
            deletedIds[4] == ids[3] );
         CPPUNIT_ASSERT_MESSAGE("ID 5 should be deleted",
            deletedIds[5] == ids[5] );//*/


         // Check that all actors timers and ids are gone.
         CPPUNIT_ASSERT_MESSAGE("All ids should be removed",
            mDeleterComp->GetIdCount() == 0 );
         CPPUNIT_ASSERT_MESSAGE("All actors should be removed from the GameManager",
            mGM->GetNumGameActors() == 0 );//*/



         // Create new actors and add to GM again
         CreateTestActors( mTestActors, maxActors, true );
         RegisterActorIds( mTestActors );
         dtCore::System::GetInstance().Step();

         // Remove a couple actors from GM
         mGM->DeleteActor(*mTestActors[1]);
         mGM->DeleteActor(*mTestActors[3]);
         dtCore::AppSleep(10); dtCore::System::GetInstance().Step();

         // Check component and GM count
         CPPUNIT_ASSERT_MESSAGE("2 actors should be removed from the GameManager",
            mGM->GetNumGameActors() == 4 );
         CPPUNIT_ASSERT_MESSAGE("2 ids should be removed",
            mDeleterComp->GetIdCount() == 4 );

         // Remove all game actors from GM
         mGM->DeleteAllActors(false);
         dtCore::AppSleep(20); dtCore::System::GetInstance().Step();

         // Check component and GM count
         CPPUNIT_ASSERT_MESSAGE("All ids should be removed",
            mDeleterComp->GetIdCount() == 0 );
         CPPUNIT_ASSERT_MESSAGE("All actors should be removed from the GameManager",
            mGM->GetNumGameActors() == 0 );
      }

   }
}
