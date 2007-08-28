/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <osg/io_utils>
#include <osg/Math>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/actorupdatemessage.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/project.h>
#include <dtDAL/resourcedescriptor.h>

#include <dtCore/system.h>
#include <dtCore/refptr.h>

#include <dtUtil/macros.h>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/Human.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <SimCore/Components/ViewerMessageProcessor.h>

//#include "TestComponent.h"

#ifdef DELTA_WIN32
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using dtCore::RefPtr;
using dtCore::ObserverPtr;


class HumanActorProxyTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(HumanActorProxyTests);

      CPPUNIT_TEST(TestPlanDeployedToReady);
      CPPUNIT_TEST(TestPlanReadyToDeployed);
      CPPUNIT_TEST(TestPlanStandingToKneelingDeployed);
      CPPUNIT_TEST(TestPlanWalkingReadyToKneelingDeployed);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp()
      {
         dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
         dtCore::System::GetInstance().Start();
         mGM = new dtGame::GameManager(*new dtCore::Scene());
         
         mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, mHumanAP);
      }

      void tearDown()
      {
         if (mGM.valid())
         {
            mGM->DeleteAllActors(true);
            mGM = NULL;
         }
         dtCore::System::GetInstance().Stop();
      }

      void TestPlanReadyToDeployed()
      {
         SimCore::Actors::Human* human;
         mHumanAP->GetActor(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         mGM->AddActor(*mHumanAP, false, false);
         
         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);

         human->GenerateNewAnimationSequence();
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();
         
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               2U, unsigned(result.size()));
         
         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_DEPLOYED_TO_READY, (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_READY, (*iter)->GetName());
      }

      void TestPlanDeployedToReady()
      {
         SimCore::Actors::Human* human;
         mHumanAP->GetActor(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);

         mGM->AddActor(*mHumanAP, false, false);
         
         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         human->GenerateNewAnimationSequence();
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();
         
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               2U, unsigned(result.size()));
         
         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_READY_TO_DEPLOYED, (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_DEPLOYED, (*iter)->GetName());
      }

      void TestPlanStandingToKneelingDeployed()
      {
         SimCore::Actors::Human* human;
         mHumanAP->GetActor(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_STANDING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         mGM->AddActor(*mHumanAP, false, false);
         
         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);

         human->GenerateNewAnimationSequence();
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();
         
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               2U, unsigned(result.size()));
         
         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL, (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEEL_DEPLOYED, (*iter)->GetName());
      }

      void TestPlanWalkingReadyToKneelingDeployed()
      {
         SimCore::Actors::Human* human;
         mHumanAP->GetActor(human);

         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::UPRIGHT_WALKING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::FIRING_POSITION);
         human->SetVelocityVector(osg::Vec3(0.0f,1.0f,0.0f));
         
         mGM->AddActor(*mHumanAP, false, false);
         
         human->SetStance(SimCore::Actors::HumanActorProxy::StanceEnum::KNEELING);
         human->SetPrimaryWeaponState(SimCore::Actors::HumanActorProxy::WeaponStateEnum::DEPLOYED);
         human->SetVelocityVector(osg::Vec3(0.0f,0.0f,0.0f));

         human->GenerateNewAnimationSequence();
         const dtAI::Planner::OperatorList& result = human->GetCurrentPlan();
         
         CPPUNIT_ASSERT_EQUAL_MESSAGE("The plan length",
               4U, unsigned(result.size()));
         
         dtAI::Planner::OperatorList::const_iterator iter = result.begin();

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_READY, (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::OPER_READY_TO_DEPLOYED, (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_STAND_TO_KNEEL, (*iter)->GetName());
         ++iter;

         CPPUNIT_ASSERT_EQUAL(SimCore::Actors::AnimationOperators::ANIM_KNEEL_DEPLOYED, (*iter)->GetName());
      }

   private:

      RefPtr<dtGame::GameManager> mGM;
      RefPtr<SimCore::Actors::HumanActorProxy> mHumanAP;

      std::vector<std::string> mPR_UprightWalk_To_KneelFiring;
      std::vector<std::string> mPR_KneelFiring_To_ProneDeployed;
      std::vector<std::string> mPR_Prone_To_KneelFiring;
};

CPPUNIT_TEST_SUITE_REGISTRATION(HumanActorProxyTests);

