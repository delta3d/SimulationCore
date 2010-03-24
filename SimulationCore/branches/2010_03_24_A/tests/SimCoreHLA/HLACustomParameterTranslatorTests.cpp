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

               // the datastream will delete the buffer.
               dtUtil::DataStream ds(buffer, 4U, true);
               long timeMillis;
               ds.Read(timeMillis);

               CPPUNIT_ASSERT_EQUAL(43433250L, timeMillis);
               CPPUNIT_ASSERT_EQUAL(4U, unsigned(size));
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
