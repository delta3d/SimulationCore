/* -*-c++-*-
* Simulation Core - ArticulationHelperTests (.h & .cpp) - Using 'The MIT License'
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
#include <osg/StateSet>
#include <dtGame/gameactorproxy.h>
#include <dtGame/gamemanager.h>
#include <SimCore/ActComps/CamoPaintStateActComp.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/Platform.h>
#include <UnitTestMain.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // TESTS OBJECT
      //////////////////////////////////////////////////////////////////////////
      class BodyPaintStateActCompTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(BodyPaintStateActCompTests);
         CPPUNIT_TEST(TestProperties);
         CPPUNIT_TEST(TestOnActor);
         CPPUNIT_TEST(TestCamoProperties);
         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            // Test Functions:
            void TestProperties();
            void TestOnActor();
            void TestCamoProperties();

            // Sub Tests
            void SubTestUniforms(osg::StateSet& stateSet);

         private:
            dtCore::RefPtr<dtGame::GameManager> mGM;
            dtCore::RefPtr<CamoPaintStateActComp> mActComp;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(BodyPaintStateActCompTests);

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActCompTests::setUp()
      {
         try
         {
            // Create the Game Manager.
            mGM = new dtGame::GameManager( *GetGlobalApplication().GetScene() );
            mGM->SetApplication(GetGlobalApplication());

            mActComp = new CamoPaintStateActComp();
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActCompTests::tearDown()
      {
         try
         {
            mGM->DeleteAllActors(true);
            mGM = NULL;

            mActComp = NULL;
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActCompTests::TestProperties()
      {
         try
         {
            const osg::Vec4 ZERO_V4;
            osg::Vec4 color1(1.0f, 0.0f, 0.0f, 1.0f);
            osg::Vec4 color2(0.0f, 1.0f, 0.0f, 1.0f);
            osg::Vec4 color3(0.0f, 0.0f, 1.0f, 1.0f);
            osg::Vec4 color4(0.0f, 0.0f, 0.0f, 1.0f);
            const BodyPaintStateActComp& constComp = *mActComp;

            // Test color properties
            CPPUNIT_ASSERT(constComp.GetPaintColor1() == ZERO_V4);
            mActComp->SetPaintColor1(color1);
            CPPUNIT_ASSERT(constComp.GetPaintColor1() == color1);

            CPPUNIT_ASSERT(constComp.GetPaintColor2() == ZERO_V4);
            mActComp->SetPaintColor2(color2);
            CPPUNIT_ASSERT(constComp.GetPaintColor2() == color2);

            CPPUNIT_ASSERT(constComp.GetPaintColor3() == ZERO_V4);
            mActComp->SetPaintColor3(color3);
            CPPUNIT_ASSERT(constComp.GetPaintColor3() == color3);

            CPPUNIT_ASSERT(constComp.GetPaintColor4() == ZERO_V4);
            mActComp->SetPaintColor4(color4);
            CPPUNIT_ASSERT(constComp.GetPaintColor4() == color4);

            // Test other vec properties
            // --- Paint State
            CPPUNIT_ASSERT(constComp.GetPaintState() == 0.0f);
            mActComp->SetPaintState(2.0f);
            CPPUNIT_ASSERT(constComp.GetPaintState() == 2.0f);

            // --- Diffuse Frame Scale
            CPPUNIT_ASSERT(constComp.GetDiffuseFrameScale() == 1.0f);
            mActComp->SetDiffuseFrameScale(-12.34f);
            CPPUNIT_ASSERT(constComp.GetDiffuseFrameScale() == -12.34f);

            // --- Overlay Frame Scale
            CPPUNIT_ASSERT(constComp.GetOverlayFrameScale() == 1.0f);
            mActComp->SetOverlayFrameScale(56.78f);
            CPPUNIT_ASSERT(constComp.GetOverlayFrameScale() == 56.78f);

            // --- Pattern Scaling
            osg::Vec4 temp(1.0f, 1.0f, 1.0f, 1.0f);
            CPPUNIT_ASSERT(constComp.GetPatternScale() == temp);
            temp.set(1.0f, 2.0f, 3.0f, 4.0f);
            mActComp->SetPatternScale(temp);
            CPPUNIT_ASSERT(constComp.GetPatternScale() == temp);

            CPPUNIT_ASSERT(constComp.GetProjectionDirection() != ZERO_V4);
            CPPUNIT_ASSERT(constComp.GetProjectionDirection() != temp);
            mActComp->SetProjectionDirection(temp);
            CPPUNIT_ASSERT(constComp.GetProjectionDirection() == temp);

            // Test file properties - ensure no crashing on file-not-found
            dtCore::ResourceDescriptor file("Test/Test.tga"); // Not a real file.
            CPPUNIT_ASSERT(constComp.GetPatternTexture().IsEmpty());
            mActComp->SetPatternTexture(file);
            CPPUNIT_ASSERT(constComp.GetPatternTexture() == file);

            CPPUNIT_ASSERT(constComp.GetReplacementDiffuseMaskTexture().IsEmpty());
            mActComp->SetReplacementDiffuseMaskTexture(file);
            CPPUNIT_ASSERT(constComp.GetReplacementDiffuseMaskTexture() == file);

            CPPUNIT_ASSERT(constComp.GetOverlayTexture().IsEmpty());
            mActComp->SetOverlayTexture(file);
            CPPUNIT_ASSERT(constComp.GetOverlayTexture() == file);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }
      
      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActCompTests::TestOnActor()
      {
         try
         {
            // Create a proxy.
            dtCore::RefPtr<dtGame::GameActorProxy> proxy;
            mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE, proxy);

            // Get the actor that was created.
            SimCore::Actors::Platform* actor = NULL;
            proxy->GetActor(actor);

            // Ensure the component exists
            mActComp = NULL;
            CamoPaintStateActComp* actComp = actor->GetComponent<CamoPaintStateActComp>();
            CPPUNIT_ASSERT(actComp != NULL);
            CPPUNIT_ASSERT(actComp->GetOwnerNode() == actor->GetOSGNode());

            // Add the actor to the scene.
            mGM->AddActor(*proxy, false, false);

            // Set some values to force uniform objects to be made.
            osg::Vec2 tmpV2;
            osg::Vec4 tmpV4;
            dtCore::ResourceDescriptor tmpFile("Fake/File.tga");
            actComp->SetPaintState(0);
            actComp->SetDiffuseFrameScale(1.0f);
            actComp->SetOverlayFrameScale(1.0f);
            actComp->SetPaintColor1(tmpV4);
            actComp->SetPaintColor2(tmpV4);
            actComp->SetPaintColor3(tmpV4);
            actComp->SetPaintColor4(tmpV4);
            actComp->SetPatternScale(tmpV4);
            actComp->SetProjectionDirection(tmpV4);
            actComp->SetReplacementDiffuseMaskTexture(tmpFile);
            actComp->SetPatternTexture(tmpFile);
            actComp->SetOverlayTexture(tmpFile);

            // Ensure that the actor component has added uniforms to the actor's state set.
            osg::StateSet* ss = actor->GetOSGNode()->getStateSet();
            CPPUNIT_ASSERT(actComp->GetStateSet() != NULL);
            CPPUNIT_ASSERT(ss != NULL);
            CPPUNIT_ASSERT(ss == actComp->GetStateSet());
            SubTestUniforms(*ss);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActCompTests::SubTestUniforms(osg::StateSet& stateSet)
      {
         typedef BodyPaintStateActComp ClassName;
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_FRAME_OFFSET_AND_SCALES) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_PAINT_COLOR_1) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_PAINT_COLOR_2) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_PAINT_COLOR_3) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_PAINT_COLOR_4) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_PATTERN_SCALE) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_PROJECTION_DIRECTION) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_REPLACEMENT_DIFFUSE_MASK_TEXTURE) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_PATTERN_TEXTURE) != NULL);
         CPPUNIT_ASSERT(stateSet.getUniform(ClassName::UNIFORM_OVERLAY_TEXTURE) != NULL);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActCompTests::TestCamoProperties()
      {
         try
         {
            const CamoPaintStateActComp& constComp = *mActComp;

            const osg::Vec4 VEC_ZEROS;
            osg::Vec4 testVec(-12.34f, 56.78f, -9.1011f, 12.13f);

            CPPUNIT_ASSERT(constComp.GetConcealMeshDims() == VEC_ZEROS);
            mActComp->SetConcealMeshDims(testVec);
            CPPUNIT_ASSERT(constComp.GetConcealMeshDims() == testVec);

            CPPUNIT_ASSERT(constComp.GetCamoId() == 0);
            mActComp->SetCamoId(3);
            CPPUNIT_ASSERT(constComp.GetCamoId() == 3);

            CPPUNIT_ASSERT( ! constComp.GetConcealedState());
            mActComp->SetConcealedState(true);
            CPPUNIT_ASSERT(constComp.GetConcealedState());

            CPPUNIT_ASSERT(constComp.GetConcealShaderGroup().empty());
            mActComp->SetConcealShaderGroup("test");
            CPPUNIT_ASSERT(constComp.GetConcealShaderGroup() == "test");

            dtCore::ResourceDescriptor testFile("Fake.file");
            CPPUNIT_ASSERT(constComp.GetConcealMesh().IsEmpty());
            mActComp->SetConcealMesh(testFile);
            CPPUNIT_ASSERT(constComp.GetConcealMesh() == testFile);

            dtCore::RefPtr<osg::Group> testNode = new osg::Group;
            CPPUNIT_ASSERT(constComp.GetParentNode() == NULL);
            mActComp->SetParentNode(testNode.get());
            CPPUNIT_ASSERT(constComp.GetParentNode() == testNode.get());

            CPPUNIT_ASSERT(constComp.GetHiderNode() == NULL);
            mActComp->SetHiderNode(testNode.get());
            CPPUNIT_ASSERT(constComp.GetHiderNode() == testNode.get());
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

   }
}
