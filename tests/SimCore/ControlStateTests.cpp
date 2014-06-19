/* -*-c++-*-
* Simulation Core - ControlStateTests (.h & .cpp) - Using 'The MIT License'
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
#include <dtUtil/macros.h>
#include <dtCore/system.h>
#include <dtCore/refptr.h>
#include <dtCore/scene.h>
#include <dtCore/project.h>
#include <dtGame/gamemanager.h>

#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/MessageType.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>


#ifdef DELTA_WIN32
#include <dtUtil/mswin.h>
#define SLEEP(milliseconds) Sleep((milliseconds))
const std::string projectContext = "ProjectAssets";
#else
#include <unistd.h>
#define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
const std::string projectContext = "ProjectAssets";
#endif

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Tests Object
      //////////////////////////////////////////////////////////////////////////
      class ControlStateTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(ControlStateTests);

         CPPUNIT_TEST(TestDiscreteControlTypeProperties);
         CPPUNIT_TEST(TestContinuousControlTypeProperties);
         CPPUNIT_TEST(TestControlStateActorProperties);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            dtCore::RefPtr<SimCore::Actors::DiscreteControl> CreateDiscreteControl(
               const std::string& controlName, unsigned long totalStates, long currentState );

            dtCore::RefPtr<SimCore::Actors::ContinuousControl> CreateContinuousControl(
               const std::string& controlName, float minValue, float maxValue, float value );

            void TestControlGroupParameter(
               const dtCore::NamedGroupParameter& testGroupParam,
               const SimCore::Actors::DiscreteControl& controlsMap );

            void TestControlGroupParameter(
               const dtCore::NamedGroupParameter& testGroupParam,
               const SimCore::Actors::ContinuousControl& controlsMap );

            template<class T_ControlType>
            void TestControlArrayGroupParameter(
               const dtCore::NamedGroupParameter& testGroupParam,
               const std::map<const std::string, dtCore::RefPtr<T_ControlType> >& controlsMap );

            void TestControlTypeName( SimCore::Actors::BaseControl& controlType );

            // Test Functions:
            void TestDiscreteControlTypeProperties();
            void TestContinuousControlTypeProperties();
            void TestControlStateActorProperties();

         private:
            dtCore::RefPtr<dtGame::GameManager> mGM;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(ControlStateTests);



      //////////////////////////////////////////////////////////////////////////
      // Tests code
      //////////////////////////////////////////////////////////////////////////
      void ControlStateTests::setUp()
      {
         try
         {
            mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateTests::tearDown()
      {
         try
         {
            mGM->DeleteAllActors(true);
            mGM = NULL;
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<SimCore::Actors::DiscreteControl> ControlStateTests::CreateDiscreteControl(
         const std::string& controlName, unsigned long totalStates, long currentState )
      {
         dtCore::RefPtr<SimCore::Actors::DiscreteControl> control = new SimCore::Actors::DiscreteControl;
         control->SetName( controlName );
         control->SetTotalStates( totalStates );
         control->SetCurrentState( currentState );
         return control;
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<SimCore::Actors::ContinuousControl> ControlStateTests::CreateContinuousControl(
         const std::string& controlName, float minValue, float maxValue, float value )
      {
         dtCore::RefPtr<SimCore::Actors::ContinuousControl> control = new SimCore::Actors::ContinuousControl;
         control->SetName( controlName );
         control->SetMinValue( minValue );
         control->SetMaxValue( maxValue );
         control->SetValue( value );
         return control;
      }

      //////////////////////////////////////////////////////////////////////////
      // OVERLOAD: DISCRETE CONTROL
      void ControlStateTests::TestControlGroupParameter(
         const dtCore::NamedGroupParameter& testGroupParam,
         const SimCore::Actors::DiscreteControl& control )
      {
         CPPUNIT_ASSERT_MESSAGE("A discrete control should create a NamedGroupParameter that shares the same name",
            testGroupParam.GetName() == control.GetName() );

         // Test total states
         const dtCore::NamedUnsignedIntParameter* paramTotalStates
            = dynamic_cast<const dtCore::NamedUnsignedIntParameter*>
            (testGroupParam.GetParameter( SimCore::Actors::DiscreteControl::PARAM_NAME_TOTAL_STATES ));

         CPPUNIT_ASSERT( paramTotalStates != NULL );
         CPPUNIT_ASSERT( control.GetTotalStates() == paramTotalStates->GetValue() );

         // Test current state
         const dtCore::NamedIntParameter* paramCurrentState
            = dynamic_cast<const dtCore::NamedIntParameter*>
            (testGroupParam.GetParameter( SimCore::Actors::DiscreteControl::PARAM_NAME_CURRENT_STATE ));

         CPPUNIT_ASSERT( paramCurrentState != NULL );
         CPPUNIT_ASSERT( control.GetCurrentState() == paramCurrentState->GetValue() );
      }

      //////////////////////////////////////////////////////////////////////////
      // OVERLOAD: CONTINUOUS CONTROL
      void ControlStateTests::TestControlGroupParameter(
         const dtCore::NamedGroupParameter& testGroupParam,
         const SimCore::Actors::ContinuousControl& control )
      {
         double errorThreshold = 0.0001;
         CPPUNIT_ASSERT_MESSAGE("A continuous control should create a NamedGroupParameter that shares the same name",
            testGroupParam.GetName() == control.GetName() );

         const dtCore::NamedFloatParameter* param
            = dynamic_cast<const dtCore::NamedFloatParameter*>
            (testGroupParam.GetParameter( SimCore::Actors::ContinuousControl::PARAM_NAME_VALUE_MIN ));

         CPPUNIT_ASSERT( param != NULL );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( control.GetMinValue(), param->GetValue(), errorThreshold );

         // Set max value
         param = dynamic_cast<const dtCore::NamedFloatParameter*>
            (testGroupParam.GetParameter( SimCore::Actors::ContinuousControl::PARAM_NAME_VALUE_MAX ));

         CPPUNIT_ASSERT( param != NULL );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( control.GetMaxValue(), param->GetValue(), errorThreshold );

         // Set value
         param = dynamic_cast<const dtCore::NamedFloatParameter*>
            (testGroupParam.GetParameter( SimCore::Actors::ContinuousControl::PARAM_NAME_VALUE ));

         CPPUNIT_ASSERT( param != NULL );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( control.GetValue(), param->GetValue(), errorThreshold );
      }

      //////////////////////////////////////////////////////////////////////////
      template<class T_ControlType>
      void ControlStateTests::TestControlArrayGroupParameter(
         const dtCore::NamedGroupParameter& testGroupParam,
         const std::map<const std::string, dtCore::RefPtr<T_ControlType> >& controlsMap )
      {
         // Ensure that the collections of controls are the same size.
         CPPUNIT_ASSERT( testGroupParam.GetParameterCount() == controlsMap.size() );

         const T_ControlType* currentControl = NULL;
         const dtCore::NamedGroupParameter* currentGroupParam = NULL;

         typedef typename std::map<const std::string, dtCore::RefPtr<T_ControlType> >::const_iterator constMapIter;
         constMapIter iter = controlsMap.begin();

         // Test each control in the map for equality with a group parameter of the same name.
         for( ; iter != controlsMap.end(); ++iter )
         {
            // Access the current control
            currentControl = iter->second.get();
            CPPUNIT_ASSERT_MESSAGE("Control state actor should NEVER have NULL entries in its control maps.",
               currentControl != NULL );

            // Access the current group parameter
            currentGroupParam = dynamic_cast<const dtCore::NamedGroupParameter*>
               (testGroupParam.GetParameter( currentControl->GetName() ));
            CPPUNIT_ASSERT_MESSAGE("Control state actor should add control group parameters by control name to the higher level control array group parameter",
               currentGroupParam != NULL );

            // Compare the group parameter to the actual control
            TestControlGroupParameter( *currentGroupParam, *currentControl );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateTests::TestControlTypeName( SimCore::Actors::BaseControl& controlType )
      {
         // Test setting the name
         std::string testName( "validName" );
         CPPUNIT_ASSERT( testName.length() < SimCore::Actors::BaseControl::NAME_LENGTH );
         controlType.SetName( testName );
         CPPUNIT_ASSERT( controlType.GetName() == testName );

         // Test setting the name longer than the
         // limit specified by the object's class;
         // the name length should be clamped to the limit.
         CPPUNIT_ASSERT( SimCore::Actors::BaseControl::NAME_LENGTH == 64 );
         // The following name should be 64 characters up to the final '8'.
         // The 0's should be the extra characters to be cut.
         testName = "123456781234567812345678123456781234567812345678123456781234567800000000";
         CPPUNIT_ASSERT( testName.length() > SimCore::Actors::BaseControl::NAME_LENGTH );

         // Test name
         controlType.SetName( testName );
         CPPUNIT_ASSERT( controlType.GetName() == testName );

         // Test retrieving the buffer safe name
         const std::string outName( controlType.GetEncodableName() );
         CPPUNIT_ASSERT( outName != testName );
         CPPUNIT_ASSERT( outName.length() == SimCore::Actors::BaseControl::NAME_LENGTH );
         CPPUNIT_ASSERT( outName == testName.substr(0,SimCore::Actors::BaseControl::NAME_LENGTH) );

         // NOTE: The name length of a control type usually
         // will not approach the NAME_LENGTH limit. If this does
         // happen, a better nomenclature system should be adopted
         // by the user that exceeded the name size limit.
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateTests::TestDiscreteControlTypeProperties()
      {
         dtCore::RefPtr<SimCore::Actors::DiscreteControl> discreteControl
            = new SimCore::Actors::DiscreteControl;

         CPPUNIT_ASSERT( discreteControl->GetName().empty() );
         TestControlTypeName( *discreteControl );

         CPPUNIT_ASSERT( discreteControl->GetTotalStates() == 0 );
         discreteControl->SetTotalStates( 5 );
         CPPUNIT_ASSERT( discreteControl->GetTotalStates() == 5 );

         CPPUNIT_ASSERT( discreteControl->GetCurrentState() == 0 );
         discreteControl->SetCurrentState( 20 );
         CPPUNIT_ASSERT( discreteControl->GetCurrentState() == 20 );
         discreteControl->SetCurrentState( -12 );
         CPPUNIT_ASSERT( discreteControl->GetCurrentState() == -12 );

         // Test Encode & Decode
         dtCore::RefPtr<SimCore::Actors::DiscreteControl> decodedControl
            = new SimCore::Actors::DiscreteControl;
         CPPUNIT_ASSERT( decodedControl->GetName().empty() );
         CPPUNIT_ASSERT( decodedControl->GetTotalStates() == 0 );
         CPPUNIT_ASSERT( decodedControl->GetCurrentState() == 0 );

         // --- Encode/Decode
         char* buffer = new char[SimCore::Actors::ContinuousControl::CONTROL_BYTE_SIZE];
         discreteControl->Encode( buffer );
         decodedControl->Decode( buffer );
         delete buffer;
         buffer = NULL;

         // --- Compare decoding results
         CPPUNIT_ASSERT( ! decodedControl->GetName().empty() );
         CPPUNIT_ASSERT( decodedControl->GetTotalStates() == discreteControl->GetTotalStates() );
         CPPUNIT_ASSERT( decodedControl->GetCurrentState() == discreteControl->GetCurrentState() );



         // Test creation of a group message parameter that represents the control.
         dtCore::RefPtr<dtCore::NamedGroupParameter> groupParam = discreteControl->GetAsGroupParameter();
         CPPUNIT_ASSERT( groupParam.valid() );
         TestControlGroupParameter( *groupParam, *discreteControl );

         // Test assignment by a group message parameter. decodedControl
         // --- Use a fresh control
         decodedControl = new SimCore::Actors::DiscreteControl;
         CPPUNIT_ASSERT( decodedControl->GetName().empty() );
         CPPUNIT_ASSERT( decodedControl->GetTotalStates() == 0 );
         CPPUNIT_ASSERT( decodedControl->GetCurrentState() == 0 );
         // --- Assign the values by group parameter
         decodedControl->SetByGroupParameter( *groupParam );
         TestControlGroupParameter( *groupParam, *decodedControl );
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateTests::TestContinuousControlTypeProperties()
      {
         dtCore::RefPtr<SimCore::Actors::ContinuousControl> continuousControl
            = new SimCore::Actors::ContinuousControl;

         CPPUNIT_ASSERT( continuousControl->GetName().empty() );
         TestControlTypeName( *continuousControl );

         // NOTE: All test values are arbitrary.

         CPPUNIT_ASSERT( continuousControl->GetMinValue() == 0.0 );
         continuousControl->SetMinValue( 10.0 );
         CPPUNIT_ASSERT( continuousControl->GetMinValue() == 10.0 );

         CPPUNIT_ASSERT( continuousControl->GetMaxValue() == 0.0 );
         continuousControl->SetMaxValue( 52.5 );
         CPPUNIT_ASSERT( continuousControl->GetMaxValue() == 52.5 );

         CPPUNIT_ASSERT( continuousControl->GetValue() == 0.0 );
         continuousControl->SetValue( 39.3 );
         CPPUNIT_ASSERT( continuousControl->GetValue() == 39.3 );

         // Test Encode & Decode
         dtCore::RefPtr<SimCore::Actors::ContinuousControl> decodedControl
            = new SimCore::Actors::ContinuousControl;
         CPPUNIT_ASSERT( decodedControl->GetName().empty() );
         CPPUNIT_ASSERT( decodedControl->GetMinValue() == 0.0 );
         CPPUNIT_ASSERT( decodedControl->GetMaxValue() == 0.0 );
         CPPUNIT_ASSERT( decodedControl->GetValue() == 0.0 );

         // --- Encode/Decode
         char* buffer = new char[SimCore::Actors::ContinuousControl::CONTROL_BYTE_SIZE];
         continuousControl->Encode( buffer );
         decodedControl->Decode( buffer );
         delete buffer;
         buffer = NULL;

         // --- Compare decoding results
         CPPUNIT_ASSERT( ! decodedControl->GetName().empty() );
         CPPUNIT_ASSERT( decodedControl->GetName() != continuousControl->GetName() );
         CPPUNIT_ASSERT( decodedControl->GetName() == continuousControl->GetEncodableName() );
         CPPUNIT_ASSERT( decodedControl->GetMinValue() == continuousControl->GetMinValue() );
         CPPUNIT_ASSERT( decodedControl->GetMaxValue() == continuousControl->GetMaxValue() );
         CPPUNIT_ASSERT( decodedControl->GetValue() == continuousControl->GetValue() );



         // Test creation of a group message parameter that represents the control.
         dtCore::RefPtr<dtCore::NamedGroupParameter> groupParam = continuousControl->GetAsGroupParameter();
         CPPUNIT_ASSERT( groupParam.valid() );
         TestControlGroupParameter( *groupParam, *continuousControl );

         // Test assignment by a group message parameter. decodedControl
         // --- Use a fresh control
         decodedControl = new SimCore::Actors::ContinuousControl;
         CPPUNIT_ASSERT( decodedControl->GetName().empty() );
         CPPUNIT_ASSERT( decodedControl->GetMinValue() == 0.0 );
         CPPUNIT_ASSERT( decodedControl->GetMaxValue() == 0.0 );
         CPPUNIT_ASSERT( decodedControl->GetValue() == 0.0 );
         // --- Assign the values by group parameter
         decodedControl->SetByGroupParameter( *groupParam );
         TestControlGroupParameter( *groupParam, *decodedControl );
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateTests::TestControlStateActorProperties()
      {
         dtCore::RefPtr<SimCore::Actors::ControlStateProxy> proxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE, proxy );
         CPPUNIT_ASSERT_MESSAGE("Game manager should be able to create a ControlActorProxy",
            proxy.valid() );

         dtCore::RefPtr<SimCore::Actors::ControlStateActor> controlState
            = static_cast<SimCore::Actors::ControlStateActor*>(&proxy->GetGameActor());


         dtCore::RefPtr<SimCore::Actors::PlatformActorProxy> testEntityProxy;
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, testEntityProxy );
         CPPUNIT_ASSERT_MESSAGE("Game manager should be able to create an Entity",
            proxy.valid() );

         dtCore::RefPtr<SimCore::Actors::Platform> testEntity
            = static_cast<SimCore::Actors::Platform*>(&testEntityProxy->GetGameActor());

         // Test setting an entity
         CPPUNIT_ASSERT( controlState->GetEntityID().ToString().empty() );
         CPPUNIT_ASSERT( controlState->GetEntity() == NULL );
         controlState->SetEntity( testEntityProxy.get() );
         CPPUNIT_ASSERT( controlState->GetEntity() == testEntity.get() );
         CPPUNIT_ASSERT( controlState->GetEntityID() == testEntity->GetUniqueId());

         // Test station type
         CPPUNIT_ASSERT( controlState->GetStationType() == 0 );
         controlState->SetStationType( 15 );
         CPPUNIT_ASSERT( controlState->GetStationType() == 15 );

         // Test number discrete controls
         CPPUNIT_ASSERT( controlState->GetNumDiscreteControls() == 0 );
         controlState->SetNumDiscreteControls( 7 );
         CPPUNIT_ASSERT( controlState->GetNumDiscreteControls() == 7 );

         // Test number continuous controls
         CPPUNIT_ASSERT( controlState->GetNumContinuousControls() == 0 );
         controlState->SetNumContinuousControls( 4 );
         CPPUNIT_ASSERT( controlState->GetNumContinuousControls() == 4 );



         // Test Adding DiscreteControl Controls
         CPPUNIT_ASSERT( controlState->GetDiscreteControlCount() == 0 );
         CPPUNIT_ASSERT( controlState->GetDiscreteControls().size() == 0 );
         dtCore::RefPtr<SimCore::Actors::DiscreteControl> dc1 = CreateDiscreteControl( "DC1", 4, 1 );
         dtCore::RefPtr<SimCore::Actors::DiscreteControl> dc2 = CreateDiscreteControl( "DC2", 8, 2 );
         dtCore::RefPtr<SimCore::Actors::DiscreteControl> dc3 = CreateDiscreteControl( "DC3", 12, 3 );
         dtCore::RefPtr<SimCore::Actors::DiscreteControl> dc4 = CreateDiscreteControl( "DC4", 16, 4 );
         // --- Add some by pointer
         CPPUNIT_ASSERT( controlState->AddControl( dc1 ) );
         CPPUNIT_ASSERT( controlState->AddControl( dc2 ) );
         // ------ Ensure that re-adding the same controls does NOT succeed
         CPPUNIT_ASSERT( ! controlState->AddControl( dc1 ) );
         CPPUNIT_ASSERT( ! controlState->AddControl( dc2 ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControlCount() == 2 );
         CPPUNIT_ASSERT( controlState->GetDiscreteControls().size() == 2 );
         // --- Add some by group parameters
         CPPUNIT_ASSERT( controlState->AddControl( dc3 ) );
         CPPUNIT_ASSERT( controlState->AddControl( dc4 ) );
         // ------ Ensure that re-adding the same controls does NOT succeed
         CPPUNIT_ASSERT( ! controlState->AddControl( dc3 ) );
         CPPUNIT_ASSERT( ! controlState->AddControl( dc4 ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControlCount() == 4 );
         CPPUNIT_ASSERT( controlState->GetDiscreteControls().size() == 4 );
         // --- Test accessing individual controls
         SimCore::Actors::DiscreteControl* testDiscreteControl
            = controlState->GetDiscreteControl( dc1->GetName() );
         CPPUNIT_ASSERT( testDiscreteControl != NULL );
         CPPUNIT_ASSERT( testDiscreteControl == dc1.get() );
         testDiscreteControl = controlState->GetDiscreteControl( dc2->GetName() );
         CPPUNIT_ASSERT( testDiscreteControl != NULL );
         CPPUNIT_ASSERT( testDiscreteControl == dc2.get() );
         testDiscreteControl = controlState->GetDiscreteControl( dc3->GetName() );
         CPPUNIT_ASSERT( testDiscreteControl != NULL );
         CPPUNIT_ASSERT( testDiscreteControl == dc3.get() );
         testDiscreteControl = controlState->GetDiscreteControl( dc4->GetName() );
         CPPUNIT_ASSERT( testDiscreteControl != NULL );
         CPPUNIT_ASSERT( testDiscreteControl == dc4.get() );



         // Test Adding ContinuousControl Controls
         SimCore::Actors::ContinuousControl* testContinuousControl = NULL;
         CPPUNIT_ASSERT( controlState->GetContinuousControlCount() == 0 );
         CPPUNIT_ASSERT( controlState->GetContinuousControls().size() == 0 );
         dtCore::RefPtr<SimCore::Actors::ContinuousControl> cc1 = CreateContinuousControl( "CC1", 1.0f, 2.5f, 1.25f );
         dtCore::RefPtr<SimCore::Actors::ContinuousControl> cc2 = CreateContinuousControl( "CC2", 2.0f, 3.5f, 3.25f );
         dtCore::RefPtr<SimCore::Actors::ContinuousControl> cc3 = CreateContinuousControl( "CC3", 4.0f, 5.5f, 4.25f );
         dtCore::RefPtr<SimCore::Actors::ContinuousControl> cc4 = CreateContinuousControl( "CC4", 5.0f, 6.5f, 5.25f );
         // --- Add some by pointer
         CPPUNIT_ASSERT( controlState->AddControl( cc1 ) );
         CPPUNIT_ASSERT( controlState->AddControl( cc2 ) );
         // ------ Ensure that re-adding the same controls does NOT succeed
         CPPUNIT_ASSERT( ! controlState->AddControl( cc1 ) );
         CPPUNIT_ASSERT( ! controlState->AddControl( cc2 ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControlCount() == 2 );
         CPPUNIT_ASSERT( controlState->GetContinuousControls().size() == 2 );
         // --- Add some by group parameters
         CPPUNIT_ASSERT( controlState->AddControl( cc3 ) );
         CPPUNIT_ASSERT( controlState->AddControl( cc4 ) );
         // ------ Ensure that re-adding the same controls does NOT succeed
         CPPUNIT_ASSERT( ! controlState->AddControl( cc3 ) );
         CPPUNIT_ASSERT( ! controlState->AddControl( cc4 ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControlCount() == 4 );
         CPPUNIT_ASSERT( controlState->GetContinuousControls().size() == 4 );
         // --- Test accessing individual controls
         testContinuousControl = controlState->GetContinuousControl( cc1->GetName() );
         CPPUNIT_ASSERT( testContinuousControl != NULL );
         CPPUNIT_ASSERT( testContinuousControl == cc1.get() );
         testContinuousControl = controlState->GetContinuousControl( cc2->GetName() );
         CPPUNIT_ASSERT( testContinuousControl != NULL );
         CPPUNIT_ASSERT( testContinuousControl == cc2.get() );
         testContinuousControl = controlState->GetContinuousControl( cc3->GetName() );
         CPPUNIT_ASSERT( testContinuousControl != NULL );
         CPPUNIT_ASSERT( testContinuousControl == cc3.get() );
         testContinuousControl = controlState->GetContinuousControl( cc4->GetName() );
         CPPUNIT_ASSERT( testContinuousControl != NULL );
         CPPUNIT_ASSERT( testContinuousControl == cc4.get() );



         // Test encode & decode of the control arrays
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE, proxy );
         CPPUNIT_ASSERT_MESSAGE("Game manager should be able to create a ControlActorProxy",
            proxy.valid() );
         dtCore::RefPtr<SimCore::Actors::ControlStateActor> decodedControlState
            = static_cast<SimCore::Actors::ControlStateActor*>(&proxy->GetGameActor());
         CPPUNIT_ASSERT( decodedControlState->GetEntityID().ToString().empty() );
         CPPUNIT_ASSERT( decodedControlState->GetStationType() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetDiscreteControlCount() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetContinuousControlCount() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetNumDiscreteControls() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetNumContinuousControls() == 0 );



         // --- Encode & decode discrete controls
         dtCore::RefPtr<dtCore::NamedGroupParameter> discreteArray
            = controlState->GetDiscreteControlsAsGroupParameter();
         CPPUNIT_ASSERT( discreteArray.valid() );

         // --- Ensure group parameter has good values and then decode it to
         //     the receiving control state actor.
         TestControlArrayGroupParameter( *discreteArray, controlState->GetDiscreteControls() );
         decodedControlState->SetDiscreteControlsByGroupParameter( *discreteArray );
         // --- Compare the assigned values
         CPPUNIT_ASSERT( decodedControlState->GetDiscreteControlCount() == controlState->GetDiscreteControlCount() );
         TestControlArrayGroupParameter( *discreteArray, decodedControlState->GetDiscreteControls() );



         // --- Encode & decode continuous controls
         dtCore::RefPtr<dtCore::NamedGroupParameter> continuousArray
            = controlState->GetContinuousControlsAsGroupParameter();
         CPPUNIT_ASSERT( continuousArray.valid() );

         // --- Ensure group parameter has good values and then decode it to
         //     the receiving control state actor.
         TestControlArrayGroupParameter( *continuousArray, controlState->GetContinuousControls() );
         decodedControlState->SetContinuousControlsByGroupParameter( *continuousArray );
         // --- Compare the assigned values
         CPPUNIT_ASSERT( decodedControlState->GetContinuousControlCount() == controlState->GetContinuousControlCount() );
         TestControlArrayGroupParameter( *continuousArray, decodedControlState->GetContinuousControls() );



         // Test removing controls (from original control state)
         // --- Remove by reference
         CPPUNIT_ASSERT( ! controlState->RemoveControl( NULL ) );

         CPPUNIT_ASSERT( controlState->RemoveControl( dc1.get() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControl( dc1->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveControl( dc1.get() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControls().size() == 3 );

         CPPUNIT_ASSERT( controlState->RemoveControl( dc3.get() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControl( dc3->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveControl( dc3.get() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControls().size() == 2 );

         CPPUNIT_ASSERT( controlState->RemoveControl( cc4.get() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControl( cc4->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveControl( cc4.get() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControls().size() == 3 );

         CPPUNIT_ASSERT( controlState->RemoveControl( cc2.get() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControl( cc2->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveControl( cc2.get() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControls().size() == 2 );

         // --- Remove by name
         CPPUNIT_ASSERT( controlState->RemoveDiscreteControl( dc4->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControl( dc4->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveDiscreteControl( dc4->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControls().size() == 1 );

         CPPUNIT_ASSERT( controlState->RemoveDiscreteControl( dc2->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControl( dc2->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveDiscreteControl( dc2->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetDiscreteControls().size() == 0 );

         CPPUNIT_ASSERT( controlState->RemoveContinuousControl( cc1->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControl( cc1->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveContinuousControl( cc1->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControls().size() == 1 );

         CPPUNIT_ASSERT( controlState->RemoveContinuousControl( cc3->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControl( cc3->GetName() ) == NULL );
         CPPUNIT_ASSERT( ! controlState->RemoveContinuousControl( cc3->GetName() ) );
         CPPUNIT_ASSERT( controlState->GetContinuousControls().size() == 0 );



         // Test clear (on the decoded control state)
         decodedControlState->Clear();
         CPPUNIT_ASSERT( decodedControlState->GetEntityID().ToString().empty() );
         CPPUNIT_ASSERT( decodedControlState->GetStationType() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetDiscreteControlCount() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetContinuousControlCount() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetNumDiscreteControls() == 0 );
         CPPUNIT_ASSERT( decodedControlState->GetNumContinuousControls() == 0 );
      }
   }
}
