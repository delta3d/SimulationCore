/* -*-c++-*-
 * SimulationCore
 * Copyright 2007-2008, Alion Science and Technology
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
 * david
 */

#include <prefix/SimCorePrefix.h>
/* -*-c++-*-
 * Simulation Core - ToolTests (.h & .cpp) - Using 'The MIT License'
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
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/MessageType.h>
#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtDAL/resourceactorproperty.h>
#include <dtDAL/project.h>
#include <dtGame/gamemanager.h>

#include <UnitTestMain.h>
#include <dtGame/testcomponent.h>

using std::shared_ptr;

namespace SimCore
{
   class TerrainActorTests : public CPPUNIT_NS::TestFixture
   {
      CPPUNIT_TEST_SUITE(TerrainActorTests);

         CPPUNIT_TEST(TestLoad);
         CPPUNIT_TEST(TestLoadInStage);

      CPPUNIT_TEST_SUITE_END();

   public:

      TerrainActorTests()
      {
      }

      ~TerrainActorTests()
      {
      }

      void setUp()
      {
         try
         {
            dtCore::Scene* scene = new dtCore::Scene();
            mGameManager = new dtGame::GameManager(*scene);
            mGameManager->SetApplication(GetGlobalApplication());

            dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
            dtCore::System::GetInstance().Start();
         }
         catch (const dtUtil::Exception& e)
         {
            CPPUNIT_FAIL(e.ToString());
         }
      }

      void tearDown()
      {
         try
         {
            dtDAL::Project::GetInstance().SetEditMode(false);

            dtCore::System::GetInstance().SetPause(false);
            dtCore::System::GetInstance().Stop();

            if (mGameManager.valid())
            {
               mGameManager->DeleteAllActors();
               mGameManager->Shutdown();
               mGameManager = nullptr;
            }
         }
         catch (const dtUtil::Exception& e)
         {
            CPPUNIT_FAIL(e.ToString());
         }
      }

      void TestLoadInStage()
      {
         dtDAL::Project::GetInstance().SetEditMode(true);

         std::shared_ptr<SimCore::Actors::TerrainActorProxy> terrain;
         SimCore::Actors::TerrainActor* terrainDrawable = nullptr;

         mGameManager->CreateActor(*SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, terrain);
         CPPUNIT_ASSERT_MESSAGE("Could not create a terrain actor", terrain.valid());
         terrain->GetActor(terrainDrawable);

         dtDAL::ResourceActorProperty* rap = nullptr;
         terrain->GetProperty("TerrainMesh", rap);
         CPPUNIT_ASSERT(rap != nullptr);

         rap->SetValue(dtDAL::ResourceDescriptor("StaticMeshes:Gun.ive"));

         // Terrain should already be loaded.
         CPPUNIT_ASSERT(!terrainDrawable->CheckForTerrainLoaded());
         mGameManager->GetScene().AddChild(terrainDrawable);
         // Have to remove the child or I get a dangling reference to the actor when the test end.
         mGameManager->GetScene().RemoveChild(terrainDrawable);
         CPPUNIT_ASSERT(terrainDrawable->CheckForTerrainLoaded());
      }

      void TestLoad()
      {
         std::shared_ptr<dtGame::TestComponent> tc = new dtGame::TestComponent;
         mGameManager->AddComponent(*tc, dtGame::GameManager::ComponentPriority::HIGHEST);

         std::shared_ptr<SimCore::Actors::TerrainActorProxy> terrain;
         SimCore::Actors::TerrainActor* terrainDrawable = nullptr;

         mGameManager->CreateActor(*SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, terrain);
         CPPUNIT_ASSERT_MESSAGE("Could not create a terrain actor", terrain.valid());
         terrain->GetActor(terrainDrawable);

         dtDAL::ResourceActorProperty* rap = nullptr;
         terrain->GetProperty("TerrainMesh", rap);
         CPPUNIT_ASSERT(rap != nullptr);

         // Load something really tiny.
         rap->SetValue(dtDAL::ResourceDescriptor("StaticMeshes:Gun.ive"));
         CPPUNIT_ASSERT(!terrainDrawable->CheckForTerrainLoaded());

         mGameManager->AddActor(*terrain, false, false);

         // Can't test this because you just never know if it's done yet.
         //CPPUNIT_ASSERT(!terrainDrawable->CheckForTerrainLoaded());
         CPPUNIT_ASSERT_EQUAL((const dtGame::Message*)(nullptr), tc->FindProcessMessageOfType(SimCore::MessageType::INFO_TERRAIN_LOADED).get());

         unsigned i = 0;
         while (tc->FindProcessMessageOfType(SimCore::MessageType::INFO_TERRAIN_LOADED) == nullptr && i < 100)
         {
            // sleep while it loads in the background.
            dtCore::AppSleep(50);
            dtCore::System::GetInstance().Step();
         }

         CPPUNIT_ASSERT(tc->FindProcessMessageOfType(SimCore::MessageType::INFO_TERRAIN_LOADED) != nullptr);
         CPPUNIT_ASSERT(terrainDrawable->CheckForTerrainLoaded());
      }

   private:
      std::shared_ptr<dtGame::GameManager> mGameManager;
   };

   CPPUNIT_TEST_SUITE_REGISTRATION(TerrainActorTests);
}

