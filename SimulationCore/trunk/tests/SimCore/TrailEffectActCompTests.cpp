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
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/drpublishingactcomp.h>
#include <dtCore/actorproperty.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/resourcedescriptor.h>
#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/refptr.h>
#include <dtUtil/macros.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/ActComps/TrailEffectActComp.h>

#include <dtGame/testcomponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>



using dtCore::RefPtr;
using dtCore::ObserverPtr;

namespace SimCore
{
   namespace ActComps
   {

      class TrailEffectActCompTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(TrailEffectActCompTests);

         CPPUNIT_TEST(TestProperties);
         CPPUNIT_TEST(TestOnActor);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp()
            {
               dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
               dtCore::System::GetInstance().Start();

               mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
               mGM->SetApplication(GetGlobalApplication());
            }

            void tearDown()
            {
               if (mGM.valid())
               {
                  mGM->DeleteAllActors(true);
                  mGM = NULL;
               }
               dtCore::System::GetInstance().Stop();
            }

            void TestProperties()
            {
               using namespace SimCore::ActComps;
               using namespace SimCore::Actors;

               dtCore::ResourceDescriptor particleFile("Particles:fire.osg");

               dtCore::RefPtr<TrailEffectActComp> actComp = new TrailEffectActComp();
               const TrailEffectActComp* constComp = actComp.get();

               CPPUNIT_ASSERT(constComp->GetTrailParticlesFile().GetResourceIdentifier().empty());
               actComp->SetTrailParticlesFile(particleFile);
               CPPUNIT_ASSERT(constComp->GetTrailParticlesFile() == particleFile);

               CPPUNIT_ASSERT( ! constComp->GetTrailAttached());
               CPPUNIT_ASSERT(constComp->IsTickable());
               actComp->SetTrailAttached(true);
               CPPUNIT_ASSERT(constComp->GetTrailAttached());
               CPPUNIT_ASSERT( ! constComp->IsTickable());

               CPPUNIT_ASSERT(constComp->GetTrailClampInterval() == TrailEffectActComp::DEFAULT_TRAIL_CLAMP_INTERVAL);
               CPPUNIT_ASSERT(constComp->GetTrailClampInterval() != 5.0f);
               actComp->SetTrailClampInterval(5.0f);
               CPPUNIT_ASSERT(constComp->GetTrailClampInterval() == 5.0f);

               CPPUNIT_ASSERT(constComp->GetTrailEnableDistance() == TrailEffectActComp::DEFAULT_TRAIL_ENABLE_DISTANCE);
               CPPUNIT_ASSERT(constComp->GetTrailEnableDistance() != 7.0f);
               actComp->SetTrailEnableDistance(7.0f);
               CPPUNIT_ASSERT(constComp->GetTrailEnableDistance() == 7.0f);
               
               std::string nodeName("TestNodeName");
               CPPUNIT_ASSERT(constComp->GetTrailAttachNodeName().empty());
               actComp->SetTrailAttachNodeName(nodeName);
               CPPUNIT_ASSERT(constComp->GetTrailAttachNodeName() == nodeName);

               CPPUNIT_ASSERT( ! constComp->IsEnabled());
               actComp->OnEnteredWorld();
               CPPUNIT_ASSERT(constComp->IsEnabled());
               actComp->SetEnabled(false);
               CPPUNIT_ASSERT( ! constComp->IsEnabled());
               actComp->SetEnabled(true);
               CPPUNIT_ASSERT(constComp->IsEnabled());
            }

            void TestOnActor()
            {
               using namespace SimCore::ActComps;
               using namespace SimCore::Actors;

               dtCore::RefPtr<PlatformActorProxy> platform;
               mGM->CreateActor(*EntityActorRegistry::HELO_PLATFORM_ACTOR_TYPE, platform);

               TrailEffectActComp* actComp = platform->GetComponent<TrailEffectActComp>();
               CPPUNIT_ASSERT(actComp != NULL);

               dtCore::ResourceDescriptor particleFile("Particles:fire.osg");
               actComp->SetTrailParticlesFile(particleFile);
               actComp->SetTrailAttached(false);

               // Ensure that the OnEnteredWorld method is called when the
               // owning actor enters the world.
               CPPUNIT_ASSERT( ! actComp->IsEnabled());
               mGM->AddActor(*platform, false, false);
               CPPUNIT_ASSERT(actComp->IsEnabled());
            }

         private:

            RefPtr<dtGame::GameManager> mGM;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(TrailEffectActCompTests);
   }
}


