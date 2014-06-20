/* -*-c++-*-
* Simulation Core - WeaponActorTests (.h & .cpp) - Using 'The MIT License'
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

#include <dtUtil/macros.h>

#include <dtCore/system.h>
#include <dtCore/refptr.h>
#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>

#include <dtCore/actorproperty.h>
#include <dtCore/project.h>
#include <dtCore/resourcedescriptor.h>

#include <dtGame/basemessages.h>
#include <dtGame/gmcomponent.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>

#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include "UnitTestMain.h"

#include <SimCore/Actors/MunitionParticlesActor.h>

namespace SimCore
{
   namespace Components
   {
      class ListeningComponent;

      //////////////////////////////////////////////////////////////////////////
      // Tests Object
      //////////////////////////////////////////////////////////////////////////

      class WeaponActorTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(WeaponActorTests);

         CPPUNIT_TEST(TestWeaponProperties);
         CPPUNIT_TEST(TestRemoveFromWorld);
         CPPUNIT_TEST(TestMessageProcessing);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            // Test Functions --------------------------------------------------
            void TestWeaponProperties();
            void TestRemoveFromWorld();
            void TestMessageProcessing();

            // Utility Functions -----------------------------------------------

            // This function will make the weapon believe it hit a target by
            // having its ReceiveContactReportCalled.
            // This handles the PhysX related code, hiding it from the unit
            // test implementations.
            // @param target The actor that was hit
            // @param trajectory The velocity vector of the impacting munition
            // @param location The location in the world that the target was hit
            void SimulateTargetHit( dtGame::GameActor* target,
               const osg::Vec3* trajectory = NULL,
               const osg::Vec3* location = NULL );

            // Creates and assigns a shooter to the tested weapon.
            // This function hides the PhysX code from the unit test code.
            void CreateShooter();

            size_t GetActorCount(dtCore::ActorType& actorType);

         protected:
         private:

            dtCore::RefPtr<dtGame::GameManager> mGM;
            dtCore::RefPtr<dtGame::MachineInfo> mMachineInfo;
            dtCore::RefPtr<SimCore::Actors::WeaponActor> mWeapon;
            dtCore::RefPtr<SimCore::Actors::WeaponActorProxy> mWeaponProxy;
            dtCore::RefPtr<ListeningComponent> mTestComp; // used in listening for weapon messages
            dtCore::RefPtr<MunitionsComponent> mMunitionsComp;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(WeaponActorTests);



      //////////////////////////////////////////////////////////////////////////
      // Test Component (for testing messages that were sent)
      //////////////////////////////////////////////////////////////////////////
      class ListeningComponent : public dtGame::GMComponent
      {
         public:
            ListeningComponent();

            virtual void ProcessMessage( const dtGame::Message& msg );

            unsigned GetShotCount() const { return mShotCount; }
            unsigned GetShotMessageCount() const { return mShotMessageCount; }

            unsigned GetDetonationMessageCount() const { return mDetMessageCount; }

            void ResetShotCount() { mShotCount = 0; mShotMessageCount = 0; }
            void ResetDetonationCount() { mDetMessageCount = 0; }

         protected:
            ~ListeningComponent();

         private:
            unsigned mShotCount;
            unsigned mShotMessageCount;
            unsigned mDetMessageCount;
      };

      //////////////////////////////////////////////////////////////////////////
      ListeningComponent::ListeningComponent()
         : dtGame::GMComponent( "TestListeningComponent" ),
         mShotCount(0),
         mShotMessageCount(0),
         mDetMessageCount(0)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      ListeningComponent::~ListeningComponent()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void ListeningComponent::ProcessMessage( const dtGame::Message& msg )
      {
         const dtGame::MessageType& type = msg.GetMessageType();

         if( type == SimCore::MessageType::SHOT_FIRED )
         {
            ++mShotMessageCount;

            // Collect the bullet count
            const SimCore::ShotFiredMessage& shotMessage
               = static_cast<const SimCore::ShotFiredMessage&>(msg);

            mShotCount += shotMessage.GetQuantityFired();
         }
         else if( type == SimCore::MessageType::DETONATION )
         {
            ++mDetMessageCount;
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // Tests code
      //////////////////////////////////////////////////////////////////////////
      void WeaponActorTests::setUp()
      {
         try
         {
            dtCore::System::GetInstance().Start();
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
            mGM->SetApplication(GetGlobalApplication());

            mMachineInfo = new dtGame::MachineInfo;

            // Create a component that listens for weapon messages
            mTestComp = new ListeningComponent;
            mGM->AddComponent( *mTestComp, dtGame::GameManager::ComponentPriority::NORMAL );

            // Create the Munitions Component that will be used to fish out a munition type.
            mMunitionsComp = new MunitionsComponent;
            mGM->AddComponent( *mMunitionsComp, dtGame::GameManager::ComponentPriority::NORMAL );

            mGM->ChangeMap("UnitTestMunitionTypesMap");

            //step a few times to ensure the map loaded
            dtCore::System::GetInstance().Step();
            dtCore::System::GetInstance().Step();
            dtCore::System::GetInstance().Step();

            // Create the weapon actor
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::WEAPON_ACTOR_TYPE, mWeaponProxy);
            mWeapon = dynamic_cast<SimCore::Actors::WeaponActor*>(&mWeaponProxy->GetGameActor());

         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorTests::tearDown()
      {
         dtCore::System::GetInstance().Stop();

         mWeapon = NULL;
         mWeaponProxy = NULL;

         if( mTestComp.valid() )
         {
            mGM->RemoveComponent( *mTestComp );
            mTestComp = NULL;
         }

         if( mMunitionsComp.valid() )
         {
            mGM->RemoveComponent( *mMunitionsComp );
            mMunitionsComp = NULL;
         }

         mGM->CloseCurrentMap();            
         dtCore::System::GetInstance().Step();
         dtCore::System::GetInstance().Step();
         dtCore::System::GetInstance().Step();

         mGM->DeleteAllActors(true);

         mGM = NULL;
         mMachineInfo = NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorTests::SimulateTargetHit( dtGame::GameActor* target,
         const osg::Vec3* trajectory,
         const osg::Vec3* location )
      {
#ifdef AGEIA_PHYSICS
         // Ready a contact report
         dtAgeiaPhysX::ContactReport report;

         // Set both vectors with the same value since both seem to be dealing
         // with the same type of data; normal may just be a normalized version
         // of the force. The weapon may use either of the two for the same data.
         if( trajectory != NULL )
         {
            report.nxVec3crContactNormal[0] = (*trajectory)[0];
            report.nxVec3crContactNormal[1] = (*trajectory)[1];
            report.nxVec3crContactNormal[2] = (*trajectory)[2];
            report.nxVec3crContactNormalForce = report.nxVec3crContactNormal;
         }

         // Set the location
         if( location != NULL )
         {
            report.lsContactPoints.push_back(
               NxVec3( (*location)[0], (*location)[1], (*location)[2] )
               );
         }

         // Notify the weapon
         mWeapon->ReceiveContactReport( report, target != NULL ? &target->GetGameActorProxy() : NULL );
#else
         dtPhysics::CollisionContact report;
         // Set both vectors with the same value since both seem to be dealing
         // with the same type of data; normal may just be a normalized version
         // of the force. The weapon may use either of the two for the same data.
         if( trajectory != NULL )
         {
            report.mNormal = *trajectory;
         }

         // Set the location
         if( location != NULL )
         {
            report.mPosition = *location;
         }
         mWeapon->ReceiveContactReport(report, target != NULL ? &target->GetGameActorProxy() : NULL);
#endif
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorTests::CreateShooter()
      {
         dtCore::RefPtr<SimCore::Actors::MunitionParticlesActorProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PHYSICS_MUNITIONS_PARTICLE_SYSTEM_TYPE, proxy );
         mWeapon->SetShooter( proxy.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      size_t WeaponActorTests::GetActorCount(dtCore::ActorType& actorType)
      {
         std::vector<dtCore::ActorProxy*> proxyArray;
         mGM->FindActorsByType(actorType, proxyArray);
         return proxyArray.size();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorTests::TestWeaponProperties()
      {
         // Add the weapon to the world so that sleep states can be tracked in
         // later property tests.
         mGM->AddActor( *mWeaponProxy, false, false );
         CreateShooter();

         // Create test objects for actor proxies.
         // --- Munition Type Actor
         std::string munitionName("TestMunitionType");

         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> munitionTypeProxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, munitionTypeProxy );
         CPPUNIT_ASSERT_MESSAGE( "GameManager should be able to create a MunitionTypeActor",
            munitionTypeProxy.valid() );
         SimCore::Actors::MunitionTypeActor* munitionType
            = static_cast<SimCore::Actors::MunitionTypeActor*>(munitionTypeProxy->GetDrawable());
         munitionType->SetName( munitionName );

         // --- Entity as an owner
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy );
         CPPUNIT_ASSERT_MESSAGE( "GameManager should be able to create an Entity",
            proxy.valid() );

         // Declare test variables
         const float value = 123.456f; // use a value that will most likely not be a default value

         // Test String & Actor Properties ---------------------------------------------
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor munition type name should be empty by default",
            mWeapon->GetMunitionTypeName().empty() );
         mWeapon->SetMunitionTypeName( "Test" );
         CPPUNIT_ASSERT( mWeapon->GetMunitionTypeName() == "Test" );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor munition type should be NULL by default",
            mWeapon->GetMunitionType() == NULL );
         mWeapon->SetMunitionType( munitionType );
         CPPUNIT_ASSERT( mWeapon->GetMunitionType() == munitionType );
         CPPUNIT_ASSERT( mWeapon->GetMunitionTypeName() == munitionName );
         mWeapon->SetMunitionType( NULL );
         CPPUNIT_ASSERT( mWeapon->GetMunitionType() == NULL );

         // --- A munition type will be needed later for a successful fired shot
         mWeapon->SetMunitionTypeProxy( munitionTypeProxy.get() );
         CPPUNIT_ASSERT( mWeapon->GetMunitionTypeName() == munitionName );
         CPPUNIT_ASSERT(mWeapon->GetMunitionType() == munitionTypeProxy->GetDrawable());

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor owner should be NULL by default",
            mWeapon->GetOwner() == NULL );
         mWeapon->SetOwner( proxy.get() );
         CPPUNIT_ASSERT( mWeapon->GetOwner() != NULL );

         // --- Ensure that the weapon does not hold onto entity memory if the
         //     entity goes out of scope.
         mWeapon->SetOwner( NULL );
         proxy = NULL;
         dtCore::BaseActorObject* owner = mWeapon->GetOwner();
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should NOT continue to hold onto its owner's memory",
            owner == NULL );



         // Test Boolean Properties --------------------------------------------
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should be registered to TickLocal by default",
            ! mWeapon->IsSleeping() );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor trigger held state should default to FALSE",
            ! mWeapon->IsTriggerHeld() );
         mWeapon->SetFireRate( 1.0 ); // disable single-shot mode
         mWeapon->SetTriggerHeld( true );
         CPPUNIT_ASSERT( mWeapon->IsTriggerHeld() );
         mWeapon->SetFireRate( 0.0 ); // enable single-shot mode
         mWeapon->SetTriggerHeld( true ); // causes an instant fire that set trigger held state to false
         //CPPUNIT_ASSERT( ! mWeapon->IsTriggerHeld() ); --commented this out after code changed on SetTriggerHeld() causing this to break

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor jammed state should default to FALSE",
            ! mWeapon->IsJammed() );
         mWeapon->SetJammed( true );
         CPPUNIT_ASSERT( mWeapon->IsJammed() );



         // Test Integer Properties --------------------------------------------
         int intValue = (int) value; // use a value that will most likely not be a default value
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor tracer frequency should default to 0",
            mWeapon->GetTracerFrequency() == 0 );
         mWeapon->SetTracerFrequency( intValue );
         CPPUNIT_ASSERT( mWeapon->GetTracerFrequency() == intValue );

         // Ammo defaults 100000 just in case a weapon gets loaded from
         // a map with a high number for ammo.
         // Since SetAmmouCount clamps the value that is set, defaulting the max
         // to a high number should cover most situations where ammo count is a
         // high number A.ny ammo count higher than the default MAY be clamped
         // when loaded from the map; this is dependent on the order in which
         // the weapon's properties are declared in the map.
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor ammo max should default to 100000",
            mWeapon->GetAmmoMax() == 100000 );
         mWeapon->SetAmmoMax( intValue );
         CPPUNIT_ASSERT( mWeapon->GetAmmoMax() == intValue );

         // NOTE: Ammo count depends on the ammo max so that it can be clamped
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor ammo count should default to 0",
            mWeapon->GetAmmoCount() == 0 );
         mWeapon->SetAmmoCount( 10 );
         CPPUNIT_ASSERT( mWeapon->GetAmmoCount() == 10 );
         mWeapon->SetAmmoCount( mWeapon->GetAmmoMax() + 100 );
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor ammo count should clamp to the ammo max if set beyond the ammo max",
            mWeapon->GetAmmoCount() == mWeapon->GetAmmoMax() );
         mWeapon->SetAmmoCount( -10 );



         // Test Float Properties ----------------------------------------------
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor recoil distance should default to 0",
            mWeapon->GetRecoilDistance() == 0.0f );
         mWeapon->SetRecoilDistance( value );
         CPPUNIT_ASSERT( mWeapon->GetRecoilDistance() == value );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor recoil rest time should default to 0",
            mWeapon->GetRecoilRestTime() == 0.0f );
         mWeapon->SetRecoilRestTime( value );
         CPPUNIT_ASSERT( mWeapon->GetRecoilRestTime() == value );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor auto sleep time should default to 0",
            mWeapon->GetAutoSleepTime() == 0.0f );
         mWeapon->SetAutoSleepTime( value );
         CPPUNIT_ASSERT( mWeapon->GetAutoSleepTime() == value );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor fire rate should default to 0 (non-automatic single-shot mode)",
            mWeapon->GetFireRate() == 0.0f );
         mWeapon->SetFireRate( value );
         CPPUNIT_ASSERT( mWeapon->GetFireRate() == value );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor message cycle time should default to a value above zero",
            mWeapon->GetTimeBetweenMessages() > 0.0f );
         mWeapon->SetTimeBetweenMessages( value );
         CPPUNIT_ASSERT( mWeapon->GetTimeBetweenMessages() == value );
         mWeapon->SetTimeBetweenMessages( -value );
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor message cycle time should be clamped to 0 if set below 0",
            mWeapon->GetTimeBetweenMessages() == 0.0f );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor jam probability should default to 0",
            mWeapon->GetJamProbability() == 0.0f );
         mWeapon->SetJamProbability( value );
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should clamp jam probability to 1 if set beyond 1",
            mWeapon->GetJamProbability() == 1.0f );
         mWeapon->SetJamProbability( -value );
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should clamp jam probability to 0 if set below 0",
            mWeapon->GetJamProbability() == 0.0f );
         mWeapon->SetJamProbability( 0.5f );
         CPPUNIT_ASSERT( mWeapon->GetJamProbability() == 0.5f );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor flash probability should default to 1",
            mWeapon->GetFlashProbability() == 1.0f );
         mWeapon->SetFlashProbability( value );
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should clamp flash probability to 1 if set beyond 1",
            mWeapon->GetFlashProbability() == 1.0f );
         mWeapon->SetFlashProbability( -value );
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should clamp flash probability to 0 if set below 0",
            mWeapon->GetFlashProbability() == 0.0f );
         mWeapon->SetFlashProbability( 0.5f );
         CPPUNIT_ASSERT( mWeapon->GetFlashProbability() == 0.5f );

         CPPUNIT_ASSERT_MESSAGE( "WeaponActor default flash time should be greater than 0.0",
            mWeapon->GetFlashTime() > 0.0f );
         mWeapon->SetFlashTime( value );
         CPPUNIT_ASSERT( mWeapon->GetFlashTime() == value );
         // --- Ensure the returned value is not the default value
         float newValue = value * 0.5f;
         mWeapon->SetFlashTime( newValue );
         CPPUNIT_ASSERT( mWeapon->GetFlashTime() == newValue );
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorTests::TestRemoveFromWorld()
      {
         // Add the weapon to the world so that sleep states can be tracked in
         // later property tests.
         mGM->AddActor( *mWeaponProxy, false, false );
         CreateShooter();
         SimCore::Actors::MunitionParticlesActorProxy* shooterProxy = mWeapon->GetShooter();
         CPPUNIT_ASSERT(shooterProxy != NULL);
         mGM->AddActor(*shooterProxy, false, false);

         dtCore::ActorType& shooterType = *SimCore::Actors::EntityActorRegistry::PHYSICS_MUNITIONS_PARTICLE_SYSTEM_TYPE;

         CPPUNIT_ASSERT_EQUAL(size_t(1), GetActorCount(shooterType));

         mGM->DeleteActor(*mWeaponProxy);
         dtCore::System::GetInstance().Step();

         CPPUNIT_ASSERT_EQUAL(size_t(0), GetActorCount(shooterType));
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponActorTests::TestMessageProcessing()
      {
         // Create a test target entity
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, proxy );
         CPPUNIT_ASSERT_MESSAGE( "GameManager should be able to create a Platform",
            proxy.valid() );
         SimCore::Actors::BaseEntity* target = NULL;
         proxy->GetDrawable(target);
         CPPUNIT_ASSERT_MESSAGE( "BaseEntityActorProxy should contain a valid Entity",
            target != NULL );

         // Ensure the MunitionsComponent has munition definitions loaded
         // so that the WeaponActor can find them when entering the world.
         //mMunitionsComp->LoadMunitionTypeTable( "UnitTestMunitionTypesMap" );

         // Create a tick message used for testing WeaponActor::OnTickLocal
         dtCore::RefPtr<dtGame::TickMessage> tickMsg;
         mGM->GetMessageFactory().CreateMessage( dtGame::MessageType::TICK_LOCAL, tickMsg );

         // Declare test time step
         float timeDelta = 0.5f;
         tickMsg->SetDeltaSimTime( timeDelta );

         // Ready the weapon for the following tests
         int ammoCount = 1000;
         mWeapon->SetAmmoCount(ammoCount); // max ammo defaults to 1000
         CPPUNIT_ASSERT( mWeapon->GetAmmoCount() == ammoCount );

         // TEST MUNITION TYPE BEHAVIOR ----------------------------------------
         // --- Set name of a mapped munition and add weapon to the game manager
         //     OnEnterWorld will look for the MunitionsComponent.
         //     The type that is loaded does not matter since this is a simple
         //     test just to see that the weapon can access the Munitions Component
         //     to obtain the Munition Type by name.
         const std::string munitionName("Generic Explosive");
         mWeapon->SetMunitionTypeName( munitionName );

         // --- Let the system process any messages necessary for the component to function.
         mGM->AddActor( *mWeaponProxy, false, false );

         // --- Obtain the munition type actor
         
         mWeapon->LoadMunitionType(mWeapon->GetMunitionTypeName());
         
         const SimCore::Actors::MunitionTypeActor* munitionType = mWeapon->GetMunitionType();
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should be able to access the MunitionsComponent for a MunitionTypeActor",
            munitionType != NULL );
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should set the munition type name when a MunitionTypeActor is loaded",
            mWeapon->GetMunitionTypeName() == munitionName );

         // TEST AUTO SLEEP BEHAVIOR -------------------------------------------
         // --- Test that weapon is sleeping by default (tick it several times)
         CPPUNIT_ASSERT_MESSAGE( "WeaponActor should NOT be sleeping by default",
            ! mWeapon->IsSleeping() );

         // --- Test wake up
         CPPUNIT_ASSERT( ! mWeapon->IsTriggerHeld() );
         CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, mWeapon->GetFireRate(), 1e-6f); // single-shot mode
         mWeapon->SetTimeBetweenMessages(timeDelta); // attempt to send a message every tick
         mWeapon->SetAutoSleepTime( 2.0f );
         mWeapon->SetTriggerHeld( true ); // causes an instant shot
         mWeapon->OnTickLocal( *tickMsg );
         mWeapon->OnTickLocal( *tickMsg ); // again to make sure the weapon only fires once
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT( ! mWeapon->IsSleeping() );
         CPPUNIT_ASSERT_EQUAL(1U, mTestComp->GetShotMessageCount());
         CPPUNIT_ASSERT_EQUAL(1U, mTestComp->GetShotCount());
         --ammoCount;
         CPPUNIT_ASSERT_EQUAL(ammoCount, mWeapon->GetAmmoCount() );
         CPPUNIT_ASSERT( ! mWeapon->IsTriggerHeld() ); // single fire mode will depress the trigger
         mTestComp->ResetShotCount();

         // --- Test auto sleep (previous test ticked the weapon forward 1 second)
         mWeapon->OnTickLocal( *tickMsg );
         mWeapon->OnTickLocal( *tickMsg );
         CPPUNIT_ASSERT( mWeapon->IsSleeping() );
         // --- Test ticking during sleep mode
         mWeapon->OnTickLocal( *tickMsg );
         mWeapon->OnTickLocal( *tickMsg );
         mWeapon->OnTickLocal( *tickMsg );
         CPPUNIT_ASSERT_MESSAGE( "Ticking the weapon actor should not cause it to wake if nothing has been done to it.",
            mWeapon->IsSleeping() );

         // --- Test wake up
         mWeapon->SetTriggerHeld( true );
         mWeapon->OnTickLocal( *tickMsg ); // 0.5
         dtCore::System::GetInstance().Step(); // process all queued messages
         CPPUNIT_ASSERT( ! mWeapon->IsSleeping() );
         CPPUNIT_ASSERT_EQUAL(1U, mTestComp->GetShotMessageCount());
         CPPUNIT_ASSERT_EQUAL(1U, mTestComp->GetShotCount());
         --ammoCount;
         CPPUNIT_ASSERT_EQUAL(ammoCount, mWeapon->GetAmmoCount());
         mTestComp->ResetShotCount();

         // --- Test without auto sleep time (will not sleep)
         mWeapon->SetAutoSleepTime( 0.0f );
         mWeapon->OnTickLocal( *tickMsg ); // 1.0
         mWeapon->OnTickLocal( *tickMsg ); // 1.5
         mWeapon->OnTickLocal( *tickMsg ); // 2.0
         mWeapon->OnTickLocal( *tickMsg ); // 2.5
         dtCore::System::GetInstance().Step(); // process all queued messages
         CPPUNIT_ASSERT( ! mWeapon->IsSleeping() );

         // --- Nothing should have been shot since weapon is still in single-shot mode.
         CPPUNIT_ASSERT_EQUAL(0U, mTestComp->GetShotCount() );
         CPPUNIT_ASSERT_EQUAL(0U, mTestComp->GetShotMessageCount() );

         // TEST RE-FIRE BEHAVIOR ----------------------------------------------
         timeDelta = 0.25f;
         tickMsg->SetDeltaSimTime( timeDelta );
         mWeapon->Reset(); // reset timers

         mWeapon->SetFireRate( 0.5f );
         mWeapon->SetTimeBetweenMessages( 1.0f );
         mWeapon->SetTriggerHeld( true );
         mWeapon->OnTickLocal( *tickMsg ); // 0.25
         mWeapon->OnTickLocal( *tickMsg ); // 0.5   // shot 1
         mWeapon->OnTickLocal( *tickMsg ); // 0.75
         mWeapon->OnTickLocal( *tickMsg ); // 1.0   // shot 2
         mWeapon->OnTickLocal( *tickMsg ); // 1.25
         mWeapon->OnTickLocal( *tickMsg ); // 1.5   // shot 3
         mWeapon->OnTickLocal( *tickMsg ); // 1.75
         mWeapon->OnTickLocal( *tickMsg ); // 2.0   // shot 4
         mWeapon->OnTickLocal( *tickMsg ); // 2.25
         mWeapon->OnTickLocal( *tickMsg ); // 2.5   // shot 5
         mWeapon->OnTickLocal( *tickMsg ); // 2.75
         mWeapon->OnTickLocal( *tickMsg ); // 3.0   // shot 6
         // --- Prevent extra ticks when stepping the system forward in following
         //     tests.
         //     Extra ticks could call TickLocal more times than expected, thus
         //     affecting the test counts of messages and shots fired.
         mWeaponProxy->UnregisterForMessages(
            dtGame::MessageType::TICK_LOCAL,
            dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );
         // --- Process all queued messages so the test component can listen for them.
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT( mTestComp->GetShotCount() == 6 );
         CPPUNIT_ASSERT( mTestComp->GetShotMessageCount() == 3 );
         mTestComp->ResetShotCount();

         // Reset counters and timers for the next test
         mWeapon->Reset();

         // --- Set re-fire rate less than the time step.
         //     The weapon will only fire as fast as it can be stepped
         mWeapon->SetFireRate( 0.125f ); // twice per 0.25
         mWeapon->OnTickLocal( *tickMsg ); // 0.25
         mWeapon->OnTickLocal( *tickMsg ); // 0.5
         mWeapon->OnTickLocal( *tickMsg ); // 0.75
         mWeapon->OnTickLocal( *tickMsg ); // 1.0
         mWeapon->OnTickLocal( *tickMsg ); // 1.25
         mWeapon->OnTickLocal( *tickMsg ); // 1.5
         mWeapon->OnTickLocal( *tickMsg ); // 1.75
         mWeapon->OnTickLocal( *tickMsg ); // 2.0
         // --- Prevent extra ticks when stepping the system forward in following
         //     tests.
         //     Extra ticks could call TickLocal more times than expected, thus
         //     affecting the test counts of messages and shots fired.
         mWeaponProxy->UnregisterForMessages(
            dtGame::MessageType::TICK_LOCAL,
            dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );
         // --- Process all queued messages so the test component can listen for them.
         dtCore::System::GetInstance().Step();
         // --- The weapon can only fire as fast as it can be stepped, so
         //     the bullet count will be lower than expected.
         CPPUNIT_ASSERT_EQUAL(8U, mTestComp->GetShotCount());
         CPPUNIT_ASSERT_EQUAL(2U, mTestComp->GetShotMessageCount());
         mTestComp->ResetShotCount();
         mTestComp->ResetDetonationCount();

         // --- Prevent extra ticks when stepping the system forward in following
         //     tests.
         //     Extra ticks could call TickLocal more times than expected, thus
         //     affecting the test counts of messages and shots fired.
         mWeaponProxy->UnregisterForMessages(
            dtGame::MessageType::TICK_LOCAL,
            dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );

         // Reset counters and timers for the next test
         mWeapon->Reset();

         // TEST MESSAGE BEHAVIOR (OUT-GOING) ----------------------------------
         mWeapon->SetFireRate( 0.5f );
         mWeapon->SetUsingBulletPhysics( true ); // means detonations are sent separately from fire messages

         // --- Test switching targets faster than message cycle time
         mWeapon->OnTickLocal( *tickMsg ); // 0.25
         SimulateTargetHit( target );            // det 1
         mWeapon->OnTickLocal( *tickMsg ); // 0.5  // msg 1  // shot 1

         SimulateTargetHit( NULL );              // det 2
         mWeapon->OnTickLocal( *tickMsg ); // 0.75
         mWeapon->OnTickLocal( *tickMsg ); // 1.0  // msg 2  // shot 2

         mWeapon->OnTickLocal( *tickMsg ); // 1.25
         mWeapon->OnTickLocal( *tickMsg ); // 1.5            // shot 3

         SimulateTargetHit( target );            // det 3
         mWeapon->OnTickLocal( *tickMsg ); // 1.75
         SimulateTargetHit( target );            // det 3 (within same detonation message cycle time of 1.0 seconds acting on same target)
         mWeapon->OnTickLocal( *tickMsg ); // 2.0  // msg 3  // shot 4

         mWeapon->OnTickLocal( *tickMsg ); // 2.25
         mWeapon->OnTickLocal( *tickMsg ); // 2.5            // shot 5

         mWeapon->OnTickLocal( *tickMsg ); // 2.75
         mWeapon->OnTickLocal( *tickMsg ); // 3.0  // msg 4  // shot 6

         mWeapon->OnTickLocal( *tickMsg ); // 3.25
         SimulateTargetHit( NULL );              // det 4 (target change causes a shot fired message on next message or trigger time)
         mWeapon->OnTickLocal( *tickMsg ); // 3.5  // msg 5  // shot 7
         mWeapon->OnTickLocal( *tickMsg ); // 3.75 // just for good measure
         mWeapon->OnTickLocal( *tickMsg ); // 4.00 // just for good measure

         // --- Process all queued messages so the test component can listen for them.
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_EQUAL(7U, mTestComp->GetShotCount());
         CPPUNIT_ASSERT_EQUAL(5U, mTestComp->GetShotMessageCount());
         CPPUNIT_ASSERT_EQUAL(4U, mTestComp->GetDetonationMessageCount());


         // Use the same test from above WITHOUT bullet physics (instant hit mode).

         // TEST MESSAGE BEHAVIOR (OUT-GOING) ----------------------------------

         // --- Reset counters and timers for the next test
         SimulateTargetHit( NULL ); // Set target back to NULL
         mWeapon->Reset();
         mTestComp->ResetShotCount();
         mTestComp->ResetDetonationCount();
         mWeapon->SetUsingBulletPhysics( false ); // physics is still used, but detonations are sent the same instant as shots
         // Has same fire rate of 0.5.

         // --- Test switching targets faster than message cycle time
         mWeapon->OnTickLocal( *tickMsg ); // 0.25
         SimulateTargetHit( target );
         mWeapon->OnTickLocal( *tickMsg ); // 0.5  // msg 1  // shot 1 & detonation
         SimulateTargetHit( NULL );
         mWeapon->OnTickLocal( *tickMsg ); // 0.75
         mWeapon->OnTickLocal( *tickMsg ); // 1.0  // msg 2  // shot 2 & detonation
         mWeapon->OnTickLocal( *tickMsg ); // 1.25
         mWeapon->OnTickLocal( *tickMsg ); // 1.5            // shot 3
         SimulateTargetHit( target );
         mWeapon->OnTickLocal( *tickMsg ); // 1.75
         SimulateTargetHit( target );
         mWeapon->OnTickLocal( *tickMsg ); // 2.0  // msg 3  // shot 4 & detonation
         mWeapon->OnTickLocal( *tickMsg ); // 2.25
         mWeapon->OnTickLocal( *tickMsg ); // 2.5            // shot 5
         mWeapon->OnTickLocal( *tickMsg ); // 2.75
         mWeapon->OnTickLocal( *tickMsg ); // 3.0  // msg 4  // shot 6 & detonation
         mWeapon->OnTickLocal( *tickMsg ); // 3.25
         SimulateTargetHit( NULL );
         mWeapon->OnTickLocal( *tickMsg ); // 3.5  // msg 5  // shot 7 & detonation
         // --- Prevent extra ticks when stepping the system forward in following
         //     tests.
         //     Extra ticks could call TickLocal more times than expected, thus
         //     affecting the test counts of messages and shots fired.
         mWeaponProxy->UnregisterForMessages(
            dtGame::MessageType::TICK_LOCAL,
            dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );
         // --- Process all queued messages so the test component can listen for them.
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_EQUAL(7U, mTestComp->GetShotCount());
         CPPUNIT_ASSERT_EQUAL(5U, mTestComp->GetShotMessageCount());
         CPPUNIT_ASSERT_EQUAL(5U, mTestComp->GetDetonationMessageCount());
      }
   }
}
