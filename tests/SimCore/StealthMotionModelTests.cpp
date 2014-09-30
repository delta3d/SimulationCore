/* -*-c++-*-
* Simulation Core - StealthMotionModelTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, Alion Science and Technology Corporation
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
* @author Chris Rodgers
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtUtil/macros.h>
#include <dtCore/camera.h>
#include <dtCore/infiniteterrain.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/transform.h>
#include <dtCore/transformable.h>
#include <dtCore/deltawin.h>
#include <SimCore/StealthMotionModel.h>
#include <osg/io_utils>
#include <dtABC/application.h>

#include <UnitTestMain.h>

using namespace SimCore;



////////////////////////////////////////////////////////////////////////////////
// TEST OBJECT CODE
////////////////////////////////////////////////////////////////////////////////
class StealthMotionModelTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(StealthMotionModelTests);

      CPPUNIT_TEST(TestProperties);
      CPPUNIT_TEST(TestEndPosition);
      CPPUNIT_TEST(TestExclusiveCollision);
      CPPUNIT_TEST(TestSpeedLimits);

   CPPUNIT_TEST_SUITE_END();

   public:

      //////////////////////////////////////////////////////////////////////////
      void setUp()
      {
         mTerrain = new dtCore::InfiniteTerrain( "Ground" );
         mTerrain->SetBuildDistance(1500.0f);
         mTerrain->SetSegmentDivisions(64);

         mApp = &GetGlobalApplication();

         mApp->GetScene()->AddChild(mTerrain.get());

         mTarget = new dtCore::Transformable();
         mApp->GetScene()->AddChild(mTarget.get());

         mMotionModel = new SimCore::StealthMotionModel();
         mMotionModel->SetScene(*mApp->GetScene());
         mMotionModel->SetEnabled(true);
         mMotionModel->SetTarget( mTarget.get() );
         mMotionModel->SetCollideWithGround(true);
         mMotionModel->SetGroundClearance(1.0f);
         mMotionModel->SetMaximumFlySpeed(100.0f);
         mMotionModel->SetCollidableGeometry(mTerrain.get());

         dtCore::System::GetInstance().Config();

         dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
         dtCore::System::GetInstance().Start();
      }

      //////////////////////////////////////////////////////////////////////////
      void tearDown()
      {
         mApp = NULL;
         mMotionModel = NULL;
         mTarget = NULL;
         mTerrain = NULL;
         mTerrainAlternate = NULL;

         dtCore::System::GetInstance().Stop();
      }

      //////////////////////////////////////////////////////////////////////////
      void TestProperties()
      {
         const StealthMotionModel* motionModelConst = mMotionModel.get();

         CPPUNIT_ASSERT( &motionModelConst->GetScene() == mApp->GetScene() );

         mMotionModel->SetTarget( mTarget.get() );
         CPPUNIT_ASSERT( motionModelConst->GetTarget() != NULL );

         mMotionModel->SetCollideWithGround(true);
         CPPUNIT_ASSERT( motionModelConst->GetCollideWithGround() );
         mMotionModel->SetCollideWithGround(false);
         CPPUNIT_ASSERT( ! motionModelConst->GetCollideWithGround() );

         CPPUNIT_ASSERT( motionModelConst->GetGroundClearance() );
         mMotionModel->SetGroundClearance(5.0f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 5.0f, motionModelConst->GetGroundClearance(), 0.0f );

         mMotionModel->SetMaximumFlySpeed(200.0f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 200.0f, motionModelConst->GetMaximumFlySpeed(), 0.0f );

         // Check setting speed limits
         float testValue = 123.4567f;
         CPPUNIT_ASSERT( StealthMotionModel::DEFAULT_SPEED_LIMIT_MIN
            != StealthMotionModel::DEFAULT_SPEED_LIMIT_MAX );

         // --- Test setting the minimum speed limit.
         CPPUNIT_ASSERT( motionModelConst->GetFlySpeedLimitMin()
            == StealthMotionModel::DEFAULT_SPEED_LIMIT_MIN );
         mMotionModel->SetFlySpeedLimitMin( testValue );
         CPPUNIT_ASSERT( motionModelConst->GetFlySpeedLimitMin() == testValue );

         // --- Test setting the maximum speed limit.
         CPPUNIT_ASSERT( motionModelConst->GetFlySpeedLimitMax()
            == StealthMotionModel::DEFAULT_SPEED_LIMIT_MAX );
         mMotionModel->SetFlySpeedLimitMax( testValue );
         CPPUNIT_ASSERT( motionModelConst->GetFlySpeedLimitMax() == testValue );
      }

      //////////////////////////////////////////////////////////////////////////
      void TestEndPosition()
      {

         float elevation = mTerrain->GetHeight( 0.0f, 0.0f );
         osg::Vec3 cameraPos( 0.0f, 0.0f, elevation+100.0f );

         dtCore::Transform transform;
         transform.SetTranslation(cameraPos);
         mTarget->SetTransform(transform, dtCore::Transformable::REL_CS);

         dtCore::System::GetInstance().Step();

         for( int step = 0; step < 5; step++ )
         {
            cameraPos[2] -= mMotionModel->GetMaximumFlySpeed();

            mTarget->GetTransform(transform, dtCore::Transformable::REL_CS);
            transform.SetTranslation(cameraPos);
            mTarget->SetTransform(transform, dtCore::Transformable::REL_CS);

            mMotionModel->CollideWithGround();

            mTarget->GetTransform(transform, dtCore::Transformable::REL_CS);
            transform.GetTranslation(cameraPos);
         }

         elevation = mTerrain->GetHeight( cameraPos[0], cameraPos[1] );

         CPPUNIT_ASSERT_DOUBLES_EQUAL( (elevation+mMotionModel->GetGroundClearance()), cameraPos[2], 0.01f );

      }

      //////////////////////////////////////////////////////////////////////////
      void TestExclusiveCollision()
      {

         dtCore::RefPtr<dtCore::InfiniteTerrain> curTerrain;

         // Add a second and different overlapping terrain
         // to the scene.
         mTerrainAlternate = new dtCore::InfiniteTerrain( "Ground" );
         mTerrainAlternate->SetBuildDistance(1000.0f);
         mTerrainAlternate->SetSegmentDivisions(32);
         mTerrainAlternate->SetVerticalScale(3.0f);
         mApp->GetScene()->AddChild(mTerrainAlternate.get());
         dtCore::System::GetInstance().Step();


         // Check each terrain for exclusive collision
         for( int terrain = 0; terrain < 2; terrain++ )
         {
            // Choose the terrain for this iteration
            if(terrain == 0)
               curTerrain = mTerrain;
            else
               curTerrain = mTerrainAlternate;

            // Setup the exclusive collider geometry
            mMotionModel->SetCollidableGeometry(curTerrain.get());


            // Start at a point on the current terrain
            float elevation = curTerrain->GetHeight( 0.0f, 0.0f );
            osg::Vec3 cameraPos( 0.0f, 0.0f, elevation+100.0f );

            // Update viewer position
            dtCore::Transform transform;
            transform.SetTranslation(cameraPos);
            mTarget->SetTransform(transform, dtCore::Transformable::REL_CS);


            for( int step = 0; step < 5; step++ )
            {
               // Try to move the viewer through the terrain
               cameraPos[2] -= mMotionModel->GetMaximumFlySpeed();

               // Update the position
               mTarget->GetTransform(transform, dtCore::Transformable::REL_CS);
               transform.SetTranslation(cameraPos);
               mTarget->SetTransform(transform, dtCore::Transformable::REL_CS);

               // Collide with the ground and adjust viewer position
               mMotionModel->CollideWithGround();

               // Copy the position information
               mTarget->GetTransform(transform, dtCore::Transformable::REL_CS);
               transform.GetTranslation(cameraPos);
            }

            // Find the elevation of the terrain at the current position
            elevation = curTerrain->GetHeight( cameraPos[0], cameraPos[1] );

            CPPUNIT_ASSERT_DOUBLES_EQUAL( (elevation+mMotionModel->GetGroundClearance()), cameraPos[2], 0.01f );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void TestSpeedLimits()
      {
         // Test MIN clamping.
         float testSpeed = StealthMotionModel::DEFAULT_SPEED_LIMIT_MIN - 10.0f;
         mMotionModel->SetMaximumFlySpeed( testSpeed );
         CPPUNIT_ASSERT( mMotionModel->GetMaximumFlySpeed() == testSpeed );

         // Step the system which will call OnMessage on the motion model.
         // The update caused by OnMessage will cap the speed limit of the
         // motion models currently set speed.
         dtCore::System::GetInstance().Step();

         CPPUNIT_ASSERT( mMotionModel->GetMaximumFlySpeed()
            == StealthMotionModel::DEFAULT_SPEED_LIMIT_MIN );


         // Test MAX clamping
         testSpeed = StealthMotionModel::DEFAULT_SPEED_LIMIT_MAX + 10.0f;
         mMotionModel->SetMaximumFlySpeed( testSpeed );
         CPPUNIT_ASSERT( mMotionModel->GetMaximumFlySpeed() == testSpeed );

         // Step the system which will call OnMessage on the motion model.
         dtCore::System::GetInstance().Step();

         CPPUNIT_ASSERT( mMotionModel->GetMaximumFlySpeed()
            == StealthMotionModel::DEFAULT_SPEED_LIMIT_MAX );
      }


   private:

      dtCore::RefPtr<SimCore::StealthMotionModel> mMotionModel;
      dtCore::RefPtr<dtCore::InfiniteTerrain> mTerrain;
      dtCore::RefPtr<dtCore::InfiniteTerrain> mTerrainAlternate;
      dtCore::RefPtr<dtCore::Transformable> mTarget;
      dtCore::RefPtr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthMotionModelTests);
