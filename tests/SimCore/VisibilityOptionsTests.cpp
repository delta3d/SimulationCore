/* -*-c++-*-
* Simulation Core - Visibility Tests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2005-2008, Alion Science and Technology Corporation
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
* @author David Guthrie
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtGame/gamemanager.h>
//#include <dtABC/application.h>
//#include <dtCore/globals.h>
//#include <dtCore/system.h>
//#include <dtCore/scene.h>
//#include <dtCore/deltawin.h>
//#include <dtUtil/fileutils.h>

#include <SimCore/VisibilityOptions.h>
#include <SimCore/Actors/BaseEntity.h>

#include <cstring>
#include <UnitTestMain.h>


using std::shared_ptr;

namespace SimCore
{
   class VisibilityOptionsTests : public CPPUNIT_NS::TestFixture
   {
      CPPUNIT_TEST_SUITE(VisibilityOptionsTests);

         CPPUNIT_TEST(TestBasicOptions);

      CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {

      }

      void tearDown()
      {

      }


      void TestBasicOptions()
      {
         const std::string ALL_OPTS_TRUE_MSG("All visibility options should be true by default but blips and tracks.");
         const std::string ALL_OPTS_FALSE_MSG("All visibility options should now be false.");
         std::shared_ptr<VisibilityOptions> visOpts = new VisibilityOptions;
         BasicVisibilityOptions basicOpts = visOpts->GetBasicOptions();
         //All basic options should be true by default.
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_TRUE_MSG, basicOpts.mDismountedInfantry);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_TRUE_MSG, basicOpts.mPlatforms);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_TRUE_MSG, !basicOpts.mSensorBlips);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_TRUE_MSG, !basicOpts.mTracks);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_TRUE_MSG, basicOpts.mBattlefieldGraphics);

         {
            const std::vector<SimCore::Actors::BaseEntityActorProxy::ForceEnum*>& forces =
               SimCore::Actors::BaseEntityActorProxy::ForceEnum::EnumerateType();
            for (size_t i = 0; i < forces.size(); ++i)
            {
               CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_TRUE_MSG, basicOpts.IsEnumVisible(*forces[i]));
            }

            const std::vector<SimCore::Actors::BaseEntityActorProxy::DomainEnum*>& domains =
               SimCore::Actors::BaseEntityActorProxy::DomainEnum::EnumerateType();
            for (size_t i = 0; i < domains.size(); ++i)
            {
               CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_TRUE_MSG, basicOpts.IsEnumVisible(*domains[i]));
            }
         }

         BasicVisibilityOptions basicOpts2;
         basicOpts2.SetAllFalse();

         visOpts->SetBasicOptions(basicOpts2);
         const BasicVisibilityOptions basicOpts3 = visOpts->GetBasicOptions();
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts3.mDismountedInfantry);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts3.mPlatforms);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts3.mSensorBlips);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts3.mTracks);
         CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts3.mBattlefieldGraphics);

         {
            const std::vector<SimCore::Actors::BaseEntityActorProxy::ForceEnum*>& forces =
               SimCore::Actors::BaseEntityActorProxy::ForceEnum::EnumerateType();
            for (size_t i = 0; i < forces.size(); ++i)
            {
               CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts2.IsEnumVisible(*forces[i]));
               CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts3.IsEnumVisible(*forces[i]));
            }

            const std::vector<SimCore::Actors::BaseEntityActorProxy::DomainEnum*>& domains =
               SimCore::Actors::BaseEntityActorProxy::DomainEnum::EnumerateType();
            for (size_t i = 0; i < domains.size(); ++i)
            {
               CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts2.IsEnumVisible(*domains[i]));
               CPPUNIT_ASSERT_MESSAGE(ALL_OPTS_FALSE_MSG, !basicOpts3.IsEnumVisible(*domains[i]));
            }
         }
      }

      VisibilityOptionsTests()
      {

      }
      ~VisibilityOptionsTests()
      {

      }

   private:

   };

   CPPUNIT_TEST_SUITE_REGISTRATION(VisibilityOptionsTests);
}
