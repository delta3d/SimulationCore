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
 * Bradley Anderegg
 */

#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <SimCore/Components/VolumeRenderingComponent.h>
#include <dtGame/gamemanager.h>
#include <dtCore/system.h>
#include <dtCore/scene.h>

#include <dtUtil/refcountedbase.h>
#include <dtUtil/refcountedbase.h>
#include <dtUtil/mathdefines.h>

#include <UnitTestMain.h>

#include <osg/StateSet>
#include <osg/io_utils>
#include <osg/MatrixTransform>
namespace SimCore
{
   namespace Components
   {

      class VolumeRenderingComponentTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(VolumeRenderingComponentTests);

         CPPUNIT_TEST(TestParticleVolumeDrawableClass);
         CPPUNIT_TEST(TestShapeVolumeRecordClass);
         CPPUNIT_TEST(TestAddRemoveVolume);
         CPPUNIT_TEST(TestAutoDeleteVolumes);
         //CPPUNIT_TEST(TestTransformVolumes);

         CPPUNIT_TEST_SUITE_END();

      public:

         void setUp()
         {
            dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
            dtCore::System::GetInstance().Start();
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

            mGM->SetApplication(GetGlobalApplication());
            mVolumeRenderingComponent = new SimCore::Components::VolumeRenderingComponent();
            mGM->AddComponent(*mVolumeRenderingComponent, dtGame::GameManager::ComponentPriority::NORMAL);
         }

         void tearDown()
         {
            mVolumeRenderingComponent = nullptr;

            if (mGM.valid())
            {
               mGM->DeleteAllActors(true);
               mGM = nullptr;
            }
            dtCore::System::GetInstance().Stop();
         }

         void TestParticleVolumeDrawableClass()
         {
            std::shared_ptr<VolumeRenderingComponent::ParticleVolumeDrawable> volume1 = new VolumeRenderingComponent::ParticleVolumeDrawable;
            
            CPPUNIT_ASSERT_EQUAL_MESSAGE("ParticleVolumeDrawable class should start with zero particles",
                     unsigned(0), volume1->GetNumParticles());

            CPPUNIT_ASSERT_MESSAGE("ParticleVolumeDrawable radius should not be zero",
                     volume1->GetParticleRadius() != 0);

            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> record1 = new VolumeRenderingComponent::ShapeVolumeRecord;
            record1->mRadius.set(1.0f, 0.0f, 0.0f);
            volume1->Init(25, record1);

            CPPUNIT_ASSERT_EQUAL_MESSAGE("ParticleVolumeDrawable should have created 25 particles",
               unsigned(25), volume1->GetNumParticles());


            osg::BoundingBox b;
            b = volume1->computeBound();
           
            CPPUNIT_ASSERT_MESSAGE("ParticleVolumeDrawable with radius 1 and particle radius 5 should should have a bounding volume with no more then 6 m radius",
               b._min.x() <= 6.0f);
            CPPUNIT_ASSERT_MESSAGE("ParticleVolumeDrawable with radius 1 and particle radius 5 should should have a bounding volume with no more then 6 m radius",
               b._min.y() <= 6.0f);
            CPPUNIT_ASSERT_MESSAGE("ParticleVolumeDrawable with radius 1 and particle radius 5 should should have a bounding volume with no more then 6 m radius",
               b._min.z() <= 6.0f);

            CPPUNIT_ASSERT_MESSAGE("ParticleVolumeDrawable with radius 1 and particle radius 5 should should have a bounding volume with no more then 6 m radius",
               b._max.x() <= 6.0f);
            CPPUNIT_ASSERT_MESSAGE("ParticleVolumeDrawable with radius 1 and particle radius 5 should should have a bounding volume with no more then 6 m radius",
               b._max.x() <= 6.0f);
            CPPUNIT_ASSERT_MESSAGE("ParticleVolumeDrawable with radius 1 and particle radius 5 should should have a bounding volume with no more then 6 m radius",
               b._max.x() <= 6.0f);


            char str[255];
            for(unsigned i = 0; i < 25; ++i)
            {
               osg::Vec4 val4 = volume1->GetPointLocation(i);
               osg::Vec3 val(val4.x(), val4.y(), val4.z());
               snprintf(str, 255, "ParticleVolumeDrawable points should be within our radius. Index %i point distance %f", i, val.length());
               //test some points to see if they are within our radius
               CPPUNIT_ASSERT_MESSAGE(str,
                     val.length() <= 1.0f);
            }

            CPPUNIT_ASSERT_EQUAL_MESSAGE("ParticleVolumeDrawable should not crash if a point is accessed that is out of bounds",
               volume1->GetPointLocation(500), osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));

         }

         void TestShapeVolumeRecordClass()
         {
            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume1 = new VolumeRenderingComponent::ShapeVolumeRecord;
          
            TestSingleVolume(*volume1);
         }

         void TestAddRemoveVolume()
         {
            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume1 = new VolumeRenderingComponent::ShapeVolumeRecord();
            VolumeRenderingComponent::ShapeRecordId id = mVolumeRenderingComponent->CreateShapeVolume(volume1.get());
            CPPUNIT_ASSERT_EQUAL(id, volume1->GetId());
            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume1Ret = mVolumeRenderingComponent->FindShapeVolumeById(id);
            CPPUNIT_ASSERT(volume1.get() == volume1Ret.get());
            volume1Ret = mVolumeRenderingComponent->FindShapeVolumeById(id + 1);
            CPPUNIT_ASSERT_MESSAGE("When passed an invalid Id the VolumeRenderingComponent should return nullptr", !volume1Ret.valid());

            std::weak_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume1ob = volume1.get();

            volume1 = nullptr;
            CPPUNIT_ASSERT(volume1ob.valid());

            mVolumeRenderingComponent->RemoveShapeVolume(volume1ob.get());
            CPPUNIT_ASSERT(!volume1ob.valid());

         }


         void TestAutoDeleteVolumes()
         {
            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume1 = new VolumeRenderingComponent::ShapeVolumeRecord;
            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume2 = new VolumeRenderingComponent::ShapeVolumeRecord;
            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume3 = new VolumeRenderingComponent::ShapeVolumeRecord;
            std::shared_ptr<VolumeRenderingComponent::ShapeVolumeRecord> volume4 = new VolumeRenderingComponent::ShapeVolumeRecord;

            volume1->mAutoDeleteAfterMaxTime = true;
            volume1->mMaxTime = 10.0f;

            std::shared_ptr<dtCore::Transformable> tformable = new dtCore::Transformable;
            volume2->mAutoDeleteOnTargetNull = true;
            volume2->mTarget = tformable->GetOSGNode();

            volume3->mFadeOut = true;
            volume3->mIntensity = 1.0f;
            volume3->mFadeOutTime = 11.0f;

            mVolumeRenderingComponent->CreateShapeVolume(volume1.get());
            mVolumeRenderingComponent->CreateShapeVolume(volume2.get());
            mVolumeRenderingComponent->CreateShapeVolume(volume3.get());
            mVolumeRenderingComponent->CreateShapeVolume(volume4.get());

            mVolumeRenderingComponent->TimeoutAndDeleteVolumes(3.0f);

            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume1->GetId()) == volume1.get());
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume2->GetId()) == volume2.get());
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume3->GetId()) == volume3.get());
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume4->GetId()) == volume4.get());

            CPPUNIT_ASSERT_MESSAGE("the fading volume intensity should have decreased some.", volume3->mIntensity < 1.0f);
            float oldIntensity = volume3->mIntensity;

            tformable = nullptr;

            mVolumeRenderingComponent->TimeoutAndDeleteVolumes(7.1f);

            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume1->GetId()) == nullptr);
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume2->GetId()) == nullptr);
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume3->GetId()) == volume3.get());
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume4->GetId()) == volume4.get());

            CPPUNIT_ASSERT_MESSAGE("the fading volume intensity should have decreased some more.", volume3->mIntensity < oldIntensity);

            mVolumeRenderingComponent->TimeoutAndDeleteVolumes(2.0f);

            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume1->GetId()) == nullptr);
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume2->GetId()) == nullptr);
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume3->GetId()) == nullptr);
            CPPUNIT_ASSERT(mVolumeRenderingComponent->FindShapeVolumeById(volume4->GetId()) == volume4.get());

            CPPUNIT_ASSERT_MESSAGE("the fading volume intensity should less that or equal to zero.", volume3->mIntensity <= 0.0f);
         }

         //void TestTransformVolumes()
         //{
         //   std::shared_ptr<VolumeRenderingComponent::ShapeVolume> volume1 = new VolumeRenderingComponent::ShapeVolume;
         //   TestTransformSinglevolume(*volume1);
         //}

         //void TestSortAndSetUniforms()
         //{
         //   mVolumeRenderingComponent->SetEnableShapeVolumes(true);
         //   mVolumeRenderingComponent->SetMaxShapeVolumeRecords(4);
         //   mVolumeRenderingComponent->SetMaxShapeVolumes(4);

         //   std::shared_ptr<VolumeRenderingComponent::ShapeVolume> volumes[8];

         //   for (unsigned i = 0; i < 4; ++i)
         //   {
         //      volumes[i] = new VolumeRenderingComponent::ShapeVolumeRecord;
         //      volumes[i]->mAttenuation = osg::Vec3(float(i), float(i), float(i));
         //      volumes[i]->mColor = osg::Vec3(float(i), float(i), float(i));
         //   }

         //   for (unsigned i = 4; i < 8; ++i)
         //   {
         //      volumes[i] = new VolumeRenderingComponent::ShapeVolume;
         //      volumes[i]->mAttenuation = osg::Vec3(float(i), float(i), float(i));
         //      volumes[i]->mColor = osg::Vec3(float(i), float(i), float(i));
         //   }

         //   for (unsigned i = 0; i < 8; ++i)
         //   {
         //      volumes[i]->mIntensity = 1.0;
         //      volumes[i]->mPosition = osg::Vec3(0.0, float(i), 0.0);
         //      mVolumeRenderingComponent->CreateShapeVolume(volumes[i]);
         //   }

         //   dtCore::Transform xform;
         //   xform.MakeIdentity();
         //   GetGlobalApplication().GetCamera()->SetTransform(xform);

         //   mVolumeRenderingComponent->TransformAndSortvolumes();
         //   osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
         //   osg::Uniform* volumeArray = ss->getOrCreateUniform("dynamic volumes", osg::Uniform::FLOAT_VEC4, mVolumeRenderingComponent->GetMaxShapeVolumes() * 3);
         //   osg::Uniform* ShapeVolumeRecordArray = ss->getOrCreateUniform("spot volumes", osg::Uniform::FLOAT_VEC4, mVolumeRenderingComponent->GetMaxShapeVolumeRecords() * 4);
         //   mVolumeRenderingComponent->UpdateShapeVolumeUniforms(volumeArray, ShapeVolumeRecordArray);

         //   for (unsigned i = 0; i < 4; ++i)
         //   {
         //      VolumeRenderingComponent::ShapeVolume* dl = volumes[i + 4].get();
         //      osg::Vec4 vec;
         //      volumeArray->getElement((i * 3), vec);
         //      CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mPosition, dl->mIntensity), vec);
         //      volumeArray->getElement((i * 3) + 1, vec);
         //      CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mColor, 1.0), vec);
         //      volumeArray->getElement((i * 3) + 2, vec);
         //      CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mAttenuation, 1.0), vec);
         //   }

         //   for (unsigned i = 0; i < 4; ++i)
         //   {
         //      VolumeRenderingComponent::ShapeVolumeRecord* dl = dynamic_cast<VolumeRenderingComponent::ShapeVolumeRecord*>(volumes[i].get());
         //      osg::Vec4 vec;
         //      ShapeVolumeRecordArray->getElement((i * 4), vec);
         //      CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mPosition, dl->mIntensity), vec);
         //      ShapeVolumeRecordArray->getElement((i * 4) + 1, vec);
         //      CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mColor, 1.0), vec);
         //      ShapeVolumeRecordArray->getElement((i * 4) + 2, vec);
         //      CPPUNIT_ASSERT_EQUAL(osg::Vec4(dl->mAttenuation, dl->mSpotExponent), vec);
         //   }

         //}

      private:

         void TestSingleVolume(const VolumeRenderingComponent::ShapeVolumeRecord& testvolume)
         {
            CPPUNIT_ASSERT_EQUAL(VolumeRenderingComponent::SPHERE, testvolume.mShapeType);
            CPPUNIT_ASSERT_EQUAL(VolumeRenderingComponent::PARTICLE_VOLUME, testvolume.mRenderMode);
            CPPUNIT_ASSERT(!testvolume.mDirtyParams);
            CPPUNIT_ASSERT(!testvolume.mDeleteMe);
            CPPUNIT_ASSERT(!testvolume.mAutoDeleteOnTargetNull);
            CPPUNIT_ASSERT(!testvolume.mAutoDeleteAfterMaxTime);
            CPPUNIT_ASSERT(!testvolume.mFadeOut);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testvolume.mIntensity, 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.15f, testvolume.mDensity, 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.15f, testvolume.mVelocity, 0.01f);
            
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testvolume.mPosition[0], 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testvolume.mPosition[1], 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testvolume.mPosition[2], 0.01f);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testvolume.mRadius[0], 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testvolume.mRadius[1], 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, testvolume.mRadius[2], 0.01f);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testvolume.mColor[0], 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testvolume.mColor[1], 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testvolume.mColor[2], 0.01f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, testvolume.mColor[3], 0.01f);

            CPPUNIT_ASSERT_EQUAL_MESSAGE("ShapeVolume should have a default shader and shader group", std::string("VolumeRenderingGroup"), testvolume.mShaderGroup);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("ShapeVolume should have a default shader and shader group", std::string("ParticleVolumeShader"), testvolume.mShaderName);
         }

         //void TestTransformSinglevolume(VolumeRenderingComponent::ShapeVolume& testvolume)
         //{
         //   std::shared_ptr<dtCore::Transformable> xformable = new dtCore::Transformable;

         //   mGM->GetScene().AddDrawable(xformable.get());

         //   testvolume.mTarget = xformable.get();

         //   dtCore::Transform xform;
         //   const osg::Vec3 pos(33.3f, -98.7f, 112.2f);
         //   //If you change this, you also have to change the one in TestTransformShapeVolumeRecords;
         //   const osg::Vec3 rot(-100.0f, 32.2f, 112.2f);
         //   xform.SetTranslation(pos);
         //   xform.SetRotation(rot);

         //   xformable->SetTransform(xform);

         //   mVolumeRenderingComponent->CreateShapeVolume(&testvolume);
         //   mVolumeRenderingComponent->TransformAndSortvolumes();

         //   CPPUNIT_ASSERT_EQUAL(pos, testvolume.mPosition);

         //}

         std::shared_ptr<dtGame::GameManager> mGM;
         std::shared_ptr<SimCore::Components::VolumeRenderingComponent> mVolumeRenderingComponent;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(VolumeRenderingComponentTests);
   }
}
