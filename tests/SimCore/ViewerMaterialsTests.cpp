/* -*-c++-*-
* Simulation Core - ViewerMaterialsTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, Alion Science and Technology Corporation
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
#include <dtCore/project.h>
#include <dtCore/datatype.h>
#include <dtGame/gamemanager.h> 

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <string>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <dtGame/messagetype.h>
#include <dtGame/messagefactory.h>
#include <dtGame/basemessages.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/ViewerMaterialActor.h>
#include <SimCore/Components/ViewerMaterialComponent.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

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

         mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
              
         dtCore::System::GetInstance().Step();

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
   
   /*SimCore::Actors::ViewerMaterialActor& viewMaterial = */
   mMaterialComponent->CreateOrChangeMaterialByFID(100);
   
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
