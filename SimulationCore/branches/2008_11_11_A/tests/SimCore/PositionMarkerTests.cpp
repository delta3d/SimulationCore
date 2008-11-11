/* -*-c++-*-
* Simulation Core - PositionMarkerTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
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
#include <osg/Vec4>

#include <dtUtil/mathdefines.h>
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
            CPPUNIT_TEST(TestColorForForce);
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
               CreatePositionMarker(pmap, pm);

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
               CreatePositionMarker(pmap, pm);

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
               CreatePositionMarker(pmap, pm);

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
               CreatePositionMarker(pmap, pm);
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
               CreatePositionMarker(pmap, pm);

               dtDAL::ColorRgbaActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_FRIENDLY_COLOR, prop);
               osg::Vec4 theColorF(1.2, 0.03, 0.33, 1.0);
               prop->SetValue(theColorF);
               CPPUNIT_ASSERT_EQUAL(theColorF, prop->GetValue());
               CPPUNIT_ASSERT_EQUAL(theColorF, pm->GetFriendlyColor());

               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_NEUTRAL_COLOR, prop);
               osg::Vec4 theColorN(1.0, 0.03, 0.81, 1.0);
               prop->SetValue(theColorN);
               CPPUNIT_ASSERT_EQUAL(theColorN, prop->GetValue());
               CPPUNIT_ASSERT_EQUAL(theColorN, pm->GetNeutralColor());

               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_OPPOSING_COLOR, prop);
               osg::Vec4 theColorOP(0.93, 0.09, 0.11, 9.9);
               prop->SetValue(theColorOP);
               CPPUNIT_ASSERT_EQUAL(theColorOP, prop->GetValue());
               CPPUNIT_ASSERT_EQUAL(theColorOP, pm->GetOpposingColor());

               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_OTHER_COLOR, prop);
               osg::Vec4 theColorOT(0.93, 0.09, 1.0, 1.0);
               prop->SetValue(theColorOT);
               CPPUNIT_ASSERT_EQUAL(theColorOT, prop->GetValue());
               CPPUNIT_ASSERT_EQUAL(theColorOT, pm->GetOtherColor());

            }

            void TestColorForForce()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               dtDAL::ColorRgbaActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_MARKER_COLOR, prop);
               CPPUNIT_ASSERT(prop != NULL);
               CPPUNIT_ASSERT(prop->IsReadOnly());

               osg::Vec4 theColor;

               pm->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::FRIENDLY);
               theColor = pm->GetColor();
               osg::Vec4 expectedColorF(pm->GetFriendlyColor());
               expectedColorF.a() = 0.8;
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorF);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorF);

               pm->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::NEUTRAL);
               theColor = pm->GetColor();
               osg::Vec4 expectedColorN(pm->GetNeutralColor());
               expectedColorN.a() = 0.8;
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorN);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorN);

               pm->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OPPOSING);
               theColor = pm->GetColor();
               osg::Vec4 expectedColorOP(pm->GetOpposingColor());
               expectedColorOP.a() = 0.8;
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP);

               pm->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::INSURGENT);
               theColor = pm->GetColor();
               osg::Vec4 expectedColorOP2(pm->GetOpposingColor());
               expectedColorOP2.a() = 0.8;
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP2);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP2);

               pm->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OTHER);
               theColor = pm->GetColor();
               osg::Vec4 expectedColorOT(pm->GetOtherColor());
               expectedColorOT.a() = 0.8;
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOT);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOT);
            }

         private:

            void CreatePositionMarker(RefPtr<PositionMarkerActorProxy>& pmap, PositionMarker*& pm)
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
