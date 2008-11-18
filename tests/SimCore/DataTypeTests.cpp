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
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>
#include <osg/Endian>
#include <dtUtil/log.h>
#include <SimCore/FloatArray2D.h>
#include <UnitTestMain.h>



////////////////////////////////////////////////////////////////////////////////
// UNIT TESTS
////////////////////////////////////////////////////////////////////////////////
class DataTypeTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(DataTypeTests);

      CPPUNIT_TEST(TestFloatArray2D);

   CPPUNIT_TEST_SUITE_END();


   public:

      //////////////////////////////////////////////////////////////////////////
      template<class T_Data>
      void GetValue( const char* buffer, T_Data& outValue )
      {
         size_t dataSize = sizeof(T_Data);
         memcpy( &outValue, buffer, dataSize );

         if( osg::getCpuByteOrder == osg::LittleEndian )
         {
            osg::swapBytes( (char*)(&outValue), dataSize );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      template<class T_Data>
      void SetValue( const T_Data& value, char* buffer, size_t bufferSize )
      {
         size_t dataSize = sizeof(T_Data);
         if( osg::getCpuByteOrder() == osg::LittleEndian )
         {
            T_Data tmp = value;
            osg::swapBytes( (char*)(&tmp), dataSize );
            memcpy( buffer, &tmp, dataSize );
         }
         else
         {
            for( size_t valueByte = 0; valueByte < dataSize && valueByte < bufferSize; ++valueByte )
            {
               buffer[valueByte] = ((const char*)(&value))[valueByte];
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void setUp()
      {
      }
      
      //////////////////////////////////////////////////////////////////////////
      void tearDown()
      {
      }
      
      //////////////////////////////////////////////////////////////////////////
      void TestFloatArray2D()
      {
         try
         {
            dtCore::RefPtr<SimCore::FloatArray2D> arrayA = new SimCore::FloatArray2D;
            dtCore::RefPtr<SimCore::FloatArray2D> arrayB = new SimCore::FloatArray2D;

            int rowSize = 5;
            CPPUNIT_ASSERT( arrayA->GetRowSize() == 0 );
            CPPUNIT_ASSERT( arrayB->GetRowSize() == 0 );
            arrayA->SetRowSize( rowSize );
            CPPUNIT_ASSERT( arrayA->GetRowSize() == rowSize );
            CPPUNIT_ASSERT( arrayB->GetRowSize() == 0 );

            typedef std::vector<float> ArrayData;
            ArrayData& dataA = arrayA->GetData();
            ArrayData& dataB = arrayB->GetData();

            CPPUNIT_ASSERT( dataA.size() == 0 );
            CPPUNIT_ASSERT( dataB.size() == 0 );

            const size_t elementCount = size_t(rowSize * 3);
            dataA.reserve( elementCount );

            float curValue = -2.0f;
            size_t element = 0;
            for( ; element < elementCount; ++element )
            {
               dataA.push_back( curValue );
               curValue += 3.5;
            }

            CPPUNIT_ASSERT( dataA.size() == elementCount );
            CPPUNIT_ASSERT( dataB.size() == 0 );



            // Setup the test buffers.
            // NOTE: 15 is the constant representation of elementCount.
            char decodeBuffer[2+2+15*4]; // 2 unsigned shorts & 15 floats
            const size_t bufferSize = sizeof(decodeBuffer);
            char encodeBuffer[2+2+15*4];
            memset( decodeBuffer, 0, sizeof(decodeBuffer) );
            memset( encodeBuffer, 0, sizeof(encodeBuffer) );

            // Set the values for the decode buffer.
            // --- Set the number of columns.
            SetValue( (unsigned short)(rowSize), decodeBuffer, bufferSize );
            int bufferOffset = 0;
            bufferOffset += 2;
            // --- Set the number of rows.
            SetValue( (unsigned short)(int(elementCount)/rowSize), &decodeBuffer[bufferOffset], bufferSize-2 );
            bufferOffset += 2;

            // --- Set the array data.
            element = 0;
            for( ; element < elementCount; ++element, bufferOffset += 4 )
            {
               curValue = dataA[element];
               SetValue( curValue, &decodeBuffer[bufferOffset], bufferSize - bufferOffset );
            }

            // Test Encode
            arrayA->Encode( encodeBuffer );
            CPPUNIT_ASSERT( memcmp( decodeBuffer, encodeBuffer, bufferSize ) == 0 );
            
            // Test Decode
            arrayB->Decode( decodeBuffer );
            CPPUNIT_ASSERT( arrayB->GetRowSize() == arrayA->GetRowSize() );
            CPPUNIT_ASSERT( dataB.size() == dataA.size() );
            CPPUNIT_ASSERT( dataB.size() == elementCount );
            element = 0;
            for( ; element < elementCount; ++element )
            {
               std::ostringstream oss;
               oss << "B Element[" << element << "] should match the element in A at the same index";
               CPPUNIT_ASSERT_MESSAGE( oss.str(), dataA[element] == dataB[element] );
            }
         }
         catch( std::exception& e )
         {
            LOG_ERROR( e.what() );
         }
      }

   private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(DataTypeTests);
