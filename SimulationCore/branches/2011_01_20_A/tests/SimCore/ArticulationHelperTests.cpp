/* -*-c++-*-
* Simulation Core - ArticulationHelperTests (.h & .cpp) - Using 'The MIT License'
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
#include <dtCore/system.h>
#include <dtCore/refptr.h>
#include <dtCore/scene.h>
#include <dtUtil/nodecollector.h>
#include <dtDAL/namedparameter.h>
#include <dtDAL/project.h>
#include <dtGame/gamemanager.h>
#include <osgSim/DOFTransform>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Components/ArticulationHelper.h>
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
      class TestArticHelper;

      //////////////////////////////////////////////////////////////////////////
      // TESTS OBJECT
      //////////////////////////////////////////////////////////////////////////
      class ArticulationHelperTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(ArticulationHelperTests);

         CPPUNIT_TEST(TestArticulationHelperProperties);
         CPPUNIT_TEST(TestArticulationHelperUpating);

         CPPUNIT_TEST_SUITE_END();

      public:

         void setUp();
         void tearDown();

         // Utility Functions:
         dtCore::RefPtr<dtDAL::NamedGroupParameter> CreateIncomingGroupParameter( 
            SimCore::Components::ArticulationMetricType& type );

         // Sub-Test Functions:
         void SubTestArticArrayGroupProperty(
            const dtCore::RefPtr<dtDAL::NamedGroupParameter>& groupProperty,
            const TestArticHelper& articHelper );

         // Test Functions:
         void TestArticulationHelperProperties();
         void TestArticulationHelperUpating();

      private:
         dtCore::RefPtr<dtGame::GameManager> mGM;
         dtCore::RefPtr<TestArticHelper> mHelper;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(ArticulationHelperTests);



      //////////////////////////////////////////////////////////////////////////
      // TEST ARTICULATION HELPER
      //////////////////////////////////////////////////////////////////////////
      class TestArticHelper : public SimCore::Components::ArticulationHelper
      {
         public:
            static const std::string DOF_TURRET;
            static const std::string DOF_TURRET_METRIC_AZIMUTH;
            static const std::string DOF_TURRET_METRIC_AZIMUTH_RATE;

            static const std::string DOF_WEAPON;
            static const std::string DOF_WEAPON_METRIC_ELEVATION;
            static const std::string DOF_WEAPON_METRIC_ELEVATION_RATE;

            TestArticHelper()
            : mTurretAzimuth(0.5f)
            , mTurretAzimuthRate(0.1f)
            , mWeaponElevation(13.6f)
            , mWeaponElevationRate(-1.1f)
            {}

            // Inherited Override Functions
            virtual dtCore::RefPtr<dtDAL::NamedGroupParameter> BuildGroupProperty();

            virtual void UpdateDOFReferences( dtUtil::NodeCollector* nodeCollector );

            virtual bool HasDOF( osgSim::DOFTransform& dof ) const;

            virtual bool HasDOFMetric( const osgSim::DOFTransform& dof, const std::string& metricName ) const;

            virtual void HandleUpdatedDOF( const osgSim::DOFTransform& dof,
               const osg::Vec3& transChange, const osg::Vec3& hprChange );

            // Accessors & Mutators
            void SetTestMetricValue( const std::string& metricName, float value );
            float GetTestMetricValue( const std::string& metricName ) const;

            // Utility Functions for Tests
            const dtDAL::NamedGroupParameter* GetMetricParameter( 
               const dtDAL::NamedGroupParameter& articulatedParam,
               const std::string& metricParamName ) const;

            float GetMetricValue( const dtDAL::NamedGroupParameter& metricParam ) const;

            void GetMetricDOFNames( const dtDAL::NamedGroupParameter& metricParam,
               std::string& outDOFName, std::string& outDOFParentName ) const;

         protected:
            virtual ~TestArticHelper() {}

         private:
            float mTurretAzimuth;
            float mTurretAzimuthRate;
            float mWeaponElevation;
            float mWeaponElevationRate;
            dtCore::RefPtr<osgSim::DOFTransform> mDOFTurret;
            dtCore::RefPtr<osgSim::DOFTransform> mDOFWeapon;
      };

      //////////////////////////////////////////////////////////////////////////
      const std::string TestArticHelper::DOF_TURRET("dof_turret_01");
      const std::string TestArticHelper::DOF_WEAPON("dof_gun_01");

      const std::string TestArticHelper::DOF_TURRET_METRIC_AZIMUTH("TurretAzimuth");
      const std::string TestArticHelper::DOF_WEAPON_METRIC_ELEVATION("WeaponElevation");
      const std::string TestArticHelper::DOF_TURRET_METRIC_AZIMUTH_RATE("TurretAzimuth"
         +SimCore::Components::ArticulationHelper::PARAM_NAME_SUFFIX_RATE);
      const std::string TestArticHelper::DOF_WEAPON_METRIC_ELEVATION_RATE("WeaponElevation"
         +SimCore::Components::ArticulationHelper::PARAM_NAME_SUFFIX_RATE);

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::NamedGroupParameter> TestArticHelper::BuildGroupProperty()
      {
         dtCore::RefPtr<dtDAL::NamedGroupParameter> articArrayProp 
            = new dtDAL::NamedGroupParameter( GetArticulationArrayPropertyName() );

         AddArticulatedParameter( *articArrayProp,
            SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH,
            DOF_TURRET_METRIC_AZIMUTH, 
            mTurretAzimuth, 0, 
            mTurretAzimuthRate, 0,
            DOF_TURRET );

         AddArticulatedParameter( *articArrayProp,
            SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION,
            DOF_WEAPON_METRIC_ELEVATION,
            mWeaponElevation, 0,
            mWeaponElevationRate, 0,
            DOF_WEAPON, DOF_TURRET );

         return articArrayProp;
      }

      //////////////////////////////////////////////////////////////////////////
      void TestArticHelper::UpdateDOFReferences( dtUtil::NodeCollector* nodeCollector )
      {
         if( nodeCollector == NULL )
         {
            mDOFTurret = NULL;
            mDOFWeapon = NULL;
         }
         else
         {
            mDOFTurret = nodeCollector->GetDOFTransform( DOF_TURRET );
            mDOFWeapon = nodeCollector->GetDOFTransform( DOF_WEAPON );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool TestArticHelper::HasDOF( osgSim::DOFTransform& dof ) const
      {
         return mDOFTurret.get() == &dof || mDOFWeapon.get() == &dof;
      }

      //////////////////////////////////////////////////////////////////////////
      bool TestArticHelper::HasDOFMetric( const osgSim::DOFTransform& dof, const std::string& metricName ) const
      {
         if( mDOFTurret.get() == &dof )
         {
            return SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH.GetName() == metricName
               || SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH.GetRelatedRateMetricName() == metricName;
         }
         else if( mDOFWeapon.get() == &dof )
         {
            return SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION.GetName() == metricName
               || SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION.GetRelatedRateMetricName() == metricName;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void TestArticHelper::HandleUpdatedDOF( const osgSim::DOFTransform& dof,
         const osg::Vec3& transChange, const osg::Vec3& hprChange )
      {

      }

      //////////////////////////////////////////////////////////////////////////
      const dtDAL::NamedGroupParameter* TestArticHelper::GetMetricParameter( 
         const dtDAL::NamedGroupParameter& articulatedParam,
         const std::string& metricParamName ) const
      {
         const dtDAL::NamedGroupParameter* metricParam
            = dynamic_cast<const dtDAL::NamedGroupParameter*>
            (articulatedParam.GetParameter( 
               SimCore::Components::ArticulationHelper::PARAM_NAME_PREFIX_ARTICULATED + metricParamName ));
         return metricParam;
      }

      //////////////////////////////////////////////////////////////////////////
      float TestArticHelper::GetMetricValue( const dtDAL::NamedGroupParameter& metricParam ) const
      {
         const dtDAL::NamedFloatParameter* valueParam = NULL;

         std::vector<const dtDAL::NamedParameter*> params;
         metricParam.GetParameters( params );

         unsigned limit = params.size();
         for( unsigned i = 0; i < limit; ++i )
         {
            valueParam = dynamic_cast<const dtDAL::NamedFloatParameter*> (params[i]);

            if( valueParam != NULL )
            {
               return valueParam->GetValue();
            }
         }

         return 0.0f;
      }

      //////////////////////////////////////////////////////////////////////////
      void TestArticHelper::GetMetricDOFNames( 
         const dtDAL::NamedGroupParameter& metricParam,
         std::string& outDOFName, std::string& outDOFParentName ) const
      {
         // Get the direct DOF name.
         const dtDAL::NamedStringParameter* dofNameParam
            = dynamic_cast<const dtDAL::NamedStringParameter*>
            (metricParam.GetParameter(SimCore::Components::ArticulationHelper::PARAM_NAME_DOF));

         outDOFName = dofNameParam != NULL ? dofNameParam->GetValue() : "";

         // Get the name of the DOF that parents the direct DOF.
         dofNameParam = dynamic_cast<const dtDAL::NamedStringParameter*>
            (metricParam.GetParameter(SimCore::Components::ArticulationHelper::PARAM_NAME_DOF_PARENT));

         outDOFParentName = dofNameParam != NULL ? dofNameParam->GetValue() : "";
      }

      //////////////////////////////////////////////////////////////////////////
      void TestArticHelper::SetTestMetricValue( const std::string& metricName, float value )
      {
         if( DOF_TURRET_METRIC_AZIMUTH == metricName )
         {
            mTurretAzimuth = value;
         }
         else if( DOF_TURRET_METRIC_AZIMUTH_RATE == metricName )
         {
            mTurretAzimuthRate = value;
         }
         else if( DOF_WEAPON_METRIC_ELEVATION == metricName )
         {
            mWeaponElevation = value;
         }
         else if( DOF_WEAPON_METRIC_ELEVATION_RATE == metricName )
         {
            mWeaponElevationRate = value;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      float TestArticHelper::GetTestMetricValue( const std::string& metricName ) const
      {
         if( DOF_TURRET_METRIC_AZIMUTH == metricName )
         {
            return mTurretAzimuth;
         }
         else if( DOF_TURRET_METRIC_AZIMUTH_RATE == metricName )
         {
            return mTurretAzimuthRate;
         }
         else if( DOF_WEAPON_METRIC_ELEVATION == metricName )
         {
            return mWeaponElevation;
         }
         else if( DOF_WEAPON_METRIC_ELEVATION_RATE == metricName )
         {
            return mWeaponElevationRate;
         }
         return 0.0f;
      }



      //////////////////////////////////////////////////////////////////////////
      // TESTS CODE
      //////////////////////////////////////////////////////////////////////////
      void ArticulationHelperTests::setUp()
      {
         try
         {
            mGM = new dtGame::GameManager( *GetGlobalApplication().GetScene() );

            mHelper = new TestArticHelper;
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ArticulationHelperTests::tearDown()
      {
         try
         {
            mGM->DeleteAllActors(true);
            mGM = NULL;
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ArticulationHelperTests::TestArticulationHelperProperties()
      {
         CPPUNIT_ASSERT( mHelper->GetArticulationArrayPropertyName() == "Articulated Parameters Array" );

         // Test Dirty
         CPPUNIT_ASSERT_MESSAGE( "ArticulationHelper dirty state should be FALSE by default, to force an update.",
            ! mHelper->IsDirty() );
         mHelper->SetDirty( true );
         CPPUNIT_ASSERT( mHelper->IsDirty() );

         // Compare the generated group parameter with the helper's values.
         dtCore::RefPtr<dtDAL::NamedGroupParameter> articProp = mHelper->BuildGroupProperty();
         SubTestArticArrayGroupProperty( articProp, *mHelper );
      }

      //////////////////////////////////////////////////////////////////////////
      void ArticulationHelperTests::SubTestArticArrayGroupProperty(
         const dtCore::RefPtr<dtDAL::NamedGroupParameter>& groupProperty,
         const TestArticHelper& helper )
      {
         CPPUNIT_ASSERT( groupProperty.valid() );

         // Declare test variables
         const float errorTolerance = 0.001f;
         float testValue = 0.0f;
         std::string dofName("");
         std::string dofParentName("");

         std::vector<const std::string*> metricNames;
         metricNames.push_back(&TestArticHelper::DOF_TURRET_METRIC_AZIMUTH);
         metricNames.push_back(&TestArticHelper::DOF_TURRET_METRIC_AZIMUTH_RATE);
         metricNames.push_back(&TestArticHelper::DOF_WEAPON_METRIC_ELEVATION);
         metricNames.push_back(&TestArticHelper::DOF_WEAPON_METRIC_ELEVATION_RATE);

         const unsigned limit = metricNames.size();

         // Test the number of metrics in the articulation array group property.
         CPPUNIT_ASSERT( groupProperty->GetParameterCount() == limit );

         // Test each metric parameter contained in the helper and articulation
         // group parameter.
         const std::string* curMetricName = NULL;
         const dtDAL::NamedGroupParameter* curMetricParam = NULL;
         for( unsigned i = 0; i < limit; ++i )
         {
            curMetricName = metricNames[i];

            // Get the current metric parameter
            curMetricParam = helper.GetMetricParameter( 
               *groupProperty, *curMetricName );
            CPPUNIT_ASSERT( curMetricParam != NULL );

            // Test metric value
            testValue = helper.GetMetricValue( *curMetricParam );
            CPPUNIT_ASSERT_DOUBLES_EQUAL(
               helper.GetTestMetricValue( *curMetricName ),
               testValue,
               errorTolerance );

            // Test the referenced DOF names
            helper.GetMetricDOFNames( *curMetricParam, dofName, dofParentName );

            if( *curMetricName == TestArticHelper::DOF_TURRET_METRIC_AZIMUTH
               || *curMetricName == TestArticHelper::DOF_TURRET_METRIC_AZIMUTH_RATE )
            {
               CPPUNIT_ASSERT( dofName == TestArticHelper::DOF_TURRET );
               CPPUNIT_ASSERT( dofParentName == SimCore::Components::ArticulationHelper::PARAM_NAME_DOF_ROOT );
            }
            else if( *curMetricName == TestArticHelper::DOF_WEAPON_METRIC_ELEVATION
               || *curMetricName == TestArticHelper::DOF_WEAPON_METRIC_ELEVATION_RATE )
            {
               CPPUNIT_ASSERT( dofName == TestArticHelper::DOF_WEAPON );
               CPPUNIT_ASSERT( dofParentName == TestArticHelper::DOF_TURRET );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ArticulationHelperTests::TestArticulationHelperUpating()
      {
         // TODO: test the articulation help on an entity.
      }

   }
}
