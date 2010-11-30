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
#include <prefix/SimCorePrefix.h>
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
#include <SimCore/CollisionGroupEnum.h>

#include <TestComponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#ifndef AGEIA_PHYSICS
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
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

         CPPUNIT_TEST(TestPhysicsDefaults);
         CPPUNIT_TEST(TestInit);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp()
            {
               dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
               dtCore::System::GetInstance().Start();
               mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

               mGM->SetApplication(GetGlobalApplication());
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
               dtDAL::ResourceActorProperty* res = NULL;
               mPlatformWithPhysicsActorProxy->GetProperty(PlatformActorProxy::PROPERTY_MESH_NON_DAMAGED_ACTOR, res);
               dtDAL::ResourceDescriptor truck("StaticMeshes:NetDemo:Vehicles:Truck.ive");
               res->SetValue(truck);
               mPlatformWithPhysicsActorProxy->GetProperty(PlatformActorProxy::PROPERTY_MESH_DAMAGED_ACTOR, res);
               res->SetValue(truck);
               CPPUNIT_ASSERT(mPlatformWithPhysics.valid());
            }

            void tearDown()
            {
               mDeadReckoningComponent = NULL;

#ifndef AGEIA_PHYSICS
               mPhysicsComp = NULL;
#endif

               mPlatformWithPhysics = NULL;
               dtCore::ObserverPtr<SimCore::Actors::PlatformWithPhysicsActorProxy> pOb = mPlatformWithPhysicsActorProxy.get();
               mPlatformWithPhysicsActorProxy = NULL;

               if (mGM.valid())
               {
                  mGM->DeleteAllActors(true);
                  mGM = NULL;
               }
               dtCore::System::GetInstance().Stop();

               CPPUNIT_ASSERT(!pOb.valid());
            }

            void TestPhysicsDefaults()
            {
               CPPUNIT_ASSERT(mPlatformWithPhysics->GetPhysicsActComp() != NULL);
               dtPhysics::PhysicsObject* po = mPlatformWithPhysics->GetPhysicsActComp()->GetMainPhysicsObject();
               CPPUNIT_ASSERT_EQUAL(dtPhysics::MechanicsType::DYNAMIC, po->GetMechanicsType());
               CPPUNIT_ASSERT_EQUAL(dtPhysics::PrimitiveType::CONVEX_HULL, po->GetPrimitiveType());
               CPPUNIT_ASSERT_EQUAL(dtPhysics::CollisionGroup(SimCore::CollisionGroup::GROUP_VEHICLE_GROUND), po->GetCollisionGroup());

               mGM->AddActor(*mPlatformWithPhysicsActorProxy, true, false);

               CPPUNIT_ASSERT_EQUAL_MESSAGE("Remotes should auto switch to kinematic",
                        dtPhysics::MechanicsType::KINEMATIC, po->GetMechanicsType());
            }

            void TestInit()
            {
#ifndef AGEIA_PHYSICS
               CPPUNIT_ASSERT(mPlatformWithPhysics->GetPhysicsActComp() != NULL);
               dtPhysics::PhysicsObject* po = mPlatformWithPhysics->GetPhysicsActComp()->GetMainPhysicsObject();
               CPPUNIT_ASSERT(po->GetBodyWrapper() == NULL);
               mGM->AddActor(*mPlatformWithPhysicsActorProxy, false, false);

               dtPhysics::BodyWrapper* bw = po->GetBodyWrapper();
               dtCore::ObserverPtr<dtPhysics::BodyWrapper> bwOb = bw;
               CPPUNIT_ASSERT(bw != NULL);
               mPlatformWithPhysics->SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE);

               CPPUNIT_ASSERT_MESSAGE("the physics object should be initialized", po->GetBodyWrapper() != NULL);
               CPPUNIT_ASSERT_MESSAGE("the physics object body should not be same as before because it should have be reloaded",
                        po->GetBodyWrapper() != bw);
               CPPUNIT_ASSERT_MESSAGE("The original body should be deleted", !bwOb.valid());
#endif
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


