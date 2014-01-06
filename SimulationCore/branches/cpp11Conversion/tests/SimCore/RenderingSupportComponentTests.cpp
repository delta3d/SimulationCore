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

#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <SimCore/Components/RenderingSupportComponent.h>
#include <dtGame/gamemanager.h>
#include <dtCore/system.h>
#include <dtCore/scene.h>

#include <dtUtil/refcountedbase.h>
#include <dtUtil/refcountedbase.h>
#include <dtUtil/mathdefines.h>

#include <UnitTestMain.h>

#include <osg/StateSet>
#include <osg/io_utils>

namespace SimCore
{
   namespace Components
   {
      class TestRenderFeature: public RenderingSupportComponent::RenderFeature
      {
      public:
         virtual void SetEnable(bool pEnable) {}
         virtual void Init(osg::Group* parent, dtCore::Camera* cam) {};
         virtual void Update() {};

      };

      class RenderingSupportComponentTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(RenderingSupportComponentTests);

         CPPUNIT_TEST(TestDynamicLightClass);
         CPPUNIT_TEST(TestSpotLightClass);
         CPPUNIT_TEST(TestAddRemoveLight);
         CPPUNIT_TEST(TestNVGS);
         CPPUNIT_TEST(TestEnableCullVisitorAndStaticTerrainPhysics);
         CPPUNIT_TEST(TestAutoDeleteLights);
         CPPUNIT_TEST(TestTransformDynamicLights);
         CPPUNIT_TEST(TestTransformSpotLights);
         CPPUNIT_TEST(TestSortAndSetUniforms);

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
            mRenderingSupportComponent = nullptr;

            if (mGM.valid())
            {
               mGM->DeleteAllActors(true);
               mGM = nullptr;
            }
            dtCore::System::GetInstance().Stop();
         }

         void TestDynamicLightClass()
         {
            std::shared_ptr<RenderingSupportComponent::DynamicLight> light1 = new RenderingSupportComponent::DynamicLight;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Dynamic lights should default to omni-directional",
                     RenderingSupportComponent::LightType::OMNI_DIRECTIONAL, light1->GetLightType());
            TestSingleLight(*light1);
         }

         void TestSpotLightClass()
         {
            std::shared_ptr<RenderingSupportComponent::SpotLight> light1 = new RenderingSupportComponent::SpotLight;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Spot lights should default to omni-directional",
                     RenderingSupportComponent::LightType::SPOT_LIGHT, light1->GetLightType());
            TestSingleLight(*light1);

            CPPUNIT_ASSERT(!light1->mUseAbsoluteDirection);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5f, light1->mSpotExponent, 0.01f);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.75f, light1->mSpotCosCutoff, 0.01f);

            CPPUNIT_ASSERT_EQUAL(osg::Vec3(0.0f, 1.0f, 0.0f), light1->mDirection);

            CPPUNIT_ASSERT_EQUAL(osg::Vec3(0.0f, 1.0f, 0.0f), light1->mCurrentDirection);
         }

         void TestAddRemoveLight()
         {
            std::shared_ptr<RenderingSupportComponent::SpotLight> light1 = new RenderingSupportComponent::SpotLight;
            RenderingSupportComponent::LightID id = mRenderingSupportComponent->AddDynamicLight(light1.get());
            CPPUNIT_ASSERT_EQUAL(id, light1->GetId());
            std::shared_ptr<RenderingSupportComponent::DynamicLight> light1Ret = mRenderingSupportComponent->GetDynamicLight(id);
            CPPUNIT_ASSERT(light1.get() == light1Ret.get());
            light1Ret = mRenderingSupportComponent->GetDynamicLight(id + 1);
            CPPUNIT_ASSERT(!light1Ret.valid());

            std::weak_ptr<RenderingSupportComponent::SpotLight> light1ob = light1.get();

            light1 = nullptr;
            CPPUNIT_ASSERT(light1ob.valid());

            mRenderingSupportComponent->RemoveDynamicLight(id);
            CPPUNIT_ASSERT(!light1ob.valid());

            //Do it twice to make sure it doesn't crash on ids it doesn't have.
            mRenderingSupportComponent->RemoveDynamicLight(id);
            //Do it on an id that never existed to make sure it doesn't crash.
            mRenderingSupportComponent->RemoveDynamicLight(id + 1);
         }

         void TestNVGS()
         {
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetNVGS() == nullptr);
            CPPUNIT_ASSERT(!mRenderingSupportComponent->GetEnableNVGS());
            mRenderingSupportComponent->SetEnableNVGS(true);
            CPPUNIT_ASSERT_MESSAGE("NO nvgs render feature was set, so it should not be possible to enable the NVGS.",
                     !mRenderingSupportComponent->GetEnableNVGS());

            mRenderingSupportComponent->SetNVGS(new TestRenderFeature);
            std::weak_ptr<const RenderingSupportComponent::RenderFeature> rndrFeatOb = mRenderingSupportComponent->GetNVGS();
            CPPUNIT_ASSERT(rndrFeatOb.valid());

            mRenderingSupportComponent->SetEnableNVGS(true);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetEnableNVGS());

            mRenderingSupportComponent->SetNVGS(nullptr);
            CPPUNIT_ASSERT(!mRenderingSupportComponent->GetEnableNVGS());
            CPPUNIT_ASSERT(!rndrFeatOb.valid());
            //Can't really test the cleanup because render features don't have a shutdown, just an INIT.
         }

         void TestEnableCullVisitorAndStaticTerrainPhysics()
         {
            CPPUNIT_ASSERT(!mRenderingSupportComponent->GetEnableCullVisitor());
            CPPUNIT_ASSERT(!mRenderingSupportComponent->GetEnableStaticTerrainPhysics());

            mRenderingSupportComponent->SetEnableStaticTerrainPhysics(true);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetEnableStaticTerrainPhysics());
            mRenderingSupportComponent->SetEnableStaticTerrainPhysics(false);
            CPPUNIT_ASSERT(!mRenderingSupportComponent->GetEnableStaticTerrainPhysics());

            mRenderingSupportComponent->SetEnableCullVisitor(true);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetEnableCullVisitor());
            mRenderingSupportComponent->SetEnableCullVisitor(false);
            CPPUNIT_ASSERT(!mRenderingSupportComponent->GetEnableCullVisitor());

            mRenderingSupportComponent->SetEnableStaticTerrainPhysics(true);
            mRenderingSupportComponent->SetEnableCullVisitor(true);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetEnableCullVisitor());
            CPPUNIT_ASSERT_MESSAGE("Enabling the cull visitor, should disable static terrain physics.",
                     !mRenderingSupportComponent->GetEnableStaticTerrainPhysics());

            mRenderingSupportComponent->SetEnableCullVisitor(true);
            mRenderingSupportComponent->SetEnableStaticTerrainPhysics(true);
            CPPUNIT_ASSERT_MESSAGE("Enabling static terrain physics should disable the cull visitor.",
                     !mRenderingSupportComponent->GetEnableCullVisitor());
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetEnableStaticTerrainPhysics());
         }

         void TestAutoDeleteLights()
         {
            std::shared_ptr<RenderingSupportComponent::SpotLight> light1 = new RenderingSupportComponent::SpotLight;
            std::shared_ptr<RenderingSupportComponent::SpotLight> light2 = new RenderingSupportComponent::SpotLight;
            std::shared_ptr<RenderingSupportComponent::SpotLight> light3 = new RenderingSupportComponent::SpotLight;
            std::shared_ptr<RenderingSupportComponent::SpotLight> light4 = new RenderingSupportComponent::SpotLight;

            light1->mAutoDeleteAfterMaxTime = true;
            light1->mMaxTime = 10.0f;

            std::shared_ptr<dtCore::Transformable> tformable = new dtCore::Transformable;
            light2->mAutoDeleteLightOnTargetNull = true;
            light2->mTarget = tformable.get();

            light3->mFadeOut = true;
            light3->mIntensity = 1.0f;
            light3->mFadeOutTime = 11.0f;

            mRenderingSupportComponent->AddDynamicLight(light1.get());
            mRenderingSupportComponent->AddDynamicLight(light2.get());
            mRenderingSupportComponent->AddDynamicLight(light3.get());
            mRenderingSupportComponent->AddDynamicLight(light4.get());

            mRenderingSupportComponent->TimeoutAndDeleteLights(3.0f);

            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light1->GetId()) == light1.get());
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light2->GetId()) == light2.get());
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light3->GetId()) == light3.get());
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light4->GetId()) == light4.get());

            CPPUNIT_ASSERT_MESSAGE("the fading light intensity should have decreased some.", light3->mIntensity < 1.0f);
            float oldIntensity = light3->mIntensity;

            tformable = nullptr;

            mRenderingSupportComponent->TimeoutAndDeleteLights(7.1f);

            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light1->GetId()) == nullptr);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light2->GetId()) == nullptr);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light3->GetId()) == light3.get());
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light4->GetId()) == light4.get());

            CPPUNIT_ASSERT_MESSAGE("the fading light intensity should have decreased some more.", light3->mIntensity < oldIntensity);

            mRenderingSupportComponent->TimeoutAndDeleteLights(2.0f);

            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light1->GetId()) == nullptr);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light2->GetId()) == nullptr);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light3->GetId()) == nullptr);
            CPPUNIT_ASSERT(mRenderingSupportComponent->GetDynamicLight(light4->GetId()) == light4.get());

            CPPUNIT_ASSERT_MESSAGE("the fading light intensity should less that or equal to zero.", light3->mIntensity <= 0.0f);
         }

         void TestTransformDynamicLights()
         {
            std::shared_ptr<RenderingSupportComponent::DynamicLight> light1 = new RenderingSupportComponent::DynamicLight;
            TestTransformSingleLight(*light1);
         }

         void TestTransformSpotLights()
         {
            std::shared_ptr<RenderingSupportComponent::SpotLight> light1 = new RenderingSupportComponent::SpotLight;
            light1->mUseAbsoluteDirection = false;
            TestTransformSingleLight(*light1);

            //Note: this has to match the one is TestTransformSingleLight
            const osg::Vec3 rot(-100.0f, 32.2f, 112.2f);
            dtCore::Transform xform;
            xform.SetRotation(rot);
            osg::Vec3 dir;
            xform.GetRow(1, dir);

            CPPUNIT_ASSERT(dtUtil::Equivalent(dir, light1->mCurrentDirection, 0.0001f));

            light1->mUseAbsoluteDirection = true;

            TestTransformSingleLight(*light1);

            //Note: this has to match the one is TestTransformSingleLight
            CPPUNIT_ASSERT_EQUAL(light1->mDirection, light1->mCurrentDirection);
         }

         void TestSortAndSetUniforms()
         {
            mRenderingSupportComponent->SetEnableDynamicLights(true);
            mRenderingSupportComponent->SetMaxSpotLights(4);
            mRenderingSupportComponent->SetMaxDynamicLights(4);

            std::shared_ptr<RenderingSupportComponent::DynamicLight> lights[8];

            for (unsigned i = 0; i < 4; ++i)
            {
               lights[i] = new RenderingSupportComponent::SpotLight;
               lights[i]->mAttenuation = osg::Vec3(float(i), float(i), float(i));
               lights[i]->mColor = osg::Vec3(float(i), float(i), float(i));
            }

            for (unsigned i = 4; i < 8; ++i)
            {
               lights[i] = new RenderingSupportComponent::DynamicLight;
               lights[i]->mAttenuation = osg::Vec3(float(i), float(i), float(i));
               lights[i]->mColor = osg::Vec3(float(i), float(i), float(i));
            }

            for (unsigned i = 0; i < 8; ++i)
            {
               lights[i]->mIntensity = 1.0;
               lights[i]->mPosition = osg::Vec3(0.0, float(i), 0.0);
               mRenderingSupportComponent->AddDynamicLight(lights[i]);
            }

            dtCore::Transform xform;
            xform.MakeIdentity();
            GetGlobalApplication().GetCamera()->SetTransform(xform);

            mRenderingSupportComponent->TransformAndSortLights();
            osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
            osg::Uniform* lightArray = ss->getOrCreateUniform("dynamic lights", osg::Uniform::FLOAT_VEC4, mRenderingSupportComponent->GetMaxDynamicLights() * 3);
            osg::Uniform* spotLightArray = ss->getOrCreateUniform("spot lights", osg::Uniform::FLOAT_VEC4, mRenderingSupportComponent->GetMaxSpotLights() * 4);
            mRenderingSupportComponent->UpdateDynamicLightUniforms(lightArray, spotLightArray);

            for (unsigned i = 0; i < 4; ++i)
            {
               RenderingSupportComponent::DynamicLight* dl = lights[i + 4].get();
               osg::Vec4 vec;
               lightArray->getElement((i * 3), vec);
               CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mPosition, dl->mIntensity), vec);
               lightArray->getElement((i * 3) + 1, vec);
               CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mColor, 1.0), vec);
               lightArray->getElement((i * 3) + 2, vec);
               CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mAttenuation, 1.0), vec);
            }

            for (unsigned i = 0; i < 4; ++i)
            {
               RenderingSupportComponent::SpotLight* dl = dynamic_cast<RenderingSupportComponent::SpotLight*>(lights[i].get());
               osg::Vec4 vec;
               spotLightArray->getElement((i * 4), vec);
               CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mPosition, dl->mIntensity), vec);
               spotLightArray->getElement((i * 4) + 1, vec);
               CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mColor, 1.0), vec);
               spotLightArray->getElement((i * 4) + 2, vec);
               CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mAttenuation, dl->mSpotExponent), vec);
            }

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
            CPPUNIT_ASSERT_MESSAGE("Dynamic lights should have no target by default", testLight.mTarget == nullptr);
         }

         void TestTransformSingleLight(RenderingSupportComponent::DynamicLight& testLight)
         {
            std::shared_ptr<dtCore::Transformable> xformable = new dtCore::Transformable;

            mGM->GetScene().AddChild(xformable.get());

            testLight.mTarget = xformable.get();

            dtCore::Transform xform;
            const osg::Vec3 pos(33.3f, -98.7f, 112.2f);
            //If you change this, you also have to change the one in TestTransformSpotLights;
            const osg::Vec3 rot(-100.0f, 32.2f, 112.2f);
            xform.SetTranslation(pos);
            xform.SetRotation(rot);

            xformable->SetTransform(xform);

            mRenderingSupportComponent->AddDynamicLight(&testLight);
            mRenderingSupportComponent->TransformAndSortLights();

            CPPUNIT_ASSERT_EQUAL(pos, testLight.mPosition);

         }

         std::shared_ptr<dtGame::GameManager> mGM;
         std::shared_ptr<SimCore::Components::RenderingSupportComponent> mRenderingSupportComponent;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(RenderingSupportComponentTests);
   }
}
