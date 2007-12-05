/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2007, Alion Science and Technology.
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
 * David Guthrie
 */
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <vector>
#include <string>

#include <dtCore/scene.h>
#include <dtABC/application.h>

#include <dtGame/gamemanager.h>

#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/hlacomponentconfig.h>

#include <SimCore/MessageType.h>
#include <SimCore/Components/HLAConnectionComponent.h>
#include <SimCore/Components/HLACustomParameterTranslator.h>

#include <UnitTestMain.h>

#ifdef DELTA_WIN32
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using dtCore::RefPtr;

class HLAConnectionComponentTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(HLAConnectionComponentTests);
  
      CPPUNIT_TEST(TestAdditionalMaps);

   CPPUNIT_TEST_SUITE_END();

   public:
      void setUp();
      void tearDown();
      void TestAdditionalMaps();
   private:
      dtUtil::Log* logger;
      RefPtr<dtHLAGM::HLAComponent> mTranslator;
      RefPtr<SimCore::Components::HLAConnectionComponent> mHLACC;
      RefPtr<dtGame::GameManager> mGameManager;
      RefPtr<dtABC::Application> mApp;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(HLAConnectionComponentTests);

// Called implicitly by CPPUNIT when the app starts
void HLAConnectionComponentTests::setUp()
{
   
   std::string logName("HLAConfigTest");
   //dtUtil::Log::GetInstance("hlafomconfigxml.cpp").SetLogLevel(dtUtil::Log::LOG_DEBUG);
   logger = &dtUtil::Log::GetInstance(logName);
   mTranslator = new dtHLAGM::HLAComponent();
   mTranslator->AddParameterTranslator( *new SimCore::Components::HLACustomParameterTranslator );
   mHLACC = new SimCore::Components::HLAConnectionComponent();
   
   // A window & camera are needed to allow terrain
   // to generate geometry.
   mApp = &GetGlobalApplication();
   
   mGameManager = new dtGame::GameManager(*mApp->GetScene());
   mGameManager->SetApplication(*mApp);
   SimCore::MessageType::RegisterMessageTypes(mGameManager->GetMessageFactory());
} 

// Called implicitly by CPPUNIT when the app terminates
void HLAConnectionComponentTests::tearDown()
{
   mTranslator = NULL;
   mHLACC = NULL;
   mGameManager = NULL;
   mApp = NULL;
}

void HLAConnectionComponentTests::TestAdditionalMaps()
{
   try
   {
      mGameManager->AddComponent(*mTranslator, dtGame::GameManager::ComponentPriority::NORMAL);
      mGameManager->AddComponent(*mHLACC, dtGame::GameManager::ComponentPriority::NORMAL);
      
      
      mApp->SetConfigPropertyValue(SimCore::Components::HLAConnectionComponent::CONFIG_PROP_ADDITIONAL_MAP + "1", 
               "Map1");
      mApp->SetConfigPropertyValue(SimCore::Components::HLAConnectionComponent::CONFIG_PROP_ADDITIONAL_MAP + "2", 
               "Map2");
      mApp->SetConfigPropertyValue(SimCore::Components::HLAConnectionComponent::CONFIG_PROP_ADDITIONAL_MAP + "3", 
               "Map3");

      std::vector<std::string> toFill;
      mHLACC->GetAdditionalMaps(toFill);
      
      CPPUNIT_ASSERT_EQUAL(3U, unsigned(toFill.size()));
      CPPUNIT_ASSERT_EQUAL(std::string("Map1"), toFill[0]);
      CPPUNIT_ASSERT_EQUAL(std::string("Map2"), toFill[1]);
      CPPUNIT_ASSERT_EQUAL(std::string("Map3"), toFill[2]);

      mApp->SetConfigPropertyValue(SimCore::Components::HLAConnectionComponent::CONFIG_PROP_ADDITIONAL_MAP + "2",
               "");

      toFill.clear();
      mHLACC->GetAdditionalMaps(toFill);
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Filling should stop when it encounters an index that doesn't exist", 
               1U, unsigned(toFill.size()));
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.What());
   }
   
}
