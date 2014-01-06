/* -*-c++-*-
* Simulation Core - BattlefieldGraphicsActorTests.cpp - Using 'The MIT License'
 * Copyright 2011, Alion Science and Technology
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
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 * 
 * David Guthrie
 */
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <dtGame/gamemanager.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/BattlefieldGraphicsActor.h>
#include <SimCore/VisibilityOptions.h>
#include <UnitTestMain.h>

#include <dtUtil/configproperties.h>
#include <dtUtil/mathdefines.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // TESTS OBJECT
      //////////////////////////////////////////////////////////////////////////
      class BattlefieldGraphicsActorTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(BattlefieldGraphicsActorTests);
         CPPUNIT_TEST(TestEnumColor);
         CPPUNIT_TEST(TestActorProperties);
         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            // Test Methods
            void TestEnumColor();
            void TestActorProperties();
            void TestVisibilityOptions();

         private:
            std::shared_ptr<dtGame::GameManager> mGM;
            std::shared_ptr<BattlefieldGraphicsActorProxy> mActor;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(BattlefieldGraphicsActorTests);

      //////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorTests::setUp()
      {
         try
         {
            // Create the Game Manager.
            mGM = new dtGame::GameManager( *GetGlobalApplication().GetScene() );
            mGM->SetApplication(GetGlobalApplication());

            // Create a proxy.
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::BATTLEFIELD_GRAPHICS_ACTOR_TYPE, mActor);

         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorTests::tearDown()
      {
         try
         {
            mActor = nullptr;

            mGM->DeleteAllActors(true);
            mGM = nullptr;
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorTests::TestEnumColor()
      {
         dtUtil::ConfigProperties& confProp = mGM->GetConfiguration();

         BattlefieldGraphicsTypeEnum::EnumerateListType::const_iterator i, iend;
         i = BattlefieldGraphicsTypeEnum::EnumerateType().begin();
         iend = BattlefieldGraphicsTypeEnum::EnumerateType().end();

         // Declare outside the loop to prevent reallocating over and over.
         std::string confStr;

         for (; i != iend; ++i)
         {
            BattlefieldGraphicsTypeEnum& curEnum = **i;

            confStr = BattlefieldGraphicsTypeEnum::CONFIG_PREFIX + curEnum.GetName();

            std::string confVal = confProp.GetConfigPropertyValue(confStr, "");
            CPPUNIT_ASSERT(confVal.empty());

            CPPUNIT_ASSERT_EQUAL(curEnum.GetDefaultColor(), curEnum.GetColor(confProp));

            osg::Vec3 newColor(1.0f, 0.986f, 0.317f);
            confProp.SetConfigPropertyValue(confStr, "1.0 0.986 0.317");

            CPPUNIT_ASSERT(dtUtil::Equivalent(newColor, curEnum.GetColor(confProp), 0.001f));
            confProp.RemoveConfigPropertyValue(confStr);
         }


      }

      //////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorTests::TestActorProperties()
      {
         try
         {
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      void BattlefieldGraphicsActorTests::TestVisibilityOptions()
      {
         IGActor* drawable = nullptr;
         mActor->GetDrawable(drawable);
         CPPUNIT_ASSERT(drawable != nullptr);
         std::shared_ptr<VisibilityOptions> vo = new VisibilityOptions();

         BasicVisibilityOptions bvo;
         bvo.SetAllFalse();
         bvo.mBattlefieldGraphics = true;
         vo->SetBasicOptions(bvo);
         CPPUNIT_ASSERT(drawable->ShouldBeVisible(*vo));
         bvo.SetAllTrue();
         bvo.mBattlefieldGraphics = false;
         vo->SetBasicOptions(bvo);
         CPPUNIT_ASSERT(!drawable->ShouldBeVisible(*vo));
      }
   }
}
