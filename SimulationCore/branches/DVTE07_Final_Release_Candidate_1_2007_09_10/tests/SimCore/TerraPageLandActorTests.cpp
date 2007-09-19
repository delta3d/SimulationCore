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

#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtABC/application.h>
#include <SimCore/AgeiaTerrainCullVisitor.h>

class TerraPageLandActorTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(TerraPageLandActorTests);
      CPPUNIT_TEST(TestFunction);
   CPPUNIT_TEST_SUITE_END();

   public:
      TerraPageLandActorTests() {}
      ~TerraPageLandActorTests() {}

      void setUp();
      void tearDown();

      void TestFunction();

   private:
     dtCore::RefPtr<dtGame::GameManager> mGM;
     dtCore::RefPtr<SimCore::Components::RenderingSupportComponent> renderingSupportComponent;
     dtCore::RefPtr<dtABC::Application> mApplication;
     dtCore::RefPtr<dtUtil::Log> mLogger;

     dtCore::RefPtr<dtCore::Scene> mScene;
     dtCore::RefPtr<dtCore::Camera> mCamera;
     dtCore::RefPtr<dtCore::DeltaWin> mWin;

};

CPPUNIT_TEST_SUITE_REGISTRATION(TerraPageLandActorTests);

/////////////////////////////////////////////////////////
void TerraPageLandActorTests::setUp()
{
   mLogger = &dtUtil::Log::GetInstance("TerraPageLandActorTests.cpp");

   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();
   
   mApplication = new dtABC::Application("config.xml");

   mScene = new dtCore::Scene();
   mWin = new dtCore::DeltaWin();
   mWin->SetPosition(0, 0, 50, 50);
   mCamera = new dtCore::Camera();
   mCamera->SetScene(mScene.get());
   mCamera->SetWindow(mWin.get());
   dtCore::System::GetInstance().Config();
  
   mGM = new dtGame::GameManager(*mScene.get());
   mGM->SetApplication(*mApplication.get());
   dtCore::System::GetInstance().Step();

   SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());

   dtCore::System::GetInstance().Step();
}

/////////////////////////////////////////////////////////
void TerraPageLandActorTests::tearDown()
{
  
   dtCore::System::GetInstance().Stop();

   mGM->GetApplication().Quit();
   mGM->DeleteAllActors(true);
   mGM = NULL;

   mApplication = NULL;
}

/////////////////////////////////////////////////////////
void TerraPageLandActorTests::TestFunction()
{
   renderingSupportComponent = new SimCore::Components::RenderingSupportComponent();
   CPPUNIT_ASSERT(renderingSupportComponent->UpdateCullVisitor() == false);

   renderingSupportComponent->SetEnableCullVisitor(true);
   mGM->AddComponent(*renderingSupportComponent, dtGame::GameManager::ComponentPriority::NORMAL);

   dtCore::System::GetInstance().Step();

   dtCore::RefPtr<dtGame::GameActorProxy> obj;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::AGEIA_TLAND_ACTOR_TYPE, obj);
   dtCore::RefPtr<NxAgeiaTerraPageLandActor> objActor = dynamic_cast<NxAgeiaTerraPageLandActor*>(obj->GetActor());

   CPPUNIT_ASSERT(objActor.valid());

   dtCore::System::GetInstance().Step();

   // Actor
   NxAgeiaTerraPageLandActor* ourActor = objActor.get();
   CPPUNIT_ASSERT(ourActor->DEFAULT_NAME == "PhysX Terra Page Listener");
#ifdef AGEIA_PHYSICS
   CPPUNIT_ASSERT(ourActor->GetPhysicsHelper() != NULL);
   CPPUNIT_ASSERT(ourActor->PassThisGeometry(957,0,0,0) == false);
   CPPUNIT_ASSERT(ourActor->PassThisGeometry(1,0,0,0) == true);
   CPPUNIT_ASSERT(ourActor->HasSomethingBeenLoaded() == false);
   CPPUNIT_ASSERT(ourActor->FinalizeTerrain(1) == false);
#endif

   // component
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME == "RenderingSupportComponent");
   CPPUNIT_ASSERT(renderingSupportComponent->UpdateCullVisitor() == true);

   // this will remind the user they probably shouldnt have changed the values 
   // unless they know the consequences
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::RENDER_BIN_ENVIRONMENT        == -2);
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::RENDER_BIN_SKY_AND_ATMOSPHERE ==  9);
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::RENDER_BIN_PRECIPITATION      == 11);
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::RENDER_BIN_TRANSPARENT        == 10);
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HWMVV_INTERIOR     == 15);
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD                == 20);
   CPPUNIT_ASSERT(SimCore::Components::RenderingSupportComponent::RENDER_BIN_MINIMAP            == 25);


   // cull visitor
   dtCore::RefPtr<SimCore::AgeiaTerrainCullVisitor> mCullVisitor = new SimCore::AgeiaTerrainCullVisitor();
   mCullVisitor->SetLandActor(ourActor);
   CPPUNIT_ASSERT(mCullVisitor->GetLandActor() == ourActor);

   mCullVisitor->SetCameraTransform(osg::Vec3(0,0,1));
   CPPUNIT_ASSERT(mCullVisitor->GetCameraTransform() == osg::Vec3(0,0,1));

   dtCore::RefPtr<osg::Transform> transform = new osg::Transform();
   mCullVisitor->SetTerrainNode(transform.get());
   CPPUNIT_ASSERT(mCullVisitor->GetTerrainNode() == transform.get());

   //TerrainHashTable* hashTable = NULL;
   //hashTable = new TerrainHashTable();
   //
   //// make sure hash function returns the same when it should
   //CPPUNIT_ASSERT(hashTable->Terrain_Hash_Function(123,112) == hashTable->Terrain_Hash_Function(123,112));
   //
   //// make sure hash function returns different when it should
   //CPPUNIT_ASSERT(hashTable->Terrain_Hash_Function(123,122) != hashTable->Terrain_Hash_Function(123,112));

   //// bucket size should be 3, this gives us our 4 lod range, (dont count tile0 just starts from subtile0)
   //CPPUNIT_ASSERT(3 == hashTable->GetHashTableBucketAmount());

   //// let's fill out our hash table
   //TerrainNode* nodeTest = new TerrainNode(NULL, "testNode/subtiles0_12x12_0.txp");

   //// make sure name got copied over correctly
   //CPPUNIT_ASSERT(nodeTest->GetNameIdentifier() == "testNode/subtiles0_12x12_0.txp");
   //
   //// make sure osg's simple name thing works like i think
   //CPPUNIT_ASSERT(nodeTest->GetSimpleNameIdent() == "subtiles0_12x12_0.txp");

   //// should be false at start for default setting
   //CPPUNIT_ASSERT(nodeTest->IsFilled() == false);

   //// make sure location were pulled out correctly
   //CPPUNIT_ASSERT(nodeTest->GetXLocation() == 12);

   //// make sure location were pulled out correctly
   //CPPUNIT_ASSERT(nodeTest->GetYLocation() == 12);
   //
   //// lod should be 0.....
   //CPPUNIT_ASSERT(nodeTest->GetLOD() == 0);

   //// hash code should be 0....
   //CPPUNIT_ASSERT(nodeTest->GetHashCode() == 0);
   //
   //hashTable->InsertNode(nodeTest);

   //// get bucket for testing
   //int bucket = nodeTest->GetHashCode() % hashTable->GetHashTableBucketAmount(); 

   //// hash code should have been set
   //CPPUNIT_ASSERT(nodeTest->GetHashCode() != 0);

   //// just make sure it got set
   //CPPUNIT_ASSERT(bucket  > -1 && bucket < hashTable->GetHashTableBucketAmount());

   //// should be set to NULL
   //CPPUNIT_ASSERT(nodeTest->GetNextNode() == NULL);

   //// make sure we can get back to that node....
   //TerrainNode * newNode = NULL;

   //newNode = hashTable->GetBucket(12, 12);

   //CPPUNIT_ASSERT(newNode != NULL);

   //// this should return the same exact node..
   //CPPUNIT_ASSERT(hashTable->GetBucket(12,12) == hashTable->GetBucket(bucket));

   //// lets try removing it...
   //// returns previous which in this case should be NULL!
   //CPPUNIT_ASSERT(hashTable->RemoveNode(nodeTest) == NULL);

   //nodeTest = new TerrainNode(NULL, "testNode/subtiles0_125x125_0.txp");
   //hashTable->InsertNode(nodeTest);

   //// do starting memory check here
   //for(int i = 0 ; i < 100; ++i)
   //{
   //   char buffer[256];
   //   sprintf(buffer, "testNode/subtiles%d_%dx%d_0.txp", rand() % 3, rand() % 100, rand() % 100);
   //   nodeTest = new TerrainNode(NULL, buffer);
   //   hashTable->InsertNode(nodeTest);
   //}

   //int sizeOne = hashTable->GetBucket(0)->GetSize();
   //int sizeTwo = hashTable->GetBucket(1)->GetSize();
   //int sizeThr = hashTable->GetBucket(2)->GetSize();

   //CPPUNIT_ASSERT(102 == sizeOne + sizeThr + sizeTwo);

   //// do a check to make sure all 3 buckets have something in them... this is not a fail proof test
   //// it should almost always pass....unless the random number generator was hax0red
   //CPPUNIT_ASSERT(hashTable->GetBucket(0) != NULL && 
   //               hashTable->GetBucket(1) != NULL && 
   //               hashTable->GetBucket(2) != NULL);

   //// should be able to find it
   //newNode = hashTable->GetFullNameNode("testNode/subtiles0_125x125_0.txp");

   //// lets see if its valid
   //CPPUNIT_ASSERT(newNode != NULL);

   //// lets see if its valid
   //CPPUNIT_ASSERT(newNode == hashTable->GetSimpleNameNode("subtiles0_125x125_0.txp"));
  
   //// hopefully valid.
   //newNode = hashTable->GetNode(125,125);
   //CPPUNIT_ASSERT(newNode != NULL);

   //// should not return null since it was put in the list like first.....
   //CPPUNIT_ASSERT(hashTable->RemoveNode(newNode) != NULL);

   //newNode = NULL;

   //delete hashTable;
   // end memory check
   // check again...
}
