/* -*-c++-*-
* Simulation Core - ArticulationHelperTests (.h & .cpp) - Using 'The MIT License'
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
* @author Chris Rodgers
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtGame/gamemanager.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/CamoConfigActor.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/ActComps/CamoPaintStateActComp.h>
#include <UnitTestMain.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // TESTS OBJECT
      //////////////////////////////////////////////////////////////////////////
      class CamoConfigActorTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(CamoConfigActorTests);
         CPPUNIT_TEST(TestCamoParamsProperties);
         CPPUNIT_TEST(TestActorProperties);
         CPPUNIT_TEST(TestCamoParamsManagement);
         CPPUNIT_TEST(TestCamoConfigLoading);
         CPPUNIT_TEST(TestCamoSettingOnActor);
         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            // Test Methods
            void TestCamoParamsProperties();
            void TestActorProperties();
            void TestCamoParamsManagement();
            void TestCamoConfigLoading();
            void TestCamoSettingOnActor();

         private:
            std::shared_ptr<dtGame::GameManager> mGM;
            std::shared_ptr<CamoConfigActorProxy> mProxy;
            CamoConfigActor* mActor;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(CamoConfigActorTests);

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorTests::setUp()
      {
         try
         {
            // Create the Game Manager.
            mGM = new dtGame::GameManager( *GetGlobalApplication().GetScene() );
            mGM->SetApplication(GetGlobalApplication());

            // Create a proxy.
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::CAMO_CONFIG_ACTOR_TYPE, mProxy);

            // Get the actor that was created.
            mProxy->GetActor(mActor);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorTests::tearDown()
      {
         try
         {
            mActor = nullptr;
            mProxy = nullptr;

            mGM->DeleteAllActors(true);
            mGM = nullptr;
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorTests::TestCamoParamsProperties()
      {
         try
         {
            std::shared_ptr<CamoParams> params = new CamoParams;
            const CamoParams& constParams = *params;

            CPPUNIT_ASSERT(constParams.GetName().empty());
            params->SetName("TestName");
            CPPUNIT_ASSERT(constParams.GetName() == "TestName");

            CPPUNIT_ASSERT(constParams.GetId() == 0);
            params->SetId(12);
            CPPUNIT_ASSERT(constParams.GetId() == 12);

            CPPUNIT_ASSERT(constParams.GetPatternScale() == 1.0f);
            params->SetPatternScale(5.0f);
            CPPUNIT_ASSERT(constParams.GetPatternScale() == 5.0f);

            const osg::Vec4 ZERO_V4;
            osg::Vec4 testColor(12.34f, -56.78f, 9.0f, -10.11f);
            CPPUNIT_ASSERT(constParams.GetColor1() == ZERO_V4);
            params->SetColor1(testColor);
            CPPUNIT_ASSERT(constParams.GetColor1() == testColor);

            CPPUNIT_ASSERT(constParams.GetColor2() == ZERO_V4);
            params->SetColor2(testColor);
            CPPUNIT_ASSERT(constParams.GetColor2() == testColor);

            CPPUNIT_ASSERT(constParams.GetColor3() == ZERO_V4);
            params->SetColor3(testColor);
            CPPUNIT_ASSERT(constParams.GetColor3() == testColor);

            CPPUNIT_ASSERT(constParams.GetColor4() == ZERO_V4);
            params->SetColor4(testColor);
            CPPUNIT_ASSERT(constParams.GetColor4() == testColor);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }
      
      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorTests::TestActorProperties()
      {
         try
         {
            const CamoConfigActor& constActor = *mActor;
            std::string filePath = "Configs/CamoConfig.xml";

            CPPUNIT_ASSERT(constActor.GetConfigFile().empty());
            mActor->SetConfigFile(filePath);
            CPPUNIT_ASSERT(constActor.GetConfigFile() == filePath);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      // Helper function for the subsequent unit test method.
      bool IsCamoInList(const CamoParams& camo, const CamoParamsList& list)
      {
         bool success = false;
         
         CamoParamsList::const_iterator curIter = list.begin();
         CamoParamsList::const_iterator endIter = list.end();
         for( ; curIter != endIter; ++curIter)
         {
            if(curIter->get() == &camo)
            {
               success = true;
               break;
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      // Helper function for the subsequent unit test method.
      CamoParams* CreateCamo(const std::string& name, CamoParams::CamoId id)
      {
         CamoParams* camo = new CamoParams;
         camo->SetName(name);
         camo->SetId(id);
         return camo;
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorTests::TestCamoParamsManagement()
      {
         try
         {
            CamoParams::CamoId camoId1 = 0;
            CamoParams::CamoId camoId2 = 5;
            CamoParams::CamoId camoId3 = 10;
            CamoParams::CamoId camoId4 = 20;
            std::string camoName1("Test Camo 123");
            std::string camoName2("Test Camo ABC");
            std::string camoName3("Test Camo 456");
            std::string camoName4("Test Camo XYZ");
            std::shared_ptr<CamoParams> camo1 = CreateCamo(camoName1, camoId1);
            std::shared_ptr<CamoParams> camo2 = CreateCamo(camoName2, camoId2);
            std::shared_ptr<CamoParams> camo3 = CreateCamo(camoName3, camoId3);
            std::shared_ptr<CamoParams> camo4 = CreateCamo(camoName4, camoId4);

            CamoParamsList camoList;
            CPPUNIT_ASSERT(mActor->GetCamoParamsCount() == 0);
            CPPUNIT_ASSERT(mActor->GetCamoParamsList(camoList) == 0);
            CPPUNIT_ASSERT(camoList.empty());

            CPPUNIT_ASSERT(mActor->AddCamoParams(*camo1));
            CPPUNIT_ASSERT(mActor->AddCamoParams(*camo2));
            CPPUNIT_ASSERT(mActor->AddCamoParams(*camo3));
            CPPUNIT_ASSERT(mActor->AddCamoParams(*camo4));
            // --- Test reinserting the same camo objects.
            CPPUNIT_ASSERT( ! mActor->AddCamoParams(*camo1));
            CPPUNIT_ASSERT( ! mActor->AddCamoParams(*camo2));
            CPPUNIT_ASSERT( ! mActor->AddCamoParams(*camo3));
            CPPUNIT_ASSERT( ! mActor->AddCamoParams(*camo4));

            CPPUNIT_ASSERT(mActor->GetCamoParamsCount() == 4);
            CPPUNIT_ASSERT(mActor->GetCamoParamsList(camoList) == 4);
            CPPUNIT_ASSERT(camoList.size() == 4);

            // Test accessing the contained objects.
            CPPUNIT_ASSERT(camo1.get() == mActor->GetCamoParamsByCamoId(camoId1));
            CPPUNIT_ASSERT(camo2.get() == mActor->GetCamoParamsByCamoId(camoId2));
            CPPUNIT_ASSERT(camo3.get() == mActor->GetCamoParamsByCamoId(camoId3));
            CPPUNIT_ASSERT(camo4.get() == mActor->GetCamoParamsByCamoId(camoId4));
            // --- Test that the objects can be found by name as well.
            CPPUNIT_ASSERT(camo1.get() == mActor->GetCamoParamsByName(camoName1));
            CPPUNIT_ASSERT(camo2.get() == mActor->GetCamoParamsByName(camoName2));
            CPPUNIT_ASSERT(camo3.get() == mActor->GetCamoParamsByName(camoName3));
            CPPUNIT_ASSERT(camo4.get() == mActor->GetCamoParamsByName(camoName4));
            // --- Test that all the camo objects were returned in the list from earlier.
            CPPUNIT_ASSERT(IsCamoInList(*camo1, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo2, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo3, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo4, camoList));

            // Test removing some objects.
            CPPUNIT_ASSERT(mActor->RemoveCamoParams(*camo1));
            CPPUNIT_ASSERT(mActor->RemoveCamoParams(*camo3));

            camoList.clear();
            CPPUNIT_ASSERT(mActor->GetCamoParamsCount() == 2);
            CPPUNIT_ASSERT(mActor->GetCamoParamsList(camoList) == 2);
            CPPUNIT_ASSERT(camoList.size() == 2);

            // --- Test removing the same ones that were just removed
            CPPUNIT_ASSERT(mActor->GetCamoParamsByCamoId(camoId1) == nullptr);
            CPPUNIT_ASSERT(mActor->GetCamoParamsByCamoId(camoId3) == nullptr);
            CPPUNIT_ASSERT( ! mActor->RemoveCamoParams(*camo1));
            CPPUNIT_ASSERT( ! mActor->RemoveCamoParams(*camo3));

            // --- Ensure the other objects can still be accessed.
            CPPUNIT_ASSERT(mActor->GetCamoParamsByCamoId(camoId2) == camo2.get());
            CPPUNIT_ASSERT(mActor->GetCamoParamsByCamoId(camoId4) == camo4.get());

            camoList.clear();
            CPPUNIT_ASSERT(mActor->GetCamoParamsCount() == 2);
            CPPUNIT_ASSERT(mActor->GetCamoParamsList(camoList) == 2);
            CPPUNIT_ASSERT(camoList.size() == 2);

            CPPUNIT_ASSERT( ! IsCamoInList(*camo1, camoList));
            CPPUNIT_ASSERT( ! IsCamoInList(*camo3, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo2, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo4, camoList));

            // --- Remove the remaining objects.
            CPPUNIT_ASSERT(mActor->RemoveCamoParams(*camo2));
            CPPUNIT_ASSERT(mActor->RemoveCamoParams(*camo4));

            camoList.clear();
            CPPUNIT_ASSERT(mActor->GetCamoParamsCount() == 0);
            CPPUNIT_ASSERT(mActor->GetCamoParamsList(camoList) == 0);
            CPPUNIT_ASSERT(camoList.empty());

         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorTests::TestCamoConfigLoading()
      {
         try
         {
            // --- Set the file to load.
            mActor->SetConfigFile("Configs/CamoConfig.xml");

            CamoParamsList camoList;
            CPPUNIT_ASSERT(mActor->GetCamoParamsCount() == 5);
            CPPUNIT_ASSERT(mActor->GetCamoParamsList(camoList) == 5);
            CPPUNIT_ASSERT(camoList.size() == 5);

            // Test accessing the camo parameter objects.
            std::string camoName1("Test");
            std::string camoName2("Desert");
            std::string camoName3("Forest");
            std::string camoName4("Tundra");
            std::string camoName5("Test2");
            const CamoParams* camo1 = mActor->GetCamoParamsByName(camoName1);
            const CamoParams* camo2 = mActor->GetCamoParamsByName(camoName2);
            const CamoParams* camo3 = mActor->GetCamoParamsByName(camoName3);
            const CamoParams* camo4 = mActor->GetCamoParamsByName(camoName4);
            const CamoParams* camo5 = mActor->GetCamoParamsByName(camoName5);
            CPPUNIT_ASSERT(camo1 != nullptr);
            CPPUNIT_ASSERT(camo2 != nullptr);
            CPPUNIT_ASSERT(camo3 != nullptr);
            CPPUNIT_ASSERT(camo4 != nullptr);
            CPPUNIT_ASSERT(camo5 != nullptr);
            // --- Test their ids
            CPPUNIT_ASSERT(camo1 == mActor->GetCamoParamsByCamoId(0));
            CPPUNIT_ASSERT(camo2 == mActor->GetCamoParamsByCamoId(1));
            CPPUNIT_ASSERT(camo3 == mActor->GetCamoParamsByCamoId(2));
            CPPUNIT_ASSERT(camo4 == mActor->GetCamoParamsByCamoId(3));
            CPPUNIT_ASSERT(camo5 == mActor->GetCamoParamsByCamoId(4));
            // --- Test that their contained names match.
            CPPUNIT_ASSERT(camo1->GetName() == camoName1);
            CPPUNIT_ASSERT(camo2->GetName() == camoName2);
            CPPUNIT_ASSERT(camo3->GetName() == camoName3);
            CPPUNIT_ASSERT(camo4->GetName() == camoName4);
            CPPUNIT_ASSERT(camo5->GetName() == camoName5);
            // --- Test that all the camo objects were returned in the list from earlier.
            CPPUNIT_ASSERT(IsCamoInList(*camo1, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo2, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo3, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo4, camoList));
            CPPUNIT_ASSERT(IsCamoInList(*camo5, camoList));
            // --- Test all camos for a default scale
            float defaultScale = 1.0f;
            CPPUNIT_ASSERT(camo1->GetPatternScale() == 10.0f); // The test camo with a non-default value.
            CPPUNIT_ASSERT(camo2->GetPatternScale() == defaultScale);
            CPPUNIT_ASSERT(camo3->GetPatternScale() == defaultScale);
            CPPUNIT_ASSERT(camo4->GetPatternScale() == defaultScale);
            CPPUNIT_ASSERT(camo5->GetPatternScale() == defaultScale);

            // Test the colors set on the "Test" camo.
            osg::Vec4 color1(1.0f, 0.0f, 0.0f, 0.0f);
            osg::Vec4 color2(0.0f, 1.0f, 0.0f, 0.0f);
            osg::Vec4 color3(0.0f, 0.0f, 1.0f, 0.0f);
            osg::Vec4 color4(1.0f, 1.0f, 0.0f, 0.0f);
            CPPUNIT_ASSERT(camo1->GetColor1() == color1);
            CPPUNIT_ASSERT(camo1->GetColor2() == color2);
            CPPUNIT_ASSERT(camo1->GetColor3() == color3);
            CPPUNIT_ASSERT(camo1->GetColor4() == color4);
            CPPUNIT_ASSERT(camo1->GetPatternTexture().GetResourceIdentifier() == "Textures/ShadersBase/CamoPatternHard.tga");
            CPPUNIT_ASSERT(camo1->GetConcealMesh().IsEmpty());

            // Test "Test2" camo.
            CPPUNIT_ASSERT(camo5->GetPatternTexture().GetResourceIdentifier() == "Textures/ShadersBase/CamoPatternHard.tga");
            CPPUNIT_ASSERT(camo5->GetConcealMesh().GetResourceIdentifier() == "StaticMeshes/conceal_mesh/conceal_net.ive");

            // Test the other loaded camos. They should have default values.
            std::string defaultPatternTexture("Textures/ShadersBase/CamoPatternMedium.tga");
            CPPUNIT_ASSERT(camo2->GetPatternTexture().GetResourceIdentifier() == defaultPatternTexture);
            CPPUNIT_ASSERT(camo3->GetPatternTexture().GetResourceIdentifier() == defaultPatternTexture);
            CPPUNIT_ASSERT(camo4->GetPatternTexture().GetResourceIdentifier() == defaultPatternTexture);
            CPPUNIT_ASSERT(camo2->GetConcealMesh().IsEmpty());
            CPPUNIT_ASSERT(camo3->GetConcealMesh().IsEmpty());
            CPPUNIT_ASSERT(camo4->GetConcealMesh().IsEmpty());
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorTests::TestCamoSettingOnActor()
      {
         using namespace SimCore::ActComps;

         try
         {
            // Load some camo definitions.
            mActor->SetConfigFile("Configs/CamoConfig.xml");
            mGM->AddActor(*mProxy);
            const CamoParams* camo1 = mActor->GetCamoParamsByName("Test");
            const CamoParams* camo2 = mActor->GetCamoParamsByName("Test2");
            CPPUNIT_ASSERT(camo1 != nullptr);
            CPPUNIT_ASSERT(camo2 != nullptr);
            CamoParams::CamoId camoId1 = camo1->GetId();
            CamoParams::CamoId camoId2 = camo2->GetId();

            // Create a proxy.
            std::shared_ptr<dtGame::GameActorProxy> proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE, proxy);

            // Get the actor that was created.
            SimCore::Actors::Platform* actor = nullptr;
            CPPUNIT_ASSERT(proxy.valid());
            proxy->GetActor(actor);

            // Get the actor's Camo Paint Actor Component
            CamoPaintStateActComp* actComp = nullptr;
            proxy->GetComponent(actComp);
            CPPUNIT_ASSERT(actComp != nullptr);
            actComp->SetCamoId(camoId2);

            // Add the actor to the scene so it can access the 
            mGM->AddActor(*proxy, false, false);

            // Ensure the camo colors were set on the actor when it entered the world.
            CPPUNIT_ASSERT(actComp->GetPaintColor1() == camo2->GetColor1());
            CPPUNIT_ASSERT(actComp->GetPaintColor2() == camo2->GetColor2());
            CPPUNIT_ASSERT(actComp->GetPaintColor3() == camo2->GetColor3());
            CPPUNIT_ASSERT(actComp->GetPaintColor4() == camo2->GetColor4());
            CPPUNIT_ASSERT(actComp->GetPatternScale().w() == camo2->GetPatternScale());
            CPPUNIT_ASSERT(actComp->GetPatternTexture() == camo2->GetPatternTexture());
            CPPUNIT_ASSERT(actComp->GetConcealMesh() == camo2->GetConcealMesh());
            CPPUNIT_ASSERT( ! actComp->GetConcealedState());
            actComp->SetConcealedState(true);
            CPPUNIT_ASSERT(actComp->GetConcealedState());
            // --- Ensure the file reference does not change
            CPPUNIT_ASSERT(actComp->GetConcealMesh() == camo2->GetConcealMesh());

            // Test another camo that does not have a conceal mesh.
            actComp->SetCamoId(camoId1);
            CPPUNIT_ASSERT(actComp->GetPaintColor1() == camo1->GetColor1());
            CPPUNIT_ASSERT(actComp->GetPaintColor2() == camo1->GetColor2());
            CPPUNIT_ASSERT(actComp->GetPaintColor3() == camo1->GetColor3());
            CPPUNIT_ASSERT(actComp->GetPaintColor4() == camo1->GetColor4());
            CPPUNIT_ASSERT(actComp->GetPatternScale().w() == camo1->GetPatternScale());
            CPPUNIT_ASSERT(actComp->GetPatternTexture() == camo1->GetPatternTexture());
            CPPUNIT_ASSERT(actComp->GetConcealMesh() == camo1->GetConcealMesh());
            CPPUNIT_ASSERT(actComp->GetConcealMesh().IsEmpty());
            // --- Ensure that the conceal state remains unchanged
            //     even though the mesh was nullified.
            CPPUNIT_ASSERT(actComp->GetConcealedState());
            actComp->SetConcealedState(false);
            CPPUNIT_ASSERT( ! actComp->GetConcealedState());
            // --- Ensure the file reference does not change
            CPPUNIT_ASSERT(actComp->GetConcealMesh().IsEmpty());
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

   }
}
