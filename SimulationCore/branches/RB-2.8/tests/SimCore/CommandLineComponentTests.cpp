/* -*-c++-*-
* Simulation Core - CommandLineComponentTests (.h & .cpp) - Using 'The MIT License'
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
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <string>

#include <dtCore/project.h>
#include <dtCore/datatype.h>
#include <dtGame/gamemanager.h> 

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <dtGame/messagetype.h>
#include <dtGame/messagefactory.h>
#include <dtGame/basemessages.h>

#include <SimCore/CommandLineObject.h>
#include <dtCore/namedparameter.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

using namespace SimCore::Actors;

// note name wasnt changed but this is commandlineobject now.

class CommandLineComponentTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(CommandLineComponentTests);
   CPPUNIT_TEST(TestFunction);
   CPPUNIT_TEST_SUITE_END();

public:
   CommandLineComponentTests()
   {
   }
   ~CommandLineComponentTests()
   {
   }

   void setUp()
   {
      dtCore::System::GetInstance().Start();

      mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());

      dtCore::System::GetInstance().Step();

      mCommandLineObject = new SimCore::CommandLineObject();

      dtCore::System::GetInstance().Step();
   }

   void tearDown()
   {
      dtCore::System::GetInstance().Stop();

      mGM->DeleteAllActors(true);
      mGM = NULL;
   }

   void TestFunction();

private:
   dtCore::RefPtr<SimCore::CommandLineObject> mCommandLineObject;
   dtCore::RefPtr<dtGame::GameManager> mGM;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CommandLineComponentTests);

////////////////////////////////////////////////////////////////////////
void CommandLineComponentTests::TestFunction()
{
   dtCore::System::GetInstance().Step();
   CPPUNIT_ASSERT_MESSAGE("mCommandLineComponent not initialized", (mCommandLineObject != NULL));
   
   dtCore::RefPtr<dtCore::NamedStringParameter> pArAmEtEr = new dtCore::NamedStringParameter("SimpleName", "lollerskatez_dont_fall_down");
   mCommandLineObject->AddParameter(pArAmEtEr.get());

   CPPUNIT_ASSERT(mCommandLineObject->GetParameter("SimpleName") != NULL);
   CPPUNIT_ASSERT(mCommandLineObject->GetParameter("") == NULL);

   mCommandLineObject->ClearParametersList();

   CPPUNIT_ASSERT(mCommandLineObject->GetParameter("SimpleName") == NULL);

   dtCore::System::GetInstance().Step();
}
