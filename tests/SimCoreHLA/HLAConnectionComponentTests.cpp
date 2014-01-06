/* -*-c++-*-
* Simulation Core - HLAConnectionComponentTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2005-2008, Alion Science and Technology Corporation
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
* David Guthrie
*/
#include <prefix/SimCorePrefix.h>
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
#include <SimCore/HLA/HLAConnectionComponent.h>
#include <SimCore/HLA/HLACustomParameterTranslator.h>
#include <SimCore/Utilities.h>

#include <UnitTestMain.h>

#ifdef DELTA_WIN32
   #include <dtUtil/mswin.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using std::shared_ptr;

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
      std::shared_ptr<dtHLAGM::HLAComponent> mTranslator;
      std::shared_ptr<SimCore::HLA::HLAConnectionComponent> mHLACC;
      std::shared_ptr<dtGame::GameManager> mGameManager;
      std::shared_ptr<dtABC::Application> mApp;
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
   mTranslator->AddParameterTranslator( *new SimCore::HLA::HLACustomParameterTranslator );
   mHLACC = new SimCore::HLA::HLAConnectionComponent();
   
   // A window & camera are needed to allow terrain
   // to generate geometry.
   mApp = &GetGlobalApplication();
   
   mGameManager = new dtGame::GameManager(*mApp->GetScene());
   mGameManager->SetApplication(*mApp);
} 

// Called implicitly by CPPUNIT when the app terminates
void HLAConnectionComponentTests::tearDown()
{
   mTranslator = nullptr;
   mHLACC = nullptr;
   mGameManager = nullptr;
   mApp = nullptr;
}

void HLAConnectionComponentTests::TestAdditionalMaps()
{
   try
   {
      mGameManager->AddComponent(*mTranslator, dtGame::GameManager::ComponentPriority::NORMAL);
      mGameManager->AddComponent(*mHLACC, dtGame::GameManager::ComponentPriority::NORMAL);
      
      mApp->SetConfigPropertyValue(SimCore::Utils::CONFIG_PROP_ADDITIONAL_MAP + "1", "Map1");
      mApp->SetConfigPropertyValue(SimCore::Utils::CONFIG_PROP_ADDITIONAL_MAP + "2", "Map2");
      mApp->SetConfigPropertyValue(SimCore::Utils::CONFIG_PROP_ADDITIONAL_MAP + "3", "Map3");
      // The config file is being found in the root dir and it is using that when it starts up.
      mApp->SetConfigPropertyValue(SimCore::Utils::CONFIG_PROP_ADDITIONAL_MAP + "4", "");

      std::vector<std::string> toFill;
      SimCore::Utils::GetAdditionalMaps(*mGameManager, toFill);
      
      CPPUNIT_ASSERT_EQUAL(3U, unsigned(toFill.size()));
      CPPUNIT_ASSERT_EQUAL(std::string("Map1"), toFill[0]);
      CPPUNIT_ASSERT_EQUAL(std::string("Map2"), toFill[1]);
      CPPUNIT_ASSERT_EQUAL(std::string("Map3"), toFill[2]);

      mApp->SetConfigPropertyValue(SimCore::Utils::CONFIG_PROP_ADDITIONAL_MAP + "2", "");

      toFill.clear();
      SimCore::Utils::GetAdditionalMaps(*mGameManager, toFill);
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Filling should stop when it encounters an index that doesn't exist", 
               1U, unsigned(toFill.size()));
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.What());
   }
   
}
