/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008, Alion Science and Technology
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * @author David Guthrie
 */
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <osg/io_utils>
#include <osg/Math>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/project.h>
#include <dtDAL/resourcedescriptor.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/refptr.h>

#include <dtUtil/macros.h>

#include <SimCore/Actors/PlatformWithPhysics.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <SimCore/Components/ViewerMessageProcessor.h>

#include <TestComponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#ifndef AGEIA_PHYSICS
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicshelper.h>
#endif

using dtCore::RefPtr;
using dtCore::ObserverPtr;

namespace SimCore
{
   namespace Actors
   {


      class PlatformWithPhysicsTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(PlatformWithPhysicsTests);

         CPPUNIT_TEST(TestInit);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp()
            {
               dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
               dtCore::System::GetInstance().Start();
               mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

               mDeadReckoningComponent = new dtGame::DeadReckoningComponent();
               mGM->AddComponent(*mDeadReckoningComponent, dtGame::GameManager::ComponentPriority::NORMAL);

#ifndef AGEIA_PHYSICS
               dtCore::RefPtr<dtPhysics::PhysicsWorld> physicsWorld = new dtPhysics::PhysicsWorld(GetGlobalApplication());
               physicsWorld->Init();
               mGM->AddComponent(*new dtPhysics::PhysicsComponent(*physicsWorld, false),
                        dtGame::GameManager::ComponentPriority::NORMAL);
#endif
               mGM->CreateActor(*EntityActorRegistry::PLATFORM_WITH_PHYSICS_ACTOR_TYPE, mPlatformWithPhysicsActorProxy);
               PlatformWithPhysics* temp;
               mPlatformWithPhysicsActorProxy->GetActor(temp);
               mPlatformWithPhysics = temp;
               CPPUNIT_ASSERT(mPlatformWithPhysics.valid());
            }

            void tearDown()
            {
               mDeadReckoningComponent = NULL;

#ifndef AGEIA_PHYSICS
               mPhysicsComp = NULL;
#endif

               mPlatformWithPhysics = NULL;
               mPlatformWithPhysicsActorProxy = NULL;

               if (mGM.valid())
               {
                  mGM->DeleteAllActors(true);
                  mGM = NULL;
               }
               dtCore::System::GetInstance().Stop();

            }

            void TestInit()
            {
               CPPUNIT_ASSERT(mPlatformWithPhysics->GetPhysicsHelper() != NULL);
               mGM->AddActor(*mPlatformWithPhysicsActorProxy, false, false);

            }

         private:

            RefPtr<dtGame::GameManager> mGM;
            RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;
            RefPtr<SimCore::Actors::PlatformWithPhysics> mPlatformWithPhysics;
            RefPtr<SimCore::Actors::PlatformWithPhysicsActorProxy> mPlatformWithPhysicsActorProxy;

#ifndef AGEIA_PHYSICS
            RefPtr<dtPhysics::PhysicsComponent> mPhysicsComp;
#endif
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(PlatformWithPhysicsTests);
   }
}


