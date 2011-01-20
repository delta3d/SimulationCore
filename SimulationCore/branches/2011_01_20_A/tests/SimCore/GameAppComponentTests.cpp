/* -*-c++-*-
* Simulation Core - GameAppComponentTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
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
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtDAL/project.h>
#include <dtDAL/datatype.h>
#include <dtGame/gamemanager.h> 

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <string>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtUtil/log.h>
#include <dtDAL/map.h>
#include <dtGame/message.h>
#include <dtGame/basemessages.h>

#include <SimCore/Components/BaseGameAppComponent.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

class BaseGameAppComponentTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE( BaseGameAppComponentTests );
   CPPUNIT_TEST(TestFunction);
   CPPUNIT_TEST_SUITE_END();

public:
   BaseGameAppComponentTests() {}
   ~BaseGameAppComponentTests() {}

   void setUp();
   void tearDown();

   void TestFunction();

private:
   dtCore::RefPtr<dtGame::GameManager> mGM;
   dtCore::RefPtr<dtUtil::Log> mLogger;

};

CPPUNIT_TEST_SUITE_REGISTRATION(BaseGameAppComponentTests);

/////////////////////////////////////////////////////////
void BaseGameAppComponentTests::setUp()
{
   mLogger = &dtUtil::Log::GetInstance("BaseGameAppComponentTests.cpp");

   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();

   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

   dtCore::System::GetInstance().Step();

   dtCore::System::GetInstance().Step();
}

/////////////////////////////////////////////////////////
void BaseGameAppComponentTests::tearDown()
{
   dtCore::System::GetInstance().Stop();

   mGM->DeleteAllActors(true);
   mGM = NULL;
}

/////////////////////////////////////////////////////////
void BaseGameAppComponentTests::TestFunction()
{
   dtCore::RefPtr<SimCore::Components::BaseGameAppComponent> baseGameAppComponent = new SimCore::Components::BaseGameAppComponent();
   mGM->AddComponent(*baseGameAppComponent, dtGame::GameManager::ComponentPriority::NORMAL);

   CPPUNIT_ASSERT_MESSAGE("Could not find BaseGameAppComponent after it was made o nos", 
   mGM->GetComponentByName(SimCore::Components::BaseGameAppComponent::DEFAULT_NAME) != NULL);

   // should have been created
   CPPUNIT_ASSERT(baseGameAppComponent->GetCommandLineObject() != NULL);
}
