#include <cppunit/extensions/HelperMacros.h>
#include <dtDAL/project.h>
#include <dtDAL/datatype.h>
#include <dtGame/gamemanager.h> 

#include <dtCore/system.h>
#include <string>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtUtil/log.h>
#include <dtDAL/map.h>
#include <dtGame/message.h>
#include <dtGame/basemessages.h>

#include <SimCore/Actors/VehicleAttachingConfigActor.h>

class VehicleConfigTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(VehicleConfigTests);
      CPPUNIT_TEST(TestFunction);
   CPPUNIT_TEST_SUITE_END();

   public:
      VehicleConfigTests() {}
      ~VehicleConfigTests() {}

      void setUp();
      void tearDown();

      void TestFunction();

   private:
     dtCore::RefPtr<dtGame::GameManager> mGM;
     dtCore::RefPtr<dtUtil::Log> mLogger;

};

CPPUNIT_TEST_SUITE_REGISTRATION(VehicleConfigTests);

/////////////////////////////////////////////////////////
void VehicleConfigTests::setUp()
{
   mLogger = &dtUtil::Log::GetInstance("VehicleConfigTests.cpp");

   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();

   mGM = new dtGame::GameManager(*new dtCore::Scene());

   dtCore::System::GetInstance().Step();

   SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());

   dtCore::System::GetInstance().Step();
}

/////////////////////////////////////////////////////////
void VehicleConfigTests::tearDown()
{
   dtCore::System::GetInstance().Stop();

   mGM->DeleteAllActors(true);
   mGM = NULL;
}

/////////////////////////////////////////////////////////
void VehicleConfigTests::TestFunction()
{
   dtCore::RefPtr<dtGame::GameActorProxy> obj;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::VEHICLE_CONFIG_ACTOR_TYPE, obj);
   dtCore::RefPtr<SimCore::Actors::VehicleAttachingConfigActor> objActor = dynamic_cast<SimCore::Actors::VehicleAttachingConfigActor*>(obj->GetActor());

   CPPUNIT_ASSERT(objActor.valid());
   
   objActor->SetInsideModelResource("TestString");
   const std::string& testAgainst = objActor->GetInsideModelResource();
   CPPUNIT_ASSERT(testAgainst == "TestString");

   objActor->SetUsesInsideModel(true);
   CPPUNIT_ASSERT(objActor->GetUsesInsideModel());

   objActor->SetRotationOffSet(osg::Vec3(0,0,20));
   CPPUNIT_ASSERT(objActor->GetRotationOffSet() == osg::Vec3(0,0,20));

   objActor->SetSeatPosition(osg::Vec3(10,10,20));
   CPPUNIT_ASSERT(objActor->GetSeatPosition() == osg::Vec3(10,10,20));
}
