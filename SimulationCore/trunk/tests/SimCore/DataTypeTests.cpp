/* -*-c++-*-
 * Simulation Core - ClampedMotionModelTests (.h & .cpp) - Using 'The MIT License'
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
#include <osg/Endian>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>
#include <SimCore/Array2DParser.h>
#include <UnitTestMain.h>



////////////////////////////////////////////////////////////////////////////////
// UNIT TESTS
////////////////////////////////////////////////////////////////////////////////
class DataTypeTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(DataTypeTests);

      CPPUNIT_TEST(TestArray2DParser);

   CPPUNIT_TEST_SUITE_END();


   public:

      //////////////////////////////////////////////////////////////////////////
      void setUp()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void tearDown()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void TestArray2DParser()
      {
         try
         {
            SimCore::Array2DParser<short> arrayA;
            SimCore::Array2DParser<short> arrayB;

            CPPUNIT_ASSERT_EQUAL(size_t(0), arrayA.GetColumns());
            arrayA.SetColumns(23);
            CPPUNIT_ASSERT_EQUAL(size_t(23), arrayA.GetColumns());
            arrayA.SetValue(37, 2, 19);
            CPPUNIT_ASSERT_EQUAL(short(37), arrayA.GetValue(2, 19));
            CPPUNIT_ASSERT_EQUAL(size_t(3), arrayA.GetRows());
            CPPUNIT_ASSERT_EQUAL(size_t(3 * 23), arrayA.GetData().size());
            //should do nothing since its past the column size.
            arrayA.SetValue(37, 1, 96);
            CPPUNIT_ASSERT_EQUAL(size_t(3 * 23), arrayA.GetData().size());

            arrayA.ClearData();
            arrayA.SetColumns(2);
            arrayA.SetValue(3, 0, 0);
            arrayA.SetValue(4, 0, 1);
            arrayA.SetValue(5, 1, 0);
            arrayA.SetValue(6, 1, 1);

            char encodeTo[100];
            size_t actualSize = arrayA.Encode(encodeTo, sizeof(encodeTo));
            CPPUNIT_ASSERT_EQUAL(arrayA.GetEncodedSize(), actualSize);
            CPPUNIT_ASSERT_EQUAL((2 * sizeof(short)) + (4 * sizeof(short)), actualSize);

            arrayB.Decode(encodeTo, actualSize);
            CPPUNIT_ASSERT_EQUAL(arrayA.GetRows(), arrayB.GetRows());
            CPPUNIT_ASSERT_EQUAL(arrayA.GetColumns(), arrayB.GetColumns());
            CPPUNIT_ASSERT_EQUAL(arrayA.GetData().size(), arrayB.GetData().size());

            for (unsigned i = 0; i < arrayA.GetData().size(); ++i)
            {
               CPPUNIT_ASSERT_EQUAL(arrayA.GetData()[i], arrayB.GetData()[i]);
            }
         }
         catch(const dtUtil::Exception& e)
         {
            CPPUNIT_FAIL(e.ToString());
         }
      }
   private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(DataTypeTests);
