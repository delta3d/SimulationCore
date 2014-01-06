/* -*-c++-*-
* Simulation Core - HUDElementsTests (.h & .cpp) - Using 'The MIT License'
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
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <dtCore/camera.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/deltawin.h>
#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/exception.h>
#include <dtUtil/datapathutils.h>
#include <SimCore/Components/StealthHUDElements.h>

#include <osg/io_utils>
#include <dtABC/application.h>

#include <UnitTestMain.h>


//////////////////////////////////////////////////////////////
// UNIT TESTS
//////////////////////////////////////////////////////////////
class HUDElementsTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(HUDElementsTests);

      //CPPUNIT_TEST(TestAllHUDElements);

      CPPUNIT_TEST(TestHUDElement);
      CPPUNIT_TEST(TestHUDText);
      CPPUNIT_TEST(TestHUDGroup);
      CPPUNIT_TEST(TestHUDButton);
      CPPUNIT_TEST(TestHUDToolbar);
      CPPUNIT_TEST(TestHUDMeter);
      CPPUNIT_TEST(TestHUDBarMeter);
      CPPUNIT_TEST(TestHUDSlideBarMeter);
      CPPUNIT_TEST(TestHUDQuadElement);

      //////////////////////////////////////////////////////////////
      CPPUNIT_TEST(TestStealthButton);
      CPPUNIT_TEST(TestStealthToolbar);
      CPPUNIT_TEST(TestStealthMeter);
      CPPUNIT_TEST(TestStealthHealthMeter);
      CPPUNIT_TEST(TestStealthAmmoMeter);
      CPPUNIT_TEST(TestStealthCompassMeter);
      CPPUNIT_TEST(TestStealthGPSMeter);
      CPPUNIT_TEST(TestStealthMGRSMeter);
      CPPUNIT_TEST(TestStealthCartesianMeter);
      CPPUNIT_TEST(TestStealthSpeedometer);
      CPPUNIT_TEST(TestStealthCallSign);

   CPPUNIT_TEST_SUITE_END();


   public:

      void setUp();
      void tearDown();

      void setupCEGUI();

      // Calls all the test functions for each element
      void TestAllHUDElements();

      //////////////////////////////////////////////////////////////
      void TestHUDElement();
      void TestHUDText();
      void TestHUDGroup();
      void TestHUDButton();
      void TestHUDToolbar();
      void TestHUDMeter();
      void TestHUDBarMeter();
      void TestHUDSlideBarMeter();
      void TestHUDQuadElement();

      //////////////////////////////////////////////////////////////
      void TestStealthButton();
      void TestStealthToolbar();
      void TestStealthMeter();
      void TestStealthHealthMeter();
      void TestStealthAmmoMeter();
      void TestStealthCompassMeter();
      void TestStealthGPSMeter();
      void TestStealthMGRSMeter();
      void TestStealthCartesianMeter();
      void TestStealthSpeedometer();
      void TestStealthCallSign();


   private:
      std::shared_ptr<dtGame::GameManager> mGM;
      std::shared_ptr<dtABC::Application> mApp;
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
      std::shared_ptr<dtGUI::CEUIDrawable> mGUI;
      std::shared_ptr<SimCore::Components::HUDGroup> mMainGUIWindow;
#else
      std::shared_ptr<dtGUI::GUI> mGUI;
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(HUDElementsTests);



//////////////////////////////////////////////////////////////
void HUDElementsTests::setupCEGUI()
{
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
   mMainGUIWindow = new SimCore::Components::HUDGroup("root","DefaultGUISheet");
   CEGUI::System::getSingleton().setGUISheet(mMainGUIWindow->GetCEGUIWindow());
   //SLEEP(200);
#endif
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::setUp()
{
   try{
      // A window & camera are needed for GUI rendering
      mApp = &GetGlobalApplication();

      mGM = new dtGame::GameManager(*mApp->GetScene());
      mGM->SetApplication(*mApp);

#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
      mGUI = &GetGlobalCEGUIDrawable();
      mApp->GetScene()->AddDrawable(mGUI.get());
      setupCEGUI();
#else
      mGUI = &GetGlobalGUI();
#endif

      dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
      dtCore::System::GetInstance().Start();
   }
   catch(dtUtil::Exception& e)
   {
      LOG_ERROR(e.ToString());
   }
   catch(CEGUI::Exception& ce)
   {
      std::ostringstream oss;
      oss << ce.getName() << ":\n" << ce.getMessage() << "\n";
      LOG_ERROR(oss.str());
   }
   catch(...)
   {
      LOG_ERROR("Unknown exception");
   }
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::tearDown()
{
   try
   {
      dtCore::System::GetInstance().Stop();

      if(mGM.valid())
         mGM->DeleteAllActors(true);

#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
      mGUI->Emancipate();

      mMainGUIWindow = nullptr;
#endif
   }
   catch(dtUtil::Exception& e)
   {
      LOG_ERROR(e.ToString());
   }
   catch(CEGUI::Exception& ce)
   {
      std::ostringstream oss;
      oss << ce.getName() << ":\n" << ce.getMessage() << "\n";
      LOG_ERROR(oss.str());
   }
   catch(...)
   {
      LOG_ERROR("Unknown exception");
   }

   mGM = nullptr;
   mGUI = nullptr;
   mApp = nullptr;
}


//////////////////////////////////////////////////////////////
void HUDElementsTests::TestAllHUDElements()
{
   // NOTE: HUDImage is not tested since it only has one function;
   // HUDImage.SetImage() which is a wrapper function of CEGUI.

   // Base HUD Elements
   TestHUDElement();
   TestHUDText();
   TestHUDGroup();
   TestHUDButton();
   TestHUDToolbar();
   TestHUDMeter();
   TestHUDBarMeter();
   TestHUDSlideBarMeter();
   TestHUDQuadElement();

   // Stealth HUD Elements
   TestStealthButton();
   TestStealthToolbar();
   TestStealthMeter();
   TestStealthHealthMeter();
   TestStealthAmmoMeter();
   TestStealthCompassMeter();
   TestStealthGPSMeter();
   TestStealthMGRSMeter();
   TestStealthCartesianMeter();
   TestStealthSpeedometer();
   TestStealthCallSign();
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDElement()
{
   std::shared_ptr<SimCore::Components::HUDElement> element =
      new SimCore::Components::HUDElement("TestElement",SimCore::Components::HUDElement::DEFAULT_IMAGE_TYPE);

   CPPUNIT_ASSERT_MESSAGE( "HUD Element should contain a valid CEGUI window",
      element->GetCEGUIWindow() != nullptr );

   // Test default alignment
   CPPUNIT_ASSERT_MESSAGE( "Default HUDAlignment should be LEFT_TOP",
      element->GetAlignment() == SimCore::Components::HUDAlignment::LEFT_TOP );
   element->SetAlignment( SimCore::Components::HUDAlignment::RIGHT_BOTTOM );
   CPPUNIT_ASSERT_MESSAGE( "New HUDAlignment should be RIGHT_BOTTOM",
      element->GetAlignment() == SimCore::Components::HUDAlignment::RIGHT_BOTTOM );

   // Test element positioning
   osg::Vec2 pos, out;
   // --- Test default position
   element->GetPosition(out);
   CPPUNIT_ASSERT_MESSAGE( "Default position should be 0,0 and in relative mode",
      pos == out && ! element->IsAbsolutePosition() );

   // --- Test new position (RELATIVE)
   pos.set(0.5f,0.5f);
   element->SetPosition(pos[0],pos[1]);
   element->GetPosition(out);
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have a NEW relative position",
      pos == out && ! element->IsAbsolutePosition() );

   // --- Test new position (ABSOLUTE)
   pos.set(300.0f,300.0f);
   element->SetPosition(pos[0],pos[1],true);
   element->GetPosition(out);
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have a NEW absolute position",
      pos == out && element->IsAbsolutePosition() );

   // --- Test new position and alignment setting (ABSOLUTE from previous call to SetPosition)
   pos.set(150.0f,150.0f);
   element->SetPosition(pos[0],pos[1],SimCore::Components::HUDAlignment::CENTER);
   element->GetPosition(out);
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have a NEW absolute position",
      pos == out && element->IsAbsolutePosition() );
   CPPUNIT_ASSERT_MESSAGE( "New HUDAlignment should be CENTER",
      element->GetAlignment() == SimCore::Components::HUDAlignment::CENTER );

   // --- Test new position and alignment setting (RELATIVE)
   pos.set(150.0f,150.0f);
   element->SetPosition(pos[0],pos[1],SimCore::Components::HUDAlignment::LEFT_BOTTOM,false);
   element->GetPosition(out);
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have a NEW relative position",
      pos == out && ! element->IsAbsolutePosition() );
   CPPUNIT_ASSERT_MESSAGE( "New HUDAlignment should be LEFT_BOTTOM",
      element->GetAlignment() == SimCore::Components::HUDAlignment::LEFT_BOTTOM );



   // Test size setting
   osg::Vec2 dimensions(0.5f,0.5f);
   element->SetSize(dimensions[0],dimensions[1]);
   element->GetSize(out);
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have a NEW relative size",
      dimensions == out && ! element->IsAbsoluteSize() );

   // --- Test absolute size
   dimensions.set(20.0f,40.0f);
   element->SetSize(dimensions[0],dimensions[1],true);
   element->GetSize(out);
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have a NEW absolute size",
      dimensions == out && element->IsAbsoluteSize() );

   // --- Test bounds setting (RELATIVE)
   float errorThreshold = 0.001f;
   osg::Vec4 bounds;
   pos.set(0.25f,0.75f);
   dimensions.set(0.1f,0.3f);
   element->SetBounds( pos[0], pos[1], dimensions[0], dimensions[1] );
   element->GetBounds( bounds );
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have NEW relative size and relative position",
      ! element->IsAbsoluteSize()
      && ! element->IsAbsolutePosition() );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( pos[0], bounds[0], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( pos[1], bounds[1], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( dimensions[0], bounds[2], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( dimensions[1], bounds[3], errorThreshold );

   // --- Test setting bounds and alignment (ABSOLUTE)
   pos.set(-100.0f,50.0f);
   dimensions.set(60.0f,30.0f);
   element->SetBounds( pos[0], pos[1], dimensions[0], dimensions[1], SimCore::Components::HUDAlignment::RIGHT_TOP, true, true );
   element->GetBounds( bounds );
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have a NEW alignment, absolute size and absolute position",
      element->GetAlignment() == SimCore::Components::HUDAlignment::RIGHT_TOP
      && element->IsAbsoluteSize()
      && element->IsAbsolutePosition() );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( pos[0], bounds[0], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( pos[1], bounds[1], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( dimensions[0], bounds[2], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( dimensions[1], bounds[3], errorThreshold );


   // Test setting position by a Vec2
   osg::Vec2 newPos( 63.0f, -31.5f );
   element->SetPositionByVec( newPos );
   element->GetPosition( pos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newPos[0], pos[0], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newPos[1], pos[1], errorThreshold );

   // Test setting size by a Vec2
   osg::Vec2 newDimensions( 32.0f, 72.0f );
   element->SetSizeByVec( newDimensions );
   element->GetSize( dimensions );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newDimensions[0], dimensions[0], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newDimensions[1], dimensions[1], errorThreshold );

   // Test setting bounds by a Vec4
   osg::Vec4 newBounds( -20.0f, 16.0f, 52.0f, 13.0f );
   element->SetBoundsByVec( newBounds );
   element->GetBounds( bounds );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newBounds[0], bounds[0], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newBounds[1], bounds[1], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newBounds[2], bounds[2], errorThreshold );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( newBounds[3], bounds[3], errorThreshold );


   // Test visibility
   bool value = element->IsVisible();
   element->SetVisible( ! value );
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should have its visibility toggled",
      element->IsVisible() != value );

   element->Show(); // just in case it is already hidden
   element->Hide();
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should be hidden",
      ! element->IsVisible() );
   element->Show();
   CPPUNIT_ASSERT_MESSAGE( "HUD Element should be visible",
      element->IsVisible() );

   // NOTE: SetProperty and GetProperty will not be tested since they are
   // mere wrappers around CEGUI functions of the same names. These functions
   // prevent crashes when requesting non-existent parameters and also eliminate
   // the step of having to access the contained CEGUI window directly.
   // Property names and values are dependent on the CEGUI scheme and implementation.
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDText()
{
   LOG_ERROR("TESTING HUD TEXT");
   std::shared_ptr<SimCore::Components::HUDText> text = new SimCore::Components::HUDText("TestText");

   // Test text setting
   std::string value("Lorem Ipsum");
   CPPUNIT_ASSERT_MESSAGE( "HUD Text should NOT have a text value",
      text->GetText() != value );
   text->SetText(value);
   CPPUNIT_ASSERT_MESSAGE( "HUD Text should have a NEW text value",
      text->GetText() == value );

   // Test text and position setting
   value = "Some more lorem ipsum";
   osg::Vec2 pos;
   text->SetText(value, 0.5f, 0.65f );
   text->GetPosition( pos );
   CPPUNIT_ASSERT_MESSAGE( "HUD Text should have NEW text and position",
      text->GetText() == value
      && pos[0] == 0.5f && pos[1] == 0.65f );

}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDGroup()
{
   const std::string childName1("Child1");
   const std::string childName2("Child2");
   const std::string childName3("Child3");
   const std::string childName4("Child4");
   std::shared_ptr<SimCore::Components::HUDGroup> group = new SimCore::Components::HUDGroup("TestGroup");
   std::shared_ptr<SimCore::Components::HUDImage> child1 = new SimCore::Components::HUDImage(childName1);
   std::shared_ptr<SimCore::Components::HUDImage> child2 = new SimCore::Components::HUDImage(childName2);
   std::shared_ptr<SimCore::Components::HUDImage> child3 = new SimCore::Components::HUDImage(childName3);
   std::shared_ptr<SimCore::Components::HUDImage> child4 = new SimCore::Components::HUDImage(childName4);

   // NOTE: Add and Remove may be changed to take references in the future.
   // A good bit of code uses the current interface so far.

   // Verify the new group is empty
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have 0 children",
      group->GetTotalElements() == 0 );

   // Attempt adding an invalid pointer
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should not add a nullptr pointer",
      !group->Add( nullptr ) );
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should not increase child count when attempting to add a nullptr pointer",
      group->GetTotalElements() == 0 );

   // Test searching for CEGUI child windows that do not exist.
   CPPUNIT_ASSERT( ! group->Has( childName1 ) );
   CPPUNIT_ASSERT( ! group->Has( childName2 ) );
   CPPUNIT_ASSERT( ! group->Has( childName3 ) );
   CPPUNIT_ASSERT( ! group->Has( childName4 ) );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName1 ) == nullptr );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName2 ) == nullptr );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName3 ) == nullptr );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName4 ) == nullptr );

   // Add all children
   group->Add( child1.get() );
   group->Add( child2.get() );
   group->Add( child3.get() );
   group->Add( child4.get() );

   // Test searching for CEGUI child windows that have been added.
   CPPUNIT_ASSERT( group->Has( childName1 ) );
   CPPUNIT_ASSERT( group->Has( childName2 ) );
   CPPUNIT_ASSERT( group->Has( childName3 ) );
   CPPUNIT_ASSERT( group->Has( childName4 ) );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName1 ) == child1->GetCEGUIWindow() );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName2 ) == child2->GetCEGUIWindow() );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName3 ) == child3->GetCEGUIWindow() );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName4 ) == child4->GetCEGUIWindow() );

   // Capture the total in a variable for easier debugging
   int total = group->GetTotalElements();
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have 4 children",
      total == 4 );

   // Remove a middle child
   CPPUNIT_ASSERT( group->Remove( child2.get() ) );
   total = group->GetTotalElements();
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have removed child 2",
      ! group->Has( *child2 )
      && total == 3 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have the other children",
      group->Has( *child1 )
      && group->Has( *child3 )
      && group->Has( *child4 ) );

   // Remove the end child
   CPPUNIT_ASSERT( group->Remove( child4.get() ) );
   total = group->GetTotalElements();
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have removed child 4",
      ! group->Has( *child4 )
      && total == 2 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have the other children",
      group->Has( *child1 )
      && group->Has( *child3 ) );

   // --- Test removing nullptr
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should return false when attempting to remove nullptr",
      ! group->Remove( nullptr ) );
   total = group->GetTotalElements();
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have 2 children after attempting to remove nullptr",
      total == 2 );

   // Remove the front child
   CPPUNIT_ASSERT( group->Remove( child1.get() ) );
   total = group->GetTotalElements();
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have removed child 1",
      ! group->Has( *child1 )
      && total == 1 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have the other child",
      group->Has( *child3 ) );

   // Remove the only child
   CPPUNIT_ASSERT( group->Remove( child3.get() ) );
   total = group->GetTotalElements();
   CPPUNIT_ASSERT_MESSAGE( "HUD Group should have removed child 3; no other children should remain",
      ! group->Has( *child3 )
      && total == 0 );

   // Test searching for CEGUI child windows that should no longer be attached.
   CPPUNIT_ASSERT( ! group->Has( childName1 ) );
   CPPUNIT_ASSERT( ! group->Has( childName2 ) );
   CPPUNIT_ASSERT( ! group->Has( childName3 ) );
   CPPUNIT_ASSERT( ! group->Has( childName4 ) );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName1 ) == nullptr );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName2 ) == nullptr );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName3 ) == nullptr );
   CPPUNIT_ASSERT( group->GetCEGUIChild( childName4 ) == nullptr );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDButton()
{
   std::shared_ptr<SimCore::Components::HUDButton> button = new SimCore::Components::HUDButton("TestButton");
   std::shared_ptr<SimCore::Components::HUDImage> imageOn = new SimCore::Components::HUDImage("On");
   std::shared_ptr<SimCore::Components::HUDImage> imageOff = new SimCore::Components::HUDImage("Off");

   button->SetActiveElement(imageOn.get());
   button->SetInactiveElement(imageOff.get());
   CPPUNIT_ASSERT_MESSAGE( "HUD Button should have an active element",
      button->GetActiveElement() == imageOn.get() );
   CPPUNIT_ASSERT_MESSAGE( "HUD Button should have an inactive element",
      button->GetInactiveElement() == imageOff.get() );

   // Test visibilities in the ACTIVE state
   button->SetActive(true);
   CPPUNIT_ASSERT_MESSAGE( "HUD Button should be active",
      button->IsActive() );
   CPPUNIT_ASSERT_MESSAGE( "HUD Button's active should be visible, only",
      imageOn->IsVisible() && ! imageOff->IsVisible() );

   // Test visibilities in the INACTIVE state
   button->SetActive(false);
   CPPUNIT_ASSERT_MESSAGE( "HUD Button should be active",
      ! button->IsActive() );
   CPPUNIT_ASSERT_MESSAGE( "HUD Button's active should be visible, only",
      ! imageOn->IsVisible() && imageOff->IsVisible() );

   // Test that the elements can be removed by setting them to nullptr
   button->SetActiveElement( nullptr );
   CPPUNIT_ASSERT_MESSAGE( "HUD Button should not have an active element",
      button->GetActiveElement() == nullptr );
   CPPUNIT_ASSERT_MESSAGE( "HUD Button should not have removed the inactive element",
      button->GetInactiveElement() != nullptr );

   button->SetInactiveElement( nullptr );
   CPPUNIT_ASSERT_MESSAGE( "HUD Button should not have an inactive element",
      button->GetInactiveElement() == nullptr );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDToolbar()
{
   // NOTE: The base toolbar deals with placement of its elements rather than
   // adjusting its own size with each addition or removal of elements.
   // Re-adjusting the toolbar's size in the base class could lead to unexpected
   // results, especially in relative position/size mode.
   //
   // Currently, resize functionality is implemented in StealthToolbar where it
   // adjusts both size and positions of the toolbar and its contained elements
   // in relative mode.
   //
   // The base toolbar class assumes its own size and its elements sizes are static.

   std::shared_ptr<SimCore::Components::HUDToolbar> toolbar = new SimCore::Components::HUDToolbar("TestToolbar");
   std::shared_ptr<SimCore::Components::HUDButton> button1 = new SimCore::Components::HUDButton("Button1");
   std::shared_ptr<SimCore::Components::HUDButton> button2 = new SimCore::Components::HUDButton("Button2");
   std::shared_ptr<SimCore::Components::HUDButton> button3 = new SimCore::Components::HUDButton("Button3");
   std::shared_ptr<SimCore::Components::HUDButton> button4 = new SimCore::Components::HUDButton("Button4");
   std::shared_ptr<SimCore::Components::HUDImage> imageStart = new SimCore::Components::HUDImage("ImageStart");
   std::shared_ptr<SimCore::Components::HUDImage> imageEnd = new SimCore::Components::HUDImage("ImageEnd");

   // Dimension variables (all values are arbitrary)
   osg::Vec2 toolbarSize, testPos;
   float errorThreshold = 0.001f;
   float endWidth = 0.25f, startWidth = 0.25f;
   float buttonWidth = 0.5f;
   float height = 0.35f;
   float testOffset; // for testing button positions

   // Set sizes of all the elements
   imageStart->SetSize( startWidth, height );
   imageEnd->SetSize( endWidth, height );
   button1->SetSize( buttonWidth, height );
   button2->SetSize( buttonWidth, height );
   button3->SetSize( buttonWidth, height );
   button4->SetSize( buttonWidth, height );

   // Add elements
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should NOT have a start element",
      toolbar->GetStartElement() == nullptr );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should NOT have a end element",
      toolbar->GetEndElement() == nullptr );
   toolbar->SetStartElement( imageStart.get() );
   toolbar->SetEndElement( imageEnd.get() );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should have a start element",
      toolbar->GetStartElement() == imageStart.get() );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should have a end element",
      toolbar->GetEndElement() == imageEnd.get() );

   // Add first button
   CPPUNIT_ASSERT( toolbar->InsertElement( button1.get() ) );
   // Add second button - to the front
   CPPUNIT_ASSERT( toolbar->InsertElement( button2.get(), 0 ) );
   // --- Try adding nullptr
   CPPUNIT_ASSERT( ! toolbar->InsertElement( nullptr ) );
   // Add third button - to the middle
   CPPUNIT_ASSERT( toolbar->InsertElement( button3.get(), 1 ) );
   // Add fourth button - to the front
   CPPUNIT_ASSERT( toolbar->InsertElement( button4.get(), 0 ) );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should have all elements",
      toolbar->GetTotalElements() == 4 );



   // Test expected indexes of the newly inserted elements
   // --- Button 1 should be at index 3
   CPPUNIT_ASSERT_MESSAGE( "Button 1 should be at index 3",
      button1.get() == toolbar->GetElement(3)
      && toolbar->GetElementIndex( *button1 ) == 3 );
   // --- Button 2 should be at index 1
   CPPUNIT_ASSERT_MESSAGE( "Button 2 should be at index 1",
      button2.get() == toolbar->GetElement(1)
      && toolbar->GetElementIndex( *button2 ) == 1 );
   // --- Button 3 should be at index 2
   CPPUNIT_ASSERT_MESSAGE( "Button 3 should be at index 2",
      button3.get() == toolbar->GetElement(2)
      && toolbar->GetElementIndex( *button3 ) == 2 );
   // --- Button 4 should be at index 0
   CPPUNIT_ASSERT_MESSAGE( "Button 4 should be at index 0",
      button4.get() == toolbar->GetElement(0)
      && toolbar->GetElementIndex( *button4 ) == 0 );



   // Test expected positions of the buttons (HORIZONTAL MODE)
   toolbar->UpdateLayout();
   testOffset = startWidth;
   // --- Button 4 position at index 0
   button4->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[0], errorThreshold );
   testOffset += buttonWidth;
   // --- Button 2 position at index 1
   button2->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[0], errorThreshold );
   testOffset += buttonWidth;
   // --- Button 3 position at index 2
   button3->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[0], errorThreshold );
   testOffset += buttonWidth;
   // --- Button 1 position at index 3
   button1->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[0], errorThreshold );
   testOffset += buttonWidth;
   // --- End element position
   imageEnd->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[0], errorThreshold );



   // Test changing layout modes (end with VERTICAL MODE)
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should be in horizontal mode by default",
      toolbar->IsHorizontal() && ! toolbar->IsVertical() );
   toolbar->SetHorizontal( false );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should now be in vertical mode",
      ! toolbar->IsHorizontal() && toolbar->IsVertical() );
   toolbar->SetVertical( false );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should now be in horizontal mode",
      toolbar->IsHorizontal() && ! toolbar->IsVertical() );
   toolbar->SetVertical( true );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should now be in vertical mode, again for the next test",
      ! toolbar->IsHorizontal() && toolbar->IsVertical() );



   // Test expected positions of the buttons (VERTICAL MODE)
   toolbar->UpdateLayout();
   testOffset = height;
   // --- Button 4 position at index 0
   button4->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[1], errorThreshold );
   testOffset += height;
   // --- Button 2 position at index 1
   button2->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[1], errorThreshold );
   testOffset += height;
   // --- Button 3 position at index 2
   button3->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[1], errorThreshold );
   testOffset += height;
   // --- Button 1 position at index 3
   button1->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[1], errorThreshold );
   testOffset += height;
   // --- End element position
   imageEnd->GetPosition( testPos );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testOffset, testPos[1], errorThreshold );



   // Remove front button - by pointer
   CPPUNIT_ASSERT( toolbar->RemoveElement( button4.get() )
      && toolbar->GetTotalElements() == 3 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should still have other elements, not button 4",
      toolbar->HasElement(*button1)
      && toolbar->HasElement(*button2)
      && toolbar->HasElement(*button3)
      && ! toolbar->HasElement(*button4) );
   // Remove middle button - by pointer
   CPPUNIT_ASSERT( toolbar->RemoveElement( button3.get() )
      && toolbar->GetTotalElements() == 2 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should still have other elements, not button 3",
      toolbar->HasElement(*button1)
      && toolbar->HasElement(*button2)
      && !toolbar->HasElement(*button3) );
   // --- Try removing nullptr
   CPPUNIT_ASSERT( ! toolbar->RemoveElement( nullptr ) );
   // Remove end button - by pointer
   CPPUNIT_ASSERT( toolbar->RemoveElement( button1.get() )
      && toolbar->GetTotalElements() == 1 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should still have other elements, not button 4",
      ! toolbar->HasElement(*button1)
      && toolbar->HasElement(*button2) );
   // Remove last button - by pointer
   CPPUNIT_ASSERT( toolbar->RemoveElement( button2.get() )
      && toolbar->GetTotalElements() == 0 );



   // Add all buttons again for next test
   toolbar->InsertElement( button1.get() );
   toolbar->InsertElement( button2.get() );
   toolbar->InsertElement( button3.get() );
   toolbar->InsertElement( button4.get() );
   // --- Verify that they have been added
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should have all elements for a third time",
      toolbar->GetTotalElements() == 4 );

   // Remove remaining elements
   toolbar->ClearElements();

   // Start and end elements should still remain
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should still have start and end elements",
      toolbar->GetStartElement() != nullptr
      && toolbar->GetEndElement() != nullptr );

   // Test removal of start and end elements by setting them to nullptr
   toolbar->SetStartElement( nullptr );
   toolbar->SetEndElement( nullptr );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should NOT have start and end elements",
      toolbar->GetStartElement() == nullptr
      && toolbar->GetEndElement() == nullptr );

   imageStart = nullptr;
   imageEnd = nullptr;
   button1 = nullptr;
   button2 = nullptr;
   button3 = nullptr;
   button4 = nullptr;
   toolbar = nullptr;
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDMeter()
{
   std::shared_ptr<SimCore::Components::HUDMeter> meter = new SimCore::Components::HUDMeter("TestMeter");
   std::shared_ptr<SimCore::Components::HUDImage> meterImage = new SimCore::Components::HUDImage("TestImage");

   // Test setting an image
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should not have an image by default",
      meter->GetImage() == nullptr );
   meter->SetImage( meterImage.get() );
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should have a new image",
      meter->GetImage() == meterImage.get() );

   // Test setting the scale
   // --- scale is the ratio of the set value to its max
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should have a default scale of 0",
      meter->GetScale() == 0.0f );
   meter->SetScale( 2.0f );
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should have a new scale",
      meter->GetScale() == 2.0f );

   // Test setting the unit count
   // --- any number larger than 0 means the meter will be
   // --- a chain of fixed sized units. 0 means the meter is
   // --- a single unit that can scale by inconsistent sized amounts.
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should have a default units count of 0",
      meter->GetUnitCount() == 0.0f );
   meter->SetUnitCount( 360.0f );
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should have a new unit count",
      meter->GetUnitCount() == 360.0f );

   // Test changing layout modes (end with VERTICAL MODE)
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should be in horizontal mode by default",
      meter->IsHorizontal() && ! meter->IsVertical() );
   meter->SetHorizontal( false );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should now be in vertical mode",
      ! meter->IsHorizontal() && meter->IsVertical() );
   meter->SetVertical( false );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should now be in horizontal mode",
      meter->IsHorizontal() && ! meter->IsVertical() );
   meter->SetVertical( true );
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should now be in vertical mode, again for the next test",
      ! meter->IsHorizontal() && meter->IsVertical() );

   // Test value setting
   // --- Not using units
   meter->SetUnitCount(0.0f);
   meter->SetValue(60.0f,600.0f,0.0f);
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 60.0f, meter->GetValue(), 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.1f, meter->GetScale(), 0.001f );
   // --- Using units
   meter->SetUnitCount(100.0f);
   meter->SetValue(24.5f,50.0f,-50.0f);
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 25.0f, meter->GetValue(), 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.75f, meter->GetScale(), 0.001f );
   meter->SetValue(-49.5f,50.0f,-50.0f);
   CPPUNIT_ASSERT_DOUBLES_EQUAL( -49.0f, meter->GetValue(), 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.01f, meter->GetScale(), 0.001f );
   meter->SetValue(49.5f,50.0f,-50.0f);
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 50.0f, meter->GetValue(), 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0f, meter->GetScale(), 0.001f );

   meterImage = nullptr;
   meter = nullptr;
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDBarMeter()
{
   std::shared_ptr<SimCore::Components::HUDBarMeter> barMeter = new SimCore::Components::HUDBarMeter("TestBarMeter");
   std::shared_ptr<SimCore::Components::HUDImage> meterImage = new SimCore::Components::HUDImage("BarImage");

   // Test setting the scale
   // --- scale is the ratio of the set value to its max
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should have a default scale of 0",
      barMeter->GetScale() == 0.0f );
   barMeter->SetScale( 2.0f );
   CPPUNIT_ASSERT_MESSAGE( "HUD Meter should have a new scale",
      barMeter->GetScale() == 2.0f );

   // Test overridden size setting
   osg::Vec2 outSize, outOriginalSize;
   barMeter->GetOriginalSize(outOriginalSize);
   barMeter->SetSize(0.75f, 0.3f);
   barMeter->GetSize(outSize);
   CPPUNIT_ASSERT( outSize != outOriginalSize );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.75f, outSize[0], 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.3f, outSize[1], 0.001f );
   barMeter->GetOriginalSize(outOriginalSize);
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.75f, outOriginalSize[0], 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.3f, outOriginalSize[1], 0.001f );

   // Test image setting
   meterImage->SetSize(0.5f,0.25f);
   barMeter->SetImage( meterImage.get() );
   CPPUNIT_ASSERT_MESSAGE("HUD Meter should have an image",
      barMeter->GetImage() == meterImage.get() );
   barMeter->GetOriginalImageSize( outOriginalSize );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.5f, outOriginalSize[0], 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.25f, outOriginalSize[1], 0.001f );

   meterImage = nullptr;
   barMeter = nullptr;
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDSlideBarMeter()
{
   std::shared_ptr<SimCore::Components::HUDSlideBarMeter> slideMeter =
      new SimCore::Components::HUDSlideBarMeter("TestSlideBarMeter");
   std::shared_ptr<SimCore::Components::HUDImage> meterImage = new SimCore::Components::HUDImage("SlideImage");

   // Test setting an image
   osg::Vec2 imageSize( 1.0, 1.0 );
   meterImage->SetSize( imageSize[0], imageSize[1] );
   slideMeter->SetSize( imageSize[0], imageSize[1] );
   slideMeter->SetImage( meterImage.get() );
   CPPUNIT_ASSERT_MESSAGE("HUD Meter should have an image",
      slideMeter->GetImage() == meterImage.get() );

   // Test setting image offset
   float offset = 0.1f;
   slideMeter->SetImageOffset( offset );
   CPPUNIT_ASSERT_MESSAGE( "HUD Slide Bar Meter should have a new image offset",
      offset == slideMeter->GetImageOffset() );

   // Test setting image range scale
   float imageRange = slideMeter->GetImageRangeScale() + 0.5f;
   slideMeter->SetImageRangeScale( imageRange );
   CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
      "HUD Slide Bar Meter should have a new image range scale",
      imageRange, slideMeter->GetImageRangeScale(), 0.001 );

   // Test slide reversing
   bool reversed = slideMeter->IsSlideReversed();
   slideMeter->SetSlideReversed( ! reversed );
   CPPUNIT_ASSERT_MESSAGE( "HUD Slide Bar Meter should have a different slide state",
      slideMeter->IsSlideReversed() != reversed );

   // Test sliding (NOT REVERSED)
   osg::Vec2 imagePos;
   float scale = 0.5f;
   float testValue;
   slideMeter->SetImageRangeScale( 1.0f );
   slideMeter->SetSlideReversed( false );

   // --- HORIZONTAL
   slideMeter->SetScale( scale );
   meterImage->GetPosition( imagePos );
   testValue = imageSize[0]*scale+offset;
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testValue, imagePos[0], 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0f, imagePos[1], 0.001f );

   // --- VERTICAL
   meterImage->SetPosition( 0.0f, 0.0f );
   slideMeter->SetHorizontal( false );
   slideMeter->SetScale( scale );
   meterImage->GetPosition( imagePos );
   testValue = imageSize[1]*scale+offset;
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0f, imagePos[0], 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testValue, imagePos[1], 0.001f );

   // Test sliding (REVERSED)
   scale = 0.3f;
   slideMeter->SetSlideReversed( true );

   // --- HORIZONTAL (Reversed)
   meterImage->SetPosition( 0.0f, 0.0f );
   slideMeter->SetHorizontal( true );
   slideMeter->SetScale( scale );
   meterImage->GetPosition( imagePos );
   testValue = imageSize[0]*scale+offset;
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testValue, imagePos[0], 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0f, imagePos[1], 0.001f );

   // --- VERTICAL (Reversed)
   meterImage->SetPosition( 0.0f, 0.0f );
   slideMeter->SetHorizontal( false );
   slideMeter->SetScale( scale );
   meterImage->GetPosition( imagePos );
   testValue = imageSize[1]*scale+offset;
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0f, imagePos[0], 0.001f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( testValue, imagePos[1], 0.001f );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestHUDQuadElement()
{
   // NOTE: Current, the HUD Quad element only operates
   // relative mode for both size and position.

   std::shared_ptr<SimCore::Components::HUDQuadElement> quad = new SimCore::Components::HUDQuadElement("TestQuad");

   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have a valid OSG node",
      quad->GetOSGNode() != nullptr );

   // Declare variables used in testing
   osg::Vec2 testVec2, outVec2;
   osg::Vec3 testVec3, outVec3;
   osg::Vec4 testColor, outColor;
   float testRotate = 0.0f;

   // Test geode access
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have created geometry", quad->GetGeode() != nullptr );

   // Test size
   testVec2.set( 0.5f, 0.2f );
   quad->SetSize( testVec2[0], testVec2[1] );
   quad->GetSize( outVec2 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have a new size",
      testVec2 == outVec2 );

   // Test offset
   testVec2.set( -0.25f, -0.1f );
   quad->SetOffset( testVec2[0], testVec2[1] );
   quad->GetOffset( outVec2 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have a new pivot offset",
      testVec2 == outVec2 );

   // Test position
   testVec3.set( 0.2f, 0.5f, 0.1f );
   testVec2.set( testVec3[0], testVec3[1] );
   quad->SetPosition( testVec3[0], testVec3[1], testVec3[2] );
   quad->GetPosition( outVec3 );
   quad->GetPosition( outVec2 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have a new 3D position",
      testVec3 == outVec3 );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should return the same 2D XY position",
      testVec2 == outVec2 );

   // Test color
   testColor.set( 0.2f, 1.0f, 0.5f, 1.0f );
   quad->SetColor( testColor[0], testColor[1], testColor[2], testColor[3] );
   quad->GetColor( outColor );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have a new color",
      testVec3 == outVec3 );

   // Test Alpha
   CPPUNIT_ASSERT( quad->GetAlpha() == 1.0f );
   quad->SetAlpha( 0.5f );
   CPPUNIT_ASSERT( quad->GetAlpha() == 0.5f );

   // Test rotation
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have a default rotation of 0",
      testRotate == quad->GetRotation() );
   quad->SetRotation( (testRotate = 0.5f) );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have a new rotation",
      testRotate == quad->GetRotation() );
   quad->SetRotation( (testRotate = -0.75f) );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should have another new rotation",
      testRotate == quad->GetRotation() );

   // Test visibility
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should be visible by default",
      quad->IsVisible() );
   quad->Hide();
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should now be hidden",
      ! quad->IsVisible() );
   quad->Show();
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should now be visible",
      quad->IsVisible() );
   quad->SetVisible( false );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should now be hidden again",
      ! quad->IsVisible() );
   quad->SetVisible( true );
   CPPUNIT_ASSERT_MESSAGE( "HUD Quad should now be visible again",
      quad->IsVisible() );


   // Test Addition of child quads
   std::shared_ptr<SimCore::Components::HUDQuadElement> child1 = new SimCore::Components::HUDQuadElement("Child1");
   std::shared_ptr<SimCore::Components::HUDQuadElement> child2 = new SimCore::Components::HUDQuadElement("Child2");
   std::shared_ptr<SimCore::Components::HUDQuadElement> child3 = new SimCore::Components::HUDQuadElement("Child3");
   // --- Ensure there are no children by default
   CPPUNIT_ASSERT( quad->GetTotalChildren() == 0 );
   CPPUNIT_ASSERT( ! quad->Has( *child1 ) );
   CPPUNIT_ASSERT( ! quad->Has( *child2 ) );
   CPPUNIT_ASSERT( ! quad->Has( *child3 ) );

   // --- Add the children
   CPPUNIT_ASSERT( quad->Add( *child1 ) );
   CPPUNIT_ASSERT( quad->Add( *child2 ) );
   CPPUNIT_ASSERT( quad->Add( *child3 ) );
   CPPUNIT_ASSERT( quad->Has( *child1 ) );
   CPPUNIT_ASSERT( quad->Has( *child2 ) );
   CPPUNIT_ASSERT( quad->Has( *child3 ) );
   CPPUNIT_ASSERT( quad->GetTotalChildren() == 3 );

   // --- Adding self MUST fail!
   CPPUNIT_ASSERT( ! quad->Add( *quad ) );
   // --- Ensure adding the same children again fails
   CPPUNIT_ASSERT( ! quad->Add( *child1 ) );
   CPPUNIT_ASSERT( ! quad->Add( *child2 ) );
   CPPUNIT_ASSERT( ! quad->Add( *child3 ) );
   CPPUNIT_ASSERT( quad->GetTotalChildren() == 3 );


   // Test Removal of child quads
   CPPUNIT_ASSERT( quad->Remove( *child1 ) );
   CPPUNIT_ASSERT( quad->Remove( *child2 ) );
   CPPUNIT_ASSERT( quad->Remove( *child3 ) );
   CPPUNIT_ASSERT( ! quad->Has( *child1 ) );
   CPPUNIT_ASSERT( ! quad->Has( *child2 ) );
   CPPUNIT_ASSERT( ! quad->Has( *child3 ) );
   CPPUNIT_ASSERT( quad->GetTotalChildren() == 0 );
   // --- Ensure removal fails for the children that were just removed previously.
   CPPUNIT_ASSERT( ! quad->Remove( *child1 ) );
   CPPUNIT_ASSERT( ! quad->Remove( *child2 ) );
   CPPUNIT_ASSERT( ! quad->Remove( *child3 ) );
}



//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthButton()
{
   std::shared_ptr<SimCore::Components::StealthButton> button =
      new SimCore::Components::StealthButton("TestStealthButton","Help","F1");

   CPPUNIT_ASSERT_MESSAGE( "Button should have a valid key label image",
      button->GetKeyLabel() != nullptr );

   CPPUNIT_ASSERT_MESSAGE( "Button should be inactive by default",
      ! button->IsActive() );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthToolbar()
{
   std::shared_ptr<SimCore::Components::StealthToolbar> toolbar =
      new SimCore::Components::StealthToolbar("TestStealthToolbar");
   std::shared_ptr<SimCore::Components::StealthButton> button1;
   std::shared_ptr<SimCore::Components::StealthButton> button2;
   std::shared_ptr<SimCore::Components::StealthButton> button3;
   std::shared_ptr<SimCore::Components::StealthButton> button4;

   CPPUNIT_ASSERT_MESSAGE( "Toolbar should have 0 buttons by default",
      toolbar->GetButtonCount() == 0 );

   // Test strings.
   // NOTE: These names must match those found in the imageset for "Toolbar"
   // in CEGUI/imagesets
   const std::string
      name1("Binoculars"),
      name2("LRF"),
      name3("NightVision"),
      name4("GPS");

   // Add all buttons
   // NOTE: Keylabel names must match those found in the imageset for "KeyLabels"
   // in CEGUI/imagesets
   toolbar->AddButton(name1, name1, "F8");
   toolbar->AddButton(name2, name2, "F9");
   toolbar->AddButton(name3, name3, "F10");
   toolbar->AddButton(name4, name4, "F11");
   button1 = dynamic_cast<SimCore::Components::StealthButton*> (toolbar->GetElement(0));
   button2 = dynamic_cast<SimCore::Components::StealthButton*> (toolbar->GetElement(1));
   button3 = dynamic_cast<SimCore::Components::StealthButton*> (toolbar->GetElement(2));
   button4 = dynamic_cast<SimCore::Components::StealthButton*> (toolbar->GetElement(3));

   CPPUNIT_ASSERT_MESSAGE( "Toolbar should have 4 buttons",
      toolbar->GetButtonCount() == 4 );


   // Test button toggling
   CPPUNIT_ASSERT( ! button1->IsActive() );
   CPPUNIT_ASSERT( ! button2->IsActive() );
   CPPUNIT_ASSERT( ! button3->IsActive() );
   CPPUNIT_ASSERT( ! button4->IsActive() );

   toolbar->SetButtonActive( name3, true );

   CPPUNIT_ASSERT( ! button1->IsActive() );
   CPPUNIT_ASSERT( ! button2->IsActive() );
   CPPUNIT_ASSERT( button3->IsActive() );
   CPPUNIT_ASSERT( ! button4->IsActive() );

   toolbar->SetButtonActive( name2, true );

   CPPUNIT_ASSERT( ! button1->IsActive() );
   CPPUNIT_ASSERT( button2->IsActive() );
   CPPUNIT_ASSERT( button3->IsActive() );
   CPPUNIT_ASSERT( ! button4->IsActive() );

   toolbar->SetButtonsActive( true );

   CPPUNIT_ASSERT( button1->IsActive() );
   CPPUNIT_ASSERT( button2->IsActive() );
   CPPUNIT_ASSERT( button3->IsActive() );
   CPPUNIT_ASSERT( button4->IsActive() );

   toolbar->SetButtonActive( name2, false );

   CPPUNIT_ASSERT( button1->IsActive() );
   CPPUNIT_ASSERT( ! button2->IsActive() );
   CPPUNIT_ASSERT( button3->IsActive() );
   CPPUNIT_ASSERT( button4->IsActive() );



   // Test getting all the button names
   std::vector<std::string> buttonNames;
   toolbar->GetButtonNames( buttonNames );

   unsigned int successes = 0;
   int limit = buttonNames.size();
   for( int i = 0; i < limit; ++i )
   {
      buttonNames[i];
      if( buttonNames[i] == name1
         || buttonNames[i] == name2
         || buttonNames[i] == name3
         || buttonNames[i] == name4 )
      {
         successes++;
      }
   }
   CPPUNIT_ASSERT_MESSAGE( "Toolbar should return all the names of the buttons it contains",
      successes == 4 );



   // Test remove
   toolbar->RemoveButton(name2);
   CPPUNIT_ASSERT( ! toolbar->HasElement( *button2 ) );
   CPPUNIT_ASSERT( toolbar->HasElement( *button1 ) );
   CPPUNIT_ASSERT( toolbar->HasElement( *button3 ) );
   CPPUNIT_ASSERT( toolbar->HasElement( *button4 ) );
   CPPUNIT_ASSERT_MESSAGE("Toolbar should now have 3 buttons",
      toolbar->GetButtonCount() == 3 );

   // --- Test button retrieval
   CPPUNIT_ASSERT( toolbar->GetButton( name1 ) == button1.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name2 ) == nullptr );
   CPPUNIT_ASSERT( toolbar->GetButton( name3 ) == button3.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name4 ) == button4.get());

   toolbar->RemoveButton(name4);
   CPPUNIT_ASSERT( ! toolbar->HasElement( *button4 ) );
   CPPUNIT_ASSERT( toolbar->HasElement( *button1 ) );
   CPPUNIT_ASSERT( toolbar->HasElement( *button3 ) );
   CPPUNIT_ASSERT_MESSAGE("Toolbar should now have 2 buttons",
      toolbar->GetButtonCount() == 2 );

   // --- Test button retrieval
   CPPUNIT_ASSERT( toolbar->GetButton( name1 ) == button1.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name2 ) == nullptr );
   CPPUNIT_ASSERT( toolbar->GetButton( name3 ) == button3.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name4 ) == nullptr );



   // Test re-adding buttons directly without the toolbar automatically creating a new button.
   std::shared_ptr<SimCore::Components::StealthButton> tmpButton;
   CPPUNIT_ASSERT( ! toolbar->AddButton( tmpButton ) ); // should not add a nullptr button.
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 2 );
   CPPUNIT_ASSERT( ! toolbar->AddButton( button1 ) );
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 2 );
   CPPUNIT_ASSERT( ! toolbar->AddButton( button3 ) );
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 2 );
   CPPUNIT_ASSERT( toolbar->AddButton( button2 ) );
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 3 );



   // Test button replacing
   int indices[] = {
      toolbar->GetElementIndex( *button1 ),
      toolbar->GetElementIndex( *button2 ),
      toolbar->GetElementIndex( *button3 ) };

   CPPUNIT_ASSERT_MESSAGE("All toolbar buttons should have different indices",
      indices[0] != indices[1] && indices[0] != indices[2] && indices[1] != indices[2] );


   // --- Swap middle button
   toolbar->ReplaceButton( name2, button4, &tmpButton );
   CPPUNIT_ASSERT( toolbar->GetButton( name1 ) == button1.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name2 ) == nullptr );
   CPPUNIT_ASSERT( toolbar->GetButton( name3 ) == button3.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name4 ) == button4.get() );
   CPPUNIT_ASSERT_MESSAGE("New button should now have the same index as the old button",
      indices[1] == toolbar->GetElementIndex( *button4 ) );
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 3 );
   CPPUNIT_ASSERT( tmpButton.get() == button2.get() );

   // --- Swap first button
   //     Not testing capture of old button here; this will determine if the function
   //     can handle a nullptr value for the third parameter.
   toolbar->ReplaceButton( name1, button2, nullptr );
   CPPUNIT_ASSERT( toolbar->GetButton( name1 ) == nullptr );
   CPPUNIT_ASSERT( toolbar->GetButton( name2 ) == button2.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name3 ) == button3.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name4 ) == button4.get() );
   CPPUNIT_ASSERT_MESSAGE("New button should now have the same index as the old button",
      indices[0] == toolbar->GetElementIndex( *button2 ) );
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 3 );
   // Not testing the tmpButton this iteration

   // --- Swap last button
   toolbar->ReplaceButton( name3, button1, &tmpButton );
   CPPUNIT_ASSERT( toolbar->GetButton( name1 ) == button1.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name2 ) == button2.get() );
   CPPUNIT_ASSERT( toolbar->GetButton( name3 ) == nullptr );
   CPPUNIT_ASSERT( toolbar->GetButton( name4 ) == button4.get() );
   CPPUNIT_ASSERT_MESSAGE("New button should now have the same index as the old button",
      indices[2] == toolbar->GetElementIndex( *button1 ) );
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 3 );
   CPPUNIT_ASSERT( tmpButton.get() == button3.get() );



   // Remove remaining elements
   toolbar->ClearButtons();
   CPPUNIT_ASSERT( toolbar->GetButtonCount() == 0 );

   // Start and end elements should still remain
   CPPUNIT_ASSERT_MESSAGE( "HUD Toolbar should still have start and end elements",
      toolbar->GetStartElement() != nullptr
      && toolbar->GetEndElement() != nullptr );

}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthMeter()
{
   std::shared_ptr<SimCore::Components::StealthMeter> meter =
      new SimCore::Components::StealthMeter("TestStealthMeter");

   meter->Initialize();

   meter->SetValue( 55.0f, 300.0f, 0.0f );
   CPPUNIT_ASSERT_MESSAGE( "Stealth Meter should have the correct numeric value",
      meter->GetValue() == 55.0f );
   CPPUNIT_ASSERT_MESSAGE( "Stealth Meter should have the correct text value",
      meter->GetTextElement().GetText() == "55" );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthHealthMeter()
{
   // NOTE: This meter is a simple subclass of a bar meter.
   // This function merely tests that this object can
   // initialize properly.
   std::shared_ptr<SimCore::Components::StealthHealthMeter> healthMeter =
      new SimCore::Components::StealthHealthMeter("TestHealthMeter");

   healthMeter->Initialize();
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthAmmoMeter()
{
   // NOTE: This meter is a simple subclass of a unit bar meter.
   // This function merely tests that this object can
   // initialize properly.
   std::shared_ptr<SimCore::Components::StealthAmmoMeter> ammoMeter =
      new SimCore::Components::StealthAmmoMeter("TestAmmoMeter");

   ammoMeter->Initialize();
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthCompassMeter()
{
   // NOTE: This meter is a simple subclass of slide bar meter.
   // This function merely tests that this object can
   // initialize properly.
   std::shared_ptr<SimCore::Components::StealthCompassMeter> compassMeter =
      new SimCore::Components::StealthCompassMeter("TestCompassMeter");

   compassMeter->Initialize();

   // Test value setting
   compassMeter->SetValue( 90.0f, 180.0f, -180.0f );
   float testValue = compassMeter->GetMeterElement().GetScale();
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.75f, testValue, 0.01f );

   compassMeter->SetValue( 270.0f, 180.0f, -180.0f );
   testValue = compassMeter->GetMeterElement().GetScale();
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.25f, testValue, 0.01f );

   compassMeter->SetValue( -270.0f, 180.0f, -180.0f );
   testValue = compassMeter->GetMeterElement().GetScale();
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.75f, testValue, 0.01f );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthGPSMeter()
{
   std::shared_ptr<SimCore::Components::StealthGPSMeter> gpsMeter =
      new SimCore::Components::StealthGPSMeter("TestGPSMeter");

   // Test variables
   std::string value1, value2;

   // Test setting values
   gpsMeter->SetLatitude( -30.5f );
   gpsMeter->SetLongitude( 60.5f );
   value1 = "-30.5";
   value2 = "60.5";
   CPPUNIT_ASSERT_MESSAGE( "GPS Meter should have new latitude/longitude text",
      value1 == gpsMeter->GetText1().GetText()
      && value2 == gpsMeter->GetText2().GetText() );

   gpsMeter->SetLatLong( 10.0f, -90.0f );
   value1 = "10";
   value2 = "-90";
   CPPUNIT_ASSERT_MESSAGE( "GPS Meter should have new latitude/longitude text, again",
      value1 == gpsMeter->GetText1().GetText()
      && value2 == gpsMeter->GetText2().GetText() );

   value1 = "RECORD";
   value2 = "Frame 10";
   gpsMeter->SetText1( value1 );
   gpsMeter->SetText2( value2 );
   CPPUNIT_ASSERT_MESSAGE( "GPS Meter should have new text",
      value1 == gpsMeter->GetText1().GetText()
      && value2 == gpsMeter->GetText2().GetText() );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthMGRSMeter()
{
   std::shared_ptr<SimCore::Components::StealthMGRSMeter> gmrsMeter =
      new SimCore::Components::StealthMGRSMeter("TestMGRSMeter");

   // Test variables
   std::string value("Test Text");

   // Test setting values
   CPPUNIT_ASSERT_MESSAGE( "MGRS Meter should have new latitude/longitude text",
      value != gmrsMeter->GetText().GetText() );

   gmrsMeter->SetText( value );
   CPPUNIT_ASSERT_MESSAGE( "MGRS Meter should have new text",
      value == gmrsMeter->GetText().GetText() );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthCartesianMeter()
{
   std::shared_ptr<SimCore::Components::StealthCartesianMeter> cartMeter =
      new SimCore::Components::StealthCartesianMeter("TestCartesianMeter");

   // Test variables
   std::string value1, value2, value3;

   // Test setting values
   CPPUNIT_ASSERT_MESSAGE( "GPS Meter should have new latitude/longitude text",
      value1 != cartMeter->GetX().GetText()
      && value2 != cartMeter->GetY().GetText()
      && value3 != cartMeter->GetZ().GetText() );

   cartMeter->SetX( -30.5f, "M");
   cartMeter->SetY( 60.5f, "M" );
   cartMeter->SetZ( 777.888f, "M" );
   value1 = "-30.5 M";
   value2 = "60.5 M";
   value3 = "777.888 M";

   CPPUNIT_ASSERT_MESSAGE( "GPS Meter should have new latitude/longitude text",
      value1 == cartMeter->GetX().GetText()
      && value2 == cartMeter->GetY().GetText()
      && value3 == cartMeter->GetZ().GetText() );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthSpeedometer()
{
   std::shared_ptr<SimCore::Components::StealthSpeedometer> speedometer =
      new SimCore::Components::StealthSpeedometer("TestSpeedometer");
   // NOTE: The needle is NOT a CEGUI object.
   // It will need to be tested to make sure it is updated properly
   // with the containing speedometer element.
   // It is different in that it has the ability and interface to rotate,
   // which is something CEGUI currently lacks.
   const SimCore::Components::HUDQuadElement* needle = speedometer->GetNeedle();

   // Register the needle; the needle is a special case
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
   CPPUNIT_ASSERT_MESSAGE( "Speedometer must be able to register its needle with the main CEGUI dtCore::DeltaDrawable",
      speedometer->RegisterNeedleWithGUI(mGUI.get()) );
#else
   speedometer->RegisterNeedleWithGUI(&GetGlobalGUI());
#endif

   // Test visibility
   CPPUNIT_ASSERT_MESSAGE( "Speedometer should be visible by default",
      speedometer->IsVisible() && needle->IsVisible() );
   speedometer->SetVisible( false );
   CPPUNIT_ASSERT_MESSAGE( "Speedometer should now be hidden",
      ! speedometer->IsVisible() && ! needle->IsVisible() );
   speedometer->SetVisible( true );
   CPPUNIT_ASSERT_MESSAGE( "Speedometer should now be visible",
      speedometer->IsVisible() && needle->IsVisible() );

   // Test value setting
   // NOTE: needle rotation is opposite to normal rotation.
   // It starts at 1 radian at 0 mph and goes to 0 radians when
   // the speedometer approaches the max speed.
   const float pi = 3.141593f;
   std::string testValue;
   speedometer->SetValue( 30.0f, 120.0f, 0.0f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.75f*pi, needle->GetRotation(), 0.01f );

   // --- Test that the needle caps its rotation at the max.
   speedometer->SetValue( 130.0f, 120.0f, 0.0f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0f, needle->GetRotation(), 0.01f );

   // --- Test that the needle caps its rotation at the min.
   speedometer->SetValue( -30.0f, 120.0f, 0.0f );
   CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0f*pi, needle->GetRotation(), 0.01f );

   // Test clean up
   CPPUNIT_ASSERT( speedometer->UnregisterNeedleWithGUI() );
}

//////////////////////////////////////////////////////////////
void HUDElementsTests::TestStealthCallSign()
{
   std::shared_ptr<SimCore::Components::StealthCallSign> callsign =
      new SimCore::Components::StealthCallSign("TestCallSign");

   callsign->SetCallSign("Lorem Ipsum");
   CPPUNIT_ASSERT_MESSAGE( "CallSign should have a new text value",
      callsign->GetTextElement()->GetText() == "Lorem Ipsum" );
}
