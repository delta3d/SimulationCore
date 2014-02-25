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
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <osg/io_utils>
#include <osg/Math>
#include <osg/Vec4>

#include <dtUtil/mathdefines.h>
#include <dtCore/system.h>
#include <dtCore/observerptr.h>
#include <dtCore/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/PositionMarker.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <dtGame/testcomponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

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
            CPPUNIT_TEST(TestPositionMarkerInitialAlpha);
            CPPUNIT_TEST(TestPositionMarkerDeleteOnFadeOut);
            CPPUNIT_TEST(TestCalcAlpha);
            CPPUNIT_TEST(TestCalcColor);
            CPPUNIT_TEST(TestAddTimer);
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
               mGM->SetApplication(GetGlobalApplication());

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

               dtCore::StringActorProperty* prop = NULL;
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

               dtCore::AbstractEnumActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_SOURCE_FORCE, prop);
               prop->SetEnumValue(BaseEntityActorProxy::ForceEnum::INSURGENT);
               CPPUNIT_ASSERT(BaseEntityActorProxy::ForceEnum::INSURGENT == prop->GetEnumValue());
               CPPUNIT_ASSERT_EQUAL(BaseEntityActorProxy::ForceEnum::INSURGENT, pm->GetSourceForce());
            }

            void TestPositionMarkerInitialAlpha()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The alpha should default to 1.0", 1.0f, pm->GetInitialAlpha(), 0.001f);

               dtCore::FloatActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_INITIAL_ALPHA, prop);
               prop->SetValue(0.3f);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3f, prop->GetValue(), 0.001f);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3f, pm->GetInitialAlpha(), 0.001f);
               pm->SetInitialAlpha(70.0f);
               // Test the prop here
               CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The alpha should clamp to 1.0", 1.0f, prop->GetValue(), 0.001f);
               pm->SetInitialAlpha(-60.00f);
               // Test accessor directly here
               CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("the alpha should clamp to 0.0", 0.0f, pm->GetInitialAlpha(), 0.001f);
            }

            void TestPositionMarkerDeleteOnFadeOut()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               CPPUNIT_ASSERT(!pm->GetDeleteOnFadeOut());
               dtCore::BooleanActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_DELETE_ON_FADE_OUT, prop);
               prop->SetValue(true);
               CPPUNIT_ASSERT(prop->GetValue());
               CPPUNIT_ASSERT(pm->GetDeleteOnFadeOut());
            }

            void TestPositionMarkerSourceService()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               dtCore::AbstractEnumActorProperty* prop = NULL;
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

            void TestCalcAlpha()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               pm->SetInitialAlpha(1.0);
               pm->SetStaleTime(3.0);
               pm->SetFadeOutTime(10.0 / 60.0);
               pm->SetReportTime(mGM->GetSimulationTime() - 5.0);
               mGM->AddActor(*pmap, false, false);
               float alpha = pm->CalculateCurrentAlpha();
               CPPUNIT_ASSERT_DOUBLES_EQUAL(0.75, alpha, 0.01);
            }

            void TestCalcColor()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               // The force here first so that the friendly color set will have to
               // force an update to work
               pm->SetForceAffiliation(BaseEntityActorProxy::ForceEnum::FRIENDLY);
               osg::Vec4 friendlyColor(1.0, 1.0, 1.0, 1.0);
               pm->SetFriendlyColor(friendlyColor);
               osg::Vec4 initialColor(pm->GetInitialColor(), 1.0);
               CPPUNIT_ASSERT_EQUAL_MESSAGE("simply setting the friendly color should update the initial color",
                        friendlyColor, initialColor);
               pm->SetStaleColor(osg::Vec4(0.5, 0.5, 0.5, 1.0));
               pm->SetInitialAlpha(1.0);
               pm->SetStaleTime(10.0 / 60.0);
               pm->SetFadeOutTime(40.0);
               pm->SetReportTime(mGM->GetSimulationTime() - 5.0);
               mGM->AddActor(*pmap, false, false);
               osg::Vec3 currentColor = pm->CalculateCurrentColor();

               CPPUNIT_ASSERT(dtUtil::Equivalent(osg::Vec3(0.75, 0.75, 0.75), currentColor, osg::Vec3::value_type(0.01)));
            }

            void TestAddTimer()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               RefPtr<PositionMarkerActorProxy> pmap2;
               PositionMarker* pm2 = NULL;
               CreatePositionMarker(pmap2, pm2);

               pm->SetDeleteOnFadeOut(false);
               pm2->SetDeleteOnFadeOut(true);
               mGM->AddActor(*pmap, false, false);

            }

            void TestColors()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               dtCore::ColorRgbaActorProperty* prop = NULL;
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

               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_STALE_COLOR, prop);
               osg::Vec4 theColorST(0.93, 0.09, 1.0, 1.0);
               prop->SetValue(theColorST);
               CPPUNIT_ASSERT_EQUAL(theColorST, prop->GetValue());
               CPPUNIT_ASSERT_EQUAL(theColorST, pm->GetStaleColor());
            }

            void TestColorForForce()
            {
               RefPtr<PositionMarkerActorProxy> pmap;
               PositionMarker* pm = NULL;
               CreatePositionMarker(pmap, pm);

               dtCore::ColorRgbaActorProperty* prop = NULL;
               pmap->GetProperty(PositionMarkerActorProxy::PROPERTY_MARKER_COLOR, prop);
               CPPUNIT_ASSERT(prop != NULL);
               CPPUNIT_ASSERT(prop->IsReadOnly());

               osg::Vec4 theColor;

               pm->SetForceAffiliation(BaseEntityActorProxy::ForceEnum::FRIENDLY);
               theColor = pm->GetInitialColorWithAlpha();
               osg::Vec4 expectedColorF(pm->GetFriendlyColor());
               expectedColorF.a() = pm->GetInitialAlpha();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorF);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorF);

               pm->SetForceAffiliation(BaseEntityActorProxy::ForceEnum::NEUTRAL);
               theColor = pm->GetInitialColorWithAlpha();
               osg::Vec4 expectedColorN(pm->GetNeutralColor());
               expectedColorN.a() = pm->GetInitialAlpha();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorN);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorN);

               pm->SetForceAffiliation(BaseEntityActorProxy::ForceEnum::OPPOSING);
               theColor = pm->GetInitialColorWithAlpha();
               osg::Vec4 expectedColorOP(pm->GetOpposingColor());
               expectedColorOP.a() = pm->GetInitialAlpha();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP);

               pm->SetForceAffiliation(BaseEntityActorProxy::ForceEnum::INSURGENT);
               theColor = pm->GetInitialColorWithAlpha();
               osg::Vec4 expectedColorOP2(pm->GetOpposingColor());
               expectedColorOP2.a() = pm->GetInitialAlpha();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP2);
               theColor = prop->GetValue();
               CPPUNIT_ASSERT_EQUAL(theColor, expectedColorOP2);

               pm->SetForceAffiliation(BaseEntityActorProxy::ForceEnum::OTHER);
               theColor = pm->GetInitialColorWithAlpha();
               osg::Vec4 expectedColorOT(pm->GetOtherColor());
               expectedColorOT.a() = pm->GetInitialAlpha();
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
