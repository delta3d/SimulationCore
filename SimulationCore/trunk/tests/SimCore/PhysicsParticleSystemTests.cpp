/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2010, Alion Science and Technology
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
 * David Guthrie
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

#include <SimCore/Actors/EntityActorRegistry.h>

#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/CollisionGroupEnum.h>

#include <TestComponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>

#include <SimCore/Actors/PhysicsParticleSystemActor.h>

using dtCore::RefPtr;
using dtCore::ObserverPtr;

namespace SimCore
{
   namespace Actors
   {


      class PhysicsParticleSystemTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(PhysicsParticleSystemTests);

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

            dtCore::RefPtr<dtPhysics::PhysicsWorld> physicsWorld = new dtPhysics::PhysicsWorld(GetGlobalApplication());
            physicsWorld->Init();
            mGM->AddComponent(*new dtPhysics::PhysicsComponent(*physicsWorld, false),
                     dtGame::GameManager::ComponentPriority::NORMAL);
            mGM->CreateActor(*EntityActorRegistry::PHYSICS_PARTICLE_SYSTEM_TYPE, mPhysicsParticleSystemActor);
            CPPUNIT_ASSERT(mPhysicsParticleSystemActor.valid());
         }

         void tearDown()
         {
            mDeadReckoningComponent = NULL;

            mPhysicsComp = NULL;

            dtCore::ObserverPtr<SimCore::Actors::PhysicsParticleSystemActorProxy> pOb = mPhysicsParticleSystemActor.get();
            mPhysicsParticleSystemActor = NULL;

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

         }

         void TestInit()
         {
            SimCore::Actors::PhysicsParticleSystemActor* drawable = NULL;
            mPhysicsParticleSystemActor->GetDrawable(drawable);
            dtPhysics::PhysicsActComp* pac = NULL;
            drawable->GetComponent(pac);
            CPPUNIT_ASSERT(pac != NULL);

            pac->SetDefaultPrimitiveType(dtPhysics::PrimitiveType::SPHERE);
            pac->SetDimensions(osg::Vec3(3.0f, 3.0f, 3.0f));

            mGM->AddActor(*mPhysicsParticleSystemActor, false, false);

         }

      private:

         RefPtr<dtGame::GameManager> mGM;
         RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;
         RefPtr<SimCore::Actors::PhysicsParticleSystemActorProxy> mPhysicsParticleSystemActor;

         RefPtr<dtPhysics::PhysicsComponent> mPhysicsComp;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(PhysicsParticleSystemTests);
   }
}


