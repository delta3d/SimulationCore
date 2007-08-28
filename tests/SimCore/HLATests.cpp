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
 * @author Eddie Johnson, Olen Bruce, and David Guthrie
 */

#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <vector>
#include <string>
#include <osg/Endian>
#include <dtUtil/coordinates.h>
#include <dtDAL/datatype.h>
#include <dtDAL/project.h>
#include <dtDAL/resourcedescriptor.h>
#include <dtHLAGM/objecttoactor.h>
#include <dtHLAGM/interactiontomessage.h>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/attributetoproperty.h>
#include <dtHLAGM/parametertoparameter.h>
#include <dtHLAGM/onetoonemapping.h>
#include <dtHLAGM/distypes.h>
#include <dtHLAGM/hlacomponentconfig.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gmcomponent.h>
#include <dtGame/messagetype.h>
#include <dtGame/defaultmessageprocessor.h>
#include <dtCore/system.h>
#include <dtCore/globals.h>

#include <SimCore/MessageType.h>

using dtCore::RefPtr;

class TestHLAComponent;

class HLATests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(HLATests);
  
      CPPUNIT_TEST(TestHLAConnection);  

   CPPUNIT_TEST_SUITE_END();

   public:
      void setUp();
      void tearDown();
      void TestHLAConnection();
      
   private:
      void TestSpecificIntTranslations(dtHLAGM::OneToOneMapping& mapping, 
         TestHLAComponent& testTrans, 
         long expectedResult);
      RefPtr<dtGame::GameManager> mGameManager;
      
      //shared buffer.
      char* buffer;
      
      static const std::string mTestGameActorLibrary;
      static const std::string mProjectContext;
      
      dtUtil::Log* logger;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(HLATests);

#if (defined (WIN32) || defined (_WIN32) || defined (__WIN32__))
  #if defined (_DEBUG)
      const std::string HLATests::mTestGameActorLibrary="testGameActorLibrary";
  #else
      const std::string HLATests::mTestGameActorLibrary="testGameActorLibrary";
  #endif
   const std::string HLATests::mProjectContext = "DVTEProject";
#else
   const std::string HLATests::mTestGameActorLibrary="testGameActorLibrary";
   const std::string HLATests::mProjectContext = "DVTEProject";
#endif

class TestHLAComponent: public dtHLAGM::HLAComponent
{
   public:
   
      void TestDeleteActor(const dtCore::UniqueId& actorId)
      {
         DeleteActor(actorId); 
      }
};

// Called implicitly by CPPUNIT when the app starts
void HLATests::setUp()
{
   try 
   {
      dtCore::SetDataFilePathList(dtCore::GetDeltaDataPathList());   
      std::string logName("HLAConfigTest");
      logger = &dtUtil::Log::GetInstance(logName);
      RefPtr<dtCore::Scene> scene = new dtCore::Scene();
      mGameManager = new dtGame::GameManager(*scene);
      mGameManager->LoadActorRegistry(mTestGameActorLibrary);
      SimCore::MessageType::RegisterMessageTypes(mGameManager->GetMessageFactory());
      
      dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
      dtCore::System::GetInstance().Start();
      buffer = new char[512];
   }
   catch (const dtUtil::Exception& e) 
   {
      CPPUNIT_FAIL((std::string("Error: ") + e.What()).c_str());
   }
}

// Called implicitly by CPPUNIT when the app terminates
void HLATests::tearDown()
{
   dtCore::System::GetInstance().Stop();
   if (mGameManager.valid())
   {
      mGameManager->UnloadActorRegistry(mTestGameActorLibrary);
      mGameManager = NULL;
   }
   delete [] buffer;
   buffer = NULL;
}

void HLATests::TestHLAConnection()
{

   //   dtDAL::ActorType *m1a1 = new dtDAL::ActorType("M1A1", "Vehicles",
   //						 "This actor represents a M1A1");
   //   std::string hlaM1A1 = "M1A1";
   // dtHLAGM::EntityType thisDisID(1,1,225,1,1,9,0);
  
   // std::vector<dtHLAGM::AttributeToProperty> thisAttributeToPropertyMappingVector;
   // thisAttributeToPropertyMappingVector.push_back(thisAttributeToPropertyMapping);
   // thisAttributeToPropertyMappingVector.push_back(thisAttributeToPropertyMapping);
   RefPtr<dtHLAGM::HLAComponent> thisHLAComponent =  new dtHLAGM::HLAComponent();

   try
   {
      dtDAL::Project::GetInstance().SetContext(mProjectContext, true);
      mGameManager->AddComponent(*thisHLAComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      dtHLAGM::HLAComponentConfig config;
      config.LoadConfiguration(*thisHLAComponent, "Federations/JNTC/JNTCMapping.xml");
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.What());
   }

   std::string fedResource = "Federations:JNTC:jntc.fed";
   std::string fedFilePath = dtDAL::Project::GetInstance().GetResourcePath(dtDAL::ResourceDescriptor("Federations:JNTC:jntc.fed"));

   CPPUNIT_ASSERT_MESSAGE("Fed file \"" + fedResource + "\" was not found.", !fedFilePath.empty());

   thisHLAComponent->JoinFederationExecution("testFedex", fedFilePath, "ThisComputer");

   try
   {
      int j = 100;
      for (int i = 0; i < j; i++)
      {
         //std::cout << "Ticking RTI.  " << j-i << " ticks left.\n";

         dtCore::System::GetInstance().Step();
      }
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.What());
   }

   thisHLAComponent->LeaveFederationExecution();

}

