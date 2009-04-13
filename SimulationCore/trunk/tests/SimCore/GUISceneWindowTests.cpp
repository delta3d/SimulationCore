/*
 * Copyright, 2007, Alion Science and Technology Corporation, all rights reserved.
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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/dtgameprefix-src.h>
#include <cppunit/extensions/HelperMacros.h>
#include <osg/CameraNode>
#include <osg/Group>
#include <SimCore/GUI/SceneWindow.h>



////////////////////////////////////////////////////////////////////////////////
// TEST CODE
////////////////////////////////////////////////////////////////////////////////
class SceneWindowTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(SceneWindowTests);

      CPPUNIT_TEST(TestSceneWindowProperties);

   CPPUNIT_TEST_SUITE_END();

   public:
      
      void setUp();
      void tearDown();

      // Test Functions
      void TestSceneWindowProperties();

   private:
      dtCore::RefPtr<SimCore::gui::SceneWindow> mSceneWin;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SceneWindowTests);

////////////////////////////////////////////////////////////////////////////////
void SceneWindowTests::setUp()
{
   CEGUI::Window* window = CEGUI::WindowManager::getSingleton().createWindow("WindowsLook/StaticImage","TestWindow");
   CEGUI::System::getSingleton().setGUISheet( window );
   mSceneWin = new SimCore::gui::SceneWindow( *window );
}

////////////////////////////////////////////////////////////////////////////////
void SceneWindowTests::tearDown()
{
}

////////////////////////////////////////////////////////////////////////////////
void SceneWindowTests::TestSceneWindowProperties()
{
   const SimCore::gui::SceneWindow* constSceneWin = mSceneWin.get();

   dtCore::RefPtr<osg::Group> hudLayer = new osg::Group;
   CPPUNIT_ASSERT( hudLayer->getNumChildren() == 0 );
   mSceneWin->InitializeCamera( *hudLayer );
   CPPUNIT_ASSERT( hudLayer->getNumChildren() == 1 );

   try
   {
      mSceneWin->GetCameraNode();
      constSceneWin->GetCameraNode();
   }
   catch( ... )
   {
      CPPUNIT_ASSERT_MESSAGE("SceneWindow might have a NULL camera node.",false);
   }

   CPPUNIT_ASSERT( mSceneWin->IsVisible() );
   mSceneWin->SetVisible( false );
   CPPUNIT_ASSERT( ! mSceneWin->IsVisible() );

   osg::Vec4 rect( -10.0f, 20.0f, -30.0f, 40.0f );
   mSceneWin->SetWindowUnits ( rect );
   osg::Vec4 testRect( constSceneWin->GetWindowUnits() );
   CPPUNIT_ASSERT( rect == testRect );

   osg::Vec2 viewCenter;
   CPPUNIT_ASSERT( constSceneWin->GetViewCenter() == viewCenter );
   mSceneWin->SetViewCenter( viewCenter );
   CPPUNIT_ASSERT( constSceneWin->GetViewCenter() == viewCenter );
   mSceneWin->SetViewCentered();
   viewCenter.set( 0.0f, 0.0f );
//   CPPUNIT_ASSERT( constSceneWin->GetViewCenter() == viewCenter );
}
