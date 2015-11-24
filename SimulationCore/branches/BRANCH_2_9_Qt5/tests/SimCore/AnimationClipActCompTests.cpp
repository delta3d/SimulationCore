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
#include <osg/StateSet>
#include <dtGame/gameactorproxy.h>
#include <dtGame/gamemanager.h>
#include <SimCore/ActComps/AnimationClipActComp.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/Platform.h>
#include <UnitTestMain.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // TESTS OBJECT
      //////////////////////////////////////////////////////////////////////////
      class AnimationClipActCompTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(AnimationClipActCompTests);
         CPPUNIT_TEST(TestProperties);
         CPPUNIT_TEST(TestOnActor);
         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            // Test Functions:
            void TestProperties();
            void TestOnActor();

         private:
            dtCore::RefPtr<dtGame::GameManager> mGM;
            dtCore::RefPtr<AnimationClipActComp> mActComp;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(AnimationClipActCompTests);

      //////////////////////////////////////////////////////////////////////////
      void AnimationClipActCompTests::setUp()
      {
         try
         {
            // Create the Game Manager.
            mGM = new dtGame::GameManager( *GetGlobalApplication().GetScene() );
            mGM->SetApplication(GetGlobalApplication());

            mActComp = new AnimationClipActComp();
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationClipActCompTests::tearDown()
      {
         try
         {
            mGM->DeleteAllActors(true);
            mGM = NULL;

            mActComp = NULL;
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationClipActCompTests::TestProperties()
      {
         try
         {
            std::string testNames[] = {
                "Element A",
                "Element B",
                "Element C"
            };
            const int numElements = (int)sizeof(testNames);
            const AnimationClipActComp& constComp = *mActComp;
            typedef std::vector<dtCore::RefPtr<AnimationPropertyContainer> > PropArray;
            PropArray propContainers;

            // Test actor component
            CPPUNIT_ASSERT( ! constComp.IsPaused());
            mActComp->SetPaused(true);
            CPPUNIT_ASSERT(constComp.IsPaused());

            dtCore::RefPtr<AnimationPropertyContainer> curPropContainer;
            CPPUNIT_ASSERT(mActComp->GetNumAnimationPropertyContainers() == 0);
            for (int i = 0; i < numElements; ++i)
            {
                mActComp->InsertNewAnimationPropertyContainer(i);
                curPropContainer = mActComp->GetAnimationPropertyContainer(i);
                curPropContainer->SetName(testNames[i]);
                CPPUNIT_ASSERT(curPropContainer->GetObjectType().DefaultsEmpty());
                propContainers.push_back(curPropContainer);
            }
            CPPUNIT_ASSERT(mActComp->GetNumAnimationPropertyContainers() == numElements);
            for (int i = 0; i < numElements; ++i)
            {
                CPPUNIT_ASSERT(mActComp->GetAnimationPropertyContainer(i) == propContainers[i].get());
            }

            // Test animation container properties
            AnimationPropertyContainer* testProps = propContainers[1]; // 2nd element
            std::string testStr("TestName");
            CPPUNIT_ASSERT(testProps->GetName().empty());
            testProps->SetName(testStr);
            CPPUNIT_ASSERT(testProps->GetName() == testStr);

            float testValue = 8.5f;
            CPPUNIT_ASSERT(testProps->GetBeginTime() == 0.0f);
            testProps->SetBeginTime(testValue);
            CPPUNIT_ASSERT(testProps->GetBeginTime() == testValue);

            testValue = 30.5f;
            CPPUNIT_ASSERT(testProps->GetEndTime() == 0.0f);
            testProps->SetEndTime(testValue);
            CPPUNIT_ASSERT(testProps->GetEndTime() == testValue);

            testValue = 0.5f;
            CPPUNIT_ASSERT(testProps->GetTimeScale() == 1.0f);
            testProps->SetTimeScale(testValue);
            CPPUNIT_ASSERT(testProps->GetTimeScale() == testValue);
            
            CPPUNIT_ASSERT(testProps->GetPlayMode() == PlayModeEnum::ONCE);
            testProps->SetPlayMode(PlayModeEnum::LOOP);
            CPPUNIT_ASSERT(testProps->GetPlayMode() == PlayModeEnum::LOOP);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }
      
      //////////////////////////////////////////////////////////////////////////
      void AnimationClipActCompTests::TestOnActor()
      {
         try
         {
            // Create a proxy.
            /*dtCore::RefPtr<dtGame::GameActorProxy> proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE, proxy);

            // Get the actor that was created.
            SimCore::Actors::Platform* actor = NULL;
            proxy->GetDrawable(actor);

            // Ensure the component exists
            mActComp = NULL;
            AnimationClipActComp* actComp = actor->GetComponent<AnimationClipActComp>();
            CPPUNIT_ASSERT(actComp != NULL);
            CPPUNIT_ASSERT(actComp->GetOwnerNode() == actor->GetOSGNode());*/

            // TODO:
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

   }
}
