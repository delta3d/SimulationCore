#include <cppunit/extensions/HelperMacros.h>
#include <dtDAL/project.h>
#include <dtDAL/datatype.h>
#include <dtGame/gamemanager.h> 

#include <dtCore/system.h>
#include <string>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <dtGame/messagetype.h>
#include <dtGame/messagefactory.h>
#include <dtGame/basemessages.h>

#include <SimCore/CommandLineObject.h>
#include <dtDAL/namedparameter.h>

const std::string IG_REGISTRY = "IG";
using namespace SimCore::entity;
using namespace SimCore::IG;

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

      mGM = new dtGame::GameManager(*new dtCore::Scene());
      mGM->LoadActorRegistry(IG_REGISTRY);

      dtCore::System::GetInstance().Step();

      SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());

      mCommandLineObject = new SimCore::CommandLineObject();

      dtCore::System::GetInstance().Step();
   }

   void tearDown()
   {
      dtCore::System::GetInstance().Stop();

      mGM->DeleteAllActors(true);
      mGM->UnloadActorRegistry(IG_REGISTRY);
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
