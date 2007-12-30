#include <prefix/SimCorePrefix-src.h>
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

   SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());

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
