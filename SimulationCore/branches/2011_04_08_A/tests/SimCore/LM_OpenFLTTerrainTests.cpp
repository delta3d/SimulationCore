/* -*-c++-*-
* Simulation Core - LM_OpenFLTTerrainTests (.h & .cpp) - Using 'The MIT License'
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

#include <SimCore/Actors/OpenFlightToIVETerrain.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

class LM_OpenFLTTerainTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(LM_OpenFLTTerainTests);
      CPPUNIT_TEST(TestFunction);
   CPPUNIT_TEST_SUITE_END();

   public:
      LM_OpenFLTTerainTests() {}
      ~LM_OpenFLTTerainTests() {}

      void setUp();
      void tearDown();

      void TestFunction();

   private:
     dtCore::RefPtr<dtGame::GameManager> mGM;
     dtCore::RefPtr<dtUtil::Log> mLogger;

};

CPPUNIT_TEST_SUITE_REGISTRATION(LM_OpenFLTTerainTests);

/////////////////////////////////////////////////////////
void LM_OpenFLTTerainTests::setUp()
{
   mLogger = &dtUtil::Log::GetInstance("LM_OpenFLTTerainTests.cpp");

   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();

   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

   dtCore::System::GetInstance().Step();

   dtCore::System::GetInstance().Step();
}

/////////////////////////////////////////////////////////
void LM_OpenFLTTerainTests::tearDown()
{
   dtCore::System::GetInstance().Stop();

   mGM->DeleteAllActors(true);
   mGM = NULL;
}

/////////////////////////////////////////////////////////
void LM_OpenFLTTerainTests::TestFunction()
{
   //////////////////////////////////////////////////////////////////////////
   dtCore::RefPtr<dtGame::GameActorProxy> obj;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::LM_OPENFLIGHT_TERRAIN_ACTORTYPE, obj);
   //////////////////////////////////////////////////////////////////////////

   CPPUNIT_ASSERT(obj->GetHideDTCorePhysicsProps());
   CPPUNIT_ASSERT_MESSAGE("dtCore physics properties should be hidden.", obj->GetProperty("Show Collision Geometry") == NULL);

   dtCore::RefPtr<SimCore::Actors::OpenFlightToIVETerrainActor> objActor = dynamic_cast<SimCore::Actors::OpenFlightToIVETerrainActor*>(obj->GetActor());

   objActor->SetPagingMinX(1.5);
   CPPUNIT_ASSERT(objActor->GetPagingMinX() == 1.5);

   objActor->SetPagingMinY(32);
   CPPUNIT_ASSERT(objActor->GetPagingMinY() == 32);

   objActor->SetPagingMaxX(2);
   CPPUNIT_ASSERT(objActor->GetPagingMaxX() == 2);

   objActor->SetPagingMaxY(13);
   CPPUNIT_ASSERT(objActor->GetPagingMaxY() == 13);

   objActor->SetPagingDelta(56);
   CPPUNIT_ASSERT(objActor->GetPagingDelta() == 56);

   objActor->SetPagingRadius(7);
   CPPUNIT_ASSERT(objActor->GetPagingRadius() == 7);

   objActor->SetPagingRange(8);
   CPPUNIT_ASSERT(objActor->GetPagingRange() == 8);

   objActor->SetPagingBaseName("DOG");
   CPPUNIT_ASSERT(objActor->GetPagingBaseName() == "DOG");

   objActor->SetPagingExpiringDelay(9);
   CPPUNIT_ASSERT(objActor->GetPagingExpiringDelay() == 9);

   objActor->SetPagingFPSTarget(11);
   CPPUNIT_ASSERT(objActor->GetPagingFPSTarget() == 11);

   objActor->SetPagingPrecompile(true);
   CPPUNIT_ASSERT(objActor->GetPagingPrecompile() == true);

   objActor->SetMaxObjectsToCompile(754);
   CPPUNIT_ASSERT(objActor->GetMaxObjectsToCompile() == 754);

   objActor->SetZOffsetforTerrain(312);
   CPPUNIT_ASSERT(objActor->GetZOffsetforTerrain() == 312);

   objActor->SetTerrainPath("URMOMSTERRAIN");
   CPPUNIT_ASSERT(objActor->GetTerrainPath() == "URMOMSTERRAIN");
}
