/* -*-c++-*-
 * SimulationCore
 * Copyright 2010, Alion Science and Technology
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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/EnvironmentProcessActor.h>
#include <SimCore/Actors/SimpleMovingShapeActor.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>
#include <dtCore/system.h>
#include <dtCore/namedgroupparameter.h>
#include <dtCore/namedgroupparameter.inl>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/gameactor.h>
#include <UnitTestMain.h>

namespace SimCore
{
   namespace Actors
   {

      class EnvironmentProcessActorTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(EnvironmentProcessActorTests);

         CPPUNIT_TEST(TestEnvironmentProcessActor);
         CPPUNIT_TEST(TestSimpleMovingShapeActor);

         CPPUNIT_TEST_SUITE_END();

      public:

         //////////////////////////////////////////////////////////////////////////
         void setUp()
         {
            dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
            dtCore::System::GetInstance().Start();
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
            mGM->SetApplication(GetGlobalApplication());

            mDeadReckoningComponent = new dtGame::DeadReckoningComponent();
            mGM->AddComponent(*mDeadReckoningComponent, dtGame::GameManager::ComponentPriority::NORMAL);
         }

         //////////////////////////////////////////////////////////////////////////
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

         //////////////////////////////////////////////////////////////////////////
         void TestEnvironmentProcessActor()
         {
            dtCore::RefPtr<EnvironmentProcessActorProxy> envProcAct;
            mGM->CreateActor(*EntityActorRegistry::ENVIRONMENT_PROCESS_ACTOR_TYPE, envProcAct);

            CPPUNIT_ASSERT_EQUAL(USHRT_MAX, envProcAct->GetSequenceNumber());
            CPPUNIT_ASSERT_EQUAL(USHRT_MAX, envProcAct->GetLastUpdateSequenceNumber());

            mGM->AddActor(*envProcAct, false, false);

            dtCore::RefPtr<dtCore::NamedGroupParameter> groupParam = new dtCore::NamedGroupParameter("Record");

            dtCore::RefPtr<dtCore::NamedGroupParameter> rec1 = new dtCore::NamedGroupParameter("1");
            dtCore::RefPtr<dtCore::NamedGroupParameter> rec2 = new dtCore::NamedGroupParameter("2");

            const osg::Vec3d testPos1(-1000.4, 1.0, 7.3);
            const osg::Vec3d testPos2(11.4, -1.66, -4366.88);
            const osg::Vec3f testVec1(27.4f, -3.7f, 36.22f);
            const osg::Vec3f testVec2(19.22f, 11.6f, -0.0023f);

            const osg::Vec3f testOrient(-1.11, 3.77, 6.28f);

            const float testHeight = 1.88f;
            const float testRate = 8.73f;

            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_INDEX, 7U);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_TYPE_CODE, unsigned(EnvironmentProcessActorProxy::GaussianPuffRecordType));
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_LOCATION, testPos1);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_ORIGINATION_LOCATION, testPos2);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_DIMENSION, testVec1);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_DIMENSION_RATE, testVec2);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_ORIENTATION, testOrient);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_VELOCITY, testVec1);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_ANGULAR_VELOCITY, testVec2);
            rec1->SetValue(EnvironmentProcessActorProxy::PARAM_CENTROID_HEIGHT, testHeight);

            rec2->SetValue(EnvironmentProcessActorProxy::PARAM_INDEX, 8U);
            rec2->SetValue(EnvironmentProcessActorProxy::PARAM_TYPE_CODE, unsigned(EnvironmentProcessActorProxy::BoundingSphereRecordType));
            rec2->SetValue(EnvironmentProcessActorProxy::PARAM_LOCATION, testPos1);
            rec2->SetValue(EnvironmentProcessActorProxy::PARAM_RADIUS, testHeight);
            rec2->SetValue(EnvironmentProcessActorProxy::PARAM_RADIUS_RATE, testRate);
            rec2->SetValue(EnvironmentProcessActorProxy::PARAM_VELOCITY, testVec1);
            rec2->SetValue(EnvironmentProcessActorProxy::PARAM_ANGULAR_VELOCITY, testVec2);

            groupParam->AddParameter(*rec1);
            groupParam->AddParameter(*rec2);

            envProcAct->SetSequenceNumber(1);
            CPPUNIT_ASSERT_EQUAL(1, envProcAct->GetSequenceNumber());
            CPPUNIT_ASSERT_EQUAL(USHRT_MAX, envProcAct->GetLastUpdateSequenceNumber());

         }

         void TestSimpleMovingShapeActor()
         {
            dtCore::RefPtr<SimpleMovingShapeActorProxy> movingShapeAct;
            mGM->CreateActor(*EntityActorRegistry::ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE, movingShapeAct);

            CPPUNIT_ASSERT(movingShapeAct->GetOwner().ToString().empty());
            CPPUNIT_ASSERT_EQUAL(0, movingShapeAct->GetIndex());

            mGM->AddActor(*movingShapeAct, false, false);

            dtGame::GameActor* drawable;
            movingShapeAct->GetActor(drawable);


         }
      private:
         dtCore::RefPtr<dtGame::GameManager> mGM;
         dtCore::RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;

      };

      CPPUNIT_TEST_SUITE_REGISTRATION(EnvironmentProcessActorTests);
   }
}
