#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>
#include <string>

#include <dtDAL/project.h>
#include <dtDAL/datatype.h>
#include <dtGame/gamemanager.h> 

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <dtGame/messagetype.h>
#include <dtGame/messagefactory.h>
#include <dtGame/basemessages.h>

#include <SimCore/CommandLineObject.h>
#include <dtDAL/namedparameter.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

using namespace SimCore::Actors;

// note name wasnt changed but this is commandlineobject now.

class CommandLineComponentTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(CommandLineComponentTests);
   CPPUNIT_TEST(TestFunction);
   CPPUNIT_TEST_SUITE_END();

public:
   CommandLineComponentTests()
   {
   }
   ~CommandLineComponentTests()
   {
   }

   void setUp()
   {
      dtCore::System::GetInstance().Start();

      mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

      dtCore::System::GetInstance().Step();

      SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());

      mCommandLineObject = new SimCore::CommandLineObject();

      dtCore::System::GetInstance().Step();
   }

   void tearDown()
   {
      dtCore::System::GetInstance().Stop();

      mGM->DeleteAllActors(true);
      mGM = NULL;
   }

   void TestFunction();

private:
   dtCore::RefPtr<SimCore::CommandLineObject> mCommandLineObject;
   dtCore::RefPtr<dtGame::GameManager> mGM;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CommandLineComponentTests);

////////////////////////////////////////////////////////////////////////
void CommandLineComponentTests::TestFunction()
{
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("mCommandLineComponent not initialized", (mCommandLineObject != NULL));
   
   dtCore::RefPtr<dtDAL::NamedStringParameter> pArAmEtEr = new dtDAL::NamedStringParameter("SimpleName", "lollerskatez_dont_fall_down");
   mCommandLineObject->AddParameter(pArAmEtEr.get());

   CPPUNIT_ASSERT(mCommandLineObject->GetParameter("SimpleName") != NULL);
   CPPUNIT_ASSERT(mCommandLineObject->GetParameter("") == NULL);

   mCommandLineObject->ClearParametersList();

   CPPUNIT_ASSERT(mCommandLineObject->GetParameter("SimpleName") == NULL);

   dtCore::System::GetInstance().Step();
}
