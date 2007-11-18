/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology, BMH Operation.
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
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include <dtUtil/macros.h>
#include <dtCore/camera.h>
#include <dtCore/infiniteterrain.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/transformable.h>
#include <dtCore/deltawin.h>
#include <SimCore/StealthMotionModel.h>

#include <osg/io_utils>
#include <dtABC/application.h>

#ifdef DELTA_WIN32
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

class StealthMotionModelTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(StealthMotionModelTests);

      CPPUNIT_TEST(TestProperties);
      CPPUNIT_TEST(TestEndPosition);
      CPPUNIT_TEST(TestExclusiveCollision);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         // Scene needs to exist before a window
         mScene = new dtCore::Scene();

         mTerrain = new dtCore::InfiniteTerrain( "Ground" );
         mTerrain->SetBuildDistance(1500.f);
         mTerrain->SetSegmentDivisions(64);

         mScene->SetName("Ground Scene");
         mScene->AddDrawable(mTerrain.get());

         mTarget = new dtCore::Transformable();

         mMotionModel = new SimCore::StealthMotionModel();
         mMotionModel->SetScene(*mScene);
         mMotionModel->SetEnabled(true); 
         mMotionModel->SetTarget( mTarget.get() );
         mMotionModel->SetCollideWithGround(true);
         mMotionModel->SetGroundClearance(1.0f);
         mMotionModel->SetMaximumFlySpeed(100.0f);

         dtCore::System::GetInstance().Config();

         dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
         dtCore::System::GetInstance().Start();
      }
      
      void tearDown()
      {
         mMotionModel = NULL;
         if(mScene.valid())
         {
            mScene->RemoveAllDrawables();
         }
         mScene = NULL;
         mTarget = NULL;
         mTerrain = NULL;
         mTerrainAlternate = NULL;
        
         dtCore::System::GetInstance().Stop();
      }
      
      void TestProperties()
      {       
         CPPUNIT_ASSERT( mMotionModel->GetScene().GetName() == "Ground Scene" );  

         mMotionModel->SetTarget( mTarget.get() );
         CPPUNIT_ASSERT( mMotionModel->GetTarget() != NULL );   

         mMotionModel->SetCollideWithGround(true);
         CPPUNIT_ASSERT( mMotionModel->GetCollideWithGround() ); 
         mMotionModel->SetCollideWithGround(false);
         CPPUNIT_ASSERT( !mMotionModel->GetCollideWithGround() );       

         CPPUNIT_ASSERT( mMotionModel->GetGroundClearance() );
         mMotionModel->SetGroundClearance(5.0f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 5.0f, mMotionModel->GetGroundClearance(), 0.0f );

         mMotionModel->SetMaximumFlySpeed(200.0f);
         CPPUNIT_ASSERT_DOUBLES_EQUAL( 200.0f, mMotionModel->GetMaximumFlySpeed(), 0.0f ); 
         
      }
      
      void TestEndPosition()
      {

         dtCore::System::GetInstance().Step();

         float elevation = mTerrain->GetHeight( 0.0f, 0.0f );
         osg::Vec3 cameraPos( 0.0f, 0.0f, elevation+100.0f );

         dtCore::Transform transform;
         transform.SetTranslation(cameraPos);
         mTarget->SetTransform(transform, dtCore::Transformable::REL_CS);


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

      void TestExclusiveCollision()
      {

         dtCore::RefPtr<dtCore::InfiniteTerrain> curTerrain;

         // Add a second and different overlapping terrain
         // to the scene.
         mTerrainAlternate = new dtCore::InfiniteTerrain( "Ground" );
         mTerrainAlternate->SetBuildDistance(1000.f);
         mTerrainAlternate->SetSegmentDivisions(32);
         mTerrainAlternate->SetVerticalScale(3.0f);
         mScene->AddDrawable(mTerrainAlternate.get());
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


   private:
      dtCore::RefPtr<SimCore::StealthMotionModel> mMotionModel;
      dtCore::RefPtr<dtCore::Scene> mScene;
      dtCore::RefPtr<dtCore::InfiniteTerrain> mTerrain;
      dtCore::RefPtr<dtCore::InfiniteTerrain> mTerrainAlternate;
      dtCore::RefPtr<dtCore::Transformable> mTarget;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthMotionModelTests);
