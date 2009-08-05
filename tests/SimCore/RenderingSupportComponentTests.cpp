/* -*-c++-*-
 * SimulationCore
 * Copyright 2007-2008, Alion Science and Technology
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
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * David Guthrie
 */

#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include <SimCore/Components/RenderingSupportComponent.h>
#include <dtGame/gamemanager.h>
#include <dtCore/system.h>

#include <UnitTestMain.h>

#include <osg/io_utils>

namespace SimCore
{
   namespace Components
   {
      class RenderingSupportComponentTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(RenderingSupportComponentTests);

         CPPUNIT_TEST(TestDynamicLightClass);
         CPPUNIT_TEST(TestSpotLightClass);

         CPPUNIT_TEST_SUITE_END();

      public:

         void setUp()
         {
            dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
            dtCore::System::GetInstance().Start();
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

            mGM->SetApplication(GetGlobalApplication());
            mRenderingSupportComponent = new SimCore::Components::RenderingSupportComponent();
            mGM->AddComponent(*mRenderingSupportComponent, dtGame::GameManager::ComponentPriority::NORMAL);
         }

         void tearDown()
         {
            mRenderingSupportComponent = NULL;

            if (mGM.valid())
            {
               mGM->DeleteAllActors(true);
               mGM = NULL;
            }
            dtCore::System::GetInstance().Stop();
         }

         void TestDynamicLightClass()
         {
            dtCore::RefPtr<RenderingSupportComponent::DynamicLight> light1 = new RenderingSupportComponent::DynamicLight;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Dynamic lights should default to omni-directional",
                     RenderingSupportComponent::LightType::OMNI_DIRECTIONAL, light1->GetLightType());
            TestSingleLight(*light1);
         }

         void TestSpotLightClass()
         {
            dtCore::RefPtr<RenderingSupportComponent::SpotLight> light1 = new RenderingSupportComponent::SpotLight;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Spot lights should default to omni-directional",
                     RenderingSupportComponent::LightType::SPOT_LIGHT, light1->GetLightType());
            TestSingleLight(*light1);
         }

      private:

         void TestSingleLight(const RenderingSupportComponent::DynamicLight& testLight)
         {
            CPPUNIT_ASSERT(!testLight.mDeleteMe);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testLight.mIntensity, 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testLight.mSaturationIntensity, 0.01f);
            CPPUNIT_ASSERT_EQUAL(osg::Vec3(1.0f, 1.0f, 1.0f), testLight.mColor);
            CPPUNIT_ASSERT_EQUAL(osg::Vec3(0.0f, 0.0f, 0.0f), testLight.mPosition);
            CPPUNIT_ASSERT_EQUAL(osg::Vec3(1.0f, 0.01f, 0.001f), testLight.mAttenuation);
            CPPUNIT_ASSERT(!testLight.mFlicker);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1f, testLight.mFlickerScale, 0.01f);
            CPPUNIT_ASSERT(!testLight.mAutoDeleteAfterMaxTime);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testLight.mMaxTime, 0.01f);
            CPPUNIT_ASSERT(!testLight.mFadeOut);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testLight.mFadeOutTime, 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testLight.mRadius, 0.01f);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The light id should be the same as the current static counter value", testLight.GetCurrentLightIdCounter(), testLight.GetId());
            CPPUNIT_ASSERT(!testLight.mAutoDeleteLightOnTargetNull);
            CPPUNIT_ASSERT_MESSAGE("Dynamic lights should have no target by default", testLight.mTarget == NULL);
         }

         dtCore::RefPtr<dtGame::GameManager> mGM;
         dtCore::RefPtr<SimCore::Components::RenderingSupportComponent> mRenderingSupportComponent;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(RenderingSupportComponentTests);
   }
}
