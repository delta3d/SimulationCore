/* -*-c++-*-
* Simulation Core - NxAgeiaTests (.h & .cpp) - Using 'The MIT License'
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
* @author Allen Danklefsen
*/
//#ifdef AGEIA_PHYSICS_ 1
#include <prefix/SimCorePrefix-src.h>
#ifdef AGEIA_PHYSICS

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
#include <NxAgeiaPhysicsHelper.h>
#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaMaterialActor.h>
#include <NxAgeiaActorRegistry.h>

#include <dtUtil/log.h>
#include <dtDAL/map.h>
#include <dtGame/message.h>
#include <dtGame/basemessages.h>

#include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
#include <SimCore/Actors/PlatformWithPhysics.h>
#include <SimCore/Actors/NxAgeiaParticleSystemActor.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

static const std::string AGEIA_REGISTRY = "dtPhysX";

using namespace dtAgeiaPhysX;


class NxAgeiaTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(NxAgeiaTests);
      CPPUNIT_TEST(TestFunction);
      CPPUNIT_TEST(TestParticleSystemPerformance);
   CPPUNIT_TEST_SUITE_END();

   public:
      NxAgeiaTests() {}
      ~NxAgeiaTests() {}

      void setUp();
      void tearDown();

      void TestFunction();
      void TestParticleSystemPerformance();

   private:
     dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaWorldComponent> worldcomp;
     dtCore::RefPtr<dtGame::GameManager> mGM;
     dtCore::RefPtr<dtUtil::Log> mLogger;

};

CPPUNIT_TEST_SUITE_REGISTRATION(NxAgeiaTests);


/////////////////////////////////////////////////////////
void NxAgeiaTests::setUp()
{
   mLogger = &dtUtil::Log::GetInstance("NAgeiaTests.cpp");

   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();

   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   mGM->LoadActorRegistry(AGEIA_REGISTRY);
   mGM->SetApplication(GetGlobalApplication());

   dtCore::System::GetInstance().Step();

   SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());

   worldcomp = new NxAgeiaWorldComponent();
   mGM->AddComponent(*worldcomp, dtGame::GameManager::ComponentPriority::NORMAL);
   dtCore::System::GetInstance().Step();
}

/////////////////////////////////////////////////////////
void NxAgeiaTests::tearDown()
{
   dtCore::System::GetInstance().Stop();

   worldcomp = NULL;

   mGM->DeleteAllActors(true);
   mGM->UnloadActorRegistry(AGEIA_REGISTRY);
   mGM = NULL;
}

/////////////////////////////////////////////////////////
void NxAgeiaTests::TestParticleSystemPerformance()
{
   typedef std::vector<dtCore::RefPtr<dtDAL::ActorProxy> > ProxyContainer;
   ProxyContainer proxies;

   //load map
   try
   {
      mGM->ChangeMap("AgeiaParticleTest");

      // 3 ticks to be safe to make sure the map has completed it's 3 step process.
      dtCore::System::GetInstance().Step();
      dtCore::System::GetInstance().Step();
      dtCore::System::GetInstance().Step();

      dtDAL::Project::GetInstance().GetMap("AgeiaParticleTest").FindProxies(proxies, "AgeiaParticleSystem");
   }
   catch (dtUtil::Exception &e)
   {
      CPPUNIT_FAIL(e.ToString());
   }

   mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__, "Performance testing AgeaiParticles with " + dtUtil::ToString(proxies.size()) + " particle actors.");

   std::vector<NxAgeiaParticleSystemActor*> particleActors;

   // Setup and find particle actors
   ProxyContainer::iterator iter, end;
   for(iter = proxies.begin(), end = proxies.end(); iter != end; ++iter)
   {
      NxAgeiaParticleSystemActorProxy* gameProxy = dynamic_cast<NxAgeiaParticleSystemActorProxy*>((*iter).get());
      if(gameProxy != NULL)
      {
         dtCore::RefPtr<dtDAL::ActorProxy> newPrototypeActor = mGM->CreateActorFromPrototype(gameProxy->GetId());
         if(newPrototypeActor.valid())
         {
            NxAgeiaParticleSystemActorProxy* newParticle =
               static_cast<NxAgeiaParticleSystemActorProxy*>(newPrototypeActor.get());

            NxAgeiaParticleSystemActor* newPartActor =
               static_cast<NxAgeiaParticleSystemActor*>(newParticle->GetActor());

            newParticle->SetInitialOwnership(dtGame::GameActorProxy::Ownership::CLIENT_LOCAL);
            newPartActor->ToggleEmitter(true);
            mGM->AddActor(*newParticle, false, false);
            particleActors.push_back(newPartActor);
         }
      }
   }


   //lets do some performance testing
   mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__, "Done loading and setup. Now testing performance of on Ageia Particle Systems.");

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   dtCore::Timer timer;
   dtCore::Timer_t timerStart, timerEnd;

   timerStart = timer.Tick();

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   int numUpdates = 1200;
   float updateTime = 1.0f / 60.0f;
   dtCore::RefPtr<dtGame::Message> message = mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::TICK_LOCAL);
   dtGame::TickMessage *tickMessage = (dtGame::TickMessage *) message.get();

   tickMessage->SetDeltaSimTime(updateTime);
   tickMessage->SetDeltaRealTime(updateTime);

   int listSize = particleActors.size();
   mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__, "  Testing AGEIA particles.  We have [" +
      dtUtil::ToString(listSize) + "], doing [" + dtUtil::ToString(numUpdates) + "] update loops.");

   timerStart = timer.Tick();
   for(int i = 0; i < numUpdates; ++i)
   {
      worldcomp->ProcessMessage(*message.get());

      // Loop through the particle systems and tick them too
      for(int j = 0; j < listSize; j ++)
      {
         NxAgeiaParticleSystemActor* partActor = particleActors[j];
         if (partActor != NULL)
            partActor->OnTickLocal(*tickMessage);
      }
   }
   timerEnd = timer.Tick();

   mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,
      "   Time Results for Update: " + dtUtil::ToString(timer.DeltaSec(timerStart, timerEnd)) + " sec.");
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}


/////////////////////////////////////////////////////////
void NxAgeiaTests::TestFunction()
{
   dtGame::GMComponent* comptest = mGM->GetComponentByName("NxAgeiaWorldComponent");
   CPPUNIT_ASSERT_MESSAGE("Ageia World Component Did not get initialized correctly",  comptest != NULL);

   NxAgeiaWorldComponent* component = dynamic_cast<NxAgeiaWorldComponent*>(comptest);
   NxScene* tehscene = &component->GetPhysicsScene("Default");

   CPPUNIT_ASSERT_MESSAGE("Default scene not initialized", (tehscene != NULL));

   const std::string &SceneNameOne = "LollerzSkates";
   const std::string &SceneNameTwo = "TehMomWhichIsTeaguez";
   const std::string &SceneNameThr = "RoflCopterz";
   NxSceneDesc desc;
   component->CreateScene(SceneNameOne, desc, false);
   component->CreateScene(SceneNameTwo, desc, false);
   component->CreateScene(SceneNameThr, desc, false);

   //component->CalculatePhysics(1/60.0f);

   tehscene = &component->GetPhysicsScene(SceneNameOne);
   CPPUNIT_ASSERT_MESSAGE("SceneNameOne scene not initialized", (tehscene != NULL));
   tehscene = &component->GetPhysicsScene(SceneNameTwo);
   CPPUNIT_ASSERT_MESSAGE("SceneNameTwo scene not initialized", (tehscene != NULL));
   tehscene = &component->GetPhysicsScene(SceneNameThr);
   CPPUNIT_ASSERT_MESSAGE("SceneNameThr scene not initialized", (tehscene != NULL));

   dtCore::RefPtr<dtGame::GameActorProxy> obj;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_WITH_PHYSICS_ACTOR_TYPE, obj);
   dtCore::RefPtr<SimCore::Actors::PlatformWithPhysics> objActor = dynamic_cast<SimCore::Actors::PlatformWithPhysics*>(obj->GetActor());

   objActor->GetPhysicsHelper()->SetAgeiaFlags(AGEIA_FLAGS_PRE_UPDATE);
   CPPUNIT_ASSERT_MESSAGE("Flags should have been !& with collision callback flag", (AGEIA_FLAGS_PRE_UPDATE & objActor->GetPhysicsHelper()->GetAgeiaFlags()) == true);

   dtCore::RefPtr<NxAgeiaMaterialActorProxy> mat;
   mGM->CreateActor(*dtAgeiaPhysX::NxAgeiaActorRegistry::PHYSX_MATERIAL_ACTOR_TYPE, mat);
   CPPUNIT_ASSERT(mat.valid());
   dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaMaterialActor> matActor = dynamic_cast<dtAgeiaPhysX::NxAgeiaMaterialActor*>(mat->GetActor());
   CPPUNIT_ASSERT_MESSAGE("Failed to create material ageia actor", matActor != NULL);

   component->DeleteScene(SceneNameThr);
   tehscene = &component->GetPhysicsScene(SceneNameThr);
   NxScene* defaultScene = &component->GetPhysicsScene("Default");

   if(tehscene != defaultScene)
   {
      printf("SceneNameThr should be the default scene and its not.\n");
   }
   //CPPUNIT_ASSERT_MESSAGE("SceneNameThr should be the default scene and its not.", (tehscene != defaultScene ));

   const std::string &TeaguesMomMaterialName = "FluffyBunnyMaterial";
   CPPUNIT_ASSERT_MESSAGE("Material that should have been created was not", false != component->RegisterMaterial(TeaguesMomMaterialName, 0.5, .77, .77) );
   CPPUNIT_ASSERT_MESSAGE("Material index that was just made should have been indexed at 1", 0 != component->GetMaterialIndex(TeaguesMomMaterialName) );
   CPPUNIT_ASSERT_MESSAGE("Tried recreating a material that was already registered. Should have returned true.", false != component->RegisterMaterial(TeaguesMomMaterialName, 0.5, .77, .77) );
   CPPUNIT_ASSERT_MESSAGE("Tried created a material into a scene that doesnt exist, should have returned false.", true != component->RegisterMaterial(TeaguesMomMaterialName, 0.5, .77, .77, "TheBunnyScene"));
   CPPUNIT_ASSERT_MESSAGE("Material index from a value we know doenst exist, should be 0", 0 == component->GetMaterialIndex(TeaguesMomMaterialName,"TheBunnyScene") );

   NxVec3 gravity(0, 6, 4);

   NxVec3 actualGravity = component->GetGravity();
   CPPUNIT_ASSERT_MESSAGE("By default, gravity should be -9.8", actualGravity != gravity);

   component->SetGravity(gravity);

   actualGravity = component->GetGravity();
   CPPUNIT_ASSERT_MESSAGE("Setting the gravity should have worked properly", actualGravity == gravity);

   component->AddCachedMesh("aCachedMesh_aghhhh_So_Pretty", true, NULL, NULL);
   NxAgeiaWorldComponent::CachedMesh* aMesh = component->GetCachedMesh("aCachedMesh_aghhhh_So_Pretty");
   CPPUNIT_ASSERT_MESSAGE("Couldnt not find the cachedMesh that should have been added", aMesh != NULL);

   /////////////////////////////////////////////////////////////////////////
   // Additional Unit tests that can be done. Or Runtime Tests checks to do
   /*
      // To be done.
      TestReleaseAllAgeiaHelpers() - Works, no unit test though :(
      Try to delete a scene with objects in it - test for those objects then.

      ///////////////////////////
      // No way to test currently
      Try To make a scene with same name - no way of testing currently.

      /////////////////////////
      // Subclassing.
      Register An Ageia Helper Twice - Is handled. no unit test without subclassing though
      Remove An Ageia Helper - Is handled. no unit test without subclassing though

      ////////////////////////
      // External Tests.
      Test Making multiple names. - Make a rope or StacksActor.
      Test Making multiple joints. - Make a RopeActor.
      Test Remove Joint. - Rope Sim, make then just delete a joint after creation. Is handled.
      Test Caching models
      Test loading static world geometry and making sure its static.
      Test setting something to kinematic and making sure it works
   */
   ///////////////////////////////////////////////////////////////
}
#endif
