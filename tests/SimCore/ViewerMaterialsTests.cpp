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

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/ViewerMaterialActor.h>
#include <SimCore/Components/ViewerMaterialComponent.h>

class ViewerMaterialsTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(ViewerMaterialsTests);
      CPPUNIT_TEST(TestFunction);
   CPPUNIT_TEST_SUITE_END();

   public:
      ViewerMaterialsTests()
      {
      }
      ~ViewerMaterialsTests()
      {
      }

      void setUp()
      {
         dtCore::System::GetInstance().Start();

         mGM = new dtGame::GameManager(*new dtCore::Scene());
              
         dtCore::System::GetInstance().Step();

         SimCore::MessageType::RegisterMessageTypes(mGM->GetMessageFactory());

         mMaterialComponent = new SimCore::Components::ViewerMaterialComponent();
         mGM->AddComponent(*mMaterialComponent, dtGame::GameManager::ComponentPriority::NORMAL);

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
      dtCore::RefPtr<SimCore::Components::ViewerMaterialComponent> mMaterialComponent;
     dtCore::RefPtr<dtGame::GameManager> mGM;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ViewerMaterialsTests);
   
void ViewerMaterialsTests::TestFunction()
{
   CPPUNIT_ASSERT_MESSAGE("Material Component not initialized", (mMaterialComponent != NULL));
   
   SimCore::Actors::ViewerMaterialActor& viewMaterial = mMaterialComponent->CreateOrChangeMaterialByFID(100);
   viewMaterial.SetBumpiness(1.0f);
   CPPUNIT_ASSERT_MESSAGE("Viewer material unable to be set 0.o", viewMaterial.GetBumpiness() == 1.0f);
   
   dtCore::RefPtr<dtGame::Message> reflkjasdfo;
   mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_MAP_LOADED, reflkjasdfo);
   mGM->SendMessage(*reflkjasdfo.get());
   dtCore::System::GetInstance().Step();

   //const ViewerMaterialActor& anotherMaterial1 = mMaterialComponent->GetConstMaterialByFID(100);
   //anotherMaterial1.SetBumpiness(1.0f);
   //CPPUNIT_ASSERT_MESSAGE("Returned const material and tried setting, if it let me set it, its wrong.", viewMaterial.GetBumpiness() == 1.0f);

   const SimCore::Actors::ViewerMaterialActor& anotherMaterial2 = mMaterialComponent->GetConstMaterialByName("hummagass");
   CPPUNIT_ASSERT_MESSAGE("Should have returned the default material", anotherMaterial2.GetName() == "DefaultMaterial");
}
