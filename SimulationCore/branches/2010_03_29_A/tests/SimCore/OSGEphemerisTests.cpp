/* -*-c++-*-
* Simulation Core - OSGEphemerisTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
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
* David Guthrie
*/

#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <osgEphemeris/DateTime.h>
#include <dtUtil/datetime.h>

#include <ctime>
//
//#include <dtABC/application.h>
//
//#include <UnitTestMain.h>


//////////////////////////////////////////////////////////////
// UNIT TESTS
//////////////////////////////////////////////////////////////
class OSGEphemerisTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(OSGEphemerisTests);

      CPPUNIT_TEST(TestDateTime);

   CPPUNIT_TEST_SUITE_END();


   public:

      //////////////////////////////////////////////////////////////
      void setUp()
      {
      }

      //////////////////////////////////////////////////////////////
      void tearDown()
      {
      }

      void TestDateTime()
      {
         osgEphemeris::DateTime osgEphDT;

         dtUtil::DateTime dtUtilDT;
         dtUtilDT.SetToLocalTime();

         static const std::string INIT_MESSAGE("The osg ephemeris time should be initialized to localtime to prevent memory corruption");
         CPPUNIT_ASSERT_EQUAL_MESSAGE(INIT_MESSAGE,
                  int(dtUtilDT.GetMonth()), osgEphDT.getMonth());
         CPPUNIT_ASSERT_EQUAL_MESSAGE(INIT_MESSAGE,
                  int(dtUtilDT.GetDay()), osgEphDT.getDayOfMonth());
         CPPUNIT_ASSERT_EQUAL_MESSAGE(INIT_MESSAGE,
                  int(dtUtilDT.GetYear()), osgEphDT.getYear());

         osgEphemeris::DateTime gmtTime = osgEphDT.getGMT();

         static const std::string MOD_MESSAGE("The osg ephemeris time should return the same time as GMT, which is a Simcore mod");
         CPPUNIT_ASSERT_EQUAL_MESSAGE(MOD_MESSAGE,
                  gmtTime.getHour(), osgEphDT.getHour());
         CPPUNIT_ASSERT_EQUAL_MESSAGE(MOD_MESSAGE,
                  gmtTime.getMinute(), osgEphDT.getMinute());
         CPPUNIT_ASSERT_EQUAL_MESSAGE(MOD_MESSAGE,
                  gmtTime.getDayOfMonth(), osgEphDT.getDayOfMonth());
      }

   private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(OSGEphemerisTests);


