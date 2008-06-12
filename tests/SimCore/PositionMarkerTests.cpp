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
#include <osg/Vec4>

#include <dtCore/system.h>
#include <dtCore/observerptr.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/PositionMarker.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <TestComponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#ifdef DELTA_WIN32
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using dtCore::RefPtr;
using dtCore::ObserverPtr;

namespace SimCore
{
   namespace Actors
   {
      class PositionMarkerTests : public CPPUNIT_NS::TestFixture 
      {
         CPPUNIT_TEST_SUITE(PositionMarkerTests);

            CPPUNIT_TEST(TestPositionMarkerSourceCallsign);
            CPPUNIT_TEST(TestPositionMarkerSourceService);
            CPPUNIT_TEST(TestPositionMarkerSourceForce);
            CPPUNIT_TEST(TestColors);
            CPPUNIT_TEST(TestPositionMarkerDelete);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp()
            {
               dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
               dtCore::System::GetInstance().Start();
               mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

               mDeadReckoningComponent = new dtGame::DeadReckoningComponent();
               mGM->AddComponent(*mDeadReckoningComponent, dtGame::GameManager::ComponentPriority::NORMAL);

            }

            void tearDown()
            {
               mDeadReckoningComponent = NULL;

               if (mGM.valid())
               {
                  mGM->DeleteAllActors(true);
                  mGM = NULL;
               }
               dtCore::System::GetInstance().Stop();
            }

            void TestPositionMarkerSourceCallsign()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMaker(pmap, pm);

               dtDAL::StringActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_SOURCE_CALLSIGN, prop);
               static const std::string TEST_STRING("Silly");
               prop->FromString(TEST_STRING);
               CPPUNIT_ASSERT_EQUAL(TEST_STRING, prop->ToString());
               CPPUNIT_ASSERT_EQUAL(TEST_STRING, pm->GetSourceCallsign());
            }

            void TestPositionMarkerSourceForce()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMaker(pmap, pm);

               dtDAL::AbstractEnumActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_SOURCE_FORCE, prop);
               prop->SetEnumValue(BaseEntityActorProxy::ForceEnum::INSURGENT);
               CPPUNIT_ASSERT(BaseEntityActorProxy::ForceEnum::INSURGENT == prop->GetEnumValue());
               CPPUNIT_ASSERT_EQUAL(BaseEntityActorProxy::ForceEnum::INSURGENT, pm->GetSourceForce());
            }

            void TestPositionMarkerSourceService()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMaker(pmap, pm);

               dtDAL::AbstractEnumActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_SOURCE_SERVICE, prop);
               prop->SetEnumValue(BaseEntityActorProxy::ServiceEnum::NAVY);
               CPPUNIT_ASSERT(BaseEntityActorProxy::ServiceEnum::NAVY == prop->GetEnumValue());
               CPPUNIT_ASSERT_EQUAL(BaseEntityActorProxy::ServiceEnum::NAVY, pm->GetSourceService());
            }

            void TestPositionMarkerDelete()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMaker(pmap, pm);
               dtCore::ObserverPtr<PositionMarker> edraw = pm;

               CPPUNIT_ASSERT_EQUAL(1, pmap->referenceCount());
               CPPUNIT_ASSERT(edraw.valid());
               pmap = NULL;
               CPPUNIT_ASSERT(!edraw.valid());
               edraw = NULL;
            }

            void TestColors()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMaker(pmap, pm);

               dtDAL::ColorRgbaActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_SPHERE_COLOR, prop);
               osg::Vec4 theColor(1.0, 0.03, 0.81, 0.3);
               prop->SetValue(theColor);
               CPPUNIT_ASSERT_EQUAL(theColor, prop->GetValue());
               CPPUNIT_ASSERT_EQUAL(theColor, pm->GetSphereColor());

               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_BOX_COLOR, prop);
               prop->SetValue(theColor);
               CPPUNIT_ASSERT_EQUAL(theColor, prop->GetValue());
               CPPUNIT_ASSERT_EQUAL(theColor, pm->GetSphereColor());
            }

         private:

            void CreatePositionMaker(RefPtr<PositionMarkerActorProxy>& pmap, PositionMarker*& pm)
            {
               mGM->CreateActor(*EntityActorRegistry::POSITION_MARKER_ACTOR_TYPE, pmap);
               CPPUNIT_ASSERT(pmap.valid());
               pmap->GetActor(pm);
               CPPUNIT_ASSERT(pm != NULL);
            }

            RefPtr<dtGame::GameManager> mGM;
            RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;

      };

      CPPUNIT_TEST_SUITE_REGISTRATION(PositionMarkerTests);
   }
}
