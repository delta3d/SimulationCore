/* -*-c++-*-
* Simulation Core - StealthViewerDataTests (.h & .cpp) - Using 'The MIT License'
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
* @author Eddie Johnson
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <StealthViewer/Qt/StealthViewerData.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <QtGui/QApplication>

class StealthViewerDataTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(StealthViewerDataTests);


   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp();

      void tearDown();

      QApplication *mQApp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StealthViewerDataTests);

void StealthViewerDataTests::setUp()
{
   std::string dir = dtUtil::FileUtils::GetInstance().CurrentDirectory();

#if defined (_MSC_VER) && defined (_DEBUG)
   dir += "bin/StealthViewerDataTestsd.exe";
#elif defined (_MSC_VER)
   dir += "bin/StealthViewerDataTests.exe";
#else
   dir += "bin/StealthViewerDataTests";
#endif

   int numParams = 1;
   const char *exe = dir.c_str();

   mQApp = new QApplication(numParams, const_cast<char**>(&exe));
}

void StealthViewerDataTests::tearDown()
{
   delete mQApp;
   mQApp = NULL;
}
