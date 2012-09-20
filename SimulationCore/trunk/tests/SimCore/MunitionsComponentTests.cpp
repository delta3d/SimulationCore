/* -*-c++-*-
* Simulation Core - MunitionsComponentTests (.h & .cpp) - Using 'The MIT License'
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

#include <osg/io_utils>
#include <dtCore/system.h>
#include <dtCore/refptr.h>
#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>
#include <dtCore/timer.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/project.h>
#include <dtDAL/resourcedescriptor.h>

#include <dtGame/basemessages.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>

#include <SimCore/Components/DamageHelper.h>
#include <SimCore/Components/MunitionDamage.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>

#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <dtUtil/mathdefines.h>
#include <UnitTestMain.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {

      //////////////////////////////////////////////////////////////////////////
      // Sub-classed MunitionsComponent
      //////////////////////////////////////////////////////////////////////////

      // The following class is used to access protected functions
      // of the MunitionsComponent.
      class TestMunitionsComponent : public MunitionsComponent
      {
         public:
            TestMunitionsComponent();

            // This function exposes a protected function
            void SetMunitionTypeTable( dtCore::RefPtr<MunitionTypeTable>& table )
            { MunitionsComponent::SetMunitionTypeTable(table); }

            // Override CreateDamageHelper to allow this component to
            // use a TestDamageHelper.
            virtual DamageHelper* CreateDamageHelper(
               SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork, float maxDamageAmount = 1.0f);

            // Override ProcessMessage to capture all messages
            virtual void ProcessMessage( const dtGame::Message& message );

            unsigned GetTotalProcessedMessagesForActor( const dtCore::UniqueId& actorID ) const;

            // @return Total of all actor update messages that passed through ProcessMessage
            unsigned GetTotalProcessedMessages() const;

            void ResetTotalProcessedMessages();

         protected:
            virtual ~TestMunitionsComponent();

         private:
            // Total actor update messages processed
            typedef std::map<dtCore::UniqueId, unsigned> MessageCountMap;
            MessageCountMap mMessageCounts;
      };



      //////////////////////////////////////////////////////////////////////////
      // Sub-classed DamageHelper
      //////////////////////////////////////////////////////////////////////////

      // The following class is used to change damage probability
      // number generation of the DamageHelper.
      class TestDamageHelper : public DamageHelper
      {
         public:
            TestDamageHelper( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork = true,
               float maxDamage = 1.0f);

            // Override GetDamageProbability to prevent random numbers
            virtual float GetDamageProbability( bool directFire ) const;

            // @param probability The probability that GetDamageProbability should return
            void SetUsedProbability( float probability ) { mUsedProbability = probability; }
            float GetUsedProbability() const { return mUsedProbability; }

         protected:
            virtual ~TestDamageHelper() {}

         private:
            float mUsedProbability;
      };



      //////////////////////////////////////////////////////////////////////////
      // Tests Object
      //////////////////////////////////////////////////////////////////////////

      class MunitionsComponentTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(MunitionsComponentTests);

         CPPUNIT_TEST(TestDISIdentifierProperties);
         CPPUNIT_TEST(TestDamageProbabilitiyProperties);
         CPPUNIT_TEST(TestDamageRangesProperties);
         CPPUNIT_TEST(TestMunitionDamageProperties);
         CPPUNIT_TEST(TestMunitionDamageTableProperties);
         CPPUNIT_TEST(TestMunitionTypeTableProperties);
         CPPUNIT_TEST(TestDamageHelperProperties);
         CPPUNIT_TEST(TestMunitionsComponentProperties);
         CPPUNIT_TEST(TestDefaultMunition);
         CPPUNIT_TEST(TestMessagingDisabled);
         CPPUNIT_TEST(TestMessageProcessing);
         CPPUNIT_TEST(TestMunitionConfigLoading);
         CPPUNIT_TEST(TestMunitionEffectsInfoActorProperties);
         CPPUNIT_TEST(TestMunitionFamilyProperties);
         CPPUNIT_TEST(TestMunitionTypeActorProperties);
         CPPUNIT_TEST(TestTracerEffectProperties);
         CPPUNIT_TEST(TestWeaponEffectProperties);
         CPPUNIT_TEST(TestWeaponEffectManagerProperties);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            // Utility Function
            void CreateTestEntities(
               std::vector<dtCore::RefPtr<SimCore::Actors::BaseEntity> >& listToFill,
               int numActors, bool putInGameManager = false );

            // Utility Function
            void SendDetonationMessage( const std::string& munitionName,
               const osg::Vec3& detonationLocation, const SimCore::Actors::BaseEntity* targetEntity,
               const osg::Vec3* trajectory );

            // Utility Function
            void MoveEntity( SimCore::Actors::BaseEntity& entity, const osg::Vec3& newLocation );

            // Test Functions:
            void TestDISIdentifierProperties();
            void TestDamageProbabilitiyProperties();
            void TestDamageRangesProperties();
            void TestMunitionDamageProperties();
            void TestMunitionDamageTableProperties();
            void TestMunitionTypeTableProperties();
            void TestDamageHelperProperties();
            void TestMunitionsComponentProperties();
            void TestDefaultMunition();
            void TestMessagingDisabled();
            void TestMessageProcessing();
            void TestMunitionConfigLoading();
            void TestMunitionEffectsInfoActorProperties();
            void TestMunitionFamilyProperties();
            void TestMunitionTypeActorProperties();
            void TestTracerEffectProperties();
            void TestWeaponEffectProperties();
            void TestWeaponEffectManagerProperties();

            void TestDetonationActor();

         protected:
         private:
            static const std::string VEHICLE_MUNITION_TABLE_NAME;
            dtCore::RefPtr<SimCore::Actors::BaseEntity> SetupTestEntityAndDamageHelper(bool autoSendMessages);

            dtCore::RefPtr<dtGame::GameManager> mGM;
            dtCore::RefPtr<TestMunitionsComponent> mDamageComp;
            dtCore::RefPtr<dtGame::MachineInfo> mMachineInfo;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(MunitionsComponentTests);

      const std::string MunitionsComponentTests::VEHICLE_MUNITION_TABLE_NAME("UnitTestsVehicle");



      //////////////////////////////////////////////////////////////////////////
      // Sub-classed MunitionsComponent Code
      //////////////////////////////////////////////////////////////////////////
      TestMunitionsComponent::TestMunitionsComponent()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      TestMunitionsComponent::~TestMunitionsComponent()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper* TestMunitionsComponent::CreateDamageHelper( SimCore::Actors::BaseEntity& entity,
         bool autoNotifyNetwork, float maxDamageAmount)
      {
         return new TestDamageHelper(entity, autoNotifyNetwork, maxDamageAmount);
      }

      //////////////////////////////////////////////////////////////////////////
      void TestMunitionsComponent::ProcessMessage( const dtGame::Message& message )
      {
         MunitionsComponent::ProcessMessage( message );
         if( message.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED )
         {
            MessageCountMap::iterator foundIter = mMessageCounts.find( message.GetAboutActorId() );

            if( foundIter == mMessageCounts.end() )
            {
               mMessageCounts.insert( std::make_pair( message.GetAboutActorId(), 0 ) );
               foundIter = mMessageCounts.find( message.GetAboutActorId() );
            }

            if( foundIter != mMessageCounts.end() )
            {
               foundIter->second++;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned TestMunitionsComponent::GetTotalProcessedMessagesForActor( const dtCore::UniqueId& actorID ) const
      {
         MessageCountMap::const_iterator foundIter = mMessageCounts.find( actorID );

         if( foundIter != mMessageCounts.end() )
         {
            return foundIter->second;
         }

         return 0;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned TestMunitionsComponent::GetTotalProcessedMessages() const
      {
         unsigned messageCount = 0;

         MessageCountMap::const_iterator iter = mMessageCounts.begin();

         for( ; iter != mMessageCounts.end(); ++iter )
         {
            messageCount += iter->second;
         }

         return messageCount;
      }

      //////////////////////////////////////////////////////////////////////////
      void TestMunitionsComponent::ResetTotalProcessedMessages()
      {
         mMessageCounts.clear();
      }



      //////////////////////////////////////////////////////////////////////////
      // Sub-classed DamageHelper Code
      //////////////////////////////////////////////////////////////////////////
      TestDamageHelper::TestDamageHelper( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork, float maxDamage )
         : DamageHelper(entity,autoNotifyNetwork, maxDamage),
         mUsedProbability(0.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      float TestDamageHelper::GetDamageProbability( bool directFire ) const
      {
         return mUsedProbability;
      }



      //////////////////////////////////////////////////////////////////////////
      // Tests code
      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::setUp()
      {
         try
         {
            dtCore::System::GetInstance().Start();
            //dtCore::RefPtr<dtCore::Scene> scene = new dtCore::Scene;
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
            mGM->SetApplication(GetGlobalApplication());

            mMachineInfo = new dtGame::MachineInfo;
            mDamageComp = new TestMunitionsComponent;
            mDamageComp->SetMunitionConfigFileName("Configs:UnitTestsConfig.xml");

            mGM->AddComponent(*mDamageComp, dtGame::GameManager::ComponentPriority::NORMAL);

            //std::string context = dtUtil::GetDeltaRootPath() + "/examples/data/demoMap";
            //dtDAL::Project::GetInstance().SetContext(context, true);
            mGM->ChangeMap("UnitTestMunitionTypesMap");

            //step a few times to ensure the map loaded
            dtCore::System::GetInstance().Step();
            dtCore::System::GetInstance().Step();
            dtCore::System::GetInstance().Step();
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::tearDown()
      {
         try
         {
            dtCore::System::GetInstance().Stop();

            mDamageComp = NULL;

            mGM->CloseCurrentMap();
            
            dtCore::System::GetInstance().Step();
            dtCore::System::GetInstance().Step();
            dtCore::System::GetInstance().Step();

            dtCore::System::GetInstance().Step();

            mGM->DeleteAllActors(true);

            mGM = NULL;
            mMachineInfo = NULL;
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }
      }


      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::CreateTestEntities(
         std::vector<dtCore::RefPtr<SimCore::Actors::BaseEntity> >& listToFill,
         int numActors, bool putInGameManager )
      {
         // Prepare lists memory
         listToFill.clear();
         listToFill.reserve(numActors);

         // Create the actors and add them to the GM

         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> curProxy;
         SimCore::Actors::BaseEntity* curEntity = NULL;
         for( int index = 0; index < numActors; index++ )
         {
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, curProxy);

            curEntity = dynamic_cast<SimCore::Actors::BaseEntity*> (&(curProxy->GetGameActor()));
            curProxy->GetComponent<dtGame::DeadReckoningHelper>()->SetAutoRegisterWithGMComponent(false);
            listToFill.push_back( curEntity );

            if(putInGameManager)
            {
               mGM->AddActor(*curProxy, false, false);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::SendDetonationMessage( const std::string& munitionName,
         const osg::Vec3& detonationLocation, const SimCore::Actors::BaseEntity* targetEntity,
         const osg::Vec3* trajectory )
      {
         // Create message
         dtCore::RefPtr<DetonationMessage> msg;
         mGM->GetMessageFactory().CreateMessage( SimCore::MessageType::DETONATION, msg );

         // Set parameters
         msg->SetDetonationLocation( detonationLocation );
         msg->SetMunitionType( munitionName );
         if( trajectory != NULL ) { msg->SetFinalVelocityVector( *trajectory ); }
         if( targetEntity != NULL ) { msg->SetAboutActorId( targetEntity->GetUniqueId() ); }

         // Send message
         mGM->SendMessage( *msg );
         dtCore::System::GetInstance().Step();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::MoveEntity( SimCore::Actors::BaseEntity& entity, const osg::Vec3& newLocation )
      {
         dtCore::Transform xform;
         entity.GetTransform( xform );
         xform.SetTranslation( newLocation );
         entity.SetTransform( xform );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestDISIdentifierProperties()
      {
         SimCore::Actors::DISIdentifier dis(1,2,3,4,5,6,7);

         // Test accessors
         CPPUNIT_ASSERT( dis.GetKind() == 1 );
         CPPUNIT_ASSERT( dis.GetDomain() == 2 );
         CPPUNIT_ASSERT( dis.GetCountry() == 3 );
         CPPUNIT_ASSERT( dis.GetCategory() == 4 );
         CPPUNIT_ASSERT( dis.GetSubcategory() == 5 );
         CPPUNIT_ASSERT( dis.GetSpecific() == 6 );
         CPPUNIT_ASSERT( dis.GetExtra() == 7 );

         // Test conversion TO string
         CPPUNIT_ASSERT( "1 2 3 4 5 6 7" == dis.ToString() );

         // Test default constructor
         SimCore::Actors::DISIdentifier dis2;
         CPPUNIT_ASSERT( "0 0 0 0 0 0 0" == dis2.ToString() );
         CPPUNIT_ASSERT( dis != dis2 );
         CPPUNIT_ASSERT( ! (dis == dis2) );
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 0 );

         // Test conversion from string with less tokens.
         dis2.SetByString( "11 12 13 14" );
         CPPUNIT_ASSERT( dis2.GetKind() == 11 );
         CPPUNIT_ASSERT( dis2.GetDomain() == 12 );
         CPPUNIT_ASSERT( dis2.GetCountry() == 13 );
         CPPUNIT_ASSERT( dis2.GetCategory() == 14 );
         CPPUNIT_ASSERT( dis2.GetSubcategory() == 0 );
         CPPUNIT_ASSERT( dis2.GetSpecific() == 0 );
         CPPUNIT_ASSERT( dis2.GetExtra() == 0 );

         // Test conversion from string with more tokens.
         dis2.SetByString( "1 2 3 4 5 6 7 8 9" );
         CPPUNIT_ASSERT( dis2.GetKind() == 1 );
         CPPUNIT_ASSERT( dis2.GetDomain() == 2 );
         CPPUNIT_ASSERT( dis2.GetCountry() == 3 );
         CPPUNIT_ASSERT( dis2.GetCategory() == 4 );
         CPPUNIT_ASSERT( dis2.GetSubcategory() == 5 );
         CPPUNIT_ASSERT( dis2.GetSpecific() == 6 );
         CPPUNIT_ASSERT( dis2.GetExtra() == 7 );

         // Test conversion from string with bad tokens.
         dis2.SetByString( "11 Bad!@# 13 Bad$%^1 15 2Bad&*( 17 Bad)_+3 9" );
         CPPUNIT_ASSERT( dis2.GetKind() == 11 );
         CPPUNIT_ASSERT( dis2.GetDomain() == 0 );
         CPPUNIT_ASSERT( dis2.GetCountry() == 13 );
         CPPUNIT_ASSERT( dis2.GetCategory() == 0 );
         CPPUNIT_ASSERT( dis2.GetSubcategory() == 15 );
         CPPUNIT_ASSERT( dis2.GetSpecific() == 2 ); // tokens starting with numbers result to a value of the first number.
         CPPUNIT_ASSERT( dis2.GetExtra() == 17 );

         // Test conversion FROM string
         dis2.SetByString( "10 9 8 7 6 5 4" );
         CPPUNIT_ASSERT( dis2.GetKind() == 10 );
         CPPUNIT_ASSERT( dis2.GetDomain() == 9 );
         CPPUNIT_ASSERT( dis2.GetCountry() == 8 );
         CPPUNIT_ASSERT( dis2.GetCategory() == 7 );
         CPPUNIT_ASSERT( dis2.GetSubcategory() == 6 );
         CPPUNIT_ASSERT( dis2.GetSpecific() == 5 );
         CPPUNIT_ASSERT( dis2.GetExtra() == 4 );

         // --- Test accessing individual numbers
         CPPUNIT_ASSERT( dis2.GetNumber(0) == 10 );
         CPPUNIT_ASSERT( dis2.GetNumber(1) == 9 );
         CPPUNIT_ASSERT( dis2.GetNumber(2) == 8 );
         CPPUNIT_ASSERT( dis2.GetNumber(3) == 7 );
         CPPUNIT_ASSERT( dis2.GetNumber(4) == 6 );
         CPPUNIT_ASSERT( dis2.GetNumber(5) == 5 );
         CPPUNIT_ASSERT( dis2.GetNumber(6) == 4 );
         // --- Test out-of-range number indexes
         CPPUNIT_ASSERT( dis2.GetNumber(7) == 0 );

         // Test assignment
         dis = dis2;
         CPPUNIT_ASSERT( dis == dis2 );
         CPPUNIT_ASSERT( ! (dis != dis2) );
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 7 );

         // Test mutators
         dis.SetKind(2);
         dis.SetDomain(4);
         dis.SetCountry(8);
         dis.SetCategory(16);
         dis.SetSubcategory(32);
         dis.SetSpecific(64);
         dis.SetExtra(128);
         CPPUNIT_ASSERT( dis.GetKind() == 2 );
         CPPUNIT_ASSERT( dis.GetDomain() == 4 );
         CPPUNIT_ASSERT( dis.GetCountry() == 8 );
         CPPUNIT_ASSERT( dis.GetCategory() == 16 );
         CPPUNIT_ASSERT( dis.GetSubcategory() == 32 );
         CPPUNIT_ASSERT( dis.GetSpecific() == 64 );
         CPPUNIT_ASSERT( dis.GetExtra() == 128 );

         // Test match degree (matches start left to right)
         dis2.Set(2,0,0,0,0,0,0);
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 1 );
         dis2.SetDomain(4);
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 2 );
         dis2.SetCountry(8);
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 3 );
         dis2.SetCategory(16);
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 4 );
         dis2.SetSubcategory(32);
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 5 );
         dis2.SetSpecific(64);
         CPPUNIT_ASSERT( dis.GetDegreeOfMatch( dis2 ) == 6 );
         // --- NOT setting last field (Extra) since previous test
         // --- showed that full equality yields a match degree of 7.
         // --- Extra will be used to test Less-Than and Greater-Than operators.

         // Test Less-Than operator
         CPPUNIT_ASSERT( dis2 < dis );
         CPPUNIT_ASSERT( dis2 <= dis );
         CPPUNIT_ASSERT( ! (dis < dis2) );
         CPPUNIT_ASSERT( ! (dis <= dis2) );

         // Test Less-Than operator
         CPPUNIT_ASSERT( dis > dis2 );
         CPPUNIT_ASSERT( dis >= dis2 );
         CPPUNIT_ASSERT( ! (dis2 > dis) );
         CPPUNIT_ASSERT( ! (dis2 >= dis) );

         // Test Less-Than, Greater-Than Equality operators
         dis2.SetExtra(128);
         CPPUNIT_ASSERT( dis >= dis2 );
         CPPUNIT_ASSERT( dis2 >= dis );
         CPPUNIT_ASSERT( dis <= dis2 );
         CPPUNIT_ASSERT( dis2 <= dis );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestDamageProbabilitiyProperties()
      {
         dtCore::RefPtr<DamageProbability> probs = new DamageProbability("TestProbs");

         CPPUNIT_ASSERT_MESSAGE("No-Damage probability should be defaulted to 0.0",
            probs->GetNoDamage() == 0.0f );
         CPPUNIT_ASSERT_MESSAGE("Mobility-Damage probability should be defaulted to 0.0",
            probs->GetMobilityDamage() == 0.0f );
         CPPUNIT_ASSERT_MESSAGE("Firepower-Damage probability should be defaulted to 0.0",
            probs->GetFirepowerDamage() == 0.0f );
         CPPUNIT_ASSERT_MESSAGE("MobilityFirepower-Damage probability should be defaulted to 0.0",
            probs->GetMobilityFirepowerDamage() == 0.0f );
         CPPUNIT_ASSERT_MESSAGE("Kill-Damage probability should be defaulted to 1.0",
            probs->GetKillDamage() == 1.0f );

         CPPUNIT_ASSERT_MESSAGE( "AbsoluteMode should be FALSE by default",
            ! probs->GetAbsoluteMode() );
         probs->SetAbsoluteMode( true );
         CPPUNIT_ASSERT_MESSAGE( "AbsoluteMode should now be TRUE",
            probs->GetAbsoluteMode() );
         probs->SetAbsoluteMode( false );

         float value1 = 0.25f;
         float value2 = 0.22f;
         float value3 = 0.15f;
         float value4 = 0.22f;
         float value5 = 0.16f;

         probs->SetNoDamage(value1);
         probs->SetMobilityDamage(value2);
         probs->SetFirepowerDamage(value3);
         probs->SetMobilityFirepowerDamage(value4);
         probs->SetKillDamage(value5);

         CPPUNIT_ASSERT_MESSAGE("No-Damage probability should have a new value",
            probs->GetNoDamage() == value1 );
         CPPUNIT_ASSERT_MESSAGE("Mobility-Damage probability should have a new value",
            probs->GetMobilityDamage() == value2 );
         CPPUNIT_ASSERT_MESSAGE("Firepower-Damage probability should have a new value",
            probs->GetFirepowerDamage() == value3 );
         CPPUNIT_ASSERT_MESSAGE("MobilityFirepower-Damage probability should have a new value",
            probs->GetMobilityFirepowerDamage() == value4 );
         CPPUNIT_ASSERT_MESSAGE("Kill-Damage probability should have a new value",
            probs->GetKillDamage() == value5 );

         value1 = 0.2f;
         value2 = 0.2f;
         value3 = 0.2f;
         value4 = 0.2f;
         value5 = 0.2f;

         // Test setting all probabilities at once
         probs->Set( value1, value2, value3, value4, value5 );

         CPPUNIT_ASSERT_MESSAGE("No-Damage probability should have another new value",
            probs->GetNoDamage() == value1 );
         CPPUNIT_ASSERT_MESSAGE("Mobility-Damage probability should have another new value",
            probs->GetMobilityDamage() == value2 );
         CPPUNIT_ASSERT_MESSAGE("Firepower-Damage probability should have another new value",
            probs->GetFirepowerDamage() == value3 );
         CPPUNIT_ASSERT_MESSAGE("MobilityFirepower-Damage probability should have another new value",
            probs->GetMobilityFirepowerDamage() == value4 );
         CPPUNIT_ASSERT_MESSAGE("Kill-Damage probability should have another new value",
            probs->GetKillDamage() == value5 );

         // Numbers are expanded internally to: 0.2, 0.4, 0.6, 0.8, 1.0
         CPPUNIT_ASSERT( probs->GetDamageType(0.1f) == DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT( probs->GetDamageType(0.3f) == DamageType::DAMAGE_MOBILITY );
         CPPUNIT_ASSERT( probs->GetDamageType(0.5f) == DamageType::DAMAGE_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.7f) == DamageType::DAMAGE_MOBILITY_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.9f) == DamageType::DAMAGE_KILL );

         // --- Test reversed
         // Numbers are expanded internally to: 1.0, 0.8, 0.6, 0.4, 0.2
         CPPUNIT_ASSERT( probs->GetDamageType(0.1f,true) == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( probs->GetDamageType(0.3f,true) == DamageType::DAMAGE_MOBILITY_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.5f,true) == DamageType::DAMAGE_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.7f,true) == DamageType::DAMAGE_MOBILITY );
         CPPUNIT_ASSERT( probs->GetDamageType(0.9f,true) == DamageType::DAMAGE_NONE );



         // Test assignment by another DamageProbability
         dtCore::RefPtr<DamageProbability> probs2 = new DamageProbability("TestProbs2");
         probs2->Set(0.05f,0.10f,0.20f,0.30f,0.35f);
         (*probs) = *probs2;
         CPPUNIT_ASSERT_MESSAGE("DamageProbabilities should be assignable by another DamageProbability",
            (*probs) == *probs2 );
         // --- Double check to make sure the == operator works; merely change one value
         probs2->SetNoDamage(1.0f);
         CPPUNIT_ASSERT_MESSAGE("DamageProbabilities == operator should work properly",
            (*probs) != *probs2 );



         // Test values with absolute mode enabled ( using the new values )
         probs->SetAbsoluteMode( true );

         // Assuming 0.05f,0.10f,0.20f,0.30f,0.35f
         CPPUNIT_ASSERT( probs->GetDamageType(0.04f) == DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT( probs->GetDamageType(0.08f) == DamageType::DAMAGE_MOBILITY );
         CPPUNIT_ASSERT( probs->GetDamageType(0.12f) == DamageType::DAMAGE_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.24f) == DamageType::DAMAGE_MOBILITY_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.32f) == DamageType::DAMAGE_KILL );

         // --- Test reversed
         // Assuming 0.05f,0.10f,0.20f,0.30f,0.35f
         CPPUNIT_ASSERT( probs->GetDamageType(0.04f,true) == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( probs->GetDamageType(0.08f,true) == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( probs->GetDamageType(0.12f,true) == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( probs->GetDamageType(0.24f,true) == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( probs->GetDamageType(0.32f,true) == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( probs->GetDamageType(0.5f,true) == DamageType::DAMAGE_NONE );
         // --- Swap values to have a true reversal
         probs->Set(0.35f,0.30f,0.20f,0.10f,0.05f);
         CPPUNIT_ASSERT( probs->GetDamageType(0.04f,true) == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( probs->GetDamageType(0.08f,true) == DamageType::DAMAGE_MOBILITY_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.12f,true) == DamageType::DAMAGE_FIREPOWER );
         CPPUNIT_ASSERT( probs->GetDamageType(0.24f,true) == DamageType::DAMAGE_MOBILITY );
         CPPUNIT_ASSERT( probs->GetDamageType(0.32f,true) == DamageType::DAMAGE_NONE );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestDamageRangesProperties()
      {
         dtCore::RefPtr<DamageRanges> ranges = new DamageRanges("TestDamageRanges");

         // Declare test variables
         osg::Vec4 testRanges(1.0,2.0,3.0,4.0);

         // Test Forward Ranges
         ranges->SetForwardRanges( testRanges[0], testRanges[1], testRanges[2], testRanges[3] );
         CPPUNIT_ASSERT( testRanges == ranges->GetForwardRanges() );
         // --- Mix up values
         testRanges.set(5.0,6.0,7.0,8.0);
         ranges->SetForwardRanges( testRanges );
         CPPUNIT_ASSERT( testRanges == ranges->GetForwardRanges() );

         // Test Deflect Ranges
         // --- Mix up values
         testRanges.set(9.0,10.0,11.0,12.0);
         ranges->SetDeflectRanges( testRanges[0], testRanges[1], testRanges[2], testRanges[3] );
         CPPUNIT_ASSERT( testRanges == ranges->GetDeflectRanges() );
         // --- Mix up values
         testRanges.set(13.0,14.0,15.0,16.0);
         ranges->SetDeflectRanges( testRanges );
         CPPUNIT_ASSERT( testRanges == ranges->GetDeflectRanges() );

         // Test Angle-Of-Fall
         ranges->SetAngleOfFall(55.0f);
         CPPUNIT_ASSERT( 55.0f == ranges->GetAngleOfFall() );

         // Test assignment by another DamageRanges object
         dtCore::RefPtr<DamageRanges> ranges2 = new DamageRanges("TestDamageRanges2");
         CPPUNIT_ASSERT( (*ranges) != *ranges2 );
         (*ranges) = *ranges2;
         CPPUNIT_ASSERT( (*ranges) == *ranges2 );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionDamageProperties()
      {
         dtCore::RefPtr<MunitionDamage> damage = new MunitionDamage("TestMunitionDamage");

         // Declare test variables
         float testValue = 150.0f;
         osg::Vec3 trajectory;
         dtCore::RefPtr<DamageRanges> testRanges = new DamageRanges("TestRanges");
         dtCore::RefPtr<DamageProbability> testProbs = new DamageProbability("TestProbs");

         // Test cutoff range
         CPPUNIT_ASSERT( damage->GetCutoffRange() != testValue );
         damage->SetCutoffRange( testValue );
         CPPUNIT_ASSERT( damage->GetCutoffRange() == testValue );

         // Test Newton force
         testValue = 500.0f;
         CPPUNIT_ASSERT( damage->GetNewtonForce() != testValue );
         damage->SetNewtonForce( testValue );
         CPPUNIT_ASSERT( damage->GetNewtonForce() == testValue );

         // Test accumulation factor
         testValue = 0.01f;
         CPPUNIT_ASSERT( damage->GetAccumulationFactor() != testValue );
         damage->SetAccumulationFactor( testValue );
         CPPUNIT_ASSERT( damage->GetAccumulationFactor() == testValue );



         // Test Direct Fire DamageProbabilities
         CPPUNIT_ASSERT( damage->GetDirectFireProbabilities() == NULL );
         damage->SetDirectFireProbabilities( 0.05f, 0.15f, 0.25f, 0.35f, 0.2f );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage should have created a new DamageProbability for Direct Fire.",
            damage->GetDirectFireProbabilities() != NULL );
         // --- Test setting probabilities with another DamageProbability object
         damage->SetDirectFireProbabilities( *testProbs );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage should have its own copy of DamageProbability and not reference the one with which it was set.",
            testProbs.get() != damage->GetDirectFireProbabilities() );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage's new DamageProbability should have the same values as the one with which it was set.",
            (*testProbs) == *damage->GetDirectFireProbabilities() );



         // Test Indirect Fire DamageProbabilities
         CPPUNIT_ASSERT( damage->GetIndirectFireProbabilities() == NULL );
         damage->SetIndirectFireProbabilities( 0.2f, 0.2f, 0.2f, 0.2f, 0.2f );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage should have created a new DamageProbability for Indirect Fire.",
            damage->GetIndirectFireProbabilities() != NULL );
         // --- Test setting probabilities with another DamageProbability object
         damage->SetIndirectFireProbabilities( *testProbs );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage should have its own copy of DamageProbability and not reference the one with which it was set.",
            testProbs.get() != damage->GetIndirectFireProbabilities() );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage's new DamageProbability should have the same values as the one with which it was set.",
            (*testProbs) == *damage->GetIndirectFireProbabilities() );



         // Test Damage Ranges (all numbers are arbitrary)
         CPPUNIT_ASSERT( damage->GetDamageRanges1_3() == NULL );
         CPPUNIT_ASSERT( damage->GetDamageRanges2_3() == NULL );
         CPPUNIT_ASSERT( damage->GetDamageRangesMax() == NULL );

         // --- Set Range 1/3
         testRanges->SetAngleOfFall( 30.0f );
         testRanges->SetForwardRanges( 100.0, 120.0, 80.0, 50.0 );
         testRanges->SetDeflectRanges( 50.0, 60.0, 40.0, 25.0 );
         damage->SetDamageRanges1_3( testRanges ); // should copy, NOT reference
         CPPUNIT_ASSERT( damage->GetDamageRanges1_3() != NULL );
         CPPUNIT_ASSERT( (*testRanges) == *(damage->GetDamageRanges1_3()) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage (Range 1/3) should only copy DamageRanges objects",
            testRanges.get() != damage->GetDamageRanges1_3() );

         // --- Set Range 2/3
         testRanges->SetAngleOfFall( 45.0f );
         testRanges->SetForwardRanges( 150.0, 180.0, 120.0, 80.0 );
         testRanges->SetDeflectRanges( 75.0, 90.0, 60.0, 40.0 );
         damage->SetDamageRanges2_3( testRanges ); // should copy, NOT reference
         CPPUNIT_ASSERT( damage->GetDamageRanges2_3() != NULL );
         CPPUNIT_ASSERT( (*testRanges) == *(damage->GetDamageRanges2_3()) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage (Range 2/3) should only copy DamageRanges objects",
            testRanges.get() != damage->GetDamageRanges2_3() );

         // --- Set Range Max
         testRanges->SetAngleOfFall( 90.0f );
         testRanges->SetForwardRanges( 200.0, 240.0, 160.0, 100.0 );
         testRanges->SetDeflectRanges( 100.0, 120.0, 80.0, 50.0 );
         damage->SetDamageRangesMax( testRanges ); // should copy, NOT reference
         CPPUNIT_ASSERT( damage->GetDamageRangesMax() != NULL );
         CPPUNIT_ASSERT( (*testRanges) == *(damage->GetDamageRangesMax()) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamage (Range Max) should only copy DamageRanges objects",
            testRanges.get() != damage->GetDamageRangesMax() );



         // Test Trajectories with Angle-Of-Fall
         const DamageRanges* ranges1 = damage->GetDamageRanges1_3(); // AOF = 30
         const DamageRanges* ranges2 = damage->GetDamageRanges2_3(); // AOF = 45
         const DamageRanges* ranges3 = damage->GetDamageRangesMax(); // AOF = 90
         CPPUNIT_ASSERT_MESSAGE("MunitionDamage should not have lost Range 1/3", ranges1 != NULL );
         CPPUNIT_ASSERT_MESSAGE("MunitionDamage should not have lost Range 2/3", ranges2 != NULL );
         CPPUNIT_ASSERT_MESSAGE("MunitionDamage should not have lost Range Max", ranges3 != NULL );

         // --- Test straight down
         trajectory.set( 0.0f, 0.0f,-1.0f );
         CPPUNIT_ASSERT( ranges3 == damage->GetDamageRangesByTrajectory(trajectory) );

         // --- Test from the sides --- should go to lowest Angle-Of-Fall (Range 1/3 in this case; 30 degrees)
         trajectory.set( 1.0f, 0.0f, 0.0f );
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, 1.0f, 0.0f );
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( -1.0f, 0.0f, 0.0f );
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, -1.0f, 0.0f );
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );

         // --- Test angled
         // ------ Between 30 and 45, favor 30 (Range 1/3)
         trajectory.set( 0.798636f, 0.0f, -0.601815f ); // 37 degrees
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( -0.798636f, 0.0f, -0.601815f ); // 37 degrees
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, 0.798636f, -0.601815f ); // 37 degrees
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, -0.798636f, -0.601815f ); // 37 degrees
         CPPUNIT_ASSERT( ranges1 == damage->GetDamageRangesByTrajectory(trajectory) );

         // ------ Between 30 and 45, favor 45 (Range 2/3)
         trajectory.set( 0.788011f, 0.0f, -0.615661f ); // 38 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( -0.788011f, 0.0f, -0.615661f ); // 38 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, 0.788011f, -0.615661f ); // 38 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, -0.788011f, -0.615661f ); // 38 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );

         // ------ Between 45 and 90, favor 45 (Range 2/3)
         trajectory.set( 0.390731f, 0.0f, -0.920505f ); // 67 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( -0.390731f, 0.0f, -0.920505f ); // 67 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, 0.390731f, -0.920505f ); // 67 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, -0.390731f, -0.920505f ); // 67 degrees
         CPPUNIT_ASSERT( ranges2 == damage->GetDamageRangesByTrajectory(trajectory) );

         // ------ Between 45 and 90, favor 90 (Range Max)
         trajectory.set( 0.374607f, 0.0f, -0.927184f ); // 68 degrees
         CPPUNIT_ASSERT( ranges3 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( -0.374607f, 0.0f, -0.927184f ); // 68 degrees
         CPPUNIT_ASSERT( ranges3 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, 0.374607f, -0.927184f ); // 68 degrees
         CPPUNIT_ASSERT( ranges3 == damage->GetDamageRangesByTrajectory(trajectory) );
         trajectory.set( 0.0f, -0.374607f, -0.927184f ); // 68 degrees
         CPPUNIT_ASSERT( ranges3 == damage->GetDamageRangesByTrajectory(trajectory) );

         // Test no trajectory --- defaults to Range Max
         trajectory.set( 0.0f, 0.0f, 0.0f );
         CPPUNIT_ASSERT( ranges3 == damage->GetDamageRangesByTrajectory(trajectory) );



         // Test the Carleton Equation
         //   totalProb = damageProb * exp( -DamageProb * ( (forwardDistance/forwardRange)^2 + (deflectDistance/deflectRange)^2 ) )
         //
         //   damageProb = the max probability of a certain damage type
         //   forwardDistance = distance from explosion along the munition's trajectory
         //   deflectDistance = distance from explosion perpendicular from the munition's trajectory
         //   forwardRange = the radial explosion distance along trajectory
         //   deflectRange = the radial explosion distance perpendicular from trajectory
         //   totalProb = scaled down version of damageProb based on distance; follows a bell curve
         //               Point zero will have full force, thus totalProb == damageProb.
         //
         // GetProbability_CarletonEquation( damageProb, forwardDistance, deflectDistance, forwardRange, deflectRange )
         testValue = damage->GetProbability_CarletonEquation( 20.0f, -5.0f, -2.0f, 30.0f, 10.0f );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 5.156080604,
            testValue,
            0.001 );

         // Test the explosive force --- Using Range Max
         osg::Vec3 testForce, targetPos, explosionPos;
         trajectory.set( 0.0f, 0.0f, -1.0f );
         testForce = damage->GetForce( targetPos, explosionPos, trajectory );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( damage->GetNewtonForce(), testForce.length(), 0.01 );

         // The following lines where copied from above from Range Max
         // NOTE: The munition damage will use the largest range of each range type.
         // testRanges->SetForwardRanges( 200.0, 240.0, 160.0, 100.0 );
         // testRanges->SetDeflectRanges( 100.0, 120.0, 80.0, 50.0 );
         targetPos.set( 0.0, -1.0, 10.0f ); // 10 meters above explosion
         testForce = damage->GetForce( targetPos, explosionPos, trajectory );
         //testValue = damage->GetProbability_CarletonEquation(
         //   damage->GetNewtonForce(), targetPos[2], targetPos[1], 240.0f, 120.0f );
         testValue = damage->GetNewtonForce() *
            damage->GetProbability_CarletonEquation(1.0, targetPos[2], targetPos[1], 240.0f, 120.0f );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( testValue, testForce.length(), 0.01 );

      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionDamageTableProperties()
      {
         dtCore::RefPtr<MunitionDamageTable> table = new MunitionDamageTable("TestMunitionDamageTable");
         dtCore::RefPtr<MunitionDamage> damage1 = new MunitionDamage("Info1");
         dtCore::RefPtr<MunitionDamage> damage2 = new MunitionDamage("Info2");
         dtCore::RefPtr<MunitionDamage> damage3 = new MunitionDamage("Info3");
         dtCore::RefPtr<MunitionDamage> damage4 = new MunitionDamage("Info4");

         // Test Adding
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should NOT have any entries by default.",
            table->GetCount() == 0 );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage1 ) );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage2 ) );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage3 ) );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage4 ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should have 4 entries.",
            table->GetCount() == 4 );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage1->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage2->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage3->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage4->GetName() ) );



         // Test Re-Adding
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should NOT re-add a MunitionDamage that it already has.",
            ! table->AddMunitionDamage( damage1 ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should still have 4 entries.",
            table->GetCount() == 4 );



         // Test Retrieval
         CPPUNIT_ASSERT( table->GetMunitionDamage( damage1->GetName() ) == damage1.get() );
         CPPUNIT_ASSERT( table->GetMunitionDamage( damage2->GetName() ) == damage2.get() );
         CPPUNIT_ASSERT( table->GetMunitionDamage( damage3->GetName() ) == damage3.get() );
         CPPUNIT_ASSERT( table->GetMunitionDamage( damage4->GetName() ) == damage4.get() );



         // Test Removing ( out of order )
         CPPUNIT_ASSERT( table->RemoveMunitionDamage( damage3->GetName() ) );
         CPPUNIT_ASSERT( ! table->HasMunitionDamage( damage3->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage1->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage2->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage4->GetName() ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should now have 3 entries.",
            table->GetCount() == 3 );

         CPPUNIT_ASSERT( table->RemoveMunitionDamage( damage4->GetName() ) );
         CPPUNIT_ASSERT( ! table->HasMunitionDamage( damage4->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage1->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage2->GetName() ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should now have 2 entries.",
            table->GetCount() == 2 );

         CPPUNIT_ASSERT( table->RemoveMunitionDamage( damage1->GetName() ) );
         CPPUNIT_ASSERT( ! table->HasMunitionDamage( damage1->GetName() ) );
         CPPUNIT_ASSERT( table->HasMunitionDamage( damage2->GetName() ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should now have 1 entry.",
            table->GetCount() == 1 );

         CPPUNIT_ASSERT( table->RemoveMunitionDamage( damage2->GetName() ) );
         CPPUNIT_ASSERT( ! table->HasMunitionDamage( damage1->GetName() ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should now have 0 entries.",
            table->GetCount() == 0 );



         // Re-add all entries and test clear
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage1 ) );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage2 ) );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage3 ) );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage4 ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should have 4 entries, again.",
            table->GetCount() == 4 );
         table->Clear();
         CPPUNIT_ASSERT_MESSAGE( "MunitionDamageTable should have cleared all entries.",
            table->GetCount() == 0 );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionTypeTableProperties()
      {
         dtCore::RefPtr<MunitionTypeTable> table = new MunitionTypeTable;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> munition1;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> munition2;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> munition3;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> munition4;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> munition5;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> munition6;

         CPPUNIT_ASSERT( table->GetCount() == 0 );
         CPPUNIT_ASSERT( table->GetOrderedListSize() == 0 );

         // Initialize the munition actors
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> proxy1;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> proxy2;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> proxy3;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> proxy4;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> proxy5;
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> proxy6;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy1 );
         munition1 = dynamic_cast<SimCore::Actors::MunitionTypeActor*> (proxy1->GetActor());
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy2 );
         munition2 = dynamic_cast<SimCore::Actors::MunitionTypeActor*> (proxy2->GetActor());
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy3 );
         munition3 = dynamic_cast<SimCore::Actors::MunitionTypeActor*> (proxy3->GetActor());
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy4 );
         munition4 = dynamic_cast<SimCore::Actors::MunitionTypeActor*> (proxy4->GetActor());
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy5 );
         munition5 = dynamic_cast<SimCore::Actors::MunitionTypeActor*> (proxy5->GetActor());
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy6 );
         munition6 = dynamic_cast<SimCore::Actors::MunitionTypeActor*> (proxy6->GetActor());

         // --- Set names and DIS identifiers
         munition6->SetDISIdentifierByString( "1 2 3 0 0 0 0" );
         proxy6->SetName( "Type6" );
         munition5->SetDISIdentifierByString( "1 2 3 1 0 0 0" );
         proxy5->SetName( "Type5" );
         munition4->SetDISIdentifierByString( "1 2 3 4 0 0 0" );
         proxy4->SetName( "Type4" );
         munition3->SetDISIdentifierByString( "1 2 3 4 1 0 0" );
         proxy3->SetName( "Type3" );
         munition2->SetDISIdentifierByString( "1 2 3 5 0 0 0" );
         proxy2->SetName( "Type2" );
         munition1->SetDISIdentifierByString( "1 2 3 5 1 0 0" );
         proxy1->SetName( "Type1" );

         // --- Add to the table, out of order
         CPPUNIT_ASSERT( table->AddMunitionType( proxy3 ) );
         CPPUNIT_ASSERT( table->AddMunitionType( proxy5 ) );
         CPPUNIT_ASSERT( table->AddMunitionType( proxy1 ) );
         CPPUNIT_ASSERT( table->AddMunitionType( proxy6 ) );
         CPPUNIT_ASSERT( table->AddMunitionType( proxy2 ) );
         CPPUNIT_ASSERT( table->AddMunitionType( proxy4 ) );
         CPPUNIT_ASSERT( table->GetCount() == 6 );
         CPPUNIT_ASSERT( table->GetOrderedListSize() == 6 );

         // Test the order of munition types
         const std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >& typeList
            = table->GetOrderedList();
         CPPUNIT_ASSERT( munition6.get() == typeList[0].get() );
         CPPUNIT_ASSERT( munition5.get() == typeList[1].get() );
         CPPUNIT_ASSERT( munition4.get() == typeList[2].get() );
         CPPUNIT_ASSERT( munition3.get() == typeList[3].get() );
         CPPUNIT_ASSERT( munition2.get() == typeList[4].get() );
         CPPUNIT_ASSERT( munition1.get() == typeList[5].get() );



         // Test retrieval of munition types
         CPPUNIT_ASSERT( ! table->HasMunitionType( "TypeUnkown" ) );
         CPPUNIT_ASSERT( table->HasMunitionType( "Type1" ) );
         CPPUNIT_ASSERT( table->HasMunitionType( "Type6" ) );
         CPPUNIT_ASSERT( table->HasMunitionType( "Type3" ) );
         CPPUNIT_ASSERT( table->GetMunitionType( "Type1" ) == munition1.get() );
         CPPUNIT_ASSERT( table->GetMunitionType( "Type6" ) == munition6.get() );
         CPPUNIT_ASSERT( table->GetMunitionType( "Type3" ) == munition3.get() );

         // Test closest matching
         CPPUNIT_ASSERT( table->GetMunitionTypeByDIS( "1 2 3 4 5 0 0" ) == munition4.get() );
         CPPUNIT_ASSERT( table->GetMunitionTypeByDIS( "1 2 0 0 0 0 0" ) == NULL );
         CPPUNIT_ASSERT( table->GetMunitionTypeByDIS( "1 2 3 4 1 9 9" ) == munition3.get() );
         CPPUNIT_ASSERT( table->GetMunitionTypeByDIS( "1 2 3 5 1 0 0" ) == munition1.get() );
         CPPUNIT_ASSERT( table->GetMunitionTypeByDIS( "8 8 8 8 8 8 8" ) == NULL );



         // Test removing munition types
         CPPUNIT_ASSERT( table->RemoveMunitionType( "Type1" ) );
         CPPUNIT_ASSERT( table->RemoveMunitionType( "Type6" ) );
         CPPUNIT_ASSERT( table->RemoveMunitionType( "Type3" ) );
         // --- Test removing an actor that was already removed
         CPPUNIT_ASSERT( ! table->RemoveMunitionType( "Type3" ) );

         CPPUNIT_ASSERT( ! table->HasMunitionType( "Type1" ) );
         CPPUNIT_ASSERT( ! table->HasMunitionType( "Type6" ) );
         CPPUNIT_ASSERT( ! table->HasMunitionType( "Type3" ) );
         CPPUNIT_ASSERT( table->GetCount() == 3 );
         CPPUNIT_ASSERT( table->GetOrderedListSize() == 3 );

         // --- Test the order
         CPPUNIT_ASSERT( munition5.get() == typeList[0].get() );
         CPPUNIT_ASSERT( munition4.get() == typeList[1].get() );
         CPPUNIT_ASSERT( munition2.get() == typeList[2].get() );

         // Test clearing
         table->Clear();
         CPPUNIT_ASSERT( table->GetCount() == 0 );
         CPPUNIT_ASSERT( table->GetOrderedListSize() == 0 );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestDamageHelperProperties()
      {

         // Create a test entity
         dtCore::RefPtr<SimCore::Actors::BaseEntity> entity;
         std::vector<dtCore::RefPtr<SimCore::Actors::BaseEntity> > entities;
         CreateTestEntities( entities, 1, true );
         entity = dynamic_cast<SimCore::Actors::BaseEntity*> (&(entities[0]->GetGameActorProxy().GetGameActor()));
         CPPUNIT_ASSERT_MESSAGE( "A valid test entity should have been created",
            entity.valid() );

         // Create the test DamageHelper
         dtCore::RefPtr<TestDamageHelper> helper = new TestDamageHelper( *entity );
         dtCore::RefPtr<MunitionDamageTable> table = new MunitionDamageTable( "TestTable" );
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper should have a reference to the entity",
            helper->GetEntity() != NULL );

         // Test default damage states
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper default damage state should be NONE",
            helper->GetDamageState() == DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT_MESSAGE("DamageHelper last-notified default damage state should be NONE",
            helper->GetLastNotifiedDamageState() == DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT_MESSAGE( "Entity should not be flaming by default.", ! entity->IsFlamesPresent() );

         // Test table assignment
         CPPUNIT_ASSERT( helper->GetMunitionDamageTable() == NULL );
         helper->SetMunitionDamageTable( table );
         CPPUNIT_ASSERT( helper->GetMunitionDamageTable() != NULL );

         // Test retrieval of entity position
         osg::Vec3 outPos, pos( 25.0f, -10.0f, 5.0f );
         MoveEntity( *entity, pos );
         helper->GetEntityPosition( outPos );
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper should retrieve the entity's current location",
            //dtUtil::Equivalent(outPos, pos, 0.0001f));
            outPos[0] == pos[0] && outPos[1] == pos[1] && outPos[2] == pos[2]);



         // Test NotifyNetwork feature
         helper->SetAutoNotifyNetwork( true );
         CPPUNIT_ASSERT( helper->GetAutoNotifyNetwork() );
         helper->SetAutoNotifyNetwork( false );
         CPPUNIT_ASSERT( ! helper->GetAutoNotifyNetwork() );
         helper->SetAutoNotifyNetwork( true );



         // Test damage state changes (which also calls notify network)
         CPPUNIT_ASSERT_MESSAGE( "The entity's damage state should be NO_DAMAGE",
            entity->GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE );

         entity->GetComponent<dtGame::DeadReckoningHelper>()->SetDeadReckoningAlgorithm( dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY );

         // Clear message count since actor was added to the GM
         dtCore::System::GetInstance().Step();
         mDamageComp->ResetTotalProcessedMessages();

         helper->SetDamage( DamageType::DAMAGE_KILL );

         // --- Check if the update messages were truly sent
         dtCore::System::GetInstance().Step();
         unsigned messagesProcessed = mDamageComp->GetTotalProcessedMessagesForActor( entity->GetUniqueId() );
         std::cout << "\n\tMessages processed: " << messagesProcessed << "\n" << std::endl;
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper should have sent a network message",
            messagesProcessed >= 1 ); // NotifyNetwork was called
         mDamageComp->ResetTotalProcessedMessages();

         CPPUNIT_ASSERT_MESSAGE( "Current damage state should be KILL",
            helper->GetDamageState() == DamageType::DAMAGE_KILL );
         // --- This confirms NotifyNetwork has been called
         CPPUNIT_ASSERT_MESSAGE( "Last notified damage state should ALSO be KILL",
            helper->GetLastNotifiedDamageState() == DamageType::DAMAGE_KILL );

         CPPUNIT_ASSERT_MESSAGE( "The entity's damage state should have been changed to DESTROYED",
            entity->GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED );

         // Note - we no longer override the old dead reckoning algorithm. otherwise,
         // when a vehicle is killed, it can't fall from the sky correctly, or if it's bumped by a bomb,
         // it won't move smoothly, it will blip statically.
         // --- Ensure the dead reckoning has NOT been changed
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper should NOT have set the entity's Dead Reckoning Algorithm upon KILL damage.",
            entity->GetComponent<dtGame::DeadReckoningHelper>()->GetDeadReckoningAlgorithm() == dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY );

         // --- Ensure that the entity is burning.
         CPPUNIT_ASSERT( entity->IsFlamesPresent() );

         // --- Test last-notified damage state if auto-notify is disabled
         helper->SetAutoNotifyNetwork( false );
         helper->SetDamage( DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT_MESSAGE( "Current damage state should now be NONE",
            helper->GetDamageState() == DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT( ! entity->IsFlamesPresent() );
         // --- This confirms NotifyNetwork has NOT been called
         CPPUNIT_ASSERT_MESSAGE( "Last notified damage state should still be KILL",
            helper->GetLastNotifiedDamageState() == DamageType::DAMAGE_KILL );
         dtCore::System::GetInstance().Step();
         // Next test is wierd. The actor automatically generates an update, via the DRPublishingActComp
         // So we want to make sure there is only 1 message, not 2.
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper should NOT have sent a network message",
            mDamageComp->GetTotalProcessedMessages() == 1 ); // NotifyNetwork was NOT called


         // Test table lost
         table = NULL;
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper should NOT still be holding onto the observed table",
            helper->GetMunitionDamageTable() == NULL );

         // Test entity lost
         entity = NULL;
         entities.clear();
         mGM->DeleteAllActors( true );
         CPPUNIT_ASSERT_MESSAGE( "DamageHelper should NOT still be holding onto the observed entity",
            helper->GetMunitionDamageTable() == NULL );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionsComponentProperties()
      {
         // Create test objects
         std::vector<dtCore::RefPtr<SimCore::Actors::BaseEntity> > entities;
         CreateTestEntities( entities, 3, true );
         const dtCore::UniqueId& id1 = entities[0]->GetUniqueId();
         const dtCore::UniqueId& id2 = entities[1]->GetUniqueId();
         const dtCore::UniqueId& id3 = entities[2]->GetUniqueId();
         const std::string tableName1("TestTable1");
         const std::string tableName2("TestTable2");
         const std::string tableName3("TestTable3");
         dtCore::RefPtr<MunitionDamageTable> table1 = new MunitionDamageTable(tableName1);
         dtCore::RefPtr<MunitionDamageTable> table2 = new MunitionDamageTable(tableName2);
         dtCore::RefPtr<MunitionDamageTable> table3 = new MunitionDamageTable(tableName3);



         // Test entity registration
         CPPUNIT_ASSERT( mDamageComp->Register( *entities[0] ) );
         CPPUNIT_ASSERT( mDamageComp->Register( *entities[1] ) );
         CPPUNIT_ASSERT( mDamageComp->Register( *entities[2] ) );
         // --- Test Re-registering when already registered
         std::cout << "NOTE: fail message to follow as expected: re-registering an entity that was already registered."
            << std::endl;
         CPPUNIT_ASSERT( ! mDamageComp->Register( *entities[0] ) );

         // Ensure that the entities were registered properly
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id1 ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id2 ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id3 ) );
         CPPUNIT_ASSERT( mDamageComp->GetHelperByEntityId( id1 )->GetEntity() == entities[0].get() );
         CPPUNIT_ASSERT( mDamageComp->GetHelperByEntityId( id2 )->GetEntity() == entities[1].get() );
         CPPUNIT_ASSERT( mDamageComp->GetHelperByEntityId( id3 )->GetEntity() == entities[2].get() );

         // Test unregistering entities (out of order)
         CPPUNIT_ASSERT( mDamageComp->Unregister( id2 ) );
         CPPUNIT_ASSERT( ! mDamageComp->HasRegistered( id2 ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id1 ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id3 ) );

         CPPUNIT_ASSERT( mDamageComp->Unregister( id3 ) );
         CPPUNIT_ASSERT( ! mDamageComp->HasRegistered( id3 ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id1 ) );

         CPPUNIT_ASSERT( mDamageComp->Unregister( id1 ) );
         CPPUNIT_ASSERT( ! mDamageComp->HasRegistered( id1 ) );

         // --- Test unregistering when already unregistered
         CPPUNIT_ASSERT( ! mDamageComp->Unregister( id1 ) );

         // Re-register all entities and test clear
         CPPUNIT_ASSERT( mDamageComp->Register( *entities[0] ) );
         CPPUNIT_ASSERT( mDamageComp->Register( *entities[1] ) );
         CPPUNIT_ASSERT( mDamageComp->Register( *entities[2] ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id1 ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id2 ) );
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( id3 ) );
         // --- Clear
         mDamageComp->ClearRegisteredEntities();
         CPPUNIT_ASSERT( ! mDamageComp->HasRegistered( id1 ) );
         CPPUNIT_ASSERT( ! mDamageComp->HasRegistered( id2 ) );
         CPPUNIT_ASSERT( ! mDamageComp->HasRegistered( id3 ) );



         // Test adding tables
         CPPUNIT_ASSERT( mDamageComp->AddMunitionDamageTable( table1 ) );
         CPPUNIT_ASSERT( mDamageComp->AddMunitionDamageTable( table2 ) );
         CPPUNIT_ASSERT( mDamageComp->AddMunitionDamageTable( table3 ) );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName1 ) == table1.get() );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName2 ) == table2.get() );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName3 ) == table3.get() );

         // Test removing tables
         CPPUNIT_ASSERT( mDamageComp->RemoveMunitionDamageTable( tableName2 ) );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName2 ) == NULL );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName1 ) == table1.get() );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName3 ) == table3.get() );

         CPPUNIT_ASSERT( mDamageComp->RemoveMunitionDamageTable( tableName3 ) );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName3 ) == NULL );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName1 ) == table1.get() );

         CPPUNIT_ASSERT( mDamageComp->RemoveMunitionDamageTable( tableName1 ) );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName1 ) == NULL );

         // Re-add all tables
         CPPUNIT_ASSERT( mDamageComp->AddMunitionDamageTable( table1 ) );
         CPPUNIT_ASSERT( mDamageComp->AddMunitionDamageTable( table2 ) );
         CPPUNIT_ASSERT( mDamageComp->AddMunitionDamageTable( table3 ) );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName1 ) == table1.get() );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName2 ) == table2.get() );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName3 ) == table3.get() );

         // Test clearing tables
         mDamageComp->ClearTables();
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName1 ) == NULL );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName2 ) == NULL );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( tableName3 ) == NULL );


         mGM->RemoveComponent(*mDamageComp);
         // Must set it back to "" so it will read the config again
         mDamageComp->SetDefaultMunitionName("");
         mDamageComp->SetDefaultKineticRoundMunitionName("");
         std::string defaultMunition = "HE Medium";
         std::string defaultMunitionKinetic = "50 Cal Round";
         GetGlobalApplication().SetConfigPropertyValue(SimCore::Components::MunitionsComponent::CONFIG_PROP_MUNITION_DEFAULT, defaultMunition);
         GetGlobalApplication().RemoveConfigPropertyValue(mDamageComp->GetName() + "." + SimCore::Components::MunitionsComponent::CONFIG_PROP_MUNITION_DEFAULT);
         GetGlobalApplication().SetConfigPropertyValue(
                  mDamageComp->GetName() + "." + SimCore::Components::MunitionsComponent::CONFIG_PROP_MUNITION_KINETIC_ROUND_DEFAULT, defaultMunitionKinetic);
         mGM->AddComponent(*mDamageComp, dtGame::GameManager::ComponentPriority::NORMAL);

         // Test default munition name property
         std::string defaultMunitionChanged("Default");
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The default munition should match the old config value.",
                  defaultMunition, mDamageComp->GetDefaultMunitionName());
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The default kinetic munition should match the config value.",
                  defaultMunitionKinetic, mDamageComp->GetDefaultKineticRoundMunitionName());
         mDamageComp->SetDefaultMunitionName( defaultMunitionChanged );
         CPPUNIT_ASSERT_EQUAL( defaultMunitionChanged, mDamageComp->GetDefaultMunitionName() );
         mDamageComp->SetDefaultKineticRoundMunitionName( defaultMunitionChanged );
         CPPUNIT_ASSERT_EQUAL( defaultMunitionChanged, mDamageComp->GetDefaultKineticRoundMunitionName() );

         mGM->RemoveComponent(*mDamageComp);
         // Must set it back to "" so it will read the config again
         mDamageComp->SetDefaultMunitionName("");
         defaultMunition = "HE Medium 2";
         GetGlobalApplication().SetConfigPropertyValue(mDamageComp->GetName() + "." +SimCore::Components::MunitionsComponent::CONFIG_PROP_MUNITION_DEFAULT, defaultMunition);
         mGM->AddComponent(*mDamageComp, dtGame::GameManager::ComponentPriority::NORMAL);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The default munition should match the new config value, which should override the old.",
                  defaultMunition, mDamageComp->GetDefaultMunitionName());

         mGM->RemoveComponent(*mDamageComp);
         GetGlobalApplication().SetConfigPropertyValue(mDamageComp->GetName() + "." + SimCore::Components::MunitionsComponent::CONFIG_PROP_MUNITION_DEFAULT, "jobo");
         GetGlobalApplication().SetConfigPropertyValue(mDamageComp->GetName() + "." + SimCore::Components::MunitionsComponent::CONFIG_PROP_MUNITION_KINETIC_ROUND_DEFAULT, "boo");

         mDamageComp->SetDefaultMunitionName(defaultMunition);
         mDamageComp->SetDefaultKineticRoundMunitionName(defaultMunitionKinetic);

         mGM->AddComponent(*mDamageComp, dtGame::GameManager::ComponentPriority::NORMAL);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The default munition value should not have changed because it was set prior to being added to the GM.",
                  defaultMunition, mDamageComp->GetDefaultMunitionName());

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The default kinetic munition value should not have changed because it was set prior to being added to the GM.",
                  defaultMunitionKinetic, mDamageComp->GetDefaultKineticRoundMunitionName());
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestDetonationActor()
      {
         // --- Create the munition type table
         dtCore::RefPtr<MunitionTypeTable> typeTable = new MunitionTypeTable;
         mDamageComp->SetMunitionTypeTable( typeTable );

         // --- Create an new munition type actor
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> newType;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, newType );
         SimCore::Actors::MunitionTypeActor* munitionType;
         newType->GetActor(munitionType);
         std::string testMunitionName( "Test Munition" );

         // --- Create an effects damage actor that will be used by the munition type
         dtCore::RefPtr<SimCore::Actors::MunitionEffectsInfoActorProxy> newEffectsInfo;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_EFFECTS_INFO_ACTOR_TYPE, newEffectsInfo );
         SimCore::Actors::MunitionEffectsInfoActor* effectsInfo = NULL;
         newEffectsInfo->GetActor(effectsInfo);
         effectsInfo->SetSmokeLifeTime( 0.0f );

         CPPUNIT_ASSERT_MESSAGE( "A new MunitionTypeActor should have been created", munitionType);

         // --- Add it to the table so the Munition Component can find it
         newType->SetName(testMunitionName);
         munitionType->SetDamageType("High Explosive");
         munitionType->SetEffectsInfoActor(newEffectsInfo.get());
         typeTable->AddMunitionType( newType );

         osg::Vec3 tankLocation(0.0f, 0.0f, 0.0f);
         RefPtr<DetonationMessage> detMsg;
         mGM->GetMessageFactory().CreateMessage(MessageType::DETONATION, detMsg);

         CPPUNIT_ASSERT(detMsg.valid());
         detMsg->SetSource(*mMachineInfo);
         detMsg->SetMunitionType(testMunitionName);
         //put the detonation in the air so we can make sure it clamps down on the tank.
         detMsg->SetDetonationLocation(osg::Vec3(tankLocation.x(), tankLocation.y(),
         tankLocation.z() + 40.0f));

         mGM->SendMessage(*detMsg);

         RefPtr<SimCore::Actors::PlatformActorProxy> entityAP;
         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, entityAP);
         CPPUNIT_ASSERT(entityAP.valid());

         std::string model("StaticMeshes:T80:t80u_good.ive");
         entityAP->GetProperty("Non-damaged actor")->FromString(model);
         dtCore::Transform xform;
         entityAP->GetGameActor().GetTransform(xform);
         xform.SetTranslation(tankLocation);
         SimCore::Actors::BaseEntity* curEntity = dynamic_cast<SimCore::Actors::BaseEntity*> (&(entityAP->GetGameActor()));
         curEntity->GetComponent<dtGame::DeadReckoningHelper>()->SetAutoRegisterWithGMComponent(false);
         mGM->AddActor(*entityAP, false, false);

         dtCore::AppSleep(10);
         dtCore::System::GetInstance().Step();

         std::vector<dtDAL::ActorProxy*> container;
         mGM->GetAllActors(container);
         CPPUNIT_ASSERT_MESSAGE("The actors container should not be empty.", !container.empty());
         unsigned numActors = container.size(); // numActors exists for debugging purposes
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The actors container should have 2 actors in it.", 2U, numActors);
         mGM->FindActorsByType(*SimCore::Actors::EntityActorRegistry::DETONATION_ACTOR_TYPE, container);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The actors container should have 1 detonation actors in it.", 1U, unsigned(container.size()));
         RefPtr<dtDAL::ActorProxy> proxy = container[0];
         SimCore::Actors::DetonationActorProxy* dap = dynamic_cast<SimCore::Actors::DetonationActorProxy*>(proxy.get());
         CPPUNIT_ASSERT_MESSAGE("The 1 actor in the GM should a be a detonation actor, hence the dynamic_cast should not have failed", dap != NULL);
         SimCore::Actors::DetonationActor& detActor = static_cast<SimCore::Actors::DetonationActor&>(dap->GetGameActor());
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The detonation actor should have 0 lingering shot seconds.",0.0f, detActor.GetSmokeLifeTime());
         detActor.GetTransform(xform);
         osg::Vec3 pos;
         xform.GetTranslation(pos);
         //         CPPUNIT_ASSERT_MESSAGE("The entity should be quite a bit lower than it's start position because of clamping.", xform.GetTranslation().z() < (tankLocation.z() + 10.0f));
         CPPUNIT_ASSERT_EQUAL_MESSAGE("X position should be the same", tankLocation.x(), pos.x());
         CPPUNIT_ASSERT_MESSAGE("Y position should be the same", osg::equivalent(tankLocation.y(), pos.y(), 0.000001f));
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestDefaultMunition()
      {
         //mDamageComp->LoadMunitionTypeTable("UnitTestMunitionTypesMap");

         // Test Default Munition Feature
         // --- Maintain a reference to at least one munition
         const SimCore::Actors::MunitionTypeActor* grenadeMunition
            = mDamageComp->GetMunition("Grenade");
         CPPUNIT_ASSERT( grenadeMunition != NULL );

         // --- Pick a munition to act as a default and ensure it exists in the
         //     munition map.
         std::string defaultMunitionName("Generic Explosive");
         const SimCore::Actors::MunitionTypeActor* defaultMunition
            = mDamageComp->GetMunition(defaultMunitionName);
         CPPUNIT_ASSERT( defaultMunition != NULL );

         std::string fakeMunitionName("FAKE");
         // Ensure a valid munition is returned.
         CPPUNIT_ASSERT( mDamageComp->GetMunition( grenadeMunition->GetName() ) == grenadeMunition );
         CPPUNIT_ASSERT( mDamageComp->GetMunition( grenadeMunition->GetName(), defaultMunitionName ) == grenadeMunition );
         // Ensure a default munition is returned.
         CPPUNIT_ASSERT( mDamageComp->GetMunition( fakeMunitionName, defaultMunitionName ) == defaultMunition );
         CPPUNIT_ASSERT( mDamageComp->GetMunition( defaultMunitionName ) == defaultMunition );
         // Ensure a bad default munition name for the second parameter does not
         // compromise the return of a valid munition.
         CPPUNIT_ASSERT( mDamageComp->GetMunition( grenadeMunition->GetName(), defaultMunitionName ) == grenadeMunition );
         // Ensure a bad munition names for both parameters causes the method to return NULL.
         CPPUNIT_ASSERT( mDamageComp->GetMunition( fakeMunitionName, fakeMunitionName ) == NULL );
         CPPUNIT_ASSERT( mDamageComp->GetMunition( fakeMunitionName ) == NULL );


         // Test accessing effects info with the similar method, GetMunitionEffectsInfo.
         //
         // --- Maintain a reference to the default effect info object so that it can be
         //     compared with subsequent calls to GetMunitionEffectsInfo
         const SimCore::Actors::MunitionEffectsInfoActor* defaultEffects
            = dynamic_cast<const SimCore::Actors::MunitionEffectsInfoActor*>(defaultMunition->GetEffectsInfoActor());
         const SimCore::Actors::MunitionEffectsInfoActor* grenadeEffects
            = dynamic_cast<const SimCore::Actors::MunitionEffectsInfoActor*>(grenadeMunition->GetEffectsInfoActor());
         CPPUNIT_ASSERT( defaultEffects != NULL );
         CPPUNIT_ASSERT( grenadeEffects != NULL );

         // --- Create an empty munition that has no effects info reference.
         //     This will be used to test that GetMunitionEffectsInfo will
         //     return the effects from the default munition.
         dtCore::RefPtr<dtDAL::ActorProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy );
         SimCore::Actors::MunitionTypeActor* emptyMunition = NULL;
         proxy->GetActor( emptyMunition );
         CPPUNIT_ASSERT( emptyMunition != NULL );
         CPPUNIT_ASSERT( emptyMunition->GetEffectsInfoActor() == NULL );

         // --- Perform the tests on GetMunitionEffectsInfo
         CPPUNIT_ASSERT( mDamageComp->GetMunitionEffectsInfo( *grenadeMunition, defaultMunitionName ) == grenadeEffects );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionEffectsInfo( *emptyMunition, defaultMunitionName ) == defaultEffects );
         // --- Test it with a non-existent default munition name
         CPPUNIT_ASSERT( mDamageComp->GetMunitionEffectsInfo( *grenadeMunition, fakeMunitionName ) == grenadeEffects );
         CPPUNIT_ASSERT( mDamageComp->GetMunitionEffectsInfo( *emptyMunition, fakeMunitionName ) == NULL );
      }

      dtCore::RefPtr<SimCore::Actors::BaseEntity> MunitionsComponentTests::SetupTestEntityAndDamageHelper(bool autoSendMessages)
      {
         // Chew up any messages that could cause problems.
         // In this case, an INFO_RESTART message was causing trouble.
         dtCore::System::GetInstance().Step();

         // Create a test entity
         dtCore::RefPtr<SimCore::Actors::BaseEntity> entity;
         std::vector<dtCore::RefPtr<SimCore::Actors::BaseEntity> > entities;
         CreateTestEntities( entities, 1, true );
         entity = dynamic_cast<SimCore::Actors::BaseEntity*> (&(entities[0]->GetGameActorProxy().GetGameActor()));
         CPPUNIT_ASSERT_MESSAGE( "A valid test entity should have been created",
            entity.valid() );

         // Create all necessary objects for detonation test runs
         // --- Load all tables needed for the component to map to damage tables.
         //mDamageComp->LoadMunitionTypeTable("UnitTestMunitionTypesMap");
         mDamageComp->LoadMunitionDamageTables("Configs:UnitTestsConfig.xml");

         // -- Register the entity
         entity->SetMunitionDamageTableName(VEHICLE_MUNITION_TABLE_NAME);
         // unregister for tick local on this entity because it will auto send updates on damage state changes, which will confuse the test.
         entity->GetGameActorProxy().UnregisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
         // NOTE: The component will try to load the munition table upon registration
         // and also link it to the helper created for the registered entity.
         mDamageComp->Register( *entity, autoSendMessages );
         dtCore::RefPtr<TestDamageHelper> helper = dynamic_cast<TestDamageHelper*>
            ( mDamageComp->GetHelperByEntityId( entity->GetUniqueId() ) );
         CPPUNIT_ASSERT_MESSAGE( "MunitionsComponent should have created a valid DamageHelper",
            helper.valid() );
         CPPUNIT_ASSERT_MESSAGE( "MunitionsComponent should have linked a valid MunitionDamageTable to the new helper",
            helper->GetMunitionDamageTable() != NULL );
         CPPUNIT_ASSERT_MESSAGE( "MunitionsComponent should have linked a valid MunitionDamageTable to the new helper",
            helper->GetMunitionDamageTable()->GetName() == VEHICLE_MUNITION_TABLE_NAME );
         CPPUNIT_ASSERT_MESSAGE( "Damage helper should have 1.0 damage by default.",
            helper->GetMaxDamageAmount() == 1.0f );

         return entity;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMessagingDisabled()
      {
         dtCore::RefPtr<SimCore::Actors::BaseEntity> entity = SetupTestEntityAndDamageHelper(false);
         dtCore::RefPtr<TestDamageHelper> helper = dynamic_cast<TestDamageHelper*>
            ( mDamageComp->GetHelperByEntityId( entity->GetUniqueId() ) );

         mDamageComp->ResetTotalProcessedMessages();

         // Test the convenience method for setting the entities damage directly
         dtCore::System::GetInstance().Step();
         mDamageComp->ResetTotalProcessedMessages();
         mDamageComp->SetDamage( *entity, DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT_MESSAGE( "MunitionsComponent should have set entity damage to DAMAGE_KILL.",
            helper->GetDamageState() == DamageType::DAMAGE_KILL );
         dtCore::System::GetInstance().Step();
         // Setting the damage state WILL cause a full actor update from the BaseEntity. Changed recently.
         CPPUNIT_ASSERT_EQUAL_MESSAGE( "MunitionsComponent should have sent ONE network message when changing damage with its own SetDamage function.",
            int(mDamageComp->GetTotalProcessedMessages()), int(1) );
         CPPUNIT_ASSERT( entity->IsMobilityDisabled() );
         CPPUNIT_ASSERT( entity->IsFirepowerDisabled() );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMessageProcessing()
      {
         dtCore::RefPtr<SimCore::Actors::BaseEntity> entity = SetupTestEntityAndDamageHelper(true);
         dtCore::RefPtr<TestDamageHelper> helper = dynamic_cast<TestDamageHelper*>
            ( mDamageComp->GetHelperByEntityId( entity->GetUniqueId() ) );

         // --- Capture the loaded tables
         dtCore::RefPtr<MunitionDamageTable> table = helper->GetMunitionDamageTable();
         dtCore::RefPtr<MunitionTypeTable> typeTable = mDamageComp->GetMunitionTypeTable();

         // --- Initialize other munition damage test objects
         std::string munitionName1( "Munition1" );
         std::string munitionName2( "Munition2" );
         std::string damageName1( "Test Bullet" );
         std::string damageName2( "Test Explosion" );
         dtCore::RefPtr<MunitionDamage> damage1 = new MunitionDamage( damageName1 );
         dtCore::RefPtr<MunitionDamage> damage2 = new MunitionDamage( damageName2 );
         dtCore::RefPtr<DamageRanges> ranges2 = new DamageRanges( "RangeMax" );

         // --- Add some test munition type actors that map to the previous munition damage names
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> munitionTypeProxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, munitionTypeProxy );
         munitionTypeProxy->SetName( munitionName1 );
         SimCore::Actors::MunitionTypeActor* munitionType1 = NULL;
         munitionTypeProxy->GetActor( munitionType1 );
         munitionType1->SetDamageType( damageName1 );
         munitionType1->SetFamily(SimCore::Actors::MunitionFamily::FAMILY_ROUND);
         CPPUNIT_ASSERT( typeTable->AddMunitionType( munitionTypeProxy ) );

         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, munitionTypeProxy );
         munitionTypeProxy->SetName( munitionName2 );
         SimCore::Actors::MunitionTypeActor* munitionType2 = NULL;
         munitionTypeProxy->GetActor( munitionType2 );
         munitionType2->SetDamageType( damageName2 );
         munitionType2->SetFamily(SimCore::Actors::MunitionFamily::FAMILY_GENERIC_EXPLOSIVE);
         CPPUNIT_ASSERT( typeTable->AddMunitionType( munitionTypeProxy ) );

         // Initialize all test objects
         // --- Info 1
         float newtonForce = 50.0f;
         damage1->SetNewtonForce( newtonForce );
         damage1->SetDirectFireProbabilities( 0.5f, 0.4f, 0.0f, 0.0f, 0.1f );

         // --- Ranges (for Info2, using simple numbers)
         ranges2->SetAngleOfFall(90.0f);
         // --- Create shells of damage area
         float rangeMax = 100.0;
         ranges2->SetForwardRanges( rangeMax, rangeMax-20.0f, rangeMax-40.0f, rangeMax-60.0f );
         ranges2->SetDeflectRanges( rangeMax, rangeMax-20.0f, rangeMax-40.0f, rangeMax-60.0f );

         // --- Info 2
         float cutoffRange = 150.0f;
         damage2->SetNewtonForce( newtonForce );
         // NOTE: the following probabilities get laid out in a linear fashion
         // over which the overall damage probability is compared, starting from lowest
         // to highest (N,M,F,MF,K).
         // The first check where overall damage probability is less than the probability
         // of a damage type wins. In this case, anything lower than 0.25 will result
         // in Mobility damage (M).
         // The values are re-interpreted to: 0.0(N), 0.25(M), 0.4(F), 0.6(MF), 1.0(K)
         damage2->SetIndirectFireProbabilities( 0.0f, 1.0f, 1.0f, 1.0f, 1.0f );
         damage2->SetCutoffRange( cutoffRange );
         damage2->SetDamageRangesMax( ranges2 );

         // --- Table
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage1 ) );
         CPPUNIT_ASSERT( table->AddMunitionDamage( damage2 ) );

         // --- Helper
         helper->SetMunitionDamageTable( table );
         // --- Set the helper's damage levels to be used
         helper->GetDamageLevels().Set(0.5f,0.45f,0.0f,0.0f,0.05f);

         // Test Direct Fire
         osg::Vec3 detLocation, trajectory(0.0f,0.0f,-1.0f);
         helper->SetUsedProbability(1.0f); // avoid randomness in this test

         // NOTE: current damage accumulation is figured as follows:
         // helper.damageModifier += (munition.mobilityDamage/2 + munition.killDamage)
         //    * helper.vulnerability * usedProbability * munition.firedQuantity

         // --- Test a survivable hit
         SendDetonationMessage( munitionName1, detLocation, entity.get(), &trajectory );
         float damageMod = helper->GetCurrentDamageRatio();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.3f, damageMod, 0.001 );
         CPPUNIT_ASSERT_MESSAGE( "Entity should be NO_DAMAGE",
            entity->GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE );

         // --- Test a mobility kill hit
         SendDetonationMessage( munitionName1, detLocation, entity.get(), &trajectory );
         damageMod = helper->GetCurrentDamageRatio();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.6f, damageMod, 0.001 );
         CPPUNIT_ASSERT_MESSAGE( "Entity should be SLIGHT DAMAGE",
            entity->GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE );

         SendDetonationMessage( munitionName1, detLocation, entity.get(), &trajectory );
         damageMod = helper->GetCurrentDamageRatio();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.9f, damageMod, 0.001 );
         CPPUNIT_ASSERT_MESSAGE( "Entity should be SLIGHT DAMAGE",
            entity->GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE );

         // --- Test a kill hit
         SendDetonationMessage( munitionName1, detLocation, entity.get(), &trajectory );
         damageMod = helper->GetCurrentDamageRatio();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0f, damageMod, 0.001 );
         CPPUNIT_ASSERT_MESSAGE( "Entity should be DESTROYED",
            entity->GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED );

         // --- Test a survivable hit, make sure damage was NOT decreased
         SendDetonationMessage( munitionName1, detLocation, entity.get(), &trajectory );
         damageMod = helper->GetCurrentDamageRatio();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0f, damageMod, 0.001 );
         CPPUNIT_ASSERT_MESSAGE( "Entity should still be DESTROYED",
            entity->GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED );

         // Test Indirect Fire (uses Carleton Equation)
         helper->SetDamage( DamageType::DAMAGE_NONE );
         // --- Use 100% vulnerability
         helper->SetUsedProbability(1.0f);

         // --- Set the damage levels to be used in this test.
         //     These levels represent the strength of the vehicle and should
         //     add up to 1.0, like a pie chart.
         helper->GetDamageLevels().Set(0.0f,0.4f,0.55f,0.0f,0.05f);
         // Probability 0.347202263 @ target offset(50,50) -> Mobility Damage  ( prob < 0.4 )
         // Probability 0.599011779 @ target offset( 0,50) -> Firepower Damage ( prob < 0.4+0.55 )
         // Probability 1.146340378 @ target offset( 0,25) -> Kill Damage      ( prob <= 1.0 )


         // NOTE: Entity is not passed into the Detonation function so that the
         // detonation will be processed by the MunitionsComponent as Indirect Fire.

         // --- Detonation above, just outside the cutoff range
         detLocation.set( 0.0f, 0.0f, 160.0f );
         SendDetonationMessage( munitionName2, detLocation, NULL, &trajectory );
         DamageType* damageType = &helper->GetDamageState();
         CPPUNIT_ASSERT( *damageType == DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT( ! entity->IsMobilityDisabled() );
         CPPUNIT_ASSERT( ! entity->IsFirepowerDisabled() );

         // --- Detonation above
         detLocation.set( 0.0f, 0.0f, 50.0f );
         mDamageComp->SetDamage( *entity, DamageType::DAMAGE_NONE );
         // Probability of 1.0(F) should gradate to 0.676633846 at 80.0m range,
         // resulting in FIREPOWER damage (according to probabilities 1.0f, 1.0f, 1.0f, 1.0f)
         SendDetonationMessage( munitionName2, detLocation, NULL, &trajectory );

         float mobProb = damage2->GetProbability_CarletonEquation( 1.0f, 50.0f, 0.0f, rangeMax, rangeMax );
         float killProb = damage2->GetProbability_CarletonEquation( 1.0f, 50.0f, 0.0f, rangeMax-60.0f, rangeMax-60.0f );
         damageMod = helper->GetCurrentDamageRatio();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( mobProb*0.5f+killProb, damageMod, 0.001 );

         damageType = &helper->GetDamageState();
         CPPUNIT_ASSERT( *damageType == DamageType::DAMAGE_FIREPOWER );
         CPPUNIT_ASSERT( ! entity->IsMobilityDisabled() );
         CPPUNIT_ASSERT( entity->IsFirepowerDisabled() );

         // --- Detonation above and to the side
         detLocation.set( 0.0f, -50.0f, 50.0f );
         mDamageComp->SetDamage( *entity, DamageType::DAMAGE_NONE );
         // Probability of 1.0(M) should gradate to 0.60653066 at 100.0m range,
         // resulting in MOBILITY damage (according to probabilities 1.0f, 1.0f, 1.0f, 1.0f)
         SendDetonationMessage( munitionName2, detLocation, NULL, &trajectory );

         mobProb = damage2->GetProbability_CarletonEquation( 1.0f, 50.0f, 50.0f, rangeMax, rangeMax );
         killProb = damage2->GetProbability_CarletonEquation( 1.0f, 50.0f, 50.0f, rangeMax-60.0f, rangeMax-60.0f );
         damageMod = helper->GetCurrentDamageRatio();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( mobProb*0.5f+killProb, damageMod, 0.001 );

         // NOTE: Originally if the actor had NO damage, the detonation would cause it
         // to take on MOBILITY damage. However, since it already has FIREPOWER damage,
         // both damages combine to become total MOBILITY_FIREPOWER damage.
         damageType = &helper->GetDamageState();
         CPPUNIT_ASSERT( *damageType == DamageType::DAMAGE_MOBILITY );
         CPPUNIT_ASSERT( entity->IsMobilityDisabled() );
         CPPUNIT_ASSERT( ! entity->IsFirepowerDisabled() );

         // --- Move detonation really close
         detLocation.set( 0.0f, 25.0f, 0.0f );
         mDamageComp->SetDamage( *entity, DamageType::DAMAGE_NONE );
         // Probability of 1.0(K) should gradate to 0.676633846 at 40m range,
         // resulting in KILL damage (according to probabilities 1.0f, 1.0f, 1.0f, 1.0f)
         SendDetonationMessage( munitionName2, detLocation, NULL, &trajectory );

         mobProb = damage2->GetProbability_CarletonEquation( 1.0f, 25.0f, 0.0f, rangeMax, rangeMax );
         killProb = damage2->GetProbability_CarletonEquation( 1.0f, 25.0f, 0.0f, rangeMax-60.0f, rangeMax-60.0f );
         damageMod = helper->GetCurrentDamageRatio();
         killProb = mobProb*0.5f+killProb;
         if( killProb > 1.0f ) { killProb = 1.0f; }
         CPPUNIT_ASSERT_DOUBLES_EQUAL( killProb, damageMod, 0.001 );

         damageType = &helper->GetDamageState();
         CPPUNIT_ASSERT( *damageType == DamageType::DAMAGE_KILL );
         CPPUNIT_ASSERT( entity->IsMobilityDisabled() );
         CPPUNIT_ASSERT( entity->IsFirepowerDisabled() );



         // Test the convenience method for setting the entities damage directly
         dtCore::System::GetInstance().Step();
         mDamageComp->ResetTotalProcessedMessages();
         mDamageComp->SetDamage( *entity, DamageType::DAMAGE_NONE );
         CPPUNIT_ASSERT_MESSAGE( "MunitionsComponent should have set entity damage to NONE.",
            helper->GetDamageState() == DamageType::DAMAGE_NONE );
         dtCore::System::GetInstance().Step();
         // One message from the damage helper (being set to true in the constructor) and one from the entity.
         CPPUNIT_ASSERT_EQUAL_MESSAGE( "MunitionsComponent should have sent exactly two network message when changing damage with its own SetDamage function.",
            int(mDamageComp->GetTotalProcessedMessages()), int(2) );
         CPPUNIT_ASSERT( ! entity->IsMobilityDisabled() );
         CPPUNIT_ASSERT( ! entity->IsFirepowerDisabled() );



         // Test Delete message
         dtCore::RefPtr<dtGame::Message> msg;
         mGM->GetMessageFactory().CreateMessage( dtGame::MessageType::INFO_ACTOR_DELETED, msg );
         msg->SetAboutActorId( entity->GetUniqueId() );
         mGM->SendMessage( *msg );
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_MESSAGE("MunitionsComponent should remove all references to an entity upon INFO_ACTOR_DELETED message",
            ! mDamageComp->HasRegistered( entity->GetUniqueId() ) );
         CPPUNIT_ASSERT( mDamageComp->GetHelperByEntityId( entity->GetUniqueId() ) == NULL );



         // Test DetonationActor creation
         // --- Ensure GM is free of actors
         std::vector<dtDAL::ActorProxy*> proxies;
         mGM->DeleteAllActors(true);
         mGM->GetAllActors(proxies);
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_MESSAGE("No actors should exist in the Game Manager.", proxies.empty());

         TestDetonationActor();



         // Test Restart message
         // --- Ensure that the munition table still exists
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( VEHICLE_MUNITION_TABLE_NAME ) != NULL );

         std::vector<dtCore::RefPtr<SimCore::Actors::BaseEntity> > entities;
         // --- Re-register the entity
         CreateTestEntities( entities, 1, true );
         entity = dynamic_cast<SimCore::Actors::BaseEntity*> (&(entities[0]->GetGameActorProxy().GetGameActor()));
         CPPUNIT_ASSERT( entity.valid() );
         CPPUNIT_ASSERT( mDamageComp->Register(*entity, true, 5.0f));
         CPPUNIT_ASSERT( mDamageComp->HasRegistered( entity->GetUniqueId() ) );
         CPPUNIT_ASSERT( mDamageComp->GetHelperByEntityId( entity->GetUniqueId() ) != NULL );
         CPPUNIT_ASSERT_MESSAGE("Damage Component should have gotten max damage from entity.",
            5.0f == mDamageComp->GetHelperByEntityId(entity->GetUniqueId())->GetMaxDamageAmount());

         // --- Send the RESTART message
         mGM->GetMessageFactory().CreateMessage( dtGame::MessageType::INFO_RESTARTED, msg );
         mGM->SendMessage( *msg );
         dtCore::System::GetInstance().Step();

         // --- Ensure that the entity was cleared
         CPPUNIT_ASSERT_MESSAGE("MunitionsComponent should remove all references to an entity upon INFO_ACTOR_DELETED message",
            ! mDamageComp->HasRegistered( entity->GetUniqueId() ) );
         CPPUNIT_ASSERT( mDamageComp->GetHelperByEntityId( entity->GetUniqueId() ) == NULL );

         // --- Ensure that the munition table has NOT been removed; it will be reloaded on RESTART
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( VEHICLE_MUNITION_TABLE_NAME ) != NULL );

         // --- Send the MAP_UNLOADED message
         mGM->GetMessageFactory().CreateMessage( dtGame::MessageType::INFO_MAP_UNLOAD_BEGIN, msg );
         mGM->SendMessage( *msg );
         dtCore::System::GetInstance().Step();

         // --- Ensure that the munition table has been removed
         CPPUNIT_ASSERT( mDamageComp->GetMunitionDamageTable( VEHICLE_MUNITION_TABLE_NAME ) == NULL );

      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionConfigLoading()
      {
         unsigned int successes = mDamageComp->LoadMunitionDamageTables( "Configs:UnitTestsConfig.xml" );

         CPPUNIT_ASSERT_MESSAGE( "MunitionConfig should have loaded 1 table and returned 1 success.",
            successes == 1 );

         dtCore::RefPtr<MunitionDamageTable> table = mDamageComp->GetMunitionDamageTable( "UnitTestsVehicle" );
         CPPUNIT_ASSERT_MESSAGE( "MunitionsComponent should have a valid table loaded by MunitionConfig",
            table.valid() );

         CPPUNIT_ASSERT( table->GetMunitionDamage("Test1Bullet") != NULL );
         CPPUNIT_ASSERT( table->GetMunitionDamage("Test2Grenade") != NULL );
         //CPPUNIT_ASSERT( table->GetMunitionDamage("High Explosive") != NULL );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionEffectsInfoActorProperties()
      {
         dtCore::RefPtr<SimCore::Actors::MunitionEffectsInfoActorProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_EFFECTS_INFO_ACTOR_TYPE, proxy );

         CPPUNIT_ASSERT_MESSAGE("GameManager should be able create a MunitionEffectInfoActorProxy", proxy );
         SimCore::Actors::MunitionEffectsInfoActor* effectsInfo
            = static_cast<SimCore::Actors::MunitionEffectsInfoActor*>(proxy->GetActor());

         CPPUNIT_ASSERT( effectsInfo != NULL );

         // Creat test variables
         std::string testValue("string test");
         float value = 0.5f;

         CPPUNIT_ASSERT_MESSAGE("MunitionEffects fire flash probability should default to 1.0",
            effectsInfo->GetFireFlashProbability() == 1.0f );
         effectsInfo->SetFireFlashProbability( value );
         CPPUNIT_ASSERT( effectsInfo->GetFireFlashProbability() == 0.5f );
         // --- Test probability clamping
         effectsInfo->SetFireFlashProbability( 2.5f );
         CPPUNIT_ASSERT( effectsInfo->GetFireFlashProbability() == 1.0f );
         effectsInfo->SetFireFlashProbability( -3.7f );
         CPPUNIT_ASSERT( effectsInfo->GetFireFlashProbability() == 0.0f );

         // Ready a value for testing that will most certainly not be initial values.
         value = 12345.0f;

         CPPUNIT_ASSERT( effectsInfo->GetFireFlashTime() != value );
         effectsInfo->SetFireFlashTime( value );
         CPPUNIT_ASSERT( effectsInfo->GetFireFlashTime() == value );

         CPPUNIT_ASSERT( effectsInfo->GetFlySoundMaxDistance() != value );
         effectsInfo->SetFlySoundMaxDistance( value );
         CPPUNIT_ASSERT( effectsInfo->GetFlySoundMaxDistance() == value );

         CPPUNIT_ASSERT( effectsInfo->GetFireSoundMaxDistance() != value );
         effectsInfo->SetFireSoundMaxDistance( value );
         CPPUNIT_ASSERT( effectsInfo->GetFireSoundMaxDistance() == value );

         CPPUNIT_ASSERT( effectsInfo->GetGroundImpactSoundMaxDistance() != value );
         effectsInfo->SetGroundImpactSoundMaxDistance( value );
         CPPUNIT_ASSERT( effectsInfo->GetGroundImpactSoundMaxDistance() == value );

         CPPUNIT_ASSERT( effectsInfo->GetEntityImpactSoundMaxDistance() != value );
         effectsInfo->SetEntityImpactSoundMaxDistance( value );
         CPPUNIT_ASSERT( effectsInfo->GetEntityImpactSoundMaxDistance() == value );

         CPPUNIT_ASSERT( effectsInfo->GetSmokeLifeTime() != value );
         effectsInfo->SetSmokeLifeTime( value );
         CPPUNIT_ASSERT( effectsInfo->GetSmokeLifeTime() == value );

         CPPUNIT_ASSERT( effectsInfo->GetFlyEffect().empty() );
         effectsInfo->SetFlyEffect( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetFlyEffect() == testValue );

         CPPUNIT_ASSERT( effectsInfo->GetFlySound().empty() );
         effectsInfo->SetFlySound( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetFlySound() == testValue );

         CPPUNIT_ASSERT( effectsInfo->GetFireEffect().empty() );
         effectsInfo->SetFireEffect( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetFireEffect() == testValue );

         CPPUNIT_ASSERT( effectsInfo->GetFireSound().empty() );
         effectsInfo->SetFireSound( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetFireSound() == testValue );

         CPPUNIT_ASSERT( effectsInfo->GetGroundImpactEffect().empty() );
         effectsInfo->SetGroundImpactEffect( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetGroundImpactEffect() == testValue );

         CPPUNIT_ASSERT( effectsInfo->GetGroundImpactSound().empty() );
         effectsInfo->SetGroundImpactSound( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetGroundImpactSound() == testValue );

         CPPUNIT_ASSERT( effectsInfo->GetEntityImpactEffect().empty() );
         CPPUNIT_ASSERT( ! effectsInfo->HasEntityImpactEffect() );
         effectsInfo->SetEntityImpactEffect( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetEntityImpactEffect() == testValue );
         CPPUNIT_ASSERT( effectsInfo->HasEntityImpactEffect() );

         CPPUNIT_ASSERT( effectsInfo->GetEntityImpactSound().empty() );
         CPPUNIT_ASSERT( ! effectsInfo->HasEntityImpactSound() );
         effectsInfo->SetEntityImpactSound( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetEntityImpactSound() == testValue );
         CPPUNIT_ASSERT( effectsInfo->HasEntityImpactSound() );

         CPPUNIT_ASSERT( effectsInfo->GetSmokeEffect().empty() );
         effectsInfo->SetSmokeEffect( testValue );
         CPPUNIT_ASSERT( effectsInfo->GetSmokeEffect() == testValue );

         // Test light name properties
         const std::string& lightName = "testLightName";
         CPPUNIT_ASSERT_MESSAGE("Fire light name should be empty by default",
            effectsInfo->GetFireLight().empty() );
         effectsInfo->SetFireLight(lightName);
         CPPUNIT_ASSERT( effectsInfo->GetFireLight() == lightName );

         CPPUNIT_ASSERT_MESSAGE("Ground impact light name should be empty by default",
            effectsInfo->GetGroundImpactLight().empty() );
         effectsInfo->SetGroundImpactLight(lightName);
         CPPUNIT_ASSERT( effectsInfo->GetGroundImpactLight() == lightName );

         CPPUNIT_ASSERT_MESSAGE("Entity impact light name should be empty by default",
            effectsInfo->GetEntityImpactLight().empty() );
         effectsInfo->SetEntityImpactLight(lightName);
         CPPUNIT_ASSERT( effectsInfo->GetEntityImpactLight() == lightName );

         CPPUNIT_ASSERT_MESSAGE("Tracer light name should be empty by default",
            effectsInfo->GetTracerLight().empty() );
         effectsInfo->SetTracerLight(lightName);
         CPPUNIT_ASSERT( effectsInfo->GetTracerLight() == lightName );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionFamilyProperties()
      {
         CPPUNIT_ASSERT( ! SimCore::Actors::MunitionFamily::FAMILY_UNKNOWN.IsExplosive() );
         CPPUNIT_ASSERT( ! SimCore::Actors::MunitionFamily::FAMILY_ROUND.IsExplosive() );
         CPPUNIT_ASSERT( SimCore::Actors::MunitionFamily::FAMILY_EXPLOSIVE_ROUND.IsExplosive() );
         CPPUNIT_ASSERT( SimCore::Actors::MunitionFamily::FAMILY_GRENADE.IsExplosive() );
         CPPUNIT_ASSERT( SimCore::Actors::MunitionFamily::FAMILY_MINE.IsExplosive() );
         CPPUNIT_ASSERT( SimCore::Actors::MunitionFamily::FAMILY_MISSILE.IsExplosive() );
         CPPUNIT_ASSERT( SimCore::Actors::MunitionFamily::FAMILY_GENERIC_EXPLOSIVE.IsExplosive() );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestMunitionTypeActorProperties()
      {
         dtCore::RefPtr<dtDAL::ActorProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxy );
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> munitionType
            = dynamic_cast<SimCore::Actors::MunitionTypeActor*>(proxy->GetActor());

         // MunitionTypeActor is not drawable so it should have a NULL OSG node.
         CPPUNIT_ASSERT( munitionType->GetOSGNode() == NULL );

         CPPUNIT_ASSERT( munitionType->GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_UNKNOWN );
         munitionType->SetFamily( SimCore::Actors::MunitionFamily::FAMILY_GRENADE );
         CPPUNIT_ASSERT( munitionType->GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_GRENADE );

         CPPUNIT_ASSERT( munitionType->GetFuseType() == 0 );
         munitionType->SetFuseType( 1000 );
         CPPUNIT_ASSERT( munitionType->GetFuseType() == 1000 );

         CPPUNIT_ASSERT( munitionType->GetWarheadType() == 0 );
         munitionType->SetWarheadType( 5000 );
         CPPUNIT_ASSERT( munitionType->GetWarheadType() == 5000 );

         CPPUNIT_ASSERT( munitionType->GetTracerFrequency() == 0 );
         munitionType->SetTracerFrequency( 10 );
         CPPUNIT_ASSERT( munitionType->GetTracerFrequency() == 10 );

         std::string testString("TestFile");
         CPPUNIT_ASSERT( munitionType->GetModel() == "" );
         munitionType->SetModel( testString );
         CPPUNIT_ASSERT( munitionType->GetModel() == testString );

         testString = "TestDamageType";
         CPPUNIT_ASSERT( munitionType->GetDamageType() == "" );
         munitionType->SetDamageType( testString );
         CPPUNIT_ASSERT( munitionType->GetDamageType() == testString );

         SimCore::Actors::DISIdentifier testDIS(1,2,3,4,5,6,7);
         testString = testDIS.ToString();
         CPPUNIT_ASSERT( munitionType->GetDISIdentifier() != testDIS );
         munitionType->SetDISIdentifier( testDIS );
         CPPUNIT_ASSERT( munitionType->GetDISIdentifier() == testDIS );
         CPPUNIT_ASSERT( munitionType->GetDISIdentifierString() == testString );

         testDIS.Set(7,6,5,4,3,2,1);
         testString = testDIS.ToString();
         CPPUNIT_ASSERT( munitionType->GetDISIdentifierString() != testString );
         munitionType->SetDISIdentifierByString( testString );
         CPPUNIT_ASSERT( munitionType->GetDISIdentifierString() == testString );
         CPPUNIT_ASSERT( munitionType->GetDISIdentifier() == testDIS );

         dtCore::RefPtr<dtDAL::ActorProxy> proxy2;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_EFFECTS_INFO_ACTOR_TYPE, proxy2 );
         dtCore::RefPtr<SimCore::Actors::MunitionEffectsInfoActor> effectsInfo
            = static_cast<SimCore::Actors::MunitionEffectsInfoActor*>(proxy2->GetActor());
         CPPUNIT_ASSERT( munitionType->GetEffectsInfoDrawable() == NULL );
         CPPUNIT_ASSERT( munitionType->GetEffectsInfoActor() == NULL );
         munitionType->SetEffectsInfoActor( proxy2.get() );
         CPPUNIT_ASSERT( munitionType->GetEffectsInfoDrawable() == effectsInfo.get() );
         CPPUNIT_ASSERT( munitionType->GetEffectsInfoActor() == effectsInfo.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestTracerEffectProperties()
      {
         dtCore::RefPtr<SimCore::Components::TracerEffect> tracer = new SimCore::Components::TracerEffect(1.0f,1.0f);

         // Test variables for update iterations
         osg::Vec3 position(-2.0,0.0,-2.0), direction(1.0,0.0,1.0);
         direction.normalize();
         float speed = 5.0;
         float maxLife = 5.0;
         osg::Vec3 velocity( direction * speed );

         CPPUNIT_ASSERT( tracer->IsActive() );

         CPPUNIT_ASSERT( tracer->GetMaxLifeTime() != maxLife );
         tracer->SetMaxLifeTime( maxLife );
         CPPUNIT_ASSERT( tracer->GetMaxLifeTime() == maxLife );

         CPPUNIT_ASSERT( tracer->GetPosition().length() == 0.0 );
         tracer->SetPosition( position );
         CPPUNIT_ASSERT( tracer->GetPosition() == position );

         // Test velocity
         CPPUNIT_ASSERT( tracer->GetVelocity().length() == 0.0 );
         tracer->SetVelocity( velocity );
         float tolerance = 0.001f;
         osg::Vec3 testVec1( tracer->GetVelocity() );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( velocity[0], testVec1[0], tolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( velocity[1], testVec1[1], tolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( velocity[2], testVec1[2], tolerance );
         CPPUNIT_ASSERT( tracer->GetPosition() == position ); // should not be affected by velocity without a call to Update.

         CPPUNIT_ASSERT( tracer->IsVisible() );
         tracer->SetVisible( false );
         CPPUNIT_ASSERT( ! tracer->IsVisible() );



         // Test execute
         tracer->SetPosition( position );
         tracer->Execute( maxLife );
         CPPUNIT_ASSERT( tracer->GetMaxLifeTime() == maxLife );
         CPPUNIT_ASSERT( tracer->IsActive() );
         CPPUNIT_ASSERT( tracer->IsVisible() );

         // --- Simulate passed time using Update
         tolerance = 1.0f; // many float multiplications yield a big difference due to imprecision
         float timeStep = 0.25f;
         velocity.set( tracer->GetVelocity() );
         osg::Vec3 expectedPosition( tracer->GetPosition() + (velocity * maxLife) );
         for( float iterTime = 0; iterTime < maxLife; iterTime += timeStep )
         {
            tracer->Update( iterTime );
         }
         CPPUNIT_ASSERT( ! tracer->IsActive() );
         CPPUNIT_ASSERT( ! tracer->IsVisible() );

         testVec1.set( tracer->GetPosition() );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedPosition[0], testVec1[0], tolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedPosition[1], testVec1[1], tolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedPosition[2], testVec1[2], tolerance );



         // Test execute again to make sure the effect can be reused
         tracer->SetPosition( position );
         tracer->Execute( maxLife );
         CPPUNIT_ASSERT( tracer->GetMaxLifeTime() == maxLife );
         CPPUNIT_ASSERT( tracer->IsActive() );
         CPPUNIT_ASSERT( tracer->IsVisible() );

         // --- Simulate passed time using Update
         expectedPosition.set( tracer->GetPosition() + (velocity * maxLife) );
         for( float iterTime = 0; iterTime < maxLife; iterTime += timeStep )
         {
            tracer->Update( iterTime );
         }
         CPPUNIT_ASSERT( ! tracer->IsActive() );
         CPPUNIT_ASSERT( ! tracer->IsVisible() );

         testVec1.set( tracer->GetPosition() );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedPosition[0], testVec1[0], tolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedPosition[1], testVec1[1], tolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedPosition[2], testVec1[2], tolerance );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestWeaponEffectProperties()
      {
         dtCore::RefPtr<SimCore::Components::WeaponEffect> effect = new SimCore::Components::WeaponEffect();

         // Test Flash Probability
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect flash probability should default to 1.0",
            effect->GetFlashProbability() == 1.0f );
         effect->SetFlashProbability( 0.5f );
         CPPUNIT_ASSERT( effect->GetFlashProbability() == 0.5f );
         // --- Test probability value clamping
         effect->SetFlashProbability( 5.0f );
         CPPUNIT_ASSERT( effect->GetFlashProbability() == 1.0f );
         effect->SetFlashProbability( -16.0f );
         CPPUNIT_ASSERT( effect->GetFlashProbability() == 0.0f );

         // Test Visible
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect should default to visible",
            effect->IsVisible() );
         effect->SetVisible( false );
         CPPUNIT_ASSERT( ! effect->IsVisible() );
         effect->SetVisible( true );
         CPPUNIT_ASSERT( effect->IsVisible() );

         // Test Flash Time
         float value = 12345.0f; // this arbitrary value will most likely not be the default value.
         CPPUNIT_ASSERT( effect->GetFlashTime() != value );
         effect->SetFlashTime( value );
         CPPUNIT_ASSERT( effect->GetFlashTime() == value );

         // Test Owner
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an BaseEntityActorProxy", proxy.valid() );
         SimCore::Actors::BaseEntity* entity = static_cast<SimCore::Actors::BaseEntity*>(proxy->GetActor());
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an Entity", entity != NULL );

         CPPUNIT_ASSERT_MESSAGE("WeaponEffect owner should be NULL by default",
            effect->GetOwner() == NULL );
         effect->SetOwner( entity );
         CPPUNIT_ASSERT( effect->GetOwner() == entity );


         // Test Sound Loading
         std::string soundFile("Sounds/weapon_tank_fire.wav");
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect sound should be NULL by default",
            effect->GetSound() == NULL );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect should be able to load a sound file",
            effect->LoadSound( soundFile ) );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::LoadSound should have assigned the sound",
            effect->GetSound() != NULL );

         // Test setting Sound directly
         dtCore::RefPtr<dtAudio::Sound> sound = effect->GetSound();
         effect->SetSound( NULL, false );
         CPPUNIT_ASSERT( effect->GetSound() == NULL );
         effect->SetSound( sound.get() );
         CPPUNIT_ASSERT( effect->GetSound() != NULL );


         // Test Execute ( similar to Flash Time ) and Update
         value = 2.0f;
         // --- Simulate a listener far enough away so that the sound will be heard after 1.5 seconds
         osg::Vec3 listenerPosition( 500.0f, 0.0f, 0.0f ); // 1.5s * 350m/s = 525m
         float expectedDelay = 1.42857f;
         CPPUNIT_ASSERT( ! effect->IsSoundPlayed() );
         CPPUNIT_ASSERT( effect->GetSoundDelay() == 0.0f );

         effect->Execute( value, listenerPosition );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::Execute should set the flash time",
            effect->GetFlashTime() == value );
         CPPUNIT_ASSERT( effect->GetSoundDelay() != 0.0f );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedDelay, effect->GetSoundDelay(), 0.001 );

         // --- Avoid playing the sound within the unit test
         effect->SetSound( NULL, false );
         CPPUNIT_ASSERT( effect->GetSound() == NULL );


         // Test Update ( affects visibility and flash timers )
         float updateDelta = 0.5f;
         float runningTime = updateDelta;

         effect->Update( updateDelta ); // 0.5;
         CPPUNIT_ASSERT_DOUBLES_EQUAL( value - runningTime, effect->GetFlashTime(), 0.001 );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( runningTime, effect->GetTimeSinceFlash(), 0.001 );
         CPPUNIT_ASSERT( effect->IsVisible() );
         CPPUNIT_ASSERT( ! effect->IsSoundPlayed() );

         runningTime += updateDelta;
         effect->Update( updateDelta ); // 1.0;
         CPPUNIT_ASSERT_DOUBLES_EQUAL( value - runningTime, effect->GetFlashTime(), 0.001 );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( runningTime, effect->GetTimeSinceFlash(), 0.001 );
         CPPUNIT_ASSERT( effect->IsVisible() );
         CPPUNIT_ASSERT( ! effect->IsSoundPlayed() );

         runningTime += updateDelta;
         effect->Update( updateDelta ); // 1.5;
         CPPUNIT_ASSERT_DOUBLES_EQUAL( value - runningTime, effect->GetFlashTime(), 0.001 );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( runningTime, effect->GetTimeSinceFlash(), 0.001 );
         CPPUNIT_ASSERT( effect->IsVisible() );
         CPPUNIT_ASSERT( effect->IsSoundPlayed() );

         runningTime += updateDelta;
         effect->Update( updateDelta ); // 2.0;
         float lastFlashTime = effect->GetFlashTime();
         CPPUNIT_ASSERT_DOUBLES_EQUAL( value - runningTime, lastFlashTime, 0.001 );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( runningTime, effect->GetTimeSinceFlash(), 0.001 );
         CPPUNIT_ASSERT_MESSAGE( "WeaponEffect should be invisible when its flash time reaches at or below 0",
            ! effect->IsVisible() );

         runningTime += updateDelta;
         effect->Update( updateDelta ); // 2.0;
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("WeaponEffect flash time should no longer be updated after reaching 0",
            lastFlashTime, effect->GetFlashTime(), 0.001 );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( runningTime, effect->GetTimeSinceFlash(), 0.001 );
         CPPUNIT_ASSERT_MESSAGE( "WeaponEffect should still be invisible while the effect is still being updated with a flash time of 0",
            ! effect->IsVisible() );

         // Test Flash Loading
         std::string particleFile("Particles/weapon_tank_flash.osg");
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect flash particles should be NULL by default",
            effect->GetFlash() == NULL );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect should be able to load a particle file",
            effect->LoadFlash( particleFile ) );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::LoadFlash should have assigned the flash particles",
            effect->GetFlash() != NULL );

         // Test Attach
         osg::ref_ptr<osgSim::DOFTransform> testDOF = new osgSim::DOFTransform;
         testDOF->setName("TestDOF");
         dtCore::ParticleSystem* flash = effect->GetFlash();
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect should have a NULL DOF by default",
            effect->GetDOF() == NULL );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::Attach should be successful with a valid owner",
            effect->Attach( testDOF.get() ) );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::Attach should have assigned the DOF transform",
            effect->GetDOF() == testDOF.get() );
         CPPUNIT_ASSERT( testDOF->getNumChildren() == 1 );
         CPPUNIT_ASSERT( flash->GetParent() != NULL );

         // --- Test Attaching again, avoid adding the flash to the DOF a second time
         effect->Attach( testDOF.get() );
         CPPUNIT_ASSERT( testDOF->getNumChildren() == 1 );
         CPPUNIT_ASSERT( flash->GetParent() != NULL );

         // Test Detach
         CPPUNIT_ASSERT( effect->Detach() );
         CPPUNIT_ASSERT( effect->GetOwner() == NULL );
         CPPUNIT_ASSERT( effect->GetDOF() == NULL );
         CPPUNIT_ASSERT( testDOF->getNumChildren() == 0 );
         CPPUNIT_ASSERT( flash->GetParent() == NULL );

         // Test Attach with No Owner
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::Attach should NOT be successful with an invalid owner",
            ! effect->Attach( testDOF.get() ) );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::Attach should NOT have assigned a DOF transform if Attach failed",
            effect->GetDOF() == NULL );

         // Re-attach to test Clear later
         effect->SetOwner( entity );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::Attach should be successful with a valid owner, again",
            effect->Attach( testDOF.get() ) );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect::Attach should have assigned the DOF transform, again",
            effect->GetDOF() == testDOF.get() );

         // Test setting Flash directly
         dtCore::RefPtr<dtCore::ParticleSystem> particles = effect->GetFlash();
         effect->SetFlash( NULL );
         CPPUNIT_ASSERT( effect->GetFlash() == NULL );
         effect->SetFlash( particles.get() );
         CPPUNIT_ASSERT( effect->GetFlash() != NULL );

         // Test Clear
         effect->Clear();
         CPPUNIT_ASSERT( effect->GetOwner() == NULL );
         CPPUNIT_ASSERT( effect->GetDOF() == NULL );
         CPPUNIT_ASSERT( effect->GetFlash() == NULL );
         CPPUNIT_ASSERT( effect->GetSound() == NULL );

         // Test that the effect does not hold onto entities nor their DOFs
         effect->SetOwner( entity );
         effect->Attach( testDOF.get() );
         // --- Prudently check that assignment still works after clearing
         CPPUNIT_ASSERT( effect->GetOwner() != NULL );
         CPPUNIT_ASSERT( effect->GetDOF() != NULL );
         entity = NULL;
         proxy = NULL;
         testDOF = NULL;
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect should NOT hold onto entity memory",
            effect->GetOwner() == NULL );
         CPPUNIT_ASSERT_MESSAGE("WeaponEffect should NOT hold onto DOF memory",
            effect->GetDOF() == NULL );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponentTests::TestWeaponEffectManagerProperties()
      {
         // Use a dummy variable to represent the player/listener position needed
         // by weapon sound effects.
         osg::Vec3 dummyPosition;

         // Create test entities
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy1;
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy2;
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy3;
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy4;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy1 );
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy2 );
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy4 );
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy3 );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an BaseEntityActorProxy", proxy1.valid() );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an BaseEntityActorProxy", proxy2.valid() );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an BaseEntityActorProxy", proxy3.valid() );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an BaseEntityActorProxy", proxy4.valid() );
         SimCore::Actors::BaseEntity* entity1 = static_cast<SimCore::Actors::BaseEntity*>(proxy1->GetActor());
         SimCore::Actors::BaseEntity* entity2 = static_cast<SimCore::Actors::BaseEntity*>(proxy2->GetActor());
         SimCore::Actors::BaseEntity* entity3 = static_cast<SimCore::Actors::BaseEntity*>(proxy3->GetActor());
         SimCore::Actors::BaseEntity* entity4 = static_cast<SimCore::Actors::BaseEntity*>(proxy4->GetActor());
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an Entity", entity1 != NULL );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an Entity", entity2 != NULL );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an Entity", entity3 != NULL );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able to create an Entity", entity4 != NULL );

         // Create test DOFs
         osg::ref_ptr<osgSim::DOFTransform> dof1 = new osgSim::DOFTransform;
         osg::ref_ptr<osgSim::DOFTransform> dof2 = new osgSim::DOFTransform;

         // Create test effects and effects manager
         dtCore::RefPtr<SimCore::Components::WeaponEffectsManager> effectMgr = new SimCore::Components::WeaponEffectsManager;

         // Test some simple properties on the effects manager
         CPPUNIT_ASSERT_MESSAGE("WeaponEffectsManager should NOT have a recycle time defaulted to 0 or less",
            effectMgr->GetRecycleTime() > 0.0f );
         effectMgr->SetRecycleTime( 0.5f );
         CPPUNIT_ASSERT( effectMgr->GetRecycleTime() == 0.5f );

         // --- Set and check the time again to be certain the first value was not the default
         float updateDelta = 1.0f;
         effectMgr->SetRecycleTime( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetRecycleTime() == updateDelta );

         CPPUNIT_ASSERT_MESSAGE("WeaponEffectsManager should NOT have a max effect time of 0 or less",
            effectMgr->GetEffectTimeMax() > 0.0f );
         effectMgr->SetEffectTimeMax( 0.0f );
         CPPUNIT_ASSERT( effectMgr->GetEffectTimeMax() == 0.0f );
         effectMgr->SetEffectTimeMax( 2.0f );
         CPPUNIT_ASSERT( effectMgr->GetEffectTimeMax() == 2.0f );

         // NOTE: WeaponEffectsManager recycles before updating so that effects
         //       have a chance to exist; recycling afterward may prevent some
         //       short-lived effects from displaying. Effects that have lived
         //       passed the max effect time will be recycled on a subsequent
         //       call to Update.



         // Create a effects info actor
         dtCore::RefPtr<SimCore::Actors::MunitionEffectsInfoActorProxy> effectInfoProxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_EFFECTS_INFO_ACTOR_TYPE, effectInfoProxy );
         CPPUNIT_ASSERT_MESSAGE("GameManager should be able create a MunitionEffectInfoActorProxy",
            effectInfoProxy.valid() );
         SimCore::Actors::MunitionEffectsInfoActor* effectsInfo
            = static_cast<SimCore::Actors::MunitionEffectsInfoActor*>(effectInfoProxy->GetActor());
         CPPUNIT_ASSERT( effectsInfo != NULL );
         effectsInfo->SetFireFlashTime( 0.5f );
         effectsInfo->SetTracerLight( SimCore::Components::TracerEffect::DEFAULT_TRACER_LIGHT.Get() );
         effectsInfo->SetTracerShaderName( SimCore::Components::TracerEffect::DEFAULT_TRACER_SHADER.Get() );
         effectsInfo->SetTracerShaderGroup( SimCore::Components::TracerEffect::DEFAULT_TRACER_SHADER_GROUP.Get() );

         // Test ApplyWeaponEffect
         CPPUNIT_ASSERT_MESSAGE("WeaponEffectsManager should have 0 effects by default",
            effectMgr->GetWeaponEffectCount() == 0 );

         effectMgr->ApplyWeaponEffect( *entity1, dof1.get(), *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 1 );
         effectMgr->ApplyWeaponEffect( *entity1, dof1.get(), *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 1 );

         effectMgr->ApplyWeaponEffect( *entity2, dof2.get(), *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 2 );
         effectMgr->ApplyWeaponEffect( *entity2, dof2.get(), *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 2 );

         effectMgr->ApplyWeaponEffect( *entity3, NULL, *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );
         effectMgr->ApplyWeaponEffect( *entity3, NULL, *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );

         effectMgr->ApplyWeaponEffect( *entity4, NULL, *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 4 );
         effectMgr->ApplyWeaponEffect( *entity4, NULL, *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 4 );

         // Test Recycle does not remove fresh effects that have not been updated
         CPPUNIT_ASSERT( effectMgr->Recycle() == 0 );

         // Test Updates
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 4 );

         // --- Test Recycle once more to be sure it does not remove effects prematurely.
         CPPUNIT_ASSERT( effectMgr->RecycleWeaponEffects() == 0 );

         // --- Remove an entity and ensure that Recycle pulls out all effects
         //     that have referenced it.
         entity4 = NULL;
         proxy4 = NULL;
         // --- Call Recycle directly to be sure it works.
         CPPUNIT_ASSERT( effectMgr->Recycle() == 1 );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );

         // --- Step one more second. The remaining effects should be 2 seconds old.
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );
         // --- Update effects 2 & 3 to restart their flash timers
         effectMgr->ApplyWeaponEffect( *entity2, NULL, *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );
         effectMgr->ApplyWeaponEffect( *entity3, NULL, *effectsInfo, dummyPosition );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );

         // --- Step one more second. The effect 1 should be 3 seconds old.
         //     Recycle will have been called on the effect object.
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 2 );

         // --- Test the max effect limit
         CPPUNIT_ASSERT_MESSAGE("WeaponEffectsManager should have an effects limit of -1 by default (means no limit)",
            effectMgr->GetMaxWeaponEffects() < 0 );
         effectMgr->SetMaxWeaponEffects( 2 );
         CPPUNIT_ASSERT( effectMgr->GetMaxWeaponEffects() == 2 );

         // --- Try adding more effects by adding one to a entity that has lost its effect.
         CPPUNIT_ASSERT( ! effectMgr->ApplyWeaponEffect( *entity1, NULL, *effectsInfo, dummyPosition ) );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 2 );
         // --- Remove the limit and add a new effect to prove that the limit
         //     had prevented adding more effects
         effectMgr->SetMaxWeaponEffects( -1 );
         CPPUNIT_ASSERT( effectMgr->ApplyWeaponEffect( *entity1, NULL, *effectsInfo, dummyPosition ) );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );

         // --- Update again. Effects 2 & 3 should be 2 seconds old and effect 1
         //     1 second old.
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );

         // --- Update once more.
         //     Effects 2 & 3 should be 3 seconds old and thus be recycled.
         //     Effect 1 should be 2 seconds old.
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 1 );

         // --- Update all entities with effects so that Clear can be tested.
         CPPUNIT_ASSERT( effectMgr->ApplyWeaponEffect( *entity1, NULL, *effectsInfo, dummyPosition ) );
         CPPUNIT_ASSERT( effectMgr->ApplyWeaponEffect( *entity2, NULL, *effectsInfo, dummyPosition ) );
         CPPUNIT_ASSERT( effectMgr->ApplyWeaponEffect( *entity3, NULL, *effectsInfo, dummyPosition ) );
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 3 );

         // Test clear
         effectMgr->ClearWeaponEffects();
         CPPUNIT_ASSERT( effectMgr->GetWeaponEffectCount() == 0 );



         // Test Tracer Effects
         dtCore::Scene* scene = &mGM->GetScene();
         const unsigned oldSceneCount = scene->GetNumberOfAddedDrawable();
         const int maxTracerEffects = 3;

         CPPUNIT_ASSERT( effectMgr->GetGameManager() == NULL );
         effectMgr->SetGameManager( mGM.get() );
         CPPUNIT_ASSERT( effectMgr->GetGameManager() == mGM.get() );

         CPPUNIT_ASSERT( effectMgr->GetMaxMunitionEffects() < 0 ); // Negative numbers mean no limit
         effectMgr->SetMaxMunitionEffects( maxTracerEffects );
         CPPUNIT_ASSERT( effectMgr->GetMaxMunitionEffects() == maxTracerEffects );

         // --- Create a Munition Effects Request to satisfy the parameters of the
         //     calls to method ApplyMunitionEffect.
         dtCore::RefPtr<SimCore::Components::MunitionEffectRequest> effectRequest
            = new SimCore::Components::MunitionEffectRequest(1, 1.0f, *effectsInfo);

         // --- Add effects with a limit in place
         osg::Vec3 pos(0.0,0.0,0.0), initialVelocity(5.0,0.0,0.0);
         effectsInfo->SetTracerLifeTime( 3.0f );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 0 );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == unsigned(maxTracerEffects) );
         CPPUNIT_ASSERT( ! effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( ! effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( ! effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == unsigned(maxTracerEffects) );
         CPPUNIT_ASSERT( scene->GetNumberOfAddedDrawable() == oldSceneCount + maxTracerEffects );

         // --- Add effects without a limit
         effectMgr->SetMaxMunitionEffects( -1 );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == maxTracerEffects + 3 );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == maxTracerEffects + 6 );
         CPPUNIT_ASSERT( scene->GetNumberOfAddedDrawable() == oldSceneCount + maxTracerEffects + 6 );

         // --- Add effects with a limit in place again
         effectMgr->SetMaxMunitionEffects( maxTracerEffects );
         CPPUNIT_ASSERT( ! effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( ! effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( ! effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == maxTracerEffects + 6 );
         CPPUNIT_ASSERT( scene->GetNumberOfAddedDrawable() == oldSceneCount + maxTracerEffects + 6 );

         // --- Update a few time to make sure that the number of effects are trimmed down
         //     NOTE: This also tests RecycleTracerEffects().
         unsigned activeCount = maxTracerEffects + 6;
         updateDelta = 1.0f;
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == activeCount );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == activeCount );
         effectMgr->Update( updateDelta );
         effectMgr->Update( updateDelta );
         unsigned curActiveCount = effectMgr->GetMunitionEffectActiveCount();
         CPPUNIT_ASSERT( curActiveCount == activeCount );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == activeCount );
         effectMgr->Update( updateDelta ); // age the effects to be in active
         effectMgr->Update( updateDelta ); // calls recycle this iteration
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 0 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == unsigned(maxTracerEffects) ); // overage should have been recycled
         CPPUNIT_ASSERT( scene->GetNumberOfAddedDrawable() == oldSceneCount + maxTracerEffects );

         effectMgr->ClearMunitionEffects();
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 0 );
         CPPUNIT_ASSERT( scene->GetNumberOfAddedDrawable() == oldSceneCount );

         // --- Re-add some tracer effects and test the effects manager's update function
         effectsInfo->SetTracerLifeTime( 4.0f );
         effectMgr->SetMaxMunitionEffects( -1 ); // no limit to test recycling

         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 1 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 1 );

         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 2 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 2 );

         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 3 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 3 );

         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 2 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 3 );

         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 2 ); // one re-activates while another deactivates
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 3 );

         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 1 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 3 );

         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 1 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 3 );

         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 0 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 3 );

         CPPUNIT_ASSERT( effectMgr->ApplyMunitionEffect( pos, initialVelocity, *effectsInfo, *effectRequest ) );
         effectMgr->Update( updateDelta );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectActiveCount() == 1 );
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 3 );

         // Ensure no more tracers have been added to the scene.
         // NOTE: Tracers remain in the scene even though they are inactive.
         //       They are merely invisible and are not updated when inactive.
         CPPUNIT_ASSERT( scene->GetNumberOfAddedDrawable() == oldSceneCount + 3 );

         // Test that Clear calls ClearTracerEffects.
         effectMgr->Clear();
         CPPUNIT_ASSERT( effectMgr->GetMunitionEffectCount() == 0 );
         CPPUNIT_ASSERT( scene->GetNumberOfAddedDrawable() == oldSceneCount );
      }
   }
}
