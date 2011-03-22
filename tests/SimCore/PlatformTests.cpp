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
#include <dtGame/drpublishingactcomp.h>

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
#include <SimCore/ActComps/CamoPaintStateActComp.h>
#include <SimCore/ActComps/WeaponSwapActComp.h>
#include <SimCore/ActComps/WheelActComp.h>

#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/CollisionGroupEnum.h>

#include <TestComponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>

using dtCore::RefPtr;
using dtCore::ObserverPtr;

namespace SimCore
{
   namespace Actors
   {

      class PlatformTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(PlatformTests);

         CPPUNIT_TEST(TestPhysicsDefaults);
         CPPUNIT_TEST(TestInit);
         CPPUNIT_TEST(TestAvailableActorComponents);

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

               dtCore::RefPtr<dtPhysics::PhysicsWorld> physicsWorld = new dtPhysics::PhysicsWorld(GetGlobalApplication());
               physicsWorld->Init();
               mGM->AddComponent(*new dtPhysics::PhysicsComponent(*physicsWorld, false),
                        dtGame::GameManager::ComponentPriority::NORMAL);

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

               mPhysicsComp = NULL;

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
               CPPUNIT_ASSERT(mPlatformWithPhysics->GetComponent<dtPhysics::PhysicsActComp>() != NULL);
               dtPhysics::PhysicsObject* po = mPlatformWithPhysics->GetComponent<dtPhysics::PhysicsActComp>()->GetMainPhysicsObject();
               CPPUNIT_ASSERT_EQUAL(dtPhysics::MechanicsType::DYNAMIC, po->GetMechanicsType());
               CPPUNIT_ASSERT_EQUAL(dtPhysics::PrimitiveType::CONVEX_HULL, po->GetPrimitiveType());
               CPPUNIT_ASSERT_EQUAL(dtPhysics::CollisionGroup(SimCore::CollisionGroup::GROUP_VEHICLE_GROUND), po->GetCollisionGroup());

               mGM->AddActor(*mPlatformWithPhysicsActorProxy, true, false);

               CPPUNIT_ASSERT_EQUAL_MESSAGE("Remotes should auto switch to kinematic",
                        dtPhysics::MechanicsType::KINEMATIC, po->GetMechanicsType());
            }

            void TestInit()
            {
               CPPUNIT_ASSERT(mPlatformWithPhysics->GetComponent<dtPhysics::PhysicsActComp>() != NULL);
               dtPhysics::PhysicsObject* po = mPlatformWithPhysics->GetComponent<dtPhysics::PhysicsActComp>()->GetMainPhysicsObject();
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
            }

            void TestAvailableActorComponents()
            {
               CheckAvailableActorComponents(*mPlatformWithPhysicsActorProxy, true, false, false, false, false);

               dtCore::RefPtr<SimCore::Actors::PlatformActorProxy> platform;
               mGM->CreateActor(*EntityActorRegistry::PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, false, false, false, false, false);

               CPPUNIT_ASSERT_MESSAGE("Platforms with the switchable type should default to having physics creation on.",
                        PlatformActorProxy::GetPhysicsCreationEnabled());

               PlatformActorProxy::SetPhysicsCreationEnabled(false);

               mGM->CreateActor(*EntityActorRegistry::PLATFORM_WITH_PHYSICS_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, true, false, false, false, false);

               mGM->CreateActor(*EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, false, false, false, false, false);

               mGM->CreateActor(*EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, false, true, true, true, false);

               mGM->CreateActor(*EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, false, true, true, true, true);

               mGM->CreateActor(*EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, false, false, false, false, false);

               mGM->CreateActor(*EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, false, false, false, false, true);



               PlatformActorProxy::SetPhysicsCreationEnabled(true);

               mGM->CreateActor(*EntityActorRegistry::PLATFORM_WITH_PHYSICS_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, true, false, false, false, false);

               mGM->CreateActor(*EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, true, false, false, false, false);

               mGM->CreateActor(*EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, true, true, true, true, false);

               mGM->CreateActor(*EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, true, true, true, true, true);

               mGM->CreateActor(*EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, true, false, false, false, false);

               mGM->CreateActor(*EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE, platform);

               CheckAvailableActorComponents(*platform, true, false, false, false, true);

            }

         private:

            void CheckAvailableActorComponents(SimCore::Actors::PlatformActorProxy& platform, bool physics, bool camo, bool weaponswap,
                     bool munitions, bool wheels)
            {
               CPPUNIT_ASSERT(platform.GetComponent<dtGame::DeadReckoningHelper>() != NULL);
               CPPUNIT_ASSERT(platform.GetComponent<dtGame::DRPublishingActComp>() != NULL);

               CPPUNIT_ASSERT_EQUAL(physics, platform.GetComponent<dtPhysics::PhysicsActComp>() != NULL);

               CPPUNIT_ASSERT_EQUAL(camo, platform.GetComponent<SimCore::ActComps::CamoPaintStateActComp>() != NULL);
               CPPUNIT_ASSERT_EQUAL(weaponswap, platform.GetComponent<SimCore::ActComps::WeaponSwapActComp>() != NULL);
               CPPUNIT_ASSERT_EQUAL(wheels, platform.GetComponent<SimCore::ActComps::WheelActComp>() != NULL);

               BaseEntity* be = NULL;
               platform.GetActor(be);
               CPPUNIT_ASSERT(be != NULL);
               CPPUNIT_ASSERT_EQUAL(munitions, be->GetAutoRegisterWithMunitionsComponent());

               if (platform.GetActorType().InstanceOf(*EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE))
               {
                  CPPUNIT_ASSERT_EQUAL(dtGame::GroundClampTypeEnum::NONE, platform.GetComponent<dtGame::DeadReckoningHelper>()->GetGroundClampType());
                  CPPUNIT_ASSERT_EQUAL(PlatformActorProxy::DomainEnum::AIR, be->GetDomain());
               }
               else if (platform.GetActorType().InstanceOf(*EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE))
               {
                  CPPUNIT_ASSERT_EQUAL(PlatformActorProxy::DomainEnum::GROUND, be->GetDomain());
               }
            }

            RefPtr<dtGame::GameManager> mGM;
            RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;
            RefPtr<SimCore::Actors::PlatformWithPhysics> mPlatformWithPhysics;
            RefPtr<SimCore::Actors::PlatformWithPhysicsActorProxy> mPlatformWithPhysicsActorProxy;

            RefPtr<dtPhysics::PhysicsComponent> mPhysicsComp;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(PlatformTests);
   }
}


