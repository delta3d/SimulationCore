/* -*-c++-*-
* allTests - This source file (.h & .cpp) - Using 'The MIT License'
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
* Chris Rodgers
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <SimCore/Actors/SoundActorProxy.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>

#include <dtGame/gamemanager.h>

#include <dtCore/enginepropertytypes.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>

#include <dtUtil/datapathutils.h>

#include <dtABC/application.h>

#include <UnitTestMain.h>

///////////////////////////////////////////////////////////////////////////////
// TEST OBJECT
///////////////////////////////////////////////////////////////////////////////
class SoundActorTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(SoundActorTests);
      CPPUNIT_TEST(TestProperties);
      CPPUNIT_TEST(TestTimedPlay);
   CPPUNIT_TEST_SUITE_END();


   public:
      void setUp();
      void tearDown();

      // Helper Methods
      void TickSystem(unsigned int milliseconds);

      // Test Methods
      void TestProperties();
      void TestTimedPlay();

   private:
      static const std::string LIBRARY_TEST_GAME_ACTOR;

      dtCore::RefPtr<dtGame::GameManager> mGameManager;
      dtCore::RefPtr<SimCore::Actors::SoundActorProxy> mProxy;
};

//Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(SoundActorTests);

///////////////////////////////////////////////////////////////////////////////
const std::string SoundActorTests::LIBRARY_TEST_GAME_ACTOR("testGameActorLibrary");

///////////////////////////////////////////////////////////////////////////////
void SoundActorTests::setUp()
{
   try
   {
      dtCore::Scene* scene = new dtCore::Scene();
      mGameManager = new dtGame::GameManager(*scene);
      mGameManager->SetApplication(GetGlobalApplication());
      mGameManager->LoadActorRegistry(LIBRARY_TEST_GAME_ACTOR);

      dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
      dtCore::System::GetInstance().Start();

      dtCore::RefPtr<const dtCore::ActorType> actorType =
         mGameManager->FindActorType("SimCore","SoundActor");
      CPPUNIT_ASSERT_MESSAGE("Could not find actor type.",actorType.valid());

      mGameManager->CreateActor(*actorType, mProxy);
      CPPUNIT_ASSERT_MESSAGE("Could not create sound actor proxy.",mProxy.valid());
   }
   catch (const dtUtil::Exception& e)
   {
      CPPUNIT_FAIL(e.ToString());
   }
}

///////////////////////////////////////////////////////////////////////////////
void SoundActorTests::tearDown()
{
   mProxy = NULL;

   dtCore::System::GetInstance().SetPause(false);
   dtCore::System::GetInstance().Stop();

   if (mGameManager.valid())
   {
      mGameManager->DeleteAllActors();
      mGameManager->UnloadActorRegistry(LIBRARY_TEST_GAME_ACTOR);
      mGameManager = NULL;
   }

}

///////////////////////////////////////////////////////////////////////////////
void SoundActorTests::TickSystem(unsigned int milliseconds)
{
   dtCore::AppSleep(milliseconds);
   dtCore::System::GetInstance().Step();
}

///////////////////////////////////////////////////////////////////////////////
void SoundActorTests::TestProperties()
{
   try
   {
      SimCore::Actors::SoundActorProxy* proxy = mProxy.get();

      // Get the tested properties.
      // Random Sound Effect
      // Offset Time
      const dtCore::BooleanActorProperty* propRandom = static_cast<const dtCore::BooleanActorProperty*>
         (proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_PLAY_AS_RANDOM ));
      const dtCore::FloatActorProperty* propOffsetTime = static_cast<const dtCore::FloatActorProperty*>
         (proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_INITIAL_OFFSET_TIME ));
      const dtCore::FloatActorProperty* propRandTimeMax = static_cast<const dtCore::FloatActorProperty*>
         (proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_MAX_RANDOM_TIME ));
      const dtCore::FloatActorProperty* propRandTimeMin = static_cast<const dtCore::FloatActorProperty*>
         (proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_MIN_RANDOM_TIME ));

      // Make sure the correct properties exist on the proxy.
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a random flag property.",
         propRandom != NULL);
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a initial-play-time-offset property.",
         propOffsetTime != NULL);
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a random-maximum-time property.",
         propRandTimeMax != NULL);
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a random-minimum-time property.",
         propRandTimeMin != NULL);
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a direction property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_DIRECTION ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a gain property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_GAIN ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a listener-relative flag property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_LISTENER_RELATIVE ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a looping flag property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_LOOPING ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a max distance property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_MAX_DISTANCE ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a max gain property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_MAX_GAIN ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a min gain property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_MIN_GAIN ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a pith property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_PITCH ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a rolloff factor property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_ROLLOFF_FACTOR ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a sound-file property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_SOUND_EFFECT ) != NULL );
      CPPUNIT_ASSERT_MESSAGE("Sound actor should have a velocity property.",
         proxy->GetProperty( SimCore::Actors::SoundActorProxy::PROPERTY_VELOCITY ) != NULL );

      // Check the default values.
      CPPUNIT_ASSERT_MESSAGE("Sound should not be random by default",
         ! propRandom->GetValue() );
      CPPUNIT_ASSERT_MESSAGE("Sound should a time offset of 0 by default",
         propOffsetTime->GetValue() == 0.0f );
      CPPUNIT_ASSERT_MESSAGE("Sound max random time is 30 by default",
         propRandTimeMax->GetValue() == SimCore::Actors::SoundActorProxy::DEFAULT_RANDOM_TIME_MAX );
      CPPUNIT_ASSERT_MESSAGE("Sound min random time is 5 by default",
         propRandTimeMin->GetValue() == SimCore::Actors::SoundActorProxy::DEFAULT_RANDOM_TIME_MIN );

      float testFloat = 0.73f;
      proxy->SetGain(testFloat);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(testFloat, proxy->GetGain(), 0.01f);

      //Change the float to make sure all the properties differ.
      testFloat += 0.1f;
      proxy->SetMaxGain(testFloat);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(testFloat, proxy->GetMaxGain(), 0.01f);

      //Change the float to make sure all the properties differ.
      testFloat -= 0.4f;
      proxy->SetMinGain(testFloat);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(testFloat, proxy->GetMinGain(), 0.01f);

      testFloat = 0.9f;
      proxy->SetPitch(testFloat);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(testFloat, proxy->GetPitch(), 0.01f);

      testFloat = 1.7f;
      proxy->SetMaxDistance(testFloat);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(testFloat, proxy->GetMaxDistance(), 0.01f);

      testFloat = 0.88f;
      proxy->SetRolloffFactor(testFloat);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(testFloat, proxy->GetRolloffFactor(), 0.01f);

      proxy->SetLooping(false);
      CPPUNIT_ASSERT_EQUAL(false, proxy->IsLooping());

      proxy->SetListenerRelative(true);
      CPPUNIT_ASSERT_EQUAL(true, proxy->IsListenerRelative());

      CPPUNIT_ASSERT_THROW_MESSAGE("Play should throw an exception since no resource was set.", proxy->Play(), dtUtil::Exception);

      //dtCore::ResourceDescriptor rd("Sounds:silence.wav");
      proxy->SetSoundResource("Sounds/silence.wav");

      proxy->SetDirection(osg::Vec3(0.1, 0.3, 0.9));
      proxy->SetVelocity(osg::Vec3(1.2, 3.3, -1.6));

      proxy->Play();

      osg::Vec3 vec;
      proxy->GetSound()->GetDirection(vec);
      CPPUNIT_ASSERT_EQUAL(vec, proxy->GetDirection());
      proxy->GetSound()->GetVelocity(vec);
      CPPUNIT_ASSERT_EQUAL(vec, proxy->GetVelocity());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(proxy->GetSound()->GetGain(), proxy->GetGain(), 0.01f);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(proxy->GetSound()->GetMaxGain(), proxy->GetMaxGain(), 0.01f);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(proxy->GetSound()->GetMinGain(), proxy->GetMinGain(), 0.01f);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(proxy->GetSound()->GetPitch(), proxy->GetPitch(), 0.01f);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(proxy->GetSound()->GetMaxDistance(), proxy->GetMaxDistance(), 0.01f);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(proxy->GetSound()->GetRolloffFactor(), proxy->GetRolloffFactor(), 0.01f);
      CPPUNIT_ASSERT_EQUAL(proxy->GetSound()->IsLooping(), proxy->IsLooping());
      CPPUNIT_ASSERT_EQUAL(proxy->GetSound()->IsListenerRelative(), proxy->IsListenerRelative());

   }
   catch (const dtUtil::Exception& e)
   {
      CPPUNIT_FAIL(e.ToString());
   }
}

///////////////////////////////////////////////////////////////////////////////
void SoundActorTests::TestTimedPlay()
{
   try
   {
      // Test loading a sound.
      //dtCore::ResourceDescriptor rd("Sounds:silence.wav");
      mProxy->SetSoundResource("Sounds/silence.wav");
      SimCore::Actors::SoundActor* soundActor = NULL;
      mProxy->GetActor(soundActor);
      dtAudio::Sound* sound = soundActor->GetSound();

      // --- Ensure the proxy returns the same sound object.
      CPPUNIT_ASSERT_MESSAGE("The sound should be null",
         sound == NULL);

      soundActor->CreateSound();
      // --- Ensure the proxy returns the same sound object.
      CPPUNIT_ASSERT_MESSAGE("The sound should Not be null after a create call",
         soundActor->GetSound() != NULL);
      soundActor->DestroySound();

      CPPUNIT_ASSERT_MESSAGE("The sound should be null after a destroy call",
         soundActor->GetSound() == NULL);

      // --- Ensure he sound does not play when simply loaded.
      dtCore::System::GetInstance().Step(); // Sends sound commands to Audio Manager.

      // Test adding to the game manager.
      const float offsetTime = 0.1f;

      //Clear all the randomness.
      mProxy->SetMinRandomTime(0.0);
      mProxy->SetMaxRandomTime(0.0);
      mProxy->SetOffsetTime(offsetTime);
      mProxy->SetLooping(true);
      mGameManager->AddActor( *mProxy, false, false );

      double simTime = dtCore::System::GetInstance().GetSimulationTime();
      dtCore::System::GetInstance().Step();
      double newSimTime = dtCore::System::GetInstance().GetSimulationTime();
      if ((newSimTime - simTime) < offsetTime)
      {
         CPPUNIT_ASSERT_MESSAGE("Sound should not be playing yet.",
                  soundActor->GetSound() == NULL);
      }
      else
      {
         CPPUNIT_ASSERT_MESSAGE("Sound should be playing now.",
                  soundActor->GetSound() != NULL && soundActor->GetSound()->IsPlaying());
      }
 
      // --- Stop the sound
      mProxy->Stop();
      dtCore::System::GetInstance().Step(); // Sends sound commands to Audio Manager.
      CPPUNIT_ASSERT_MESSAGE( "Sound should now deleted",
         soundActor->GetSound() == NULL);

      // --- Start the sound to test stopping on removal from the system.
      mProxy->Play();
      dtCore::System::GetInstance().Step(); // Sends sound commands to Audio Manager.
      CPPUNIT_ASSERT_MESSAGE( "Sound should be playing again.",
               soundActor->GetSound() != NULL && soundActor->GetSound()->IsPlaying());

      // Test removing from the game manager.
      mGameManager->DeleteActor( *mProxy );
      dtCore::System::GetInstance().Step(); // Removes actor from world and sends commands to Audio Manager.
      // such as stop.
      CPPUNIT_ASSERT_MESSAGE( "Sound should have been stopped when the actor was removed from the world",
               soundActor->GetSound() == NULL );

      // Test adding with a time offset of 0.
      mProxy->SetOffsetTime( 0.0f );
      mGameManager->AddActor( *mProxy, false, false );
      dtCore::System::GetInstance().Step();
      CPPUNIT_ASSERT_MESSAGE( "Sound be playing.",
               soundActor->GetSound() != NULL && soundActor->GetSound()->IsPlaying());

      // --- Remove again
      mGameManager->DeleteActor( *mProxy );
      dtCore::System::GetInstance().Step();
      CPPUNIT_ASSERT_MESSAGE( "Sound should have been stopped when the actor was removed from the world again.",
               soundActor->GetSound() == NULL);

   }
   catch (const dtUtil::Exception& e)
   {
      CPPUNIT_FAIL(e.ToString());
   }
}

