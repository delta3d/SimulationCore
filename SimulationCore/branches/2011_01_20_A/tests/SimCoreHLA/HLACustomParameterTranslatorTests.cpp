/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008, Alion Science and Technology
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
 * @author David Guthrie
 */
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <vector>
#include <string>

#include <dtUtil/macros.h>
#include <dtUtil/coordinates.h>
#include <dtUtil/datastream.h>
#include <dtDAL/datatype.h>

#include <dtHLAGM/objecttoactor.h>
#include <dtHLAGM/interactiontomessage.h>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/attributetoproperty.h>
#include <dtHLAGM/parametertoparameter.h>
#include <dtHLAGM/onetoonemapping.h>
#include <dtHLAGM/distypes.h>

#include <SimCore/HLA/HLACustomParameterTranslator.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace HLA
   {
      class HLACustomParameterTranslatorTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(HLACustomParameterTranslatorTests);

         CPPUNIT_TEST(TestTimeConversion);
         CPPUNIT_TEST(TestVec3Conversion);

         CPPUNIT_TEST_SUITE_END();

      public:
         void setUp()
         {
            std::string logName("HLACustomParameterTranslatorTests");
            mLogger = &dtUtil::Log::GetInstance(logName);
            mTranslator = new HLACustomParameterTranslator;
         }

         void tearDown()
         {
            mTranslator = NULL;
         }

         void TestTimeConversion()
         {
            dtHLAGM::OneToManyMapping otm("teaTime", HLACustomAttributeType::MILLISECOND_TIME_TYPE, false ,false);
            dtHLAGM::OneToManyMapping::ParameterDefinition pd("doubleTime", dtDAL::DataType::DOUBLE, "", false);
            otm.GetParameterDefinitions().push_back(pd);

            dtCore::RefPtr<dtGame::DoubleMessageParameter> timeParam = new dtGame::DoubleMessageParameter("doubleTime");

            char* buffer = new char[4];
            try
            {
               std::vector<dtCore::RefPtr<dtGame::MessageParameter> > parameters;
               parameters.push_back(timeParam);

               // the datastream will delete the buffer.
               dtUtil::DataStream ds(buffer, 4U, true);
               long timeMillis = 3002201;
               ds.Write(timeMillis);

               mTranslator->MapToMessageParameters(buffer, 4U, parameters, otm);

               CPPUNIT_ASSERT_DOUBLES_EQUAL(3002.201, timeParam->GetValue(), 0.001);
            }
            catch (const dtUtil::Exception& ex)
            {
               CPPUNIT_FAIL(ex.What());
            }

            buffer = new char[4];
            try
            {
               timeParam->SetValue(43433.25);
               std::vector<dtCore::RefPtr<const dtGame::MessageParameter> > parameters;
               parameters.push_back(timeParam);

               // size must be passed in by reference
               size_t size = 4U;
               mTranslator->MapFromMessageParameters(buffer, size, parameters, otm);
               CPPUNIT_ASSERT_EQUAL(4U, unsigned(size));

               // the datastream will delete the buffer.
               dtUtil::DataStream ds(buffer, 4U, true);
               long timeMillis;
               ds.Read(timeMillis);

               CPPUNIT_ASSERT_EQUAL(43433250L, timeMillis);
            }
            catch (const dtUtil::Exception& ex)
            {
               CPPUNIT_FAIL(ex.What());
            }
         }

         void TestVec3Conversion()
         {
            dtHLAGM::OneToManyMapping otmD("3Doubles", HLACustomAttributeType::VEC3D_TYPE, false ,false);
            dtHLAGM::OneToManyMapping otmF("3Floats", HLACustomAttributeType::VEC3F_TYPE, false ,false);
            dtHLAGM::OneToManyMapping::ParameterDefinition pdV3D("testVec3Double", dtDAL::DataType::VEC3D, "", false);
            dtHLAGM::OneToManyMapping::ParameterDefinition pdV3F("testVec3Float", dtDAL::DataType::VEC3F, "", false);
            otmD.GetParameterDefinitions().push_back(pdV3D);
            otmF.GetParameterDefinitions().push_back(pdV3F);

            dtCore::RefPtr<dtGame::Vec3dMessageParameter> vecDoubleParam = new dtGame::Vec3dMessageParameter("testVec3Double");
            dtCore::RefPtr<dtGame::Vec3fMessageParameter> vecFloatParam = new dtGame::Vec3fMessageParameter("testVec3Float");

            double epsilon = 0.001;

            const size_t D_BUFFER_SIZE = sizeof(double) * 3;
            const size_t F_BUFFER_SIZE = sizeof(float) * 3;
            char* bufferD = new char[D_BUFFER_SIZE];
            char* bufferF = new char[F_BUFFER_SIZE];
            osg::Vec3d testValueD(1234.56789, 9876.54321, -0.123456789);
            osg::Vec3f testValueF(10.11f, -121.314f, 1516.17f);
            try
            {
               std::vector<dtCore::RefPtr<dtGame::MessageParameter> > parametersD;
               std::vector<dtCore::RefPtr<dtGame::MessageParameter> > parametersF;
               parametersD.push_back(vecDoubleParam);
               parametersF.push_back(vecFloatParam);

               // the datastream will delete the buffer.
               dtUtil::DataStream dsD(bufferD, D_BUFFER_SIZE, true);
               dtUtil::DataStream dsF(bufferF, F_BUFFER_SIZE, true);
               dsD.Write(testValueD);
               dsF.Write(testValueF);

               mTranslator->MapToMessageParameters(bufferD, D_BUFFER_SIZE, parametersD, otmD);
               mTranslator->MapToMessageParameters(bufferF, F_BUFFER_SIZE, parametersF, otmF);

               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueD.x(), vecDoubleParam->GetValue().x(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueD.y(), vecDoubleParam->GetValue().y(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueD.z(), vecDoubleParam->GetValue().z(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueF.x(), vecFloatParam->GetValue().x(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueF.y(), vecFloatParam->GetValue().y(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueF.z(), vecFloatParam->GetValue().z(), epsilon);
            }
            catch (const dtUtil::Exception& ex)
            {
               CPPUNIT_FAIL(ex.What());
            }

            bufferD = new char[D_BUFFER_SIZE];
            bufferF = new char[F_BUFFER_SIZE];
            osg::Vec3d testValueD2(9.8, -76.54, 321.1011);
            osg::Vec3f testValueF2(-12.34f, 876.5f, 1011.9f);
            try
            {
               vecDoubleParam->SetValue(testValueD2);
               vecFloatParam->SetValue(testValueF2);
               std::vector<dtCore::RefPtr<const dtGame::MessageParameter> > parametersD;
               std::vector<dtCore::RefPtr<const dtGame::MessageParameter> > parametersF;
               parametersD.push_back(vecDoubleParam);
               parametersF.push_back(vecFloatParam);

               // Buffer size must be passed in by reference
               size_t bufferSize = D_BUFFER_SIZE;
               mTranslator->MapFromMessageParameters(bufferD, bufferSize, parametersD, otmD);
               // Verify that the max size equals the expected size.
               CPPUNIT_ASSERT_EQUAL(D_BUFFER_SIZE, bufferSize);
               // The datastream will delete the buffer.
               dtUtil::DataStream dsD(bufferD, bufferSize, true);
               mTranslator->MapFromMessageParameters(bufferD, bufferSize, parametersD, otmD);

               bufferSize = F_BUFFER_SIZE;
               mTranslator->MapFromMessageParameters(bufferF, bufferSize, parametersF, otmF);
               // Verify that the max size equals the expected size.
               CPPUNIT_ASSERT_EQUAL(F_BUFFER_SIZE, bufferSize);
               // The datastream will delete the buffer.
               dtUtil::DataStream dsF(bufferF, bufferSize, true);
               mTranslator->MapFromMessageParameters(bufferF, bufferSize, parametersF, otmF);

               // Read from the data streams
               osg::Vec3d resultD;
               osg::Vec3f resultF;
               dsD.Read(resultD);
               dsF.Read(resultF);

               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueD2.x(), resultD.x(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueD2.y(), resultD.y(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueD2.z(), resultD.z(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueF2.x(), resultF.x(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueF2.y(), resultF.y(), epsilon);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(testValueF2.z(), resultF.z(), epsilon);
            }
            catch (const dtUtil::Exception& ex)
            {
               CPPUNIT_FAIL(ex.What());
            }
         }

      private:
         dtUtil::Log* mLogger;
         RefPtr<HLACustomParameterTranslator> mTranslator;
      };

      // Registers the fixture into the 'registry'
      CPPUNIT_TEST_SUITE_REGISTRATION(HLACustomParameterTranslatorTests);
   }
}
