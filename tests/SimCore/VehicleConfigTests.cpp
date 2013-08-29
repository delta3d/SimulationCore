/* -*-c++-*-
* Simulation Core - VehicleConfigTests (.h & .cpp) - Using 'The MIT License'
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

#include <SimCore/Actors/VehicleAttachingConfigActor.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

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

   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

   dtCore::System::GetInstance().Step();

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
   dtCore::RefPtr<SimCore::Actors::VehicleAttachingConfigActor> objActor = dynamic_cast<SimCore::Actors::VehicleAttachingConfigActor*>(obj->GetDrawable());

   CPPUNIT_ASSERT(objActor.valid());
   
   dtDAL::ResourceDescriptor rdTest("StaticMeshes:hazard_cone.ive");

   {
      CPPUNIT_ASSERT_EQUAL(objActor->GetInsideModelResourceGood(), dtDAL::ResourceDescriptor::NULL_RESOURCE);
      objActor->SetInsideModelResourceGood(rdTest);
      const dtDAL::ResourceDescriptor& testAgainst = objActor->GetInsideModelResourceGood();
      CPPUNIT_ASSERT_EQUAL(rdTest, testAgainst);
   }

   {
      CPPUNIT_ASSERT_EQUAL(objActor->GetInsideModelResourceDamaged(), dtDAL::ResourceDescriptor::NULL_RESOURCE);
      objActor->SetInsideModelResourceDamaged(rdTest);
      const dtDAL::ResourceDescriptor& testAgainst = objActor->GetInsideModelResourceDamaged();
      CPPUNIT_ASSERT_EQUAL(rdTest, testAgainst);
   }

   {
      CPPUNIT_ASSERT_EQUAL(objActor->GetInsideModelResourceDestroyed(), dtDAL::ResourceDescriptor::NULL_RESOURCE);
      objActor->SetInsideModelResourceDestroyed(rdTest);
      const dtDAL::ResourceDescriptor& testAgainst = objActor->GetInsideModelResourceDestroyed();
      CPPUNIT_ASSERT_EQUAL(rdTest, testAgainst);
   }

   objActor->SetUsesInsideModel(true);
   CPPUNIT_ASSERT(objActor->GetUsesInsideModel());

   objActor->SetRotationOffSet(osg::Vec3(0,0,20));
   CPPUNIT_ASSERT(objActor->GetRotationOffSet() == osg::Vec3(0,0,20));

   objActor->SetSeatPosition(osg::Vec3(10,10,20));
   CPPUNIT_ASSERT(objActor->GetSeatPosition() == osg::Vec3(10,10,20));
}
