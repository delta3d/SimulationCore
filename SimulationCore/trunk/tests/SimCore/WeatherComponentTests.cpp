/* -*-c++-*-
* Simulation Core - WeatherComponentTests (.h & .cpp) - Using 'The MIT License'
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

#include <dtABC/application.h>

#include <dtActors/engineactorregistry.h>

#include <dtCore/system.h>
#include <dtCore/refptr.h>
#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>
#include <dtCore/camera.h>
#include <dtCore/environment.h>
#include <dtCore/deltawin.h>

#include <dtDAL/project.h>
#include <dtDAL/map.h>

#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>
#include <dtGame/messagefactory.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>
#include <dtGame/messageparameter.h>

#include <dtUtil/exception.h>

#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/MessageType.h>
#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <UnitTestMain.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {

      //////////////////////////////////////////////////////////////
      // Testable Sub-classed Component
      // (allows public access to protected functions)
      //////////////////////////////////////////////////////////////
      class TestWeatherComponent : public WeatherComponent
      {
         public:

            TestWeatherComponent();

            Actors::DayTimeActorProxy* GetDayTimeActor()
            {
               return WeatherComponent::GetDayTimeActor();
            }

            Actors::UniformAtmosphereActorProxy* GetAtmosphereActor()
            {
               return WeatherComponent::GetAtmosphereActor();
            }

            // Make some protected functions public
            dtABC::Weather::CloudType ClassifyClouds( const SimCore::Actors::UniformAtmosphereActor& atmos );
            dtABC::Weather::WindType ClassifyWind( const SimCore::Actors::UniformAtmosphereActor& atmos );
            dtABC::Weather::VisibilityType ClassifyVisibility( const SimCore::Actors::UniformAtmosphereActor& atmos );

         protected:

            virtual ~TestWeatherComponent();

         private:
      };

      //////////////////////////////////////////////////////////////
      TestWeatherComponent::TestWeatherComponent()
      {

      }

      //////////////////////////////////////////////////////////////
      TestWeatherComponent::~TestWeatherComponent()
      {

      }

      //////////////////////////////////////////////////////////////
      dtABC::Weather::CloudType TestWeatherComponent::ClassifyClouds( const SimCore::Actors::UniformAtmosphereActor& atmos )
      {
         return WeatherComponent::ClassifyClouds(atmos);
      }

      //////////////////////////////////////////////////////////////
      dtABC::Weather::WindType TestWeatherComponent::ClassifyWind( const SimCore::Actors::UniformAtmosphereActor& atmos )
      {
         return WeatherComponent::ClassifyWind(atmos);
      }

      //////////////////////////////////////////////////////////////
      dtABC::Weather::VisibilityType TestWeatherComponent::ClassifyVisibility( const SimCore::Actors::UniformAtmosphereActor& atmos )
      {
         return WeatherComponent::ClassifyVisibility(atmos);
      }

      //////////////////////////////////////////////////////////////
      // Atmosphere Parameters: for changing multiple values within
      // a UniforAtmosphereActor
      //////////////////////////////////////////////////////////////
      struct AtmosphereParams
      {
         // The following was copied from UniformAtmosphereActor:
         float mVisibility;      // km
         float mCloudBaseHeight; // m
         float mCloudTopHeight;  // m
         float mCloudThickness;  // m
         float mFogCover;        // %
         float mFogThickness;    // m
         float mPrecipRate;      // mm/h
         osg::Vec2 mWindSpeed;   // m/s
         SimCore::Actors::CloudType* mCloudType;
         SimCore::Actors::PrecipitationType* mPrecipType;

         bool operator == (const AtmosphereParams &rhs) const
         {
            return mVisibility      == rhs.mVisibility      &&
                   mCloudBaseHeight == rhs.mCloudBaseHeight &&
                   mCloudTopHeight  == rhs.mCloudTopHeight  &&
                   mCloudThickness  == rhs.mCloudThickness  &&
                   mFogCover        == rhs.mFogCover        &&
                   mFogThickness    == rhs.mFogThickness    &&
                   mPrecipRate      == rhs.mPrecipRate      &&
                   mWindSpeed       == rhs.mWindSpeed       &&
                   mCloudType       == rhs.mCloudType       &&
                   mPrecipType      == rhs.mPrecipType;
         }

         bool operator != (const AtmosphereParams &rhs) const
         {
            return !(*this == rhs);
         }
      };

      //////////////////////////////////////////////////////////////
      // Tests Object
      //////////////////////////////////////////////////////////////

      class WeatherComponentTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(WeatherComponentTests);

            CPPUNIT_TEST(TestProperties); // This will also call tests for the Atmosphere & DayTime actors.
            CPPUNIT_TEST(TestMessageProcessing);
            CPPUNIT_TEST(TestUpdateEnabling);
            CPPUNIT_TEST(TestDayTimeActor);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            void CreateEnvironmentActor( dtCore::RefPtr<Actors::IGEnvironmentActorProxy>& ptr );
            void CreateAtmosphereActor( dtCore::RefPtr<Actors::UniformAtmosphereActorProxy>& ptr );
            void CreateDayTimeActor( dtCore::RefPtr<Actors::DayTimeActorProxy>& ptr );

            // Returns the total IDs registered to the component
            void AdvanceSimTime( double deltaTime );

            void TestProperties();
            void TestMessageProcessing();
            void TestAtmosphereActor();
            void TestDayTimeActor();

            void TestAtmosphereUpdates();
            void TestEnvironmentValues( const AtmosphereParams& params, SimCore::Actors::UniformAtmosphereActor& atmos );

            // This function will be used to populate values into new
            // UnifornAtmosphereActors. This will mostly be used in a
            // test where the weather component's atmosphere actor is
            // replaced with another atmosphere actor.
            void AssignAtmosphereValues( const AtmosphereParams& params, SimCore::Actors::UniformAtmosphereActor& actor );
            void GetAtmosphereValues(AtmosphereParams &params, const SimCore::Actors::UniformAtmosphereActor& actor);

            void TestUpdateEnabling();

         protected:
         private:

            dtCore::RefPtr<dtGame::GameManager> mGM;
            dtCore::RefPtr<TestWeatherComponent> mWeatherComp;
            dtCore::RefPtr<dtGame::MachineInfo> mMachineInfo;
            dtCore::RefPtr<dtABC::Application> mApp;

            dtCore::RefPtr<Actors::IGEnvironmentActorProxy> mEnv;
            dtCore::RefPtr<Actors::UniformAtmosphereActorProxy> mAtmos;
            dtCore::RefPtr<Actors::DayTimeActorProxy> mDayTime;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(WeatherComponentTests);


      //////////////////////////////////////////////////////////////
      // Tests code
      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::setUp()
      {
         try
         {
            dtCore::System::GetInstance().Start();

            mApp = &GetGlobalApplication();

            mGM = new dtGame::GameManager(*mApp->GetScene());
            mGM->SetApplication( *mApp );

            mMachineInfo = new dtGame::MachineInfo;
            mWeatherComp = new TestWeatherComponent;

            mGM->AddComponent(*mWeatherComp, dtGame::GameManager::ComponentPriority::NORMAL);

            CreateEnvironmentActor(mEnv);
            CreateAtmosphereActor(mAtmos);
            CreateDayTimeActor(mDayTime);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::tearDown()
      {
         dtCore::System::GetInstance().Stop();

         mApp = NULL;

         if (mGM.valid())
         {
            mGM->DeleteAllActors(true);
         }

         mGM = NULL;
         mMachineInfo = NULL;
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::CreateEnvironmentActor( dtCore::RefPtr<Actors::IGEnvironmentActorProxy>& ptr )
      {
         mGM->CreateActor( *Actors::EntityActorRegistry::ENVIRONMENT_ACTOR_TYPE, ptr );
         CPPUNIT_ASSERT_MESSAGE("IGEnvironmentActor must be obtainable from the EntityActorRegistry", ptr.valid() );
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::CreateAtmosphereActor( dtCore::RefPtr<Actors::UniformAtmosphereActorProxy>& ptr )
      {
         mGM->CreateActor( *Actors::EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE, ptr );
         CPPUNIT_ASSERT_MESSAGE("UniformAtmosphereActor must be obtainable from the EntityActorRegistry", ptr.valid() );
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::CreateDayTimeActor( dtCore::RefPtr<Actors::DayTimeActorProxy>& ptr )
      {
         mGM->CreateActor( *Actors::EntityActorRegistry::DAYTIME_ACTOR_TYPE, ptr );
         CPPUNIT_ASSERT_MESSAGE("DayTimeActor must be obtainable from the EntityActorRegistry", ptr.valid() );
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::TestDayTimeActor()
      {
         Actors::DayTimeActor* actor = static_cast<Actors::DayTimeActor*> (mDayTime->GetDrawable());

         CPPUNIT_ASSERT_MESSAGE("DayTimeActor should be valid", actor != NULL );

         CPPUNIT_ASSERT_MESSAGE("Year should be 0",
            actor->GetYear() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Month should be 0",
            actor->GetMonth() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Day should be the 0",
            actor->GetDay() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Hour should be 0",
            actor->GetHour() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Minute should be 0",
            actor->GetMinute() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Second should be 0",
            actor->GetSecond() == 0 );

         // The following time stamp value was captured from a weather server.
         // 1166207083 == Fri Dec 15 18:24:43 2006
         actor->SetTime(1166207083);

         CPPUNIT_ASSERT_MESSAGE("Year should be 106 (2006-1900)",
            actor->GetYear() == (2006-1900) );
         CPPUNIT_ASSERT_MESSAGE("Month should be December (index 11)",
            actor->GetMonth() == 11 );
         CPPUNIT_ASSERT_MESSAGE("Day should be the 15",
            actor->GetDay() == 15 );
         CPPUNIT_ASSERT_MESSAGE("Hour should be 18",
            actor->GetHour() == 18 );
         CPPUNIT_ASSERT_MESSAGE("Minute should be 24",
            actor->GetMinute() == 24 );
         CPPUNIT_ASSERT_MESSAGE("Second should be 43",
            actor->GetSecond() == 43 );

         // Test the time zone offset
         CPPUNIT_ASSERT_MESSAGE("Time zone offset should be 0 by default",
            actor->GetPrimeMeridianHourOffset() == 0 );
         // --- Test clamping the offset
         actor->SetPrimeMeridianHourOffset( -100 );
         CPPUNIT_ASSERT_MESSAGE("Time zone offset should clamp to -23 at the lower limit",
            actor->GetPrimeMeridianHourOffset() == -23 );
         actor->SetPrimeMeridianHourOffset( 100 );
         CPPUNIT_ASSERT_MESSAGE("Time zone offset should clamp to 23 at the higher limit",
            actor->GetPrimeMeridianHourOffset() == 23 );
         actor->SetPrimeMeridianHourOffset( -5 );
         CPPUNIT_ASSERT_MESSAGE("Time zone offset should have a new value",
            actor->GetPrimeMeridianHourOffset() == -5 );

         // Test setting the SAME Greenwich Mean Time with while figuring in the time zone offset.
         // The following time stamp value was captured from a weather server.
         // 1166207083 == Fri Dec 15 18:24:43 2006
         actor->SetTime(1166207083);

         CPPUNIT_ASSERT_MESSAGE("Year should still be 106 (2006-1900)",
            actor->GetYear() == (2006-1900) );
         CPPUNIT_ASSERT_MESSAGE("Month should still be December (index 11)",
            actor->GetMonth() == 11 );
         CPPUNIT_ASSERT_MESSAGE("Day should still be the 15",
            actor->GetDay() == 15 );
         CPPUNIT_ASSERT_MESSAGE("Hour should still be 13",
            actor->GetHour() == 18 );
         CPPUNIT_ASSERT_MESSAGE("Minute should still be 24",
            actor->GetMinute() == 24 );
         CPPUNIT_ASSERT_MESSAGE("Second should still be 43",
            actor->GetSecond() == 43 );

         // Test offsetting the hour out of the 0 to 23 range. This should NOT happen.
         // --- Ensure the original hour is still 18 without the Prime Meridian offset
         actor->SetPrimeMeridianHourOffset( 0 );
         CPPUNIT_ASSERT( actor->GetPrimeMeridianHourOffset() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Original hour should still be maintained by the DayTimeActor", actor->GetHour() == 18 );

      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::TestAtmosphereActor()
      {
         CPPUNIT_ASSERT(mAtmos != NULL);
         Actors::UniformAtmosphereActor* actor = static_cast<Actors::UniformAtmosphereActor*> (mAtmos->GetDrawable());

         CPPUNIT_ASSERT_MESSAGE("UniformAtmosphereActor should be valid", actor != NULL );

         // Note: the tested values are random and have no relevance.

         float testValue = 1000.0f;
         CPPUNIT_ASSERT_MESSAGE("Visibility should NOT be 1000.0",
            actor->GetVisibilityDistance() != testValue );
         actor->SetVisibilityDistance( testValue );
         CPPUNIT_ASSERT_MESSAGE("Visibility should be 1000.0",
            actor->GetVisibilityDistance() == testValue );

         CPPUNIT_ASSERT_MESSAGE("CloudBaseHeight should be 0.0",
            actor->GetCloudBaseHeight() == 0.0f );
         testValue = 2500.0f;
         actor->SetCloudBaseHeight( testValue );
         CPPUNIT_ASSERT_MESSAGE("CloudBaseHeight should be 2500.0",
            actor->GetCloudBaseHeight() == testValue );

         CPPUNIT_ASSERT_MESSAGE("CloudTopHeight should be 0.0",
            actor->GetCloudTopHeight() == 0.0f );
         testValue = 3600.0f;
         actor->SetCloudTopHeight( testValue );
         CPPUNIT_ASSERT_MESSAGE("CloudTopHeight should be 3600.0",
            actor->GetCloudTopHeight() == testValue );

         CPPUNIT_ASSERT_MESSAGE("CloudThickness should be 0.0",
            actor->GetCloudThickness() == 0.0f );
         testValue = 30.0f;
         actor->SetCloudThickness( testValue );
         CPPUNIT_ASSERT_MESSAGE("CloudThickness should be 30.0",
            actor->GetCloudThickness() == testValue );

         CPPUNIT_ASSERT_MESSAGE("FogCover should be 0.0",
            actor->GetFogCover() == 0.0f );
         testValue = 15.0f;
         actor->SetFogCover( testValue );
         CPPUNIT_ASSERT_MESSAGE("FogCover should be 15.0",
            actor->GetFogCover() == testValue );

         CPPUNIT_ASSERT_MESSAGE("FogThickness should be 0.0",
            actor->GetFogThickness() == 0.0f );
         testValue = 21.0f;
         actor->SetFogThickness( testValue );
         CPPUNIT_ASSERT_MESSAGE("FogThickness should be 21.0",
            actor->GetFogThickness() == testValue );

         CPPUNIT_ASSERT_MESSAGE("PrecipitationRate should be 0.0",
            actor->GetPrecipitationRate() == 0.0f );
         testValue = 63.0f;
         actor->SetPrecipitationRate( testValue );
         CPPUNIT_ASSERT_MESSAGE("PrecipitationRate should be 63.0",
            actor->GetPrecipitationRate() == testValue );

         testValue = 12.25f;
         actor->SetWindSpeedX( testValue );
         CPPUNIT_ASSERT_MESSAGE("WindSpeedX should be 12.25",
            actor->GetWindSpeedX() == testValue );

         testValue = 13.50f;
         actor->SetWindSpeedY( testValue );
         CPPUNIT_ASSERT_MESSAGE("WindSpeedY should be 13.50",
            actor->GetWindSpeedY() == testValue );

         osg::Vec2 wind( 4.75, 18.92 );
         actor->SetWind( wind );
         CPPUNIT_ASSERT_MESSAGE("WindSpeedX should be 4.75",
            actor->GetWindSpeedX() == wind[0] );
         CPPUNIT_ASSERT_MESSAGE("WindSpeedY should be 18.92",
            actor->GetWindSpeedY() == wind[1] );
         CPPUNIT_ASSERT_MESSAGE("Wind should be [4.75, 18.92]",
            actor->GetWind() == wind );

         CPPUNIT_ASSERT_MESSAGE("CloudType should be CLEAR",
            actor->GetCloudType() == SimCore::Actors::CloudType::CLEAR );
         actor->SetCloudType( SimCore::Actors::CloudType::CIRROCUMULUS );
         CPPUNIT_ASSERT_MESSAGE("CloudType should be CIRROCUMULUS",
            actor->GetCloudType() == SimCore::Actors::CloudType::CIRROCUMULUS );

         CPPUNIT_ASSERT_MESSAGE("PrecipitationType should be NONE",
            actor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::NONE);
         actor->SetPrecipitationType( SimCore::Actors::PrecipitationType::FREEZING_RAIN );
         CPPUNIT_ASSERT_MESSAGE("PrecipitationType should be FREEZING_RAIN",
            actor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::FREEZING_RAIN );

      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::TestProperties()
      {
         // The following 2 tests are done within this test for 2 reasons:
         // 1 - To avoid more registry library loading and unloading via setUp and tearDown.
         // 2 - These tests merely test the properties of the actors.
         TestAtmosphereActor();
         TestDayTimeActor();

         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have an AtmosphereActor",
            mWeatherComp->GetAtmosphereActor() == NULL );

         mWeatherComp->SetEphemerisEnvironment(static_cast<SimCore::Actors::IGEnvironmentActor*>(mEnv->GetDrawable()));
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an EnvironmentActor",
            mWeatherComp->GetEphemerisEnvironment() != NULL );

         // Test setting the base elevation (from which fog changes are calculated for the view)
         double testValue = mWeatherComp->GetBaseElevation() + 1000.0;
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have a NEW base elevation",
            mWeatherComp->GetBaseElevation() != testValue );
         mWeatherComp->SetBaseElevation( testValue );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have a NEW base elevation",
            mWeatherComp->GetBaseElevation() == testValue );

         // Test setting the max visibility (the horizontal visibility)
         testValue = mWeatherComp->GetMaxVisibility() + 1000.0;
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have a NEW max visibility",
            mWeatherComp->GetMaxVisibility() != testValue );
         mWeatherComp->SetMaxVisibility( testValue );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have a NEW max visibility",
            mWeatherComp->GetMaxVisibility() == testValue );

         // Test setting max elevation visibility (the vertical visibility)
         //testValue = mWeatherComp->GetMaxElevationVisibility() + 1000.0;
         //CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have a NEW max elevation visibility",
         //   mWeatherComp->GetMaxElevationVisibility() != testValue );
         //mWeatherComp->SetMaxElevationVisibility( testValue );
         //CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have a NEW max elevation visibility",
         //   mWeatherComp->GetMaxElevationVisibility() == testValue );

         // Test setting near
         testValue = mWeatherComp->GetNearClipPlane() + 1000.0;
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have a NEW near clip distance",
            mWeatherComp->GetNearClipPlane() != (float)testValue );
         mWeatherComp->SetNearClipPlane( (float)testValue );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( (float)testValue, mWeatherComp->GetNearClipPlane(), 0.0001 );

         // Test setting far
         testValue = mWeatherComp->GetFarClipPlane() + 1000.0;
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have a NEW far clip distance",
            mWeatherComp->GetFarClipPlane() != (float)testValue );
         mWeatherComp->SetFarClipPlane( (float)testValue );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have a NEW far clip distance",
            mWeatherComp->GetFarClipPlane() == (float)testValue );

         // Test setting near and far
         testValue = mWeatherComp->GetNearClipPlane() + 1000.0f;
         float testValue2 = mWeatherComp->GetFarClipPlane() + 1000.0f;
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have a NEW near clip distance",
            mWeatherComp->GetNearClipPlane() != (float)testValue );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have a NEW far clip distance",
            mWeatherComp->GetFarClipPlane() != testValue2 );
         mWeatherComp->SetNearFarClipPlanes( (float)testValue, testValue2 );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have a NEW near clip distance",
            mWeatherComp->GetNearClipPlane() == (float)testValue );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have a NEW far clip distance",
            mWeatherComp->GetFarClipPlane() == testValue2 );
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::TestMessageProcessing()
      {
         // Test actor create messages
         // --- Test the Environment create message

         mWeatherComp->SetEphemerisEnvironment(static_cast<SimCore::Actors::IGEnvironmentActor*>(mEnv->GetDrawable()));
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an EnvironmentActor",
            mWeatherComp->GetEphemerisEnvironment() != NULL );

         // --- Test the DayTime create message
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have an DayTimeActor",
            mWeatherComp->GetDayTimeActor() == NULL );
         Actors::DayTimeActor* actor = static_cast<Actors::DayTimeActor*> (mDayTime->GetDrawable());
         actor->SetTime(1166207083);
         mGM->AddActor( *mDayTime, false, false );
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an DayTimeActor",
            mWeatherComp->GetDayTimeActor() != NULL );

         // --- Test the Atmosphere create message
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have an AtmosphereActor",
            mWeatherComp->GetAtmosphereActor() == NULL );
         mGM->AddActor( *mAtmos, false, false );
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an AtmosphereActor",
            mWeatherComp->GetAtmosphereActor() != NULL );

         // Prepare to capture and test the time set on
         // the weather component's environment actor.
         int year, month, day, hour, minute, second;
         year = month = day = hour = minute = second = 0;
         CPPUNIT_ASSERT_MESSAGE("DayTimeActor SHOULD have a non-zero year",
            actor->GetYear() != year );
         CPPUNIT_ASSERT_MESSAGE("DayTimeActor SHOULD have a non-zero month",
            actor->GetMonth() != month );
         CPPUNIT_ASSERT_MESSAGE("DayTimeActor SHOULD have a non-zero day",
            actor->GetDay() != day );
         CPPUNIT_ASSERT_MESSAGE("DayTimeActor SHOULD have a non-zero hour",
            actor->GetHour() != hour );
         CPPUNIT_ASSERT_MESSAGE("DayTimeActor SHOULD have a non-zero minute",
            actor->GetMinute() != minute );
         CPPUNIT_ASSERT_MESSAGE("DayTimeActor SHOULD have a non-zero second",
            actor->GetSecond() != second );

         // Make sure the environment actor was updated with the time contained
         // within the DayTimeActor that was captured.
         //const dtActors::BasicEnvironmentActorProxy* env =
         //   dynamic_cast<const dtActors::BasicEnvironmentActorProxy*> (mWeatherComp->GetEphemerisEnvironment());
         //CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an IEnvGameActorProxy",
         //   env != NULL );

         //const dtActors::BasicEnvironmentActor* envActor =
         //   dynamic_cast<const dtActors::BasicEnvironmentActor*> (&env->GetGameActor());
         //CPPUNIT_ASSERT_MESSAGE("WeatherComponent's IEnvGameActorProxy SHOULD have an EnvironmentActor",
         //   envActor != NULL );

         // --- Test the captured values
         /*envActor->GetTimeAndDate(year, month, day, hour, minute, second);
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent's environment SHOULD be set to the correct year",
            actor->GetYear() == year-1900 );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent's environment SHOULD be set to the correct month",
            actor->GetMonth() == month-1 );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent's environment SHOULD be set to the correct day",
            actor->GetDay() == day );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent's environment SHOULD be set to the correct hour",
            actor->GetHour() == hour );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent's environment SHOULD be set to the correct minute",
            actor->GetMinute() == minute );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent's environment SHOULD be set to the correct second",
            actor->GetSecond() == second );*/

         // Run through some atmosphere change tests.
         // WARNING!!! --- The original atmosphere will not exist after this function.
         //                It will have been replaced by a new atmosphere actor.
         TestAtmosphereUpdates();

         // Test actor delete messages
         // --- Delete all actors
         mGM->DeleteAllActors();
         dtCore::System::GetInstance().Step();

         // --- Test the DayTime delete message
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have an DayTimeActor",
            mWeatherComp->GetDayTimeActor() == NULL );

         // --- Test the Atmosphere delete message
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have an AtmosphereActor",
            mWeatherComp->GetAtmosphereActor() == NULL );

         // Add all actors again to test reset
         // --- Test the Environment create message
         mWeatherComp->SetEphemerisEnvironment(static_cast<SimCore::Actors::IGEnvironmentActor*>(mEnv->GetDrawable()));
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an EnvironmentActor",
            mWeatherComp->GetEphemerisEnvironment() != NULL );

         // --- Test the DayTime create message
         mGM->AddActor( *mDayTime, false, false );
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an DayTimeActor",
            mWeatherComp->GetDayTimeActor() != NULL );

         // --- Test the Atmosphere create message
         mGM->AddActor( *mAtmos, false, false );
         dtCore::System::GetInstance().Step();
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have an AtmosphereActor",
            mWeatherComp->GetAtmosphereActor() != NULL );



         // Clear the component
         mWeatherComp->Clear();

         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have an AtmosphereActor",
            mWeatherComp->GetAtmosphereActor() == NULL );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should still have a DayTimeActor.  See the comments in the code.",
            mWeatherComp->GetDayTimeActor() != NULL );
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent should NOT have an Environment Actor",
            mWeatherComp->GetEphemerisEnvironment() == NULL );



         // Simulate the environment loading from a map file
         dtCore::System::GetInstance().Step();
         RefPtr<dtGame::Message> msg =
            mGM->GetMessageFactory().CreateMessage( dtGame::MessageType::INFO_MAP_LOADED );
         mGM->SendMessage( *msg );
         dtCore::System::GetInstance().Step();
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::TestAtmosphereUpdates()
      {
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have the ORIGINAL AtmosphereActor",
            mAtmos.get() == mWeatherComp->GetAtmosphereActor() );

         SimCore::Actors::UniformAtmosphereActor* atmos =
            dynamic_cast<SimCore::Actors::UniformAtmosphereActor*> (mAtmos->GetDrawable());

         // Test update changes to the original atmosphere actor
         AtmosphereParams params = {5.0f,5000.0f,10000.0f,10.0f,20.0f,500.0f,15.0f,
            osg::Vec2(2.0f,4.0f),
            &SimCore::Actors::CloudType::CIRRUS, &SimCore::Actors::PrecipitationType::RAIN};

         TestEnvironmentValues( params, *atmos );



         // Create a new atmosphere actor that will replace the original
         // atmosphere actor.
         dtCore::RefPtr<SimCore::Actors::UniformAtmosphereActorProxy> atmosProxy;
         CreateAtmosphereActor( atmosProxy );

         // Validate new atmosphere actor proxy
         CPPUNIT_ASSERT_MESSAGE("A NEW AtmosphereActor SHOULD have been created", atmosProxy.valid() );
         mGM->AddActor(*atmosProxy,false,false);

         // Get the new proxy's actor
         atmos = dynamic_cast<SimCore::Actors::UniformAtmosphereActor*> (atmosProxy->GetDrawable());

         // Assign values to the new atmosphere actor
         AtmosphereParams params2 = {10.0f,10000.0f,20000.0f,20.0f,40.0f,1000.0f,30.0f,
            osg::Vec2(-4.0f,-2.0f),
            &SimCore::Actors::CloudType::ALTOSTRATUS, &SimCore::Actors::PrecipitationType::HAIL};

         // Ensure that the values exist
         std::cout << "NOTE: warning will follow as expected: testing replacement of old atmosphere actor with a newly discovered one"
            << std::endl;
         TestEnvironmentValues( params2, *atmos );



         // Make sure the original atmosphere actor has been replaced
         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have the NEW AtmosphereActor",
            mAtmos.get() != mWeatherComp->GetAtmosphereActor()
            && atmosProxy == mWeatherComp->GetAtmosphereActor());
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::TestEnvironmentValues(
         const AtmosphereParams& params, SimCore::Actors::UniformAtmosphereActor& atmos )
      {
         // Create the update message
         dtCore::RefPtr<dtGame::ActorUpdateMessage> msg;
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED, msg);

         // Ensure message is valid
         CPPUNIT_ASSERT_MESSAGE( "GameManager could not create an ActorUpdateMessage.",
            msg.valid() );

         // Fill the message with the updated data
         msg->SetAboutActorId( atmos.GetGameActorProxy().GetId() );
         msg->SetActorType(*SimCore::Actors::EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE);
         AssignAtmosphereValues(params, atmos);

         // Send the message
         mWeatherComp->ProcessMessage(*msg);
         dtCore::System::GetInstance().Step();

         // Also test that the atmosphere change has updated the
         // weather component's environment actor properly.
         // TODO: Test the changes in the weather component's environment actor,
         // that have occurred during an atmosphere update.
         //dtActors::BasicEnvironmentActorProxy* envProxy = mWeatherComp->GetEnvironment();
         //dtABC::Weather& weather = dynamic_cast<dtActors::BasicEnvironmentActor*>(envProxy->GetActor())->GetWeather();
         //dtCore::Environment* env = weather.GetEnvironment();

         //// Test the resulting visibility
         //float visibility = env->GetVisibility()/1000.0f; // convert meters to kilometers
         //CPPUNIT_ASSERT_MESSAGE( "WeatherComponent's dtCoreEnvironment SHOULD have the correct visibility",
         //   visibility == params.mVisibility );

         //// The following lines test unused/incomplete features:
         //CPPUNIT_ASSERT_MESSAGE( "WeatherComponent's Weather SHOULD have the correct visibility classification",
         //   weather.GetBasicVisibilityType() == mWeatherComp->ClassifyVisibility( atmos ) );
         //CPPUNIT_ASSERT_MESSAGE( "WeatherComponent's Weather SHOULD have the correct wind classification",
         //   weather.GetBasicWindType() == mWeatherComp->ClassifyWind( atmos ) );
         //CPPUNIT_ASSERT_MESSAGE( "WeatherComponent's Weather SHOULD have the correct cloud classification",
         //   weather.GetBasicCloudType() == mWeatherComp->ClassifyClouds( atmos ) );
      }

      //////////////////////////////////////////////////////////////
      void WeatherComponentTests::AssignAtmosphereValues(
         const AtmosphereParams& params, SimCore::Actors::UniformAtmosphereActor& actor )
      {
         actor.SetVisibilityDistance( params.mVisibility );
         actor.SetCloudBaseHeight( params.mCloudBaseHeight );
         actor.SetCloudTopHeight( params.mCloudTopHeight );
         actor.SetCloudThickness( params.mCloudThickness );
         actor.SetFogCover( params.mFogCover );
         actor.SetFogThickness( params.mFogThickness );
         actor.SetPrecipitationRate( params.mPrecipRate );
         actor.SetWind( params.mWindSpeed );
         actor.SetCloudType( *params.mCloudType );
         actor.SetPrecipitationType( *params.mPrecipType );
      }

      ///////////////////////////////////////////////////////////////
      void WeatherComponentTests::GetAtmosphereValues(AtmosphereParams &params,
         const SimCore::Actors::UniformAtmosphereActor& actor)
      {
         params.mVisibility      =  actor.GetVisibilityDistance();
         params.mCloudBaseHeight =  actor.GetCloudBaseHeight();
         params.mCloudTopHeight  =  actor.GetCloudTopHeight();
         params.mCloudThickness  =  actor.GetCloudThickness();
         params.mFogCover        =  actor.GetFogCover();
         params.mFogThickness    =  actor.GetFogThickness();
         params.mPrecipRate      =  actor.GetPrecipitationRate();
         params.mWindSpeed       =  actor.GetWind();
         params.mCloudType       = &actor.GetCloudType();
         params.mPrecipType      = &actor.GetPrecipitationType();
      }

      ///////////////////////////////////////////////////////////////
      void WeatherComponentTests::TestUpdateEnabling()
      {
         TestWeatherComponent &weatherComp = *mWeatherComp;

         CPPUNIT_ASSERT_MESSAGE("Updates on the weather component should be enabled by default",
            weatherComp.GetUpdatesEnabled());

         mWeatherComp->SetUpdatesEnabled(false);
         CPPUNIT_ASSERT(!weatherComp.GetUpdatesEnabled());

         mGM->AddActor(*mAtmos, false, false);
         dtCore::System::GetInstance().Step();

         CPPUNIT_ASSERT_MESSAGE("WeatherComponent SHOULD have the ORIGINAL AtmosphereActor",
            mAtmos.get() == weatherComp.GetAtmosphereActor());

         SimCore::Actors::UniformAtmosphereActor &atmos =
            static_cast<SimCore::Actors::UniformAtmosphereActor&>(mAtmos->GetGameActor());

         // Test update changes to the original atmosphere actor
         AtmosphereParams params = {5.0f,5000.0f,10000.0f,10.0f,20.0f,500.0f,15.0f,
            osg::Vec2(2.0f,4.0f),
            &SimCore::Actors::CloudType::CIRRUS, &SimCore::Actors::PrecipitationType::RAIN};

         AssignAtmosphereValues(params, atmos);

         dtCore::RefPtr<dtGame::ActorUpdateMessage> msg;
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED, msg);

         // Ensure message is valid
         CPPUNIT_ASSERT_MESSAGE("GameManager could not create an ActorUpdateMessage.",
            msg.valid());

         // Fill the message with the updated data
         msg->SetAboutActorId(atmos.GetUniqueId());

         // Send the message
         mGM->SendMessage(*msg);
         dtCore::System::GetInstance().Step();

         AtmosphereParams toFillIn;
         GetAtmosphereValues(toFillIn, atmos);
      }
   }
}
