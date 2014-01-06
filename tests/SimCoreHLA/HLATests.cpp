/* -*-c++-*-
* Simulation Core - HLATests (.h & .cpp) - Using 'The MIT License'
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
* @author Eddie Johnson, Olen Bruce, and David Guthrie
*/
#include <prefix/SimCorePrefix.h>
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
#include <dtCore/scene.h>

#include <SimCore/MessageType.h>
#include <SimCore/HLA/HLACustomParameterTranslator.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

using std::shared_ptr;

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
      std::shared_ptr<dtGame::GameManager> mGameManager;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(HLATests);

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
      std::shared_ptr<dtCore::Scene> scene = GetGlobalApplication().GetScene();
      mGameManager = new dtGame::GameManager(*scene);

      dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
      dtCore::System::GetInstance().Start();
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
      mGameManager = nullptr;
   }
}

void HLATests::TestHLAConnection()
{
   std::shared_ptr<dtHLAGM::HLAComponent> thisHLAComponent =  new dtHLAGM::HLAComponent();

   try
   {
      mGameManager->AddComponent(*thisHLAComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      dtHLAGM::HLAComponentConfig config;
      thisHLAComponent->AddParameterTranslator(*new SimCore::HLA::HLACustomParameterTranslator());
      config.LoadConfiguration(*thisHLAComponent, "Federations/RPR1Mapping.xml");
   }
   catch (const dtUtil::Exception& ex)
   {
      CPPUNIT_FAIL(ex.What());
   }

   const std::string fedResource("Federations:RPR-FOM.fed");
   std::string fedFilePath = dtDAL::Project::GetInstance().GetResourcePath(dtDAL::ResourceDescriptor(fedResource));

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

