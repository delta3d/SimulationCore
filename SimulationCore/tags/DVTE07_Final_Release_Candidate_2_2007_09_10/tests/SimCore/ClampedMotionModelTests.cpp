/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include <dtCore/camera.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/transformable.h>
#include <dtCore/deltawin.h>
#include <dtCore/logicalinputdevice.h>
#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <SimCore/ClampedMotionModel.h>

#include <osg/io_utils>
#include <dtABC/application.h>

#if (defined (WIN32) || defined (_WIN32) || defined (__WIN32__))
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

//////////////////////////////////////////////////////////////
// TESTABLE SUB-CLASS
//////////////////////////////////////////////////////////////
class TestClampedMotionModel : public SimCore::ClampedMotionModel
{
public:
   TestClampedMotionModel( dtCore::Keyboard* keyboard, dtCore::Mouse* mouse );

   void SetKeyHeld( bool held ) { mKeyHeld = held; }

   // This function is the primary reason for sub-classing.
   // This helps simulate the modifier key being pressed
   // when a free-look key is required for motion.
   virtual bool IsKeyHeld() const;

protected: 
   virtual ~TestClampedMotionModel() {}

private:
   bool mKeyHeld;
};

//////////////////////////////////////////////////////////////
TestClampedMotionModel::TestClampedMotionModel(
    dtCore::Keyboard* keyboard, dtCore::Mouse* mouse )
    : SimCore::ClampedMotionModel( keyboard, mouse )
{
}

//////////////////////////////////////////////////////////////
bool TestClampedMotionModel::IsKeyHeld() const
{
   return mKeyHeld;
}



//////////////////////////////////////////////////////////////
// UNIT TESTS
//////////////////////////////////////////////////////////////
class ClampedMotionModelTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(ClampedMotionModelTests);

      CPPUNIT_TEST(TestProperties);
      CPPUNIT_TEST(TestTransforms);

   CPPUNIT_TEST_SUITE_END();


   public:

      //////////////////////////////////////////////////////////////
      void setUp()
      {
         // Scene needs to exist before a window
         mScene = new dtCore::Scene();

         // A window & camera are needed to allow terrain
         // to generate geometry.
         mApp = new dtABC::Application("config.xml");
         mApp->GetWindow()->SetPosition(0, 0, 50, 50);

         mGM = new dtGame::GameManager(*mScene);
         mGM->SetApplication( *mApp );

         mCamera = new dtCore::Camera();
         mCamera->SetScene(mScene.get());
         mCamera->SetWindow(mApp->GetWindow());

         mTarget = new dtCore::Transformable();
         mAttachable = new dtCore::Transformable();

         mMotionModel = new TestClampedMotionModel( mApp->GetKeyboard(), mApp->GetMouse() );
         mMotionModel->SetKeyHeld(false); // do not start simulating the key being held
         mMotionModel->SetRecenterMouse(false); // avoid messing with the pointer during run-time
         mMotionModel->SetMaximumMouseTurnSpeed(15.0f); // code clamps mouse speed to 15 degrees

         // Setup the transformable hierarchy
         mMotionModel->SetTarget( mAttachable.get() );
         mAttachable->AddChild( mCamera.get() );
         mTarget->AddChild( mAttachable.get() );

         dtCore::System::GetInstance().Config();

         dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
         dtCore::System::GetInstance().Start();

         mMotionModel->SetEnabled(true);
      }
      
      //////////////////////////////////////////////////////////////
      void tearDown()
      {
         mMotionModel = NULL;
         if(mScene.valid())
         {
            mScene->RemoveAllDrawables();
         }
         mScene = NULL;
         mCamera->SetScene(NULL);
         mCamera->SetWindow(NULL);
         mCamera = NULL;

         mGM->DeleteAllActors(true);
         mGM = NULL;

         mApp = NULL;
         dtCore::System::GetInstance().Stop();
      }
      
      //////////////////////////////////////////////////////////////
      void TestProperties()
      {       
         mMotionModel->SetTarget( NULL );
         CPPUNIT_ASSERT( mMotionModel->GetTarget() == NULL ); 
         mMotionModel->SetTarget( mTarget.get() );
         CPPUNIT_ASSERT( mMotionModel->GetTarget() != NULL ); 

         float limit = 30.0f;
         CPPUNIT_ASSERT( mMotionModel->GetLeftRightLimit() < 0.0f ); 
         mMotionModel->SetLeftRightLimit( limit );
         CPPUNIT_ASSERT( mMotionModel->GetLeftRightLimit() == limit ); 

         CPPUNIT_ASSERT( mMotionModel->GetUpDownLimit() < 0.0f ); 
         mMotionModel->SetUpDownLimit( limit );
         CPPUNIT_ASSERT( mMotionModel->GetUpDownLimit() == limit ); 

         bool value = mMotionModel->GetFreeLookByKey();
         mMotionModel->SetFreeLookByKey( ! value );
         CPPUNIT_ASSERT( mMotionModel->GetFreeLookByKey() == ! value );

         value = mMotionModel->GetFreeLookByMouseButton();
         mMotionModel->SetFreeLookByMouseButton( ! value );
         CPPUNIT_ASSERT( mMotionModel->GetFreeLookByMouseButton() == ! value );

         value = mMotionModel->GetRecenterMouse();
         mMotionModel->SetRecenterMouse( ! value );
         CPPUNIT_ASSERT( mMotionModel->GetRecenterMouse() == ! value );

         CPPUNIT_ASSERT( ! mMotionModel->IsReverseUpDown() );
         mMotionModel->SetReverseUpDown( true );
         CPPUNIT_ASSERT( mMotionModel->IsReverseUpDown() );

         CPPUNIT_ASSERT( ! mMotionModel->IsReverseLeftRight() );
         mMotionModel->SetReverseLeftRight( true );
         CPPUNIT_ASSERT( mMotionModel->IsReverseLeftRight() );

         osg::Vec3 restHPR(30.0,60.0,0.0);
         CPPUNIT_ASSERT( restHPR != mMotionModel->GetRestingRotation() );
         mMotionModel->SetRestingRotation( restHPR );
         CPPUNIT_ASSERT( restHPR == mMotionModel->GetRestingRotation() );
      }

      //////////////////////////////////////////////////////////////
      void AdvanceTime( float timeDelta = 1.0f )
      {
         double time[] = {timeDelta,timeDelta}; // motion model accesses index 1
         dtCore::Base::MessageData msg;
         msg.message = "preframe";
         msg.userData = &time;

         // Bypass the whole message system and send the message
         // directly to the motion model to be ticked with EXACT time.
         //
         // NOTE: The local message struct pointer SHOULD be safe since
         // this motion model does not maintain nor pass the pointer
         // beyond the scope of this function call.
         mMotionModel->OnMessage( &msg );
      }

      //////////////////////////////////////////////////////////////
      void SimulatMouseMove( float x, float y )
      {
         dtCore::LogicalAxis* axis = mMotionModel->GetLeftRightMouseAxis();
         axis->SetState(-x); // positive rotation results to negative heading
         axis = mMotionModel->GetUpDownMouseAxis();
         axis->SetState(y);

         // Simulate one second of time passing
         AdvanceTime(1.0f);
      }

      //////////////////////////////////////////////////////////////
      void CompareRotations( float delta, bool horizontal )
      {
         dtCore::Transform attachedTrans;
         mAttachable->GetTransform( attachedTrans, dtCore::Transformable::REL_CS );
         osg::Vec3 attachedRotate;
         attachedTrans.GetRotation( attachedRotate );

         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Orientation of attached motion model should NOT exceed its clamping limit.",
            delta, attachedRotate[horizontal?0:1], 0.01 );
      }

      //////////////////////////////////////////////////////////////
      void TestOrientationLimits( float limitLeftRight, float limitUpDown )
      {
         mMotionModel->SetLeftRightLimit(limitLeftRight);
         mMotionModel->SetUpDownLimit(limitUpDown);

         bool freeLook = ! mMotionModel->GetFreeLookByKey() || mMotionModel->IsKeyHeld();
         float degrees = 0.0f;

         if( limitLeftRight >= 0.0f ) // negative limits mean free-range
         {
            // Rotate 360 right
            for( degrees = 0.0f; degrees <= 720.0f; degrees += 30.0f )
            {
               SimulatMouseMove( 30.0f, 0.0f );
            }
            CompareRotations(freeLook?limitLeftRight:0.0f, true);

            // Rotate 360 left
            for( degrees = 720.0f; degrees >= 0.0f; degrees -= 30.0f )
            {
               SimulatMouseMove( -30.0f, 0.0f );
            }
            CompareRotations(freeLook?-limitLeftRight:0.0f, true);
         }
         
         if( limitUpDown >= 0.0f ) // negative limits mean free-range
         {
            // Rotate 360 down
            for( degrees = 720.0f; degrees >= 0.0f; degrees -= 30.0f )
            {
               SimulatMouseMove( 0.0f, -30.0f );
            }
            CompareRotations(freeLook?-limitUpDown:0.0f, false);

            // Rotate 360 up
            for( degrees = 0.0f; degrees <= 720.0f; degrees += 30.0f )
            {
               SimulatMouseMove( 0.0f, 30.0f );
            }
            CompareRotations(freeLook?limitUpDown:0.0f, false);
         }
      }
      
      //////////////////////////////////////////////////////////////
      void TestTransforms()
      {
         // Test motion with regular mouse motion
         mMotionModel->SetFreeLookByKey(false);

         // Test key-less free-look. 
         TestOrientationLimits( 45.0f, 45.0f );
         TestOrientationLimits( -1.0f, 45.0f ); // free horizontal rotation
         TestOrientationLimits( 45.0f, -1.0f ); // free vertical rotation
         TestOrientationLimits( 0.0f, 0.0f );

         // Test motion requiring a modifier key being held
         // --- Test without the key being pressed
         mMotionModel->SetFreeLookByKey(true);
         mMotionModel->SetKeyHeld(false);
         TestOrientationLimits( 45.0f, 45.0f );
         TestOrientationLimits( 0.0f, 0.0f );
         // --- Test with the key being pressed
         mMotionModel->SetKeyHeld(true); // allow free look
         TestOrientationLimits( 45.0f, 45.0f );
         TestOrientationLimits( -1.0f, 45.0f ); // free horizontal rotation
         TestOrientationLimits( 45.0f, -1.0f ); // free vertical rotation
         TestOrientationLimits( 0.0f, 0.0f );
      }


   private:
      dtCore::RefPtr<dtGame::GameManager> mGM;
      dtCore::RefPtr<TestClampedMotionModel> mMotionModel;
      dtCore::RefPtr<dtCore::Scene> mScene;
      dtCore::RefPtr<dtCore::Transformable> mTarget;
      dtCore::RefPtr<dtCore::Transformable> mAttachable;
      dtCore::RefPtr<dtCore::Camera> mCamera;
      dtCore::RefPtr<dtABC::Application> mApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ClampedMotionModelTests);
