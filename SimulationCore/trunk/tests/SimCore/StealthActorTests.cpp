/* -*-c++-*-
* Simulation Core - StealthActorTests (.h & .cpp) - Using 'The MIT License'
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
* @author David Guthrie
*/
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <osg/io_utils>
#include <osg/Math>
#include <osg/Vec3>

#include <dtABC/application.h>

#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/gamemanager.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/project.h>
#include <dtDAL/map.h>

#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/transform.h>
#include <dtCore/globals.h>
#include <dtCore/deltawin.h>

#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <UnitTestMain.h>
#include <TestComponent.h>

#if defined (WIN32) || defined (_WIN32) || defined (__WIN32__)
   #include <dtUtil/mswin.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using dtCore::RefPtr;

class StealthActorTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(StealthActorTests);

      CPPUNIT_TEST(TestStealthActorProperties);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         dtCore::System::GetInstance().Start();
         mApp = &GetGlobalApplication();

         mGM = new dtGame::GameManager(*mApp->GetScene());
         mGM->SetApplication( *mApp );
         mDeadReckoningComponent = new dtGame::DeadReckoningComponent("DeadReckoningComponent");
         mTestComponent = new TestComponent();
         mGM->AddComponent(*mDeadReckoningComponent, dtGame::GameManager::ComponentPriority::NORMAL);
         mGM->AddComponent(*mTestComponent, dtGame::GameManager::ComponentPriority::NORMAL);

         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, mStealthProxy);


         mStealthActor = dynamic_cast<SimCore::Actors::StealthActor*>(&mStealthProxy->GetGameActor());

         CPPUNIT_ASSERT(mStealthProxy.valid());
         CPPUNIT_ASSERT(mStealthActor.valid());
      }

      void tearDown()
      {
         mStealthProxy = NULL;
         mStealthActor = NULL;
         mDeadReckoningComponent = NULL;
         mTestComponent = NULL;
         mApp = NULL;
         if (mGM.valid())
         {
            mGM->DeleteAllActors(true);
            mGM = NULL;
         }
         dtCore::System::GetInstance().Stop();
      }

      void TestStealthActorProperties()
      {
         CPPUNIT_ASSERT_MESSAGE("Attaching as third person should default to true.",
            mStealthActor->GetAttachAsThirdPerson());

         mStealthActor->SetAttachAsThirdPerson(false);

         CPPUNIT_ASSERT_MESSAGE("Attaching as third person should be false.",
            !mStealthActor->GetAttachAsThirdPerson());

         CPPUNIT_ASSERT_MESSAGE("Default value should be 1.",
            osg::equivalent(mStealthActor->GetMaxTranslationError(), 0.5f, 0.001f));

         float newValue = 25.03;
         mStealthActor->SetMaxTranslationError(newValue);

         CPPUNIT_ASSERT(osg::equivalent(mStealthActor->GetMaxTranslationError(), newValue, 0.001f));

         mStealthActor->SetMaxRotationError(newValue);

         CPPUNIT_ASSERT(osg::equivalent(mStealthActor->GetMaxRotationError(), newValue, 0.001f));

         dtGame::Invokable *invoke = mStealthActor->GetGameActorProxy().GetInvokable("AttachToActor");
         CPPUNIT_ASSERT_MESSAGE("The AttachToActor invokable should not be NULL", invoke != NULL);

         invoke = mStealthActor->GetGameActorProxy().GetInvokable("WarpToPosition");
         CPPUNIT_ASSERT_MESSAGE("The WarpToPosition invokable should not be NULL", invoke != NULL);

      }




   private:

      RefPtr<dtGame::GameManager> mGM;
      RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;
      RefPtr<TestComponent> mTestComponent;
      RefPtr<SimCore::Actors::StealthActorProxy> mStealthProxy;
      RefPtr<SimCore::Actors::StealthActor> mStealthActor;
      RefPtr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthActorTests);


