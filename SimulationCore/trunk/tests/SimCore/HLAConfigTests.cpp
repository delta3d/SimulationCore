/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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

#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <vector>
#include <string>

#include <dtUtil/coordinates.h>
#include <dtCore/globals.h>
#include <dtDAL/datatype.h>
#include <dtDAL/project.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>

#include <dtHLAGM/objecttoactor.h>
#include <dtHLAGM/interactiontomessage.h>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/attributetoproperty.h>
#include <dtHLAGM/parametertoparameter.h>
#include <dtHLAGM/onetoonemapping.h>
#include <dtHLAGM/distypes.h>
#include <dtHLAGM/hlacomponentconfig.h>
#include <dtHLAGM/exceptionenum.h>
#include <dtHLAGM/rprparametertranslator.h>

#include <SimCore/MessageType.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/HLACustomParameterTranslator.h>

#if (defined (WIN32) || defined (_WIN32) || defined (__WIN32__))
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
   const std::string projectContext = "DVTEProject";
#else
   const std::string projectContext = "DVTEProject";
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using dtCore::RefPtr;

class HLAConfigTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(HLAConfigTests);
  
      CPPUNIT_TEST(TestLoadJNTCConfigXML);
      CPPUNIT_TEST(TestLoadVISITConfigXML);

   CPPUNIT_TEST_SUITE_END();

   public:
      void setUp();
      void tearDown();
      void TestLoadJNTCConfigXML();
      void TestLoadVISITConfigXML();
   private:
      dtUtil::Log* logger;
      RefPtr<dtHLAGM::HLAComponent> mTranslator;
      RefPtr<dtGame::GameManager> mGameManager;
      static const char* const mHLAActorRegistry;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(HLAConfigTests);

const char* const HLAConfigTests::mHLAActorRegistry = "IG";

// Called implicitly by CPPUNIT when the app starts
void HLAConfigTests::setUp()
{
   
   dtCore::SetDataFilePathList(dtCore::GetDeltaDataPathList());   
   std::string logName("HLAConfigTest");
   //dtUtil::Log::GetInstance("hlafomconfigxml.cpp").SetLogLevel(dtUtil::Log::LOG_DEBUG);
   logger = &dtUtil::Log::GetInstance(logName);
   mTranslator = new dtHLAGM::HLAComponent();
   mTranslator->AddParameterTranslator( *new SimCore::Components::HLACustomParameterTranslator );
   dtCore::Scene* scene = new dtCore::Scene();
   mGameManager = new dtGame::GameManager(*scene);
   SimCore::MessageType::RegisterMessageTypes(mGameManager->GetMessageFactory());
   dtDAL::Project::GetInstance().SetContext(projectContext, true);   
} 

// Called implicitly by CPPUNIT when the app terminates
void HLAConfigTests::tearDown()
{
   mTranslator = NULL;
   if (mGameManager->GetRegistry(mHLAActorRegistry) != NULL)
      mGameManager->UnloadActorRegistry(mHLAActorRegistry);  

   mGameManager = NULL;
   
}

void HLAConfigTests::TestLoadJNTCConfigXML()
{
   try
   {
      dtDAL::Project::GetInstance().SetContext(projectContext);
      mGameManager->AddComponent(*mTranslator, dtGame::GameManager::ComponentPriority::NORMAL);
      dtHLAGM::HLAComponentConfig config;
      config.LoadConfiguration(*mTranslator, "Federations/JNTC/JNTCMapping.xml");      
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.What());
   }
   
}

void HLAConfigTests::TestLoadVISITConfigXML()
{
   try
   {
      dtDAL::Project::GetInstance().SetContext(projectContext);
      mGameManager->AddComponent(*mTranslator, dtGame::GameManager::ComponentPriority::NORMAL);
      dtHLAGM::HLAComponentConfig config;
      config.LoadConfiguration(*mTranslator, "Federations/DVTE-VISIT/VisitMapping.xml");      
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.What());
   }
   
}
