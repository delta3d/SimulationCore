/* -*-c++-*-
* Simulation Core - Using 'The MIT License'
* Copyright (C) 2010, Alion Science and Technology Corporation
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
#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <SimCore/ActComps/TrailerHitchActComp.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/FourWheelVehicleActor.h>
#include <UnitTestMain.h>
#include <dtUtil/mathdefines.h>

#include <dtPhysics/palphysicsworld.h>
#include <dtPhysics/physicscomponent.h>

using std::shared_ptr;

namespace SimCore
{

class TrailerHitchActCompTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(TrailerHitchActCompTests);

   CPPUNIT_TEST(TestGetSet);
   CPPUNIT_TEST(TestAddLocal);
   CPPUNIT_TEST(TestAddRemote);
   CPPUNIT_TEST(TestAddRemoteWithTractorControl);
   CPPUNIT_TEST(TestInvalidTrailerID);
   CPPUNIT_TEST(TestCascadingDeletes);
   CPPUNIT_TEST(TestCascadingDeletesOff);
   CPPUNIT_TEST(TestRotateTrailer);

   CPPUNIT_TEST_SUITE_END();

public:

   TrailerHitchActCompTests()
   {
   }

   ~TrailerHitchActCompTests()
   {
   }

   void setUp()
   {
      // Initialize CEGUI
      try
      {
         dtCore::System::GetInstance().Start();
         dtABC::Application& app = GetGlobalApplication();

         mGM = new dtGame::GameManager(*app.GetScene());
         mGM->SetApplication( app );

         dtCore::System::GetInstance().Step();
      }
      catch(const dtUtil::Exception& e)
      {
         CPPUNIT_FAIL(e.ToString());
      }

      try
      {
         std::shared_ptr<dtPhysics::PhysicsWorld> physicsWorld = new dtPhysics::PhysicsWorld(GetGlobalApplication());
         physicsWorld->Init();
         mGM->AddComponent(*new dtPhysics::PhysicsComponent(*physicsWorld, false),
                  dtGame::GameManager::ComponentPriority::NORMAL);
         mGM->AddComponent(*new dtGame::DeadReckoningComponent(),
                  dtGame::GameManager::ComponentPriority::NORMAL);

         mGM->CreateActor(*Actors::EntityActorRegistry::FOUR_WHEEL_VEHICLE_ACTOR_TYPE, mVehicle);
         mVehicle->SetNonDamagedResource(dtDAL::ResourceDescriptor("StaticMeshes:NetDemo:Vehicles:Truck.ive"));
         mGM->CreateActor(*Actors::EntityActorRegistry::FOUR_WHEEL_VEHICLE_ACTOR_TYPE, mTrailer);
         mTrailer->SetNonDamagedResource(dtDAL::ResourceDescriptor("StaticMeshes:NetDemo:Vehicles:Truck.ive"));
      }
      catch (const dtUtil::Exception& ex)
      {
         CPPUNIT_FAIL(ex.ToString());
      }
   }

   void tearDown()
   {
      dtCore::System::GetInstance().Stop();
      mVehicle = nullptr;
      mTrailer = nullptr;
      if (mGM != nullptr)
      {
         mGM->Shutdown();
      }
      mGM = nullptr;
   }

   void TestGetSet()
   {
      std::shared_ptr<SimCore::ActComps::TrailerHitchActComp> trailerAC = new SimCore::ActComps::TrailerHitchActComp();

      CPPUNIT_ASSERT(trailerAC->GetCascadeDeletes());

      CPPUNIT_ASSERT_DOUBLES_EQUAL(60.0f, trailerAC->GetRotationMaxYaw(), 0.01f);

      CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0f, trailerAC->GetRotationMaxCone(), 0.01f);

      CPPUNIT_ASSERT_EQUAL(SimCore::ActComps::HitchTypeEnum::HITCH_TYPE_5TH_WHEEL, trailerAC->GetHitchType());

      CPPUNIT_ASSERT_EQUAL(SimCore::ActComps::TrailerHitchActComp::REAR_HITCH_DOF_NAME_DEFAULT.Get(), trailerAC->GetHitchNodeNameTractor());

      CPPUNIT_ASSERT_EQUAL(SimCore::ActComps::TrailerHitchActComp::FRONT_HITCH_DOF_NAME_DEFAULT.Get(), trailerAC->GetHitchNodeNameTrailer());

      CPPUNIT_ASSERT_EQUAL(dtCore::UniqueId(""), trailerAC->GetTrailerActorId());

      CPPUNIT_ASSERT(dtUtil::Equivalent(osg::Vec3(), trailerAC->GetCurrentHitchRotHPR(), osg::Vec3::value_type(0.01)));

      CPPUNIT_ASSERT_EQUAL(true, trailerAC->GetCascadeDeletes());

      CPPUNIT_ASSERT_EQUAL(false, trailerAC->GetUseCurrentHitchRotToMoveTrailerWhenRemote());
   }

   void TestAddLocal()
   {
      TestAddLocalOrRemote(false);
   }

   void TestAddRemote()
   {
      TestAddLocalOrRemote(true);
   }

   void TestAddRemoteWithTractorControl()
   {
      TestAddLocalOrRemote(true, true);
   }

   void TestAddLocalOrRemote(bool remote, bool enableRemoteMovement = false)
   {
      dtCore::Transform xform, xformTrailer;
      dtCore::Transformable* tx = nullptr;
      mVehicle->GetDrawable(tx);

      /// x and z are negative and y not 0 so 0,0,0 won't pass translation checks.
      xform.SetTranslation(-37.1f, 91.2f, -77.3f);
      xform.SetRotation(90.0f, -0.1f, -0.2f);
      tx->SetTransform(xform);

      dtGame::DeadReckoningHelper* drHelper = nullptr;
      mVehicle->GetComponent(drHelper);
      drHelper->SetLastKnownTranslation(xform.GetTranslation());
      drHelper->SetLastKnownRotation(xform.GetRotation());

      std::shared_ptr<SimCore::ActComps::TrailerHitchActComp> trailerAC = new SimCore::ActComps::TrailerHitchActComp();

      mVehicle->AddComponent(*trailerAC);

      trailerAC->SetTrailerActorId(mTrailer->GetId());
      trailerAC->SetUseCurrentHitchRotToMoveTrailerWhenRemote(enableRemoteMovement);

      CPPUNIT_ASSERT(!trailerAC->GetAttached());

      mGM->AddActor(*mTrailer, remote, false);

      CPPUNIT_ASSERT(trailerAC->LookupTrailer() == mTrailer->GetDrawable());

      mGM->AddActor(*mVehicle, remote, false);

      dtCore::AppSleep(17);
      dtCore::System::GetInstance().Step();

      // Note the actors should not fall any, because of the Keep above ground code.
      // if this test fails later, one should check to see if that is the cause.

      mVehicle->GetDrawable(tx);
      tx->GetTransform(xform);
      mTrailer->GetDrawable(tx);
      tx->GetTransform(xformTrailer);

      CPPUNIT_ASSERT(dtUtil::Equivalent(xform.GetRotation(), xformTrailer.GetRotation(), 1.0f));
      // Rough check to make sure the trailer is in the about the right place.
      CPPUNIT_ASSERT(xform.GetTranslation()[0] < xformTrailer.GetTranslation()[0]);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(xform.GetTranslation()[1], xformTrailer.GetTranslation()[1], 0.5f);
      CPPUNIT_ASSERT(xform.GetTranslation()[2] < xformTrailer.GetTranslation()[2]);

      CPPUNIT_ASSERT(trailerAC->GetAttached());

      bool parented = mTrailer->GetDrawable()->GetParent() == mVehicle->GetDrawable();

      CPPUNIT_ASSERT_EQUAL(enableRemoteMovement && remote, parented);

      dtGame::DeadReckoningHelper* drHelperTrailer = nullptr;
      mTrailer->GetComponent(drHelperTrailer);
      CPPUNIT_ASSERT(drHelperTrailer != nullptr);

      if (parented)
      {
         CPPUNIT_ASSERT_EQUAL_MESSAGE("the trailer should have dr turned off when it's attached.",
                  dtGame::DeadReckoningAlgorithm::NONE, drHelperTrailer->GetDeadReckoningAlgorithm());
      }

      trailerAC->Detach();

      CheckDetached(*trailerAC);
   }

   void TestRotateTrailer()
   {
      std::shared_ptr<SimCore::ActComps::TrailerHitchActComp> trailerAC = new SimCore::ActComps::TrailerHitchActComp();

      mVehicle->AddComponent(*trailerAC);

      trailerAC->SetTrailerActorId(mTrailer->GetId());
      trailerAC->SetUseCurrentHitchRotToMoveTrailerWhenRemote(true);

      osg::Vec3 firstRot(37.6f, 6.1f, -2.2f);
      trailerAC->SetCurrentHitchRotHPR(firstRot);
      CPPUNIT_ASSERT_EQUAL(firstRot, trailerAC->GetCurrentHitchRotHPR());

      CPPUNIT_ASSERT(!trailerAC->GetAttached());

      mGM->AddActor(*mTrailer, true, false);

      CPPUNIT_ASSERT(trailerAC->LookupTrailer() == mTrailer->GetDrawable());

      mGM->AddActor(*mVehicle, true, false);

      dtCore::AppSleep(17);
      dtCore::System::GetInstance().Step();

      dtCore::Transform xform;
      dtCore::Transformable* tx = nullptr;
      mTrailer->GetDrawable(tx);

      // Since the vehicle is at identity rotation, the rotation of the trailer should match the hitch rotation.
      tx->GetTransform(xform);
      CPPUNIT_ASSERT(dtUtil::Equivalent(firstRot, xform.GetRotation(), 1.0f));
      CPPUNIT_ASSERT(dtUtil::Equivalent(firstRot, trailerAC->GetCurrentHitchRotHPR(), 1.0f));

      // Set to identity
      osg::Vec3 secondRot(0.0f, 0.0f, 0.0f);
      trailerAC->SetCurrentHitchRotHPR(secondRot);

      tx->GetTransform(xform);
      CPPUNIT_ASSERT(dtUtil::Equivalent(secondRot, xform.GetRotation(), 1.0f));
      CPPUNIT_ASSERT(dtUtil::Equivalent(secondRot, trailerAC->GetCurrentHitchRotHPR(), 1.0f));

      // move the vehicle to the first rotation
      xform.SetRotation(firstRot);
      mVehicle->GetDrawable(tx);
      tx->SetTransform(xform);

      // The trailer should be the same because it should have moved with it.
      mTrailer->GetDrawable(tx);
      tx->GetTransform(xform);

      CPPUNIT_ASSERT(dtUtil::Equivalent(firstRot, xform.GetRotation(), 1.0f));
      CPPUNIT_ASSERT(dtUtil::Equivalent(secondRot, trailerAC->GetCurrentHitchRotHPR(), 1.0f));
   }

   void TestInvalidTrailerID()
   {
      std::shared_ptr<SimCore::ActComps::TrailerHitchActComp> trailerAC = new SimCore::ActComps::TrailerHitchActComp();

      mVehicle->AddComponent(*trailerAC);

      trailerAC->SetTrailerActorId(dtCore::UniqueId());

      CPPUNIT_ASSERT(trailerAC->LookupTrailer() == nullptr);

      mGM->AddActor(*mVehicle, false, false);

      CPPUNIT_ASSERT(trailerAC->LookupTrailer() == nullptr);
   }

   void TestCascadingDeletes()
   {
      std::shared_ptr<SimCore::ActComps::TrailerHitchActComp> trailerAC = new SimCore::ActComps::TrailerHitchActComp();

      mVehicle->AddComponent(*trailerAC);

      trailerAC->SetTrailerActorId(mTrailer->GetId());
      trailerAC->SetCascadeDeletes(true);

      mGM->AddActor(*mTrailer, false, false);
      mGM->AddActor(*mVehicle, false, false);

      dtCore::AppSleep(17);
      dtCore::System::GetInstance().Step();

      mGM->DeleteActor(*mVehicle);

      dtCore::AppSleep(17);
      dtCore::System::GetInstance().Step();

     CPPUNIT_ASSERT_MESSAGE("Trailer should be deleted", !mTrailer->IsInGM());
   }

   void TestCascadingDeletesOff()
   {
      std::shared_ptr<SimCore::ActComps::TrailerHitchActComp> trailerAC = new SimCore::ActComps::TrailerHitchActComp();

      mVehicle->AddComponent(*trailerAC);

      trailerAC->SetTrailerActorId(mTrailer->GetId());
      trailerAC->SetCascadeDeletes(false);

      mGM->AddActor(*mTrailer, false, false);
      mGM->AddActor(*mVehicle, false, false);

      dtCore::AppSleep(17);
      dtCore::System::GetInstance().Step();

      mGM->DeleteActor(*mVehicle);

      dtCore::AppSleep(17);
      dtCore::System::GetInstance().Step();

     CPPUNIT_ASSERT_MESSAGE("Trailer should be deleted", mTrailer->IsInGM());

     CheckDetached(*trailerAC);
   }

private:

   void CheckDetached(SimCore::ActComps::TrailerHitchActComp& trailerAC)
   {
      dtGame::DeadReckoningHelper* drHelper = nullptr;
      mVehicle->GetComponent(drHelper);
      dtGame::DeadReckoningHelper* drHelperTrailer = nullptr;
      mTrailer->GetComponent(drHelperTrailer);
      CPPUNIT_ASSERT(drHelperTrailer != nullptr);

      CPPUNIT_ASSERT(!trailerAC.GetAttached());

      bool parented = mTrailer->GetDrawable()->GetParent() == mVehicle->GetDrawable();
      bool inScene = mTrailer->GetDrawable()->GetParent() == &mGM->GetScene();

      CPPUNIT_ASSERT(!parented);
      CPPUNIT_ASSERT(inScene);
      CPPUNIT_ASSERT(trailerAC.GetTrailer() == nullptr);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("the detached trailer should just pick up the dr algorithm of its former parent so it can dr correctly.",
               drHelper->GetDeadReckoningAlgorithm(), drHelperTrailer->GetDeadReckoningAlgorithm());

   }

   std::shared_ptr<dtGame::GameManager> mGM;
   std::shared_ptr<Actors::FourWheelVehicleActorProxy> mVehicle;
   std::shared_ptr<Actors::FourWheelVehicleActorProxy> mTrailer;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TrailerHitchActCompTests);

}
