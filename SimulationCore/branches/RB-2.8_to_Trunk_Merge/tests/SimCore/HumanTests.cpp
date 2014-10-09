/* -*-c++-*-
* Simulation Core - HumanTests (.h & .cpp) - Using 'The MIT License'
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
* @author Eddie Johnson
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <osg/io_utils>
#include <osg/Math>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/drpublishingactcomp.h>
#include <dtGame/actorupdatemessage.h>

#include <dtCore/actorproperty.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/resourcedescriptor.h>

#include <dtCore/system.h>
#include <dtCore/refptr.h>
#include <dtCore/scene.h>

#include <dtUtil/macros.h>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/Human.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/ActComps/WeaponInventoryActComp.h>

#include <SimCore/Components/ViewerMessageProcessor.h>

#include <dtPhysics/physicsactcomp.h>

#include <UnitTestMain.h>
#include <dtABC/application.h>

using dtCore::RefPtr;
using dtCore::ObserverPtr;


class HumanTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(HumanTests);

      CPPUNIT_TEST(TestPlanDeployedToReady);
      CPPUNIT_TEST(TestPlanReadyToDeployed);
      CPPUNIT_TEST(TestPlanStandingToKneelingDeployed);
      CPPUNIT_TEST(TestPlanStandingToCrouchingNoWeaponMoving);
      CPPUNIT_TEST(TestCrouchingNoWeaponMovingToCrawlingReady);
      CPPUNIT_TEST(TestPlanWalkingReadyToKneelingDeployed);
      CPPUNIT_TEST(TestPlanShotStanding);
      CPPUNIT_TEST(TestPlanShotWalking);
      CPPUNIT_TEST(TestPlanShotRunning);
      CPPUNIT_TEST(TestPlanShotKneeling);
      CPPUNIT_TEST(TestPlanShotCrouching);
      CPPUNIT_TEST(TestPlanShotSquatting);
      CPPUNIT_TEST(TestPlanShotProne);
      CPPUNIT_TEST(TestPlanShotCrouching);
      CPPUNIT_TEST(TestPlanActionStanding);
      CPPUNIT_TEST(TestPlanActionKneeling);
      CPPUNIT_TEST(TestPlanActionProne);
      CPPUNIT_TEST(TestStartup);
      CPPUNIT_TEST(TestActorComponents);
      CPPUNIT_TEST(TestPrimaryWeapon);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
         dtCore::System::GetInstance().Start();
         mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
         mGM->SetApplication(GetGlobalApplication());
         mGM->AddComponent(*new dtGame::DeadReckoningComponent(), dtGame::GameManager::ComponentPriority::NORMAL);

         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, mHumanAP);

         dtGame::DRPublishingActComp* drPub = NULL;
         mHumanAP->GetComponent(drPub);
         // Stop it from updating the value.
         drPub->SetPublishLinearVelocity(false);
         drPub->SetPublishAngularVelocity(false);
      }

      void tearDown()
      {
         mHumanAP = NULL;

         if (mGM.valid())
         {
            mGM->DeleteAllActors(true);
            mGM = NULL;
         }
         dtCore::System::GetInstance().Stop();
      }

      void TestPrimaryWeapon()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         dtCore::StringActorProperty* meshNameProp;
         mHumanAP->GetProperty(SimCore::Actors::HumanActorProxy::PROPERTY_WEAPON_MESH, meshNameProp);
         CPPUNIT_ASSERT(meshNameProp != NULL);

         CPPUNIT_ASSERT_EQUAL(std::string("PrimaryWeapon"), meshNameProp->GetValue());
         CPPUNIT_ASSERT_EQUAL(std::string("PrimaryWeapon"), human->GetWeaponMeshName());

         std::string newValue("poo");
         meshNameProp->SetValue(newValue);

         CPPUNIT_ASSERT_EQUAL(newValue, meshNameProp->GetValue());
         CPPUNIT_ASSERT_EQUAL(newValue, human->GetWeaponMeshName());
      }

      void TestPlanReadyToDeployed()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               2U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_DEPLOYED_TO_READY.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_WALK_READY.Get(), (*iter)->GetName());
      }

      void TestPlanDeployedToReady()
      {
         try
         {
            SimCore::Actors::Human* human = NULL;
            mHumanAP->GetDrawable(human);

            human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
            human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);

            mGM->AddActor(*mHumanAP, false, false);
            // have to call this because the human ignores the plan if no model is set..
            human->UpdatePlanAndAnimations();

            human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
            human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

            CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                     human->GenerateNewAnimationSequence());
            const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

            CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
                  2U, unsigned(result.size()));

            dtAI::Planner::OperatorList::const_iterator iter = result.begin();

            CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_READY_TO_DEPLOYED.Get(), (*iter)->GetName());
            ++iter;

            CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_WALK_DEPLOYED.Get(), (*iter)->GetName());
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      void TestPlanStandingToKneelingDeployed()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               2U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_LOW_WALK_DEPLOYED.Get(), (*iter)->GetName());
      }

      void TestPlanStandingToCrouchingNoWeaponMoving()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::NO_WEAPON);

         dtGame::DRPublishingActComp* drPub = NULL;
         mHumanAP->GetComponent(drPub);
         // Stop it from updating the value.
         drPub->SetPublishLinearVelocity(false);

         drPub->SetVelocity(osg::Vec3(0.0f, 0.0f, 0.0f));

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::CROUCHING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::NO_WEAPON);

         drPub->SetVelocity(osg::Vec3(1.1f, 1.2f, 1.3f));
         human->SetMaxTimePerIteration(0.35);

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               2U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_LOW_WALK_DEPLOYED.Get(), (*iter)->GetName());
      }

      void TestCrouchingNoWeaponMovingToCrawlingReady()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         dtGame::DRPublishingActComp* drPub = NULL;
         mHumanAP->GetComponent(drPub);
         // Stop it from updating the value.
         drPub->SetPublishLinearVelocity(false);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::CROUCHING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::NO_WEAPON);

         drPub->SetVelocity(osg::Vec3(1.1f, 0.3f, 0.4f));

         human->SetMaxTimePerIteration(0.25);

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::CRAWLING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);

         drPub->SetVelocity(osg::Vec3(1.1f, 1.2f, 1.3f));

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               3U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEEL_TO_PRONE.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_DEPLOYED_TO_READY.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_CRAWL_READY.Get(), (*iter)->GetName());
      }

      void TestPlanShotStanding()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_STANDING,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_STANDING);
      }

      void TestPlanShotWalking()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_STANDING,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_STANDING);
      }

      void TestPlanShotRunning()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_RUNNING,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_STANDING,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_STANDING);
      }

      void TestPlanShotKneeling()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_KNEELING,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_KNEELING);
      }

      void TestPlanShotCrouching()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::CROUCHING,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_KNEELING,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_KNEELING);
      }

      void TestPlanShotSquatting()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::SQUATTING,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_KNEELING,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_KNEELING);
      }

      void TestPlanShotProne()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::PRONE,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_PRONE,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_PRONE);
      }

      void TestPlanShotCrawling()
      {
         TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum::CRAWLING,
                  SimCore::Actors::AnimationOperators::ANIM_SHOT_PRONE,
                  SimCore::Actors::AnimationOperators::ANIM_DEAD_PRONE);
      }

      void TestPlanWalkingReadyToKneelingDeployed()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         dtGame::DRPublishingActComp* drPub = NULL;
         mHumanAP->GetComponent(drPub);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);
         drPub->SetVelocity(osg::Vec3(0.0f,1.0f,0.0f));

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);
         drPub->SetVelocity(osg::Vec3(0.0f,0.0f,0.0f));

         human->SetMaxTimePerIteration(0.45);

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               3U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();


         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_READY_TO_DEPLOYED.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_LOW_WALK_DEPLOYED.Get(), (*iter)->GetName());
      }

      void TestPlanActionStanding()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         dtGame::DRPublishingActComp* drPub = NULL;
         mHumanAP->GetComponent(drPub);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::CROUCHING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::NO_WEAPON);

         drPub->SetVelocity(osg::Vec3(1.5f, 1.5f, 1.5f));

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING, "Shoot");
         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING, "Eat");
         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING, "Spit");

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               6U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEEL_TO_STAND.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STANDING_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STANDING_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STANDING_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_LOW_WALK_DEPLOYED.Get(), (*iter)->GetName());
      }

      void TestPlanActionKneeling()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::NO_WEAPON);

         dtGame::DRPublishingActComp* drPub = NULL;
         mHumanAP->GetComponent(drPub);

         drPub->SetVelocity(osg::Vec3(1.5f, 1.5f, 1.5f));

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING, "Shoot");
         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING, "Eat");
         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING, "Spit");
         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING, "Throw Rock");

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               7U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEELING_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEELING_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEELING_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEELING_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEEL_TO_STAND.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_WALK_DEPLOYED.Get(), (*iter)->GetName());
      }

      void TestPlanActionProne()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);
         //human->SetMaxTimePerIteration(2.00);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::NO_WEAPON);

         dtGame::DRPublishingActComp* drPub = NULL;
         mHumanAP->GetComponent(drPub);

         drPub->SetVelocity(osg::Vec3(1.5f, 1.5f, 1.5f));

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::PRONE, "Shoot");
         human->ExecuteAction(SimCore::Actors::HumanActorProxy::StanceEnum::PRONE, "Play Dead");

         //change the final stance
         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);

         CPPUNIT_ASSERT_MESSAGE("Plan failed - see error log. May have taken too long, or been impossible.",
                  human->GenerateNewAnimationSequence());
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               7U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEEL_TO_PRONE.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_PRONE_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_PRONE_ACTION.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_PRONE_TO_KNEEL.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_DEPLOYED_TO_READY.Get(), (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_LOW_WALK_READY.Get(), (*iter)->GetName());
         ++iter;
      }

      void TestStartup()
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         dtGame::DeadReckoningHelper* drHelper = NULL;
         mHumanAP->GetComponent(drHelper);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);
         drHelper->SetLastKnownVelocity(osg::Vec3(0.0f,0.0f,0.0f));

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               1U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_WALK_DEPLOYED.Get(), (*iter)->GetName());
      }

      void TestActorComponents()
      {
         dtCore::RefPtr<SimCore::Actors::HumanActorProxy> humanWithPhysics;
         dtCore::RefPtr<SimCore::Actors::HumanActorProxy> humanWarfighter;

         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_PHYSICS_ACTOR_TYPE, humanWithPhysics);
         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::WARFIGHTER_ACTOR_TYPE, humanWarfighter);

         CheckActorComponents(*mHumanAP, false, false);
         CheckActorComponents(*humanWithPhysics, true, false);
         CheckActorComponents(*humanWarfighter, true, true);
      }
   private:

      void CheckActorComponents(SimCore::Actors::HumanActorProxy& human, bool physics, bool weapons)
      {
         CPPUNIT_ASSERT_EQUAL(physics, human.GetComponent<dtPhysics::PhysicsActComp>() != NULL);
         CPPUNIT_ASSERT_EQUAL(weapons, human.GetComponent<SimCore::ActComps::WeaponInventoryActComp>() != NULL);
      }

      void TestPlanShot(SimCore::Actors::HumanActorProxy::StanceEnum& stance, const std::string& expectedShotAnim,
               const std::string& expectedDeadAnim )
      {
         SimCore::Actors::Human* human = NULL;
         mHumanAP->GetDrawable(human);

         human->SetStance(stance);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         mGM->AddActor(*mHumanAP, false, false);
         // have to call this because the human ignores the plan if no model is set..
         human->UpdatePlanAndAnimations();

         human->SetDamageState(SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED);

         bool shouldBeTrue = human->GenerateNewAnimationSequence();
         CPPUNIT_ASSERT(shouldBeTrue);
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();

         std::ostringstream opListText;

         opListText << "The plan: " << std::endl;

         dtAI::Planner::OperatorList::const_iterator i, end;
         i = result.begin();
         end = result.end();
         for (; i != result.end(); ++i)
         {
            opListText << "   ";
            opListText << (*i)->GetName() << std::endl;
         }

         CPPUNIT_ASSERT_EQUAL_MESSAGE(opListText.str(),
               2U, unsigned(result.size()));

         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(expectedShotAnim, (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(expectedDeadAnim, (*iter)->GetName());
      }

      RefPtr<dtGame::GameManager> mGM;
      RefPtr<SimCore::Actors::HumanActorProxy> mHumanAP;

      std::vector<std::string> mPR_UprightWalk_To_KneelFiring;
      std::vector<std::string> mPR_KneelFiring_To_ProneDeployed;
      std::vector<std::string> mPR_Prone_To_KneelFiring;
};

CPPUNIT_TEST_SUITE_REGISTRATION(HumanTests);

